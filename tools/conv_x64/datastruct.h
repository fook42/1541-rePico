//
//
#include <stdint.h>

#ifndef _datastruct_h_
#define _datastruct_h_

#define MAX_TRACKS      (42)
#define DIRECTORY_TRACK (17)
#define DIR_ID_OFFSET   (0xA2)


extern const uint16_t d64_track_offset[MAX_TRACKS];
extern const uint8_t  d64_sector_count[4];
extern const uint8_t  d64_track_zone[MAX_TRACKS];

#define D64_SECTOR_SIZE (256)
uint8_t d64_sector_puffer[21*D64_SECTOR_SIZE+5];

#define GCR_SYNCMARK    (0xFF)

#define G64_VERSION     (0x00)
#define G64_TRACKCOUNT  MAX_TRACKS
#define G64_TRACKSIZE   (7928)  // = 0x1ef8

extern const uint8_t g64_head[12];
uint32_t g64_jumptable[2*G64_TRACKCOUNT];
uint32_t g64_speedtable[2*G64_TRACKCOUNT];

extern const uint8_t g64_speedzones[];

#define G64_HEADERSIZE (sizeof(g64_head)+sizeof(g64_jumptable)+sizeof(g64_speedtable)) // should be 684 bytes

uint16_t g64_tracklen[G64_TRACKCOUNT];

// enough space to store 42 tracks, each 7928 bytes byte length
uint8_t g64_tracks[G64_TRACKCOUNT][G64_TRACKSIZE];

enum {UNDEF_IMAGE, G64_IMAGE, D64_IMAGE};

enum {FR_OK, FR_FAIL};

extern const uint8_t d64_sector_gap[4];
#define HEADER_GAP_BYTES (9)


////////////////////////////////////////////7
#define FIL FILE
#define UINT unsigned int

#endif
