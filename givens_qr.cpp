// ============================================================================
// givens_qr.cpp  —  Givens QR decomposition with CORDIC for Versal VCK190
// ============================================================================
// What this file implements:
//   1. cordic_vectoring : computes magnitude and angle of (x,y) using shifts
//                         and additions only — no sqrt, no division
//   2. cordic_rotation  : computes cos/sin of an angle — same hardware
//   3. givens_qr        : Givens rotation loop zeroing sub-diagonal entries
//                         uses CORDIC for the rotation coefficients cs, sn
//   4. back_substitution: forward sub on L, then back sub on R via QR factors
//   5. givens_qr_top    : top-level function with AXI interfaces
//
// HLS pragmas used:
//   PIPELINE   : allow loop iterations to overlap (throughput = 1 cycle)
//   ARRAY_PARTITION : split arrays across BRAMs for parallel access
//   INTERFACE  : define AXI ports on the top function
//   INLINE     : inline small functions to avoid call overhead
// ============================================================================

#include "givens_qr.h"

// ============================================================================
// FUNCTION 1: CORDIC VECTORING MODE
// ============================================================================
// Purpose: given two real numbers x_in and y_in, compute:
//   magnitude = sqrt(x_in² + y_in²)   (with CORDIC gain compensation)
//   angle     = atan2(y_in, x_in)     in radians
//
// How CORDIC vectoring works:
//   Start with vector (x, y). Repeatedly rotate it toward the x-axis.
//   After ITER iterations, y → 0 and x → magnitude × K (CORDIC gain).
//   The accumulated rotation angle = atan2(y_in, x_in).
//
//   At each step i:
//     if y > 0 : rotate clockwise   (subtract atan(2^-i))
//     if y < 0 : rotate anti-clock  (add atan(2^-i))
//     x_new = x - sigma * y * 2^-i
//     y_new = y + sigma * x * 2^-i
//     z_new = z - sigma * atan(2^-i)
//   where sigma = sign(y)
//
//   Key: multiplying by 2^-i is just a right bit-shift — free on hardware!
// ============================================================================
void cordic_vectoring(t_cordic  x_in,
                      t_cordic  y_in,
                      t_cordic &magnitude,
                      t_cordic &angle)
{
    #pragma HLS INLINE  // inline into caller — avoids function call overhead

    t_cordic x = x_in;
    t_cordic y = y_in;
    t_cordic z = 0;     // angle accumulator

    // Handle quadrant: CORDIC only works in [-pi/2, pi/2]
    // Rotate input to first/fourth quadrant if needed
    if (x_in < 0) {
        x = -x_in;
        y = -y_in;
        z = (y_in >= 0) ? t_cordic(3.14159265f) : t_cordic(-3.14159265f);
    }

    // Main CORDIC iteration loop
    // PIPELINE pragma: each iteration starts 1 cycle after previous
    // However, loop-carried dependency on x,y,z means II > 1 here —
    // HLS will schedule II=3 or II=4 automatically. This is expected.
    CORDIC_LOOP: for (int i = 0; i < ITER; i++) {
        #pragma HLS PIPELINE
        #pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER

        t_cordic sigma  = (y >= 0) ? t_cordic(-1) : t_cordic(1);
        // Bit shift: x >> i  (right shift by i positions = multiply by 2^-i)
        t_cordic x_shift = x >> i;   // = x * 2^-i
        t_cordic y_shift = y >> i;   // = y * 2^-i

        t_cordic x_new = x - sigma * y_shift;
        t_cordic y_new = y + sigma * x_shift;
        t_cordic z_new = z - sigma * t_cordic(ATAN_TABLE[i]);

        x = x_new;
        y = y_new;
        z = z_new;
    }

    // x now holds magnitude × CORDIC_gain (K ≈ 1.6468)
    // Compensate by multiplying by 1/K = CORDIC_K_INV ≈ 0.6073
    magnitude = x * t_cordic(CORDIC_K_INV);
    angle     = z;
}

