# CORDIC Algorithm: Complete Mathematical Reference
## Application to Givens QR Decomposition and Matrix Inverse for MVDR Beamforming

**Platform:** Xilinx Versal VCK190 | **Tool:** Vitis HLS 2024.1 | **Target Clock:** 300 MHz

---

## Table of Contents

1. [What is CORDIC?](#1-what-is-cordic)
2. [The Core Mathematical Idea](#2-the-core-mathematical-idea)
3. [CORDIC Vectoring Mode — Computing Magnitude and Angle](#3-cordic-vectoring-mode)
4. [CORDIC Rotation Mode — Computing Cosine and Sine](#4-cordic-rotation-mode)
5. [CORDIC Gain and Compensation](#5-cordic-gain-and-compensation)
6. [The atan Lookup Table — Derivation](#6-the-atan-lookup-table)
7. [Convergence and Error Analysis](#7-convergence-and-error-analysis)
8. [How CORDIC Replaces Division and Square Root](#8-how-cordic-replaces-division-and-square-root)
9. [Applying CORDIC to Givens Rotation](#9-applying-cordic-to-givens-rotation)
10. [Full Givens QR Decomposition](#10-full-givens-qr-decomposition)
11. [Computing the Inverse via QR — The Mathematical Path](#11-computing-the-inverse-via-qr)
12. [Forward and Back Substitution](#12-forward-and-back-substitution)
13. [End-to-End MVDR Pipeline](#13-end-to-end-mvdr-pipeline)
14. [Fixed-Point Precision Analysis](#14-fixed-point-precision-analysis)
15. [Hardware Architecture Summary](#15-hardware-architecture-summary)

---

## 1. What is CORDIC?

**CORDIC** stands for **CO**ordinate **R**otation **DI**gital **C**omputer. It was invented by Jack Volder in 1959 for real-time navigation computation on aircraft where no hardware multiplier existed.

The fundamental insight is elegant: **any trigonometric, hyperbolic, or square-root computation can be reduced to a sequence of shift-and-add operations** by iteratively rotating a vector through pre-computed angles that are powers of two.

Why this matters for FPGAs:

| Operation | Standard Hardware Cost | CORDIC Cost |
|-----------|----------------------|-------------|
| `sqrt(x)` | ~20 cycles, 8 DSP slices | 16 cycles, **0 DSP slices** |
| `a / b` | ~16 cycles, 6 DSP slices | 16 cycles, **0 DSP slices** |
| `atan2(y,x)` | ~30 cycles, LUT-heavy | 16 cycles, **0 DSP slices** |
| `cos(θ), sin(θ)` | LUT + interpolation | 16 cycles, **0 DSP slices** |

CORDIC replaces all of these with **bit shifts** (free on hardware — just rewire the bus) and **additions** (cheap — one LUT per bit).

---

## 2. The Core Mathematical Idea

Start with a 2D vector **v** = (x, y). A rotation by angle θ transforms it to:

```
x' = x·cos(θ) − y·sin(θ)
y' = y·cos(θ) + x·sin(θ)
```

Factor out cos(θ):

```
x' = cos(θ) · [x − y·tan(θ)]
y' = cos(θ) · [y + x·tan(θ)]
```

Now the key trick: **choose θ such that tan(θ) = ±2⁻ⁱ**.

When `tan(θᵢ) = 2⁻ⁱ`, multiplication by `tan(θᵢ)` becomes a **right bit-shift by i positions** — which costs zero hardware.

Define the sequence of micro-rotation angles:

```
θᵢ = arctan(2⁻ⁱ)   for i = 0, 1, 2, ..., N-1
```

At each step i, we choose to rotate **clockwise or counter-clockwise** based on a direction variable σᵢ ∈ {+1, −1}.

The CORDIC micro-rotation at step i:

```
x_{i+1} = x_i − σᵢ · y_i · 2⁻ⁱ
y_{i+1} = y_i + σᵢ · x_i · 2⁻ⁱ
z_{i+1} = z_i − σᵢ · arctan(2⁻ⁱ)
```

where z accumulates the total angle rotated.

This is the **unified CORDIC kernel**. The two modes differ only in how σᵢ is chosen.

---

## 3. CORDIC Vectoring Mode

### Purpose
Given inputs (x₀, y₀), compute:
- `r = √(x₀² + y₀²)` — the magnitude (Euclidean norm)
- `φ = arctan(y₀ / x₀)` — the phase angle

### How σᵢ is Chosen
In vectoring mode, the goal is to **drive y toward zero** — rotating the vector onto the x-axis. At each step:

```
σᵢ = −sign(yᵢ)
```

If y > 0, rotate clockwise (σ = −1) to reduce y.  
If y < 0, rotate counter-clockwise (σ = +1) to increase y toward zero.

### The Full Iteration

```
x₀, y₀  — input vector components
z₀ = 0   — angle accumulator starts at zero

For i = 0, 1, ..., N−1:
    σᵢ    = −sign(yᵢ)
    x_{i+1} = xᵢ − σᵢ · yᵢ · 2⁻ⁱ
    y_{i+1} = yᵢ + σᵢ · xᵢ · 2⁻ⁱ
    z_{i+1} = zᵢ − σᵢ · arctan(2⁻ⁱ)
```

### What Converges

After N iterations:

```
y_N  → 0                         (y is driven to zero)
x_N  → K · √(x₀² + y₀²)         (x holds the magnitude, scaled by CORDIC gain K)
z_N  → arctan(y₀ / x₀)           (z holds the angle)
```

where K is the **CORDIC gain** (explained in Section 5).

### Worked Example (N = 4 iterations)

Input: (x₀, y₀) = (3, 4), expected result: r = 5, φ = arctan(4/3) = 0.9273 rad

```
i=0: σ = −sign(4) = −1
     x₁ = 3 − (−1)·4·2⁰  = 3 + 4 = 7
     y₁ = 4 + (−1)·3·2⁰  = 4 − 3 = 1
     z₁ = 0 − (−1)·arctan(1) = 0.7854

i=1: σ = −sign(1) = −1
     x₂ = 7 − (−1)·1·2⁻¹ = 7 + 0.5 = 7.5
     y₂ = 1 + (−1)·7·2⁻¹ = 1 − 3.5 = −2.5
     z₂ = 0.7854 − (−1)·arctan(0.5) = 0.7854 + 0.4636 = 1.2490

i=2: σ = −sign(−2.5) = +1
     x₃ = 7.5 − (+1)·(−2.5)·2⁻² = 7.5 + 0.625 = 8.125
     y₃ = −2.5 + (+1)·7.5·2⁻²   = −2.5 + 1.875 = −0.625
     z₃ = 1.2490 − (+1)·arctan(0.25) = 1.2490 − 0.2450 = 1.0040

i=3: σ = −sign(−0.625) = +1
     x₄ = 8.125 + 0.625·2⁻³ = 8.125 + 0.0781 = 8.203
     y₄ = −0.625 + 8.125·2⁻³ = −0.625 + 1.016 = 0.391
     z₄ = 1.0040 − arctan(0.125) = 1.0040 − 0.1244 = 0.8796
```

After 4 iterations: x₄ ≈ 8.203 ≈ K·5 = 1.6468·5 = 8.234 ✓ (converging)  
z₄ ≈ 0.8796 → converging to 0.9273 ✓

With N=16 iterations the error drops below 10⁻⁵.

---

## 4. CORDIC Rotation Mode

### Purpose
Given an angle θ, compute:
- `cos(θ)` 
- `sin(θ)`

### How σᵢ is Chosen
In rotation mode, the goal is to **drive z toward zero** — consuming the target angle through micro-rotations. At each step:

```
σᵢ = sign(zᵢ)
```

If z > 0, rotate counter-clockwise (σ = +1) to reduce remaining angle.  
If z < 0, rotate clockwise (σ = −1) to increase it toward zero.

### The Full Iteration

```
x₀ = K_inv = 1/K ≈ 0.6073   (pre-compensated unit vector — see Section 5)
y₀ = 0
z₀ = θ                       (target angle to rotate by)

For i = 0, 1, ..., N−1:
    σᵢ    = sign(zᵢ)
    x_{i+1} = xᵢ − σᵢ · yᵢ · 2⁻ⁱ
    y_{i+1} = yᵢ + σᵢ · xᵢ · 2⁻ⁱ
    z_{i+1} = zᵢ − σᵢ · arctan(2⁻ⁱ)
```

### What Converges

```
z_N  → 0
x_N  → K · (1/K) · cos(θ) = cos(θ)     (gain compensated by initial value)
y_N  → K · (1/K) · sin(θ) = sin(θ)
```

Starting with x₀ = 1/K automatically compensates for the CORDIC gain — no extra multiplication needed.

---

## 5. CORDIC Gain and Compensation

### Where the Gain Comes From

Each micro-rotation at step i multiplies the vector magnitude by:

```
|rotation factor| = √(1 + tan²(θᵢ)) = √(1 + 2⁻²ⁱ)
```

After N iterations, the total accumulated gain is:

```
K = ∏ᵢ₌₀ᴺ⁻¹ √(1 + 2⁻²ⁱ)
```

For N = 16:

```
K = √(1+1) · √(1+0.25) · √(1+0.0625) · √(1+0.015625) · ...
  = 1.4142 · 1.1180 · 1.0308 · 1.0078 · 1.0020 · ...
  ≈ 1.6468
```

This gain is **constant for a fixed N** — it never changes regardless of input. So it can be pre-computed once and stored as a compile-time constant.

### Two Ways to Compensate

**Method 1 — Post-multiply** (vectoring mode):  
After computing x_N (which equals K·r), multiply by 1/K:
```
r = x_N × (1/K) = x_N × 0.6073
```
One multiplication — acceptable since it's done once per rotation.

**Method 2 — Pre-scale** (rotation mode):  
Start with x₀ = 1/K instead of x₀ = 1.  
Then x_N = K · (1/K) · cos(θ) = cos(θ) exactly.  
**Zero extra operations.**

In our HLS implementation:
- Vectoring mode uses Method 1 (one multiply by `CORDIC_K_INV` at the end)
- Rotation mode uses Method 2 (x₀ initialised to `CORDIC_K_INV`)

---

## 6. The atan Lookup Table — Derivation

The CORDIC algorithm requires the values `arctan(2⁻ⁱ)` for i = 0, 1, ..., N−1.

These are computed once offline and stored as a ROM on the FPGA:

```
θ₀ = arctan(2⁰)   = arctan(1.0000) = 0.78539816 rad  (45°)
θ₁ = arctan(2⁻¹)  = arctan(0.5000) = 0.46364761 rad  (26.565°)
θ₂ = arctan(2⁻²)  = arctan(0.2500) = 0.24497866 rad  (14.036°)
θ₃ = arctan(2⁻³)  = arctan(0.1250) = 0.12435499 rad  (7.125°)
θ₄ = arctan(2⁻⁴)  = arctan(0.0625) = 0.06241881 rad  (3.576°)
θ₅ = arctan(2⁻⁵)  = arctan(0.0313) = 0.03123983 rad  (1.790°)
θ₆ = arctan(2⁻⁶)  = arctan(0.0156) = 0.01562373 rad  (0.895°)
θ₇ = arctan(2⁻⁷)  = arctan(0.0078) = 0.00781238 rad  (0.448°)
...
θ₁₅ = arctan(2⁻¹⁵) = 0.00003052 rad  (0.00175°)
```

### Important Property

The sum of all angles determines the **convergence range** of CORDIC:

```
Σᵢ₌₀ᴺ⁻¹ θᵢ > θ₀ = arctan(1) = π/4
```

This means CORDIC can rotate by any angle in the range **[−π/2, +π/2]**.

For angles outside this range (i.e., second or third quadrant), we pre-rotate by ±π before starting:

```c
// Quadrant correction (in givens_qr.cpp):
if (x_in < 0) {
    x = -x_in;
    y = -y_in;
    z = (y_in >= 0) ? π : −π;   // pre-add ±π to angle accumulator
}
```

This maps any input into the first/fourth quadrant before CORDIC starts.

---

## 7. Convergence and Error Analysis

### Convergence Rate

The angular residual after N iterations is bounded by:

```
|z_N| ≤ θ_{N-1} = arctan(2⁻⁽ᴺ⁻¹⁾) ≈ 2⁻⁽ᴺ⁻¹⁾  radians
```

The magnitude error is:

```
|x_N − K·r| ≤ K · r · |z_N|²/2 ≈ K · r · 2⁻²ᴺ / 2
```

For N = 16 iterations:

```
Angular error   ≤ 2⁻¹⁵ ≈ 3.05 × 10⁻⁵ radians
Magnitude error ≈ K · r · 4.7 × 10⁻¹⁰   (negligible)
```

### Comparison with Fixed-Point Precision

Our fixed-point type `t_cordic = ap_fixed<32,4>` has resolution 2⁻²⁸ ≈ 3.7×10⁻⁹.

The CORDIC angular error (3×10⁻⁵) is much larger than the fixed-point quantisation error (3.7×10⁻⁹), so **CORDIC iteration count is the dominant error source** — adding more iterations gives more accuracy, not more bits.

For our MVDR application, N=16 gives angular error of 3×10⁻⁵ radians, which produces null depth degradation well within our 3 dB budget (confirmed by MATLAB fixed-point simulation: 0.56 dB degradation).

---

## 8. How CORDIC Replaces Division and Square Root

This is the most practically important section for understanding why CORDIC matters for Givens rotations.

### The Givens Rotation Problem

Given two complex numbers a and b (entries in a matrix column), the Givens rotation requires:

```
r  = √(|a|² + |b|²)      ← square root
cs = a / r                ← division
sn = −b / r               ← division
```

On a CPU, `sqrt` and `/` are single instructions. On an FPGA, they each require a multi-cycle iterative circuit costing many DSP slices.

### CORDIC Solution — Step by Step

**For real-valued a and b:**

**Step 1** — Use CORDIC vectoring on (|a|, |b|) to get r and angle simultaneously:

```
Input to vectoring:  x₀ = |a|,  y₀ = |b|

After N iterations:
    x_N = K · √(|a|² + |b|²) = K · r
    z_N = arctan(|b| / |a|) = φ     (the Givens angle)

Compensate: r_actual = x_N × (1/K)
```

**Step 2** — Use CORDIC rotation on angle φ to get cs and sn:

```
Input to rotation: x₀ = 1/K,  y₀ = 0,  z₀ = φ

After N iterations:
    x_N = cos(φ) = |a| / r = cs
    y_N = sin(φ) = |b| / r = sn
```

**No division was performed. No square root was computed.**  
Both came from shift-add iterations on pre-computed constants.

**For complex-valued a and b:**

Complex entries require additional steps. For a = aᵣ + jaᵢ and b = bᵣ + jbᵢ:

```
Step 1: mag_a = |a| = √(aᵣ² + aᵢ²)   via CORDIC vectoring on (aᵣ, aᵢ)
        ang_a = ∠a = arctan(aᵢ / aᵣ)  (from same vectoring call, in z register)

Step 2: mag_b = |b| = √(bᵣ² + bᵢ²)   via CORDIC vectoring on (bᵣ, bᵢ)
        ang_b = ∠b = arctan(bᵢ / bᵣ)

Step 3: r = √(mag_a² + mag_b²)         via CORDIC vectoring on (mag_a, mag_b)
        rot_angle = arctan(mag_b / mag_a)

Step 4: cs_mag = cos(rot_angle)         via CORDIC rotation
        sn_mag = sin(rot_angle)

Step 5: cs = cs_mag · e^{j·ang_a}      = cs_mag · (cos(ang_a) + j·sin(ang_a))
        sn = sn_mag · e^{j·(ang_b+π)} = −sn_mag · e^{j·ang_b}
```

Total CORDIC calls: **5 calls × 16 iterations = 80 shift-add cycles** per Givens coefficient pair.  
Equivalent floating-point cost: 2 `sqrt` + 2 `atan2` + 2 `cos` + 2 `sin` = **8 transcendental calls**.

---

## 9. Applying CORDIC to Givens Rotation

### The Givens Rotation Matrix

For complex entries at rows j and i of a matrix, the Givens rotation G(j,i) is defined as:

```
G =  [  cs    −sn  ]     where:
     [ sn*    cs*  ]

cs = a/r,   sn = −b/r,   r = √(|a|² + |b|²)
a = R[j,j],  b = R[i,j]  (pivot and element to zero)
```

This is a 2×2 unitary matrix: G^H · G = I (columns are orthonormal).

### Verification that G Zeros R[i,j]

After applying G to rows j and i:

```
new R[j,j] = cs·a + (−sn)·b = (a/r)·a + (b/r)·b = (|a|² + |b|²)/r = r²/r = r  ✓
new R[i,j] = sn*·a + cs*·b  = (−b*/r)·a + (a*/r)·b
           = (−b*a + a*b)/r
```

For real a and b: `(−b·a + a·b)/r = 0` ✓  
For complex a, b: `(−b*a + a*b)/r = 2j·Im(a*b)/r` — this is zero only if a and b are in phase.

For complex Givens, the full phase compensation (Steps 5 in Section 8) ensures the element is exactly zeroed.

### Effect on the Upper Triangular Structure

The Givens rotations are applied **column by column, bottom to top**:

```
Column j: zero elements R[M-1,j], R[M-2,j], ..., R[j+1,j] in sequence
```

Each rotation zeros exactly one sub-diagonal element while preserving zeros already created in previous columns — this is guaranteed by the bottom-to-top sweep order.

After all M(M-1)/2 rotations, R is upper triangular.

---

## 10. Full Givens QR Decomposition

### Algorithm

```
Input:  A ∈ ℂᴹˣᴹ   (the upper Cholesky factor Lt of Rxx)
Output: Q ∈ ℂᴹˣᴹ   (unitary: Q^H·Q = I)
        R ∈ ℂᴹˣᴹ   (upper triangular)
        such that A = Q·R

Initialise: R ← A,   Q ← Iₘ  (M×M identity)

For j = 1 to M:                          (column loop)
    For i = M down to j+1:               (row loop, bottom to top)
        If |R[i,j]| > ε:                 (skip if already zero)
            Compute cs, sn from R[j,j], R[i,j]  via CORDIC
            
            [R[j,k]]  ←  [ cs    −sn ] · [R[j,k]]    for all k
            [R[i,k]]     [ sn*   cs* ]   [R[i,k]]
            
            [Q[r,j]  Q[r,i]]  ←  [Q[r,j]  Q[r,i]] · [cs*  sn*]    for all r
                                                       [−sn  cs ]
```

### Operation Count

- Total rotations: M(M-1)/2 = 28 for M=8, 2016 for M=64
- CORDIC calls per rotation: 5 (for complex entries)
- Iterations per CORDIC call: 16
- Row update operations per rotation: 2M complex multiply-adds
- Total operations (M=8): 28 × (5×16 + 2×8×2) ≈ 28 × 112 = 3136 operations
- Total operations (M=64): 2016 × (5×16 + 2×64×2) ≈ 2016 × 336 ≈ 677,000 operations

At 300 MHz with pipeline II=1 on the inner loop: estimated latency for M=64 ≈ 10,000–20,000 cycles ≈ 33–67 μs.

### Numerical Properties

The Givens QR has excellent numerical stability because:
1. Each rotation is exactly unitary (||G||₂ = 1) — no amplification of errors
2. The CORDIC computation itself introduces bounded error ≤ 2⁻¹⁵ radians per rotation
3. Accumulated error after M(M-1)/2 rotations: ≤ 2016 × 2⁻¹⁵ ≈ 0.062 radians for M=64

This accumulated error explains the 0.56 dB null degradation observed in MATLAB fixed-point simulation.

---

## 11. Computing the Inverse via QR

### Why We Don't Invert Directly

Direct matrix inversion of an M×M matrix costs **O(M³)** operations using Gaussian elimination.  
For M=64: 64³ = 262,144 operations.

More critically, direct inversion is **numerically unstable** — errors in intermediate steps amplify dramatically.

### The QR Approach: Solving Rxx·w = a_s

The MVDR weight vector requires solving:

```
Rxx · w = a_s
```

This is equivalent to computing `w = Rxx⁻¹ · a_s`, but we **never form Rxx⁻¹ explicitly**.

Instead, we use the factored form. With `Rxx = L · Lᴴ` (Cholesky) and `Lᴴ = Q · R` (QR via Givens):

```
Rxx · w = a_s
L · Lᴴ · w = a_s
L · Q · R · w = a_s
```

This is solved in three stages, each requiring only **triangular system solves** — O(M²) each.

### Mathematical Path — Stage by Stage

**Stage 1: Cholesky factorisation** (done on PS ARM in software)

```
Rxx = L · Lᴴ

where L is lower triangular with real positive diagonal.
This exists because Rxx is Hermitian positive definite (guaranteed after diagonal loading).

Cost: O(M³/3) — cheaper than full LU by factor of 2
Done once per coherence interval (~ms timescale)
```

**Stage 2: Givens QR on Lᴴ** (done on PL via our HLS module)

```
Lᴴ = Q · R

where Q is unitary (Q^H·Q = I) and R is upper triangular.
This is what our givens_qr() function computes.
```

**Stage 3: Forward substitution — solve L·z = a_s** (done on PL in back_substitution())

Since L is lower triangular:

```
L · z = a_s

Solve row by row from top (i=0) to bottom (i=M-1):

z[0] = a_s[0] / L[0,0]

z[1] = (a_s[1] − L[1,0]·z[0]) / L[1,1]

z[2] = (a_s[2] − L[2,0]·z[0] − L[2,1]·z[1]) / L[2,2]

General: z[i] = (a_s[i] − Σₖ₌₀ⁱ⁻¹ L[i,k]·z[k]) / L[i,i]
```

Note: L[i,i] is real and positive (Cholesky diagonal property) — this simplifies the division.

**Stage 4: Multiply by Q^H**

```
rhs = Q^H · z

rhs[i] = Σₖ₌₀ᴹ⁻¹ conj(Q[k,i]) · z[k]
```

This is a matrix-vector multiply with a unitary matrix — no inversion, no division.

**Stage 5: Back substitution — solve R·w = rhs** (done on PL)

Since R is upper triangular:

```
R · w = rhs

Solve row by row from bottom (i=M-1) to top (i=0):

w[M-1] = rhs[M-1] / R[M-1,M-1]

w[M-2] = (rhs[M-2] − R[M-2,M-1]·w[M-1]) / R[M-2,M-2]

General: w[i] = (rhs[i] − Σₖ₌ᵢ₊₁ᴹ⁻¹ R[i,k]·w[k]) / R[i,i]
```

**Stage 6: Distortionless normalisation** (done on PS ARM)

```
W_MVDR = w / (a_sᴴ · w)

This enforces: W_MVDRᴴ · a_s = 1   (unity gain at target direction)

The scalar (a_sᴴ · w) is a single complex dot product — trivial on ARM.
```

### Why This is Equivalent to Rxx⁻¹ · a_s

Trace through the algebra:

```
Rxx · w = a_s
⟺  L·Lᴴ · w = a_s                     (Cholesky)
⟺  L · (Lᴴ · w) = a_s
⟺  L · (Q·R · w) = a_s                 (QR of Lᴴ)
⟺  L·Q · (R·w) = a_s

Let v = R·w, then:
L·Q · v = a_s
Q^H · L^{-1} · a_s = R · w             (multiply left by Q^H·L^{-1})

So: w = R^{-1} · Q^H · L^{-1} · a_s

And: W_MVDR = w / (a_sᴴ · w) = Rxx^{-1}·a_s / (a_sᴴ · Rxx^{-1}·a_s)  ✓
```

This is exactly the MVDR optimal weight vector from Lagrange multiplier optimisation.

### Visualising the Factored Solve

```
      Rxx · w = a_s
       ↓
  [L · Lᴴ] · w = a_s
       ↓
  Step 1: L · z = a_s          → z = L⁻¹·a_s    (forward sub, O(M²))
       ↓
  Now: Lᴴ · w = z
  But: Lᴴ = Q·R
       ↓
  Step 2: Q·R · w = z
  Step 3: rhs = Q^H · z = R · w  (multiply left by Q^H, O(M²))
       ↓
  Step 4: R · w = rhs            → w = R⁻¹·rhs  (back sub, O(M²))
       ↓
  Step 5: normalise w            → W_MVDR       (scalar divide, O(M))

Total cost: O(M²) — vs O(M³) for direct inversion
```

---

## 12. Forward and Back Substitution

### Forward Substitution in Detail

Solves L·z = a_s where L is lower triangular.

```
L = [ L₀₀   0    0   ...  0  ]
    [ L₁₀  L₁₁   0   ...  0  ]
    [ L₂₀  L₂₁  L₂₂  ...  0  ]
    [ ...                     ]
    [ Lₘ₀  Lₘ₁  ...  ... Lₘₘ ]

Expanding L·z = a_s row by row:

Row 0:  L₀₀·z₀ = a_s[0]
        → z₀ = a_s[0] / L₀₀

Row 1:  L₁₀·z₀ + L₁₁·z₁ = a_s[1]
        → z₁ = (a_s[1] − L₁₀·z₀) / L₁₁

Row 2:  L₂₀·z₀ + L₂₁·z₁ + L₂₂·z₂ = a_s[2]
        → z₂ = (a_s[2] − L₂₀·z₀ − L₂₁·z₁) / L₂₂
```

Each row i has i multiplications and 1 division. Total: M²/2 multiplications + M divisions.

Note for Cholesky L: **diagonal entries L[i,i] are always real and positive**, so the division is by a real positive number — simpler in hardware than complex division.

### Back Substitution in Detail

Solves R·w = rhs where R is upper triangular (result of QR).

```
R = [ R₀₀  R₀₁  R₀₂  ...  R₀ₘ ]
    [  0   R₁₁  R₁₂  ...  R₁ₘ ]
    [  0    0   R₂₂  ...  R₂ₘ ]
    [ ...                      ]
    [  0    0    0   ...  Rₘₘ  ]

Expanding R·w = rhs row by row from bottom:

Row M-1: Rₘₘ·wₘ = rhs[M-1]
         → wₘ = rhs[M-1] / Rₘₘ

Row M-2: R_{M-2,M-1}·wₘ + R_{M-2,M-2}·w_{M-2} = rhs[M-2]
         → w_{M-2} = (rhs[M-2] − R_{M-2,M-1}·wₘ) / R_{M-2,M-2}

General: w[i] = (rhs[i] − Σₖ₌ᵢ₊₁ᴹ⁻¹ R[i,k]·w[k]) / R[i,i]
```

Note for QR R: **diagonal entries R[i,i] are complex in general** (unlike Cholesky). Division by a complex number:

```
(p + jq) / (r + js) = (pr + qs)/(r² + s²) + j(qr − ps)/(r² + s²)
```

This requires 4 multiplications and 1 real division. In hardware: use a reciprocal LUT for `1/(r²+s²)`.

---

## 13. End-to-End MVDR Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│                    PS ARM (Software)                            │
│                                                                 │
│  1. Receive X (M×N data matrix) from ADC via DMA               │
│  2. Compute Rxx = (1/N)·X·X^H + δ·I  [covariance + diag load] │
│  3. Compute L = chol(Rxx, 'lower')    [Cholesky factorisation] │
│  4. Write L, L^H, a_s to DDR memory                            │
│  5. Trigger PL hardware via AXI4-Lite control register          │
│  6. Poll done flag                                              │
│  7. Read w from DDR                                             │
│  8. Compute scale = a_s^H · w         [scalar dot product]     │
│  9. W_MVDR = w / scale                [normalisation]          │
│ 10. Apply W_MVDR to streaming data: y = W_MVDR^H · x           │
└──────────────────────────────┬──────────────────────────────────┘
                               │ AXI4-Lite (start/done)
                               │ AXI4 (L, L^H, a_s, w via DDR)
┌──────────────────────────────▼──────────────────────────────────┐
│              PL FPGA Fabric (givens_qr_top HLS module)          │
│                                                                 │
│  ┌─────────────┐    ┌─────────────────┐    ┌────────────────┐  │
│  │  DMA Read   │    │   Givens QR     │    │ Back-Substit.  │  │
│  │  L^H, L,   │───▶│  (CORDIC-based) │───▶│  Fwd sub on L  │  │
│  │  a_s        │    │  Produces Q, R  │    │  Q^H · z       │  │
│  └─────────────┘    └─────────────────┘    │  Back sub on R │  │
│                                            └───────┬────────┘  │
│                                                    │            │
│  ┌─────────────┐                                   │            │
│  │  DMA Write  │◀──────────────────────────────────┘            │
│  │  w to DDR   │                                                │
│  └─────────────┘                                                │
└─────────────────────────────────────────────────────────────────┘
```

### Timing Budget (M=64, 300 MHz)

| Stage | Location | Estimated Cycles | Time |
|-------|----------|-----------------|------|
| Covariance Rxx | PS ARM | ~50,000 | ~167 μs |
| Cholesky L | PS ARM | ~20,000 | ~67 μs |
| DMA write to DDR | AXI/NoC | ~1,000 | ~3.3 μs |
| Givens QR (M=64) | PL HLS | ~15,000 | ~50 μs |
| Back substitution | PL HLS | ~3,000 | ~10 μs |
| DMA read from DDR | AXI/NoC | ~500 | ~1.7 μs |
| Normalisation | PS ARM | ~200 | ~0.7 μs |
| **Total** | | **~89,700** | **~299 μs** |

Weight update rate: ~3.3 kHz — well within radar coherence intervals (~10 ms).

---

## 14. Fixed-Point Precision Analysis

### Word Length Summary (derived from MATLAB range logging)

| Variable | Max Magnitude | Format | Int Bits | Frac Bits | Resolution |
|----------|--------------|--------|----------|-----------|------------|
| Rxx | 27.54 | `ap_fixed<16,6>` | 5+sign | 10 | 2⁻¹⁰ ≈ 9.8×10⁻⁴ |
| L, Lt | 5.23 | `ap_fixed<16,4>` | 3+sign | 12 | 2⁻¹² ≈ 2.4×10⁻⁴ |
| Q | 1.00 | `ap_fixed<16,2>` | 1+sign | 14 | 2⁻¹⁴ ≈ 6.1×10⁻⁵ |
| R | 5.23 | `ap_fixed<18,4>` | 3+sign | 14 | 2⁻¹⁴ ≈ 6.1×10⁻⁵ |
| z, rhs | 0.48 | `ap_fixed<16,1>` | 0+sign | 15 | 2⁻¹⁵ ≈ 3.1×10⁻⁵ |
| W_MVDR | 0.023 | `ap_fixed<18,1>` | 0+sign | 17 | 2⁻¹⁷ ≈ 7.6×10⁻⁶ |
| CORDIC internal | variable | `ap_fixed<32,4>` | 3+sign | 28 | 2⁻²⁸ ≈ 3.7×10⁻⁹ |

### Validation Results (from MATLAB `MVDR_FixedPoint.m`)

```
Distortionless check : |a_s^H · W| = 0.999985   (target: 1.000000)
Null depth (float)   : −78.09 dB
Null depth (fixed)   : −77.54 dB
Null degradation     : 0.56 dB                   (limit: 3.00 dB)  ✓ PASS
Mainlobe degradation : 0.00 dB                   (limit: 1.00 dB)  ✓ PASS
```

---

## 15. Hardware Architecture Summary

### Resource Estimate (M=8, Vitis HLS 2024.1, Versal VCK190)

| Resource | Estimated | VCK190 Available | Utilisation |
|----------|-----------|-----------------|-------------|
| LUT | ~12,000 | 899,840 | ~1.3% |
| FF (Flip-Flop) | ~8,000 | 1,799,680 | ~0.4% |
| DSP58 | ~24 | 1,968 | ~1.2% |
| BRAM 36K | ~8 | 967 | ~0.8% |
| Target Clock | 300 MHz | — | — |

For M=64, multiply LUT and FF estimates by ~8×, DSP by ~4× — still well within VCK190 capacity.

### Key Design Decisions

1. **CORDIC over Newton-Raphson sqrt**: CORDIC uses 0 DSPs vs ~8 DSPs for NR sqrt. For 2016 rotations (M=64), this saves ~16,000 DSP operations.

2. **Row-only update in Givens loop**: Only 2 rows of R and 2 columns of Q change per rotation. Full matrix multiply would cost M² extra operations per rotation. Row-only update costs 2M — a factor M/2 = 32 improvement for M=64.

3. **ARRAY_PARTITION cyclic**: Splits BRAM banks so the inner k-loop can read M elements per cycle. Without partitioning, BRAM has only 2 read ports — the k-loop would take M cycles instead of 1.

4. **Normalisation on PS**: Division by complex scalar `(a_s^H · w)` moved to ARM. On PL, complex division requires a reciprocal circuit (~10 DSPs). On ARM, it's a single instruction. This saves hardware at the cost of a negligible AXI round-trip.

5. **Burst DMA transfers**: Reading the 64×64 matrix as a burst (one AXI transaction) instead of element-by-element reduces AXI overhead from 4096 transactions to 1, saving ~4095 × 5 ns = ~20 μs.

---

*Generated for MVDR Adaptive Beamforming on Versal VCK190*  
*MATLAB validation: MVDR_Givens_Corrected.m, MVDR_FixedPoint.m*  
*HLS implementation: givens_qr.h, givens_qr.cpp, givens_qr_tb.cpp*
