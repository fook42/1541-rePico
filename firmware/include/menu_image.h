/**********************************
 * header - generate menu image header
 *
 * Author: F00K42
 * Last change: 2026/05/08
 ***********************************/

#ifndef _MENU_IMAGE_H
#define _MENU_IMAGE_H

#include <stdint.h>
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

void generate_empty_image(uint8_t image_id1, uint8_t image_id2);
void generate_menu_bam(char* image_name, uint8_t image_id1, uint8_t image_id2);
void generate_menu_image(DIR* dir_obj);
void convert_track2gcr(uint8_t track_nr, uint8_t image_id1, uint8_t image_id2);

#endif