/* mymenu definition */
// last change: 09/11/2025

#ifndef MYMENUE_H
#define MYMENUE_H

#include "menu.h"

MENU_STRUCT main_menu;
MENU_STRUCT image_menu;
MENU_STRUCT settings_menu;
MENU_STRUCT info_menu;

enum  MENU_IDS{M_BACK, M_IMAGE, M_SETTINGS, M_INFO, \
               M_BACK_IMAGE, M_LOAD_IMAGE, M_UNLOAD_IMAGE, M_WP_IMAGE, M_NEW_IMAGE, M_SAVE_IMAGE, \
               M_BACK_SETTINGS, M_RESTART, \
               M_BACK_INFO, M_VERSION_INFO, M_SDCARD_INFO};

/// Hauptmenü
MENU_ENTRY main_menu_entrys[] = {{"Disk Menu", M_IMAGE,ENTRY_MENU,   0,&image_menu},
                                 {"Settings",  M_SETTINGS,ENTRY_MENU,0,&settings_menu},
                                 {"Info",      M_INFO,ENTRY_MENU,    0,&info_menu}};
/// Image Menü
MENU_ENTRY image_menu_entrys[] = {{"Load Image",   M_LOAD_IMAGE},
                                  {"Save Image",   M_SAVE_IMAGE},
                                  {"Unload Image", M_UNLOAD_IMAGE},
                                  {"WriteProt.",   M_WP_IMAGE, ENTRY_ONOFF, 1},
                                  {"New Image",    M_NEW_IMAGE}};
/// Settings Menü
MENU_ENTRY settings_menu_entrys[] = {{"Restart",M_RESTART}};
/// Info Menü
MENU_ENTRY info_menu_entrys[] = {{"Version",     M_VERSION_INFO},
                                 {"SD Card Info",M_SDCARD_INFO}};



#endif
