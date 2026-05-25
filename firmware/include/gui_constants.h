/**********************************
 * header - GUI constants and layout
 *
 * original Author: Th.Kathanek
 * modified/adopted: F00K42
 * Last change: 2026/05/25
***********************************/
#ifndef GUI_CONSTANCY_H
#define GUI_CONSTANCY_H

#define TIMEOUT1_KEY2  250000llu // 250ms
#define TIMEOUT2_KEY2 1000000llu // 1s

enum INPUT_MODE{INPUT_MODE_BUTTON, INPUT_MODE_ENCODER};
enum KEY_CODES{KEY0_DOWN, KEY0_UP, KEY1_DOWN, KEY1_UP, KEY2_DOWN, KEY2_UP, KEY2_TIMEOUT1, KEY2_TIMEOUT2, NO_KEY};
enum GUI_MODE{GUI_INFO_MODE, GUI_MENU_MODE, GUI_FILE_BROWSER, GUI_SELECTOR};

// config of Display texts

// config of Display texts
#define disp_tracktxt_p       0,0
#define disp_tracktxt_s       "T:"
#define disp_trackno_p        2,0

#define disp_motortxt_p       5,0
#define disp_motor_on_s       "Motor"
#define disp_motor_off_s      "     "

#define disp_writeprottxt_p   11,0
#define disp_writeprot_on_s   "WProt"
#define disp_writeprot_off_s  "     "

#define disp_scrollfilename_p 0,1
#define disp_nofilemounted_s  "No Image Mounted"
#define disp_unsupportedimg_p 0,0
#define disp_unsupportedimg_s "Img unsupported!"

#define disp_versiontxt_p     0,0
#define disp_versiontxt_s     "- 1541-rePico -"
#define disp_firmwaretxt_p    0,1
#define disp_firmwaretxt_s    "Firmware:"

#define disp_sdinfo_manuf_p   0,0
#define disp_sdinfo_manuf_s   "MANU:"
#define disp_sdinfo_oem_p     8,0
#define disp_sdinfo_oem_s     "OEM: "
#define disp_sdinfo_prod_p    0,1
#define disp_sdinfo_prod_s    ""
#define disp_sdinfo_size_p    8,1
#define disp_sdinfo_size_s    "SD-Size:"
#define disp_sdinfo_rev_p     0,0
#define disp_sdinfo_rev_s     "Rev:"
#define disp_sdinfo_serial_p  8,0
#define disp_sdinfo_serial_s  ""
#define disp_sdinfo_part_p    0,1
#define disp_sdinfo_part_s    "Part:"

#endif // GUI_CONSTANCY_H
