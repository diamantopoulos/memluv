
add_custom_target(obfuscate 
COMMENT "Obfuscating memluv code"
COMMAND python3 obfs.py
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/etc/obfs
DEPENDS ${MAIN_BIN_FILE}
)

add_custom_target(get_obfuscator
COMMENT "Downloading Stunnix Obfuscator"
COMMAND wget http://www.stunnix.com/pad/trial-nomail/cxxo/Stunnix-CXX-Obfus-4.1-Linux-trial.tar.gz
COMMAND mkdir Stunnix-CXX-Obfus-4.1-Linux-trial
COMMAND tar xzf Stunnix-CXX-Obfus-4.1-Linux-trial.tar.gz -C Stunnix-CXX-Obfus-4.1-Linux-trial
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/contrib
)