// ============================================================================
// FUNCTION 2: CORDIC ROTATION MODE
// ============================================================================
// Purpose: given an angle, compute cos(angle) and sin(angle)
//
// How CORDIC rotation works:
//   Start with vector (1/K, 0) — pre-compensated unit vector.
//   Rotate it by the target angle using the same shift-add structure.
//   After ITER iterations, x → cos(angle), y → sin(angle).
//
//   At each step i:
//     if z > 0 : rotate anti-clockwise (positive direction)
//     if z < 0 : rotate clockwise
//     x_new = x - sigma * y * 2^-i
//     y_new = y + sigma * x * 2^-i
//     z_new = z - sigma * atan(2^-i)
// ============================================================================
void cordic_rotation(t_cordic  angle,
                     t_cordic &cos_out,
                     t_cordic &sin_out)
{
    #pragma HLS INLINE

    // Start with CORDIC-gain-compensated unit vector
    t_cordic x = t_cordic(CORDIC_K_INV);   // 1/K ≈ 0.6073
    t_cordic y = 0;
    t_cordic z = angle;

    CORDIC_ROT_LOOP: for (int i = 0; i < ITER; i++) {
        #pragma HLS PIPELINE
        #pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER

        t_cordic sigma  = (z >= 0) ? t_cordic(1) : t_cordic(-1);
        t_cordic x_shift = x >> i;
        t_cordic y_shift = y >> i;

        t_cordic x_new = x - sigma * y_shift;
        t_cordic y_new = y + sigma * x_shift;
        t_cordic z_new = z - sigma * t_cordic(ATAN_TABLE[i]);

        x = x_new;
        y = y_new;
        z = z_new;
    }

    cos_out = x;   // cos(angle)
    sin_out = y;   // sin(angle)
}

