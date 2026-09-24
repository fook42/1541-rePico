/**********************************
 * header - read and write image routines
 *
 * Author: F00K42
 * Last change: 2026/02/14
 ***********************************/

#ifndef _RWROUTINES_H_
#define _RWROUTINES_H_

#include <stdint.h>
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

typedef enum {
    FILL_UP = 0,
    FILL_NONE = 1,
    FILL_DOWN = 2
} fill_direction_t;

int8_t read_disk(FIL* fd, const int image_type, FILINFO fileinfo);
int8_t write_disk(FIL* fd, const int image_type, const uint8_t num_tracks);

void convert_d64track2gcr(const uint8_t track_nr, const uint8_t image_id1, const uint8_t image_id2);
void convert_gcr2d64track(uint8_t track_nr);

size_t buffer_to_track(uint8_t* buffer, const size_t buffer_len, const uint8_t track_nr, uint8_t next_sector, uint8_t* last_sector);

int8_t fill_tracks_with_file(int8_t file_track, uint8_t* file_buffer_pointer, size_t buffer_size, const fill_direction_t direction, const uint8_t num_max_tracks, const uint8_t my_id1, const uint8_t my_id2);


#endif
