/**********************************
 * generate menu image header
 *
 * Author: F00K42
 * Last change: 2026/05/08
 ***********************************/

#include "menu_image.h"
#include "rw_routines.h"
#include "globals.h"
#include "gcr.h"
#include "ctype.h"

#define MAX_DIR_ENTRIES (128)

void generate_empty_image(uint8_t image_id1, uint8_t image_id2)
{
    memset(g64_jumptable,     0, sizeof(g64_jumptable));
    memset(g64_speedtable,    0, sizeof(g64_speedtable));
    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));

    for(uint8_t track_nr=0; track_nr<G64_TRACKCOUNT; ++track_nr)
    {
        g64_jumptable[track_nr*2]  = (uint32_t) (G64_HEADERSIZE+track_nr*(G64_TRACKSIZE+sizeof(g64_tracklen[0])));
        g64_speedtable[track_nr*2] = (uint32_t) (g64_speedzones[d64_track_zone[track_nr]]);

        convert_d64track2gcr(track_nr, image_id1, image_id2);
    }
}

void generate_menu_bam(char* image_name, uint16_t image_id, uint16_t image_dostype)
{
    // generate "ascii"-data first... then convert the complete track to GCR

    uint8_t* P = &d64_sector_puffer[1];

    memset(P, 0, 256);
    // P[0]=DIRECTORY_TRACK+1;  // we fill this in, when populating the directory
    // P[1]=0x01;
    P[2]='A'; // DOS version = 0x41

    uint8_t j=0;
    for(uint8_t i=0x90; i<0xA0; i++)
    {
        if (image_name[j]!=0)
            P[i] = tolower(image_name[j++]);
        else
            P[i] = 0xA0;
    }
    for(uint8_t i=0xA0; i<0xAA; i++)
    {
        P[i] = 0xA0;
    }
    P[DIR_ID_OFFSET]   =  image_id    &0xFF;
    P[DIR_ID_OFFSET+1] = (image_id>>8)&0xFF;
    P[DIR_DOSTYPE_OFFSET]   =  image_dostype    &0xFF;
    P[DIR_DOSTYPE_OFFSET+1] = (image_dostype>>8)&0xFF;

}

void generate_menu_image(DIR* dir_obj)
{
    FILINFO     fb_dir_menu_entries[MAX_DIR_ENTRIES];
    FRESULT     fr;
    uint16_t    dir_sector_nr=0;
    uint8_t*    P;
    uint8_t*    dir_sector_P = &d64_sector_puffer[1];
    
    uint8_t i=0;

    while(i<MAX_DIR_ENTRIES)
    {
        if (0==(i&0x7)) // we have 8*x = i
        {
            dir_sector_nr++;
            if (d64_sector_count[d64_track_zone[DIRECTORY_TRACK]] <= dir_sector_nr)
            {
                break;
            }
            dir_sector_P[0]=DIRECTORY_TRACK+1;
            dir_sector_P[1]=dir_sector_nr;
            dir_sector_P += D64_SECTOR_SIZE;
            P = dir_sector_P;
            memset(P, 0, 256);
            P=&P[2]; // skip the "next-sector"-pointer
        }
        fr = f_readdir(dir_obj, &(fb_dir_menu_entries[i]));
        if((0 == fb_dir_menu_entries[i].fname[0]) || (FR_OK != fr))
        {
            break;
        }

        P[0]=0x82;  // PRG
        if(fb_dir_menu_entries[i].fattrib & AM_DIR)
        {
            P[0]+=0x40;  // ">" ..
        }
        P[1]=i;     // track
        P[2]=0x00;  // sector
        uint8_t j=0;
        for(uint8_t k=3; k<19; k++)
        {
            if (fb_dir_menu_entries[i].fname[j]!=0)
                P[k] = tolower(fb_dir_menu_entries[i].fname[j++]);
            else
                P[k] = 0xA0;
        }
        P[28]=0x01; // size low
        P[29]=0x00; // size high

        P = &P[32];
        i++;
    }
}

