# 1541-rePico
replacement of analog-part of Commodore 1541-Floppy devices based on Pi-Pico(2)

## preface/credits ##
this project was derived from the original 1541-rebuild from Thorsten Kattanek (https://github.com/ThKattanek/1541-rebuild) !
parts of his code were taken from there and adopted for Raspberry Pico and modified to handle disk images differently. Thanks!

the C64 selector-programm was developed and implemented by Peiselulli !

## Features ##

latest version:
- software: 1.5.0
- hardware: 1.6

### Hardware ###

single board, size 59mm x 60 mm

- Raspberry Pico2 socket/solder-pads
- VIA 40pin socket
- levelshifter (BSS138 & 74LVC4245) for stable signals
- connectors for
  - I2C Display
  - SPI SD-Card adapter
  - Rotary-Encoder + Switch (KY-040 compatible)
  - Write-Protect-Signal

one size fits for all 1541 models.

### Software ###

#### Pico2 Firmware ####

- mounting of D64 & G64 & PRG Files
- 35 & 42 track image files supported
- read and write access for both D64 & G64
- sd-card hotswap feature (refresh of directory content)

#### C64 Image Selector ####

- browsing through sd-card content via "virtual disk-image"
  - subdirectory support
  - 4 way scrolling
- highlighting and selecting of different file-types (D64,G64,PRG,Folder)
- simple mounting and also fastloading of selected entry


## how to build ##

### prepare tools (linux) ###

```
sudo apt install cmake ninja-build libusb-1.0-0-dev build-essential pkg-config python3 xxd gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

### pico-sdk ###

from : <https://github.com/raspberrypi/pico-sdk>

- clone this repo
- create a environment variable called "PICO_SDK_PATH" which points to this repo
- place a "export PICO_SDK_PATH= <path to your pico-sdk>" at the end of your .profile script
- then cd into this repo-folder and initialize the mbedtls submodule (picotool mentiones this)

all in one:
```
git clone https://github.com/raspberrypi/pico-sdk.git
export PICO_SDK_PATH=$(pwd)/pico-sdk
echo "export PICO_SDK_PATH=$PICO_SDK_PATH" >> ~/.profile"
cd pico-sdk
git submodule update --init lib/mbedtls
```

### picotool ###

from : <https://github.com/raspberrypi/picotool>

- clone this repo
- install the picotool as described in README.md and BUILDING.md file there

```
git clone https://github.com/raspberrypi/picotool.git
```

### build steps ###

prepare build-folder / configure Ninja for build the cmake project

```
cd firmware
mkdir build
cd build
cmake -G Ninja ..
```

now trigger a build

```
ninja
```


## howto flash ##

connect your pico2 via USB cable and use picotool to flash the firmware

```
picotool load -t uf2 1541-rePico.uf2 -x -f
```


## debugging with OpenOCD ##

### prerequisites ###

- **CMSIS-DAP Compatible Debugger** (e.g., Raspberry Pi Debug Probe, STLink, or SEGGER J-Link) 
get binaries for a pico here : https://github.com/raspberrypi/debugprobe
- **OpenOCD** installed ( built in devcontainer or  with "build_openocd.sh")
- **gdb-multiarch** for GDB debugging ( preinstalled in devcontainer )

### setup ###

1. Connect your debugger to the Pico using the SWD pins:
   - SWCLK → GPIO 24 (or pin 20)
   - SWDIO → GPIO 25 (or pin 21)
   - GND → GND

2. Ensure the debugger is connected via USB to your development machine

### starting OpenOCD ###

Start OpenOCD in a terminal:

```
openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg
```

OpenOCD will listen on port 3333 for GDB connections.

### debugging in VS Code ###

VS Code launch configurations are available:

**Pico Debug (Cortex-Debug)** - Integrated debugging with OpenOCD

#### using Cortex-Debug (recommended) ####

Press `F5` or select "Pico Debug (Cortex-Debug)" from the Run menu. This will:
- Start OpenOCD automatically
- Load your firmware
- Break at `main()`
- Allow step debugging, breakpoints, and variable inspection

#### using external OpenOCD ####

1. Start OpenOCD in a terminal (see above)
2. Select "Pico Debug (Cortex-Debug with external OpenOCD)" from Run menu
3. VS Code will connect to the running OpenOCD instance

### useful GDB commands ###

When debugging, common GDB commands include:

```
continue         # Resume execution
step             # Step into function
next             # Step over function
break <func>     # Set breakpoint at function
break <file>:<line>  # Set breakpoint at file:line
print <var>      # Print variable value
display <var>    # Show variable on each step
backtrace        # Show call stack
```

### troubleshooting ###

**"Device not found"**: 
- Check CMSIS-DAP debugger is connected
- Verify SWD pin connections
- Try `openocd -d` for debug output
- Connect the USB for the targetdevice, then the USB for debugprobe
( to keep correct order of /dev/ttyACMx )

**GDB connection failed**: 
- Ensure OpenOCD is running on port 3333
- Check firewall settings

# documentation #

## project documentation ##

t.b.d.

## supported disk formats ##

currently [D64](/doc/D64.TXT) and [G64](/doc/G64.TXT) are supported as for reading and writing.  
as bonus, **PRG** files can be loaded by an on-the-fly routine that creates a valid D64 image out of it.

additionaly a small tool [conv_x64](/tools) is provided that allows easy conversion from one to the other on you linux-pc.

## Pictures ##

impressions of current setup and PCB can be found [here](/doc/pictures).

## Links ##

- great collection of basics about disk-image-formats can be found here:  
<https://ist.uwaterloo.ca/~schepers/formats.html>

- forum64 project discussion thread (base of 1541-rebuild and 1541-rePico)  
<https://www.forum64.de/index.php?thread/59884-laufwerk-der-1541-emulieren>

