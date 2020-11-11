src = $(wildcard src/*.c) \
	$(wildcard src/mtlib/*.c) \
	$(wildcard src/modules/*.c) \
	$(wildcard src/modules/gl/*.c) \
	$(wildcard src/modules/image/*.c) \
	$(wildcard src/modules/player/*.c) \
	$(wildcard src/modules/text/*.c) \
	$(wildcard src/modules/view/*.c) \
	$(wildcard src/modules/wm/*.c)

CFLAGS = -I/usr/local/include \
    -I/usr/local/include/GL \
    -I/usr/local/include/SDL2 \
    -Isrc \
    -Isrc/ui \
    -Isrc/mtlib \
    -Isrc/views \
    -Isrc/modules \
    -Isrc/modules/gl \
    -Isrc/modules/wm \
    -Isrc/modules/text \
    -Isrc/modules/view \
    -Isrc/modules/image \
    -Isrc/modules/player


LDFLAGS = -L/usr/local/lib \
	-lm \
	-lGL \
    -lGLEW \
    -lSDL2 \
    -lSDL2_image \
    -lavutil \
    -lavcodec \
    -lavdevice \
    -lavformat \
    -lavfilter \
    -lswresample \
    -lswscale \
    -Isrc/modules/player

obj = $(src:.c=.o)
CC = clang

zenmusic: $(obj)
	$(CC) -o bin/$@ $^ $(LDFLAGS)

clean:
	rm -f $(obj) myprog
