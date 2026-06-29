/*
 * main.c - MVDR Beamformer Control Program
 * Target: Versal VCK190, Cortex-A72 (bare-metal), Vitis 2024.2 (SDT flow)
 *
 * Current state of debugging:
 *   - Drivers initialize OK
 *   - Cache flush works
 *   - DMA chunked at 4096 bytes completes successfully
 *   - RDBS / Covariance / MVDR hang on IsDone (under investigation)
 *
 * Data formats (from HLS source):
 *   Input samples:    ap_fixed<16,1> complex, packed 32-bit (real<<16 | imag)
 *   R matrix:         ap_fixed<32,9> complex {int32 re, int32 im}
 *   Steering vec a:   ap_fixed<32,9> complex {int32 re, int32 im}
 *   Output weights w: ap_fixed<32,9> complex {int32 re, int32 im}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xaxidma.h"
#include "sleep.h"

#include "xrdbs_kernel.h"
#include "xcovariance_estimation.h"
#include "xmvdr_weights.h"

#include "input_samples.h"      /* uint32_t input_samples[64000] */
#include "steering_vector.h"    /* int32_t  steering_vector[21*2] */

#define N_CHANNELS       64
#define N_BEAMS          21
#define N_SNAPSHOTS      1000

#define DDR_INPUT_BASE   0x10000000UL
#define DDR_R_BASE       0x10100000UL
#define DDR_A_BASE       0x10200000UL
#define DDR_W_BASE       0x10300000UL

#define INPUT_SIZE_BYTES (N_CHANNELS * N_SNAPSHOTS * 4)
#define R_SIZE_BYTES     (N_BEAMS * N_BEAMS * 8)
#define A_SIZE_BYTES     (N_BEAMS * 8)
#define W_SIZE_BYTES     (N_BEAMS * 8)

#define W_FRAC_BITS      23
#define W_SCALE          (1 << W_FRAC_BITS)

static XAxiDma                  dma_inst;
static XRdbs_kernel             rdbs_inst;
static XCovariance_estimation   cov_inst;
static XMvdr_weights            mvdr_inst;

static int init_drivers(void)
{
    int status;

    XAxiDma_Config *dma_cfg = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_BASEADDR);
    if (!dma_cfg) { xil_printf("[ERR] DMA LookupConfig failed\r\n"); return XST_FAILURE; }
    status = XAxiDma_CfgInitialize(&dma_inst, dma_cfg);
    if (status != XST_SUCCESS) { xil_printf("[ERR] DMA CfgInitialize failed (%d)\r\n", status); return status; }
    XAxiDma_IntrDisable(&dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

    XRdbs_kernel_Config *rdbs_cfg = XRdbs_kernel_LookupConfig(XPAR_RDBS_KERNEL_0_BASEADDR);
    if (!rdbs_cfg) { xil_printf("[ERR] RDBS LookupConfig failed\r\n"); return XST_FAILURE; }
    status = XRdbs_kernel_CfgInitialize(&rdbs_inst, rdbs_cfg);
    if (status != XST_SUCCESS) { xil_printf("[ERR] RDBS CfgInitialize failed (%d)\r\n", status); return status; }

    XCovariance_estimation_Config *cov_cfg =
        XCovariance_estimation_LookupConfig(XPAR_COVARIANCE_ESTIMATION_0_BASEADDR);
    if (!cov_cfg) { xil_printf("[ERR] Cov LookupConfig failed\r\n"); return XST_FAILURE; }
    status = XCovariance_estimation_CfgInitialize(&cov_inst, cov_cfg);
    if (status != XST_SUCCESS) { xil_printf("[ERR] Cov CfgInitialize failed (%d)\r\n", status); return status; }

    XMvdr_weights_Config *mvdr_cfg = XMvdr_weights_LookupConfig(XPAR_MVDR_WEIGHTS_0_BASEADDR);
    if (!mvdr_cfg) { xil_printf("[ERR] MVDR LookupConfig failed\r\n"); return XST_FAILURE; }
    status = XMvdr_weights_CfgInitialize(&mvdr_inst, mvdr_cfg);
    if (status != XST_SUCCESS) { xil_printf("[ERR] MVDR CfgInitialize failed (%d)\r\n", status); return status; }

    return XST_SUCCESS;
}

static void print_q9_23(int32_t v)
{
    int neg = (v < 0);
    uint32_t mag = neg ? (uint32_t)(-(int64_t)v) : (uint32_t)v;
    uint32_t int_part  = mag >> W_FRAC_BITS;
    uint32_t frac_part = mag & (W_SCALE - 1);
    uint64_t frac_dec  = ((uint64_t)frac_part * 1000000ULL) >> W_FRAC_BITS;
    xil_printf("%s%lu.%06lu", neg ? "-" : " ", int_part, frac_dec);
}

