all: sky.bmp


sky.bmp: main.svg
	inkscape -i longSky -e sky.png ../main.svg
	convert sky.png -type truecolor sky.bmp
	rm -f sky.png

distclean:
	rm -f sky.bmp sky.png
	make -C clean
