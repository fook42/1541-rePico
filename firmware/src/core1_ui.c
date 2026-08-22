/////////////////////////////////////////////////
// 1541-rePico - Core 1 UI Task
/////////////////////////////////////////////////
// author: F00K42
// date: 2026/08/22
// repo: https://github.com/fook42/1541-rePico
/////////////////////////////////////////////////
//
// Core 1 runs independently to handle all display rendering
// and user input processing, freeing Core 0 for time-critical
// disk I/O and byte transfer operations.
//

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#include "pinout.h"
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
#include "menu_image.h"
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"
#include "main.h"
#include "core1_ui.h"

// Message queue for inter-core communication
// Using a simple ring buffer (256 entries max)
#define UI_MSG_QUEUE_SIZE 256

typedef struct {
    ui_message_t messages[UI_MSG_QUEUE_SIZE];
    volatile uint16_t write_pos;
    volatile uint16_t read_pos;
    spin_lock_t *lock;
} ui_message_queue_t;

static ui_message_queue_t ui_msg_queue = {
    .write_pos = 0,
    .read_pos = 0,
    .lock = NULL
};

// Synchronization
static spin_lock_t *ui_queue_lock = NULL;

/////////////////////////////////////////////////////////////////////
// Initialize Core 1 UI subsystem
/////////////////////////////////////////////////////////////////////
void init_core1_ui(void)
{
    // Allocate a spin lock for thread-safe queue access
    ui_queue_lock = spin_lock_init(spin_lock_claim_unused(true));
    ui_msg_queue.lock = ui_queue_lock;
    
    printf("[Core0] Initializing Core 1 UI task...\n");
    
    // Launch Core 1 with the UI task
    multicore_launch_core1(core1_ui_task);
    
    printf("[Core0] Core 1 UI task launched\n");
}

/////////////////////////////////////////////////////////////////////
// Send message from Core 0 to Core 1
/////////////////////////////////////////////////////////////////////
void send_ui_message(ui_message_type_t msg_type, uint32_t param1, uint32_t param2)
{
    if (ui_queue_lock == NULL) return; // Not initialized
    
    uint32_t save = spin_lock_blocking(ui_queue_lock);
    
    // Check if queue is full
    uint16_t next_write = (ui_msg_queue.write_pos + 1) & (UI_MSG_QUEUE_SIZE - 1);
    if (next_write != ui_msg_queue.read_pos)
    {
        ui_message_t *msg = &ui_msg_queue.messages[ui_msg_queue.write_pos];
        msg->type = msg_type;
        msg->param1 = param1;
        msg->param2 = param2;
        ui_msg_queue.write_pos = next_write;
    }
    
    spin_unlock(ui_queue_lock, save);
}

/////////////////////////////////////////////////////////////////////
// Check if messages available in queue
/////////////////////////////////////////////////////////////////////
uint32_t ui_message_available(void)
{
    if (ui_queue_lock == NULL) return 0;
    return (ui_msg_queue.write_pos != ui_msg_queue.read_pos) ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////
// Get next message from queue (blocking)
/////////////////////////////////////////////////////////////////////
ui_message_t get_ui_message(void)
{
    ui_message_t msg = {UI_MSG_NONE, 0, 0};
    
    if (ui_queue_lock == NULL) return msg;
    
    // Wait for message to arrive
    while (ui_msg_queue.read_pos == ui_msg_queue.write_pos)
    {
        tight_loop_contents();
    }
    
    uint32_t save = spin_lock_blocking(ui_queue_lock);
    
    msg = ui_msg_queue.messages[ui_msg_queue.read_pos];
    ui_msg_queue.read_pos = (ui_msg_queue.read_pos + 1) & (UI_MSG_QUEUE_SIZE - 1);
    
    spin_unlock(ui_queue_lock, save);
    
    return msg;
}

/////////////////////////////////////////////////////////////////////
// Core 1 UI Task Main Loop
/////////////////////////////////////////////////////////////////////
void core1_ui_task(void)
{
    printf("[Core1] UI Task Started\n");
    
    // Initialize display on Core 1
    if (display_init())
    {
        display_home();
    }
    
    show_start_message();
    
    // Initialize input keys on Core 1
    init_key_inputs();
    
    // Setup menus
    menu_init(&main_menu,     main_menu_entrys,     count_of(main_menu_entrys),     LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&image_menu,    image_menu_entrys,    count_of(image_menu_entrys),    LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&settings_menu, settings_menu_entrys, count_of(settings_menu_entrys), LCD_LINE_SIZE, LCD_LINE_COUNT);
    menu_init(&info_menu,     info_menu_entrys,     count_of(info_menu_entrys),     LCD_LINE_SIZE, LCD_LINE_COUNT);
    
    menu_set_root(&main_menu);
    
    sleep_ms(1500);
    
    display_clear();
    display_home();
    
    set_gui_mode(GUI_SELECTOR);
    
    printf("[Core1] UI Task Main Loop Started\n");
    
    // Main UI loop on Core 1
    while (true)
    {
        // Check for messages from Core 0
        if (ui_message_available())
        {
            ui_message_t msg = get_ui_message();
            
            switch (msg.type)
            {
                case UI_MSG_KEY_PRESS:
                    // Button pressed - handled in gpio_callback on Core 1
                    update_gui();
                    break;
                    
                case UI_MSG_UPDATE_DISPLAY:
                    // Refresh display
                    update_gui();
                    break;
                    
                case UI_MSG_SET_GUI_MODE:
                    // Change GUI mode
                    set_gui_mode((uint8_t)msg.param1);
                    break;
                    
                case UI_MSG_TRACK_UPDATE:
                    // Track position changed - update display
                    update_gui();
                    break;
                    
                case UI_MSG_MOTOR_STATUS:
                    // Motor status changed - update display
                    update_gui();
                    break;
                    
                case UI_MSG_IMAGE_MOUNTED:
                    // New image mounted - refresh display
                    update_gui();
                    break;
                    
                case UI_MSG_SHUTDOWN:
                    printf("[Core1] UI Task Shutdown\n");
                    return;
                    
                default:
                    break;
            }
        }
        
        // Continuous UI update (e.g., scrolling text, animations)
        update_gui();
        
        // Yield to prevent Core 1 from starving Core 0
        sleep_us(1000);
    }
}

/////////////////////////////////////////////////////////////////////
