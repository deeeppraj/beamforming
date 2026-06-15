#include "givens.h"

// ============================================================================
// FUNCTIONS 1 & 2: CORDIC (vectoring and rotation modes)
// ============================================================================

void cordic_vectoring(t_cordic x_in, t_cordic y_in,
                      t_cordic &magnitude, t_cordic &angle) {
#pragma HLS INLINE
    t_cordic x = x_in, y = y_in, z = 0;
    if (x_in < 0) {
        x = -x_in; y = -y_in;
        z = (y_in >= 0) ? t_cordic(3.14159265) : t_cordic(-3.14159265);
    }
    CORDIC_VEC_LOOP: for (int i = 0; i < ITER; i++) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER
        t_cordic sigma   = (y >= 0) ? t_cordic(-1) : t_cordic(1);
        t_cordic x_shift = x >> i;
        t_cordic y_shift = y >> i;
        t_cordic x_new   = x - sigma * y_shift;
        t_cordic y_new   = y + sigma * x_shift;
        t_cordic z_new   = z - sigma * ATAN_TABLE[i];
        x = x_new; y = y_new; z = z_new;
    }
    magnitude = x * CORDIC_K_INV;
    angle     = z;
}

void cordic_rotation(t_cordic angle, t_cordic &cos_out, t_cordic &sin_out) {
#pragma HLS INLINE
    t_cordic x = CORDIC_K_INV, y = 0, z = angle;
    CORDIC_ROT_LOOP: for (int i = 0; i < ITER; i++) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER
        t_cordic sigma   = (z >= 0) ? t_cordic(1) : t_cordic(-1);
        t_cordic x_shift = x >> i;
        t_cordic y_shift = y >> i;
        t_cordic x_new   = x - sigma * y_shift;
        t_cordic y_new   = y + sigma * x_shift;
        t_cordic z_new   = z - sigma * ATAN_TABLE[i];
        x = x_new; y = y_new; z = z_new;
    }
    cos_out = x; sin_out = y;
}

// ============================================================================
// FUNCTION: CHOLESKY DECOMPOSITION
// ============================================================================

void cholesky_core(cplx_rxx  Rxx_in[M][M],
                   cplx_chol L_out[M][M],
                   cplx_chol Lt_out[M][M]) {
#pragma HLS ARRAY_PARTITION variable=Rxx_in cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=L_out  cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Lt_out cyclic factor=M dim=1

    INIT_ZEROS: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            L_out[i][j]  = cplx_chol(0, 0);
            Lt_out[i][j] = cplx_chol(0, 0);
        }
    }

    COL_LOOP_CHOL: for (int j = 0; j < M; j++) {

        // Diagonal: L[j][j] = sqrt(Rxx[j][j] - sum_k |L[j][k]|^2)
        t_accum sum_diag = 0;
        DIAG_DOT: for (int k = 0; k < j; k++) {
#pragma HLS PIPELINE II=1
            t_chol l_jk_r = L_out[j][k].real();
            t_chol l_jk_i = L_out[j][k].imag();
            sum_diag += t_accum(l_jk_r * l_jk_r) + t_accum(l_jk_i * l_jk_i);
        }

        t_rxx   rxx_jj   = Rxx_in[j][j].real();
        t_accum diag_arg = t_accum(rxx_jj) - sum_diag;

        // [FIX-3b] Clamp: guard against negative argument on non-PD Rxx.
        if (diag_arg < t_accum(0)) diag_arg = t_accum(0);

        // [FIX-3c] Cast to double for sqrt (hls::sqrt has no ap_fixed overload
        // at C-sim time in Vitis HLS 2025.2 — ambiguous without explicit cast).
        // At synthesis the double sqrt is still resolved in fixed-point HW;
        // this cast is C-sim-compatible and avoids the float-NaN path.
        t_chol l_jj = t_chol(hls::sqrt((double)diag_arg));

        L_out[j][j]  = cplx_chol(l_jj, 0);
        Lt_out[j][j] = cplx_chol(l_jj, 0);

        // [FIX: ternary ambiguity] Use if/else instead of ?: so the compiler
        // never needs to reconcile two different ap_fixed promotion widths.
        t_chol inv_l_jj;
        if (l_jj > t_chol(1e-6))
            inv_l_jj = t_chol(t_chol(1.0) / l_jj);   // explicit outer cast
        else
            inv_l_jj = t_chol(0);

        // Off-diagonal: L[i][j] = (Rxx[i][j] - sum_k L[i][k]*conj(L[j][k])) / L[j][j]
        ROW_LOOP_CHOL: for (int i = j + 1; i < M; i++) {
            cplx_accum sum_off(0, 0);
            OFF_DIAG_DOT: for (int k = 0; k < j; k++) {
#pragma HLS PIPELINE II=1
                t_chol l_ik_r = L_out[i][k].real();
                t_chol l_ik_i = L_out[i][k].imag();
                t_chol l_jk_r = L_out[j][k].real();
                t_chol l_jk_i = L_out[j][k].imag();
                sum_off.real(sum_off.real() + t_accum(l_ik_r*l_jk_r) + t_accum(l_ik_i*l_jk_i));
                sum_off.imag(sum_off.imag() + t_accum(l_ik_i*l_jk_r) - t_accum(l_ik_r*l_jk_i));
            }
            t_chol val_r = t_chol(t_accum(t_accum(Rxx_in[i][j].real()) - sum_off.real()) * t_accum(inv_l_jj));
            t_chol val_i = t_chol(t_accum(t_accum(Rxx_in[i][j].imag()) - sum_off.imag()) * t_accum(inv_l_jj));

            L_out[i][j]  = cplx_chol(val_r,  val_i);
            Lt_out[j][i] = cplx_chol(val_r, -val_i);
        }
    }
}

