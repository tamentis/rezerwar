all: sky.bmp

sky.bmp: sky.png
	convert sky.png -type truecolor sky.bmp

sky.png: ../main.svg
	inkscape -i longSky -e sky.png ../main.svg

clean:
	rm -f sky.bmp sky.png
