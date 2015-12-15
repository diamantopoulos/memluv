
ADD_SUBDIRECTORY (hls)

SET( DIJKSTRA_BIN_FILE "dijkstra" )

add_executable (${DIJKSTRA_BIN_FILE} dijkstra_small.c)

# set_target_properties(${DIJKSTRA_BIN_FILE} PROPERTIES COMPILE_FLAGS "-Wno-all -Wno-pedantic -Wno-missing-prototypes -Wno-missing-declarations -Wno-conversion")
ADD_DEFINITIONS(-std=c99 -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wconversion -Wstrict-prototypes)
ADD_DEFINITIONS( -DNO_DEFINE__SYNTHESIS__ )

# Linking with memluv
target_link_libraries (${DIJKSTRA_BIN_FILE} memluv m)

add_custom_target(test-dijkstra
COMMAND dijkstra ../apps/mibench/network/dijkstra/input.dat
COMMENT "Running dijkstra test"
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${DIJKSTRA_BIN_FILE}
)


