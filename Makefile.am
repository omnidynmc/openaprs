EXTRA_DIST = include/*.h \
             etc/example.conf \
             bursts/auth.burst.example \
             libraries/putmysql/*.h \
             sql/* \
             modules/*/*.h \
             modules/*/*.cpp \
             modules/*/Makefile.in \
             modules/Makefile.in
     

SUBDIRS = libraries\
          src

world:
	cd libraries/openframe && ./configure && make install
	cd libraries/openstats && ./configure && make install
	cd libraries/stomp && ./configure && make install
	cd libraries/opensat && ./configure && make install
	make install

makeall:
	cd libraries/openframe && make install
	cd libraries/openstats && make install
	cd libraries/stomp && make install
	cd libraries/opensat && make install
	make install

cleanall:
	cd libraries/openframe && make distclean
	cd libraries/openstats && make distclean
	cd libraries/stomp && make distclean
	cd libraries/opensat && make distclean
	make distclean

dist-hook:
	rm -rf $(distdir)/logs/*
