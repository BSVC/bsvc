BSVC Changes
============

2.2 to 2.1 (Apr 30, 2015)
-------------------------
BSVC 2.2 is the first major release since 1998, and is an update for modern
systems.  This is a "first pass" release to get BSVC running on modern host
machines.  Specific changes include:

* Relicensing under the GPL v2.1.
* Converting the assembler to ANSI C11, adding function prototypes, and
  reformatting the source code.
* Converting the rest of the system to ANSI C++ and starting to use C++ 11
  features (auto, lambdas, etc).
* Replacing some custom linked-list code with STL containers.
* Replacing most uses of C-style strings with std::string objects.
* Reformatting the C++ code with 'clang-format' and removing lots of extraneous
  comments.  Copyright information is now centralized in a file in the
  distribution top-level.
* Replaced the recursive Makefile build system with a single non-recursive
  Makefile.  The build system now does real dependency analysis.
* Many code-cleanups throughout the system: it now builds C++ code with -Wall
  and -Werror.
* Removing Makefiles for obsolete platforms (Ultrix, HP-UX, etc).
* The sim68000 and sim68360 code was reogranized under a single directory tree
  and now share devices and the S-record loader.

Additionally, several serious bugs in the simulator were fixed:

* Updating for 64 bit host systems: Negative offsets in address register
  indirect with displacement mode would cause bus errors on systems with
  64-bit 'unsigned long', since the sign-extension code assumed 32-bit longs
  on the host and the result did not overflow properly (thus yielding an
  'address' outside of the 32-bit address space).
  Fixed by using a properly sized type for simulator addresses.
* Events were being lost in the event handling code due to the 'myIterations'
  variable quickly growing so large that the number of microseconds to use when
  searching for the next event to handle became negative.
  Fixed by rewriting the event handler in terms of nano-seconds and clamping
  the minimum time delta per Check() invocation to 1ns.

The system is now working well enough that it can run the distributed samples,
as well as boot a port of the Xinu (http://www.xinu.cs.purdue.edu) operating
system.

### Administrivia

BSVC hosting has shifted to github and, as stated, the system has been
relicensed under the GPL v2.1.

Additionally, the mailing list has moved and been split into separate 'users'
and 'announce' lists:

* bsvc-announce@googlegroups.com:
   [Announcements (low traffic)](https://groups.google.com/forum/#!forum/bsvc-announce)
* bsvc-users@googlegroups.com:
   [Questions and general discussion](https://https://groups.google.com/forum/#!forum/bsvc-users)

2.0 to 2.1 (11/08/98)
---------------------
* Some minor changes were made to the user interface to support Tcl/Tk 8.0
* Some minor changes were made to the C++ code to support compiling with GCC
  2.8.1
* Jorand Didier's Motorola 68360 (sim68360) simulator has been added 
  to the distribution.  It doesn't contain all of the 68360 devices 
  but it does provide a CPU32 based simulator.
* Xavier Plasencia's timer device has been added to the 68000
  simulator (modeled after the 68230 PI/T timer)
* Fixed a bug with the MOVEA instruction in the Motorola simulators
  - Thanks to Jorand Didier for pointing this problem out to me
* Added support for the ABCD and MOVEP instructions to the
  Motorola simulators
  - Thanks to Trappel Martin and Xavier Plasencia for this code
* Fixed bug in 68000 assembler when using $FFFF as an immediate operand
  - Thanks to Craig A. Haynal for pointing this problem out to me


2.0 Beta 1 to 2.0 (08/08/96)
----------------------------
* Modified the BSVC C++ framework to use "standard" C++ libraries
  (STL).  Most of this work was done by Seunghee Lee so the 
  framework could be ported to Windows 95.  
* Some minor changes were made to the user interface to support
  running it under Tcl/Tk for Windows 95.  This work was done
  by Seunghee Lee.
* Modified Sim68000 to work with the new version of the C++ framework
* Removed the Hector 1600 and MIL-STD-1750A simulators until they
  can be ported to the new framework
* Fixed a problem with the predecrement and postincrement addressing
  modes when the stack pointer and a byte size operation was specified
  - Thanks to Jorand Didier for pointing this problem out to me
 
1.0.4 to 2.0 Beta 1 (02/10/96)
-------------------------------
* User interface has undergone a major revision
* Oliver M. Kellogg's MIL-STD-1750A simulator has been added 
  to the distribution
* Breakpoints set using Tutor under Sim68000 work correctly
* Support for GCC 2.7.2 and Tk 4.0 added

1.0.3 to 1.0.4 (07/31/95)
-------------------------
* Stack pointer bug in the ADDA, SUBA, CMPA, and LEA instructions fixed
  - Thanks to James Yuill at NCSU for reporting this bug

1.0.2 to 1.0.3 (07/25/95)
-------------------------
* Bug in Sim68000's DIVS, BCHG, BCLR, BSET, and BTST instructions fixed
  - Thanks to Robert Woodside at NCSU for reporting the DIVS bug
  - Thanks to John R Potter at Penn State for reporting Bit instrution bug
* M68681 interrupt level bug fixed
* Sim68000 interrupt servicing modified so breakpoints at a service
  routine are handled correctly
  - Thanks to Jonathan Bredin at the University of Pennsylvania for 
    making me check the interrupts out
* Font resources changed for the GUI
* Insert registers in trace window bug fixed
  - Thanks to Stephanie Watson at NCSU for reporting the bug

1.0.1 to 1.0.2 (02/18/95)
-------------------------
* Added support for the SUBX instruction to Sim68000
* Bug in Sim68000's CMPM and RTR instructions fixed
  - Thanks to John R Potter at Penn State for reporting the bug
* Mailing list for BSVC has been setup

1.0 to 1.0.1 (02/11/95)
-----------------------
* Memory viewer's dump utility fixed
* Bug in Sim68000's CMP and CMPA instructions fixed
  - Thanks to John R Potter at Penn State for reporting the bug>
* Solaris support added

1.0b3 to 1.0
------------
* The memory viewer has been made larger and faster
* WWW Home Page selection has been added to the help menu
  This starts a WWW browser pointed at the BSVC Home Page
* Added code to handle the STOP instruction
* Fixed a problem with the RTS instruction
* Unimplemented intructions cause an illegal instruction exception
* Fixed the 68000 assembler's INCLUDE directive so that it can
  be in any case
* Compiles with GCC 2.6.2

1.0b2 to 1.0b3
--------------
* Added an INCLUDE directive to the 68000 assembler
* Fixed a bug in the Program Listing window that caused it to
  only work for programs listed in ascending order
* Corrected a small problem with the file selector that caused it
  to "grab" the mouse while reading a directory (This causes problems
  with networked file systems that take a long time to read)
* Fixed bugs in two of the Framework classes that caused them not
  to compile under GCC 2.6.0

1.0b1 to 1.0b2
--------------
* Added a BREAK instruction to the 68000 simulator and assembler
  that acts like a breakpoint.  When the simulator executes this
  instruction while "running" it will stop running like it
  hit a breakpoint.
* Added a new file selector to the user interface
* Fixed several small bugs in the HECTOR 1600 simulator
