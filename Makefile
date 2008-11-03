CC=gcc
CFLAGS=`sdl-config --cflags` -Wall -ggdb -O2
LIBS=`sdl-config --libs` -lSDL_image
PROGRAM=rezerwar
OBJECTS=main.o rmalloc.o rboard.o rboard_blocks.o rboard_drops.o rblock.o \
	rdrop.o routput.o rboard_output.o

$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(PROGRAM)

all: $(PROGRAM)

$(OBJECTS): %.o: %.c rezerwar.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROGRAM) tags TAGS LOG

tags:
	etags *.c *.h
