CC=gcc
# CC=llvm-gcc

# Standard
CFLAGS=`sdl-config --cflags` -Wall -O2
LIBS=`sdl-config --libs`

# Debug (no optimization)
CFLAGS=`sdl-config --cflags` -Wall -ggdb
LIBS=`sdl-config --libs`

# Profiling
#CFLAGS=`sdl-config --cflags` -pg -Wall
#LIBS=`sdl-config --libs` -pg

PROGRAM=rezerwar
OBJECTS=main.o rmalloc.o rboard.o rboard_blocks.o rboard_drops.o rblock.o \
	rdrop.o routput.o rboard_output.o rcube.o rboard_cube.o events.o \
	engine_sdl.o strlcpy.o menus.o text.o

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

all: $(PROGRAM)

$(OBJECTS): %.o: %.c rezerwar.h
	$(CC) $(CFLAGS) -c $<

win32: rezerwar
	strip -s rezerwar
	upx rezerwar

clean:
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG
	rm -rf doc

tags:
	etags *.c *.h

doc:
	mkdir -p doc/
	doxygen Doxyfile
