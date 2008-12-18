# You might need to change this to point to the location
# of the FastCGI header files.
FCGI_INCLUDE=-I/usr/local/include/fcgi_stdio.h
FCGI_LIBS=-lfcgi

CC=gcc
LIBS=-L/usr/local/lib cgic/libcgic.a -lcurl
INCLUDE=-Icgic 
CFLAGS+=-g -Wall
#CFLAGS+=-O2
OBJECTS+=main.o fetch.o error.o request.o
CGIC_VER = 205
CGIC_URL = http://www.boutell.com/cgic/cgic$(CGIC_VER).tar.gz

QUIET_SUBDIR0  = +$(MAKE) -C # space to separate -C and subdir
QUIET_SUBDIR1  =

all: jsonptunnel.fcgi 

cgi: jsonptunnel.cgi

fcgi: CFLAGS+=$(FCGI_INCLUDE)
fcgi: CFLAGS+=-DUSE_FASTCGI=1
fcgi: jsonptunnel.fcgi

jsonptunnel.cgi: $(OBJECTS) libcgic
	$(CC) $(CFLAGS) $(INCLUDE) -o jsonptunnel.cgi  $(OBJECTS) $(LIBS)

jsonptunnel.fcgi: $(OBJECTS) libcgic
	$(CC) $(CFLAGS) $(INCLUDE) -o jsonptunnel.fcgi  $(OBJECTS) $(LIBS) $(FCGI_LIBS)

libcgic:
	$(QUIET_SUBDIR0)cgic CFLAGS='$(CFLAGS)' $(QUIET_SUBDIR1) libcgic.a

get-cgic:
	curl $(CGIC_URL) | tar -xz && rm -rf cgic && mv cgic$(CGIC_VER) cgic 
	patch cgic/cgic.c patches/fastcgic-cgic.patch 

clean:
	rm -f *.o *.a jsonptunnel.fcgi jsonptunnel.cgi cgic/libcgic.a cgic/cgic.o
