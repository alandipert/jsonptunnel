CC=gcc
LIBS=cgic/libcgic.a -lcurl -lfcgi
FCGI_HEADERS=-I/opt/include
INCLUDE=-Icgic $(FCGI_HEADERS) 
CFLAGS+=-g -Wall
CFLAGS+=$(INCLUDE)
#CFLAGS+=-O2
OBJECTS+=main.o fetch.o cache.o error.o request.o debug.o
CGIC_VER = 205
CGIC_URL = http://www.boutell.com/cgic/cgic$(CGIC_VER).tar.gz
INSTALLDIR = /Users/alan/Sites/cgi-bin

QUIET_SUBDIR0  = +$(MAKE) -C # space to separate -C and subdir
QUIET_SUBDIR1  =

all: jsonptunnel.fcgi 

jsonptunnel.fcgi: $(OBJECTS) libcgic
	$(CC) $(CFLAGS) $(INCLUDE) -o jsonptunnel.fcgi  $(OBJECTS) $(LIBS)

install: jsonptunnel.fcgi
	cp jsonptunnel.fcgi $(INSTALLDIR) 

libcgic:
	$(QUIET_SUBDIR0)cgic $(QUIET_SUBDIR1) libcgic.a

get-cgic:
	curl $(CGIC_URL) | tar -xz && rm -rf git && mv cgic$(CGIC_VER) cgic 

clean:
	rm -f *.o *.a jsonptunnel.fcgi
