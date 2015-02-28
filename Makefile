
TARGET?=a.out
CFLAGS=-std=c99 -Wall -Wno-missing-braces $(shell sdl-config --cflags) $(shell pkg-config --cflags gl glew) $(XFLAGS)
LIBS=-lm -lfreeimage $(shell sdl-config --libs) $(shell pkg-config --libs gl glew)
SRC=$(wildcard *.c)
DEBUG=fog.debug
FAST=fog
SMALL=fog.small

.PHONY: clean all target

all: $(DEBUG) $(FAST) $(SMALL)

$(DEBUG): $(SRC) Makefile
	make XFLAGS="-g -O0" TARGET=$@ target
$(FAST): $(SRC) Makefile
	make XFLAGS="-g -O3 -DNDEBUG" TARGET=$@ target
$(SMALL): $(SRC) Makefile
	make XFLAGS="-g -Os -DNDEBUG" TARGET=$@ target
target: $(SRC)
	gcc -o $(TARGET) $(SRC) $(LIBS) $(CFLAGS) 
clean:
	rm -f $(DEBUG) $(FAST) $(SMALL)

