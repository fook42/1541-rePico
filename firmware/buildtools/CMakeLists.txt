cmake_minimum_required(VERSION 3.21)

### build dali-packer ...
find_program(ACME_TOOL "acme" PATHS "/usr/local/bin;/usr/bin;/bin" NO_CACHE )

set(DALI_WORKDIR ${CMAKE_CURRENT_SOURCE_DIR}/bitfire)
set(DALI_EXECUTABLE ${DALI_WORKDIR}/packer/dali/dali)

if(ACME_TOOL)
      add_custom_command(
            WORKING_DIRECTORY ${DALI_WORKDIR}
            OUTPUT ${DALI_EXECUTABLE}
            COMMAND make dali
      )
else()
      MESSAGE(FATAL_ERROR "### ACME compiler not found - Dali packer can not be created!!! ###")
endif()


add_custom_target(GenDali ALL
      DEPENDS ${DALI_EXECUTABLE})

add_executable(dali_tool IMPORTED GLOBAL)

set_property(TARGET dali_tool PROPERTY IMPORTED_LOCATION ${DALI_EXECUTABLE} )

### build tok64 ...

set(TOK64_WORKDIR ${CMAKE_CURRENT_SOURCE_DIR}/tok64)
set(TOK64_EXECUTABLE ${TOK64_WORKDIR}/tok64)

add_custom_command(
	WORKING_DIRECTORY ${TOK64_WORKDIR}
	OUTPUT ${TOK64_EXECUTABLE}
	COMMAND make tok64
)

add_custom_target(GenTok64 ALL
      DEPENDS ${TOK64_EXECUTABLE})

add_executable(tok64_tool IMPORTED GLOBAL)

set_property(TARGET tok64_tool PROPERTY IMPORTED_LOCATION ${TOK64_EXECUTABLE} )

### build gen_shifted_wav ...
# Host tool (built with the system compiler, NOT the cross-compiler):
# generates shiftedWavData*.h headers from 8-bit PCM files for the firmware.
# It is compiled via a custom command that invokes gcc directly, so the
# Pico SDK's cross-compiler (arm-none-eabi-gcc) can never be picked up.
find_program(HOST_C_COMPILER NAMES gcc cc clang)

if(NOT HOST_C_COMPILER)
    MESSAGE(FATAL_ERROR "### No host C compiler (gcc/cc/clang) found - gen_shifted_wav can not be built! ###")
endif()

set(GEN_SHIFTED_WAV_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/gen_shifted_wav)

add_custom_command(
        OUTPUT ${GEN_SHIFTED_WAV_EXECUTABLE}
        COMMAND ${HOST_C_COMPILER} -O2 -Wall -o ${GEN_SHIFTED_WAV_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_shifted_wav.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen_shifted_wav.c
)

add_custom_target(GenShiftedWav ALL
        DEPENDS ${GEN_SHIFTED_WAV_EXECUTABLE})

add_executable(gen_shifted_wav_tool IMPORTED GLOBAL)

set_property(TARGET gen_shifted_wav_tool PROPERTY IMPORTED_LOCATION ${GEN_SHIFTED_WAV_EXECUTABLE} )

