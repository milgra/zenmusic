/*
 UI Connector Module for Zen Multimedia Desktop System
 */

#ifndef ui_connector_h
#define ui_connector_h

void ui_init(int, int);
void ui_render();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "gl_connector.c"
#include "mtbm.c"
#include "mtfb.c"

fb_t* vert_buf;
bm_t* tex_bmp;

void ui_init(int width, int height)
{
  gl_init(width, height);
  tex_bmp = bm_new(1024, 1024);
  bm_fill(tex_bmp, 0, 0, 1024, 1024, 0xFF0000FF);
  bm_fill(tex_bmp, 512, 512, 1024, 1024, 0x00FFFFFF);

  GLfloat vertexes[] = {
      0.0, 0.0, 0.0f, 0.0f,
      1024.0, 0.0, 1.0f, 0.0f,
      0.0, -1024.0, 0.0f, 1.0f,
      1024.0, 0.0, 1.0f, 0.0f,
      1024.0, -1024.0, 1.0f, 1.0f,
      0.0, -1024.0, 0.0f, 1.0f};

  vert_buf = fb_alloc();
  fb_app_arr(vert_buf, vertexes, 30);
}

void ui_render()
{
  gl_render(vert_buf, tex_bmp);
}

#endif
