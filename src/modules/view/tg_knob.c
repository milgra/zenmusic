
#ifndef texgen_knob_h
#define texgen_knob_h

#include "view.c"

typedef struct _tg_knob_t
{
  float angle;
  bm_t* back;
  bm_t* fore;
} tg_knob_t;

void tg_knob_add(view_t* view);
void tg_knob_set_angle(view_t* view, float angle);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"

void tg_knob_gen(view_t* view)
{
  tg_knob_t* tg = view->tex_gen_data;

  if (view->frame.local.w > 0 && view->frame.local.h > 0)
  {
    if (view->texture.bitmap == NULL && view->frame.local.w > 0 && view->frame.local.h > 0)
    {
      bm_t* bmp = bm_new(view->frame.local.w, view->frame.local.h);
      tg->back  = bm_new(view->frame.local.w, view->frame.local.h);
      tg->fore  = bm_new(view->frame.local.w, view->frame.local.h);

      gfx_arc_grad(tg->back,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   (view->frame.local.w / 2.0) - 3.0,
                   (view->frame.local.w / 2.0),
                   0,
                   3.14 * 2,
                   0x00000044,
                   0);

      gfx_arc_grad(tg->back,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   0,
                   (view->frame.local.w / 2.0) - 2.0,
                   0,
                   3.14 * 2,
                   0xEEEEEEFF,
                   0xEEEEEEFF);

      gfx_arc_grad(tg->back,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   27,
                   35,
                   0,
                   3.14 * 2,
                   0x00000011,
                   0x00000011);

      gfx_arc_grad(tg->fore,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   27.0,
                   30.0,
                   0,
                   3.14 * 2,
                   0x00000044,
                   0);

      gfx_arc_grad(tg->fore,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   0,
                   28.0,
                   0,
                   3.14 * 2,
                   0xDDDDDDFF,
                   0xDDDDDDFF);

      view_set_texture_bmp(view, bmp);
    }

    if (tg->angle < 0) tg->angle += 6.28;

    gfx_insert(view->texture.bitmap, tg->back, 0, 0);

    if (tg->angle > 3.14 * 3 / 2)
    {
      gfx_arc_grad(view->texture.bitmap,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   27.0,
                   35.0,
                   3.14 * 3 / 2,
                   tg->angle,
                   0x999999FF,
                   0x999999FF);
    }
    else
    {
      gfx_arc_grad(view->texture.bitmap,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   27.0,
                   35.0,
                   3.14 * 3 / 2,
                   6.28,
                   0x999999FF,
                   0x999999FF);

      gfx_arc_grad(view->texture.bitmap,
                   (view->frame.local.w - 1.0) / 2.0,
                   (view->frame.local.h - 1.0) / 2.0,
                   27.0,
                   35.0,
                   0,
                   tg->angle,
                   0x999999FF,
                   0x999999FF);
    }

    gfx_blend_rgba(view->texture.bitmap, 0, 0, tg->fore);
    view->texture.changed = 1;
  }
}

void tg_knob_add(view_t* view)
{
  tg_knob_t* tg = mem_calloc(sizeof(tg_knob_t), "tg_knob", NULL, NULL);
  tg->angle     = 3 * 3.14 / 2;

  view->tex_gen_data = tg;
  view->tex_gen      = tg_knob_gen;
}

void tg_knob_set_angle(view_t* view, float angle)
{
  tg_knob_t* tg = view->tex_gen_data;

  tg->angle           = angle;
  view->texture.state = TS_BLANK; // force rerender
}

#endif
