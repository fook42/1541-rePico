/**********************************
 * generate menu image header
 *
 * Author: F00K42
 * Last change: 2026/05/08
 ***********************************/

#include "menu_image.h"
#include "globals.h"
#include "gcr.h"
#include "ctype.h"

#define MAX_DIR_ENTRIES (32)

void generate_empty_image(uint8_t image_id1, uint8_t image_id2)
{
    uint8_t* P;
    uint8_t buffer[4];
    uint8_t header_bytes[5];
    uint8_t* current_sector;
    uint8_t SUM;

    buffer[0] = image_id2;
    // This is the second character of the ID that you specify when creating a new disk.
    // The drive uses this and the next byte to check with the byte in memory to ensure
    // the disk is not swapped in the mean time.
    buffer[1] = image_id1;
    buffer[2] = 0x0F;
    buffer[3] = 0x0F;
    ConvertToGCR(buffer, header_bytes);

    memset(g64_jumptable,  0, sizeof(g64_jumptable));
    memset(g64_speedtable, 0, sizeof(g64_speedtable));

    for(uint8_t track_nr=0; track_nr<G64_TRACKCOUNT; ++track_nr)
    {
        uint8_t num_of_sectors = d64_sector_count[d64_track_zone[track_nr]];

        memset(d64_sector_puffer, 0, (num_of_sectors*D64_SECTOR_SIZE)+1);

        g64_jumptable[track_nr*2]  = (uint32_t) (G64_HEADERSIZE+track_nr*(G64_TRACKSIZE+sizeof(g64_tracklen[0])));
        g64_speedtable[track_nr*2] = (uint32_t) (g64_speedzones[d64_track_zone[track_nr]]);

        const uint8_t chksum_trackid = (track_nr+1) ^ image_id2 ^ image_id1;
        const uint8_t gap_size = d64_sector_gap[d64_track_zone[track_nr]];

        P = g64_tracks[track_nr];

        current_sector = d64_sector_puffer;

        for(uint8_t sector_nr=0; sector_nr < num_of_sectors; ++sector_nr)
        {
            current_sector[0] = 0x07;                   // data-marker for all sectors

            P[0] = GCR_SYNCMARK;								// SYNC
            P[1] = GCR_SYNCMARK;								// SYNC
            P[2] = GCR_SYNCMARK;								// SYNC
            P[3] = GCR_SYNCMARK;								// SYNC
            P[4] = GCR_SYNCMARK;								// SYNC

            buffer[0] = 0x08;							// Header Markierung
            buffer[1] = sector_nr ^ chksum_trackid;     // Checksumme
            buffer[2] = sector_nr;
            buffer[3] = track_nr+1;
            ConvertToGCR(buffer, &P[5]);

            P[10] = header_bytes[0];                     // fill in constant header
            P[11] = header_bytes[1];                     //  bytes containing
            P[12] = header_bytes[2];                     //  disk-id2 & id1
            P[13] = header_bytes[3];
            P[14] = header_bytes[4];
            P += 15;

            // GAP Bytes als Lücke
            for (uint8_t i = HEADER_GAP_BYTES; i>0; --i)
            {
                *P++ = 0x55;
            }

            // SYNC
            *P++ = GCR_SYNCMARK;								// SYNC
            *P++ = GCR_SYNCMARK;								// SYNC
            *P++ = GCR_SYNCMARK;								// SYNC
            *P++ = GCR_SYNCMARK;								// SYNC
            *P++ = GCR_SYNCMARK;								// SYNC

            SUM = 0x07;     // checksum is prefilled with data-marker
                            // -> the complete buffer can be processed
            for (int i=0; i<257; ++i)
            {
                SUM ^= current_sector[i];
            }

            for (int i=0; i<256; i+=4)
            {
                ConvertToGCR(&(current_sector[i]), P);
                P += 5;
            }

            buffer[0] = current_sector[256];
            buffer[1] = SUM;   // Checksum
            buffer[2] = 0;
            buffer[3] = 0;
            ConvertToGCR(buffer, P);
            P += 5;

            // GCR Bytes als Lücken auffüllen (sorgt für eine Gleichverteilung)
            memset(P, 0x55, gap_size);
            P += gap_size;

            current_sector += 256;
        }
        g64_tracklen[track_nr]=(uint16_t) (P - g64_tracks[track_nr]);

        memset(P, 0x55, G64_TRACKSIZE-(P - g64_tracks[track_nr]));
    }
}

void generate_menu_bam(char* image_name, uint8_t image_id1, uint8_t image_id2)
{
    // generate "ascii"-data first... then convert the complete track to GCR

    uint8_t* P = &d64_sector_puffer[1];

    memset(P, 0, 256);
    P[0]=DIRECTORY_TRACK+1;
    P[1]=0x01;
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
    P[DIR_ID_OFFSET]   = image_id1;
    P[DIR_ID_OFFSET+1] = image_id2;
}

