// ============================================================================
//  givens_qr.cpp — Givens QR + CORDIC MVDR kernel for Versal VCK190
//  Tool   : Vitis HLS 2024.1
//  Clock  : 300 MHz
//
//  Functions implemented:
//    1. cordic_vectoring  : sqrt(x²+y²) and atan2(y,x) using shift-adds
//    2. cordic_rotation   : cos/sin from angle using shift-adds
//    3. givens_qr         : complex Givens rotation loop → Q, R
//    4. back_substitution : forward sub + Q^H multiply + back sub → weights
//    5. givens_qr_top     : AXI-interfaced top function
//
//  CHANGES FROM ORIGINAL:
//    [FIX-A]  cordic_vectoring/rotation: ATAN_TABLE is now t_cordic type
//             (was float[]); eliminates implicit float→fixed cast per iter.
//    [FIX-B]  givens_qr: replaced hls::sqrt + threshold with squared-magnitude
//             threshold using CORDIC_THRESHOLD_SQ. Avoids multi-cycle sqrt
//             in the Givens loop and removes data-dependent variable latency.
//    [FIX-C]  givens_qr: removed redundant second cordic_vectoring call that
//             computed (mag_a, mag_b) a second time just to get rot_angle.
//             A single call now returns both r_mag and rot_angle.
//    [FIX-D]  givens_qr: FIXED PHASE BUG in sn computation.
//             Original: cordic_rotation(ang_b + π, sn_cos, sn_sin)
//             Correct : cordic_rotation(ang_b,      sn_cos, sn_sin)
//             The π offset was double-negating the imaginary part of sn
//             because sn_conj is separately formed on the next line.
//             With the correct phase, the Givens matrix G applied to rows
//             [j, i] is:
//               G = [ cs       sn   ]   (cs, sn complex)
//                   [-conj(sn) cs*  ]
//             and G^H·A zeros out A[i][j] correctly.
//    [FIX-E]  givens_qr_top: added missing s_axilite interface pragmas for
//             ALL pointer arguments (Lt, L, a_s, w_out).  Without these,
//             offset=slave has no register to store the DDR base addresses
//             written by the PS, causing the kernel to access address 0x0.
//    [FIX-F]  back_substitution: t_cordic(1.0f)/lii and t_cordic(1.0f)/denom
//             are kept (synthesise to Xilinx divider IP at ~15 DSPs each)
//             but are moved outside inner loops where possible.  Comment
//             updated to reflect synthesis cost.
//    [FIX-G]  m_axi depth parameters updated: depth=M*M for 2D arrays,
//             depth=M for 1D arrays.  Was already correct for M=8 (64 and 8)
//             but comment clarified.
// ============================================================================

#include "givens_qr.h"

// ============================================================================
//  FUNCTION 1: CORDIC VECTORING MODE
//  Given (x_in, y_in), compute:
//    magnitude = sqrt(x² + y²)  (gain-compensated)
//    angle     = atan2(y_in, x_in)  in radians
//
//  Algorithm (shift-add, no multipliers in the core loop):
//    Start at (x, y).  Each step rotates toward the x-axis:
//      sigma   = sign(y)             (drives rotation direction)
//      x_new   = x  − sigma * y * 2^{-i}   (shift-and-subtract)
//      y_new   = y  + sigma * x * 2^{-i}   (shift-and-add)
//      z_new   = z  − sigma * atan(2^{-i}) (angle accumulator)
//    After ITER steps:  y → 0,  x → K * magnitude,  z → atan2(y_in, x_in)
//    Compensate gain:  magnitude = x * (1/K)
//
//  Hardware cost:
//    Each iteration: 2 shifts + 4 adds (on t_cordic = ap_fixed<32,4>)
//    No multipliers in the loop — entirely free on DSPs
//    Quadrant pre-rotation:  1 negation + 1 comparison (1 LUT each)
// ============================================================================
void cordic_vectoring(t_cordic  x_in,
                      t_cordic  y_in,
                      t_cordic &magnitude,
                      t_cordic &angle)
{
#pragma HLS INLINE

    t_cordic x = x_in;
    t_cordic y = y_in;
    t_cordic z = 0;

    // Quadrant normalisation: CORDIC vectoring converges only for x > 0.
    // Rotate to first/fourth quadrant by negating both components if x < 0.
    // Adjust angle accumulator to account for the 180° rotation.
    if (x_in < 0) {
        x = -x_in;
        y = -y_in;
        z = (y_in >= 0) ? t_cordic(3.14159265) : t_cordic(-3.14159265);
    }

    // Main CORDIC loop — loop-carried dependency on x, y, z prevents II=1.
    // HLS achieves II=3 or II=4 automatically; this is expected and correct.
    // Do NOT add #pragma HLS UNROLL here — it creates M parallel CORDIC units
    // and explodes resource usage for no benefit inside a sequential loop.
    CORDIC_VEC_LOOP: for (int i = 0; i < ITER; i++) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER

        t_cordic sigma   = (y >= 0) ? t_cordic(-1) : t_cordic(1);
        t_cordic x_shift = x >> i;    // x * 2^{-i}  — bit shift, free
        t_cordic y_shift = y >> i;    // y * 2^{-i}  — bit shift, free

        t_cordic x_new = x - sigma * y_shift;
        t_cordic y_new = y + sigma * x_shift;
        t_cordic z_new = z - sigma * ATAN_TABLE[i];   // [FIX-A]: t_cordic table

        x = x_new;
        y = y_new;
        z = z_new;
    }

    // Compensate CORDIC gain: x holds K*magnitude; multiply by 1/K.
    // CORDIC_K_INV = 0.6073 is a compile-time constant → 1 DSP multiply.
    magnitude = x * CORDIC_K_INV;
    angle     = z;
}

