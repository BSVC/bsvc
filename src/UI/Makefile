all: bsvc.tk bsvc

clean:
	rm -f bsvc bsvc.tk

bsvc:	Makefile
	echo '#!/bin/sh' > bsvc
	echo 'ARGS="$$@"' >> bsvc
	echo 'export ARGS' >> bsvc
	echo 'exec $(WISH) -f $(LIBDIR)/UI/bsvc.tk' >> bsvc
	chmod +x bsvc

bsvc.tk:	Makefile
	echo '#!$(WISH) -f' > bsvc.tk
	echo 'set Program(InstallDir) $(LIBDIR)' >> bsvc.tk
	echo 'set Program(BitmapDir) $$Program(InstallDir)/UI/bitmaps' >> bsvc.tk
	echo 'set Program(HelpDir) $$Program(InstallDir)/UI/help' >> bsvc.tk
	echo 'set Program(LibDir) $$Program(InstallDir)/UI' >> bsvc.tk
	echo 'option readfile $$Program(InstallDir)/UI/bsvc.ad 40' >> bsvc.tk
	echo 'source $$Program(InstallDir)/UI/main.tk' >> bsvc.tk

install:	bsvc bsvc.tk
	$(MKDIR) -p $(LIBDIR)/UI
	$(MKDIR) -p $(LIBDIR)/UI/bitmaps
	$(MKDIR) -p $(LIBDIR)/UI/help
	$(INSTALL) *.tk *.ad $(LIBDIR)/UI
	$(INSTALL) bitmaps/* $(LIBDIR)/UI/bitmaps
	$(INSTALL) help/* $(LIBDIR)/UI/help
	$(INSTALL) bsvc $(BINDIR)
