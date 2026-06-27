# Synthesis & Implementation Fix Report
## MVDR System — VCK190 Vivado 2025.2

---

## Summary of Issues Found

| # | Issue | Severity | Status |
|---|---|---|---|
| 1 | smartconnect_1 black box | BLOCKING | Fix required |
| 2 | x_in_0 IOSTANDARD undefined | BLOCKING | Fix required |
| 3 | Clock defined on wrong pin | BLOCKING (causes timing) | Fix required |
| 4 | Setup/Hold violations | Caused by #3 | Resolves after #3 |
| 5 | Thermal margin 0°C | Warning | Monitor |

---

## Fix 1 — smartconnect_1 Black Box

**Error:**
```
[DRC INBB-3] Cell 'mvdr_bd_i/smartconnect_1/inst' has undefined contents
and is considered a black box. opt_design cannot complete.
```

**Cause:** smartconnect_1 output products were never generated.

**Fix in Vivado Block Design:**

1. Right-click `smartconnect_1` → **Reset Output Products**
2. Right-click `smartconnect_1` → **Generate Output Products** → select **Global** → click **Generate**
3. If still failing — delete smartconnect_1, re-add it, reconfigure:
   - Number of Slave Interfaces: **2**
   - Number of Master Interfaces: **1**
4. Reconnect:

| From | Port | To | Port |
|---|---|---|---|
| mvdr_weights_0 | m_axi_gmem1 | smartconnect_1 | S00_AXI |
| mvdr_weights_0 | m_axi_gmem2 | smartconnect_1 | S01_AXI |
| smartconnect_1 | M00_AXI | axi_noc_0 | S06_AXI |

5. Re-run **Generate Output Products** on the full block design

---

## Fix 2 — x_in_0 IOSTANDARD Undefined

**Error:**
```
[Designutils 20-4377] Port x_in_0_tready has UNDEFINED or DIFF_UNDEFINED
standard. SSN calculation cannot be done.
```

**Cause:** External AXI-Stream port `x_in_0` has no IO standard assigned in XDC.

**Fix — Add to XDC file:**

```tcl
set_property IOSTANDARD LVCMOS18 [get_ports x_in_0_*]
```

---

## Fix 3 — Clock Constraints (Root Cause of All Timing Violations)

**Methodology Errors:**
```
Primary clock created on inappropriate pin clk_wizard_0/inst/clk_in1
Primary clock clk_in1 overrides insertion delay of clk_pl_0
```

**Cause:** Vivado auto-created the primary clock on the clocking wizard
input pin instead of the CIPS output or the wizard output. This makes
all timing analysis reference the wrong point, producing garbage slack
numbers that do not respond to frequency changes — which is why
reducing to 250 MHz had no effect.

**Fix — Replace all existing clock constraints in XDC with:**

```tcl
###############################################################################
# Primary clock — CIPS PL0 reference output (actual source)
###############################################################################
create_clock -period 5.002 -name clk_pl_0 \
    [get_pins versal_cips_0/inst/pspmc_0/inst/IO_CLK_OUT_0]

###############################################################################
# PL clock — Clocking Wizard MMCM output (300 MHz)
###############################################################################
create_clock -period 3.333 -name pl_clk300 \
    [get_pins clk_wiz_0/inst/mmcme4_adv_inst/CLKOUT0]

###############################################################################
# LPDDR4 reference clock — 100 MHz differential input
###############################################################################
set_property PACKAGE_PIN BH51 [get_ports {lpddr4_clk_p}]
set_property PACKAGE_PIN BJ51 [get_ports {lpddr4_clk_n}]
set_property IOSTANDARD LVDS  [get_ports {lpddr4_clk_p}]
set_property IOSTANDARD LVDS  [get_ports {lpddr4_clk_n}]

###############################################################################
# x_in_0 AXI-Stream external port IO standard
###############################################################################
set_property IOSTANDARD LVCMOS18 [get_ports x_in_0_*]

###############################################################################
# Clock groups — clk_pl_0 and pl_clk300 are asynchronous
# (MMCM re-synchronises them internally)
###############################################################################
set_clock_groups -asynchronous \
    -group [get_clocks clk_pl_0] \
    -group [get_clocks pl_clk300]

###############################################################################
# False path on reset
###############################################################################
set_false_path -from [get_pins versal_cips_0/inst/pspmc_0/inst/IO_RST_OUT_0]
```

**After applying:**
- Set clk_wiz_0 back to **300 MHz** output
- Re-run Implementation
- Setup and Hold violations should resolve — they were artifacts of wrong clock definition

---

## Fix 4 — Timing (Expected Resolution After Fix 3)

**Current values (incorrect — caused by wrong clock definition):**

| Type | Worst Slack | Total Violation | Failing Endpoints |
|---|---|---|---|
| Setup | -0.579 ns | -226.190 ns | 1030 |
| Hold | -0.288 ns | -1462.393 ns | 28833 |

**Why 250 MHz did not help:** Vivado was not analyzing timing against
the actual clock tree — frequency change had no effect because the
clock source was wrong.

**After Fix 3**, if violations persist at 300 MHz:
- Try **280 MHz** first (period = 3.571 ns)
- If still failing at 280 MHz, the issue is in HLS kernel pipelining —
  re-export kernels from Vitis HLS with target clock set to **4 ns (250 MHz)**
  to give more slack margin

---

## No Action Required

### Pulse Width — OK
| Type | Worst Slack | Failing Endpoints |
|---|---|---|
| Pulse Width | 0.238 ns | 0 |

### Utilization — Healthy

| Resource | Utilization |
|---|---|
| LUT | 5.08% |
| FF | 1.53% |
| BRAM | 3.31% |
| DSP | 14.74% |
| IO | 12.86% |
| MMCM | 8.33% |

### Power — Acceptable

| Category | Power | Share of Dynamic |
|---|---|---|
| PS | 1.166 W | 34% |
| NoC + DDRMC | 0.559 W | 16% |
| DSP | 0.674 W | 19% |
| Logic | 0.391 W | 11% |
| IO | 0.249 W | 7% |
| Clocks | 0.155 W | 4% |
| BRAM | 0.089 W | 3% |
| XPLL | 0.152 W | 4% |
| MMCM | 0.062 W | 2% |

- Total on-chip power: **16.939 W**
- Junction temperature: **100°C** — thermal margin is 0°C
- Ensure active cooling (heatsink + fan) is attached to VCK190 before running

---

## Fix Order

| Step | Action |
|---|---|
| 1 | Reset and regenerate smartconnect_1 output products |
| 2 | Replace XDC file with corrected constraints above |
| 3 | Set clk_wiz_0 back to 300 MHz |
| 4 | Re-run Generate Output Products on full block design |
| 5 | Re-run Implementation |
| 6 | Check timing report — violations should be gone |
| 7 | If timing still fails at 300 MHz, reduce to 280 MHz and retry |
