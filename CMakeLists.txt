# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (MEMLUV) 

# The MemLuv version number.
SET (MEMLUV_VERSION_MAJOR 0)
SET (MEMLUV_VERSION_MINOR 1)

option(OBFUSCATOR "Generate obfuscated source code" OFF)
if(OBFUSCATOR)
  SET(SRC_PATH "src_obfs")
else(OBFUSCATOR)
  SET(SRC_PATH "src")
endif(OBFUSCATOR)

ADD_SUBDIRECTORY (${SRC_PATH})
ADD_SUBDIRECTORY (etc)
#ADD_SUBDIRECTORY (apps/playground)
ADD_SUBDIRECTORY (apps/mibench/network/dijkstra/)


option(HLS_SUPPORT "Generate Xilinx Vivado HLS targets" ON)
if(HLS_SUPPORT)
  ADD_SUBDIRECTORY (${SRC_PATH}/hls)
endif(HLS_SUPPORT)

# Set to debug build as default choice
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WITH_DEBUG ON)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)

if(BUILD_DOCUMENTATION)
  add_subdirectory(doc)
endif (BUILD_DOCUMENTATION)

message(STATUS "Debug built: " ${WITH_DEBUG})
message(STATUS "Obfuscated code: " ${OBFUSCATOR}, " (src_path: ${SRC_PATH})")
message(STATUS "Support for Doxygen documentation: " ${BUILD_DOCUMENTATION})
message(STATUS "Support for Xilinx Vivado HLS targets: " ${HLS_SUPPORT})

add_custom_target(threesome 
COMMAND make accelerators
COMMAND make test-accelerators
COMMAND make vivado_accelerators
DEPENDS ${MAIN_BIN_FILE}
)
