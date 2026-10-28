#include "files.h"

/////////////////////////////////////////////////////////////////////

FRESULT mount_sdcard(const char* mount_path)
{
    BYTE mount_option = 1; /* 0=Do not mount (delayed mount), 1=Mount immediately */

    FRESULT fr = f_mount(&fs, mount_path, mount_option);
    if (FR_OK == fr)
    {
        fb_dir_entry_count = get_dir_entry_count(mount_path); // open card, count entries on root level
    }
    return fr;
}

FRESULT umount_sdcard(void)
{
    // reset path for next sd-card mount
    current_path[0]='/';
    current_path[1]=0;
    return f_unmount("");
}

void show_fs_error(FRESULT error_code)
{
    // display_string("f_mount error:");
    // display_data(error_code+'A');
    const char* error_txt=FRESULT_str(error_code);
    size_t err_str_len=strlen(error_txt);
    uint8_t err_str_offset=0;
    irq_key_value = NO_KEY;
    do
    {
        display_setcursor(0, 1);
        display_print(error_txt, err_str_offset++, 16);
        sleep_ms(333);
    }
    while (((err_str_offset+15)<err_str_len) && (irq_key_value != KEY2_DOWN));

    while(irq_key_value != KEY2_DOWN) {};
    irq_key_value = NO_KEY;
    return;
}
/////////////////////////////////////////////////////////////////////

void handle_menu_image(void)
{
    static uint8_t datafile_blklimit = 64;  // set initial limit to 64*256 bytes = 16kb
    FILINFO hmi_dir_entry;

    if (SELECTOR_IMAGE != akt_image_type)
    {
        // insert the virtual menu-image
        insert_menu_image(current_path, datafile_blklimit);
        infomode_update();
    } else {
        // we have the selector inserted.. now handle the selection
        if (track_is_written)
        {
            if (DIRECTORY_TRACK == track_write_nr)
            {
                // something was changed on the image.. lets fetch the image-number
                // waste some time to make sure a complete block has been written
                display_setcursor(disp_scrollfilename_p);
                for(uint8_t i=0; i<LCD_LINE_SIZE; i++)
                {
                    display_data(display_cursor_char);
                    sleep_ms(250/LCD_LINE_SIZE);
                }
                display_setcursor(disp_scrollfilename_p);
                for(uint8_t i=0; i<LCD_LINE_SIZE; i++)
                {
                    display_data(' ');
                    sleep_ms(250/LCD_LINE_SIZE);
                }
                // simple approach: convert the complete track, all 19 sectors.. then select sector 2 and read 2 bytes
                convert_gcr2d64track(DIRECTORY_TRACK);
                selected_image_nr = *((uint16_t*) &d64_sector_puffer[1+2*D64_SECTOR_SIZE]);
                datafile_blklimit = d64_sector_puffer[3+2*D64_SECTOR_SIZE];

                if (0 != selected_image_nr)
                {
                    FRESULT fr;

                    if ((1 < strlen(current_path)) && (1 == selected_image_nr))
                    {
                        // first entry selected, which is ".." in this case
                        // create a fake dir-entry and open it afterwards
                        strcpy(hmi_dir_entry.fname, "..");
                        hmi_dir_entry.fattrib = AM_DIR;
                        fr = FR_OK;
                    } else {
                        seek_to_dir_entry(selected_image_nr-1, current_path);
                        fr = f_readdir(&dir_object, &hmi_dir_entry);
                    }

                    if((0 != hmi_dir_entry.fname[0]) && (FR_OK == fr))
                    {
                        if (TYPE_VALID != open_dir_entry(hmi_dir_entry))
                        {
                            // no valid image available / or we jumped into a folder
                            is_image_mount=false;
                            //rebuild the data-file
                            insert_menu_image(current_path, datafile_blklimit);
                            infomode_update();
                        } else
                        {
                            set_gui_mode(GUI_INFO_MODE);
                        }
                    }
                }
            }
            track_is_written = false;
        }
    }
}

