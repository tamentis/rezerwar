# $Id$
#
# rezerwar Makefile for Linux
# Copyright (c) 2009 Bertrand Janin
#


include ../MkConf


PROGRAM=rezerwar

OBJECTS=main.o menus.o events.o lvlhandler.o hiscore.o \
	board.o block.o cube.o text.o path.o \
	memory.o fatal.o sfx.o engine_sdl.o \
	strlcpy.o a_chimneys.o a_sky.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

$(OBJECTS): %.o: %.c rezerwar.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG

tags:
	etags *.c *.h

memwatch:
	top -d 0.1 -p `pidof rezerwar`

valgrind:
	valgrind --leak-check=full ./$(PROGRAM) >& LOG

