
# This make file has only been tested on a RPI 3B+.
CC=gcc

SRC=tiger_gl_test.c

LDFLAGS=-g -L../utils/libs -L./ -L../tiger_gl -L../utils/libs -L/usr/local/lib -ltiger_gl -lini -lmiscutils -lstrutils -llogutils -lfreeimage -lz -lpthread -lm
CFLAGS=-std=gnu99

CFLAGS += -g -Wall -O2 -I./ -I../utils/incs -I/usr/include/directfb -I/usr/include/directfb/direct -I../tiger_gl

PRG=tiger_gl_test
PRG2=video-example

all: $(PRG) $(PRG2)

$(PRG): tiger_gl_test.o ../tiger_gl/libtiger_gl.a
	$(CC) tiger_gl_test.o -o $(PRG) $(LDFLAGS)

tt.o: tiger_gl_test.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PRG2): video-example.o ../tiger_gl/libtiger_gl.a
	$(CC) video-example.o -o $(PRG2) $(LDFLAGS)

video-example.o: video-example.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tiger_gl_test.o video-example.o $(PRG) $(PRG2)