// ============================================================================
//  FUNCTION 2: CORDIC ROTATION MODE
//  Given angle θ, compute cos(θ) and sin(θ).
//
//  Algorithm:
//    Start at (1/K, 0) — pre-compensated unit vector.
//    Each step accumulates rotation toward θ:
//      sigma   = sign(z)             (drives rotation direction)
//      x_new   = x − sigma * y * 2^{-i}
//      y_new   = y + sigma * x * 2^{-i}
//      z_new   = z − sigma * atan(2^{-i})
//    After ITER steps:  z → 0,  x → cos(θ),  y → sin(θ)
//    (gain compensation already built in via initial x = 1/K)
// ============================================================================
void cordic_rotation(t_cordic  angle,
                     t_cordic &cos_out,
                     t_cordic &sin_out)
{
#pragma HLS INLINE

    t_cordic x = CORDIC_K_INV;   // pre-compensated: 1/K ≈ 0.6073
    t_cordic y = 0;
    t_cordic z = angle;

    CORDIC_ROT_LOOP: for (int i = 0; i < ITER; i++) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=ITER max=ITER avg=ITER

        t_cordic sigma   = (z >= 0) ? t_cordic(1) : t_cordic(-1);
        t_cordic x_shift = x >> i;
        t_cordic y_shift = y >> i;

        t_cordic x_new = x - sigma * y_shift;
        t_cordic y_new = y + sigma * x_shift;
        t_cordic z_new = z - sigma * ATAN_TABLE[i];   // [FIX-A]

        x = x_new;
        y = y_new;
        z = z_new;
    }

    cos_out = x;
    sin_out = y;
}

