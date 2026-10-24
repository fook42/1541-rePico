/**********************************
 * file - GUI state updates, menu handling and display logic
 *
 * Author: F00K42
 * Last change: 2026/10/24
 ***********************************/

#include "gui.h"

void show_longpress(void)
{
    static uint8_t shown_time_steps;
    uint64_t my_now_time = time_us_64();
    uint64_t my_down_time = key2_down_time;
    char filler;
    if (my_down_time > my_now_time)
    {
        my_down_time -= (my_now_time+1);
        my_now_time = ((uint64_t)-1);
    }

    const uint64_t block_step = TIMEOUT2_KEY2/LCD_LINE_SIZE;
    uint8_t  time_steps = (my_now_time-my_down_time)/block_step;
    filler = display_pointer_char;
    if (time_steps>=LCD_LINE_SIZE) { filler = display_cursor_char; }

    if (shown_time_steps != time_steps)
    {
        display_setcursor(0,2);
        for(int i=0; i<LCD_LINE_SIZE; i++)
        {
            if(i<time_steps)
                display_data(filler);
            else
                display_data(' ');
        }
    }
}

void update_gui(void)
{
    static uint8_t shown_half_track = 255;
    static bool shown_motor_status = false;
    static bool key2_pressed = false;
    static uint32_t wait_counter0 = 0;
    bool new_motor_status;
    uint8_t key_code = get_key_from_buffer();
    char byte_str[8];
    FILINFO next_dir_entry;

    switch (current_gui_mode)
    {
    case GUI_INFO_MODE:

        if(KEY2_UP == key_code)
        {
            key2_pressed = false;
            set_gui_mode(GUI_MENU_MODE);
        } else if(KEY2_TIMEOUT2 == key_code)
        {
            key2_pressed = false;
            // next image...
            if (selected_image_nr<fb_dir_entry_count)
            {
                seek_to_dir_entry(selected_image_nr, current_path);
                FRESULT fr = f_readdir(&dir_object, &next_dir_entry);
                if((0 != next_dir_entry.fname[0]) && (FR_OK == fr))
                {
                    if(!(next_dir_entry.fattrib & AM_DIR))
                    {
                        if (TYPE_VALID == open_dir_entry(next_dir_entry))
                        {
                            ++selected_image_nr;
                            set_gui_mode(GUI_INFO_MODE);
                        }
                    }
                }
            }
        } else if(KEY2_TIMEOUT1 == key_code)
        {
            key2_pressed = false;
        } else if(KEY2_DOWN == key_code)
        {
            key2_pressed = true;
        }
        if (key2_pressed) { show_longpress(); }

        if(shown_half_track != akt_half_track)
        {
            shown_half_track = akt_half_track;
            display_setcursor(disp_trackno_p);
            (void)dez2out((shown_half_track>>1)+1, 2, byte_str);
            display_data(byte_str[0]);
            display_data(byte_str[1]);
        }

        new_motor_status = get_motor_status();
        if(shown_motor_status != new_motor_status)
        {
            shown_motor_status = new_motor_status;
            display_setcursor(disp_motortxt_p);
            if(shown_motor_status)
                display_string(disp_motor_on_s);
            else
                display_string(disp_motor_off_s);
        }

        if((is_image_mount) && (gui_current_line_offset > 0))
        {
            //// Filename Scrolling

            ++wait_counter0;

            if(300000 == wait_counter0)
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

    case GUI_SELECTOR:
        if(KEY2_UP == key_code)
        {
            unmount_image();
            set_gui_mode(GUI_MENU_MODE);
            break;
        } else if(KEY2_TIMEOUT2 == key_code)
        {
            // exit_main = 0;
        }

        handle_menu_image();
       
        if(shown_half_track != akt_half_track)
        {
            shown_half_track = akt_half_track;
            display_setcursor(disp_trackno_p);
            (void)dez2out((shown_half_track>>1)+1, 2, byte_str);
            display_data(byte_str[0]);
            display_data(byte_str[1]);
        }

        new_motor_status = get_motor_status();
        if(shown_motor_status != new_motor_status)
        {
            shown_motor_status = new_motor_status;
            display_setcursor(disp_motortxt_p);
            if(shown_motor_status)
                display_string(disp_motor_on_s);
            else
                display_string(disp_motor_off_s);
        }
        break;

    default:
        break;
    }
}

void check_menu_events(const uint16_t menu_event)
{
    const uint8_t command = (uint8_t) ((menu_event >> 8) & 0xff);
    const uint8_t value = (uint8_t) (menu_event & 0xff);

    FRESULT fr;

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
                case M_MENU_IMAGE:
                    set_gui_mode(GUI_SELECTOR);
                    break;
                case M_LOAD_IMAGE:
                    fr = mount_sdcard(current_path);
                    display_clear();
                    display_home();
                    if (FR_OK == fr)
                    {
                        set_gui_mode(GUI_FILE_BROWSER);
                    } else {
                        display_string("f_mount error:");
                        display_data(fr+'A');
                        show_fs_error(fr);
                        menu_refresh();
                    }
                    break;

                case M_SAVE_IMAGE:
                    if (is_image_mount)
                    {
                        char byte_str[8];
                        int file_op_status;
                        // todo: create save-file dialog, name, type
                        // for now: open a "standard-file" (G64)
                        fr = mount_sdcard(current_path);
                        display_clear();
                        display_home();
                        if (FR_OK != fr)
                        {
                            display_string("f_mount error:");
                            display_data(fr+'A');
                            show_fs_error(fr);
                            menu_refresh();
                            break;
                        }
                        fr = f_open(&fd, "1541-repico.g64", FA_CREATE_ALWAYS|FA_WRITE);
                        display_string("G64 file open...");
                        if (FR_OK == fr)
                        {
                            display_setcursor(0,1);
                            file_op_status = write_disk(&fd, G64_IMAGE, num_max_tracks);
                            if (file_op_status>0)
                            {
                                (void)dez2out(file_op_status+1,2,byte_str);
                                display_data(byte_str[0]);
                                display_data(byte_str[1]);
                                display_string(" Tracks saved ");
                            } else {
                                display_string("Error:");
                                (void)dez2out(-file_op_status,2,byte_str);
                                display_data(byte_str[0]);
                                display_data(byte_str[1]);
                            }
                        } else {
                            display_setcursor(0,1);
                            display_string("failed !");
                        }
                        f_close(&fd);
                        sleep_ms(3000);
                        display_clear();
                        display_home();
                        fr = f_open(&fd, "1541-repico.d64", FA_CREATE_ALWAYS|FA_WRITE);
                        display_string("D64 file open...");
                        if (FR_OK == fr)
                        {
                            display_setcursor(0,1);
                            file_op_status = write_disk(&fd, D64_IMAGE, num_max_tracks);
                            if (file_op_status>0)
                            {
                                (void)dez2out(file_op_status+1,2,byte_str);
                                display_data(byte_str[0]);
                                display_data(byte_str[1]);
                                display_string(" Tracks saved ");
                            } else {
                                display_string("Error:");
                                (void)dez2out(-file_op_status,2,byte_str);
                                display_data(byte_str[0]);
                                display_data(byte_str[1]);
                            }
                        } else {
                            display_setcursor(0,1);
                            display_string("failed !");
                        }
                        f_close(&fd);
                        sleep_ms(3000);
                        umount_sdcard();
                        menu_refresh();
                    }
                    break;

                case M_UNLOAD_IMAGE:
                    unmount_image();
                    umount_sdcard();
                    set_gui_mode(GUI_INFO_MODE);
                    break;

                case M_RELOAD_DISK:
                    if (is_image_mount)
                    {
                        is_image_mount = false;
                        for(uint8_t i=0; i<LCD_LINE_SIZE; i++)
                        {
                            display_setcursor(i,0);
                            display_data(display_cursor_char);
                            sleep_ms(50);
                        }
                        for(uint8_t i=0; i<LCD_LINE_SIZE; i++)
                        {
                            display_setcursor(i,0);
                            display_data(' ');
                            sleep_ms(50);
                        }
                        is_image_mount = true;
                        set_gui_mode(GUI_INFO_MODE);
                    }
                    break;

                case M_WP_IMAGE:
                    if(menu_get_entry_var1(&image_menu, M_WP_IMAGE))
                    {
                        enable_write_protection();
                    } else {
                        disable_write_protection();
                    }
                    set_gui_mode(GUI_INFO_MODE);
                    break;

                /// Settings Menü
                case M_RESTART:
                    // exit_main = 0;
                    menu_refresh();
                    break;

                /// Info Menü
                case M_VERSION_INFO:
                    show_start_message();
                    while(irq_key_value != KEY2_DOWN) {};
                    irq_key_value = NO_KEY;
                    menu_refresh();
                    break;

                case M_SDCARD_INFO:
                    if (FR_OK == mount_sdcard(current_path))
                    {
                        show_sdcard_info_message();
                        while(irq_key_value != KEY2_DOWN) {};
                        irq_key_value = NO_KEY;
                    } else {
                        display_clear();
                        display_home();
                        display_string("f_mount error:");
                        display_data(fr+'A');
                        show_fs_error(fr);
                    }
                    umount_sdcard();
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
    current_gui_mode = gui_mode;
    switch(gui_mode)
    {
        case GUI_INFO_MODE:
            infomode_update();
            break;

        case GUI_MENU_MODE:
            menu_refresh();
            break;

        case GUI_FILE_BROWSER:
            filebrowser_refresh();
            break;

        case GUI_SELECTOR:
            handle_menu_image();
            break;

        default:
            break;
    }
}

void show_start_message(void)
{
    display_clear();
    display_setbright(true);
    display_setcursor(disp_versiontxt_p);
    display_string(disp_versiontxt_s);
    display_setcursor(disp_firmwaretxt_p);
    display_string(disp_firmwaretxt_s);
    display_string(VERSION);
}

/////////////////////////////////////////////////////////////////////

void infomode_update(void)
{
    uint8_t byte_str[3];

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
        {
            var = 0;
        }

        gui_current_line_offset = var;
        gui_line_scroll_pos = 0;
        gui_line_scroll_direction = 0;
        gui_line_scroll_end_begin_wait = 6;
    } else {
        display_string(disp_nofilemounted_s);
    }
}

/////////////////////////////////////////////////////////////////////
