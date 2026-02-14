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

int8_t read_disk(FIL* fd, const int image_type);
int8_t write_disk(FIL* fd, const int image_type, const uint8_t num_tracks);

#endif
