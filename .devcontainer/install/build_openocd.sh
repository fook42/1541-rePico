#!/bin/bash

# Install OpenOCD dependencies
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
    build-essential \
    git \
    libtool \
    libtool-bin \
    pkg-config \
    libusb-1.0-0-dev \
    libftdi-dev \
    libhidapi-dev \
    libudev-dev \
    libusb-dev \
    libusb-1.0-0-dev \
    libftdi-dev \
    libhidapi-dev \
    libudev-dev \
    autoconf \
    automake \
    make \
    gcc \
    g++ \
    cmake \
    ninja-build \
    wget \
    unzip       

# Create build directory
mkdir -p build/
cd build/

# Clone OpenOCD repository
git clone --depth 1 https://github.com/raspberrypi/openocd.git 
cd openocd/

# Initialize and update git submodules (including jimtcl)
git submodule update --init --recursive

# Build jimtcl first
cd jimtcl
./configure
make -j$(nproc)
sudo make install
cd ..

# Configure and build OpenOCD
./bootstrap
./configure --enable-cmsis-dap --disable-werror --with-jimtcl=jimtcl
make -j$(nproc)
sudo make install
