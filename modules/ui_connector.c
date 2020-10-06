/*
 UI Connector Module for Zen Multimedia Desktop System
 */

#ifndef ui_connector_h
#define ui_connector_h

void
ui_init(int, int);
void
ui_render();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_connector.c"

void
ui_init(int width, int height)
{
  gl_init(width, height);
}

void
ui_render()
{
  gl_render();
}

#endif