// ============================================================================
//  FUNCTION 3: GIVENS QR DECOMPOSITION
//
//  Input : A_in[M][M]  (complex, typically the upper Cholesky factor Lt)
//  Output: Q[M][M]     (unitary)
//          R[M][M]     (upper triangular)
//  Such that A_in = Q * R.
//
//  For each column j (left to right), each sub-diagonal element A[i][j]
//  (i = M-1 down to j+1) is zeroed by a Givens rotation G_{i,j}.
//  The rotation is computed from magnitudes via CORDIC vectoring/rotation.
//  The unitary matrix Q accumulates the conjugate-transposed rotations: Q = G^H.
//
//  Complex Givens rotation for element (i,j):
//    a = R[j][j],  b = R[i][j]
//    cs = conj(a) / r,   sn = conj(b) / r,   r = sqrt(|a|²+|b|²)
//    (following the convention in Golub & Van Loan §5.1.9 for complex matrices)
//
//    G applied to rows [j, i]:
//      [ R[j][:] ]  ←  [  cs    sn ] [ R[j][:] ]
//      [ R[i][:] ]     [ -sn*  cs* ] [ R[i][:] ]
//    After application: R[i][j] = 0 (by construction of cs, sn).
//
//  Phase derivation (complex cs, sn):
//    |cs| = |a|/r,   phase(cs) = -phase(a)   (so cs*a = |a|²/r = real positive)
//    |sn| = |b|/r,   phase(sn) = -phase(b)   (so sn*b = |b|²/r = real positive)
//
//    [FIX-D] The original code used phase(sn) = phase(b) + π, which is
//    equivalent to -phase(b) + (π - π) — wait, no: ang_b + π ≠ -ang_b.
//    Example: if ang_b = π/4, then:
//      correct: phase(sn) = -π/4  → sn = (|b|/r) * e^{-jπ/4}
//      buggy:   ang_b + π = 5π/4 → sn = (|b|/r) * e^{j5π/4} = -(|b|/r)*e^{jπ/4}
//    The bug negated |sn| in the complex plane, making the rotation wrong.
//    Fix: use -ang_b as the rotation angle for sn.
//
//  HLS pragmas:
//    ARRAY_PARTITION cyclic on R and Q (column dim) → parallel column access
//    in the RUPDATE and QUPDATE inner loops (target II=1).
// ============================================================================
void givens_qr(cplx_chol A_in[M][M],
               cplx_qmat Q[M][M],
               cplx_rmat R[M][M])
{
#pragma HLS ARRAY_PARTITION variable=R    cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Q    cyclic factor=M dim=1
#pragma HLS ARRAY_PARTITION variable=A_in cyclic factor=M dim=2

    // Working copies at CORDIC precision to avoid precision loss during rotation
    cplx_cordic Rw[M][M];
    cplx_cordic Qw[M][M];

#pragma HLS ARRAY_PARTITION variable=Rw cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=Qw cyclic factor=M dim=1

    // ── Initialise: Rw ← A_in,  Qw ← Identity ──────────────────────────────
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
        // Sequential: column j depends on the result of all rotations in
        // columns 0..j-1.  Do NOT pipeline or unroll this level.

        ROW_LOOP: for (int i = M-1; i > j; i--) {
            // Sequential: each row-i rotation in column j changes Rw[j][j]
            // which is the pivot for the next rotation.

            t_cordic b_real = Rw[i][j].real();
            t_cordic b_imag = Rw[i][j].imag();

            // [FIX-B] Squared-magnitude threshold — avoids hls::sqrt.
            // hls::sqrt is multi-cycle and creates variable latency inside
            // the sequential ROW_LOOP, which defeats the fixed-latency model
            // that co-simulation requires.
            t_cordic abs_sq = b_real*b_real + b_imag*b_imag;

            if (abs_sq > CORDIC_THRESHOLD_SQ) {

                t_cordic a_real = Rw[j][j].real();
                t_cordic a_imag = Rw[j][j].imag();

                // ── Compute |a|, phase(a), |b|, phase(b) via CORDIC ─────────
                t_cordic mag_a, ang_a;
                t_cordic mag_b, ang_b;
                cordic_vectoring(a_real, a_imag, mag_a, ang_a);
                cordic_vectoring(b_real, b_imag, mag_b, ang_b);

                // ── r = sqrt(mag_a² + mag_b²);  rot_angle = atan2(mag_b, mag_a)
                // [FIX-C] Single CORDIC call returns both r_mag and rot_angle.
                //          Original had TWO calls with (mag_a, mag_b) — one for
                //          r_mag (discarded) and one for rot_angle.  Fixed.
                t_cordic r_mag, rot_angle;
                cordic_vectoring(mag_a, mag_b, r_mag, rot_angle);

                // ── cs_mag = |a|/r = cos(rot_angle)
                //    sn_mag = |b|/r = sin(rot_angle)
                t_cordic cs_mag, sn_mag;
                cordic_rotation(rot_angle, cs_mag, sn_mag);

                // ── Phase of cs: phase(cs) = -ang_a
                //    Phase of sn: phase(sn) = -ang_b
                //
                //    [FIX-D] Original used ang_b + π for sn phase, which is
                //    WRONG — it placed sn in the opposite complex half-plane,
                //    making the rotation non-unitary.
                //    Correct: phase(sn) = -ang_b so that conj(a)*sn + conj(b)*cs
                //    produces the correct zero at R[i][j] after the rotation.
                t_cordic cs_cos, cs_sin;   // cos/sin of phase(cs) = -ang_a
                t_cordic sn_cos, sn_sin;   // cos/sin of phase(sn) = -ang_b

                cordic_rotation(-ang_a, cs_cos, cs_sin);   // phase = -ang_a
                cordic_rotation(-ang_b, sn_cos, sn_sin);   // phase = -ang_b  [FIX-D]

                // cs = cs_mag * (cos(-ang_a) + j*sin(-ang_a))
                cplx_cordic cs(cs_mag * cs_cos, cs_mag * cs_sin);
                // sn = sn_mag * (cos(-ang_b) + j*sin(-ang_b))
                cplx_cordic sn(sn_mag * sn_cos, sn_mag * sn_sin);

                // Precompute conjugates (used in both row and column updates)
                cplx_cordic cs_conj( cs.real(), -cs.imag());
                cplx_cordic sn_conj( sn.real(), -sn.imag());

                // ── Apply rotation to rows j and i of Rw ────────────────────
                // Givens matrix applied on the left:
                //   [ Rw[j][:] ]  ←  [  cs    sn ] [ Rw[j][:] ]
                //   [ Rw[i][:] ]     [ -sn*  cs* ] [ Rw[i][:] ]
                //
                // For each column k:
                //   new_rjk =  cs * Rw[j][k]  +  sn * Rw[i][k]
                //   new_rik = -sn* * Rw[j][k] + cs* * Rw[i][k]
                RUPDATE: for (int k = 0; k < M; k++) {
#pragma HLS PIPELINE II=1
                    cplx_cordic rjk = Rw[j][k];
                    cplx_cordic rik = Rw[i][k];

                    // cs * rjk  (complex multiply: (a+jb)(c+jd)=ac-bd+j(ad+bc))
                    Rw[j][k] = cplx_cordic(
                        cs.real()*rjk.real() - cs.imag()*rjk.imag()
                      + sn.real()*rik.real() - sn.imag()*rik.imag(),
                        cs.real()*rjk.imag() + cs.imag()*rjk.real()
                      + sn.real()*rik.imag() + sn.imag()*rik.real()
                    );

                    // -sn_conj * rjk  +  cs_conj * rik
                    Rw[i][k] = cplx_cordic(
                       -sn_conj.real()*rjk.real() + sn_conj.imag()*rjk.imag()
                      + cs_conj.real()*rik.real() - cs_conj.imag()*rik.imag(),
                       -sn_conj.real()*rjk.imag() - sn_conj.imag()*rjk.real()
                      + cs_conj.real()*rik.imag() + cs_conj.imag()*rik.real()
                    );
                } // RUPDATE

                // ── Accumulate Q: update columns j and i of Qw ──────────────
                // Q = Q * G^H  where G = Givens matrix from above.
                // G^H applied on the right to columns [j, i] of Q:
                //   [ Qw[:,j]  Qw[:,i] ] ← [ Qw[:,j]  Qw[:,i] ] * [ cs*  -sn ]
                //                                                    [ sn*   cs ]
                //
                // For each row r:
                //   new_qrj =  cs* * Qw[r][j] + sn* * Qw[r][i]
                //   new_qri = -sn  * Qw[r][j] + cs  * Qw[r][i]
                QUPDATE: for (int r = 0; r < M; r++) {
#pragma HLS PIPELINE II=1
                    cplx_cordic qrj = Qw[r][j];
                    cplx_cordic qri = Qw[r][i];

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
                } // QUPDATE

            } // if abs_sq > threshold

        } // ROW_LOOP
    } // COL_LOOP

    // ── Cast working arrays to output types ──────────────────────────────────
    CAST_OUT: for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE II=1
            R[i][j] = cplx_rmat(t_rmat(Rw[i][j].real()),
                                  t_rmat(Rw[i][j].imag()));
            Q[i][j] = cplx_qmat(t_qmat(Qw[i][j].real()),
                                  t_qmat(Qw[i][j].imag()));
        }
    }
}

