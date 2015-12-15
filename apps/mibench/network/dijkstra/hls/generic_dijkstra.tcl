#*******************************************************************************
# Vendor: Xilinx #Associated Filename: generic.tcl
# Purpose: Vivado HLS tcl script 
# Author: diamantd@microlab.ntua.gr
#*******************************************************************************

catch {eval exec "uname -m"} machine_name
catch {eval exec "uname -s"} kernel_name
catch {eval exec "vivado_hls -r"} vivado_hls_home

if {$machine_name == "x86_64" && $kernel_name == "Linux"} {
    set gcc_lib $vivado_hls_home/Linux_x86_64/tools/gcc/lib64
} elseif {$machine_name == "i686" && $kernel_name == "Linux"} {
    set gcc_lib $vivado_hls_home/Linux_x86/tools/gcc/lib
} else {
    set gcc_lib $vivado_hls_home/msys/lib/gcc/mingw32/4.6.2
}
set ::env(LD_LIBRARY_PATH) $gcc_lib:$::env(LD_LIBRARY_PATH)

# Create the design project
open_project prj -reset

# Define the top level function for hardware synthesis
set_top dijkstra

# Select the files for hardware synthesis
add_files ../memluv.c
add_files ../../apps/mibench/network/dijkstra/dijkstra_small.c

# Select the files required for the testbench
add_files -tb ../../apps/mibench/network/dijkstra/dijkstra_small.c ../../apps/mibench/network/dijkstra/input.dat

# Set the name of the solution for this design
open_solution "solution1"

# Select the FPGA 
set_part "xc7z020clg484-1"
#set_part "xc7k325tffg900-2"

create_clock -period "150MHz"

#config_dataflow -default_channel fifo
#config_schedule -effort high -verbose
#config_bind -effort high

# Vivado HLS commands for C simulation
csim_design -argv "../../../../../../apps/mibench/network/dijkstra/input.dat"

# Vivado HLS commands for C to RTL synthesis
csynth_design

# Vivado HLS commands for RTL simulation
cosim_design -rtl vhdl -tool isim -trace_level all  -argv "../../../../../../apps/mibench/network/dijkstra/input.dat"
#cosim_design -rtl vhdl -tool modelsim -trace_level all
#cosim_design -rtl vhdl -tool ncsim -trace_level all
#cosim_design -argv "../../../../../../apps/mibench/network/dijkstra/input.dat" 

# Vivado HLS commands for RTL implementation
#export_design -format ip_catalog -version 1.05.a

close_project

exit
