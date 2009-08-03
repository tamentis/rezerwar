# $Id$
#
# rezerwar Makefile for Linux
# Copyright (c) 2009 Bertrand Janin
#


include ../MkConf

OBJECTS=main.o menus.o events.o lvlhandler.o hiscore.o \
	board.o block.o cube.o text.o path.o \
	memory.o fatal.o sfx.o engine_sdl.o \
	strlcpy.o chimneys.o sky.o mole.o


PROGRAM=rezerwar

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

$(OBJECTS): %.o: %.c rezerwar.h ../config.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG

tags:
	etags *.c *.h

memwatch:
	top -d 0.1 -p `pidof rezerwar`

valgrind:
	valgrind --leak-check=full ./$(PROGRAM) >& LOG

win32: icon_dot_o
	$(CC) $(OBJECTS) ../data/gfx/icons/icons.o $(LIBS) -o $(PROGRAM)
	strip -s $(PROGRAM).exe
	upx $(PROGRAM).exe