// ============================================================================
//  FUNCTION 4: BACK-SUBSTITUTION
//
//  Solves  Rxx · w = a_s  exploiting the QR decomposition of Lt:
//    Lt = Q · R   →   Rxx = L · Lt = L · Q · R
//
//  So  Rxx · w = a_s  becomes  L·Q·R·w = a_s.  Solve in three steps:
//
//  Step 1 — Forward substitution:  L · z = a_s
//    z[i] = (a_s[i] - Σ_{k<i} L[i][k]*z[k]) / L[i][i]
//    L[i][i] is real and positive (Cholesky diagonal guarantee).
//
//  Step 2 — Multiply by Q^H:  rhs = Q^H · z
//    rhs[i] = Σ_k conj(Q[k][i]) * z[k]
//    (Q^H)_{ij} = conj(Q_{ji})
//
//  Step 3 — Back substitution:  R · w = rhs
//    w[i] = (rhs[i] - Σ_{k>i} R[i][k]*w[k]) / R[i][i]
//    R[i][i] is complex in general.
//
//  [FIX-F] Division in steps 1 and 3 synthesises to Xilinx divider IP
//  (~15 DSPs, ~30 cycle latency each).  The divisions are in sequential
//  loops (cannot be pipelined) so each incurs full latency.
//  For M=8: 8 + 8 = 16 divisions total.  Acceptable.
//  For M=64: 128 divisions — consider a Newton-Raphson reciprocal LUT.
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
            cplx_cordic lik(t_cordic(L[i][k].real()), t_cordic(L[i][k].imag()));
            // sum += L[i][k] * z[k]   (complex multiply)
            sum = cplx_cordic(
                sum.real() + lik.real()*z[k].real() - lik.imag()*z[k].imag(),
                sum.imag() + lik.real()*z[k].imag() + lik.imag()*z[k].real()
            );
        }

        // z[i] = (a_s[i] - sum) / L[i][i]
        // Cholesky diagonal is real and positive → scalar real division.
        // [FIX-F] Synthesises to Xilinx divider IP.
        t_cordic lii     = t_cordic(L[i][i].real());
        t_cordic inv_lii = t_cordic(1.0) / lii;

        cplx_cordic as_i(t_cordic(a_s[i].real()), t_cordic(a_s[i].imag()));
        z[i] = cplx_cordic(
            (as_i.real() - sum.real()) * inv_lii,
            (as_i.imag() - sum.imag()) * inv_lii
        );
    }

    // ── Step 2: rhs = Q^H · z ───────────────────────────────────────────────
    // (Q^H)[i][k] = conj(Q[k][i])
    cplx_cordic rhs[M];
