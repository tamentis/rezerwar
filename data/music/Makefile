all: level1.mp3 menu.mp3

level1.mp3: level1.it
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 -hqmixer -X -d wav level1.it
	mikmod -q -hqmixer -X -d wav level1.it
	mv music.wav level1.wav
#	oggenc level1.wav -o level1.ogg
	lame -b 160 level1.wav level1.mp3
	rm -f level1.wav

menu.mp3: menu.it
	@echo Exporting MOD to OGG...
#	mikmod -q -f 48000 --hqmixer -X -d wav menu.it
	mikmod -q --hqmixer -X -d wav menu.it
	mv music.wav menu.wav
#	oggenc menu.wav -o menu.ogg
	lame -b 160 menu.wav menu.mp3
	rm -f menu.wav

distclean:
	rm -f *.wav *.ogg *.mp3
