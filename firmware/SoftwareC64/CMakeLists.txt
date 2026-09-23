cmake_minimum_required(VERSION 3.13)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

find_program(ACME_TOOL "acme" PATHS "/usr/local/bin;/usr/bin;/bin" NO_CACHE )

option(BUILD_MENUPRG    "Enable compilation of menu.prg" ON)
option(COMPRESS_MENUPRG "Enable compression of menu.prg with dali" ON)
### TODO: COMPRESS_MENUPRG is not considered .. dali prg compression is done always for now

set(INTRO_PRG_BINARY     "intro.prg")
set(SELECTOR_PRG_BINARY  "selector.prg")
set(MENU_PAL_PRG_BINARY  "menu.prg")
set(MENU_NTSC_PRG_BINARY "menu_ntsc.prg")
set(MENU_16_PRG_BINARY   "menu_16.prg")

set(INTRO_HEADER         "${PROJECT_INCLUDE}/c64_intro.h")
set(SELECTOR_HEADER      "${PROJECT_INCLUDE}/boot_selector.h")
set(MENU_PAL_HEADER      "${PROJECT_INCLUDE}/c64_menu_pal.h")
set(MENU_NTSC_HEADER     "${PROJECT_INCLUDE}/c64_menu_ntsc.h")
set(MENU_C16_HEADER      "${PROJECT_INCLUDE}/c16_menu.h")

set(ACME_OPTIONS --format cbm --cpu 6510 -DNO_BASIC_HEADER=1)
set(DALI_OPTIONS_C64 --sfx 0x0808)
set(DALI_OPTIONS_C16 --sfx_plus4 0x1008)

if(BUILD_MENUPRG)
  if(ACME_TOOL)
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS selector.asm plus4boot.bin
      OUTPUT ${SELECTOR_PRG_BINARY}
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -o ${SELECTOR_PRG_BINARY} selector.asm
    )

    ## Menu_pal - build
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS menu.asm fastload_0140.bin fastload_e000.bin
      OUTPUT tmp_pal.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -o tmp_pal.prg menu.asm
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_pal.prg
      OUTPUT ${MENU_PAL_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_PAL_PRG_BINARY} tmp_pal.prg
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS menu.asm exos_ntsc.asm
      OUTPUT tmp_ntsc.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DNTSC=1 -o tmp_ntsc.prg menu.asm
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_ntsc.prg
      OUTPUT ${MENU_NTSC_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C64} -o ${MENU_NTSC_PRG_BINARY} tmp_ntsc.prg
    )

    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS menu.asm fastload_16_0700.bin fastload_16_f300.bin
      OUTPUT tmp_c16.prg
      COMMAND ${ACME_TOOL} ${ACME_OPTIONS} -DC16=1 -o tmp_c16.prg menu.asm
    )
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS tmp_c16.prg
      OUTPUT ${MENU_16_PRG_BINARY}
      COMMAND dali_tool ${DALI_OPTIONS_C16} -o ${MENU_16_PRG_BINARY} tmp_c16.prg
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
  DEPENDS ${MENU_PAL_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_PAL_PRG_H_\\n\#define _MENU_PAL_PRG_H_\" > ${MENU_PAL_HEADER}
  COMMAND xxd -i -n \"menu_pal_prg\" ${MENU_PAL_PRG_BINARY} >> ${MENU_PAL_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_PAL_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_PAL_HEADER}
)

add_custom_target(GenMenuC64Ntsc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_NTSC_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_NTSC_PRG_H_\\n\#define _MENU_NTSC_PRG_H_\" > ${MENU_NTSC_HEADER}
  COMMAND xxd -i -n \"menu_ntsc_prg\" ${MENU_NTSC_PRG_BINARY} >> ${MENU_NTSC_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_NTSC_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_NTSC_HEADER}
)

add_custom_target(GenMenuC16
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${MENU_16_PRG_BINARY}
  COMMAND echo \"\#ifndef _MENU_16_PRG_H_\\n\#define _MENU_16_PRG_H_\" > ${MENU_C16_HEADER}
  COMMAND xxd -i -n \"menu_16_prg\" ${MENU_16_PRG_BINARY} >> ${MENU_C16_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${MENU_C16_HEADER}
  COMMAND echo \"\#endif\" >> ${MENU_C16_HEADER}
)

add_dependencies(GenSelector    GenDali)
add_dependencies(GenMenuC64Pal  GenDali)
add_dependencies(GenMenuC64Ntsc GenDali)
add_dependencies(GenMenuC16     GenDali)

add_custom_target(GenIntro
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${INTRO_PRG_BINARY}
  COMMAND echo \"\#ifndef _INTRO_PRG_H_\\n\#define _INTRO_PRG_H_\" > ${INTRO_HEADER}
  COMMAND xxd -i -n \"intro_prg\" ${INTRO_PRG_BINARY} >> ${INTRO_HEADER}
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${INTRO_HEADER}
  COMMAND echo \"\#endif\" >> ${INTRO_HEADER}
)
