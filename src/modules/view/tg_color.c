/*
  Solid color texture generator
  Shows color in view
 */

#ifndef texgen_color_h
#define texgen_color_h

#include "view.c"

typedef struct _tg_color_t
{
  uint32_t color;
} tg_color_t;

void tg_color_add(view_t* view, uint32_t color);

#endif

#if __INCLUDE_LEVEL__ == 0

void tg_color_gen(view_t* view)
{
  tg_color_t* gen = view->tex_gen_data;

  char idbuffer[100] = {0};
  snprintf(idbuffer, 20, "color %i", gen->color);

  bm_t* bmp = bm_new(view->frame.local.w, view->frame.local.h);
  bm_fill(bmp,
          0,
          0,
          view->frame.local.w,
          view->frame.local.h,
          gen->color);
  view_set_texture(view, bmp, idbuffer);
}

void tg_color_add(view_t* view, uint32_t color)
{
  tg_color_t* gen = mtmem_alloc(sizeof(tg_color_t), "tg_color_t", NULL, NULL);
  gen->color      = color;

  view->texture.state = TS_BLANK;
  view->tex_gen_data  = gen;
  view->tex_gen       = tg_color_gen;
}

#endif
