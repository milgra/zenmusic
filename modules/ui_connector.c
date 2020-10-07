/*
 UI Connector Module for Zen Multimedia Desktop System

  events -> ui_generator -> ui_connector -> ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_connector_h
#define ui_connector_h

void ui_connector_init(int, int);
void ui_connector_render();
void ui_connector_update(int x, int y);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "ui_compositor.c"

int boxy = 0;

void ui_connector_init(int width, int height)
{
  ui_compositor_init(width, height);

  bm_t* gr_bmp = bm_new(256, 256);
  bm_t* bl_bmp = bm_new(150, 70);

  bm_fill(gr_bmp, 0, 0, 256, 256, 0x00FF00FF);
  bm_fill(bl_bmp, 0, 0, 150, 70, 0x0000FFFF);

  ui_compositor_add("id1", 20.0, 20.0, 150.0, 70.0, bl_bmp);
  ui_compositor_add("id2", 200.0, 420.0, 256.0, 256.0, gr_bmp);
}

void ui_connector_update(int x, int y)
{
}

void ui_connector_render()
{
}

#endif