// ============================================================================
// FUNCTION 3: GIVENS QR DECOMPOSITION
// ============================================================================
// Purpose: decompose complex matrix A (M×M) into Q (unitary) and R (upper
//          triangular) such that A = Q * R, using Givens rotations.
//
// Algorithm (same as your MATLAB givensQR but in C++ with CORDIC):
//   for j = 0 to M-1             (column)
//     for i = M-1 down to j+1   (row, bottom to top)
//       compute CORDIC rotation to zero A[i][j]
//       apply rotation to rows j and i of R
//       accumulate into Q via columns j and i
//
// Hardware notes:
//   - Outer loop (j) : M iterations, sequential (loop-carried dependency)
//   - Inner loop (i) : M-j-1 iterations, sequential (row update)
//   - Row update (k) : M iterations over columns — PIPELINE this
//   - ARRAY_PARTITION on R and Q allows parallel column access in k-loop
// ============================================================================
void givens_qr(cplx_chol A_in[M][M],
               cplx_qmat Q[M][M],
               cplx_rmat R[M][M])
{
    // ── Array partitioning ───────────────────────────────────────────────────
    // Partition R and Q along their column dimension (dim=2 for R, dim=1 for Q)
    // cyclic factor=M means every column gets its own BRAM bank —
    // allows the k-loop to read/write M elements per cycle.
    // For M=8  : full unroll possible (8 elements → registers, not BRAM)
    // For M=64 : change factor to 8 (8 banks, 8 elements each)
    #pragma HLS ARRAY_PARTITION variable=R cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=Q cyclic factor=M dim=1
    #pragma HLS ARRAY_PARTITION variable=A_in cyclic factor=M dim=2

    // Internal working copy (wider type to avoid precision loss during rotation)
    cplx_cordic Rw[M][M];    // working R (CORDIC precision)
    cplx_cordic Qw[M][M];    // working Q (CORDIC precision)

    #pragma HLS ARRAY_PARTITION variable=Rw cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=Qw cyclic factor=M dim=1

    // ── Initialise: R ← A,  Q ← Identity ───────────────────────────────────
    INIT_ROW: for (int i = 0; i < M; i++) {
        INIT_COL: for (int j = 0; j < M; j++) {
            #pragma HLS PIPELINE II=1
            Rw[i][j] = cplx_cordic(t_cordic(A_in[i][j].real()),
                                    t_cordic(A_in[i][j].imag()));
            Qw[i][j] = (i == j) ? cplx_cordic(t_cordic(1), t_cordic(0))
                                 : cplx_cordic(t_cordic(0), t_cordic(0));
        }
    }

    // ── Main Givens rotation loops ───────────────────────────────────────────
    COL_LOOP: for (int j = 0; j < M; j++) {
        // No pipeline on outer loop — sequential column processing required

        ROW_LOOP: for (int i = M-1; i > j; i--) {
            // No pipeline on row loop — sequential row processing required
            // Each (i,j) rotation depends on the previous one in same column

            // ── Step 1: check if element needs zeroing ───────────────────────
            // Small threshold avoids CORDIC on near-zero elements
            t_cordic r_real = Rw[i][j].real();
            t_cordic r_imag = Rw[i][j].imag();

            t_cordic abs_rij = hls::sqrt(r_real*r_real + r_imag*r_imag);

            if (abs_rij > t_cordic(1e-6)) {

                // ── Step 2: CORDIC to get rotation magnitude and angle ────────
                // We need cs = a/r,  sn = -b/r
                // where a = R[j][j],  b = R[i][j],  r = sqrt(|a|²+|b|²)
                //
                // For complex entries, we use the magnitude of each:
                //   |a| = |R[j][j]|,   |b| = |R[i][j]|
                //   r   = sqrt(|a|²+|b|²)  via CORDIC vectoring
                //   angle = atan2(|b|, |a|)
                //   cs  = cos(angle) = |a|/r
                //   sn  = sin(angle) = |b|/r
                // Phase of cs/sn are adjusted from phases of a and b.

                t_cordic a_real = Rw[j][j].real();
                t_cordic a_imag = Rw[j][j].imag();
                t_cordic b_real = Rw[i][j].real();
                t_cordic b_imag = Rw[i][j].imag();

                // Magnitude of a and b
                t_cordic mag_a, ang_a, mag_b, ang_b;
                cordic_vectoring(a_real, a_imag, mag_a, ang_a);
                cordic_vectoring(b_real, b_imag, mag_b, ang_b);

                // r = sqrt(mag_a² + mag_b²) via CORDIC vectoring
                t_cordic r_mag, dummy_ang;
                cordic_vectoring(mag_a, mag_b, r_mag, dummy_ang);

                // cs = mag_a/r,  sn = mag_b/r  via CORDIC rotation
                // Instead of dividing, use: cs = cos(atan2(mag_b, mag_a))
                //                           sn = sin(atan2(mag_b, mag_a))
                t_cordic rot_angle, dum2;
                cordic_vectoring(mag_a, mag_b, dum2, rot_angle);

                t_cordic cs_mag, sn_mag;
                cordic_rotation(rot_angle, cs_mag, sn_mag);

                // cs (complex): magnitude cs_mag, phase = phase of a = ang_a
                // sn (complex): magnitude sn_mag, phase = phase of -b = ang_b+pi
                t_cordic cs_cos, cs_sin, sn_cos, sn_sin;
                cordic_rotation(ang_a,                  cs_cos, cs_sin);
                cordic_rotation(ang_b + t_cordic(3.14159265f), sn_cos, sn_sin);

                cplx_cordic cs(cs_mag * cs_cos, cs_mag * cs_sin);
                cplx_cordic sn(sn_mag * sn_cos, sn_mag * sn_sin);
                // Conjugate of sn for the Givens matrix
                cplx_cordic sn_conj(sn.real(), -sn.imag());
                cplx_cordic cs_conj(cs.real(), -cs.imag());

                // ── Step 3: Apply rotation to rows j and i of R ──────────────
                // [R[j][k]]   =  [ cs    -sn ] [R[j][k]]
                // [R[i][k]]      [ sn*   cs* ] [R[i][k]]
                // for all k = 0..M-1
                RUPDATE: for (int k = 0; k < M; k++) {
                    #pragma HLS PIPELINE II=1
                    // Read current values
                    cplx_cordic rjk = Rw[j][k];
                    cplx_cordic rik = Rw[i][k];

                    // Complex multiply:  cs*rjk - sn*rik
                    // (a+jb)(c+jd) = ac-bd + j(ad+bc)
                    cplx_cordic new_rjk(
                        cs.real()*rjk.real() - cs.imag()*rjk.imag()
                      - sn.real()*rik.real() + sn.imag()*rik.imag(),
                        cs.real()*rjk.imag() + cs.imag()*rjk.real()
                      - sn.real()*rik.imag() - sn.imag()*rik.real()
                    );

                    // sn_conj*rjk + cs_conj*rik
                    cplx_cordic new_rik(
                        sn_conj.real()*rjk.real() - sn_conj.imag()*rjk.imag()
                      + cs_conj.real()*rik.real() - cs_conj.imag()*rik.imag(),
                        sn_conj.real()*rjk.imag() + sn_conj.imag()*rjk.real()
                      + cs_conj.real()*rik.imag() + cs_conj.imag()*rik.real()
                    );

                    Rw[j][k] = new_rjk;
                    Rw[i][k] = new_rik;
                }

                // ── Step 4: Accumulate Q — update columns j and i ────────────
                // Q = Q * G^H
                // [Q[r][j]  Q[r][i]] = [Q[r][j]  Q[r][i]] * [cs*  sn* ]
                //                                             [-sn  cs  ]
                QUPDATE: for (int r = 0; r < M; r++) {
                    #pragma HLS PIPELINE II=1
                    cplx_cordic qrj = Qw[r][j];
                    cplx_cordic qri = Qw[r][i];

                    cplx_cordic new_qrj(
                        cs_conj.real()*qrj.real() - cs_conj.imag()*qrj.imag()
                      - sn.real()*qri.real()      + sn.imag()*qri.imag(),
                        cs_conj.real()*qrj.imag() + cs_conj.imag()*qrj.real()
                      - sn.real()*qri.imag()      - sn.imag()*qri.real()
                    );

                    cplx_cordic new_qri(
                        sn_conj.real()*qrj.real() - sn_conj.imag()*qrj.imag()
                      + cs.real()*qri.real()      - cs.imag()*qri.imag(),
                        sn_conj.real()*qrj.imag() + sn_conj.imag()*qrj.real()
                      + cs.real()*qri.imag()      + cs.imag()*qri.real()
                    );

                    Qw[r][j] = new_qrj;
                    Qw[r][i] = new_qri;
                }
            } // end if abs_rij > threshold
        } // end ROW_LOOP
    } // end COL_LOOP

    // ── Cast working arrays back to output types ─────────────────────────────
    CAST_R: for (int i = 0; i < M; i++) {
        CAST_R_COL: for (int j = 0; j < M; j++) {
            #pragma HLS PIPELINE II=1
            R[i][j] = cplx_rmat(t_rmat(Rw[i][j].real()),
                                 t_rmat(Rw[i][j].imag()));
            Q[i][j] = cplx_qmat(t_qmat(Qw[i][j].real()),
                                 t_qmat(Qw[i][j].imag()));
        }
    }
}

