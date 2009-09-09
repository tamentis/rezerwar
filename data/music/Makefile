all: level1.ogg menu.ogg

# OGGENC_OPTS=--resample 48000
COMMENT_OPTS=-a -t 'ARTIST=Tamentis'

level1.ogg: level1.wav
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 -hqmixer -X -d wav level1.it
#	mikmod -q -hqmixer -X -d wav level1.it
#	mv music.wav level1.wav
	oggenc $(OGGENC_OPTS) level1.wav -o level1.ogg
	vorbiscomment $(COMMENT_OPTS) -t 'TITLE=Rezerwar - Level 1' level1.ogg
#	lame $(LAME_OPTS) "rezerwar - Level 1" level1.wav level1.mp3
#	rm -f level1.wav

menu.ogg: menu.wav
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 --hqmixer -X -d wav menu.it
#	mikmod -q --hqmixer -X -d wav menu.it
#	mv music.wav menu.wav
	oggenc $(OGGENC_OPTS) menu.wav -o menu.ogg
	vorbiscomment $(COMMENT_OPTS) -t 'TITLE=Rezerwar - Title' menu.ogg
#	lame $(LAME_OPTS) "rezerwar - Menu" menu.wav menu.mp3
#	rm -f menu.wav

distclean:
	rm -f *.wav *.ogg *.mp3
