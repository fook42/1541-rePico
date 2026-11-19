#!/bin/bash
set -euo pipefail

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
    autoconf \
    automake \
    make \
    gcc \
    g++ \
    cmake \
    ninja-build \
    wget \
    unzip       

LIBUSB_INCLUDE_DIR="/usr/include/libusb-1.0"
if [[ ! -f "${LIBUSB_INCLUDE_DIR}/libusb.h" ]]; then
    echo "Missing ${LIBUSB_INCLUDE_DIR}/libusb.h after dependency install" >&2
    exit 1
fi

# Create build directory
mkdir -p build/
cd build/

# Clone OpenOCD repository
if [[ ! -d openocd ]]; then
    git clone --depth 1 https://github.com/raspberrypi/openocd.git
fi
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
if pkg-config --exists libusb-1.0; then
    LIBUSB_CFLAGS="$(pkg-config --cflags libusb-1.0)"
    LIBUSB_LIBS="$(pkg-config --libs libusb-1.0)"
else
    LIBUSB_CFLAGS="-I${LIBUSB_INCLUDE_DIR}"
    LIBUSB_LIBS="-lusb-1.0"
fi

CPPFLAGS="${CPPFLAGS:-} -I${LIBUSB_INCLUDE_DIR}" \
LIBUSB1_CFLAGS="${LIBUSB_CFLAGS}" LIBUSB1_LIBS="${LIBUSB_LIBS}" \
    ./configure --enable-cmsis-dap --disable-werror --with-jimtcl=jimtcl
make -j$(nproc)
sudo make install