// ============================================================================
// FUNCTION 4: BACK-SUBSTITUTION
// ============================================================================
// Purpose: solve Rxx·w = a_s using QR factors of Lt (upper Cholesky factor)
//
// Steps:
//   1. Forward substitution: solve L·z = a_s
//      z[i] = (a_s[i] - sum_{k<i} L[i][k]*z[k]) / L[i][i]
//
//   2. Multiply by Q^H: rhs = Q^H · z
//      rhs[i] = sum_k conj(Q[k][i]) * z[k]
//
//   3. Back substitution: solve R·w = rhs
//      w[i] = (rhs[i] - sum_{k>i} R[i][k]*w[k]) / R[i][i]
// ============================================================================
void back_substitution(cplx_qmat   Q[M][M],
                       cplx_rmat   R[M][M],
                       cplx_chol   L[M][M],
                       cplx_steer  a_s[M],
                       cplx_weight w[M])
{
    #pragma HLS ARRAY_PARTITION variable=Q   cyclic factor=M dim=1
    #pragma HLS ARRAY_PARTITION variable=R   cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=L   cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=a_s complete
    #pragma HLS ARRAY_PARTITION variable=w   complete

    // ── Step 1: Forward substitution  L·z = a_s ─────────────────────────────
    cplx_cordic z[M];
    #pragma HLS ARRAY_PARTITION variable=z complete

    FWD_ROW: for (int i = 0; i < M; i++) {
        cplx_cordic sum(t_cordic(0), t_cordic(0));

        FWD_COL: for (int k = 0; k < i; k++) {
            #pragma HLS PIPELINE II=1
            // complex multiply: L[i][k] * z[k]
            cplx_cordic lik(t_cordic(L[i][k].real()), t_cordic(L[i][k].imag()));
            sum = cplx_cordic(
                sum.real() + lik.real()*z[k].real() - lik.imag()*z[k].imag(),
                sum.imag() + lik.real()*z[k].imag() + lik.imag()*z[k].real()
            );
        }

        // z[i] = (a_s[i] - sum) / L[i][i]
        // Division: multiply by 1/L[i][i] — for hardware use reciprocal LUT
        // Here we use CORDIC-based division: a/b = a * (1/b)
        // 1/L[i][i] computed as: L[i][i] is real (Cholesky diagonal is real)
        t_cordic lii     = t_cordic(L[i][i].real());  // diagonal is real
        t_cordic inv_lii = t_cordic(1.0f) / lii;      // reciprocal (synthesises to divider IP)

        cplx_cordic as_i(t_cordic(a_s[i].real()), t_cordic(a_s[i].imag()));
        z[i] = cplx_cordic(
            (as_i.real() - sum.real()) * inv_lii,
            (as_i.imag() - sum.imag()) * inv_lii
        );
    }

    // ── Step 2: rhs = Q^H · z ───────────────────────────────────────────────
    // Q^H means conjugate transpose: (Q^H)[i][k] = conj(Q[k][i])
    cplx_cordic rhs[M];
    #pragma HLS ARRAY_PARTITION variable=rhs complete

    RHS_ROW: for (int i = 0; i < M; i++) {
        cplx_cordic acc(t_cordic(0), t_cordic(0));

        RHS_COL: for (int k = 0; k < M; k++) {
            #pragma HLS PIPELINE II=1
            // conj(Q[k][i]) * z[k]
            // conj(a+jb) = a-jb
            t_cordic qki_r =  t_cordic(Q[k][i].real());
            t_cordic qki_i = -t_cordic(Q[k][i].imag());  // conjugate

            acc = cplx_cordic(
                acc.real() + qki_r*z[k].real() - qki_i*z[k].imag(),
                acc.imag() + qki_r*z[k].imag() + qki_i*z[k].real()
            );
        }
        rhs[i] = acc;
    }

    // ── Step 3: Back substitution  R·w_unnorm = rhs ──────────────────────────
    // Process from i = M-1 down to 0 (back substitution = reverse order)
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

        // w_unnorm[i] = (rhs[i] - sum) / R[i][i]
        // R[i][i] is complex in general — divide by complex number:
        // (a+jb)/(c+jd) = (ac+bd)/(c²+d²) + j(bc-ad)/(c²+d²)
        t_cordic rii_r = t_cordic(R[i][i].real());
        t_cordic rii_i = t_cordic(R[i][i].imag());
        t_cordic denom = rii_r*rii_r + rii_i*rii_i;
        // denom is real positive — use reciprocal
        t_cordic inv_denom = t_cordic(1.0f) / denom;

        cplx_cordic num(rhs[i].real() - sum.real(),
                        rhs[i].imag() - sum.imag());

        w_unnorm[i] = cplx_cordic(
            (num.real()*rii_r + num.imag()*rii_i) * inv_denom,
            (num.imag()*rii_r - num.real()*rii_i) * inv_denom
        );
    }

    // ── Cast output to weight type ───────────────────────────────────────────
    // NOTE: distortionless normalisation (divide by a_s^H * w) is done
    // in software on the PS ARM — division by a scalar complex number
    // is cheaper there than building a divider in PL.
    OUT_CAST: for (int i = 0; i < M; i++) {
        #pragma HLS PIPELINE II=1
        w[i] = cplx_weight(t_weight(w_unnorm[i].real()),
                            t_weight(w_unnorm[i].imag()));
    }
}

