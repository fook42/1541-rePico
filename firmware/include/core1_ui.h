/**********************************
 * header - Core 1 UI task management
 *
 * Author: F00K42
 * Date: 2026/08/22
 * 
 * Core 1 handles all display rendering and user input processing.
 * Core 0 handles time-critical disk I/O and byte transfer.
 ***********************************/

#ifndef _CORE1_UI_H_
#define _CORE1_UI_H_

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

// Message types for inter-core communication
typedef enum {
    UI_MSG_NONE = 0,
    UI_MSG_KEY_PRESS,           // Button/key input
    UI_MSG_UPDATE_DISPLAY,      // Request display refresh
    UI_MSG_SET_GUI_MODE,        // Change GUI mode
    UI_MSG_TRACK_UPDATE,        // Track position changed
    UI_MSG_MOTOR_STATUS,        // Motor on/off
    UI_MSG_IMAGE_MOUNTED,       // New disk image mounted
    UI_MSG_SHUTDOWN             // Shutdown Core 1
} ui_message_type_t;

// Message structure for Core 1 communication
typedef struct {
    ui_message_type_t type;
    uint32_t param1;
    uint32_t param2;
} ui_message_t;

// Core 1 main task entry point
void core1_ui_task(void);

// Initialize Core 1 and message queue
void init_core1_ui(void);

// Send a message from Core 0 to Core 1
void send_ui_message(ui_message_type_t msg_type, uint32_t param1, uint32_t param2);

// Check if UI message queue has pending messages (for Core 0)
uint32_t ui_message_available(void);

// Get next UI message from queue (blocking)
ui_message_t get_ui_message(void);

#endif // _CORE1_UI_H_
