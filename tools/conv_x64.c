//////////////////////////////
// conv_x64 tool
// - converts d64/g64 to d64/g64
// - "uses" same routines as in 1541-re... project
//
// last change: 2026/02/14 - fook42


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "gcr.h"
#include "datastruct.h"
#include "rw_tracks.h"

const uint16_t d64_track_offset[MAX_TRACKS] = { 0x0000,0x0015,0x002A,0x003F,0x0054,0x0069,0x007E,0x0093,
                                                0x00A8,0x00BD,0x00D2,0x00E7,0x00FC,0x0111,0x0126,0x013B,
                                                0x0150,0x0165,0x0178,0x018B,0x019E,0x01B1,0x01C4,0x01D7,
                                                0x01EA,0x01FC,0x020E,0x0220,0x0232,0x0244,0x0256,0x0267,
                                                0x0278,0x0289,0x029A,0x02AB,0x02BC,0x02CD,0x02DE,0x02EF,
                                                0x0300,0x0311 };

const uint8_t d64_sector_count[4] = {21,    //Spuren 1-17
                                     19,    //Spuren 18-24
                                     18,    //Spuren 25-30
                                     17};   //Spuren 31-42

const uint8_t d64_track_zone[MAX_TRACKS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //Spuren 1-17
                                            1,1,1,1,1,1,1,                      //Spuren 18-24
                                            2,2,2,2,2,2,                        //Spuren 25-30
                                            3,3,3,3,3,3,3,3,3,3,3,3};           //Spuren 31-42

const uint8_t g64_head[] = {'G','C','R','-','1','5','4','1', G64_VERSION, G64_TRACKCOUNT*2, (G64_TRACKSIZE&0xff), ((G64_TRACKSIZE>>8)&0xff)};
const uint8_t g64_speedzones[] = { 3, 2, 1, 0};
const uint8_t d64_sector_gap[4] = {12, 21, 16, 13}; // von GPZ Code übernommen imggen

int main(int argc, char *argv[])
{
    char* filein_name;
    char* fileout_name;
    FILE* filein_ptr;
    FILE* fileout_ptr;
    int   err_code=0;
    int   filein_type =UNDEF_IMAGE;
    int   fileout_type=UNDEF_IMAGE;
    uint8_t max_track=0;

    memset(g64_tracks, 0, sizeof(g64_tracks));

    if(3 != argc)
    {
        printf("\nusage: %s <filein> <fileout>\n\n",argv[0]);
        printf("  supports *.d64/g64 in and *.d64/g64 out\n\n");
        return -1;
    }

    filein_name  = argv[1];
    fileout_name = argv[2];

    // reading....
    printf("File in: %s ... ", filein_name);

    filein_ptr = fopen(filein_name, "rb");

    if (NULL == filein_ptr)
    {
        printf("open error!\n");
        return -1;
    }
    printf("opened!\n");

    if (0 == strncmp(&filein_name[strlen(filein_name)-4],".d64",4))
    {
        filein_type=D64_IMAGE;
    } else if (0 == strncmp(&filein_name[strlen(filein_name)-4],".g64",4))
    {
        filein_type=G64_IMAGE;
    }

    printf("reading ... ");
    err_code = read_disk(filein_ptr, filein_type);
    fclose(filein_ptr);
    if (0<err_code)
    {
        printf("done (%d Tracks read)!\n", err_code+1);
        max_track=err_code+1;
    } else {
        printf("failed (errorcode: %d)!\n", err_code);
        return -1;
    }

    // writing....
    printf("File out: %s ... ", fileout_name);
    fileout_ptr = fopen(fileout_name, "wb");

    if (NULL == fileout_ptr)
    {
        printf("open error!\n");
        return -1;
    }
    printf("opened!\n");

    if (0 == strncmp(&fileout_name[strlen(fileout_name)-4],".d64",4))
    {
        fileout_type=D64_IMAGE;
    } else if (0 == strncmp(&fileout_name[strlen(fileout_name)-4],".g64",4))
    {
        fileout_type=G64_IMAGE;
    }

    printf("writing ... ");
    err_code = write_disk(fileout_ptr, fileout_type, max_track);
    fclose(fileout_ptr);
    if (0<err_code)
    {
        printf("done (%d Tracks written)!\n", err_code+1);
    } else {
        printf("failed (errorcode: %d)!\n", err_code);
        return -1;
    }

    return 0;
}
