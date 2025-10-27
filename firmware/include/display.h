/* general routines for display */
// implementation: F00K42
// last change: 11/10/2025

#ifndef _DISPLAY_H_INCLUDE_
#define _DISPLAY_H_INCLUDE_

#include "oled.h"
#include "lcd.h"

enum {
    display_none_char = 0,
    display_more_top_char = 1,
    display_more_down_char,
    display_cursor_char,
    display_dir_char,
    display_disk_char
 };


#ifndef _EXTERN_
#define _EXTERN_ extern
#endif

// function pointers to be used later
_EXTERN_ void (*display_setup)(void);
_EXTERN_ void (*display_clear)(void);
_EXTERN_ void (*display_home)(void);
_EXTERN_ void (*display_setcursor)(uint8_t, uint8_t);
_EXTERN_ void (*display_data)(uint8_t);
_EXTERN_ void (*display_string)(const char*);
_EXTERN_ void (*display_print)(char*, uint8_t, uint8_t);
_EXTERN_ void (*display_generatechar)(uint8_t, const uint8_t*);

// detect the display type & set it up
uint8_t display_init(void);

#endif // _DISPLAY_H_INCLUDE_
