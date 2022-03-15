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

dist-hook:
	rm -rf $(distdir)/logs/*
