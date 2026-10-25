cmake_minimum_required(VERSION 3.13)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

find_program(ACME_TOOL   "acme"   PATHS "/usr/local/bin;/usr/bin;/bin" NO_CACHE )

option(BUILD_MENUPRG     "Enable compilation of menu.prg" ON)
option(COMPRESS_MENUPRG  "Enable compression of menu.prg with dali" ON)
option(BUILD_SELECTORPRG "Enable creation of selector.prg" ON)

### TODO: COMPRESS_MENUPRG is not considered .. dali prg compression is done always for now

set(SELECTOR_PRG_SOURCE      "selector.bas")
set(MENU_PRG_SOURCE          "menu.asm")

set(INTRO_PRG_BINARY         "intro.prg")
set(SELECTOR_PRG_BINARY      "selector.prg")
set(MENU_C64_PAL_PRG_BINARY  "menu.prg")
set(MENU_C64_NTSC_PRG_BINARY "menu_ntsc.prg")
set(MENU_C16_PRG_BINARY      "menu_16.prg")
set(MENU_P4_PRG_BINARY       "menu_p4.prg")
set(MENU_VIC20_PRG_BINARY    "menu_20.prg")
set(FE3LOADER_PRG_BINARY     "fe3loader.prg")

set(INTRO_HEADER         "${PROJECT_INCLUDE}/c64_intro.h")
set(SELECTOR_HEADER      "${PROJECT_INCLUDE}/boot_selector.h")
set(MENU_C64_PAL_HEADER  "${PROJECT_INCLUDE}/c64_menu_pal.h")
set(MENU_C64_NTSC_HEADER "${PROJECT_INCLUDE}/c64_menu_ntsc.h")
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
    ## Menu_pal - build
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} fastload_0140.bin fastload_e000.bin
      OUTPUT tmp_pal.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -o tmp_pal.prg ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_pal.prg
      OUTPUT ${MENU_C64_PAL_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_C64_PAL_PRG_BINARY} tmp_pal.prg
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} exos_ntsc.asm
      OUTPUT tmp_ntsc.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DNTSC=1 -o tmp_ntsc.prg ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_ntsc.prg
      OUTPUT ${MENU_C64_NTSC_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_C64_NTSC_PRG_BINARY} tmp_ntsc.prg
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE}
      OUTPUT tmp_c16.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DC16=1 -o tmp_c16.prg ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_c16.prg
      OUTPUT ${MENU_C16_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C16} -o ${MENU_C16_PRG_BINARY} tmp_c16.prg
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${MENU_PRG_SOURCE} fastload_16_0700.bin fastload_16_f300.bin
      OUTPUT tmp_p4.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DPLUS4=1 -o tmp_p4.prg ${MENU_PRG_SOURCE}
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_p4.prg
      OUTPUT ${MENU_P4_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C16} -o ${MENU_P4_PRG_BINARY} tmp_p4.prg
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

add_custom_target(GenMenuC64Pal
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_C64_PAL_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_C64_PAL_PRG_H_\\n\#define _MENU_C64_PAL_PRG_H_\" > ${MENU_C64_PAL_HEADER}
  COMMAND xxd -i -n \"menu_c64_pal_prg\" ${MENU_C64_PAL_PRG_BINARY} >> ${MENU_C64_PAL_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C64_PAL_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C64_PAL_HEADER}
)

add_custom_target(GenMenuC64Ntsc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_C64_NTSC_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_C64_NTSC_PRG_H_\\n\#define _MENU_C64_NTSC_PRG_H_\" > ${MENU_C64_NTSC_HEADER}
  COMMAND xxd -i -n \"menu_c64_ntsc_prg\" ${MENU_C64_NTSC_PRG_BINARY} >> ${MENU_C64_NTSC_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C64_NTSC_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C64_NTSC_HEADER}
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
add_dependencies(GenMenuC64Pal  GenDali)
add_dependencies(GenMenuC64Ntsc GenDali)
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

