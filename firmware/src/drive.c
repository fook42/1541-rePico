/**********************************
 * drive routines and control functions
 *
 * Author: F00K42
 * Last change: 2026/10/28
 ***********************************/

#include "main.h"
#include "drive.h"

/////////////////////////////////////////////////////////////////////

int64_t steppertimer_callback(alarm_id_t id, void *user_data)
{
    (void) id;
    (void) user_data;

    send_byte_ready = false;
    stop_bytetimer();

    // neue track Geschwindigkeit setzen -> timer restart
    akt_track_pos = 0;

    akt_half_track = selected_track&0x7E;

    start_bytetimer(akt_half_track);
    send_byte_ready = true;
    turn_sound_off();

    return 0;
}


void start_stepper_timer(void)
{
    turn_sound_on();

    if (stepper_alarm) { cancel_alarm(stepper_alarm); }
    stepper_alarm = add_alarm_in_ms(STEPPER_DELAY_TIME, steppertimer_callback, NULL, false);
}

/////////////////////////////////////////////////////////////////////

void check_stepper_signals(void)
{
    // Auf Steppermotor aktivität prüfen
    // und auswerten
    if(stepper_signal_r_pos != stepper_signal_w_pos)    // Prüfen ob sich was neues im Ringpuffer für die Steppersignale befindet
    {
        uint8_t stepper = stepper_signal_puffer[(stepper_signal_r_pos+255)&0xFF]<<2;
        stepper |= stepper_signal_puffer[stepper_signal_r_pos];
        stepper_signal_r_pos++;

        switch(stepper)
        {
            case 0b00000011:
            case 0b00000100:
            case 0b00001001:
            case 0b00001110:
                {
                    // DEC
                    stepper_dec();
                    start_stepper_timer();
                }
                break;

            case 0b00000001:
            case 0b00000110:
            case 0b00001011:
            case 0b00001100:
                {
                    // INC
                    stepper_inc();
                    start_stepper_timer();
                }
                break;

            default:
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////

void send_disk_change(void)
{
    if(floppy_wp)
    {                   // WP enabled = wps set to 0
        set_wps();
        sleep_ms(1);
        clear_wps();
        sleep_ms(1);
        set_wps();
        sleep_ms(1);
        clear_wps();
    } else {            // WP disabled = wps set to 1
        clear_wps();
        sleep_ms(1);
        set_wps();
        sleep_ms(1);
        clear_wps();
        sleep_ms(1);
        set_wps();
    }
}

/////////////////////////////////////////////////////////////////////

void init_writeprot(void)
{
    gpio_init(GPIO_WPS);
    gpio_set_dir(GPIO_WPS, GPIO_IN);
    gpio_set_pulls(GPIO_WPS, false, false);
    // remark:
    // 1541 schematics include a 74ls04 which drives WPS low by default
    // having a pull-up enabled is not a good idea
}

/////////////////////////////////////////////////////////////////////

void unmount_image(void)
{
    close_disk_image(&fd);
    is_image_mount = 0;
    akt_image_type = UNDEF_IMAGE;
    num_max_tracks = 0;
    enable_write_protection();
    menu_set_entry_var1(&image_menu, M_WP_IMAGE, 1);
    send_disk_change();
}

/////////////////////////////////////////////////////////////////////

void show_sdcard_info_message(void)
{
    sd_card_t* my_sdcard;
    uint32_t sdcard_size = 0;

    if (0 != fs.fs_type)    // check for valid mounted file-system
    {
        display_clear();
        display_home();

        display_string(disp_sdinfo_size_s);
        my_sdcard = sd_get_by_num(0);   // get a sd_card_info-struct from current card
        if (NULL != my_sdcard)
        {
            sdcard_size = my_sdcard->get_num_sectors(my_sdcard);
            char data[9];
            dez2out((int32_t)(sdcard_size*512E-9),0,data);
            display_string(data);
        }
        display_string(" GB");

        display_setcursor(0,1);
        display_string(disp_sdinfo_part_s);

        switch (fs.fs_type)
        {
            case FS_FAT12:
                display_string("FAT12");
                break;
            case FS_FAT16:
                display_string("FAT16");
                break;
            case FS_FAT32:
                display_string("FAT32");
                break;
            case FS_EXFAT:
                display_string("EXT FAT");
                break;
            default:
                break;
        }
    }
    //sleep_ms(START_MESSAGE_TIME);

    // struct sd_raw_info info;

    // uint8_t counter = 5;
    // uint8_t get_info_ok = 0;
    // char out_str[21];

    // while(counter != 0)
    // {
    //     if(0 != sd_raw_get_info(&info))
    //     {
    //         display_setcursor(disp_sdinfo_manuf_p);
    //         display_string(disp_sdinfo_manuf_s);
    //         sprintf(out_str, "%.x", info.manufacturer);
    //         display_string(out_str);

    //         display_setcursor(disp_sdinfo_oem_p);
    //         display_string(disp_sdinfo_oem_s);
    //         display_string((char*) info.oem);

    //         display_setcursor(disp_sdinfo_prod_p);
    //         display_string(disp_sdinfo_prod_s);
    //         display_string((char*) info.product);

    //         display_setcursor(disp_sdinfo_size_p);
    //         display_string(disp_sdinfo_size_s);
    //         sprintf(out_str, "%d MB", (uint16_t)(info.capacity / 1024 / 1024));
    //         display_string(out_str);

    //         get_info_ok = 1;

    //         break;
    //     }

    //     release_sd_card();
    //     (void) init_sd_card();

    //     counter--;
    // }

    // if(!get_info_ok)
    // {
    //     display_clear();
    //     display_setcursor(disp_geterr_failure_p);
    //     display_string(disp_geterr_failure_s);
    //     display_setcursor(disp_sdrawgetinfo_p);
    //     display_string(disp_sdrawgetinfo_s);
    //     return;
    // }

    // _delay_ms(START_MESSAGE_TIME);

    // display_clear();

    // display_setcursor(disp_sdinfo_rev_p);
    // display_string(disp_sdinfo_rev_s);
    // sprintf(out_str,"%c.%c",(info.revision>>4)+'0', (info.revision&0x0f)+'0');
    // display_string(out_str);

    // display_setcursor(disp_sdinfo_serial_p);
    // display_string(disp_sdinfo_serial_s);
    // sprintf(out_str,"%04X%04X",(unsigned int) (info.serial >> 16),(unsigned int) (info.serial & 0xffff));
    // display_string(out_str);

    // display_setcursor(disp_sdinfo_part_p);
    // display_string(disp_sdinfo_part_s);
    // switch(partition->type)
    // {
    //     case 0x01:
    //         display_string("FAT12");
    //         break;

    //     case 0x04:
    //         display_string("FAT16 <32MB");
    //         break;

    //     case 0x05:
    //         display_string("EXTENDED");
    //         break;

    //     case 0x06:
    //         display_string("FAT16");
    //         break;

    //     case 0x0b:
    //         display_string("FAT32");
    //         break;

    //     case 0x0c:
    //         display_string("FAT32 LBA");
    //         break;

    //     case 0x0e:
    //         display_string("FAT16 LBA");
    //         break;

    //     case 0x0f:
    //         display_string("EXT LBA");
    //         break;

    //     default:
    //         display_string("UNKNOWN");
    //         break;
    // }
    // _delay_ms(START_MESSAGE_TIME);
}

/////////////////////////////////////////////////////////////////////

void init_stepper(void)
{
    // Stepper PINs als Eingang schalten
    gpio_init(GPIO_STP0);
    gpio_init(GPIO_STP1);
    gpio_set_pulls(GPIO_STP0, true, false);
    gpio_set_pulls(GPIO_STP1, true, false);
    gpio_set_dir(GPIO_STP0, GPIO_IN);
    gpio_set_dir(GPIO_STP1, GPIO_IN);

    selected_track = (INIT_TRACK << 1);
    akt_half_track = selected_track;

    // Pin Change Interrupt für beide STPx PIN's aktivieren
    gpio_set_irq_enabled(GPIO_STP0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(GPIO_STP1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

/////////////////////////////////////////////////////////////////////

void stepper_inc(void)
{
    if(selected_track >= ((NUM_TRACKS_MAX-1)<<1)) return;

    ++selected_track;
}

/////////////////////////////////////////////////////////////////////

void stepper_dec(void)
{
    if(selected_track == 0) return;

    --selected_track;
}

/////////////////////////////////////////////////////////////////////

void init_motor(void)
{
    // Als Eingang schalten
    gpio_init(GPIO_MTR);
    gpio_set_pulls(GPIO_MTR, true, false);
    gpio_set_dir(GPIO_MTR, GPIO_IN);
}

/////////////////////////////////////////////////////////////////////

void init_control_signals(void)
{
    // Als Ausgang schalten
    //DDxn = 0 , PORTxn = 0 --> HiZ
    //DDxn = 1 , PORTxn = 0 --> Output Low (Sink)
    gpio_init(GPIO_BRDY);
    gpio_set_pulls(GPIO_BRDY, false, false);
    gpio_set_dir(GPIO_BRDY, GPIO_IN);
    // BYTE_READY_DDR &= ~(1<<BYTE_READY);             // Byte Ready auf HiZ
    // BYTE_READY_PORT &= ~(1<<BYTE_READY);

    gpio_init(GPIO_SYNC);
    gpio_set_dir(GPIO_SYNC, GPIO_OUT);

    gpio_set_drive_strength(GPIO_SYNC, GPIO_DRIVE_STRENGTH_12MA);

    // Als Eingang schalten
    gpio_init_mask(PAPORT_MASK);
    gpio_set_pulls(GPIO_PAPORT  , true, false);
    gpio_set_pulls(GPIO_PAPORT+1, true, false);
    gpio_set_pulls(GPIO_PAPORT+2, true, false);
    gpio_set_pulls(GPIO_PAPORT+3, true, false);
    gpio_set_pulls(GPIO_PAPORT+4, true, false);
    gpio_set_pulls(GPIO_PAPORT+5, true, false);
    gpio_set_pulls(GPIO_PAPORT+6, true, false);
    gpio_set_pulls(GPIO_PAPORT+7, true, false);
    gpio_set_dir_in_masked(PAPORT_MASK); // should set all 8 bits to input

    gpio_set_drive_strength(GPIO_PAPORT  , GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+1, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+2, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+3, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+4, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+5, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+6, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(GPIO_PAPORT+7, GPIO_DRIVE_STRENGTH_12MA);

    gpio_init(GPIO_SOE);
    gpio_set_pulls(GPIO_SOE, true, false);
    gpio_set_dir(GPIO_SOE, GPIO_IN);

    gpio_init(GPIO_OE);
    gpio_set_pulls(GPIO_OE, true, false);
    gpio_set_dir(GPIO_OE, GPIO_IN);
}

/////////////////////////////////////////////////////////////////////

void init_soe_gatearray(void)
{
    gpio_init(GPIO_SOE_GA);
    gpio_set_dir(GPIO_SOE_GA, GPIO_OUT);
    set_soe_gatearray();
}

/////////////////////////////////////////////////////////////////////

void init_bytetimer(void)
{
    stop_bytetimer();
}

/////////////////////////////////////////////////////////////////////

void start_bytetimer(uint8_t half_track)
{
    (void) add_repeating_timer_us(-bytetimer_values[d64_track_zone[half_track>>1]], repeating_timer_callback, NULL, &bytetimer);
}

/////////////////////////////////////////////////////////////////////

void stop_bytetimer(void)
{
    (void) cancel_repeating_timer(&bytetimer);
}

/////////////////////////////////////////////////////////////////////

void init_sound(void)
{
    gpio_init(GPIO_SND);
    gpio_set_dir(GPIO_SND, GPIO_OUT);
    turn_sound_off();
}

///////////////////////////////////////
///////// ISR
///////////////////////////////////////


//ISR (TIMER0_COMPA_vect)
bool repeating_timer_callback(__unused struct repeating_timer *t)
{
    // ISR wird alle 26,28,30 oder 32µs ausfgrufen
    // Je nach dem welche Spur gerade aktiv ist

    uint8_t akt_gcr_byte;

    if(get_so_status())
    {
        static uint8_t old_gcr_byte = 0;
        uint8_t is_sync;
        // LESE MODUS
        // Daten aus Ringpuffer senden wenn Motor an und ein Image gemountet ist
        if(get_motor_status() && is_image_mount)
        {                                                               // Wenn Motor läuft
            akt_gcr_byte = g64_tracks[akt_half_track>>1][akt_track_pos++];          // Nächstes GCR Byte holen
            if(akt_track_pos == g64_tracklen[akt_half_track>>1]) akt_track_pos = 0; // Ist Spurende erreicht? Zurück zum Anfang

            if((GCR_SYNCMARK == akt_gcr_byte) && (GCR_SYNCMARK == old_gcr_byte))    // Prüfen auf SYNC (mindesten 2 aufeinanderfolgende 0xFF)
            {                                                           // Wenn SYNC
                clear_sync();                                           // SYNC Leitung auf Low setzen
                is_sync = 1;                                            // SYNC Merker auf 1
            }
            else
            {                                                           // Wenn kein SYNC
                set_sync();                                             // SYNC Leitung auf High setzen
                is_sync = 0;                                            // SYNC Merker auf 0
            }
        }
        else
        {                                                               // Wenn Motor nicht läuft
            akt_gcr_byte = 0x00;                                        // 0 senden wenn Motor aus
            is_sync = 0;                                                // SYNC Merker auf 0
        }

        // SOE
        // Unabhängig ob der Motor läuft oder nicht
        if(get_soe_status())
        {
            if(!is_sync)
            {
                gpio_set_dir_out_masked(PAPORT_MASK);
                out_gcr_byte(akt_gcr_byte);

                if(send_byte_ready)
                {
                    // BYTE_READY für 3µs löschen
                    clear_byte_ready();
                    busy_wait_us(3);
                    set_byte_ready();
                }
            }
            // else --> kein Byte senden !!
        }
        old_gcr_byte = akt_gcr_byte;
    }
    else
    {
        // SCHREIB MODUS

        if (!block_data_changes)    // only accept changes when no image-dump is happening
        {
            // SOE
            // Unabhängig ob der Motor läuft oder nicht
            if(get_soe_status())
            {
                gpio_set_dir_in_masked(PAPORT_MASK);
                akt_gcr_byte = in_gcr_byte();

                if(send_byte_ready)
                {
                    // BYTE_READY für 3µs löschen
                    clear_byte_ready();
                    busy_wait_us(3);
                    set_byte_ready();
                }
            }

            // Daten aus Ringpuffer senden wenn Motor an
            if(get_motor_status())
            {
                if (!track_is_written)
                {
                    track_write_nr  = akt_half_track>>1;
                    track_write_pos = akt_track_pos;
                }
                // Wenn Motor läuft
                g64_tracks[akt_half_track>>1][akt_track_pos++] = akt_gcr_byte;  // Nächstes GCR Byte schreiben
                track_is_written = true;
                if(akt_track_pos == g64_tracklen[akt_half_track>>1]) akt_track_pos = 0;    // Ist Spurende erreicht? Zurück zum Anfang
            }
        }
    }
    return true;
}

