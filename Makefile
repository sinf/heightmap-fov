
TARGET?=a.out
CFLAGS=-std=c99 -Wall -Wno-missing-braces $(shell sdl-config --cflags) $(shell pkg-config --cflags gl glew) $(XFLAGS)
LIBS=-lm -lfreeimage $(shell sdl-config --libs) $(shell pkg-config --libs gl glew)
SRC=$(wildcard *.c)
DEBUG=fog.debug
FAST=fog

.PHONY: clean all target debug fast

all: $(DEBUG) $(FAST)

$(DEBUG): $(SRC) Makefile
	make XFLAGS="-g -O0" TARGET=$@ target
$(FAST): $(SRC) Makefile
	make XFLAGS="-g -O3 -DNDEBUG" TARGET=$@ target
target: $(SRC)
	gcc -o $(TARGET) $(SRC) $(LIBS) $(CFLAGS) 
clean:
	rm -f $(DEBUG) $(FAST)

