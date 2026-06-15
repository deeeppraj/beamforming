=== MATLAB Verification of Givens QR HLS Testbench (v2) ===

[INFO] Rxx reconstructed from testbench log.
       Rows/cols 1-4 are exact (4x4 block printed).
       Off-diagonal elements for rows 5-64 are set to 0
       (only diagonal was logged for those rows).

[CHECK] Rxx[0][0] from log   : 4.480155
[CHECK] Rxx Hermitian error  : 0.00e+00  (should be ~0)

[CHECK] Cholesky residual ||L*L^H - Rxx|| / ||Rxx|| : 1.48e-16

--- TEST 3: Diagonal Rxx ---
  w[0] DUT=0.999992  REF=1.000000
  Max error : 8.00e-06  (TOL=5.0e-02)
  => PASS

--- TEST 4: Dense Rxx ---
  Cosine similarity (direction)        : 0.970433  (1=perfect)
  Max error (scale-aligned)            : 0.1144  (TOL=5.0e-02)
  Array gain DUT  a_s^H*w              : 17.0798 -0.0000j
  Array gain REF  a_s^H*w              : 14.2457 +0.0000j
  Gain ratio |DUT|/|REF|               : 1.1989

  [NOTE] REF is approximate for rows 5-64 (only diagonal known).
         Cosine similarity is the most reliable metric here.
  => MARGINAL — likely ref mismatch from unknown off-diagonal elements

--- PHYSICS CHECKS ---
  Im(a_s^H * w_dut) = -0.0000  (should be ~0 for MVDR)
  |w| range: [0.1807, 0.3700]  (expect ~0.1–0.4 for Rxx~4*I)
  Phase track std-dev (w vs a_s): 0.1822 rad  (< pi/4 = 0.7854 expected)
=== Done. Figures 1-3 show visual comparison. ===
>> givens_check
=== MATLAB Verification v3 (full Rxx) ===

[CHECK] Rxx size          : 64x64
[CHECK] Rxx[0][0]         : 4.480155  (expect 4.480155)
[CHECK] Hermitian error   : 0.00e+00  (should be ~0)
[CHECK] Chol residual     : 1.62e-16  (should be ~1e-15)

--- TEST 3: Diagonal Rxx ---
  w[0]  DUT=0.999992  REF=1.000000
  Max error : 8.00e-06  (TOL=5.0e-02)  => PASS

--- TEST 4: Dense Rxx ---
  Max error (raw)              : 0.0005
  Max error (scale-aligned)    : 0.0005  (TOL=5.0e-02)
  Cosine similarity            : 1.000000  (1=perfect)
  Array gain DUT  a_s^H*w      : 17.0798 -0.0000j
  Array gain REF  a_s^H*w      : 17.0768 -0.0000j
  |Im(a_s^H*w_dut)|            : 0.000032  (MVDR distortionless, expect ~0)
  Gain ratio |DUT|/|REF|       : 1.0002

  => PASS — direction correct (cosine > 0.99)

--- PHYSICS CHECKS ---
  |w| range        : [0.1807, 0.3700]  (expect 0.1–0.4 for Rxx~4*I)
  mean(|w|)        : 0.2712  (expect ~1/4.4 = 0.2262)
  Phase-track std  : 0.1822 rad  (< pi/4=0.7854 is good)

  Top-3 worst elements:
    w[ 1]  DUT=(0.2883,+0.0149j)  REF=(0.2878,+0.0148j)  err=0.0005
    w[ 5]  DUT=(0.1971,+0.1321j)  REF=(0.1975,+0.1322j)  err=0.0004
    w[11]  DUT=(0.0960,+0.3025j)  REF=(0.0957,+0.3027j)  err=0.0004
=== Done. Figures 1-4 produced. ===
