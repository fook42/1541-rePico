# 1541-rePico
replacement of analog-part of Commodore 1541-Floppy devices based on Pi-Pico(2)

## preface ##
this project was derived from the original 1541-rebuild from Thorsten Kattanek (https://github.com/ThKattanek/1541-rebuild) !
parts of his code were taken from there and adopted for Raspberry Pico and modified to handle disk images differently. Thanks!


## how to build ##

### prepare tools (linux) ###

```
sudo apt install cmake ninja-build
```


### picotool ###

from : <https://github.com/raspberrypi/picotool>



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