void insert_menu_image(char* menu_path, const uint8_t max_blocks)
{
    FRESULT fr = mount_sdcard(menu_path);
    if (FR_OK == fr)
    {
        (void) f_closedir(&dir_object);

        char pattern[] = {"*"};

        dir_object.pat = pattern;           /* Save pointer to pattern string */
        fr = f_opendir(&dir_object, menu_path);  /* Open the target directory */

        if(FR_OK == fr)
        {
            stop_bytetimer();
            send_byte_ready = false;            // disable VIA transfer

            create_menu_image(menu_path, &dir_object, &id1, &id2, &num_max_tracks, image_filename, max_blocks);

            akt_track_pos = 0;
            selected_track = (INIT_TRACK << 1);
            akt_half_track = selected_track;

            send_byte_ready = true;             // enable VIA transfer
            is_image_mount = true;

            akt_image_type = SELECTOR_IMAGE;    // to identify the write-back-channel handling
            track_is_written = false;

            disable_write_protection();         // we need to be able to receive the answer of menu-selector as "write"

            send_disk_change();

            start_bytetimer(akt_half_track);    // start the track-spinning

            menu_set_entry_var1(&image_menu, M_WP_IMAGE, floppy_wp);
        } else {
            display_clear();
            display_home();
            display_string("f_opendir :");
            display_data(fr+'A');
            show_fs_error(fr);
        }
    } else {
        display_clear();
        display_home();
        display_string("f_mount error:");
        display_data(fr+'A');
        show_fs_error(fr);
    }
}

/////////////////////////////////////////////////////////////////////


void filebrowser_update(uint8_t key_code)
{
    static uint32_t fbup_wait_counter0 = 0;

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
        if((fb_cursor_pos < (LCD_LINE_COUNT-1)) && (fb_cursor_pos < (fb_dir_entry_count-1)))
        {
            ++fb_cursor_pos;
            filebrowser_refresh();
        }
        else
        {
            if(fb_window_pos < (fb_dir_entry_count - LCD_LINE_COUNT))
            {
                ++fb_window_pos;
                filebrowser_refresh();
            }
        }
        break;
    case KEY2_UP:
        //fn open dir_entry...
        uint8_t ode_return = open_dir_entry(fb_dir_entry[fb_cursor_pos]);
        if (TYPE_VALID != ode_return)
        {
            // no valid image available / or we jumped into a folder
            if (TYPE_NONE == ode_return)
            {
                display_clear();
                display_setcursor(disp_unsupportedimg_p);
                display_string(disp_unsupportedimg_s);
                sleep_ms(1000);
            }
            is_image_mount=false;
            filebrowser_refresh();
        } else {
            selected_image_nr = fb_window_pos+fb_cursor_pos+1;
            set_gui_mode(GUI_INFO_MODE);
        }
        break;
    case KEY2_TIMEOUT1:
        set_gui_mode(GUI_MENU_MODE);
        break;
    case KEY2_TIMEOUT2:
        // move up one directory level if possible
        if (1 < strlen(current_path))
        {
            FILINFO fbu_dir_entry;
            strcpy(fbu_dir_entry.fname, "..");
            fbu_dir_entry.fattrib = AM_DIR;
            (void) open_dir_entry(fbu_dir_entry);
            is_image_mount=false;
            filebrowser_refresh();
        }
        break;

    default:
    }

    //// Filename Scrolling
    ++fbup_wait_counter0;

    if((0 < fb_current_line_offset) && (300000 <= fbup_wait_counter0))
    {
        fbup_wait_counter0 = 0;

        if(0 == fb_line_scroll_end_begin_wait)
        {
            // Es darf gescrollt werden
            if(!fb_line_scroll_direction)
            {
                ++fb_line_scroll_pos;
                if(fb_current_line_offset <= fb_line_scroll_pos)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 1;
                }
            }
            else
            {
                --fb_line_scroll_pos;
                if(0 == fb_line_scroll_pos)
                {
                    fb_line_scroll_end_begin_wait = 6;
                    fb_line_scroll_direction = 0;
                }
            }

            display_setcursor(1,fb_cursor_pos);
            display_data('\xFF');
            display_print(fb_dir_entry[fb_cursor_pos].fname,fb_line_scroll_pos, LCD_LINE_SIZE-2 );
            display_setcursor(0,fb_cursor_pos);
            display_setinverse(true);
        }
        else
        {
            --fb_line_scroll_end_begin_wait;
        }
    }
}

