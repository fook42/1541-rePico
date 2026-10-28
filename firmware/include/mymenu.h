/**********************************
 * header - mymenu definition
 *
 * Author: F00K42
 * Last change: 2025/11/09
***********************************/

#ifndef MYMENUE_H
#define MYMENUE_H

#include "menu.h"

extern MENU_STRUCT main_menu;
extern MENU_STRUCT image_menu;
extern MENU_STRUCT settings_menu;
extern MENU_STRUCT info_menu;

enum  MENU_IDS{M_BACK, M_IMAGE, M_SETTINGS, M_INFO, \
               M_BACK_IMAGE, M_MENU_IMAGE, M_LOAD_IMAGE, M_SAVE_IMAGE, M_UNLOAD_IMAGE, M_RELOAD_DISK, M_WP_IMAGE, M_NEW_IMAGE, \
               M_BACK_SETTINGS, M_RESTART, \
               M_BACK_INFO, M_VERSION_INFO, M_SDCARD_INFO};

extern MENU_ENTRY main_menu_entrys[];
extern MENU_ENTRY image_menu_entrys[];
extern MENU_ENTRY settings_menu_entrys[];
extern MENU_ENTRY info_menu_entrys[];

#endif
