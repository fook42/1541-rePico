cmake_minimum_required(VERSION 3.13)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

# Generated headers (shiftedWavData*.h) go into the build folder,
# on the include search path (see top-level CMakeLists.txt).
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

set(PCMFILE         "song_8bit.pcm")
set(WAVDATA_HEADER  "${PROJECT_INCLUDE}/wavData.h")

add_custom_target(GenWavData
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${PCMFILE}
  COMMAND echo \"\#ifndef _WAV_DATA_H_\\n\#define _WAV_DATA_H_\" > ${WAVDATA_HEADER}
  COMMAND xxd -i -n \"wav_data\" ${PCMFILE} >> ${WAVDATA_HEADER}
  COMMAND echo \"\#endif\" >> ${WAVDATA_HEADER}
  COMMAND sed -i 's/unsigned char/const unsigned char/' ${WAVDATA_HEADER}
)

# Host tool that generates the shiftedWavData*.h headers.
# It is compiled with a custom command that invokes gcc directly, so the
# Pico SDK's cross-compiler (arm-none-eabi-gcc) can never be picked up.
find_program(HOST_C_COMPILER NAMES gcc cc clang)

if(NOT HOST_C_COMPILER)
    MESSAGE(FATAL_ERROR "### No host C compiler (gcc/cc/clang) found - gen_shifted_wav can not be built! ###")
endif()

set(GEN_SHIFTED_WAV_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/gen_shifted_wav)

add_custom_command(
  OUTPUT ${GEN_SHIFTED_WAV_EXECUTABLE}
  COMMAND ${HOST_C_COMPILER} -O2 -Wall -o ${GEN_SHIFTED_WAV_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../buildtools/gen_shifted_wav.c
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../buildtools/gen_shifted_wav.c
  COMMENT "Building gen_shifted_wav (host tool, gcc)"
)

# Generate the headers; depends on the tool binary as a plain file path.
add_custom_target(GenShiftedWavData ALL
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${GEN_SHIFTED_WAV_EXECUTABLE} song_8bit.pcm 1khz_8bit.pcm 500hz_8bit.pcm step_8bit.pcm
  COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
  COMMAND ${GEN_SHIFTED_WAV_EXECUTABLE} song_8bit.pcm ${GENERATED_DIR}/shiftedWavData.h   shifted_wav_data
  COMMAND ${GEN_SHIFTED_WAV_EXECUTABLE} 1khz_8bit.pcm ${GENERATED_DIR}/shiftedWavData1.h shifted_wav_data1
  COMMAND ${GEN_SHIFTED_WAV_EXECUTABLE} 500hz_8bit.pcm ${GENERATED_DIR}/shiftedWavData2.h shifted_wav_data2
  COMMAND ${GEN_SHIFTED_WAV_EXECUTABLE} step_8bit.pcm ${GENERATED_DIR}/shiftedWavData3.h shifted_wav_data3
)
