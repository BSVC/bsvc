###############################################################################
# Makefile for BSVC User Interface
#
# By: Bradford W. Mott
#
# December 5,1993
###############################################################################
# $Id:$
###############################################################################

all: bsvc.tk bsvc

clean:
	rm -f bsvc bsvc.tk

bsvc: Makefile
	echo '#!/bin/sh' > bsvc
	echo '# TCL_LIBRARY=......' >> bsvc
	echo '# TK_LIBRARY=......' >> bsvc
	echo '# export TCL_LIBRARY TK_LIBRARY' >> bsvc
	echo 'ARGS=$$*' >> bsvc
	echo 'export ARGS' >> bsvc
	echo 'exec $(WISH) -f $(INSTALL_DIR)/UI/bsvc.tk' >> bsvc
	chmod +x bsvc

bsvc.tk: Makefile
	echo '#!$(WISH) -f' > bsvc.tk
	echo 'set Program(InstallDir) $(INSTALL_DIR)' >> bsvc.tk
	echo 'set Program(BitmapDir) $$Program(InstallDir)/UI/bitmaps' >> bsvc.tk
	echo 'set Program(HelpDir) $$Program(InstallDir)/UI/help' >> bsvc.tk
	echo 'set Program(LibDir) $$Program(InstallDir)/UI' >> bsvc.tk
	echo 'set Program(WWWBrowser) "$(WWW_BROWSER)"' >> bsvc.tk
	echo 'set Program(WWWHomePage) "$(WWW_HOME_PAGE)"' >> bsvc.tk
	echo 'set Program(StopIndicator) C:\\BSVC\\bin\\STOP' >> bsvc.tk
	echo 'if {$$tcl_version >= 7.5 && $$tcl_platform(platform) == "windows"} {' >> bsvc.tk
	echo '  option readfile $$Program(InstallDir)/UI/bsvc.win 40' >> bsvc.tk
	echo '} else {' >> bsvc.tk
	echo '  option readfile $$Program(InstallDir)/UI/bsvc.ad 40' >> bsvc.tk
	echo '}' >> bsvc.tk
	echo 'source $$Program(InstallDir)/UI/main.tk' >> bsvc.tk

install: bsvc bsvc.tk
	$(MKDIR) $(INSTALL_DIR)/UI
	$(MKDIR) $(INSTALL_DIR)/UI/bitmaps
	$(MKDIR) $(INSTALL_DIR)/UI/help
	$(INSTALL) *.tk *.ad $(INSTALL_DIR)/UI
	$(INSTALL) bitmaps/* $(INSTALL_DIR)/UI/bitmaps
	$(INSTALL) help/* $(INSTALL_DIR)/UI/help
	$(INSTALL) bsvc $(INSTALL_DIR)

