# This Makefile has been generated from the Configure script.
# Shareware copyright 1993, by Sam Lantinga


CC = gcc
PTYCHAR=\"\"
HEXDIGIT=\"\"
PTYOPTS = -DPTYCHAR=$(PTYCHAR) -DHEXDIGIT=$(HEXDIGIT)

CFLAGS = -g -DDEBUG -O2 -DTERMCAP -DHAVE_UTHOST -DHAVE_TERMIO_H -DHAVE_UNISTD_H -DNEED_SELECT_H -DHAVE_WAIT4 -DSHELL=\"/bin/csh\" $(PTYOPTS)
LIBS =  -ltermcap -lnsl
OBJS = splitvt.o misc.o utmp.o vt100.o videomem.o terminal.o vttest.o vtmouse.o \
       parserc.o lock.o cut-paste.o

splitvt: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

clean: 
	rm -f *.o core 

distclean: clean
	rm -f splitvt Makefile

install: install-man
	mv splitvt /usr/local/bin/splitvt
	mv examples/xsplitvt /usr/local/bin/xsplitvt

install-man:
	cp splitvt.man /usr/local/man/man1/splitvt.1
