cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME BOOT)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    bootloader
    APP/LED
    APP/Printf
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# ============================================================================
# Bootloader Target
# ============================================================================

# 创建 Bootloader 可执行文件
add_executable(bootloader.elf
    Core/Src/main.c
    Core/Src/stm32f7xx_hal_msp.c
    Core/Src/system_stm32f7xx.c
    startup_stm32f767xx.s
    bootloader/ymodem.c
    APP/LED/led.c
    APP/Printf/printf.c
)

# 设置 Bootloader 链接脚本
target_link_options(bootloader.elf PRIVATE
    -T${CMAKE_SOURCE_DIR}/bootloader/STM32F767XX_BOOTLOADER.ld
)

# 包含头文件
target_include_directories(bootloader.elf PRIVATE
    Core/Inc
    bootloader
    APP/LED
    APP/Printf
    Drivers/STM32F7xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/CMSIS/Include
)

# 编译定义
target_compile_definitions(bootloader.elf PRIVATE
    USE_HAL_DRIVER
    STM32F767xx
)

# 链接库
target_link_libraries(bootloader.elf
    stm32cubemx
)
