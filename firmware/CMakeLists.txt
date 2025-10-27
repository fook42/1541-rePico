# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(1541-rePico C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
file(GLOB_RECURSE mysources  CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE myincludes CONFIGURE_DEPENDS "include/*.h")

add_executable(1541-rePico ${mysources} ${myincludes})

pico_set_program_name(1541-rePico "1541-rePico")
pico_set_program_version(1541-rePico "0.1")

# Generate PIO header
pico_generate_pio_header(1541-rePico ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(1541-rePico 0)
pico_enable_stdio_usb(1541-rePico 0)

add_subdirectory(no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

# Add the standard include files to the build
target_include_directories(1541-rePico PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add the standard library to the build
# Add any user requested libraries
target_link_libraries(1541-rePico 
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
        )

pico_add_extra_outputs(1541-rePico)


