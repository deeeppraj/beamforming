# ============================================================
#  run_hls.tcl  –  Vitis HLS Project Script
#  Target: Versal VCK190  (xcvc1902-vsva2197-2MP-e-S)
#  Tool  : Vitis HLS 2024.1
#  Usage : vitis_hls -f run_hls.tcl
# ============================================================

# ── Project Setup ────────────────────────────────────────────
open_project mvdr_qr_prj
set_top mvdr_qr_top

# ── Source Files ─────────────────────────────────────────────
add_files mvdr_qr.cpp
add_files -tb tb_mvdr_qr.cpp -cflags "-Wno-unused-label"

# ── Solution for Versal ──────────────────────────────────────
open_solution "solution_versal" -flow_target vitis

# Versal AI Core VCK190 part
set_part {xcvc1902-vsva2197-2MP-e-S}

# Target 400 MHz (2.5 ns period)
create_clock -period 2.5 -name default

# ── C Simulation ─────────────────────────────────────────────
csim_design -clean -argv ""

# ── Synthesis ────────────────────────────────────────────────
csynth_design

# ── Co-simulation (optional: requires RTL sim) ───────────────
# cosim_design -wave_debug -trace_level all

# ── Export RTL as Vitis Kernel (.xo) ─────────────────────────
export_design -format xo -output mvdr_qr_kernel.xo

# ── Report timing & resource ─────────────────────────────────
puts "\n=== Synthesis Report Summary ==="
puts [report_utilization]
puts [report_timing_summary]

close_project
