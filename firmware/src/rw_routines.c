/**********************************
 * read and write image routines
 *
 * Author: F00K42
 * Last change: 2026/02/14
 ***********************************/

#include "rw_routines.h"
#include "globals.h"
#include "gcr.h"


int8_t read_disk(FIL* fd, const int image_type)
{
    uint8_t* P;
    uint8_t buffer[4];
    uint8_t header_bytes[5];
    uint8_t* current_sector;
    uint32_t offset = 0;
    uint8_t SUM;
    int8_t  last_track = -99;
    UINT    bytes_read;
    FRESULT fr;

    switch(image_type)
    {
        ///////////////////////////////////////////////////////////////////////////
        case G64_IMAGE: // G64
        {
            P = d64_sector_puffer; // use this d64_buffer as temporary storage
            fr = f_read(fd, P, sizeof(g64_head), &bytes_read);
            if((FR_OK != fr) || (sizeof(g64_head) != bytes_read))
            {
                last_track = -bytes_read;
                break;
            }

            if (0 != strncmp(g64_head, P, 8)) // we accept "GCR-1541" for the header
            {
                last_track = -2;
                break;
            }

            // read the jumptable for track-offsets and track count
            fr = f_read(fd, g64_jumptable, sizeof(g64_jumptable), &bytes_read);
            if((FR_OK != fr) || (sizeof(g64_jumptable) != bytes_read))
            {
                last_track = -3;
                break;
            }
            // read the speedtable for track-speeds
            fr = f_read(fd, g64_speedtable, sizeof(g64_speedtable), &bytes_read);
            if((FR_OK != fr) || (sizeof(g64_speedtable) != bytes_read))
            {
                last_track = -4;
                break;
            }

            SUM = 0; // used to count the existing track data
            for(uint8_t track_nr=0; track_nr<G64_TRACKCOUNT; track_nr++)
            {
                offset = g64_jumptable[track_nr<<1];
                if(0 == offset)
                {
                    // we clean existing data in case there is no trackdata stored.
                    memset(g64_tracks[track_nr], 0x00, G64_TRACKSIZE);
                    continue;
                }
                // found some track-offset .. now read the track itself
                fr = f_lseek(fd, offset);
                if(FR_OK != fr)
                {
                    last_track = -5;
                    break;
                }

                // read track length and store it.
                fr = f_read(fd, &g64_tracklen[track_nr], sizeof(g64_tracklen[0]), &bytes_read);
                if((FR_OK != fr) || (sizeof(g64_tracklen[0]) != bytes_read))
                {
                    last_track = -6;
                    break;
                }

                // read track data itself
                fr = f_read(fd, g64_tracks[track_nr], g64_tracklen[track_nr], &bytes_read);
                if((FR_OK != fr) || (g64_tracklen[track_nr] != bytes_read))
                {
                    last_track = -7;
                    break;
                }

                // fill up the remaining bytes
                memset(&g64_tracks[track_nr][g64_tracklen[track_nr]], 0x55, G64_TRACKSIZE-g64_tracklen[track_nr]);

                ++SUM;
                last_track=track_nr;
            }
        }
        break;

        case D64_IMAGE: // D64
        {
            // now convert the D64 to GCR raw-data and fill G64 structures

            id1 = 0;
            id2 = 0;
            // extract id-fields from directory track for checksum calculation
            // assumption, disk has a dir in track 18 in DOS-format where ID fiels are populated
            offset = (((uint32_t) d64_track_offset[DIRECTORY_TRACK]) << 8) + DIR_ID_OFFSET;
            if (FR_OK == (fr = f_lseek(fd, offset)))
            {
                fr = f_read(fd, buffer, 2, &bytes_read);
                if ((FR_OK == fr) && (2 == bytes_read))
                {
                    id1 = buffer[0];
                    id2 = buffer[1];
                }
            }

            buffer[0] = id2;
            // This is the second character of the ID that you specify when creating a new disk.
            // The drive uses this and the next byte to check with the byte in memory to ensure
            // the disk is not swapped in the mean time.
            buffer[1] = id1;
            buffer[2] = 0x0F;
            buffer[3] = 0x0F;
            ConvertToGCR(buffer, header_bytes);

            memset(g64_jumptable,  0, sizeof(g64_jumptable));
            memset(g64_speedtable, 0, sizeof(g64_speedtable));

            for(uint8_t track_nr=0; track_nr<G64_TRACKCOUNT; ++track_nr)
            {
                offset = ((uint32_t) d64_track_offset[track_nr]) << 8;   // we store only 16bit values;
                uint8_t num_of_sectors = d64_sector_count[d64_track_zone[track_nr]];

                fr = f_lseek(fd, offset);
                if(FR_OK != fr)
                {
                    break;
                }

                fr = f_read(fd, &d64_sector_puffer[1], num_of_sectors*D64_SECTOR_SIZE, &bytes_read);
                if((FR_OK != fr) || ((num_of_sectors*D64_SECTOR_SIZE) != bytes_read))
                {
                    break;
                }

                g64_jumptable[track_nr*2]  = (uint32_t) (G64_HEADERSIZE+track_nr*(G64_TRACKSIZE+sizeof(g64_tracklen[0])));
                g64_speedtable[track_nr*2] = (uint32_t) (g64_speedzones[d64_track_zone[track_nr]]);

                const uint8_t chksum_trackid = (track_nr+1) ^ id2 ^ id1;
                const uint8_t gap_size = d64_sector_gap[d64_track_zone[track_nr]];

                P = g64_tracks[track_nr];

                current_sector = d64_sector_puffer;

                for(uint8_t sector_nr=0; sector_nr < num_of_sectors; ++sector_nr)
                {
                    current_sector[0] = 0x07;                   // data-marker for all sectors

                    P[0] = 0xFF;								// SYNC
                    P[1] = 0xFF;								// SYNC
                    P[2] = 0xFF;								// SYNC
                    P[3] = 0xFF;								// SYNC
                    P[4] = 0xFF;								// SYNC

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
                    *P++ = 0xFF;								// SYNC
                    *P++ = 0xFF;								// SYNC
                    *P++ = 0xFF;								// SYNC
                    *P++ = 0xFF;								// SYNC
                    *P++ = 0xFF;								// SYNC

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

                last_track = track_nr;
            }
        }
        break;

        default: break;
    }
    return last_track;
}

int8_t write_disk(FIL* fd, const int image_type, const uint8_t num_tracks)
{
    UINT bytes_write;
    uint8_t* P;
    uint8_t* Out_P;
    uint8_t sector_nr;
    uint8_t num;
    uint8_t temp;
    int32_t offset = 0;
    int32_t offset_track = 0;

    int8_t last_track = -1;
    FRESULT fr;

    switch(image_type)
    {
        ///////////////////////////////////////////////////////////////////////////
        case G64_IMAGE:	// G64
        {
            // GCR-1541 header
            fr = f_write(fd, g64_head, sizeof(g64_head),&bytes_write);
            if((FR_OK != fr) || (sizeof(g64_head) != bytes_write))
            {
                last_track = -fr-20;
                break;
            }

            // jumptable
            fr = f_write(fd, g64_jumptable, sizeof(g64_jumptable),&bytes_write);
            if((FR_OK != fr) || (sizeof(g64_jumptable) != bytes_write))
            {
                last_track = -fr-40;
                break;
            }

            // speedtable
            fr = f_write(fd, g64_speedtable, sizeof(g64_speedtable),&bytes_write);
            if((FR_OK != fr) || (sizeof(g64_speedtable) != bytes_write))
            {
                last_track = -fr-60;
                break;
            }

            // raw-tracks
            for (int track_nr=0; track_nr<num_tracks; track_nr++)
            {
                fr = f_write(fd, &g64_tracklen[track_nr], sizeof(g64_tracklen[0]),&bytes_write);
                if((FR_OK != fr) || (sizeof(g64_tracklen[0]) != bytes_write))
                {
                    last_track = -fr-80;
                    break;
                }

                UINT track_write_len = G64_TRACKSIZE;
                if (num_tracks > (track_nr+1))
                {
                    if (0 != g64_jumptable[(track_nr+1)*2])
                    {
                        track_write_len = g64_jumptable[(track_nr+1)*2]-g64_jumptable[track_nr*2]-sizeof(g64_tracklen[track_nr]);
                    }
                }

                fr = f_write(fd, &g64_tracks[track_nr], track_write_len,&bytes_write);
                if((FR_OK != fr) || (track_write_len != bytes_write))
                {
                    last_track = -fr-80;
                    break;
                }
                last_track = track_nr;
            }
            break;
        }
        case D64_IMAGE:
        {
            for (int track_nr=0; track_nr<num_tracks; track_nr++)
            {
                P = g64_tracks[track_nr];
                sector_nr = d64_sector_count[d64_track_zone[track_nr]];
                uint8_t *P_end = &g64_tracks[track_nr][g64_tracklen[track_nr]];

                offset_track = ((int32_t) d64_track_offset[track_nr]) << 8;   // we store only 16bit values;

                // find first track-marker .. FF FF 52 ... FF FF 55
                do
                {
                    // tricky thing.. while searching for first track-marker
                    //  copy all "wrapped" bytes of last sector to the end again.
                    while((temp = *P++) != 0xFF) { *P_end++ = temp; };
                    if (*P++ == 0xFF)
                    {
                        while(*P == 0xFF) { ++P; };
                        if (*P == 0x52)
                        {
                            break;
                        }
                    }
                } while(1);
                ConvertFromGCR(P, d64_sector_puffer);
                if ((track_nr+1) != d64_sector_puffer[3])
                {
                    break;
                }
                P += 5; // skip the header
                offset = offset_track + (d64_sector_puffer[2]*D64_SECTOR_SIZE);
                if(FR_OK == (fr=f_lseek(fd, offset)))
                {
                    // lets extract the given FloppyID for further readback of GCR...
                    // ConvertFromGCR(P, d64_sector_puffer);
                    // id2 = d64_sector_puffer[0];
                    // id1 = d64_sector_puffer[1];
                    P += 5; // skip the header-gap-bytes
                    // find sector-marker
                    do
                    {
                        while(*P++ != 0xFF) { };
                        if (*P++ == 0xFF)
                        {
                            while(*P == 0xFF) { ++P; };
                            if (*P == 0x55)
                            {
                                break;
                            }
                        }
                    } while(1);
                    // ----
                    Out_P = d64_sector_puffer;
                    for(int i=0; i<65; ++i)
                    {
                        ConvertFromGCR(P, Out_P);
                        P += 5;
                        Out_P += 4;
                    }
                    fr = f_write(fd, &d64_sector_puffer[1], D64_SECTOR_SIZE,&bytes_write);
                    if((FR_OK != fr) || (D64_SECTOR_SIZE != bytes_write))
                    {
                        last_track = -fr-60;
                        break;
                    }
                } else {
                    last_track = -fr-40;
                    break;
                }

                for(num=0; num<(sector_nr-1); ++num)
                {
                    // find track-marker .. FF FF 52 ... FF FF 55
                    do
                    {
                        while(*P++ != 0xFF) { };
                        if (*P++ == 0xFF)
                        {
                            while(*P == 0xFF) { ++P; };
                            if (*P == 0x52)
                            {
                                break;
                            }
                        }
                    } while(1);
                    ConvertFromGCR(P, d64_sector_puffer);
                    if ((track_nr+1) != d64_sector_puffer[3])
                    {
                        break;
                    }
                    P += 4;
                    offset = offset_track + (d64_sector_puffer[2]*D64_SECTOR_SIZE);
                    if(FR_OK == (fr=f_lseek(fd, offset)))
                    {
                        // find sector-marker
                        do
                        {
                            while(*P++ != 0xFF) { };
                            if (*P++ == 0xFF)
                            {
                                while(*P == 0xFF) { ++P; };
                                if (*P == 0x55)
                                {
                                    break;
                                }
                            }
                        } while(1);
                        // ----
                        Out_P = d64_sector_puffer;
                        for(int i=0; i<65; ++i)
                        {
                            ConvertFromGCR(P, Out_P);
                            P += 5;
                            Out_P += 4;
                        }
                        fr = f_write(fd, &d64_sector_puffer[1], D64_SECTOR_SIZE,&bytes_write);
                        if((FR_OK != fr) || (D64_SECTOR_SIZE != bytes_write))
                        {
                            last_track = -fr;
                            break;
                        }
                    } else {
                        last_track = -fr-20;
                        break;
                    }
                }
                last_track = track_nr;
            }
            break;
        }
        default: break;
    }
    return last_track;
}
