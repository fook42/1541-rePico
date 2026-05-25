#!/bin/bash

#
# Users host Gitconfig is copied by default into the devcontainer which can lead to
# issues with local paths. Therefore, some settings are removed or overwritten
# accordingly. See also
#   https://github.com/microsoft/vscode-remote-release/issues/4632
#   https://github.com/microsoft/vscode-remote-release/issues/4603
#

sudo usermod -aG plugdev vscode
WS=`pwd`
echo "Working in $WS"
sudo cp $WS/.devcontainer/install/*.rules /etc/udev/rules.d/
echo "checking for openocd for RP2350"
if [[ ! -f build/openocd_rp2350 ]]; then
    echo "Installing OpenOCD for RP2350"
    $WS/.devcontainer/install/build_openocd.sh 
    touch build/openocd_rp2350
fi
cd /usr/bin

echo "checking for picotool with load function"
if [[ ! -f picotool ]]; then
    sudo cp $WS/.devcontainer/install/picotool .
fi

echo "checking for multiarch binaries"
if [[ ! -e objdump-multiarch ]]; then
    echo "Ubuntun 24.04 has no special multiarch binaries anymore"
    sudo ln -s /usr/bin/objdump objdump-multiarch  
    sudo ln -s /usr/bin/nm nm-multiarch 
fi
cd -
