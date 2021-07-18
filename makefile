CC = clang
VERSION = 1
BUILDVER != cat version.num

SOURCES = \
	${:!ls src/modules/zen_core/*.c!} \
	${:!ls src/modules/zen_math/*.c!} \
	${:!ls src/modules/zen_wm/*.c!} \
	${:!ls src/modules/zen_media_player/*.c!} \
	${:!ls src/modules/zen_media_transcoder/*.c!} \
	${:!ls src/modules/zen_ui/*.c!} \
	${:!ls src/modules/zen_ui/gl/*.c!} \
	${:!ls src/modules/zen_ui/view/*.c!} \
	${:!ls src/modules/zen_ui/text/*.c!} \
	${:!ls src/modules/zen_ui/html/*.c!} \
	${:!ls src/zenmusic/*.c!} \
	${:!ls src/zenmusic/ui/*.c!}

CFLAGS = \
	-I/usr/local/include \
	-I/usr/local/include/GL \
	-I/usr/local/include/SDL2 \
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

OBJECTS := ${SOURCES:.c=.o}

build: ${OBJECTS}
	${CC} ${OBJECTS} -o bin/zenmusicdev ${LDFLAGS}

.c.o:
	${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET} -O3 -DVERSION=${VERSION} -DBUILD=${BUILDVER}

clean:
	rm -f ${OBJECTS} zenmusic

deps:
	pkg install ffmpeg sdl2 glew

vjump: 
	$(shell ./version.sh "$$(cat version.num)" > version.num)

rectest:
	tst/test_rec.sh 0

runtest:
	tst/test_run.sh

install: build
	/usr/bin/install -c -s -m 755 bin/zenmusic /usr/local/bin
	/usr/bin/install -d -m 755 /usr/local/share/zenmusic
	cp res/* /usr/local/share/zenmusic/

remove:
	rm /usr/local/bin/zenmusic
	rm -r /usr/local/share/zenmusic
