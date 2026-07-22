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
#include "c64_selector.h"
#include "c64_intro.h"

#define MAX_DIR_ENTRIES (128)

void generate_empty_image(const uint8_t image_id1, const uint8_t image_id2, const uint8_t track_number)
{
    memset(g64_jumptable,     0, sizeof(g64_jumptable));
    memset(g64_speedtable,    0, sizeof(g64_speedtable));
    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));
    memcpy(d64_track_zone, default_d64_track_zone, sizeof(default_d64_track_zone));

    for(uint8_t track_nr=0; track_nr<track_number; ++track_nr)
    {
        g64_jumptable[track_nr*2]  = (uint32_t) (G64_HEADERSIZE+track_nr*(G64_TRACKSIZE+sizeof(g64_tracklen[0])));
        g64_speedtable[track_nr*2] = (uint32_t) (g64_speedzones[d64_track_zone[track_nr]]);

        convert_d64track2gcr(track_nr, image_id1, image_id2);
    }
}

void generate_bam(const char* const image_name, const uint8_t* const image_id_buffer)
{
    uint8_t* P = &d64_sector_puffer[1];

    memset(P, 0, 256);
    P[0]=DIRECTORY_TRACK+1;  // we fill this in, when populating the directory
    P[1]=0x01;
    P[2]='A'; // DOS version = 0x41

    uint8_t j=0;
    for(uint8_t i=0x90; i<0xAB; i++)
    {
        if (0 != image_name[j])
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

void generate_directory_entry(const uint8_t* filename, const uint8_t filetype, const uint8_t des_track, const uint8_t des_sector, const uint16_t size)
{
    // assumption: d64_sector_puffer holds entire DIRECTORY_TRACK 18!!
    uint8_t*    P;
    uint8_t*    dir_sector_P = &d64_sector_puffer[1];

    dir_sector_P += D64_SECTOR_SIZE;
    P = dir_sector_P;

    while (0 != P[2])
    {
        P+=0x20;
    }  // skip existing entries

    P=&P[2]; // skip the "next-sector"-pointer

    P[0]=filetype;      // 0x82 = CBMDOS_TYPE_PRG
    P[1]=des_track+1;   // track
    P[2]=des_sector;    // sector
    uint8_t j=0;
    for(uint8_t k=3; k<19; k++)
    {
        if (0 != filename[j])
            P[k] = toupper(filename[j++]);
        else
            P[k] = 0xA0;
    }
    P[28]=(size)&0xFF;    // size low
    P[29]=(size>>8)&0xFF; // size high
}

size_t generate_menu_file(DIR* dir_obj, const uint8_t* dir_path, const uint8_t dest_track)
{
    FILINFO     fb_dir_menu_entry;

    uint8_t*    P;
    uint8_t*    file_sector_P  = g64_tracks[dest_track];
    const uint8_t*    charP = dir_path;

    char        file_extension[5]={0};

    P = file_sector_P;
    // create file-entry header
    *P++ = 0x01;
    *P++ = 0x08;

    //store current path to menu-file
    uint8_t dirname_len = strlen(dir_path);
    if (0 == dirname_len)
    {
        charP = (uint8_t*) version_str;
    } else if (38 < dirname_len)
    {
        *P++ = '.';
        *P++ = '.';
        charP = &dir_path[dirname_len-38];
    }

    while (0 != charP[0])
    {
        *P++ = *charP++;
    }
    *P++ = 0;

    if (1 < dirname_len)
    {
        *P++ = TYPE_DIR;
        *P++ = '.';
        *P++ = '.';
        *P++ = 0;
    }

    do
    {
        FRESULT fr = f_readdir(dir_obj, &fb_dir_menu_entry);
        if((0 == fb_dir_menu_entry.fname[0]) || (FR_OK != fr))
        {
            break;
        }

        if(fb_dir_menu_entry.fattrib & AM_DIR)
        {
            *P++ = TYPE_DIR;   // directory
        } else {
            size_t namelen = strlen(fb_dir_menu_entry.fname);
            if (4 > namelen)
            {
                *P++ = TYPE_UNKNOWN;// unknown file
            } else {
                strcpy(file_extension, fb_dir_menu_entry.fname+(namelen - 4));

                int i=0;
                while(0 != file_extension[i])
                {
                    file_extension[i] = tolower(file_extension[i]);
                    ++i;
                }
                if (0 == strcmp(file_extension,".d64"))
                {
                    *P++ = TYPE_D64;   // D64 file
                }
                else if (0 == strcmp(file_extension,".g64"))
                {
                    *P++ = TYPE_G64;   // G64 file
                }
                else if (0 == strcmp(file_extension,".prg"))
                {
                    *P++ = TYPE_PRG;   // PRG file
                }
                else
                {
                    *P++ = TYPE_UNKNOWN;// unknown file
                }
            }
        }

        uint8_t c = 0;
        int j = 0;
        do
        {
            c = fb_dir_menu_entry.fname[j];
            *P++ = tolower(c);
            j++;
        }
        while (0 != c);
    }
    while(true);

    return (size_t)(P-file_sector_P);
}

void create_menu_image(const char* menu_path, DIR* dir_obj, uint8_t* id1_p, uint8_t* id2_p, uint8_t* num_tracks_p, char* image_name_p)
{
    const uint8_t id_buffer[]={" F00K"};      // disk-id
    const uint8_t num_max_tracks = NUM_TRACKS_STD;
    const uint8_t my_id1 = id_buffer[0];
    const uint8_t my_id2 = id_buffer[1];

    generate_empty_image(my_id1,my_id2,num_max_tracks);

    // generates menu-file..
    size_t menu_file_len = generate_menu_file(dir_obj, menu_path, SCRATCH_TRACK);
    size_t buffer_size = menu_file_len;
    size_t buffer_left;
    int8_t file_track = MENU_DATA_TRACK, next_file_track = file_track;
    uint8_t* file_buffer_pointer = g64_tracks[SCRATCH_TRACK];
    uint8_t prev_sector = 0;
    do
    {
        buffer_left = buffer_to_track(file_buffer_pointer, buffer_size, file_track, &prev_sector);
        if (buffer_left>0)
        {
            next_file_track = file_track-1;
            file_buffer_pointer += (buffer_size-buffer_left);
            buffer_size = buffer_left;
            // last sector ?? -> update last sector-chain-pointer to new "file_track,0"...
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE]=next_file_track+1;
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE+1]=0;
        }
        convert_d64track2gcr(file_track, my_id1, my_id2);
        file_track = next_file_track;
        /* code */
    } while ((buffer_left>0) && (file_track>=0));

    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));
    for(uint8_t track_nr=SCRATCH_TRACK; track_nr<num_max_tracks; ++track_nr)
    {
        convert_d64track2gcr(track_nr, my_id1, my_id2);
    }

    // generates selector_file..
    buffer_size = menu_prg_len;
    file_track = SELECTOR_TRACK;
    next_file_track = file_track;
    file_buffer_pointer = (uint8_t*) &menu_prg[0];
    prev_sector = 0;
    do
    {
        buffer_left = buffer_to_track(file_buffer_pointer, buffer_size, file_track, &prev_sector);
        if (buffer_left>0)
        {
            next_file_track = (file_track+1)%num_max_tracks;
            file_buffer_pointer += (buffer_size-buffer_left);
            buffer_size = buffer_left;
            // last sector ?? -> update last sector-chain-pointer to new "file_track,0"...
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE]=next_file_track+1;
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE+1]=0;
        }
        convert_d64track2gcr(file_track, my_id1, my_id2);
        file_track = next_file_track;
        /* code */
    } while (buffer_left>0);

    // generates intro file..
    buffer_size = intro_prg_len;
    file_track++;   // we just take the next track after the last selector-file-track
    uint8_t intro_track = file_track;   //store for directory-creation
    file_buffer_pointer = (uint8_t*) &intro_prg[0];
    prev_sector = 0;
    do
    {
        buffer_left = buffer_to_track(file_buffer_pointer, buffer_size, file_track, &prev_sector);
        if (buffer_left>0)
        {
            next_file_track = (file_track+1)%num_max_tracks;
            file_buffer_pointer += (buffer_size-buffer_left);
            buffer_size = buffer_left;
            // last sector ?? -> update last sector-chain-pointer to new "file_track,0"...
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE]=next_file_track+1;
            d64_sector_puffer[1+prev_sector*D64_SECTOR_SIZE+1]=0;
        }
        convert_d64track2gcr(file_track, my_id1, my_id2);
        file_track = next_file_track;
        /* code */
    } while (buffer_left>0);

    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));
    generate_bam("- 1541 REPICO -", id_buffer);
    // create a file-entry in the directory...
    generate_directory_entry("SELECTOR", CBMDOS_TYPE_PRG, SELECTOR_TRACK ,0,((uint16_t) (menu_prg_len/254))+1);
    generate_directory_entry("DATAFILE", CBMDOS_TYPE_PRG, MENU_DATA_TRACK,0,((uint16_t) (menu_file_len/254))+1);
    generate_directory_entry("INTRO",    CBMDOS_TYPE_PRG, intro_track    ,0,((uint16_t) (intro_prg_len/254))+1);
    convert_d64track2gcr(DIRECTORY_TRACK, my_id1, my_id2);

    strcpy(image_name_p, "\06 ONSCREEN MENU");
    *id1_p = my_id1;
    *id2_p = my_id2;
    *num_tracks_p = num_max_tracks;
}
