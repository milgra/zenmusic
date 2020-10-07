clang \
    -I/usr/local/include/SDL2 \
    -I/usr/local/include/GL \
    -I/usr/local/include \
    -L/usr/local/lib \
    -lSDL2 \
    -lGL \
    -lGLEW \
    -lm \
    -lpthread \
    -I./lib \
    -I./modules \
    modules/gl_connector.c \
    modules/wm_connector.c \
    modules/ui_connector.c \
    modules/ui_rect.c \
    lib/math2.c \
    lib/math3.c \
    lib/math4.c \
    lib/mtmem.c \
    lib/mtbm.c \
    lib/mttm.c \
    lib/mtpipe.c \
    lib/mtfb.c \
    lib/mtmap.c \
    lib/mtvec.c \
    lib/mtcstr.c \
    zenmusic.c \
    -o zenmusic
