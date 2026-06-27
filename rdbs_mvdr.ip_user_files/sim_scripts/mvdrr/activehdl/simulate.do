transcript off
onbreak {quit -force}
onerror {quit -force}
transcript on

asim +access +r +m+mvdrr  -L xil_defaultlib -L xilinx_vip -L xpm -L proc_sys_reset_v5_0_17 -L smartconnect_v1_0 -L axi_infrastructure_v1_1_0 -L axi_register_slice_v2_1_36 -L axi_vip_v1_1_22 -L axi_apb_bridge_v3_0_21 -L versal_cips_ps_vip_v1_0_14 -L cpm4_v1_0_20 -L cpm5_v1_0_20 -L noc_nmu_sim_v1_0_0 -L noc_hbm_nmu_sim_v1_0_0 -L xlconstant_v1_1_10 -L axi_datamover_v5_1_37 -L axi_sg_v4_1_21 -L axi_dma_v7_1_37 -L xbip_utils_v3_0_15 -L axi_utils_v2_0_11 -L xbip_pipe_v3_0_11 -L xbip_dsp48_wrapper_v3_0_7 -L mult_gen_v12_0_24 -L floating_point_v7_1_21 -L xilinx_vip -L unisims_ver -L unimacro_ver -L secureip -O5 xil_defaultlib.mvdrr xil_defaultlib.glbl

do {mvdrr.udo}

run 1000ns

endsim

quit -force
