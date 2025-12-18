# 1541-rePico
replacement of analog-part of Commodore 1541-Floppy devices based on Pi-Pico(2)

## prepare tools (linux) ##

```
sudo apt install cmake ninja-build
```


### picotool

from : <https://github.com/raspberrypi/picotool>



## howto build ##

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




