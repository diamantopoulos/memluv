

add_custom_target(vivado 
COMMAND vivado_hls -f generic.tcl
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(vivado_accelerators
#COMMAND vivado_hls -f generic_accelerators.tcl | tail -n 20
COMMAND vivado_hls -f generic_accelerators.tcl
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(vsim 
COMMAND vsim ./prj/solution1/sim/vhdl/yadmm.wlf &
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(vsim_accelerators
COMMAND vsim ./prj/solution1/sim/vhdl/accelerators.wlf &
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(isim_all
COMMAND isimgui -view ./prj/solution1/sim/vhdl/isim.wdb &
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(vivado_histogram
COMMAND vivado_hls -f generic_histogram.tcl
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${MAIN_BIN_FILE}
)