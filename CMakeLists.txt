cmake_minimum_required(VERSION 3.22)

# Point to your toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake")

# Project Name
set(CMAKE_PROJECT_NAME Reflex)
project(${CMAKE_PROJECT_NAME} LANGUAGES C CXX ASM)

# Setup compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# STM32C092 MCU Flags (Cortex-M0+, No FPU, Soft ABI)
set(MCU_FLAGS "-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -ffunction-sections -fdata-sections -flax-vector-conversions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MCU_FLAGS} -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -flax-vector-conversions")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS}") 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MCU_FLAGS} -Wl,--gc-sections -Wl,--print-memory-usage")

# Include toolchain file
# include("cmake/gcc-arm-none-eabi.cmake")

# Defines
add_compile_definitions(
    USE_FULL_LL_DRIVER
    STM32C092xx
)

# Generate firmware version info
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_TIME "%H:%M:%S")

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_HASH "Unknown")
endif()

set(CMAKE_PROJECT_VERSION "0.1.0")

# Includes
include_directories(
    # App Includes
    ${CMAKE_CURRENT_SOURCE_DIR}/Reflex/Control
    ${CMAKE_CURRENT_SOURCE_DIR}/Reflex/Messaging
    ${CMAKE_CURRENT_SOURCE_DIR}/Reflex/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Reflex/System

    # SDK Includes
    ${CMAKE_CURRENT_SOURCE_DIR}/SDK/MCU
    ${CMAKE_CURRENT_SOURCE_DIR}/SDK/Drivers

    # Middleware Includes (CMSIS & ST HAL/LL)
    ${CMAKE_CURRENT_SOURCE_DIR}/SDK/Middleware/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/SDK/Middleware/CMSIS/Device/ST/STM32C0xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/SDK/Middleware/ST/STM32C0xx_HAL_Driver/Inc
)

# Sources
file(GLOB_RECURSE APP_SOURCES 
    "Reflex/Control/*.cpp"
    "Reflex/Messaging/*.cpp"
    "Reflex/System/*.cpp"
    "Reflex/Source/*.c"
    "Reflex/Source/*.cpp"
    "Reflex/Source/*.S"
)

file(GLOB_RECURSE SDK_SOURCES
    "SDK/MCU/*.cpp"
    "SDK/Drivers/*.cpp"
)

# STM32 HAL/LL Drivers
file(GLOB ST_DRIVER_SOURCES
    "SDK/Middleware/ST/STM32C0xx_HAL_Driver/Src/*.c"
)

add_executable(${CMAKE_PROJECT_NAME}
    ${APP_SOURCES}
    ${SDK_SOURCES}
    ${ST_DRIVER_SOURCES}
)

# Linker script
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Reflex/STM32C092xx_FLASH.ld")
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE 
    -T ${LINKER_SCRIPT}
)

# Post-Build: Generate standard .bin and .hex files
set(TARGET_ELF "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf")
set(TARGET_BIN "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin")
set(TARGET_HEX "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_ELF} ${TARGET_BIN}
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET_ELF} ${TARGET_HEX}
    COMMENT "Generating Reflex binary (.bin) and intel hex (.hex)"
)