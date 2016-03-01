#!/bin/sh

DEFAULT_PREFIX=/usr/local/bsvc
DEFAULT_WISH=/usr/bin/wish

LINUX_MAKEFILE=Makefile.Linux

echo "Script to compile and install BSVC on Linux"
echo "BSVC will be installed on $DEFAULT_PREFIX)"

export PREFIX=$DEFAULT_PREFIX

# Test if WISH exists
if [ -f $DEFAULT_WISH ];
    then
        echo "WISH is installed correctly on $DEFAULT_WISH"
        export WISH=$DEFAULT_WISH
    else
        echo "WISH not found on $DEFAULT_WISH. Please, install (if needed) and edit this script."
        exit 3
fi


# Compile BSVC
echo "\n\nCompile:"
make -f $LINUX_MAKEFILE

if [ $? -ne 0 ];
    then
        echo "Compilation failed. Exiting."
        exit 1
fi

# Install BSVC
echo "\n\nInstall:"
sudo make -f $LINUX_MAKEFILE install

if [ $? -ne 0 ];
    then
        echo "Installation failed. Exiting."
        exit 2
    else
        # If installation successful -> clean files
        make -f $LINUX_MAKEFILE clean
        echo "Installation Successfull"
        exit 0
fi


