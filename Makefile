CC=gcc
# CC=llvm-gcc

# Standard
CFLAGS=`sdl-config --cflags` -Wall -O2
LIBS=`sdl-config --libs` -lSDL_mixer

# Debug (no optimization)
# CFLAGS=`sdl-config --cflags` -Wall -ggdb
# LIBS=`sdl-config --libs` -lSDL_mixer

# Profiling
# CFLAGS=`sdl-config --cflags` -Wall -pg -g -Wall
# LIBS=`sdl-config --libs` -lSDL_mixer -g -pg

PROGRAM=rezerwar
OBJECTS=main.o menus.o events.o lvlhandler.o hiscore.o \
	board.o block.o cube.o text.o \
	memory.o fatal.o sfx.o engine_sdl.o \
	strlcpy.o \
	a_chimneys.o a_sky.o

all: gfx_build music_build $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

music_build:
	make -C music/

gfx_build:
	make -C gfx/

$(OBJECTS): %.o: %.c rezerwar.h
	$(CC) $(CFLAGS) -c $<

icon_dot_o:
	make -C gfx/icons/

win32: icon_dot_o
	$(CC) $(OBJECTS) gfx/icons/icons.o $(LIBS) -o $(PROGRAM)
	strip -s $(PROGRAM).exe
	upx $(PROGRAM).exe

clean:
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG
	rm -rf doc hiscore.dat

distclean: clean
	make -C gfx/ clean
	make -C music/ clean

tags:
	etags *.c *.h

doc:
	mkdir -p doc/
	doxygen Doxyfile

memwatch:
	top -d 0.1 -p `pidof rezerwar`

valgrind:
	valgrind --leak-check=full ./$(PROGRAM) >& LOG

lvltest: rmalloc.o strlcpy.o lvlhandler.c
	gcc $(CFLAGS) -c lvlhandler.c
	gcc rmalloc.o strlcpy.o lvlhandler.o -o lvlhandler