uint8_t open_dir_entry(FILINFO od_file_entry)
{
    int8_t last_track_read;

    stop_bytetimer();
    send_byte_ready = false;      // this blocks current transfers to the VIA
    close_disk_image(&fd);

    if(od_file_entry.fattrib & AM_DIR)
    {
        // selected entry seems to be a directory
        if (0 == strcmp(od_file_entry.fname, ".."))
        {
            // parent directory selected .. so we go one level up
            char* last_slash = strrchr(current_path,'/');
            if (NULL != last_slash)
            {
                *last_slash = 0;
            }
            if (1 > strlen(current_path))
            {
                current_path[0]='/';
                current_path[1]=0;
            }
        } else {
            // append new filder-name to the existing path
            if (1 < strlen(current_path))
            {
                strcat(current_path, "/");
            }
            strcat(current_path, od_file_entry.fname);
        }
        f_chdir(current_path);
        fb_dir_entry_count = get_dir_entry_count(current_path);

        fb_cursor_pos = 0;
        fb_window_pos = 0;
        return TYPE_DIR;
    }

    akt_image_type = open_disk_image(&fd, &od_file_entry);

    if(UNDEF_IMAGE == akt_image_type)
    {
        fd.obj.fs = 0;
    }

    if(0 == fd.obj.fs)
    {
        return TYPE_NONE;
    }

    strcpy(image_filename, od_file_entry.fname);

    // read complete image
    if ((last_track_read=read_disk(&fd, akt_image_type, od_file_entry ))>0)
    {
        close_disk_image(&fd);  // we can close the image - everything needed is in ram now.
        akt_track_pos = 0;

        send_byte_ready = true;         // enable VIA transfer
        is_image_mount = true;
        num_max_tracks = last_track_read+1;

        enable_write_protection();      // this is the default
        if (0 == (od_file_entry.fattrib & AM_RDO))
        {
            disable_write_protection();
        }

        send_disk_change();

        start_bytetimer(akt_half_track);    // start the track-spinning

        menu_set_entry_var1(&image_menu, M_WP_IMAGE, floppy_wp);
    } else {
        close_disk_image(&fd);
        akt_image_type = UNDEF_IMAGE;
        is_image_mount = false;
        return TYPE_NONE;
    }
    return TYPE_VALID;    // dont know which type we opened, but it was okay
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

    seek_to_dir_entry(fb_window_pos, current_path);

    uint8_t i=0;

    if ((1 < strlen(current_path)) && (0 == fb_window_pos))
    {
        strcpy(fb_dir_entry[0].fname, "..");
        fb_dir_entry[0].fattrib = AM_DIR;
        ++i;
    }

    while((i<LCD_LINE_COUNT) && ((fb_window_pos + i) < fb_dir_entry_count))
    {
        FRESULT fr = f_readdir(&dir_object, &(fb_dir_entry[i]));
        if((0 == fb_dir_entry[i].fname[0]) || (FR_OK != fr))
        {
            break;
        }
        ++i;
    }

    for (uint8_t j=0; j<i; j++)
    {
        display_setcursor(0,j);
        if (fb_cursor_pos == j)
        {
            display_setinverse(true);
        }
        if(fb_dir_entry[j].fattrib & AM_DIR)
        {
            display_data(display_dir_char);
        } else {
            display_data(' ');
        }

        display_print(fb_dir_entry[j].fname, 0, LCD_LINE_SIZE-2);
    }

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

    display_setcursor(0, fb_cursor_pos);
    display_setinverse(true);
    // display_data(display_pointer_char);

    // Für Scrollenden Filename
    int8_t var = (int8_t)strlen(fb_dir_entry[fb_cursor_pos].fname) - (LCD_LINE_SIZE-2);
    if(var < 0)
        fb_current_line_offset = 0;
    else
        fb_current_line_offset = var;

    fb_line_scroll_pos = 0;
    fb_line_scroll_direction = 0;
    fb_line_scroll_end_begin_wait = 6;
}

