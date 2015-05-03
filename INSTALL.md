BSVC "A Microprocessor Simulation Framework"
============================================

I've tested BSVC under the following operating systems:

    Linux  2.x
    Solaris (SunOS 5.5.1)

The makefiles for these system should work.  I'm sure there are still 
bugs in BSVC so if you find any please let me know.

Installation
------------

1. Edit the appropriate operating system-specific Makefile in the src/
   directory:

   - Change PREFIX to where you want BSVC installed.

   - Change WISH to the path/filename of the Tcl/Tk "wish"
     executable.  If you don't know where wish is then try
     `which wish` and see if it's in your path anywhere.

   - You can also customize compilers and other tools used
     during the build.

2. Build the BSVC executables:

   - Using the correct makefile for your system, run GNU Make:

         `make -f Makefile.your_os`     (e.g for Linux use Makefile.linux)

     or

         `gmake -f Makefile.your_os`    (if GNU make is called `gmake`)

   - If there is no makefile for your system then you'll have to make
     one yourself.  If you do then please send it to me so I can include
     it in the distribution.

3. Install the BSVC executable:

   - To install the BSVC executables do:

         `make -f Makefile.your_os install`


Testing the installation
------------------------

After you have built the programs there are some sample setups and programs
for the Motorola 68000 simulator in the bsvc/samples/m68000 directory.

Run 'bsvc' from the $BINDIR directory to start the user interface.
Then choose "File / Open Setup..." and select the "serial.setup" in the 
bsvc/samples/m68000 directory.  After the system starts up choose 
"File / Load Program..." and load the "tutor.h68" program.  Press "Reset"
and then press "Run" in about 10 seconds you should see "TUTOR >" in the
terminal window that appeared.

I would suggest adding the $BINDIR directory to your PATH if you're going
to be using it very often.

There's a postscript version of the user manual in the bsvc/doc/manual
directory.

If you have any questions or comments send email to the mailing list:

  bsvc-users@googlegroups.com

If you'd like announcements about new versions, etc, subscribe to the
announcements list:

  bsvc-announce@googlegroups.com
