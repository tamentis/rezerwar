CC=gcc
# CC=llvm-gcc

# Standard
#CFLAGS=`sdl-config --cflags` -Wall -O2
#LIBS=`sdl-config --libs`

# Debug (no optimization)
CFLAGS=`sdl-config --cflags` -Wall -ggdb
LIBS=`sdl-config --libs` -lSDL_mixer

# Profiling
#CFLAGS=`sdl-config --cflags` -pg -ggdb -Wall
#LIBS=`sdl-config --libs` -pg

PROGRAM=rezerwar
OBJECTS=main.o rmalloc.o rboard.o rboard_blocks.o rblock.o \
	rcube.o rboard_cube.o events.o sfx.o lvlhandler.o \
	engine_sdl.o strlcpy.o menus.o text.o hiscore.o \
	a_chimneys.o a_sky.o strsep.o fatal.o

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
