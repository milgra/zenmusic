UNAME := $(shell uname -s)
CC = clang
OBJDIRDEV = bin/obj/dev
OBJDIRREL = bin/obj/rel
VERSION = 1

SOURCES = \
	$(wildcard src/modules/*.c) \
	$(wildcard src/modules/zen_core/*.c) \
	$(wildcard src/modules/zen_math/*.c) \
	$(wildcard src/modules/zen_wm/*.c) \
	$(wildcard src/modules/image/*.c) \
	$(wildcard src/modules/zen_media_player/*.c) \
	$(wildcard src/modules/zen_media_transcoder/*.c) \
	$(wildcard src/modules/zen_ui/*.c) \
	$(wildcard src/modules/zen_ui/gl/*.c) \
	$(wildcard src/modules/zen_ui/view/*.c) \
	$(wildcard src/modules/zen_ui/text/*.c) \
	$(wildcard src/modules/zen_ui/html/*.c) \
	$(wildcard src/zenmusic/*.c) \
	$(wildcard src/zenmusic/ui/*.c)

CFLAGS = \
	-Isrc/modules \
	-Isrc/modules/zen_core \
	-Isrc/modules/zen_math \
	-Isrc/modules/zen_wm \
	-Isrc/modules/image \
	-Isrc/modules/zen_media_player \
	-Isrc/modules/zen_media_transcoder \
	-Isrc/modules/zen_ui \
	-Isrc/modules/zen_ui/gl \
	-Isrc/modules/zen_ui/view \
	-Isrc/modules/zen_ui/text \
	-Isrc/modules/zen_ui/html \
	-Isrc/zenmusic \
	-Isrc/zenmusic/ui


ifeq ($(UNAME),FreeBSD)
     CFLAGS += -I/usr/local/include \
	       -I/usr/local/include/GL \
	       -I/usr/local/include/SDL2
endif		
ifeq ($(UNAME),Linux)
     CFLAGS += -I/usr/include \
	       -I/usr/include/GL \
	       -I/usr/include/SDL2
endif		

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

rel: $(OBJECTSREL)
	$(CC) $^ -o bin/zenmusic $(LDFLAGS)

dev: $(OBJECTSDEV)
	$(CC) $^ -o bin/zenmusicdev $(LDFLAGS)

$(OBJECTSDEV): $(OBJDIRDEV)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -g -DDEBUG -DVERSION=0 -DBUILD=0

$(OBJECTSREL): $(OBJDIRREL)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -O3 -DVERSION=$(VERSION) -DBUILD=$(shell cat version.num)

clean:
	rm -f $(OBJECTSDEV) zenmusic
	rm -f $(OBJECTSREL) zenmusic

deps:
	@sudo pkg install ffmpeg sdl2 glew

vjump: 
	$(shell ./version.sh "$$(cat version.num)" > version.num)

rectest:
	tst/test_rec.sh 0

runtest:
	tst/test_run.sh

install: rel
ifeq ($(UNAME),FreeBSD)
	/usr/bin/install -c -s -m 755 bin/zenmusic /usr/local/bin
	/usr/bin/install -d -m 755 /usr/local/share/zenmusic
	cp res/* /usr/local/share/zenmusic/
endif		
ifeq ($(UNAME),Linux)
	/usr/bin/install -c -s -m 755 bin/zenmusic /usr/bin
	/usr/bin/install -d -m 755 /usr/share/zenmusic
	cp res/* /usr/share/zenmusic/
endif		

remove:
ifeq ($(UNAME),FreeBSD)
	rm /usr/local/bin/zenmusic
	rm -r /usr/local/share/zenmusic
endif
ifeq ($(UNAME),Linux)
	rm /usr/lbin/zenmusic
	rm -r /usr/share/zenmusic
endif
