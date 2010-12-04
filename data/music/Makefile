# You need xmp and oggenc

all: level1.ogg menu.ogg

# OGGENC_OPTS=--resample 48000
COMMENT_OPTS=-a -t 'ARTIST=Tamentis'

level1.ogg: level1.it
	xmp -d wav --output-file=- level1.it | oggenc $(OGGENC_OPTS) - -o level1.ogg
	vorbiscomment $(COMMENT_OPTS) -t 'TITLE=Rezerwar - Level 1' level1.ogg
#	lame $(LAME_OPTS) "rezerwar - Level 1" level1.wav level1.mp3
#	rm -f level1.wav

level1.mp3: level1.it
	xmp -d wav --output-file=- level1.it | lame - level1.mp3

menu.ogg: menu.it
	xmp -d wav --output-file=- menu.it | oggenc $(OGGENC_OPTS) - -o menu.ogg
	vorbiscomment $(COMMENT_OPTS) -t 'TITLE=Rezerwar - Title' menu.ogg
#	lame $(LAME_OPTS) "rezerwar - Menu" menu.wav menu.mp3
#	rm -f menu.wav

distclean:
	rm -f *.wav *.ogg *.mp3
