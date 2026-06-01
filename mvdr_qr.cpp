// ============================================================
//  mvdr_qr.cpp  –  QR-MVDR Beamformer HLS Kernel
//  Versal VCK190 | Vitis HLS 2024.1
// ============================================================
#include "mvdr_qr.h"
#include <hls_math.h>

// ──────────────────────────────────────────────────────────────
//  TOP-LEVEL FUNCTION
// ──────────────────────────────────────────────────────────────
void mvdr_qr_top(
    cdata_t   X[M * N_SNAP],
    cdata_t   a_s[M],
    cweight_t w_out[M])
{
#pragma HLS INTERFACE m_axi port=X     offset=slave bundle=gmem0 depth=512
#pragma HLS INTERFACE m_axi port=a_s   offset=slave bundle=gmem1 depth=8
#pragma HLS INTERFACE m_axi port=w_out offset=slave bundle=gmem2 depth=8
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Local buffers (on-chip BRAM/URAM)
    cacc_t R_mat[M * M];
    cacc_t y_vec[M];
    cacc_t z_vec[M];

#pragma HLS ARRAY_PARTITION variable=R_mat cyclic factor=8
#pragma HLS ARRAY_PARTITION variable=y_vec complete
#pragma HLS ARRAY_PARTITION variable=z_vec complete

    // Step 1 – Form R = X * X^H / N_snap
    compute_R(X, R_mat);

    // Step 2 – In-place Householder QR  =>  R_mat becomes upper triangular R
    cacc_t Q_dummy[M * M];  // not used downstream
    householder_qr(R_mat, Q_dummy);

    // Step 3 – Forward sub: R^H * y = a_s
    cacc_t a_acc[M];
    COPY_A: for (int i = 0; i < M; i++) {
#pragma HLS UNROLL
        a_acc[i] = cacc_t((acc_t)a_s[i].real(), (acc_t)a_s[i].imag());
    }
    forward_substitution(R_mat, a_acc, y_vec);

    // Step 4 – Back sub: R * z = y
    back_substitution(R_mat, y_vec, z_vec);

    // Step 5 – Normalize: w = z / (a_s^H * z)
    normalize_weight(z_vec, a_s, w_out);
}

