cmake_minimum_required(VERSION 3.10)

AUX_SOURCE_DIRECTORY(./src sc_demo_src)
AUX_SOURCE_DIRECTORY(./src/token sc_demo_src)
AUX_SOURCE_DIRECTORY(./src/utils sc_demo_src)


# Add the static library
add_library(sc_demo STATIC ${sc_demo_src})
target_include_directories(sc_demo PUBLIC
    ./inc
    ${CMAKE_SOURCE_DIR}/sc_lib/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/${SCMODULE}/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/GPIO
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/token
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/utils
    ${CMAKE_SOURCE_DIR}/sc_demo/inc
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
