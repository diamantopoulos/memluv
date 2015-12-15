

add_custom_target(vivado_dijkstra 
COMMAND vivado_hls -f ../../apps/mibench/network/dijkstra/hls/generic_dijkstra.tcl
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/hls
DEPENDS ${DIJKSTRA_BIN_FILE}
)
