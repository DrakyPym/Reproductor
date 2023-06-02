EXEC = reproductorr
CLIB = -I./lib/portaudio/include ./lib/portaudio/lib/.libs/libportaudio.a -lrt -lm -lasound -ljack -lsndfile

all: $(EXEC)

$(EXEC): reproductor.c hideShow.o tools.o controlSong.o
	gcc -o $@ $^ $(CLIB)

hideShow.o: hideShow.c hideShow.h
	gcc -c hideShow.c

tools.o: tools.c tools.h
	gcc -c tools.c 

controlSong.o: controlSong.c controlSong.h tools.o
	gcc -c controlSong.c -I./lib/portaudio/include

install-deps:
	cd lib/portaudio && ./configure && $(MAKE) -j
	sudo apt install libsndfile1-dev cmake clang pulseaudio libasound-dev libjack-dev

.PHONY: install-deps

uninstall-deps:
	cd lib/portaudio && $(MAKE) uninstall
	rm -rf lib/portaudio

.PHONY: uninstall-deps

clean:
	rm -f $(EXEC) ocultarMostrar.o

.PHONY: clean