cmake_minimum_required(VERSION 3.10)
SET(CCMAKE_COLORS='s=39:p=220:c=207:n=196:y=46')

SET(GCC_STDLIB_TARGET_PATH thumb/v7/nofp)
SET(GCC_VERSION 9.2.1)
SET(STDLIB
" \
    ${GCC_ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/${GCC_STDLIB_TARGET_PATH}/libm.a \
    ${GCC_ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/${GCC_STDLIB_TARGET_PATH}/libc.a \
    ${GCC_ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/${GCC_STDLIB_TARGET_PATH}/libstdc++.a \
    ${GCC_ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/${GCC_STDLIB_TARGET_PATH}/libg.a \
    ${GCC_ARM_TOOLCHAIN_DIR}/arm-none-eabi/lib/${GCC_STDLIB_TARGET_PATH}/libnosys.a \
    ${GCC_ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/${GCC_VERSION}/${GCC_STDLIB_TARGET_PATH}/libgcc.a"
)

#项目名称
PROJECT(userspace)

# 项目配置
ADD_DEFINITIONS(-DHOST_NAME="${SCMODULE}")
INCLUDE("${CMAKE_SOURCE_DIR}/config/Config_APP.cmake")
INCLUDE("${KERNEL_DIR}/${SCMODULE}/KernelDefinitions.cmake")

# configure_file(
#     "${PROJECT_SOURCE_DIR}/script/userspace_config.h.in"
#     "${PROJECT_SOURCE_DIR}/script/userspace_config.h"
# )

# 预定义
if(DEFINED SDK_ADD_PRIV_PREMACRO)
	ADD_DEFINITIONS(${SDK_ADD_PRIV_PREMACRO})
endif()
if(DEFINED SDK_KERNEL_PREMACRO)
	ADD_DEFINITIONS(${SDK_KERNEL_PREMACRO})
endif()


# 编译子目录
if(NOT DEFINED SIMCOM_SDK)
    if(DEFINED QL)
        add_subdirectory(./ql_lib ql_lib)
    else()
        add_subdirectory(./sc_lib sc_lib)
    endif()
endif()

if(DEFINED QL)
    add_subdirectory(./ql_demo ql_demo)
else()
    add_subdirectory(./sc_demo sc_demo)
endif()


# 外部库列表
add_library(third_libs OBJECT IMPORTED GLOBAL)

if(DEFINED THIRD_LIB)
set_property(TARGET third_libs APPEND PROPERTY IMPORTED_OBJECTS
    ${THIRD_LIB}
)
endif()
if(DEFINED SIMCOM_SDK)
if(DEFINED QL)
set_property(TARGET third_libs APPEND PROPERTY IMPORTED_OBJECTS
    "${CMAKE_SOURCE_DIR}/ql_lib/${SCMODULE}/lib/libql_lib.a"
)
else()
set_property(TARGET third_libs APPEND PROPERTY IMPORTED_OBJECTS
    "${CMAKE_SOURCE_DIR}/sc_lib/${SCMODULE}/lib/libsc_lib.a"
)
endif()
endif()


# app源码及编译目标文件定义
# AUX_SOURCE_DIRECTORY(./ app_src)
if(DEFINED QL)
SET(app_src ql_application.c)
INCLUDE_DIRECTORIES(
    ./config
    ./ql_lib/inc
)
if(DEFINED SIMCOM_SDK)
    INCLUDE_DIRECTORIES(
        ./ql_lib/inc
        ./ql_lib/inc/lwip
        ./ql_lib/inc/lwip/arch
        ./ql_lib/inc/lwip/ipv4
        ./ql_lib/inc/lwip/ipv6
        ./ql_lib/inc/lwip/lwip
        ./ql_lib/inc/lwip/netif
    )
endif()
else()
SET(app_src sc_application.c)
INCLUDE_DIRECTORIES(
    ./config
    ./sc_lib/inc
)
if(DEFINED SIMCOM_SDK)
    INCLUDE_DIRECTORIES(
        ./sc_lib/inc
        ./sc_lib/inc/lwip
        ./sc_lib/inc/lwip/arch
        ./sc_lib/inc/lwip/ipv4
        ./sc_lib/inc/lwip/ipv6
        ./sc_lib/inc/lwip/lwip
        ./sc_lib/inc/lwip/netif
    )
endif()
endif()
add_executable(userspace ${app_src} $<TARGET_OBJECTS:third_libs>)
set_target_properties(userspace PROPERTIES SUFFIX ".elf")
set_target_properties(userspace PROPERTIES OUTPUT_NAME ${APP_NAME})