#pragma HLS ARRAY_PARTITION variable=rhs complete

    RHS_ROW: for (int i = 0; i < M; i++) {
        cplx_cordic acc(t_cordic(0), t_cordic(0));

        RHS_COL: for (int k = 0; k < M; k++) {
#pragma HLS PIPELINE II=1
            // conj(Q[k][i]) = (Q[k][i].re, -Q[k][i].im)
            t_cordic qki_r =  t_cordic(Q[k][i].real());
            t_cordic qki_i = -t_cordic(Q[k][i].imag());   // conjugate

            // acc += conj(Q[k][i]) * z[k]
            acc = cplx_cordic(
                acc.real() + qki_r*z[k].real() - qki_i*z[k].imag(),
                acc.imag() + qki_r*z[k].imag() + qki_i*z[k].real()
            );
        }
        rhs[i] = acc;
    }

    // ── Step 3: Back substitution  R·w = rhs ────────────────────────────────
    // Process i = M-1 down to 0 (reverse order required for back substitution).
    cplx_cordic w_unnorm[M];
#pragma HLS ARRAY_PARTITION variable=w_unnorm complete

    BCK_ROW: for (int i = M-1; i >= 0; i--) {
        cplx_cordic sum(t_cordic(0), t_cordic(0));

        BCK_COL: for (int k = i+1; k < M; k++) {
#pragma HLS PIPELINE II=1
            cplx_cordic rik(t_cordic(R[i][k].real()), t_cordic(R[i][k].imag()));
            // sum += R[i][k] * w_unnorm[k]
            sum = cplx_cordic(
                sum.real() + rik.real()*w_unnorm[k].real() - rik.imag()*w_unnorm[k].imag(),
                sum.imag() + rik.real()*w_unnorm[k].imag() + rik.imag()*w_unnorm[k].real()
            );
        }

        // w_unnorm[i] = (rhs[i] - sum) / R[i][i]
        // R[i][i] is complex: divide (a+jb)/(c+jd) = ((ac+bd) + j(bc-ad)) / (c²+d²)
        t_cordic rii_r   = t_cordic(R[i][i].real());
        t_cordic rii_i   = t_cordic(R[i][i].imag());
        t_cordic denom   = rii_r*rii_r + rii_i*rii_i;
        t_cordic inv_den = t_cordic(1.0) / denom;   // [FIX-F]

        cplx_cordic num(rhs[i].real() - sum.real(),
                        rhs[i].imag() - sum.imag());

        w_unnorm[i] = cplx_cordic(
            (num.real()*rii_r + num.imag()*rii_i) * inv_den,
            (num.imag()*rii_r - num.real()*rii_i) * inv_den
        );
    }

    // ── Cast output to weight type ───────────────────────────────────────────
    // Distortionless normalisation (divide by a_s^H * w) is done in PS software
    // — complex scalar division is cheaper on the ARM than in PL.
    OUT_CAST: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=1
        w[i] = cplx_weight(t_weight(w_unnorm[i].real()),
                            t_weight(w_unnorm[i].imag()));
    }
}

