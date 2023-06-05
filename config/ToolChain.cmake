cmake_minimum_required(VERSION 3.10)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

SET(CMAKE_C_COMPILER "arm-none-eabi-gcc")
SET(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
SET(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -Map <TARGET>.map -o <TARGET> --whole-archive <OBJECTS> <LINK_LIBRARIES> --no-whole-archive ${STDLIB}")
SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -Map <TARGET>.map -o <TARGET> --whole-archive <OBJECTS> <LINK_LIBRARIES> --no-whole-archive ${STDLIB}")
# SET(LINK_CMD "arm-none-eabi-ld")
SET(OBJCPY "arm-none-eabi-objcopy")
SET(CRC_SET "crc_set")

#设置私有编译参数
SET(FNO_BUILTIN_FLAGS "-fno-builtin-printf -fno-builtin-malloc -fno-builtin-free -fno-builtin-time -fno-builtin-gmtime -fno-builtin-gettimeofday -fno-builtin-mktime -fno-builtin-puts -fno-builtin-signal")
SET(CMAKE_C_FLAGS "-c -MMD -mlong-calls -mcpu=cortex-r4 -mlittle-endian -mthumb -mfloat-abi=soft -Wall -Werror -ffunction-sections -fdata-sections ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c99 -Os")
SET(CMAKE_CXX_FLAGS "-c -MMD -mlong-calls  -mcpu=cortex-r4  -mlittle-endian -mthumb -mthumb-interwork -mfloat-abi=soft -Wall -Werror -ffunction-sections -fdata-sections ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c++11 -Os")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")