/////////////////////////////////////////////////////////////////////

uint16_t get_dir_entry_count(const char* entrycount_path)
{
    uint16_t entry_count = 0;

    f_closedir(&dir_object);

    char pattern[] = {"*"};

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    if (FR_OK == f_opendir(&dir_object, entrycount_path))  /* Open the target directory */
    {
        FILINFO gdec_dir_entry;
        while(FR_OK == f_readdir(&dir_object, &gdec_dir_entry))
        {
            if(0 == gdec_dir_entry.fname[0])
            {
                break;
            }
            ++entry_count;
        }
    }
    if (1 < strlen(entrycount_path)) { ++entry_count; }
    return entry_count;
}

/////////////////////////////////////////////////////////////////////

uint16_t seek_to_dir_entry(uint16_t entry_num, const char* seek_path)
{
    f_closedir(&dir_object);

    char pattern[] = {"*"};
    // if we are in a subfolder and not the first entry (="..") was selected, decrease the seek index by 1
    if ((1 < strlen(seek_path)) && (0 < entry_num)) { --entry_num; }

    dir_object.pat = pattern;           /* Save pointer to pattern string */
    if(FR_OK == f_opendir(&dir_object, seek_path))  /* Open the target directory */
    {
        f_readdir(&dir_object, 0);  // rewind the directory
        while (0 < entry_num)
        {
            FILINFO seek_dir_entry;
            FRESULT fr = f_readdir(&dir_object, &seek_dir_entry);
            if((FR_OK != fr) || (0 == seek_dir_entry.fname[0]))
            {
                break;
            }
            --entry_num;
        }
    }
    return entry_num;
}

/////////////////////////////////////////////////////////////////////

uint8_t open_disk_image(FIL* fd, FILINFO *file_entry)
{
    size_t namelen;
    char extension[5];
    uint8_t image_type;

    namelen = strlen(file_entry->fname);
    if(4 > namelen) return UNDEF_IMAGE;

    // check the extension for a supported type (d64, g64, prg)
    namelen -= 4; // move copy-pointer 4 backwards
    for (int i=0; i<5; i++)
    {
        extension[i] = tolower(file_entry->fname[namelen+i]);
    }

    if(0 == strcmp(extension,".g64"))
    {
        image_type = G64_IMAGE;
    }
    else if(0 == strcmp(extension,".d64"))
    {
        image_type = D64_IMAGE;
    }
    else if(0 == strcmp(extension,".prg"))
    {
        image_type = PRG_IMAGE;
    } else {
        // extension unknown -> we wont try to open the file at all..
        return UNDEF_IMAGE;
    }

    if (FR_OK != f_chdir(current_path))
    {
        return UNDEF_IMAGE;
    }
    if (FR_OK != f_open(fd, file_entry->fname, FA_READ))
    {
        // image could not be opened for reading
        f_close(fd);
        return UNDEF_IMAGE;
    }
    return image_type;
}

/////////////////////////////////////////////////////////////////////

void close_disk_image(FIL* fd)
{
    f_close(fd);
}

/////////////////////////////////////////////////////////////////////