if(DEFINED QL)
if(NOT DEFINED SIMCOM_SDK)
target_link_libraries(userspace PRIVATE ql_lib)
endif()
target_link_libraries(userspace PRIVATE ql_demo)
else()
if(NOT DEFINED SIMCOM_SDK)
target_link_libraries(userspace PRIVATE sc_lib)
endif()
target_link_libraries(userspace PRIVATE sc_demo)
endif()

SET(SDK_ADD_PRIV_WRAPFLAGS
    --wrap=fopen
    --wrap=fclose
    --wrap=fread
    --wrap=fwrite
    --wrap=fseek
    --wrap=ferror
    --wrap=feof
    --wrap=ftell
    --wrap=rewind
    --wrap=clearerr
    --wrap=remove
    --wrap=rename
    --wrap=mkdir
)

TARGET_LINK_OPTIONS(userspace PRIVATE
    -gc-sections -T${LINK_SCRIP}
    -NOSTDLIB ${SDK_ADD_PRIV_WRAPFLAGS}
)


# 库文件输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


# 自定义命令
# add_custom_command(
#     TARGET userspace
#     POST_BUILD
#     DEPENDS userspace
#     COMMAND ${OBJCPY}
#     ARGS -O binary ${CMAKE_BINARY_DIR}/${APP_NAME}.elf ${CMAKE_BINARY_DIR}/${APP_NAME}.bin
# )

if(NOT DEFINED SIMCOM_SDK)
# 安装SDK包
SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})  # default is /usr/local
INSTALL(DIRECTORY config script examples
    DESTINATION ./release/${SCMODULE}
    PATTERN "*.gitignore*"  EXCLUDE
)

if("${SCMODULE}" STREQUAL "A7680C_LANS_SG")
INSTALL(DIRECTORY ../tavor/tools/linux ../tavor/tools/win32
    DESTINATION ./release/${SCMODULE}/tools
    PATTERN "../tavor/tools/win32/cmake" EXCLUDE
    PATTERN "../tavor/tools/win32/cross_tool" EXCLUDE
    PATTERN "../tavor/tools/win32/msys64" EXCLUDE
    PATTERN "../tavor/tools/win32/aboot/images/boot33.bin" EXCLUDE
)
else()
INSTALL(DIRECTORY ../tavor/tools/linux ../tavor/tools/win32
    DESTINATION ./release/${SCMODULE}/tools
    PATTERN "../tavor/tools/win32/cmake" EXCLUDE
    PATTERN "../tavor/tools/win32/cross_tool" EXCLUDE
    PATTERN "../tavor/tools/win32/msys64" EXCLUDE
)
endif()

if(DEFINED QL)
INSTALL(DIRECTORY ql_demo ql_lib
    DESTINATION ./release/${SCMODULE}
    PATTERN "ql_lib/src"  EXCLUDE
    PATTERN "*.gitignore*"  EXCLUDE
)
INSTALL(DIRECTORY ql_lib/inc ql_lib/${SCMODULE}
    DESTINATION ./release/${SCMODULE}/ql_lib
    PATTERN "ql_lib/${SCMODULE}/src"  EXCLUDE
    PATTERN "*.gitignore*"  EXCLUDE
)
INSTALL(FILES ql_application.c
    DESTINATION ./release/${SCMODULE}
)
INSTALL(TARGETS ql_lib
    ARCHIVE DESTINATION ./release/${SCMODULE}/ql_lib/${SCMODULE}/lib
)
else()
INSTALL(DIRECTORY sc_demo
    DESTINATION ./release/${SCMODULE}
    PATTERN "*.gitignore*"  EXCLUDE
)
INSTALL(DIRECTORY sc_lib/inc sc_lib/${SCMODULE}
    DESTINATION ./release/${SCMODULE}/sc_lib
    PATTERN "sc_lib/${SCMODULE}/src"  EXCLUDE
    PATTERN "*.gitignore*"  EXCLUDE
)
INSTALL(FILES sc_application.c
    DESTINATION ./release/${SCMODULE}
)
INSTALL(TARGETS sc_lib
    ARCHIVE DESTINATION ./release/${SCMODULE}/sc_lib/${SCMODULE}/lib
)
endif()

INSTALL(DIRECTORY ${KERNEL_DIR}/${SCMODULE}
    DESTINATION ./release/${SCMODULE}/kernel
    PATTERN "*.c" EXCLUDE
    PATTERN "*.h" EXCLUDE
)

INSTALL(FILES CMakeLists.txt app_build_doc.md makeDepend.mak ../Makefile
    DESTINATION ./release/${SCMODULE}
)

if("${SCMODULE}" STREQUAL "A7680C_LANS_SG")
    INSTALL(FILES ../tavor/tools/win32/aboot/images/sc_images/A7680C_LANS_SG/boot33.bin
        DESTINATION ./release/${SCMODULE}/tools/win32/aboot/images
    )
endif()

endif()
