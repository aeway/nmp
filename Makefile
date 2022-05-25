
PORTAUDIO="/usr/lib/x86_64-linux-gnu/libportaudio.so.2"
SNDFILE=`pkg-config --libs --cflags sndfile`
SOX=`pkg-config --libs --cflags sox`

.PHONY: all

all:
	g++ -lncurses $(PORTAUDIO) $(SNDFILE) $(SOX) -pthread *.cpp -o omp2
