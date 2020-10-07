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
#include "math4.c"
#include "mtbm.c"
#include "mtfb.c"
#include "mttm.c"
#include "ui_rect.c"

fb_t* flt_buf;
tm_t* tex_map;

uir_t* brect;
uir_t* grect;
uir_t* frect;

int boxy = 0;

void ui_init(int width, int height)
{
  gl_init(width, height);

  flt_buf = fb_alloc();
  tex_map = tm_new();

  bm_t* gr_bmp = bm_new(256, 256);
  bm_t* bl_bmp = bm_new(150, 70);

  bm_fill(gr_bmp, 0, 0, 256, 256, 0x00FF00FF);
  bm_fill(bl_bmp, 0, 0, 150, 70, 0x0000FFFF);

  tm_put(tex_map, "green", gr_bmp);
  tm_put(tex_map, "blue", bl_bmp);

  v4_t btc = tm_get(tex_map, "blue");
  v4_t gtc = tm_get(tex_map, "green");

  brect = uir_new("id1", 20.0, 20.0, 150.0, 70.0, btc.x, btc.y, btc.z, btc.w);
  grect = uir_new("id2", 200.0, 420.0, 256.0, 256.0, gtc.x, gtc.y, gtc.z, gtc.w);

  frect = uir_new("full", 0, 0, 1024, 1024, 0.0, 0.0, 1.0, 1.0);
}

void ui_render()
{
  fb_reset(flt_buf);

  ++boxy;

  uir_set_dim(brect, 20.0, boxy, 150.0, 70.0);

  //  fb_app_arr(flt_buf, frect->vertexes, 24);
  fb_app_arr(flt_buf, brect->vertexes, 24);
  fb_app_arr(flt_buf, grect->vertexes, 24);

  gl_render(flt_buf, tex_map->bm);
}

#endif
