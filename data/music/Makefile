all: level1.ogg menu.ogg

level1.ogg: level1.it
	@echo Exporting MOD to OGG...
	mikmod -q -f 48000 -hqmixer -X -d wav level1.it
	oggenc music.wav -o level1.ogg
	lame -b 160 music.wav level1.mp3
	rm -f music.wav

menu.ogg: menu.it
	@echo Exporting MOD to OGG...
	mikmod -q -f 48000 --hqmixer -X -d wav menu.it
	oggenc music.wav -o menu.ogg
	lame -b 160 music.wav menu.mp3
	rm -f music.wav

distclean:
	rm -f *.wav *.ogg *.mp3
