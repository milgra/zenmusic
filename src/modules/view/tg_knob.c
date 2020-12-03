
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

      mtgraphics_arc_grad(tg->back,
                          (view->frame.local.w - 1.0) / 2.0,
                          (view->frame.local.h - 1.0) / 2.0,
                          (view->frame.local.w / 2.0) - 7.0,
                          (view->frame.local.w / 2.0),
                          0,
                          3.14 * 2,
                          0x00000044,
                          0);

      mtgraphics_arc_grad(tg->back,
                          (view->frame.local.w - 1.0) / 2.0,
                          (view->frame.local.h - 1.0) / 2.0,
                          0,
                          (view->frame.local.w / 2.0) - 7.0,
                          0,
                          3.14 * 2,
                          0xDEDEDEFF,
                          0xDEDEDEFF);

      /* mtgraphics_circle(tg->back, */
      /*                   (view->frame.local.w - 1.0) / 2.0, */
      /*                   (view->frame.local.h - 1.0) / 2.0, */
      /*                   (view->frame.local.w / 2.0 - 5.0), */
      /*                   2.0, */
      /* 0xFF0000FF); */

      mtgraphics_circle(tg->fore,
                        (view->frame.local.w - 1.0) / 2.0,
                        (view->frame.local.h - 1.0) / 2.0,
                        30.0,
                        4.0,
                        0x00000044);

      mtgraphics_circle(tg->fore,
                        (view->frame.local.w - 1.0) / 2.0,
                        (view->frame.local.h - 1.0) / 2.0,
                        26.0,
                        2.0,
                        0x666666FF);

      view_set_texture_bmp(view, bmp);
    }

    if (tg->angle < 0) tg->angle += 6.28;

    bm_insert(view->texture.bitmap, tg->back, 0, 0);

    if (tg->angle > 3.14 * 3 / 2)
    {
      mtgraphics_arc(view->texture.bitmap,
                     (view->frame.local.w - 1.0) / 2.0,
                     (view->frame.local.h - 1.0) / 2.0,
                     (view->frame.local.w / 2.0 - 2.0),
                     1.0,
                     0x000000FF,
                     3.14 * 3 / 2,
                     tg->angle);
    }
    else
    {
      mtgraphics_arc(view->texture.bitmap,
                     (view->frame.local.w - 1.0) / 2.0,
                     (view->frame.local.h - 1.0) / 2.0,
                     (view->frame.local.w / 2.0 - 2.0),
                     1.0,
                     0x000000FF,
                     3.14 * 3 / 2,
                     6.28);

      mtgraphics_arc(view->texture.bitmap,
                     (view->frame.local.w - 1.0) / 2.0,
                     (view->frame.local.h - 1.0) / 2.0,
                     (view->frame.local.w / 2.0 - 2.0),
                     1.0,
                     0x000000FF,
                     0,
                     tg->angle);
    }

    mtgraphics_circle(view->texture.bitmap,
                      (view->frame.local.w - 1.0) / 2.0 + 39 * cos(tg->angle),
                      (view->frame.local.h - 1.0) / 2.0 + 39 * sin(tg->angle),
                      7.0,
                      1.0,
                      0xABABABFF);

    bm_insert_blend(view->texture.bitmap, tg->fore, 0, 0);
    view->texture.changed = 1;
  }
}

void tg_knob_add(view_t* view)
{
  tg_knob_t* tg = mtmem_calloc(sizeof(tg_knob_t), "tg_knob", NULL, NULL);
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
