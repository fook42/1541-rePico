cmake_minimum_required(VERSION 3.13)

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)

find_program(ACME_BINARY "acme" PATHS "/usr/local/bin;/usr/bin;/bin" NO_CACHE )

option(BUILD_MENUPRG "Enable compiling of menu.prg" ON)

if(BUILD_MENUPRG)
  if(ACME_BINARY)
    add_custom_command(
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS menu.asm
      OUTPUT menu.prg
      COMMAND ${ACME_BINARY} menu.asm
    )
  else()
    MESSAGE("### ACME not found - using precompiled menu.prg ###")
  endif()
else()
  MESSAGE("### using precompiled menu.prg ###")
endif()

add_custom_target(GenSelector
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS menu.prg
  COMMAND echo \"\#ifndef _MENU_PRG_H_\\n\#define _MENU_PRG_H_\" > ${PROJECT_INCLUDE}/c64_selector.h
  COMMAND xxd -i -n \"menu_prg\" menu.prg >> ${PROJECT_INCLUDE}/c64_selector.h
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${PROJECT_INCLUDE}/c64_selector.h
  COMMAND echo \"\#endif\" >> ${PROJECT_INCLUDE}/c64_selector.h
)
add_custom_target(GenIntro
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND echo \"\#ifndef _INTRO_PRG_H_\\n\#define _INTRO_PRG_H_\" > ${PROJECT_INCLUDE}/c64_intro.h
  COMMAND xxd -i -n \"intro_prg\" intro.prg >> ${PROJECT_INCLUDE}/c64_intro.h
  COMMAND sed -i \"s!unsigned!const unsigned!g\" ${PROJECT_INCLUDE}/c64_intro.h
  COMMAND echo \"\#endif\" >> ${PROJECT_INCLUDE}/c64_intro.h
)
