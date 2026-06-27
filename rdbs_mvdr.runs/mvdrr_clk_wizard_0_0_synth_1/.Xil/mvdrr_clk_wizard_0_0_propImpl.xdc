set_property SRC_FILE_INFO {cfile:c:/Users/Deepraj/Desktop/rdbs_mvdr/rdbs_mvdr.gen/sources_1/bd/mvdrr/ip/mvdrr_clk_wizard_0_0/mvdrr_clk_wizard_0_0.xdc rfile:../../../rdbs_mvdr.gen/sources_1/bd/mvdrr/ip/mvdrr_clk_wizard_0_0/mvdrr_clk_wizard_0_0.xdc id:1 order:EARLY scoped_inst:inst} [current_design]
current_instance inst
set_property src_info {type:SCOPED_XDC file:1 line:9 export:INPUT save:INPUT read:READ} [current_design]
set_input_jitter [get_clocks -of_objects [get_ports clk_in1]] 0.1
