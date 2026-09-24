/**********************************
 * generate menu image header
 *
 * Author: F00K42
 * Last change: 2026/09/23
 ***********************************/

#include "menu_image.h"
#include "rw_routines.h"
#include "globals.h"
#include "gcr.h"
#include "ctype.h"
#include "c64_intro.h"
#include "boot_selector.h"
#include "c64_menu_pal.h"
#include "c64_menu_ntsc.h"
#include "c16_menu.h"

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
    if (2 > dirname_len)
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
    /* create this disklayout

        T01  ====== upper limit
        T17  ^\_start DATAFILE (MENU_DATA_FILE) - start: MENU_DATA_TRACK fixed

        T18  DIRECTORY TRACK

        T19  v/ start SELECTOR                  - start: SELECTOR_TRACK fixed
        ..   ====
        Txx  v/ start MENU_PAL_C64              - start: C64 Pal MenuPRG variable
        ..   ====
        Txx  v/ start MENU_C16                  - start: C16 MenuPRG variable
        ..   ====
        Txx  v/ start MENU_NTSC_C64             - start: C64 Ntsc MenuPRG variable
        ..   ====

        Txx  v/ start INTRO                     - start: intro_track variable
        ..   |
        T35  ====== lower limit
    
    */

    const uint8_t id_buffer[]={" F00K"};            // disk-id
    const uint8_t num_max_tracks = NUM_TRACKS_STD;  // 35 Tracks should be enough
    const uint8_t my_id1 = id_buffer[0];
    const uint8_t my_id2 = id_buffer[1];

    generate_empty_image(my_id1,my_id2,num_max_tracks);

    // generates menu-datafile..
    size_t menu_file_len = generate_menu_file(dir_obj, menu_path, SCRATCH_TRACK);

    (void) fill_tracks_with_file(MENU_DATA_TRACK, g64_tracks[SCRATCH_TRACK], menu_file_len, FILL_UP, num_max_tracks, my_id1, my_id2);

    // need to clean out temporary data from "generate_menu_file" in track-buffer memory
    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));
    for(uint8_t track_nr=SCRATCH_TRACK; track_nr<num_max_tracks; ++track_nr)
    {
        convert_d64track2gcr(track_nr, my_id1, my_id2);
    }

    int8_t last_track;
    last_track = fill_tracks_with_file(SELECTOR_TRACK, (uint8_t*) &selector_prg[0], selector_prg_len, FILL_DOWN, num_max_tracks, my_id1, my_id2);

    uint8_t menu_pal_track = last_track+1;
    last_track = fill_tracks_with_file(menu_pal_track, (uint8_t*) &menu_pal_prg[0], menu_pal_prg_len, FILL_DOWN, num_max_tracks, my_id1, my_id2);

    uint8_t menu_16_track = last_track+1;
    last_track = fill_tracks_with_file(menu_16_track,  (uint8_t*) &menu_16_prg[0],  menu_16_prg_len,  FILL_DOWN, num_max_tracks, my_id1, my_id2);

    uint8_t menu_ntsc_track = last_track+1;
    last_track = fill_tracks_with_file(menu_ntsc_track,(uint8_t*) &menu_ntsc_prg[0],menu_ntsc_prg_len,FILL_DOWN, num_max_tracks, my_id1, my_id2);

    uint8_t intro_track = last_track+1;
    last_track = fill_tracks_with_file(intro_track,    (uint8_t*) &intro_prg[0],    intro_prg_len,    FILL_DOWN, num_max_tracks, my_id1, my_id2);

    memset(d64_sector_puffer, 0, sizeof(d64_sector_puffer));
    generate_bam("- 1541 REPICO -", id_buffer);
    // create a file-entry in the directory...
    generate_directory_entry("SELECTOR", CBMDOS_TYPE_PRG, SELECTOR_TRACK ,0,((uint16_t) (selector_prg_len/254))+1);
    generate_directory_entry("P",        CBMDOS_TYPE_PRG, menu_pal_track ,0,((uint16_t) (menu_pal_prg_len/254))+1);
    generate_directory_entry("+",        CBMDOS_TYPE_PRG, menu_16_track  ,0,((uint16_t) (menu_16_prg_len/254))+1);
    generate_directory_entry("N",        CBMDOS_TYPE_PRG, menu_ntsc_track,0,((uint16_t) (menu_ntsc_prg_len/254))+1);
    generate_directory_entry("DATAFILE", CBMDOS_TYPE_PRG, MENU_DATA_TRACK,0,((uint16_t) (menu_file_len/254))+1);
    generate_directory_entry("INTRO",    CBMDOS_TYPE_PRG, intro_track    ,0,((uint16_t) (intro_prg_len/254))+1);
    convert_d64track2gcr(DIRECTORY_TRACK, my_id1, my_id2);

    strcpy(image_name_p, "\06 ONSCREEN MENU");
    *id1_p = my_id1;
    *id2_p = my_id2;
    *num_tracks_p = num_max_tracks;
}
