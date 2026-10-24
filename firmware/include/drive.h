/**********************************
 * header - drive routines and control functions
 *
 * Author: F00K42
 * Last change: 2026/10/24
 ***********************************/

#ifndef _DRIVE_H_
#define _DRIVE_H_

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "menu.h"
#include "lcd.h"
#include "display.h"
#include "gui_constants.h"
#include "pinout.h"
#include "globals.h"

extern void (*display_clear)(void);
extern void (*display_home)(void);
extern void (*display_setcursor)(const uint8_t column, const uint8_t row);
extern void (*display_string)(const char* char_array);
extern void (*display_print)(const char* char_array, const uint8_t array_offset, const uint8_t print_length);

extern MENU_STRUCT image_menu;

// drive state and control functions
int64_t steppertimer_callback(alarm_id_t id, void *user_data);
void start_stepper_timer(void);
void check_stepper_signals(void);
void send_disk_change(void);
void init_writeprot(void);
void unmount_image(void);
void show_sdcard_info_message(void);

void init_stepper(void);
void stepper_inc(void);
void stepper_dec(void);
void init_motor(void);
void init_control_signals(void);
void init_soe_gatearray(void);
void init_bytetimer(void);
void start_bytetimer(uint8_t half_track);
void stop_bytetimer(void);
void init_sound(void);

bool repeating_timer_callback(__unused struct repeating_timer *t);

#endif