void generate_menu_image(DIR* dir_obj)
{
    FILINFO     fb_dir_menu_entries[MAX_DIR_ENTRIES];
    FRESULT     fr;
    uint16_t    dir_sector_nr=1;
    uint8_t*    P;
    uint8_t*    dir_sector_P;
    

    dir_sector_P = &d64_sector_puffer[1+(dir_sector_nr<<8)];

    P = dir_sector_P;
    memset(P, 0, 256);
    // we start with one-sector for the dir.. lets expand this later
    // P[0]=DIRECTORY_TRACK+1;
    // P[1]=0x04;

    P=&P[2]; // skip the "next-sector"-pointer

    uint8_t i=0;

    while(i<MAX_DIR_ENTRIES)
    {
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
        if (0==(i&0x7)) // we have 8*x = i
        {
            dir_sector_nr++;
            dir_sector_P[0]=DIRECTORY_TRACK+1;
            dir_sector_P[1]=dir_sector_nr;
            dir_sector_P = &d64_sector_puffer[1+(dir_sector_nr<<8)];
            P = dir_sector_P;
            memset(P, 0, 256);
            P=&P[2]; // skip the "next-sector"-pointer
        }
    }
}

void convert_track2gcr(uint8_t track_nr, uint8_t image_id1, uint8_t image_id2)
{
    uint8_t* P;
    uint8_t buffer[4];
    uint8_t header_bytes[5];
    uint8_t* current_sector;
    uint8_t SUM;

    const uint8_t num_of_sectors = d64_sector_count[d64_track_zone[track_nr]];
    const uint8_t chksum_trackid = (track_nr+1) ^ image_id2 ^ image_id1;
    const uint8_t gap_size = d64_sector_gap[d64_track_zone[track_nr]];

    buffer[0] = image_id2;
    buffer[1] = image_id1;
    buffer[2] = 0x0F;
    buffer[3] = 0x0F;
    ConvertToGCR(buffer, header_bytes);

    P = g64_tracks[track_nr];

    current_sector = d64_sector_puffer;

    for(uint8_t sector_nr=0; sector_nr < num_of_sectors; ++sector_nr)
    {
        current_sector[0] = 0x07;                   // data-marker for all sectors

        P[0] = GCR_SYNCMARK;								// SYNC
        P[1] = GCR_SYNCMARK;								// SYNC
        P[2] = GCR_SYNCMARK;								// SYNC
        P[3] = GCR_SYNCMARK;								// SYNC
        P[4] = GCR_SYNCMARK;								// SYNC

        buffer[0] = 0x08;							// Header Markierung
        buffer[1] = sector_nr ^ chksum_trackid;     // Checksumme
        buffer[2] = sector_nr;
        buffer[3] = track_nr+1;
        ConvertToGCR(buffer, &P[5]);

        P[10] = header_bytes[0];                     // fill in constant header
        P[11] = header_bytes[1];                     //  bytes containing
        P[12] = header_bytes[2];                     //  disk-id2 & id1
        P[13] = header_bytes[3];
        P[14] = header_bytes[4];
        P += 15;

        // GAP Bytes als Lücke
        for (uint8_t i = HEADER_GAP_BYTES; i>0; --i)
        {
            *P++ = 0x55;
        }

        // SYNC
        *P++ = GCR_SYNCMARK;								// SYNC
        *P++ = GCR_SYNCMARK;								// SYNC
        *P++ = GCR_SYNCMARK;								// SYNC
        *P++ = GCR_SYNCMARK;								// SYNC
        *P++ = GCR_SYNCMARK;								// SYNC

        SUM = 0x07;     // checksum is prefilled with data-marker
                        // -> the complete buffer can be processed
        for (int i=0; i<257; ++i)
        {
            SUM ^= current_sector[i];
        }

        for (int i=0; i<256; i+=4)
        {
            ConvertToGCR(&(current_sector[i]), P);
            P += 5;
        }

        buffer[0] = current_sector[256];
        buffer[1] = SUM;   // Checksum
        buffer[2] = 0;
        buffer[3] = 0;
        ConvertToGCR(buffer, P);
        P += 5;

        // GCR Bytes als Lücken auffüllen (sorgt für eine Gleichverteilung)
        memset(P, 0x55, gap_size);
        P += gap_size;

        current_sector += 256;
    }
    g64_tracklen[track_nr]=(uint16_t) (P - g64_tracks[track_nr]);

    memset(P, 0x55, G64_TRACKSIZE-(P - g64_tracks[track_nr]));
}