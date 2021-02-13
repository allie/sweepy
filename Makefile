CC=clang
CFLAGS=-std=c99 -g -c -D_POSIX_C_SOURCE=200112L -DUNOFFICIAL_MODE -Iinclude -I/usr/local/include
LDFLAGS=-lSDL2 -lSDL2_image -lm
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mines
OS=$(shell gcc -dumpmachine)

ifneq (, $(findstring mingw, $(OS)))
	LDFLAGS := -lmingw32 -lSDL2main $(LDFLAGS)
	CC=gcc
endif

all: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS += -DDEBUG_MODE
debug: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE)
	find . -name "*.o" -print0 | xargs -0 rm -rf
