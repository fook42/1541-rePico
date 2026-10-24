/**********************************
 * header - GUI state and menu handling
 *
 * Author: F00K42
 * Last change: 2026/10/24
 ***********************************/

#ifndef _GUI_H_
#define _GUI_H_

#include <stdbool.h>
#include <stdint.h>

#include "version.h"
#include "main.h"
#include "drive.h"
#include "files.h"
#include "mymenu.h"
#include "gui_constants.h"

void show_longpress(void);
void update_gui(void);
void check_menu_events(const uint16_t menu_event);
void set_gui_mode(const uint8_t gui_mode);
void show_start_message(void);
void infomode_update(void);

#endif
