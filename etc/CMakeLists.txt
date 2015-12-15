add_custom_target(gnuplot 
COMMENT "Plotting with gnuplot"
COMMAND gnuplot plot1.gnu
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/etc/gnuplot
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(pyplot 
COMMENT "Plotting with pyplot"
COMMAND python plot.py
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/etc/pyplot
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(measurements 
COMMENT "Collecting measurements"
COMMAND python3 sta4.py
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/etc/measurements
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(mtrace 
COMMENT "Parsing mtrace log data"
COMMAND python3 parse_mtrace.py
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/etc/mtrace
DEPENDS ${MAIN_BIN_FILE}
)

if(OBFUSCATOR)
ADD_SUBDIRECTORY (obfs)
endif(OBFUSCATOR)

