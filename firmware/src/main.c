/////////////////////////////////////////////////
// 1541-rePico
/////////////////////////////////////////////////
// author: F00K42
// last changed: 2026/02/11
// repo: https://github.com/fook42/1541-rePico
/////////////////////////////////////////////////


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "pinout.h"

#define _EXTERN_

#include "version.h"
#include "display.h"
#include "lcd.h"
#include "oled.h"
#include "i2c.h"
#include "gcr.h"
#include "menu.h"
#include "mymenu.h"
#include "gui_constants.h"
#include "globals.h"
#include "rw_routines.h"

#include "hw_config.h"
#include "f_util.h"
#include "ff.h"

#include "main.h"


#define START_MESSAGE_TIME 1500

volatile uint8_t irq_key_value = NO_KEY;

volatile bool rotary_input_block = false;

// ---------------------------------------------------------------

int64_t input_debounce_callback(alarm_id_t id, void *user_data)
{
    rotary_input_block = false;
    return 0;
}

void gpio_callback(uint gpio, uint32_t events)
{
    if ((GPIO_STP0==gpio) || (GPIO_STP1==gpio))
    {
        // general gpio-ISR .. triggered for STP0 or STP1 change.. no need to detect the cause
        stepper_signal_puffer[stepper_signal_w_pos] = ((bool_to_bit(gpio_get(GPIO_STP0))<<1) | (bool_to_bit(gpio_get(GPIO_STP1))));
        stepper_signal_w_pos++;
    } else if (GPIO_BT3==gpio)
    {
        irq_key_value=(events==GPIO_IRQ_EDGE_FALL)?KEY2_DOWN:KEY2_UP;
    } else if ((GPIO_BT1==gpio) && (false == rotary_input_block) && (NO_KEY==irq_key_value))
    {
        rotary_input_block = true;
        if (gpio_get(GPIO_BT2))
        {
            irq_key_value = KEY1_DOWN;
        } else {
            irq_key_value = KEY0_DOWN;
        }
        input_debounce_alarm = add_alarm_in_ms(INPUT_DEBOUNCE_TIME, input_debounce_callback, NULL, false);
    }
}

// ---------------------------------------------------------------

