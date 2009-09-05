all: level1.mp3 menu.mp3

LAME_OPTS=--resample 48 -b 160 --ta Tamentis --tl "rezerwar soundtrack" --ty 2009 --tt

level1.mp3: level1.wav
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 -hqmixer -X -d wav level1.it
#	mikmod -q -hqmixer -X -d wav level1.it
#	mv music.wav level1.wav
#	oggenc level1.wav -o level1.ogg
	lame $(LAME_OPTS) "rezerwar - Level 1" level1.wav level1.mp3
#	rm -f level1.wav

menu.mp3: menu.wav
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 --hqmixer -X -d wav menu.it
#	mikmod -q --hqmixer -X -d wav menu.it
#	mv music.wav menu.wav
#	oggenc menu.wav -o menu.ogg
	lame $(LAME_OPTS) "rezerwar - Menu" menu.wav menu.mp3
#	rm -f menu.wav

distclean:
	rm -f *.wav *.ogg *.mp3
