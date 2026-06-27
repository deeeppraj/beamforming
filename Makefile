# ============================================================
#  Makefile  –  QR-MVDR on Versal VCK190
#  Targets: csim | synth | cosim | xo | xclbin | host | all
# ============================================================

PLATFORM   := xilinx_vck190_base_202410_1
PART       := xcvc1902-vsva2197-2MP-e-S
TARGET     := hw       # hw | hw_emu | sw_emu
FREQ_MHZ   := 400

XRT        ?= /opt/xilinx/xrt
VITIS      ?= /tools/Xilinx/Vitis/2024.1
HLS        := $(VITIS)/bin/vitis_hls
VPP        := $(VITIS)/bin/v++

KERNEL_XO  := mvdr_qr_kernel.xo
XCLBIN     := mvdr_qr_$(TARGET).xclbin
HOST_EXE   := host_mvdr_qr

CXXFLAGS   := -O2 -std=c++17 -I$(XRT)/include
LDFLAGS    := -L$(XRT)/lib -lxrt_coreutil -lpthread

# ─────────────────────────────────────────────────────────────
.PHONY: all csim synth cosim xo xclbin host clean

all: host xclbin

# 1. C simulation only
csim:
	$(HLS) -f run_hls.tcl -tclargs csim

# 2. HLS synthesis
synth:
	$(HLS) -f run_hls.tcl

# 3. Co-simulation (needs RTL sim license)
cosim:
	$(HLS) -f run_hls.tcl -tclargs cosim

# 4. Export kernel .xo from HLS project
xo: synth
	cp mvdr_qr_prj/solution_versal/impl/export/$(KERNEL_XO) .

# 5. Link to .xclbin
xclbin: xo
	$(VPP) -l -t $(TARGET)                       \
	       --platform $(PLATFORM)                \
	       --config vitis_link.cfg               \
	       --kernel_frequency $(FREQ_MHZ)        \
	       $(KERNEL_XO) -o $(XCLBIN)

# 6. Build host
host: host_mvdr_qr.cpp
	g++ $(CXXFLAGS) $< -o $(HOST_EXE) $(LDFLAGS)

# 7. Run on hardware
run: host xclbin
	./$(HOST_EXE) $(XCLBIN)

# 8. Clean
clean:
	rm -rf mvdr_qr_prj *.xo *.xclbin $(HOST_EXE) \
	       *.log *.jou _x vitis_analyzer*
