all: sky.bmp tdc.bmp sprites.bmp gameback.bmp


# Splash image
tdc.bmp: tools/xcf2png
	tools/xcf2png tdc.xcf tdc32.bmp
	convert -type truecolor tdc32.bmp tdc.bmp
	rm -f tdc32.bmp


# Game background
gameback.bmp: main.svg
	inkscape -e gameback.png main.svg
	convert gameback.png -type truecolor -fill '#00ffff' -opaque '#6b97d2' gameback.bmp
	rm -f gameback.png


# Scrolling sky
sky.bmp: main.svg
	inkscape -i longSky -e sky.png main.svg
	convert sky.png -type truecolor sky.bmp
	rm -f sky.png

# Sprites
sprites.bmp: sprites.xcf
	tools/xcf2png sprites.xcf sprites_32.bmp
	convert sprites_32.bmp -type truecolor sprites.bmp
	rm -f sprites_32.bmp


distclean:
	rm -f sky.bmp sky.png tdc.bmp tdc32.bmp sprites.bmp sprites_32.bmp \
		gameback.bmp gameback.png