// ============================================================================
// FUNCTION 5: TOP FUNCTION  (this is what Vitis HLS synthesises)
// ============================================================================
// This is the hardware block boundary.
// Everything outside this function = software (PS ARM).
// Everything inside = hardware (PL fabric).
//
// Interface pragmas define how data crosses the PS↔PL boundary:
//   m_axi  : AXI4 master — hardware reads/writes DDR memory directly
//   s_axilite : AXI4-Lite slave — PS writes control registers (start/done)
// ============================================================================
void givens_qr_top(cplx_chol   Lt[M][M],
                   cplx_chol   L[M][M],
                   cplx_steer  a_s[M],
                   cplx_weight w_out[M])
{
    // ── Interface pragmas ────────────────────────────────────────────────────
    // m_axi: hardware DMA from DDR — PS writes Lt, L, a_s to DDR,
    //        hardware reads them, writes w_out back
    #pragma HLS INTERFACE m_axi port=Lt    depth=64  offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=L     depth=64  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=a_s   depth=8   offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=w_out depth=8   offset=slave bundle=gmem3

    // s_axilite: control signals — PS starts hardware and polls done flag
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    // ── Local arrays (on-chip BRAM) ──────────────────────────────────────────
    cplx_chol  Lt_local[M][M];
    cplx_chol  L_local[M][M];
    cplx_steer a_s_local[M];
    cplx_qmat  Q_local[M][M];
    cplx_rmat  R_local[M][M];
    cplx_weight w_local[M];

    #pragma HLS ARRAY_PARTITION variable=Lt_local  cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=L_local   cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=a_s_local complete
    #pragma HLS ARRAY_PARTITION variable=Q_local   cyclic factor=M dim=1
    #pragma HLS ARRAY_PARTITION variable=R_local   cyclic factor=M dim=2
    #pragma HLS ARRAY_PARTITION variable=w_local   complete

    // ── Step 1: Burst read from DDR into on-chip BRAMs ───────────────────────
    // Burst transfer is much more efficient than element-by-element reads
    READ_LT: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            #pragma HLS PIPELINE II=1
            Lt_local[i][j] = Lt[i][j];
        }
    }
    READ_L: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            #pragma HLS PIPELINE II=1
            L_local[i][j] = L[i][j];
        }
    }
    READ_AS: for (int i = 0; i < M; i++) {
        #pragma HLS PIPELINE II=1
        a_s_local[i] = a_s[i];
    }

    // ── Step 2: Givens QR decomposition ──────────────────────────────────────
    givens_qr(Lt_local, Q_local, R_local);

    // ── Step 3: Back-substitution to get weight vector ───────────────────────
    back_substitution(Q_local, R_local, L_local, a_s_local, w_local);

    // ── Step 4: Burst write weight vector back to DDR ────────────────────────
    WRITE_W: for (int i = 0; i < M; i++) {
        #pragma HLS PIPELINE II=1
        w_out[i] = w_local[i];
    }
}
