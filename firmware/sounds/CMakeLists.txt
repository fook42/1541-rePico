cmake_minimum_required(VERSION 3.13)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

## option(SWAPWAV      "Enable swapping of upper and lower 16bit of wav file" ON)

set(PCMFILE         "testsound_8bit.pcm")
set(WAVDATA_HEADER  "${PROJECT_INCLUDE}/wavData.h")

add_custom_target(GenWavData
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${PCMFILE}
  COMMAND echo \"\#ifndef _WAV_DATA_H_\\n\#define _WAV_DATA_H_\" > ${WAVDATA_HEADER}
  COMMAND xxd -i -n \"wav_data\" ${PCMFILE} >> ${WAVDATA_HEADER}
  COMMAND echo \"\#endif\" >> ${WAVDATA_HEADER}
)

##  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${WAVDATA_HEADER}

## if(SWAPWAV)
## sed -i 's!\( 0x[0-9a-f]*\),\( 0x[0-9a-f]*\),\( 0x[0-9a-f]*\),\( 0x[0-9a-f]*\)!\3,\4,\1,\2!g' $outfile
## endif()