// ============================================================================
// FUNCTION 3: GIVENS QR DECOMPOSITION
// ============================================================================

void givens_qr(cplx_chol A_in[M][M], cplx_qmat Q[M][M], cplx_rmat R[M][M]) {
#pragma HLS ARRAY_PARTITION variable=R    cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Q    cyclic factor=M dim=1
#pragma HLS ARRAY_PARTITION variable=A_in cyclic factor=M dim=2

    cplx_cordic Rw[M][M];
    cplx_cordic Qw[M][M];
#pragma HLS ARRAY_PARTITION variable=Rw cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Qw cyclic factor=M dim=1

    INIT_ROW: for (int i = 0; i < M; i++) {
        INIT_COL: for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            Rw[i][j] = cplx_cordic(t_cordic(A_in[i][j].real()),
                                    t_cordic(A_in[i][j].imag()));
            Qw[i][j] = (i == j) ? cplx_cordic(t_cordic(1), t_cordic(0))
                                 : cplx_cordic(t_cordic(0), t_cordic(0));
        }
    }

    COL_LOOP: for (int j = 0; j < M; j++) {
        ROW_LOOP: for (int i = M-1; i > j; i--) {
            t_cordic b_real = Rw[i][j].real(), b_imag = Rw[i][j].imag();
            t_cordic abs_sq = b_real*b_real + b_imag*b_imag;

            if (abs_sq > CORDIC_THRESHOLD_SQ) {
                t_cordic a_real = Rw[j][j].real(), a_imag = Rw[j][j].imag();
                t_cordic mag_a, ang_a, mag_b, ang_b;

                cordic_vectoring(a_real, a_imag, mag_a, ang_a);
                cordic_vectoring(b_real, b_imag, mag_b, ang_b);

                t_cordic r_mag, rot_angle;
                cordic_vectoring(mag_a, mag_b, r_mag, rot_angle);

                t_cordic cs_mag, sn_mag;
                cordic_rotation(rot_angle, cs_mag, sn_mag);

                t_cordic cs_cos, cs_sin, sn_cos, sn_sin;
                cordic_rotation(-ang_a, cs_cos, cs_sin);
                cordic_rotation(-ang_b, sn_cos, sn_sin);

                cplx_cordic cs(cs_mag * cs_cos, cs_mag * cs_sin);
                cplx_cordic sn(sn_mag * sn_cos, sn_mag * sn_sin);
                cplx_cordic cs_conj( cs.real(), -cs.imag());
                cplx_cordic sn_conj( sn.real(), -sn.imag());

                RUPDATE: for (int k = 0; k < M; k++) {
#pragma HLS PIPELINE II=1
                    cplx_cordic rjk = Rw[j][k], rik = Rw[i][k];
                    Rw[j][k] = cplx_cordic(
                        cs.real()*rjk.real() - cs.imag()*rjk.imag()
                      + sn.real()*rik.real() - sn.imag()*rik.imag(),
                        cs.real()*rjk.imag() + cs.imag()*rjk.real()
                      + sn.real()*rik.imag() + sn.imag()*rik.real()
                    );
                    Rw[i][k] = cplx_cordic(
                       -sn_conj.real()*rjk.real() + sn_conj.imag()*rjk.imag()
                      + cs_conj.real()*rik.real() - cs_conj.imag()*rik.imag(),
                       -sn_conj.real()*rjk.imag() - sn_conj.imag()*rjk.real()
                      + cs_conj.real()*rik.imag() + cs_conj.imag()*rik.real()
                    );
                }

                QUPDATE: for (int r = 0; r < M; r++) {
#pragma HLS PIPELINE II=1
                    cplx_cordic qrj = Qw[r][j], qri = Qw[r][i];
                    Qw[r][j] = cplx_cordic(
                        cs_conj.real()*qrj.real() - cs_conj.imag()*qrj.imag()
                      + sn_conj.real()*qri.real() - sn_conj.imag()*qri.imag(),
                        cs_conj.real()*qrj.imag() + cs_conj.imag()*qrj.real()
                      + sn_conj.real()*qri.imag() + sn_conj.imag()*qri.real()
                    );
                    Qw[r][i] = cplx_cordic(
                       -sn.real()*qrj.real() + sn.imag()*qrj.imag()
                      + cs.real()*qri.real() - cs.imag()*qri.imag(),
                       -sn.real()*qrj.imag() - sn.imag()*qrj.real()
                      + cs.real()*qri.imag() + cs.imag()*qri.real()
                    );
                }
            }
        }
    }

    CAST_OUT: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            R[i][j] = cplx_rmat(t_rmat(Rw[i][j].real()), t_rmat(Rw[i][j].imag()));
            Q[i][j] = cplx_qmat(t_qmat(Qw[i][j].real()), t_qmat(Qw[i][j].imag()));
        }
    }
}

