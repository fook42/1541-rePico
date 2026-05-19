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

void generate_empty_image(uint8_t image_id1, uint8_t image_id2, uint8_t track_number)
{
    memset(g64_jumptable,     0, sizeof(g64_jumptable));
    memset(g64_speedtable,    0, sizeof(g64_speedtable));
    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));

    for(uint8_t track_nr=0; track_nr<track_number; ++track_nr)
    {
        g64_jumptable[track_nr*2]  = (uint32_t) (G64_HEADERSIZE+track_nr*(G64_TRACKSIZE+sizeof(g64_tracklen[0])));
        g64_speedtable[track_nr*2] = (uint32_t) (g64_speedzones[d64_track_zone[track_nr]]);

        convert_d64track2gcr(track_nr, image_id1, image_id2);
    }
}

void generate_bam(char* image_name, uint8_t* image_id_buffer)
{
    uint8_t* P = &d64_sector_puffer[1];

    memset(P, 0, 256);
    P[0]=DIRECTORY_TRACK+1;  // we fill this in, when populating the directory
    P[1]=0x01;
    P[2]='A'; // DOS version = 0x41

    uint8_t j=0;
    for(uint8_t i=0x90; i<0xAB; i++)
    {
        if (image_name[j]!=0)
            P[i] = toupper(image_name[j++]);
        else
            P[i] = 0xA0;
    }
    P[DIR_ID_OFFSET]   = image_id_buffer[0];
    P[DIR_ID_OFFSET+1] = image_id_buffer[1];
    P[DIR_ID_OFFSET+2] = image_id_buffer[2];
    P[DIR_ID_OFFSET+3] = image_id_buffer[3];
    P[DIR_ID_OFFSET+4] = image_id_buffer[4];
}

void generate_directory_entry(uint8_t* filename, uint8_t filetype, uint8_t des_track, uint8_t des_sector, uint16_t size)
{
    // assumption: d64_sector_puffer holds entire DIRECTORY_TRACK 18!!
    uint8_t*    P;
    uint8_t*    dir_sector_P = &d64_sector_puffer[1];

    uint8_t i=0;

    dir_sector_P += D64_SECTOR_SIZE;
    P = dir_sector_P;

    while (P[2]!=0)
    {
        P+=0x20;
    }  // skip existing entries

    P=&P[2]; // skip the "next-sector"-pointer

    P[0]=filetype;      // 0x82=PRG
    P[1]=des_track+1;   // track
    P[2]=des_sector;    // sector
    uint8_t j=0;
    for(uint8_t k=3; k<19; k++)
    {
        if (filename[j]!=0)
            P[k] = toupper(filename[j++]);
        else
            P[k] = 0xA0;
    }
    P[28]=(size)&0xFF;    // size low
    P[29]=(size>>8)&0xFF; // size high
}

uint16_t generate_menu_file(DIR* dir_obj, const uint8_t dest_track, const uint8_t sector_interleave)
{
    FILINFO     fb_dir_menu_entry;
    FRESULT     fr;
    uint8_t     sector_start   = 0;
    uint8_t     file_sector_nr = 0;
    uint8_t     sector_amount  = 1;

    uint8_t*    P;
    uint8_t*    file_sector_P  = &d64_sector_puffer[1];
    const uint8_t num_of_sectors = d64_sector_count[d64_track_zone[dest_track]];
    char        file_extension[5]={0};

    uint16_t    dir_entry=0;
    uint8_t     pos = 4;    // skip track+sector & start-adr

    file_sector_P[2]=0x01;
    file_sector_P[3]=0x08;

    do
    {
        fr = f_readdir(dir_obj, &fb_dir_menu_entry);
        if((0 == fb_dir_menu_entry.fname[0]) || (FR_OK != fr))
        {
            break;
        }
        dir_entry++;

        if(fb_dir_menu_entry.fattrib & AM_DIR)
        {
            file_sector_P[pos] = TYPE_DIR;   // directory
        } else {
            size_t namelen = strlen(fb_dir_menu_entry.fname);
            strcpy(file_extension, fb_dir_menu_entry.fname+(namelen - 4));

            int i=0;
            while(file_extension[i] != '\0')
            {
                file_extension[i] = tolower(file_extension[i]);
                ++i;
            }
            if(!strcmp(file_extension,".d64"))
            {
                file_sector_P[pos] = TYPE_D64;   // D64 file
            }
            else if(!strcmp(file_extension,".g64"))
            {
                file_sector_P[pos] = TYPE_G64;   // G64 file
            }
            else if(!strcmp(file_extension,".prg"))
            {
                file_sector_P[pos] = TYPE_PRG;   // PRG file
            }
            else
            {
                file_sector_P[pos] = TYPE_UNKNOWN;// unknown file
            }
        }
        if(255 == pos) {
            sector_amount++;
            file_sector_nr = (file_sector_nr+sector_interleave)%num_of_sectors; // for interleaving and "fast reading"

            file_sector_P[0]=dest_track+1;
            file_sector_P[1]=file_sector_nr;
            file_sector_P = &d64_sector_puffer[1+file_sector_nr*D64_SECTOR_SIZE];
            memset(file_sector_P, 0, 256);
            pos=2;  // skip the sector header (track+sector or 00+length)
        } else {
            pos++;
        }


        uint8_t c = 0;
        int j;
        j = 0;
        do
        {
            c = fb_dir_menu_entry.fname[j];
            file_sector_P[pos] = tolower(c);
            if(255 == pos) {
                sector_amount++;
                file_sector_nr = (file_sector_nr+sector_interleave)%num_of_sectors; // for interleaving and "fast reading"

                file_sector_P[0]=dest_track+1;
                file_sector_P[1]=file_sector_nr;
                file_sector_P = &d64_sector_puffer[1+file_sector_nr*D64_SECTOR_SIZE];
                memset(file_sector_P, 0, 256);
                pos=2;  // skip the sector header (track+sector or 00+length)
            } else {
                pos++;
            }
            j++;
        }
        while (0 != c);
    }
    while(true);
    file_sector_P[0]=0;
    file_sector_P[1]=pos-1;

    return sector_amount;
}

