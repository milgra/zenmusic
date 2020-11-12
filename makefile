CC = clang
OBJDIR = bin/obj

SOURCES = $(wildcard src/*.c) \
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

OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))

zenmusic: $(OBJECTS)
	$(CC) $^ -o bin/$@ $(LDFLAGS)

# As we keep the source tree we have to create the
# needed directories for every object
$(OBJECTS): $(OBJDIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJECTS) zenmusic
