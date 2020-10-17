clang \
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
    -I/usr/local/include \
    -I/usr/local/include/GL \
    -I/usr/local/include/SDL2 \
    -Isrc \
    -Isrc/ui \
    -Isrc/mtlib \
    -Isrc/views \
    -Isrc/modules \
    -Isrc/modules/gl \
    -Isrc/modules/ui \
    -Isrc/modules/wm \
    -Isrc/modules/text \
    -Isrc/modules/player \
    src/modules/gl/gl_connector.c \
    src/modules/gl/gl_floatbuffer.c \
    src/modules/ui/ui_manager.c \
    src/modules/ui/view.c \
    src/modules/ui/eh_drag.c \
    src/modules/ui/tg_text.c \
    src/modules/ui/tg_color.c \
    src/modules/ui/tg_texmap.c \
    src/modules/ui/tg_video.c \
    src/modules/ui_connector.c \
    src/modules/ui_compositor.c \
    src/modules/wm/wm_event.c \
    src/modules/wm/wm_connector.c \
    src/modules/text/font.c \
    src/modules/text/text.c \
    src/modules/player/player.c \
    src/modules/player/clock.c \
    src/modules/player/codec.c \
    src/modules/player/frame.c \
    src/modules/player/packet.c \
    src/modules/player/stream.c \
    src/modules/player/video.c \
    src/modules/player/decoder.c \
    src/modules/player/strcomm.c \
    src/modules/player/render.c \
    src/modules/player/options.c \
    src/mtlib/mtbitmap.c \
    src/mtlib/mttexmap.c \
    src/mtlib/mtchannel.c \
    src/mtlib/mtmap.c \
    src/mtlib/mtmath2.c \
    src/mtlib/mtmath3.c \
    src/mtlib/mtmath4.c \
    src/mtlib/mtmemory.c \
    src/mtlib/mtvector.c \
    src/mtlib/mtstring.c \
    src/mtlib/mtcstring.c \
    src/eh_songs.c \
    src/zenmusic.c \
    src/common.c \
    -o bin/zenmusic
