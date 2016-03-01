make -f Makefile.Linux clean
export PREFIX=/usr/local/bsvc
export WISH=/usr/bin/wish  
make -f Makefile.Linux
echo "\n\nInstalar:\n"
sudo make -f Makefile.Linux install

