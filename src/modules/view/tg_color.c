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
  tg_color_t* gen = view->tgdata;

  bm_t* bmp = bm_new(view->frame.w, view->frame.h);
  bm_fill(bmp,
          0,
          0,
          view->frame.w,
          view->frame.h,
          gen->color);
  view_set_texture(view, bmp);
}

void tg_color_add(view_t* view, uint32_t color)
{
  tg_color_t* gen = mtmem_alloc(sizeof(tg_color_t), "tg_color_t", NULL, NULL);
  gen->color      = color;

  view->tex_state = TS_BLANK;
  view->tgdata    = gen;
  view->tg        = tg_color_gen;
}

#endif