// ============================================================================
//  FUNCTION 5: TOP FUNCTION
//
//  This is the hardware block boundary for Vitis HLS synthesis.
//  Everything inside = PL fabric.  Everything outside = PS ARM.
//
//  Interface:
//    m_axi       : AXI4 master — hardware DMA to/from DDR
//    s_axilite   : AXI4-Lite slave — PS writes base addresses + asserts ap_start
//
//  [FIX-E] ALL pointer arguments now have BOTH m_axi AND s_axilite pragmas.
//  The m_axi pragma with offset=slave means: "the DDR base address for this
//  port is stored in an AXI-Lite register".  Without the matching s_axilite
//  pragma for each pointer, HLS has no register to write the address into,
//  and the kernel defaults to accessing DDR at address 0x0 — silent corruption.
//
//  [FIX-G] depth=M*M for 2D arrays, depth=M for 1D arrays (M=8 → 64 and 8).
//  These only affect co-simulation memory allocation; synthesis ignores depth.
// ============================================================================
void givens_qr_top(cplx_chol   Lt[M][M],
                   cplx_chol   L[M][M],
                   cplx_steer  a_s[M],
                   cplx_weight w_out[M])
{
    // ── m_axi: data path (DDR ↔ on-chip BRAM) ────────────────────────────────
    // Four separate bundles → four independent AXI4 channels on the NoC.
    // Depth = total element count accessed (M*M for matrices, M for vectors).
#pragma HLS INTERFACE m_axi port=Lt    depth=64  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=L     depth=64  offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=a_s   depth=8   offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi port=w_out depth=8   offset=slave bundle=gmem3

    // ── s_axilite: control registers ─────────────────────────────────────────
    // [FIX-E] ALL pointer arguments must appear here.
    // HLS generates a register map:
    //   0x00 : control (ap_start, ap_done, ap_idle, ap_ready)
    //   0x04 : global interrupt enable
    //   0x08 : ip interrupt enable / status
    //   0x10 : Lt  base address (low  32 bits)
    //   0x14 : Lt  base address (high 32 bits)
    //   0x1c : L   base address (low  32 bits)
    //   0x20 : L   base address (high 32 bits)
    //   0x28 : a_s base address (low  32 bits)
    //   ...  etc.
    // PS XRT driver writes these before asserting bit[0] of 0x00 (ap_start).
#pragma HLS INTERFACE s_axilite port=Lt    bundle=control
#pragma HLS INTERFACE s_axilite port=L     bundle=control
#pragma HLS INTERFACE s_axilite port=a_s   bundle=control
#pragma HLS INTERFACE s_axilite port=w_out bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // ── On-chip local arrays (BRAM) ───────────────────────────────────────────
    cplx_chol   Lt_local[M][M];
    cplx_chol   L_local[M][M];
    cplx_steer  a_s_local[M];
    cplx_qmat   Q_local[M][M];
    cplx_rmat   R_local[M][M];
    cplx_weight w_local[M];

#pragma HLS ARRAY_PARTITION variable=Lt_local  cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=L_local   cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=a_s_local complete
#pragma HLS ARRAY_PARTITION variable=Q_local   cyclic factor=M dim=1
#pragma HLS ARRAY_PARTITION variable=R_local   cyclic factor=M dim=2
#pragma HLS ARRAY_PARTITION variable=w_local   complete

    // ── Phase 1: Burst read DDR → on-chip ────────────────────────────────────
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

    // ── Phase 2: Givens QR of Lt ──────────────────────────────────────────────
    givens_qr(Lt_local, Q_local, R_local);

    // ── Phase 3: Back-substitution → weight vector ───────────────────────────
    back_substitution(Q_local, R_local, L_local, a_s_local, w_local);

    // ── Phase 4: Burst write on-chip → DDR ───────────────────────────────────
    WRITE_W: for (int i = 0; i < M; i++) {
#pragma HLS PIPELINE II=1
        w_out[i] = w_local[i];
    }
}
