all: level1.ogg menu.ogg

level1.ogg: level1.it
	@echo Exporting MOD...
	mikmod -q --hqmixer -X -d wav level1.it
	oggenc music.wav -o level1.ogg
	rm -f music.wav

menu.ogg: menu.it
	@echo Exporting MOD...
	mikmod -q --hqmixer -X -d wav menu.it
	oggenc music.wav -o menu.ogg
	rm -f music.wav

distclean:
	rm -f *.wav *.ogg
