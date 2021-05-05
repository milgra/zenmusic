CC = clang
OBJDIRDEV = bin/obj/dev
OBJDIRREL = bin/obj/rel

SOURCES = \
	$(wildcard src/mtlib/*.c) \
	$(wildcard src/modules/*.c) \
	$(wildcard src/modules/data/*.c) \
	$(wildcard src/modules/file/*.c) \
	$(wildcard src/modules/image/*.c) \
	$(wildcard src/modules/player/*.c) \
	$(wildcard src/modules/text/*.c) \
	$(wildcard src/modules/html/*.c) \
	$(wildcard src/modules/net/*.c) \
	$(wildcard src/modules/ui/*.c) \
	$(wildcard src/modules/ui/gl/*.c) \
	$(wildcard src/modules/ui/wm/*.c) \
	$(wildcard src/modules/ui/view/*.c) \
	$(wildcard src/zenmusic/*.c) \
	$(wildcard src/zenmusic/ui/*.c)

CFLAGS = \
	-I/usr/local/include \
	-I/usr/local/include/GL \
	-I/usr/local/include/SDL2 \
	-Isrc/mtlib \
	-Isrc/modules \
	-Isrc/modules/data \
	-Isrc/modules/file \
	-Isrc/modules/net \
	-Isrc/modules/text \
	-Isrc/modules/html \
	-Isrc/modules/image \
	-Isrc/modules/player \
	-Isrc/modules/ui \
	-Isrc/modules/ui/gl \
	-Isrc/modules/ui/wm \
	-Isrc/modules/ui/view \
	-Isrc/zenmusic \
	-Isrc/zenmusic/ui \

LDFLAGS = \
	-L/usr/local/lib \
	-lm \
	-lGL \
	-lGLEW \
	-lSDL2 \
	-lavutil \
	-lavcodec \
	-lavdevice \
	-lavformat \
	-lavfilter \
	-lswresample \
	-lswscale \
	-lpthread

OBJECTSDEV := $(addprefix $(OBJDIRDEV)/,$(SOURCES:.c=.o))
OBJECTSREL := $(addprefix $(OBJDIRREL)/,$(SOURCES:.c=.o))

#zenmusic: deps $(OBJECTS)

rel: $(OBJECTSREL)
	$(CC) $^ -o bin/zenmusic $(LDFLAGS)

dev: $(OBJECTSDEV)
	$(CC) $^ -o bin/zenmusicdev $(LDFLAGS)	

$(OBJECTSDEV): $(OBJDIRDEV)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -g -DDEBUG

$(OBJECTSREL): $(OBJDIRREL)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -O3

deps:
	@sudo pkg install ffmpeg sdl2 glew

clean:
	rm -f $(OBJECTSDEV) zenmusic
	rm -f $(OBJECTSREL) zenmusic

install: rel
	/usr/bin/install -c -s -m 755 bin/zenmusic /usr/local/bin
	/usr/bin/install -d -m 755 /usr/local/share/zenmusic
	cp res/* /usr/local/share/zenmusic/