int main(void)
{
    int status;

    xil_printf("\r\n========================================\r\n");
    xil_printf(" MVDR Beamformer  --  VCK190\r\n");
    xil_printf("========================================\r\n");

    if (init_drivers() != XST_SUCCESS) {
        xil_printf("[FATAL] Driver init failed\r\n");
        return -1;
    }
    xil_printf("[OK]  Drivers initialised\r\n");

    /* Stage input data in DDR */
    memcpy((void *)DDR_INPUT_BASE, input_samples,    INPUT_SIZE_BYTES);
    memcpy((void *)DDR_A_BASE,     steering_vector,  A_SIZE_BYTES);
    xil_printf("[OK]  Input samples   @ 0x%08lX  (%lu bytes)\r\n",
               DDR_INPUT_BASE, (unsigned long)INPUT_SIZE_BYTES);
    xil_printf("[OK]  Steering vector @ 0x%08lX  (%lu bytes)\r\n",
               DDR_A_BASE,    (unsigned long)A_SIZE_BYTES);

    /* Flush cache so kernels see fresh DDR data */
    Xil_DCacheFlushRange((UINTPTR)DDR_INPUT_BASE, INPUT_SIZE_BYTES);
    Xil_DCacheFlushRange((UINTPTR)DDR_A_BASE,     A_SIZE_BYTES);

    /* Reset DMA in case prior run left it in a bad state */
    XAxiDma_Reset(&dma_inst);
    while (!XAxiDma_ResetIsDone(&dma_inst)) { }

    /* Configure RDBS with full N_SNAPSHOTS and start it (idles waiting on AXIS) */
    XRdbs_kernel_Set_num_snapshots(&rdbs_inst, N_SNAPSHOTS);
    XRdbs_kernel_Start(&rdbs_inst);
    xil_printf("[OK]  RDBS started (num_snapshots=%d)\r\n", N_SNAPSHOTS);

    /* Configure Covariance and start it (idles waiting on AXIS) */
    XCovariance_estimation_Set_R(&cov_inst, DDR_R_BASE);
    XCovariance_estimation_Start(&cov_inst);
    xil_printf("[OK]  Covariance started, R -> 0x%08lX\r\n", DDR_R_BASE);

    /* Kick off DMA — single transfer of full input buffer.
     * Buffer Length Register is now 23 bits (max 8 MB) so the full
     * 256000-byte transfer fits in one shot, giving RDBS a continuous
     * uninterrupted stream of all 64000 samples. */
    xil_printf("[..]  DMA streaming %lu bytes\r\n", (unsigned long)INPUT_SIZE_BYTES);
    status = XAxiDma_SimpleTransfer(&dma_inst,
                 DDR_INPUT_BASE,
                 INPUT_SIZE_BYTES,
                 XAXIDMA_DMA_TO_DEVICE);
    if (status != XST_SUCCESS) {
        xil_printf("[FATAL] DMA failed (%d)\r\n", status);
        return -1;
    }
    while (XAxiDma_Busy(&dma_inst, XAXIDMA_DMA_TO_DEVICE)) { }
    xil_printf("[OK]  DMA done\r\n");

    /* Diagnostic: poll RDBS with timeout, print status if hangs */
    {
        volatile unsigned long t = 100000000UL;
        while (!XRdbs_kernel_IsDone(&rdbs_inst) && t > 0) t--;
        xil_printf("RDBS: done=%u idle=%u ready=%u\r\n",
                   (unsigned)XRdbs_kernel_IsDone(&rdbs_inst),
                   (unsigned)XRdbs_kernel_IsIdle(&rdbs_inst),
                   (unsigned)XRdbs_kernel_IsReady(&rdbs_inst));
    }

    /* Diagnostic: poll Covariance with timeout, print status if hangs */
    {
        volatile unsigned long t = 100000000UL;
        while (!XCovariance_estimation_IsDone(&cov_inst) && t > 0) t--;
        xil_printf("Cov:  done=%u idle=%u ready=%u\r\n",
                   (unsigned)XCovariance_estimation_IsDone(&cov_inst),
                   (unsigned)XCovariance_estimation_IsIdle(&cov_inst),
                   (unsigned)XCovariance_estimation_IsReady(&cov_inst));
    }

    /* Refresh PS view of R (covariance writes via NoC) */
    Xil_DCacheInvalidateRange((UINTPTR)DDR_R_BASE, R_SIZE_BYTES);

    /* Configure and start MVDR */
    XMvdr_weights_Set_R_in (&mvdr_inst, DDR_R_BASE);
    XMvdr_weights_Set_a_in (&mvdr_inst, DDR_A_BASE);
    XMvdr_weights_Set_w_out(&mvdr_inst, DDR_W_BASE);
    XMvdr_weights_Start(&mvdr_inst);
    xil_printf("[OK]  MVDR started (R@0x%08lX, a@0x%08lX, w@0x%08lX)\r\n",
               DDR_R_BASE, DDR_A_BASE, DDR_W_BASE);

    /* Diagnostic: poll MVDR with timeout, print status if hangs */
    {
        volatile unsigned long t = 100000000UL;
        while (!XMvdr_weights_IsDone(&mvdr_inst) && t > 0) t--;
        xil_printf("MVDR: done=%u idle=%u ready=%u\r\n",
                   (unsigned)XMvdr_weights_IsDone(&mvdr_inst),
                   (unsigned)XMvdr_weights_IsIdle(&mvdr_inst),
                   (unsigned)XMvdr_weights_IsReady(&mvdr_inst));
    }

    /* Read weights out of DDR (whether MVDR finished or not -
     * will be garbage if it didn't, but shows us what's there) */
    Xil_DCacheInvalidateRange((UINTPTR)DDR_W_BASE, W_SIZE_BYTES);

    int32_t w[N_BEAMS * 2];
    memcpy(w, (void *)DDR_W_BASE, W_SIZE_BYTES);

    xil_printf("\r\n=== 21 MVDR Weights (Q9.23 fixed-point) ===\r\n");
    xil_printf("  beam |        real           imag\r\n");
    xil_printf("  -----+------------------------------\r\n");
    for (int i = 0; i < N_BEAMS; i++) {
        xil_printf("  w[%2d] | ", i);
        print_q9_23(w[2 * i]);
        xil_printf("   ");
        print_q9_23(w[2 * i + 1]);
        xil_printf("    (raw: %ld, %ld)\r\n",
                   (long)w[2 * i], (long)w[2 * i + 1]);
    }
    xil_printf("\r\n[END]\r\n");

    return 0;
}
