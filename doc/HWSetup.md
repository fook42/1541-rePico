
# 1541-rePico PCB-Assembly and Hardware Setup

(Hint: click on images for a larger version)

## PCB soldering

### 1. lets start with the given PCB (here you see a exemplary one with SMD parts populated)

[<img src="pictures/setup/00.PCB_Version17.jpg" alt="PCB Version 1.7" width="20%"/>](pictures/setup/00.PCB_Version17.jpg)

### 2. install the precision pinheaders which are used to mount into the VIA-socket on your 1541

to keep them in place (right distance/height), you may use some ic-socket as shown here:\
[<img src="pictures/setup/01.pinheader_bottom.jpg" alt="pinheader bottom" width="20%"/>](pictures/setup/01.pinheader_bottom.jpg)
[<img src="pictures/setup/01.pinheader_helper.jpg" alt="pinheader helper" width="20%"/>](pictures/setup/01.pinheader_helper.jpg)

and this is how it looks from the top:\
[<img src="pictures/setup/01.pinheader_top.jpg" alt="pinheader top" width="20%"/>](pictures/setup/01.pinheader_top.jpg)

### 3. solder VIA socket, 10k resistor and reset-switch

[<img src="pictures/setup/02.socket_switch_resistor.jpg" alt="pinheader top" width="20%"/>](pictures/setup/02.socket_switch_resistor.jpg)

### 4. pico2 socket strips

its helpful to install the pico2 there already to align these sockets\
[<img src="pictures/setup/03.PicoSocket.jpg" alt="pinheader top" width="20%"/>](pictures/setup/03.PicoSocket.jpg)
[<img src="pictures/setup/03.PicoSocket_Pico2.jpg" alt="pinheader top" width="20%"/>](pictures/setup/03.PicoSocket_Pico2.jpg)

### 5. lots of pinheaders

this is what needs to be soldered:\
[<img src="pictures/setup/04.Pinheader_all.jpg" alt="pinheader top" width="20%"/>](pictures/setup/04.Pinheader_all.jpg)

when you're done it looks like this:\
[<img src="pictures/setup/04.Pinheader_installed.jpg" alt="pinheader top" width="20%"/>](pictures/setup/04.Pinheader_installed.jpg)
[<img src="pictures/setup/04.Pinheader_top.jpg" alt="pinheader top" width="20%"/>](pictures/setup/04.Pinheader_top.jpg)

### 6. final parts: buzzer and capacitor

[<img src="pictures/setup/05.buzzer_cap.jpg" alt="pinheader top" width="20%"/>](pictures/setup/05.buzzer_cap.jpg)
[<img src="pictures/setup/05.buzzer_cap_top.jpg" alt="pinheader top" width="20%"/>](pictures/setup/05.buzzer_cap_top.jpg)

## installation into 1541-1

### replace VIA with rePico

you need to identify the right VIA, which needs to be pulled off your 1541 and placed into the socket of your 1541-rePico.\
[<img src="pictures/setup/06.installed1541-1.jpg" alt="pinheader top" width="20%"/>](pictures/setup/06.installed1541-1.jpg)

this table may help you to find out, which VIA chip is the right one:
| floppy-device | VIA IC-designator |
| ------------- | ----------------- |
| 1541-1 longboard / PCB 251830 | UC2 |
| 1541-1 shortboard / PCB 251854 | UC3 |
| 1541-2 / ASSY 340503 | U8 |

### cables

now have a look at the cables:\
[<img src="pictures/setup/07.cabling-1541-1.jpg" alt="pinheader top" width="20%"/>](pictures/setup/07.cabling-1541-1.jpg)

---
important: the "red" cable connects the original WriteProtect-signal of your 1541 to the single-pinheader J5 of rePico.\
its connected to ...
| floppy-device | VIA IC-designator |
| ------------- | ----------------- |
| 1541-1 longboard / PCB 251830 | P6 - Pin 12 |
| 1541-1 shortboard / PCB 251854 | P3 - Pin 12 |
| 1541-2 / ASSY 340503 | CN4 - Pin 3 (where Pin 1+2 are not populated) |

---


and here you find the same cables on your peripherals (display, sdcard, rotary)\
[<img src="pictures/setup/07.cabling-display.jpg" alt="pinheader top" width="20%"/>](pictures/setup/07.cabling-display.jpg)
[<img src="pictures/setup/07.cabling-sdcard.jpg" alt="pinheader top" width="20%"/>](pictures/setup/07.cabling-sdcard.jpg)
[<img src="pictures/setup/07.cabling-rotary.jpg" alt="pinheader top" width="20%"/>](pictures/setup/07.cabling-rotary.jpg)

### 3d-printed front

thanks to [Mares74](https://github.com/Mares74) we have a nice replacement front available:\
[<img src="pictures/setup/08.3d-front-1541.jpg" alt="pinheader top" width="20%"/>](pictures/setup/08.3d-front-1541.jpg)
[<img src="pictures/setup/08.3d-front-back.jpg" alt="pinheader top" width="20%"/>](pictures/setup/08.3d-front-back.jpg)
