/**********************************
 * header - generate menu image header
 *
 * Author: F00K42
 * Last change: 2026/09/23
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
    TYPE_T64 = 5,
    TYPE_VALID = 255,
    TYPE_UNKNOWN = 255
};

#define T64_HEADER_SIZE    (64)
#define T64_CONTNAME_SIZE  (24)
#define T64_ENTRY_SIZE     (32)
#define T64_ENTRYNAME_SIZE (16)

void generate_empty_image(const uint8_t image_id1, const uint8_t image_id2, const uint8_t track_number);
void generate_bam(const char* const image_name, const uint8_t* const image_id_buffer);

void generate_directory_entry(const uint8_t* filename, const uint8_t filetype, const uint8_t des_track, const uint8_t des_sector, const uint16_t size);
size_t generate_menu_file(DIR* dir_obj, const uint8_t* dir_path, const uint8_t dest_track, const uint8_t max_blocks);

void create_menu_image(const char* menu_path, DIR* dir_obj, uint8_t* id1_p, uint8_t* id2_p, uint8_t* num_tracks_p, char* image_name_p, const uint8_t datafile_limit);
int create_T64_menu_image(FIL* t64_fd, uint8_t* id1_p, uint8_t* id2_p, uint8_t* num_tracks_p, char* image_name_p);

#endif