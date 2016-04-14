
CC = gcc


.PHONY: all

tetris: tetris.c Makefile
	$(CC) `pkg-config --cflags --libs sdl2` -lm -Wall -g tetris.c -o tetris

all: tetris
