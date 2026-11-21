cmake_minimum_required(VERSION 3.21)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

find_program(ACME_TOOL   "acme"   PATHS "/usr/local/bin;/usr/bin;/bin" NO_CACHE )

option(BUILD_MENUPRG     "Enable compilation of menu.prg" ON)
option(COMPRESS_MENUPRG  "Enable compression of menu.prg with dali" ON)
option(BUILD_SELECTORPRG "Enable creation of selector.prg" ON)

### TODO: COMPRESS_MENUPRG is not considered .. dali prg compression is done always for now

set(SELECTOR_PRG_SOURCE      "selector.bas")
set(MENU_PRG_SOURCE          "menu.asm")

set(INTRO_PRG_BINARY         "${CMAKE_CURRENT_SOURCE_DIR}/intro.prg")
set(SELECTOR_PRG_BINARY      "${CMAKE_CURRENT_SOURCE_DIR}/selector.prg")
set(MENU_C64_PRG_BINARY      "${CMAKE_CURRENT_SOURCE_DIR}/menu.prg")
set(MENU_C64_DTV_PRG_BINARY  "${CMAKE_CURRENT_SOURCE_DIR}/menu_dtv.prg")
set(MENU_C16_PRG_BINARY      "${CMAKE_CURRENT_SOURCE_DIR}/menu_16.prg")
set(MENU_P4_PRG_BINARY       "${CMAKE_CURRENT_SOURCE_DIR}/menu_p4.prg")
set(MENU_VIC20_PRG_BINARY    "${CMAKE_CURRENT_SOURCE_DIR}/menu_20.prg")
set(FE3LOADER_PRG_BINARY     "${CMAKE_CURRENT_SOURCE_DIR}/fe3loader.prg")

set(TMP_MENU_C64_PRG         "${CMAKE_CURRENT_SOURCE_DIR}/tmp.prg")
set(TMP_MENU_C64_DTV_PRG     "${CMAKE_CURRENT_SOURCE_DIR}/tmp_dtv.prg")
set(TMP_MENU_C16_PRG         "${CMAKE_CURRENT_SOURCE_DIR}/tmp_c16.prg")
set(TMP_MENU_P4_PRG          "${CMAKE_CURRENT_SOURCE_DIR}/tmp_p4.prg")

set(INTRO_HEADER         "${PROJECT_INCLUDE}/c64_intro.h")
set(SELECTOR_HEADER      "${PROJECT_INCLUDE}/boot_selector.h")
set(MENU_C64_HEADER      "${PROJECT_INCLUDE}/c64_menu.h")
set(MENU_C64_DTV_HEADER  "${PROJECT_INCLUDE}/c64_menu_dtv.h")
set(MENU_C16_HEADER      "${PROJECT_INCLUDE}/c16_menu.h")
set(MENU_P4_HEADER       "${PROJECT_INCLUDE}/p4_menu.h")
set(MENU_VIC20_HEADER    "${PROJECT_INCLUDE}/vic20_menu.h")
set(FE3LOADER_HEADER     "${PROJECT_INCLUDE}/fe3loader.h")

set(ACME_OPTIONS --format cbm --cpu 6510 -DNO_BASIC_HEADER=1)
set(DALI_OPTIONS_C64 --sfx 0x0808)
set(DALI_OPTIONS_C16 --sfx_plus4 0x1008)

if(BUILD_SELECTORPRG)
  add_custom_command(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${SELECTOR_PRG_SOURCE}
    OUTPUT ${SELECTOR_PRG_BINARY}
    COMMAND tok64_tool /toprg /stomp ${SELECTOR_PRG_SOURCE}
  )
else()
  MESSAGE("### using precompiled selector prg ###")
endif()

if(BUILD_MENUPRG)
  if(ACME_TOOL)
    ## Menu C64 - build
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} exos.asm
      OUTPUT ${TMP_MENU_C64_PRG}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -o ${TMP_MENU_C64_PRG} ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${TMP_MENU_C64_PRG}
      OUTPUT ${MENU_C64_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_C64_PRG_BINARY} ${TMP_MENU_C64_PRG}
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} fastload_0140.bin fastload_e000.bin
      OUTPUT ${TMP_MENU_C64_DTV_PRG}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DDTV=1 -o ${TMP_MENU_C64_DTV_PRG} ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${TMP_MENU_C64_DTV_PRG}
      OUTPUT ${MENU_C64_DTV_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_C64_DTV_PRG_BINARY} ${TMP_MENU_C64_DTV_PRG}
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE}
      OUTPUT ${TMP_MENU_C16_PRG}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DC16=1 -o ${TMP_MENU_C16_PRG} ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${TMP_MENU_C16_PRG}
      OUTPUT ${MENU_C16_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C16} -o ${MENU_C16_PRG_BINARY} ${TMP_MENU_C16_PRG}
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} fastload_16_0700.bin fastload_16_f300.bin
      OUTPUT ${TMP_MENU_P4_PRG}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DPLUS4=1 -o ${TMP_MENU_P4_PRG} ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${TMP_MENU_P4_PRG}
      OUTPUT ${MENU_P4_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C16} -o ${MENU_P4_PRG_BINARY} ${TMP_MENU_P4_PRG}
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} vic20loader.asm
      OUTPUT ${MENU_VIC20_PRG_BINARY}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DVIC20=1 -o ${MENU_VIC20_PRG_BINARY} ${MENU_PRG_SOURCE}
    )
  else()
    MESSAGE("### ACME not found - using precompiled prgs ###")
  endif()
