all: newgaul.bmp

newgaul.bmp: newgaul.png
	convert newgaul.png -type truecolor newgaul.bmp

newgaul.png: newgaul.svg
	inkscape -B newgaul.svg -e newgaul.png

clean:
	rm -f newgaul.bmp newgaul.png