int main()
{
    stdio_init_all();

    // init input-keys
    init_key_inputs();

    // Stepper Initialisieren
    init_stepper();

    // Motor Initialisieren
    init_motor();

    // Steursignale BYTE_READY, SYNC und SOE Initialisieren
    init_control_signals();

    init_soe_gatearray();
    clear_soe_gatearray();

    init_bytetimer();

    init_writeprot();
    enable_write_protection();

    if (display_init())
    {
        display_clear();
        display_home();
    }

    show_start_message();

    fr = f_mount(&fs, "", 1);
    uint8_t retry_count = 3;

    while ((FR_OK != fr) && (retry_count > 0)) {
        display_setcursor(0, 0);
        display_string("f_mount error:");
        display_data(fr+'0');
        display_setcursor(0, 1);
        display_print(FRESULT_str(fr),0,16);
        display_setcursor(0, 2);
        display_print(FRESULT_str(fr),16,16);
        display_setcursor(0, 3);
        display_string("retrying ... ");
        display_data(retry_count+'0');
        retry_count--;
        sleep_ms(2000);
        fr = f_mount(&fs, "", 1);
        display_clear();
    }

    display_setcursor(0, 0);
    display_string("f_mount okay");

    fb_dir_entry_count = get_dir_entry_count(); // open card, count entries on root level

    sleep_ms(1000);

    display_clear();
    display_home();

    uint8_t dsp_zeile = 0;

    // setup menus
    menu_init(&main_menu,     main_menu_entrys,     count_of(main_menu_entrys),     LCD_COLS, LCD_ROWS);
    menu_init(&image_menu,    image_menu_entrys,    count_of(image_menu_entrys),    LCD_COLS, LCD_ROWS);
    menu_init(&settings_menu, settings_menu_entrys, count_of(settings_menu_entrys), LCD_COLS, LCD_ROWS);
    menu_init(&info_menu,     info_menu_entrys,     count_of(info_menu_entrys),     LCD_COLS, LCD_ROWS);

    menu_set_root(&main_menu);
    // ----
    set_gui_mode(GUI_MENU_MODE);

    while (true) {
        check_stepper_signals();
        check_motor_signal();
        update_gui();
    }
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int64_t steppertimer_callback(alarm_id_t id, void *user_data)
{
    send_byte_ready = false;
    stop_bytetimer();

    // neue track Geschwindigkeit setzen -> timer restart
    akt_track_pos = 0;

    akt_half_track = selected_track&0x7E;

    start_bytetimer(akt_half_track);
    send_byte_ready = true;
    return 0;
}


void start_stepper_timer(void)
{
    if (stepper_alarm) { cancel_alarm(stepper_alarm); }
    stepper_alarm = add_alarm_in_ms(STEPPER_DELAY_TIME, steppertimer_callback, NULL, false);
}

/////////////////////////////////////////////////////////////////////

void check_stepper_signals(void)
{
    uint8_t stepper;
    // Auf Steppermotor aktivität prüfen
    // und auswerten
    if(stepper_signal_r_pos != stepper_signal_w_pos)    // Prüfen ob sich was neues im Ringpuffer für die Steppersignale befindet
    {
        stepper  = stepper_signal_puffer[stepper_signal_r_pos-1]<<2;
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

void check_motor_signal(void)
{
    if(!get_motor_status())
    {
        // Sollte der aktuelle Track noch veränderungen haben so wird hier erstmal gesichert.
        if(track_is_written)
        {
            send_byte_ready = false;
            stop_bytetimer();

            track_is_written = false;
            // write_disk_track(fd,akt_image_type,old_half_track>>1,gcr_track, &gcr_track_length);

            start_bytetimer(akt_half_track);
            send_byte_ready = true;
        }
    }
}

/////////////////////////////////////////////////////////////////////

void init_key_inputs(void)
{
    gpio_init(GPIO_BT1);
    gpio_init(GPIO_BT2);
    gpio_init(GPIO_BT3);
    gpio_set_dir(GPIO_BT1, GPIO_IN);
    gpio_set_dir(GPIO_BT2, GPIO_IN);
    gpio_set_dir(GPIO_BT3, GPIO_IN);
    gpio_set_pulls(GPIO_BT1, true, false);
    gpio_set_pulls(GPIO_BT2, true, false);
    gpio_set_pulls(GPIO_BT3, true, false);
    gpio_set_irq_enabled_with_callback(GPIO_BT1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(GPIO_BT3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

uint8_t get_key_from_buffer(void)
{
    uint8_t val;

    val = irq_key_value;    // get last detected key
    irq_key_value = NO_KEY; // reset last state - ready for more

    return val;
}

void update_gui(void)
{
    static uint8_t old_half_track = 255;
    static bool old_motor_status = false;
    static uint32_t wait_counter0 = 0;
    bool new_motor_status;
    uint8_t key_code = get_key_from_buffer();
    char byte_str[16];

    switch (current_gui_mode)
    {
    case GUI_INFO_MODE:

        if(KEY2_UP == key_code)
        {
            set_gui_mode(GUI_MENU_MODE);
        } else if(KEY2_TIMEOUT2 == key_code)
        {
            // exit_main = 0;
        }

        if(old_half_track != akt_half_track)
        {
            display_setcursor(disp_trackno_p);
            (void)dez2out((akt_half_track>>1)+1, 2, byte_str);  // much faster than sprintf
            display_data(byte_str[0]);
            display_data(byte_str[1]);
            old_half_track = akt_half_track;
        }


        new_motor_status = get_motor_status();
        if(old_motor_status != new_motor_status)
        {
            old_motor_status = new_motor_status;
            display_setcursor(disp_motortxt_p);
            if(new_motor_status)
                display_string(disp_motor_on_s);
            else
                display_string(disp_motor_off_s);
        }

        if(is_image_mount)
        {
            //// Filename Scrolling

            ++wait_counter0;

            if((gui_current_line_offset > 0) && (wait_counter0 == 300000))
            {
                wait_counter0 = 0;

                if(0 == gui_line_scroll_end_begin_wait)
                {
                    // Es darf gescrollt werden
                    if(!gui_line_scroll_direction)
                    {
                        ++gui_line_scroll_pos;
                        if(gui_line_scroll_pos >= gui_current_line_offset)
                        {
                            gui_line_scroll_end_begin_wait = 6;
                            gui_line_scroll_direction = 1;
                        }
                    }
                    else
                    {
                        --gui_line_scroll_pos;
                        if(gui_line_scroll_pos == 0)
                        {
                            gui_line_scroll_end_begin_wait = 6;
                            gui_line_scroll_direction = 0;
                        }
                    }

                    display_setcursor(disp_scrollfilename_p);
                    display_print(image_filename,gui_line_scroll_pos,LCD_LINE_SIZE);
                } else {
                    --gui_line_scroll_end_begin_wait;
                }
            }
        }
        break;

    case GUI_MENU_MODE:
        check_menu_events(menu_update(key_code));
        break;

    case GUI_FILE_BROWSER:
        filebrowser_update(key_code);
        break;

    default:
        break;
    }
}

void check_menu_events(const uint16_t menu_event)
{
    const uint8_t command = (uint8_t) ((menu_event >> 8) & 0xff);
    const uint8_t value = (uint8_t) (menu_event & 0xff);

    switch(command)
    {
        case MC_EXIT_MENU:
            set_gui_mode(GUI_INFO_MODE);
            break;

        case MC_SELECT_ENTRY:
            switch(value)
            {
                /// Main Menü

                /// Image Menü
                case M_LOAD_IMAGE:
                    set_gui_mode(GUI_FILE_BROWSER);
                    break;

                case M_UNLOAD_IMAGE:
                    unmount_image();
                    set_gui_mode(GUI_INFO_MODE);
                    break;

                case M_WP_IMAGE:
                    if(menu_get_entry_var1(&image_menu, M_WP_IMAGE))
                    {
                        enable_write_protection();
                    } else {
                        disable_write_protection();
                    }
                    menu_refresh();
                    break;

                /// Settings Menü
                case M_RESTART:
                    // exit_main = 0;
                    menu_refresh();
                    break;

                /// Info Menü
                case M_VERSION_INFO:
                    //show_start_message();
                    menu_refresh();
                    break;

                case M_SDCARD_INFO:
                    //show_sdcard_info_message();
                    menu_refresh();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////

void set_gui_mode(const uint8_t gui_mode)
{
    uint8_t byte_str[6];
    current_gui_mode = gui_mode;
    switch(gui_mode)
    {
    case GUI_INFO_MODE:
        display_clear();

        display_setcursor(disp_tracktxt_p);
        display_string(disp_tracktxt_s);

        display_setcursor(disp_trackno_p);
        (void)dez2out((akt_half_track>>1)+1, 2, byte_str);
        display_data(byte_str[0]);
        display_data(byte_str[1]);

        if(get_motor_status())
        {
            display_setcursor(disp_motortxt_p);
            display_string(disp_motor_on_s);
        }

        if(floppy_wp)
        {
            display_setcursor(disp_writeprottxt_p);
            display_string(disp_writeprot_on_s);
        }

        display_setcursor(disp_scrollfilename_p);
        if(is_image_mount)
        {
            display_print(image_filename,0,LCD_LINE_SIZE);

            // Für Scrollenden Filename
            int8_t var = (int8_t)strlen(image_filename) - LCD_LINE_SIZE;
            if(var < 0)
                gui_current_line_offset = 0;
            else
                gui_current_line_offset = var;
            gui_line_scroll_pos = 0;
            gui_line_scroll_direction = 0;
            gui_line_scroll_end_begin_wait = 6;
        } else {
            display_string(disp_nofilemounted_s);
        }

        break;
    case GUI_MENU_MODE:
        menu_refresh();
        break;
    case GUI_FILE_BROWSER:
        filebrowser_refresh();
        break;
    default:
        break;
    }
}

void show_start_message(void)
{
    display_clear();
    display_setcursor(disp_versiontxt_p);
    display_string(disp_versiontxt_s);
    display_setcursor(disp_firmwaretxt_p);
    display_string(disp_firmwaretxt_s);
    display_string(VERSION);
    sleep_ms(START_MESSAGE_TIME);

    display_clear();
}

/////////////////////////////////////////////////////////////////////

void filebrowser_update(uint8_t key_code)
{
    static uint32_t fbup_wait_counter0 = 0;
    int8_t tracks_read;

    switch (key_code)
    {
    case KEY0_DOWN:
        if(fb_cursor_pos > 0)
        {
            --fb_cursor_pos;
            filebrowser_refresh();
        }
        else
        {
            if(fb_window_pos > 0)
            {
                --fb_window_pos;
                filebrowser_refresh();
            }
        }
        break;
    case KEY1_DOWN:
        if((fb_cursor_pos < LCD_LINE_COUNT-1) && (fb_cursor_pos < fb_dir_entry_count-1))
        {
            ++fb_cursor_pos;
            filebrowser_refresh();
        }
        else
        {
            if(fb_window_pos < fb_dir_entry_count - LCD_LINE_COUNT)
            {
                ++fb_window_pos;
                filebrowser_refresh();
            }
        }
        break;
    case KEY2_UP:
        stop_bytetimer();
        send_byte_ready = false;      // this blocks current transfers to the VIA

        close_disk_image(&fd);

        if(fb_dir_entry[fb_cursor_pos].fattrib & AM_DIR)
        {
            // Eintrag ist ein Verzeichnis
            f_chdir(fb_dir_entry[fb_cursor_pos].fname);
            fb_cursor_pos = 0;
            fb_window_pos = 0;
            filebrowser_refresh();
            return;
        }

        open_disk_image(&fd, &fb_dir_entry[fb_cursor_pos], &akt_image_type);

        if(UNDEF_IMAGE == akt_image_type)
        {
            display_clear();
            display_setcursor(disp_unsupportedimg_p);
            display_string(disp_unsupportedimg_s);
            sleep_ms(1000);
        }

        if((0 == fd.obj.fs) || (UNDEF_IMAGE == akt_image_type))
        {
            is_image_mount = false;
            filebrowser_refresh();
            return;
        }

        strcpy(image_filename, fb_dir_entry[fb_cursor_pos].fname);

        //new: read complete image
        if ((tracks_read=read_disk(&fd, akt_image_type))>0)
        {
            close_disk_image(&fd);  // we can close the image - everything needed is in ram now.
            akt_track_pos = 0;
            // selected_track = INIT_TRACK << 1;   // select directory-track as default (quicker access)
            // akt_half_track = selected_track;

            send_byte_ready = true;         // enable VIA transfer
            is_image_mount = true;

            send_disk_change();

            start_bytetimer(akt_half_track);    // start the track-spinning

            menu_set_entry_var1(&image_menu, M_WP_IMAGE, floppy_wp);

            set_gui_mode(GUI_INFO_MODE);
        } else {
            close_disk_image(&fd);
            akt_image_type = UNDEF_IMAGE;
            is_image_mount = false;
        }

        break;
    case KEY2_TIMEOUT1:
        set_gui_mode(GUI_MENU_MODE);
        break;
    }

    //// Filename Scrolling
    ++fbup_wait_counter0;

    if((fb_current_line_offset > 0) && (fbup_wait_counter0 >= 300000))
    {
        fbup_wait_counter0 = 0;

        if(0 == fb_line_scroll_end_begin_wait)
        {
            // Es darf gescrollt werden
            if(!fb_line_scroll_direction)
            {
                ++fb_line_scroll_pos;
                if(fb_line_scroll_pos >= fb_current_line_offset)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 1;
                }
            }
            else
            {
                --fb_line_scroll_pos;
                if(fb_line_scroll_pos == 0)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 0;
                }
            }

            display_setcursor(2,fb_cursor_pos);
            display_print(fb_dir_entry[fb_cursor_pos].fname,fb_line_scroll_pos, LCD_LINE_SIZE-3 );
        }
        else
        {
            --fb_line_scroll_end_begin_wait;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// refresh filebrowser view:
// - clear display
// - list max.4 entries from current directory (starting from fb_window_pos)
// - mark directories with a symbol
// - create arrow-up and arrow-down when needed
// - prepare filename-scrolling when too long (for other view)
//
void filebrowser_refresh(void)
{
    display_clear();

    // fr = f_opendir(&dir_object, path);  /* Open the target directory */
    // if (fr == FR_OK) {
    // }
    seek_to_dir_entry(fb_window_pos);

    uint8_t i=0;

    while((i<LCD_LINE_COUNT) && ((fb_window_pos + i) < fb_dir_entry_count))
    {
        fr = f_readdir(&dir_object, &(fb_dir_entry[i]));
        if((fb_dir_entry[i].fname[0]==0) || (fr != FR_OK))
        {
            break;
        }

        display_setcursor(1,i);
        if(fb_dir_entry[i].fattrib & AM_DIR)
        {
            display_data(display_dir_char);
        } else {
            display_data(' ');
        }

        display_print(fb_dir_entry[i].fname, 0, LCD_LINE_SIZE-3);

        i++;
    }

    display_setcursor(0, fb_cursor_pos);
    display_data(display_cursor_char);


    if(fb_window_pos > 0)
    {
        display_setcursor(LCD_LINE_SIZE-1,0);
        display_data(display_more_top_char);
    }

    if((fb_window_pos + LCD_LINE_COUNT) < fb_dir_entry_count)
    {
        display_setcursor(LCD_LINE_SIZE-1, LCD_LINE_COUNT-1);
        display_data(display_more_down_char);
    }

    // Für Scrollenden Filename
    int8_t var = (int8_t)strlen(fb_dir_entry[fb_cursor_pos].fname) - (LCD_LINE_SIZE-3);
    if(var < 0)
        fb_current_line_offset = 0;
    else
        fb_current_line_offset = var;

    fb_line_scroll_pos = 0;
    fb_line_scroll_direction = 0;
    fb_line_scroll_end_begin_wait = 6;
}

/////////////////////////////////////////////////////////////////////

uint16_t get_dir_entry_count(void)
{
    uint16_t entry_count = 0;

//    f_closedir(&dir_object);

    char pattern[] = {"*"};
    char path[] = {""};

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    fr = f_opendir(&dir_object, path);  /* Open the target directory */
    if (FR_OK == fr)
    {
        while(FR_OK == f_readdir(&dir_object, &dir_entry))
        {
            if(0 == dir_entry.fname[0])
            {
                break;
            }
            if(!(dir_entry.fattrib & (AM_SYS | AM_HID )))
            {
                entry_count++;
            }
        }
    }
    return entry_count;
}

/////////////////////////////////////////////////////////////////////

uint16_t seek_to_dir_entry(uint16_t entry_num)
{
    uint16_t entry_count = 0;

    //        fat_reset_dir(dd);
    f_closedir(&dir_object);

    char pattern[] = {"*"};
    char path[] = {""};

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    fr = f_opendir(&dir_object, path);  /* Open the target directory */

    if((FR_OK == fr) || (entry_num != 0))
    {
        while(FR_OK == f_readdir(&dir_object, &dir_entry) && (entry_count < (entry_num-1)))
        {
            if(0 == dir_entry.fname[0])
            {
                break;
            }
            if(!(dir_entry.fattrib & (AM_SYS | AM_HID )))
            {
                entry_count++;
            }
        }
    }
    return entry_count;
}

/////////////////////////////////////////////////////////////////////

void open_disk_image(FIL* fd, FILINFO *file_entry, uint8_t* image_type)
{
    const int namelen = strlen(file_entry->fname);
    if(namelen < 4) return;

    char extension[5];
    int i;
    FRESULT fr = FR_NO_FILE;

    // Extension überprüfen --> g64 oder d64
    strcpy(extension, file_entry->fname+(namelen - 4));

    i=0;
    while(extension[i] != '\0')
    {
        extension[i] = tolower(extension[i]);
        ++i;
    }

    if(!strcmp(extension,".g64"))
    {
        // Laut Extension ein G64
        fr = f_open(fd, file_entry->fname, FA_READ);
        if (FR_OK == fr)
        {
            *image_type = G64_IMAGE;
            open_g64_image(fd);
            enable_write_protection();
        }
    }
    else if(!strcmp(extension,".d64"))
    {
        // Laut Extension ein D64
        fr = f_open(fd, file_entry->fname, FA_READ);
        if (FR_OK == fr)
        {
            *image_type = D64_IMAGE;
            open_d64_image(fd);
            enable_write_protection();
        }
    }

    if (FR_OK != fr)
    {
        // Nicht unterstützt
        f_close(fd);
        *image_type = UNDEF_IMAGE;
    }
    return;
}

/////////////////////////////////////////////////////////////////////

void close_disk_image(FIL* fd)
{
    f_close(fd);
}

/////////////////////////////////////////////////////////////////////

void open_g64_image(FIL* fd)
{
    return;
}

/////////////////////////////////////////////////////////////////////

void open_d64_image(FIL* fd)
{
    // extract DiskID from $165A2+A3 .. better than nothing.
    FSIZE_t offset = (((FSIZE_t) d64_track_offset[DIRECTORY_TRACK]) << 8) + 0xA2;
    uint8_t buffer[2];
    UINT byte_read;
    FRESULT fr;
    fr = f_lseek(fd, offset);
    if (FR_OK == fr)
    {
        fr = f_read(fd, buffer, 2, &byte_read);
        if ((FR_OK == fr) && (2 == byte_read))
        {
            id1 = buffer[0];
            id2 = buffer[1];
        }
    }
    return;
}

/////////////////////////////////////////////////////////////////////

void init_writeprot(void)
{
    gpio_init(GPIO_WPS);
    gpio_set_dir(GPIO_WPS, GPIO_OUT);
    gpio_set_pulls(GPIO_WPS, false, false);
    // remark:
    // 1541 schematics include a 74ls04 which drives WPS low by default
    // having a pull-up enabled is not a good idea
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

void unmount_image(void)
{
    close_disk_image(&fd);
    is_image_mount = 0;
    akt_image_type = UNDEF_IMAGE;
    enable_write_protection();
    menu_set_entry_var1(&image_menu, M_WP_IMAGE, 1);
    send_disk_change();
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
    if(selected_track >= ((MAX_TRACKS-1)<<1)) return;

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

    // Als Eingang schalten
    gpio_init_mask(PAPORT_MASK);
    gpio_set_pulls(GPIO_PAPORT, true, false);
    gpio_set_pulls(GPIO_PAPORT+1, true, false);
    gpio_set_pulls(GPIO_PAPORT+2, true, false);
    gpio_set_pulls(GPIO_PAPORT+3, true, false);
    gpio_set_pulls(GPIO_PAPORT+4, true, false);
    gpio_set_pulls(GPIO_PAPORT+5, true, false);
    gpio_set_pulls(GPIO_PAPORT+6, true, false);
    gpio_set_pulls(GPIO_PAPORT+7, true, false);
    gpio_set_dir_in_masked(PAPORT_MASK); // should set all 8 bits to input

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

///////////////////////////////////////
///////// ISR
///////////////////////////////////////


//ISR (TIMER0_COMPA_vect)
bool repeating_timer_callback(__unused struct repeating_timer *t)
{
    // ISR wird alle 26,28,30 oder 32µs ausfgrufen
    // Je nach dem welche Spur gerade aktiv ist

    uint8_t akt_gcr_byte;
    static uint8_t old_gcr_byte = 0;
    uint8_t is_sync;

    if(get_so_status())
    {
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
                    sleep_us(3);
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
                sleep_us(3);
                set_byte_ready();
            }
        }

        // Daten aus Ringpuffer senden wenn Motor an
        if(get_motor_status())
        {
            // Wenn Motor läuft
            g64_tracks[akt_half_track>>1][akt_track_pos++] = akt_gcr_byte;  // Nächstes GCR Byte schreiben
            track_is_written = true;
            if(akt_track_pos == g64_tracklen[akt_half_track>>1]) akt_track_pos = 0;    // Ist Spurende erreicht? Zurück zum Anfang
        }
    }
    return true;
}
