# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/${SRC_PATH}/MemluvConfig.h.in"
  "${PROJECT_SOURCE_DIR}/${SRC_PATH}/MemluvConfig.h"
  )

include(CheckLibraryExists)

# add the binary tree to the search path for include files
# so that we will find MemluvConfig.h
include_directories("${PROJECT_BINARY_DIR}")
  
CHECK_LIBRARY_EXISTS(m sin "" HAVE_LIB_M)                                                                                                
                                                                                                                                         
if (HAVE_LIB_M)                                                                                                                          
    set(EXTRA_LIBS ${EXTRA_LIBS} m)                                                                                                      
endif (HAVE_LIB_M)

IF(CMAKE_COMPILER_IS_GNUCC)
  MESSAGE(STATUS "=== GCC C COMPILER DETECTED")
SET(GCC_MEMLUV_COMPILE_FLAGS "-O2 -ggdb")
SET(GCC_MEMLUV_LINK_FLAGS    "-lm")
ADD_DEFINITIONS(-std=c99 -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wconversion -Wstrict-prototypes)
ADD_DEFINITIONS( -DNO_DEFINE__SYNTHESIS__ )
ENDIF(CMAKE_COMPILER_IS_GNUCC)

SET( CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MEMLUV_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_MEMLUV_LINK_FLAGS}" )

SET( MAIN_BIN_FILE "test_memluv" )
SET( GEN_ACCELERATORS_BIN "gen_accelerators" )
SET( GEN_MEMELUV_WRAPPERS_BIN "gen_memluv_wrappers" )
SET( TEST_ACCELERATORS_BIN "test_accelerators" )

add_library (memluv memluv.c)

add_executable (${MAIN_BIN_FILE} test_memluv.c)
add_executable (${GEN_ACCELERATORS_BIN} gen_kernels/gen_accelerators.c gen_kernels/gen_regression.c 
                                        gen_kernels/gen_dfadd.c gen_kernels/gen_pca.c 
                                        gen_kernels/gen_mmul.c gen_kernels/gen_kmeans.c 
                                        gen_kernels/gen_strmatch.c  gen_kernels/gen_histogram.c
                                        ../unittest/src/gen_unittest.c)
set_target_properties(${GEN_ACCELERATORS_BIN} PROPERTIES COMPILE_FLAGS "-Wno-all -Wno-pedantic -Wno-missing-prototypes -Wno-missing-declarations -Wno-conversion")
add_executable (${GEN_MEMELUV_WRAPPERS_BIN} gen_memluv_wrappers.c)
add_executable (${TEST_ACCELERATORS_BIN} test_accelerators.c)

# Linking with memluv
target_link_libraries (${MAIN_BIN_FILE} memluv ${EXTRA_LIBS})
target_link_libraries (${TEST_ACCELERATORS_BIN} memluv ${EXTRA_LIBS})

add_custom_target(test-memluv 
COMMAND rm -f memluv_stats_heap-*.txt
COMMAND test_memluv
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(accelerators
COMMAND gen_accelerators 
#COMMAND gcc -o -std=c99 -Wall test_accelerators accelerators.c ../src/test_accelerators.c ./src/libmemluv.a -lm
#COMMAND ./test_accelerators
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${GEN_ACCELERATORS_BIN}
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(wrappers
COMMAND gen_memluv_wrappers 
COMMENT "Built HW heaps generator and generated HW heaps & MemLuv wrappers"
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${GEN_MEMELUV_WRAPPERS_BIN}
)

add_custom_target(test-accelerators
COMMAND rm -f memluv_stats_heap-*.txt
COMMAND test_accelerators 
COMMENT "Erased all HW heap log files and executing test_accelerators tb"
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${GEN_ACCELERATORS_BIN}
DEPENDS ${TEST_ACCELERATORS_BIN}
)

find_path(VALGRIND_INCLUDE_DIR valgrind.h /usr/include /usr/include/valgrind /usr/local/include /usr/local/include/valgrind)
if (VALGRIND_INCLUDE_DIR)
	message(STATUS "Building with Valgrind support")
	set(CTEST_MEMORYCHECK_COMMAND, "valgrind --tool=memcheck")
	set(CTEST_MEMORYCHECK_COMMAND_OPTIONS, "--leak-check=full")
	add_definitions("-DUSE_VALGRIND=1")
else (VALGRIND_INCLUDE_DIR)
        message(STATUS "Valgrind headers not found -- disabling valgrind support")
endif(VALGRIND_INCLUDE_DIR)


add_custom_target(memcheck 
COMMAND ${CTEST_MEMORYCHECK_COMMAND} ${CTEST_MEMORYCHECK_COMMAND_OPTIONS} ./test_memluv
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src
DEPENDS ${MAIN_BIN_FILE}
)

#add_dependencies(run test_memluv)
