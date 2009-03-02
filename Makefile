CC=gcc
# CC=llvm-gcc

# Standard
#CFLAGS=`sdl-config --cflags` -Wall -O2
#LIBS=`sdl-config --libs`

# Debug (no optimization)
CFLAGS=`sdl-config --cflags` -Wall -ggdb
LIBS=`sdl-config --libs`

# Profiling
#CFLAGS=`sdl-config --cflags` -pg -ggdb -Wall
#LIBS=`sdl-config --libs` -pg

PROGRAM=rezerwar
OBJECTS=main.o rmalloc.o rboard.o rboard_blocks.o rblock.o \
	rcube.o rboard_cube.o events.o \
	engine_sdl.o strlcpy.o menus.o text.o hiscore.o \
	a_chimneys.o a_sky.o

all: gfx_build $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

gfx_build:
	make -C gfx

$(OBJECTS): %.o: %.c rezerwar.h
	$(CC) $(CFLAGS) -c $<

icon_dot_o:
	make -C gfx/icons/

win32: icon_dot_o
	$(CC) $(OBJECTS) gfx/icons/icons.o $(LIBS) -o $(PROGRAM)
	strip -s $(PROGRAM).exe
	upx $(PROGRAM).exe

clean:
	make -C gfx/ clean
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG
	rm -rf doc

tags:
	etags *.c *.h

doc:
	mkdir -p doc/
	doxygen Doxyfile
