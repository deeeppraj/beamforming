namespace eval ::optrace {
  variable script "C:/Users/Deepraj/Desktop/rdbs_mvdr/rdbs_mvdr.runs/impl_1/mvdrr_wrapper.tcl"
  variable category "vivado_impl"
}

# Try to connect to running dispatch if we haven't done so already.
# This code assumes that the Tcl interpreter is not using threads,
# since the ::dispatch::connected variable isn't mutex protected.
if {![info exists ::dispatch::connected]} {
  namespace eval ::dispatch {
    variable connected false
    if {[llength [array get env XILINX_CD_CONNECT_ID]] > 0} {
      set result "true"
      if {[catch {
        if {[lsearch -exact [package names] DispatchTcl] < 0} {
          set result [load librdi_cd_clienttcl[info sharedlibextension]] 
        }
        if {$result eq "false"} {
          puts "WARNING: Could not load dispatch client library"
        }
        set connect_id [ ::dispatch::init_client -mode EXISTING_SERVER ]
        if { $connect_id eq "" } {
          puts "WARNING: Could not initialize dispatch client"
        } else {
          puts "INFO: Dispatch client connection id - $connect_id"
          set connected true
        }
      } catch_res]} {
        puts "WARNING: failed to connect to dispatch server - $catch_res"
      }
    }
  }
}
if {$::dispatch::connected} {
  # Remove the dummy proc if it exists.
  if { [expr {[llength [info procs ::OPTRACE]] > 0}] } {
    rename ::OPTRACE ""
  }
  proc ::OPTRACE { task action {tags {} } } {
    ::vitis_log::op_trace "$task" $action -tags $tags -script $::optrace::script -category $::optrace::category
  }
  # dispatch is generic. We specifically want to attach logging.
  ::vitis_log::connect_client
} else {
  # Add dummy proc if it doesn't exist.
  if { [expr {[llength [info procs ::OPTRACE]] == 0}] } {
    proc ::OPTRACE {{arg1 \"\" } {arg2 \"\"} {arg3 \"\" } {arg4 \"\"} {arg5 \"\" } {arg6 \"\"}} {
        # Do nothing
    }
  }
}

proc start_step { step } {
  set stopFile ".stop.rst"
  if {[file isfile .stop.rst]} {
    puts ""
    puts "*** Halting run - EA reset detected ***"
    puts ""
    puts ""
    return -code error
  }
  set beginFile ".$step.begin.rst"
  set platform "$::tcl_platform(platform)"
  set user "$::tcl_platform(user)"
  set pid [pid]
  set host ""
  if { [string equal $platform unix] } {
    if { [info exist ::env(HOSTNAME)] } {
      set host $::env(HOSTNAME)
    } elseif { [info exist ::env(HOST)] } {
      set host $::env(HOST)
    }
  } else {
    if { [info exist ::env(COMPUTERNAME)] } {
      set host $::env(COMPUTERNAME)
    }
  }
  set ch [open $beginFile w]
  puts $ch "<?xml version=\"1.0\"?>"
  puts $ch "<ProcessHandle Version=\"1\" Minor=\"0\">"
  puts $ch "    <Process Command=\".planAhead.\" Owner=\"$user\" Host=\"$host\" Pid=\"$pid\">"
  puts $ch "    </Process>"
  puts $ch "</ProcessHandle>"
  close $ch
}

proc end_step { step } {
  set endFile ".$step.end.rst"
  set ch [open $endFile w]
  close $ch
}

proc step_failed { step } {
  set endFile ".$step.error.rst"
  set ch [open $endFile w]
  close $ch
OPTRACE "impl_1" END { }
}


OPTRACE "impl_1" START { ROLLUP_1 }
OPTRACE "Phase: Write Device Image" START { ROLLUP_AUTO }
OPTRACE "write_device_image setup" START { }
start_step write_device_image
set ACTIVE_STEP write_device_image
set rc [catch {
  create_msg_db write_device_image.pb
  set_param general.usePosixSpawnForFork 1
  set_param chipscope.maxJobs 2
  set_param power.disableGlitchAnalysis 1
  set_param netlist.writeMacroLevel 0
  set_param power.enableUnconnectedCarry8PinPower 1
  set_param power.enableCarry8RouteBelPower 1
  set_param power.xpeLogicHierarchyThreshold 50
  set_param power.enableLutRouteBelPower 1
  set_param runs.launchOptions { -jobs 8  }
  open_checkpoint mvdrr_wrapper_routed.dcp
  set_property webtalk.parent_dir C:/Users/Deepraj/Desktop/rdbs_mvdr/rdbs_mvdr.cache/wt [current_project]
  set_property segmented_configuration 0 [current_design]
OPTRACE "read constraints: write_bitstream" START { }
OPTRACE "read constraints: write_bitstream" END { }
OPTRACE "read constraints: write_bitstream" START { }
OPTRACE "read constraints: write_bitstream" END { }
  set_property XPM_LIBRARIES {XPM_CDC XPM_FIFO XPM_MEMORY} [current_project]
  catch { write_mem_info -force -no_partial_mmi mvdrr_wrapper.mmi }
OPTRACE "write_device_image setup" END { }
OPTRACE "write_device_image" START { }
  write_device_image -force mvdrr_wrapper.pdi 
OPTRACE "write_device_image" END { }
OPTRACE "write_device_image misc" START { }
OPTRACE "read constraints: write_bitstream_post" START { }
OPTRACE "read constraints: write_bitstream_post" END { }
OPTRACE "read constraints: write_device_image_post" START { }
OPTRACE "read constraints: write_device_image_post" END { }
  catch {write_debug_probes -quiet -force mvdrr_wrapper}
  catch {file copy -force mvdrr_wrapper.ltx debug_nets.ltx}
  close_msg_db -file write_device_image.pb
} RESULT]
if {$rc} {
OPTRACE "write_device_image write_checkpoint" START { CHECKPOINT }
OPTRACE "write_device_image write_checkpoint" END { }
  step_failed write_device_image
  return -code error $RESULT
} else {
  end_step write_device_image
  unset ACTIVE_STEP 
}

OPTRACE "write_device_image misc" END { }
OPTRACE "Phase: Write Device Image" END { }
OPTRACE "impl_1" END { }
