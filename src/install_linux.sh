#!/bin/sh

DEFAULT_PREFIX=/usr/local/bsvc
DEFAULT_WISH=/usr/bin/wish

LINUX_MAKEFILE=Makefile.Linux

echo ""
echo "Script to compile and install BSVC on Linux"
echo "Please, input where bsvc will be installed (Default: $DEFAULT_PREFIX)"
read console_prefix
if [ -z $console_prefix ];
    then
        export PREFIX=$DEFAULT_PREFIX
    else
        export PREFIX=$console_prefix
fi

if [ -f $DEFAULT_WISH ];
    then
        echo "WISH is installed correctly on $DEFAULT_WISH"
        export WISH=$DEFAULT_WISH
    else
        echo "WISH not found on $DEFAULT_WISH. Please, install it and enter new location:"
        read console_wish
        export WISH=$console_wish
fi
#echo $PREFIX
#echo $WISH

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


