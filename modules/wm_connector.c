#ifndef wm_connector_h
#define wm_connector_h

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

extern SDL_GLContext* wm_context;
extern float wm_scale;

#endif

#if __INCLUDE_LEVEL__ == 0

void
wm_init()
{}

void
wm_loop()
{}

void
wm_free()
{}

#endif
