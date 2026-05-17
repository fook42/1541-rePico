# 1541-rePico
replacement of analog-part of Commodore 1541-Floppy devices based on Pi-Pico(2)

## preface ##
this project was derived from the original 1541-rebuild from Thorsten Kattanek (https://github.com/ThKattanek/1541-rebuild) !
parts of his code were taken from there and adopted for Raspberry Pico and modified to handle disk images differently. Thanks!


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


# documentation #

## project documentation ##

t.b.d.

## supported disk formats ##

currently [D64](/doc/D64.TXT) and [G64](/doc/G64.TXT) are supported as for reading and writing.  
additionaly a small tool [conv_x64](/tools) is provided that allows easy conversion from one to the other on you linux-pc.

## Pictures ##

impressions of current setup and PCB can be found [here](/doc/pictures).

## Links ##

- great collection of basics about disk-image-formats can be found here:  
<https://ist.uwaterloo.ca/~schepers/formats.html>

- forum64 project discussion thread (base of 1541-rebuild and 1541-rePico)  
<https://www.forum64.de/index.php?thread/59884-laufwerk-der-1541-emulieren>

