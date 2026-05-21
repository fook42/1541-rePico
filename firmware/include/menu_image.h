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

enum {
    TYPE_NONE = 0,
    TYPE_DIR = 1,
    TYPE_D64 = 2,
    TYPE_G64 = 3,
    TYPE_PRG = 4,
    TYPE_UNKNOWN = 255
};

void generate_empty_image(uint8_t image_id1, uint8_t image_id2, uint8_t track_number);
void generate_bam(char* image_name, uint8_t* image_id_buffer);

void generate_directory_entry(uint8_t* filename, uint8_t filetype, uint8_t des_track, uint8_t des_sector, uint16_t size);
uint16_t generate_menu_file(DIR* dir_obj, uint8_t* dir_path, const uint8_t dest_track);

#endif