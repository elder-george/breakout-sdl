CFLAGS=-I C:/tools/MinGW/include -std=c++0x -Wall -pedantic 
LIBRARY_PATH=C:/tools/MinGW/lib;C:/tools/MinGW/x86_64-w64-mingw32/lib
LDLIBS=-lsdl2 -lSDL2_image -lsdl2_ttf -lgdi32 -lwinmm -lole32 -limm32 -lversion -loleaut32

all: bin/breakout

bin/%.dll:
	cp dlls/$(notdir $@) $@

bin/assets/%: bin/assets
	cp assets/$(notdir $@) $@

bin:
	mkdir -p bin

bin/assets:
	mkdir -p bin/assets


bin/breakout: bin breakout.cpp bin/sdl2.dll bin/sdl2_image.dll bin/sdl2_ttf.dll bin/libpng16-16.dll \
                bin/assets/breakout_pieces_1.png bin/assets/SourceSansPro-Regular.ttf
	g++ -g $(CFLAGS) -o $@ $(filter %.cpp, $^) $(LDLIBS)
