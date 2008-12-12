CC=gcc
LIBS=cgic/libcgic.a
INCLUDE=-Icgic
CFLAGS+=-g -Wall -Icgic
OBJECTS+=jsonptunnel.o
CGIC_VER = 205
CGIC_URL = http://www.boutell.com/cgic/cgic$(CGIC_VER).tar.gz

QUIET_SUBDIR0  = +$(MAKE) -C # space to separate -C and subdir
QUIET_SUBDIR1  =

all: jsonptunnel.cgi 

jsonptunnel.cgi: $(OBJECTS) libcgic
	$(CC) $(CFLAGS) -o jsonptunnel.cgi $(OBJECTS) $(LIBS)

install: jsonptunnel.cgi
	cp jsonptunnel.cgi /Users/alan/Sites/cgi-bin

libcgic:
	$(QUIET_SUBDIR0)cgic $(QUIET_SUBDIR1) libcgic.a

get-cgic:
	curl $(CGIC_URL) | tar -xz && rm -rf git && mv cgic$(CGIC_VER) cgic 

clean:
	rm -f *.o *.a jsonptunnel.cgi
