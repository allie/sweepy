CC=clang
CFLAGS=-std=c11 -c -D_POSIX_C_SOURCE=200809L -DUNOFFICIAL_MODE -Iinclude -I/usr/local/include
LDFLAGS=-lSDL2 -lm
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sweepy
OS=$(shell gcc -dumpmachine)

ifneq (, $(findstring mingw, $(OS)))
	LDFLAGS := -lmingw32 -lSDL2main $(LDFLAGS)
	CC=gcc
endif

.PHONY: all debug prod clean

all: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS += -DDEBUG_MODE -g
debug: all

prod: CFLAGS += -O3
prod: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE)
	find . -name "*.o" -print0 | xargs -0 rm -f