else()
  MESSAGE("### using precompiled prgs ###")
endif()

add_custom_target(GenSelector
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${SELECTOR_PRG_BINARY}
  COMMAND echo \"\#ifndef _SELECTOR_PRG_H_\\n\#define _SELECTOR_PRG_H_\" > ${SELECTOR_HEADER}
  COMMAND xxd -i -n \"selector_prg\" ${SELECTOR_PRG_BINARY} >> ${SELECTOR_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${SELECTOR_HEADER}
  COMMAND echo \"\#endif\" >> ${SELECTOR_HEADER}
)

add_custom_target(GenMenuC64
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_C64_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_C64_PRG_H_\\n\#define _MENU_C64_PRG_H_\" > ${MENU_C64_HEADER}
  COMMAND xxd -i -n \"menu_c64_prg\" ${MENU_C64_PRG_BINARY} >> ${MENU_C64_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C64_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C64_HEADER}
)

add_custom_target(GenMenuC64Dtv
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_C64_DTV_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_C64_DTV_PRG_H_\\n\#define _MENU_C64_DTV_PRG_H_\" > ${MENU_C64_DTV_HEADER}
  COMMAND xxd -i -n \"menu_c64_dtv_prg\" ${MENU_C64_DTV_PRG_BINARY} >> ${MENU_C64_DTV_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C64_DTV_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C64_DTV_HEADER}
)

add_custom_target(GenMenuC16
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_C16_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_C16_PRG_H_\\n\#define _MENU_C16_PRG_H_\" > ${MENU_C16_HEADER}
  COMMAND xxd -i -n \"menu_c16_prg\" ${MENU_C16_PRG_BINARY} >> ${MENU_C16_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C16_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C16_HEADER}
)

add_custom_target(GenMenuP4
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_P4_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_P4_PRG_H_\\n\#define _MENU_P4_PRG_H_\" > ${MENU_P4_HEADER}
  COMMAND xxd -i -n \"menu_p4_prg\" ${MENU_P4_PRG_BINARY} >> ${MENU_P4_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_P4_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_P4_HEADER}
)

add_custom_target(GenMenuVIC20
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_VIC20_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_VIC20_PRG_H_\\n\#define _MENU_VIC20_PRG_H_\" > ${MENU_VIC20_HEADER}
  COMMAND xxd -i -n \"menu_vic20_prg\" ${MENU_VIC20_PRG_BINARY} >> ${MENU_VIC20_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_VIC20_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_VIC20_HEADER}
)

add_dependencies(GenSelector    GenTok64)
add_dependencies(GenMenuC64     GenDali)
add_dependencies(GenMenuC64Dtv  GenDali)
add_dependencies(GenMenuC16     GenDali)
add_dependencies(GenMenuP4      GenDali)

add_custom_target(GenIntro
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${INTRO_PRG_BINARY}
  COMMAND echo \"\#ifndef _INTRO_PRG_H_\\n\#define _INTRO_PRG_H_\" > ${INTRO_HEADER}
  COMMAND xxd -i -n \"intro_prg\" ${INTRO_PRG_BINARY} >> ${INTRO_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${INTRO_HEADER}
  COMMAND echo \"\#endif\" >> ${INTRO_HEADER}
)

add_custom_target(GenFe3loader
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${FE3LOADER_PRG_BINARY}
  COMMAND echo \"\#ifndef _FE3LOADER_PRG_H_\\n\#define _FE3LOADER_PRG_H_\" > ${FE3LOADER_HEADER}
  COMMAND xxd -i -n \"fe3loader_prg\" ${FE3LOADER_PRG_BINARY} >> ${FE3LOADER_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${FE3LOADER_HEADER}
  COMMAND echo \"\#endif\" >> ${FE3LOADER_HEADER}
)

