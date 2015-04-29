BSVC: A Microprocessor Simulation Framework
===========================================

BSVC is a microprocessor simulation framework written in C++ and Tcl/Tk.
It was developed as a senior design project at North Carolina State
University by Bradford W. Mott (see the Credits.txt file for copyright
information).

Since its original development many professors and students have found
BSVC to be a useful tool in courses based on the Motorola 68000.  It has
also been used as a starting point for other senior design projects.

In April 2015, maintenance was restarted by Dan Cross and the system was
relicensed under the GNU Public License, version 2.1.

The BSVC distribution contains the following:

* Motorola 68000 simulator & assembler (Supports the M68681 Dual UART)
* Motorola 68360 simulator (CPU32 like simulator)
* Simple devices including the M68681 DUART, a timer, RAM and a GDB interface
* BSVC Graphical User Interface (written in Tcl/Tk)
* BSVC Simulator Framework (C++ classes)

Code cross-compiled with GCC is known to load and run.

Supported Systems
-----------------
The BSVC distribution is known to compile on the following systems:

* Linux
* Solaris
* Mac OS X
* FreeBSD

Any reasonable POSIX system should work.

Required Software
-----------------
BSVC requires the following software to compile and run:

* C++11 and C11 compliant compilers for C++ and C, respectively.
  Both clang and recent versions of GCC are known to work.
* GNU Make
* A recent version of Tcl/Tk

Distribution Site
-----------------
The BSVC distribution can be downloaded from Github:

  https://github.com/BSVC/bsvc

Copyright and License
---------------------
BSVC is Copyright (C) 1993 - 1998 by Bradford W. Mott

BSVC is free software. It is released under the terms of the GNU
General Public License, version 2.1.

Contacts
--------
For more information see the BSVC site on Github.

If you have any questions regarding BSVC send mail to:

* bsvc-users@googlegroups.com
  (https://groups.google.com/forum/#!forum/bsvc-users)

If you want to receive email announcements about future releases,
subscribe to:

* bsvc-announce@googlegroups.com
  (https://groups.google.com/forum/#!forum/bsvc-announce)
