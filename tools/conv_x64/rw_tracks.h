
#ifndef _rw_tracks_h_
#define _rw_tracks_h_

#include <stdint.h>
#include <stdio.h>
#include "datastruct.h"

int8_t read_disk(FIL* fd, const int image_type);
int8_t write_disk(FIL* fd, const int image_type, const uint8_t num_tracks);

// these are mocks/forwards for linux-use
int f_read(FIL *f, void* dest, unsigned int size, unsigned int* bytes);
int f_write(FIL *f, void* dest, unsigned int size, unsigned int* bytes);
int f_close(FIL *f);
int f_lseek(FIL *f, int offset);

#endif