// ──────────────────────────────────────────────────────────────
//  STEP 1 – SAMPLE COVARIANCE MATRIX  R = X * X^H / N
// ──────────────────────────────────────────────────────────────
void compute_R(cdata_t X[M * N_SNAP], cacc_t R[M * M])
{
#pragma HLS PIPELINE II=1

    INIT_R: for (int idx = 0; idx < M * M; idx++) {
#pragma HLS UNROLL
        R[idx] = cacc_t(0, 0);
    }

    // R(i,j) = sum_n X(i,n) * conj(X(j,n))
    R_ROW: for (int i = 0; i < M; i++) {
        R_COL: for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            cacc_t sum(0, 0);
            R_SNAP: for (int n = 0; n < N_SNAP; n++) {
#pragma HLS UNROLL factor=8
                cdata_t xi = X[i * N_SNAP + n];
                cdata_t xj = X[j * N_SNAP + n];
                // xi * conj(xj)
                acc_t re = (acc_t)xi.real() * (acc_t)xj.real()
                         + (acc_t)xi.imag() * (acc_t)xj.imag();
                acc_t im = (acc_t)xi.imag() * (acc_t)xj.real()
                         - (acc_t)xi.real() * (acc_t)xj.imag();
                sum += cacc_t(re, im);
            }
            // divide by N_snap (use shift for power-of-2)
            // N_SNAP=64 -> right shift 6
            R[i * M + j] = cacc_t(sum.real() >> 6, sum.imag() >> 6);
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  STEP 2 – HOUSEHOLDER QR DECOMPOSITION (in-place)
//  Input  : A (M x M Hermitian PD)
//  Output : A overwritten with upper-triangular R
// ──────────────────────────────────────────────────────────────
void householder_qr(cacc_t A[M * M], cacc_t Q[M * M])
{
    // Initialize Q = I
    INIT_Q: for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE
            Q[i * M + j] = (i == j) ? cacc_t(1, 0) : cacc_t(0, 0);
        }

    QR_COL: for (int k = 0; k < M; k++) {
#pragma HLS PIPELINE off

        // --- Extract column k from row k downward ---
        cacc_t v[M];
        acc_t  norm_sq(0);

        EXTRACT: for (int i = k; i < M; i++) {
#pragma HLS UNROLL
            v[i] = A[i * M + k];
            norm_sq += v[i].real() * v[i].real()
                     + v[i].imag() * v[i].imag();
        }

        // norm = sqrt(norm_sq)
        acc_t norm_val = hls::sqrt((float)norm_sq);

        // Compute Householder reflector sign
        // alpha = -sign(v[k]) * norm
        acc_t  vk_re  = v[k].real();
        acc_t  alpha  = (vk_re >= 0) ? -norm_val : norm_val;

        // u = v + alpha * e_k
        v[k] = cacc_t(v[k].real() + alpha, v[k].imag());

        // Compute ||u||^2
        acc_t u_norm_sq(0);
        UNORM: for (int i = k; i < M; i++) {
#pragma HLS UNROLL
            u_norm_sq += v[i].real() * v[i].real()
                       + v[i].imag() * v[i].imag();
        }

        if (u_norm_sq == acc_t(0)) continue;

        acc_t inv_u2 = acc_t(2.0f) / (float)u_norm_sq;

        // Apply H = I - 2*u*u^H / ||u||^2 to A from left
        // A := H * A  (only columns k..M-1)
        APP_LEFT: for (int j = k; j < M; j++) {
#pragma HLS PIPELINE II=2
            // dot = u^H * A[:,j]  (rows k..M-1)
            cacc_t dot(0, 0);
            DOT_L: for (int i = k; i < M; i++) {
#pragma HLS UNROLL
                // conj(v[i]) * A[i,j]
                acc_t re =  v[i].real() * A[i*M+j].real()
                          + v[i].imag() * A[i*M+j].imag();
                acc_t im =  v[i].real() * A[i*M+j].imag()
                          - v[i].imag() * A[i*M+j].real();
                dot += cacc_t(re, im);
            }
            dot *= cacc_t(inv_u2, 0);
            // A[i,j] -= v[i] * dot
            SUB_L: for (int i = k; i < M; i++) {
#pragma HLS UNROLL
                acc_t re = v[i].real()*dot.real() - v[i].imag()*dot.imag();
                acc_t im = v[i].real()*dot.imag() + v[i].imag()*dot.real();
                A[i*M+j] -= cacc_t(re, im);
            }
        }

        // Apply H to Q from right: Q := Q * H^H = Q * H  (H = H^H)
        APP_RIGHT: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=2
            cacc_t dot(0, 0);
            DOT_R: for (int j = k; j < M; j++) {
#pragma HLS UNROLL
                acc_t re = Q[i*M+j].real()*v[j].real()
                         + Q[i*M+j].imag()*v[j].imag();
                acc_t im = Q[i*M+j].imag()*v[j].real()
                         - Q[i*M+j].real()*v[j].imag();
                dot += cacc_t(re, im);
            }
            dot *= cacc_t(inv_u2, 0);
            DOT_RS: for (int j = k; j < M; j++) {
#pragma HLS UNROLL
                acc_t re = dot.real()*v[j].real() - dot.imag()*v[j].imag();
                acc_t im = dot.real()*v[j].imag() + dot.imag()*v[j].real();
                Q[i*M+j] -= cacc_t(re, im);
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  STEP 3 – FORWARD SUBSTITUTION  L * y = b  (L = R^H)
// ──────────────────────────────────────────────────────────────
void forward_substitution(cacc_t U[M*M], cacc_t b[M], cacc_t y[M])
{
    // U is upper triangular; L = U^H is lower triangular
    // Solve L * y = b  row by row
    FWD_ROW: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=2
        cacc_t sum(0, 0);
        FWD_COL: for (int j = 0; j < i; j++) {
#pragma HLS UNROLL
            // L[i,j] = conj(U[j,i])
            cacc_t Lij( U[j*M+i].real(), -U[j*M+i].imag());
            acc_t re = Lij.real()*y[j].real() - Lij.imag()*y[j].imag();
            acc_t im = Lij.real()*y[j].imag() + Lij.imag()*y[j].real();
            sum += cacc_t(re, im);
        }
        // L[i,i] = conj(U[i,i])  (diagonal, imag usually 0 after QR)
        cacc_t Lii(U[i*M+i].real(), -U[i*M+i].imag());
        cacc_t num = b[i] - sum;
        // Complex division by Lii
        acc_t denom = Lii.real()*Lii.real() + Lii.imag()*Lii.imag();
        y[i] = cacc_t(
            (num.real()*Lii.real() + num.imag()*Lii.imag()) / (float)denom,
            (num.imag()*Lii.real() - num.real()*Lii.imag()) / (float)denom
        );
    }
}

// ──────────────────────────────────────────────────────────────
//  STEP 4 – BACK SUBSTITUTION  U * z = y
// ──────────────────────────────────────────────────────────────
void back_substitution(cacc_t U[M*M], cacc_t y[M], cacc_t z[M])
{
    BACK_ROW: for (int i = M-1; i >= 0; i--) {
#pragma HLS PIPELINE II=2
        cacc_t sum(0, 0);
        BACK_COL: for (int j = i+1; j < M; j++) {
#pragma HLS UNROLL
            acc_t re = U[i*M+j].real()*z[j].real() - U[i*M+j].imag()*z[j].imag();
            acc_t im = U[i*M+j].real()*z[j].imag() + U[i*M+j].imag()*z[j].real();
            sum += cacc_t(re, im);
        }
        cacc_t Uii = U[i*M+i];
        cacc_t num  = y[i] - sum;
        acc_t  denom = Uii.real()*Uii.real() + Uii.imag()*Uii.imag();
        z[i] = cacc_t(
            (num.real()*Uii.real() + num.imag()*Uii.imag()) / (float)denom,
            (num.imag()*Uii.real() - num.real()*Uii.imag()) / (float)denom
        );
    }
}

// ──────────────────────────────────────────────────────────────
//  STEP 5 – NORMALIZE  w = z / (a_s^H * z)
// ──────────────────────────────────────────────────────────────
void normalize_weight(cacc_t z[M], cdata_t a_s[M], cweight_t w_out[M])
{
    cacc_t dot(0, 0);
    DOT_NORM: for (int i = 0; i < M; i++) {
#pragma HLS UNROLL
        // conj(a_s[i]) * z[i]
        acc_t re = (acc_t)a_s[i].real()*z[i].real()
                 + (acc_t)a_s[i].imag()*z[i].imag();
        acc_t im = (acc_t)a_s[i].real()*z[i].imag()
                 - (acc_t)a_s[i].imag()*z[i].real();
        dot += cacc_t(re, im);
    }
    // w = z / dot  (complex division)
    acc_t d2 = dot.real()*dot.real() + dot.imag()*dot.imag();
    WRITE_W: for (int i = 0; i < M; i++) {
#pragma HLS UNROLL
        acc_t wr = (z[i].real()*dot.real() + z[i].imag()*dot.imag()) / (float)d2;
        acc_t wi = (z[i].imag()*dot.real() - z[i].real()*dot.imag()) / (float)d2;
        w_out[i] = cweight_t((weight_t)wr, (weight_t)wi);
    }
}
