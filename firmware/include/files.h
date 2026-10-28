/**********************************
 * header - file browser and image handling
 *
 * Author: F00K42
 * Last change: 2026/10/28
 ***********************************/

#ifndef _FILES_H_
#define _FILES_H_

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "menu.h"
#include "mymenu.h"
#include "display.h"
#include "gui_constants.h"
#include "menu_image.h"
#include "rw_routines.h"

extern void (*display_clear)(void);
extern void (*display_home)(void);
extern void (*display_setcursor)(const uint8_t column, const uint8_t row);
extern void (*display_data)(const uint8_t character);
extern void (*display_string)(const char* char_array);
extern void (*display_print)(const char* char_array, const uint8_t array_offset, const uint8_t print_length);
extern void (*display_setinverse)(bool inverse);

extern MENU_STRUCT image_menu;
extern uint16_t selected_image_nr;
extern uint8_t num_max_tracks;
extern bool floppy_wp;
extern bool is_image_mount;
extern volatile bool track_is_written;
extern volatile uint8_t akt_half_track;
extern uint8_t selected_track;
extern char current_path[512];
extern char image_filename[256];
extern FIL fd;
extern FATFS fs;
extern DIR dir_object;
extern FILINFO fb_dir_entry[];

FRESULT mount_sdcard(const char* mount_path);
FRESULT umount_sdcard(void);
void show_fs_error(FRESULT error_code);

void handle_menu_image(void);
void insert_menu_image(char* menu_path, const uint8_t max_blocks);

void filebrowser_update(uint8_t key_code);
void filebrowser_refresh(void);

uint16_t get_dir_entry_count(const char* entrycount_path);
uint16_t seek_to_dir_entry(uint16_t entry_num, const char* seek_path);

uint8_t open_dir_entry(FILINFO od_file_entry);
uint8_t open_disk_image(FIL* fd, FILINFO *file_entry);
void close_disk_image(FIL* fd);

#endif