// ============================================================================
// FUNCTION 4: BACK-SUBSTITUTION
// ============================================================================

void back_substitution(cplx_qmat Q[M][M], cplx_rmat R[M][M], cplx_chol L[M][M],
                       cplx_steer a_s[M], cplx_weight w[M]) {
#pragma HLS ARRAY_PARTITION variable=Q   cyclic factor=M dim=1
#pragma HLS ARRAY_PARTITION variable=R   cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=L   cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=a_s complete
#pragma HLS ARRAY_PARTITION variable=w   complete

    cplx_cordic z[M];
#pragma HLS ARRAY_PARTITION variable=z complete

    // Step 1: Forward substitution — L * z = a_s
    FWD_ROW: for (int i = 0; i < M; i++) {
        cplx_cordic sum(t_cordic(0), t_cordic(0));
        FWD_COL: for (int k = 0; k < i; k++) {
#pragma HLS PIPELINE II=1
            cplx_cordic lik(t_cordic(L[i][k].real()), t_cordic(L[i][k].imag()));
            sum = cplx_cordic(
                sum.real() + lik.real()*z[k].real() - lik.imag()*z[k].imag(),
                sum.imag() + lik.real()*z[k].imag() + lik.imag()*z[k].real()
            );
        }
        t_cordic lii = t_cordic(L[i][i].real());
        // [FIX: ternary ambiguity] if/else avoids ap_fixed promotion width clash
        t_cordic inv_lii;
        if (lii > t_cordic(1e-6))
            inv_lii = t_cordic(t_cordic(1.0) / lii);
        else
            inv_lii = t_cordic(0);
        cplx_cordic as_i(t_cordic(a_s[i].real()), t_cordic(a_s[i].imag()));
        z[i] = cplx_cordic(
            (as_i.real() - sum.real()) * inv_lii,
            (as_i.imag() - sum.imag()) * inv_lii
        );
    }

    // Step 2: rhs = Q^H * z
    cplx_cordic rhs[M];
#pragma HLS ARRAY_PARTITION variable=rhs complete

    RHS_ROW: for (int i = 0; i < M; i++) {
        cplx_cordic acc(t_cordic(0), t_cordic(0));
        RHS_COL: for (int k = 0; k < M; k++) {
#pragma HLS PIPELINE II=1
            t_cordic qki_r =  t_cordic(Q[k][i].real());
            t_cordic qki_i = -t_cordic(Q[k][i].imag());
            acc = cplx_cordic(
                acc.real() + qki_r*z[k].real() - qki_i*z[k].imag(),
                acc.imag() + qki_r*z[k].imag() + qki_i*z[k].real()
            );
        }
        rhs[i] = acc;
    }

    // Step 3: Back substitution — R * w = rhs
    cplx_cordic w_unnorm[M];
#pragma HLS ARRAY_PARTITION variable=w_unnorm complete

    BCK_ROW: for (int i = M-1; i >= 0; i--) {
        cplx_cordic sum(t_cordic(0), t_cordic(0));
        BCK_COL: for (int k = i+1; k < M; k++) {
#pragma HLS PIPELINE II=1
            cplx_cordic rik(t_cordic(R[i][k].real()), t_cordic(R[i][k].imag()));
            sum = cplx_cordic(
                sum.real() + rik.real()*w_unnorm[k].real() - rik.imag()*w_unnorm[k].imag(),
                sum.imag() + rik.real()*w_unnorm[k].imag() + rik.imag()*w_unnorm[k].real()
            );
        }
        t_cordic rii_r = t_cordic(R[i][i].real());
        t_cordic rii_i = t_cordic(R[i][i].imag());
        t_cordic denom = rii_r*rii_r + rii_i*rii_i;
        // [FIX: ternary ambiguity] if/else avoids ap_fixed promotion width clash
        t_cordic inv_den;
        if (denom > t_cordic(1e-8))
            inv_den = t_cordic(t_cordic(1.0) / denom);
        else
            inv_den = t_cordic(0);
        cplx_cordic num(rhs[i].real() - sum.real(), rhs[i].imag() - sum.imag());
        w_unnorm[i] = cplx_cordic(
            (num.real()*rii_r + num.imag()*rii_i) * inv_den,
            (num.imag()*rii_r - num.real()*rii_i) * inv_den
        );
    }

    OUT_CAST: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=1
        w[i] = cplx_weight(t_weight(w_unnorm[i].real()), t_weight(w_unnorm[i].imag()));
    }
}

