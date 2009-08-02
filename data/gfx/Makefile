all: sky.bmp tdc.bmp

# Splash image
#   Because xcf2png creates a 32bit BMP most of the time, remove the alpha
#   channel with ImageMagick before saving.
tdc.bmp: tools/xcf2png
	tools/xcf2png tdc.xcf tdc32.bmp
	convert -alpha Deactivate tdc32.bmp tdc.bmp
	rm -f tdc32.bmp

# Scrolling sky
sky.bmp: main.svg
	inkscape -i longSky -e sky.png main.svg
	convert sky.png -type truecolor sky.bmp
	rm -f sky.png

distclean:
	rm -f sky.bmp sky.png tdc.bmp tdc32.bmp
	make -C clean