// ============================================================================
// FUNCTION 5: TOP FUNCTION (Architecture B — Integrated Cholesky)
// ============================================================================

void givens_qr_top(cplx_rxx    Rxx[M][M],
                   cplx_steer  a_s[M],
                   cplx_weight w_out[M])
{
#pragma HLS INTERFACE m_axi port=Rxx   depth=4096 offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=a_s   depth=64   offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=w_out depth=64   offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port=Rxx   bundle=control
#pragma HLS INTERFACE s_axilite port=a_s   bundle=control
#pragma HLS INTERFACE s_axilite port=w_out bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    cplx_rxx    Rxx_local[M][M];
    cplx_steer  a_s_local[M];
    cplx_chol   L_local[M][M];
    cplx_chol   Lt_local[M][M];
    cplx_qmat   Q_local[M][M];
    cplx_rmat   R_local[M][M];
    cplx_weight w_local[M];

#pragma HLS ARRAY_PARTITION variable=Rxx_local  cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=L_local    cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Lt_local   cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=a_s_local  complete
#pragma HLS ARRAY_PARTITION variable=Q_local    cyclic factor=M dim=1
#pragma HLS ARRAY_PARTITION variable=R_local    cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=w_local    complete

    READ_RXX: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            Rxx_local[i][j] = Rxx[i][j];
        }
    }
    READ_AS: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=1
        a_s_local[i] = a_s[i];
    }

    cholesky_core(Rxx_local, L_local, Lt_local);
    givens_qr(Lt_local, Q_local, R_local);
    back_substitution(Q_local, R_local, L_local, a_s_local, w_local);

    WRITE_W: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=1
        w_out[i] = w_local[i];
    }
}