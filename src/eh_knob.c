#ifndef eh_knob_h
#define ev_knob_h

#include "view.c"

void eh_knob_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"
#include <stdio.h>

typedef struct _eh_knob_t
{
  char* id;
  bm_t* top;
  bm_t* bottom;
} eh_knob_t;

void eh_knob_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    if (view->texture.bitmap == NULL && view->frame.local.w > 0 && view->frame.local.h > 0)
    {
      eh_knob_t* eh = view->evt_han_data;

      bm_t* bmp  = bm_new(view->frame.local.w, view->frame.local.h);
      eh->top    = bm_new(view->frame.local.w, view->frame.local.h);
      eh->bottom = bm_new(view->frame.local.w, view->frame.local.h);

      mtgraphics_circle(eh->bottom,
                        (view->frame.local.w - 1.0) / 2.0,
                        (view->frame.local.h - 1.0) / 2.0,
                        (view->frame.local.w / 2.0 - 2.0),
                        1.0,
                        0x999999FF);
      mtgraphics_circle(eh->top,
                        (view->frame.local.w - 1.0) / 2.0,
                        (view->frame.local.h - 1.0) / 2.0,
                        30.0,
                        1.0,
                        0x666666FF);

      bm_insert(bmp, eh->bottom, 0, 0);
      bm_insert_blend(bmp, eh->top, 0, 0);
      view_set_texture_bmp(view, bmp);
    }
  }
  else if (ev.type == EV_MDOWN)
  {
    if (ev.x < view->frame.global.x + view->frame.global.w &&
        ev.x > view->frame.global.x &&
        ev.y < view->frame.global.y + view->frame.global.h &&
        ev.y > view->frame.global.y &&
        view->texture.bitmap)
    {
      eh_knob_t* eh = view->evt_han_data;

      float dx    = ev.x - (view->frame.global.x + view->frame.global.w / 2.0);
      float dy    = ev.y - (view->frame.global.y + view->frame.global.h / 2.0);
      float angle = atan2(dy, dx);

      if (angle < 0) angle += 6.28;

      bm_insert(view->texture.bitmap, eh->bottom, 0, 0);

      if (angle > 3.14 * 3 / 2)
      {
        mtgraphics_arc(view->texture.bitmap,
                       (view->frame.local.w - 1.0) / 2.0,
                       (view->frame.local.h - 1.0) / 2.0,
                       (view->frame.local.w / 2.0 - 2.0),
                       1.0,
                       0x000000FF,
                       3.14 * 3 / 2,
                       angle);
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
                       angle);
      }

      bm_insert_blend(view->texture.bitmap, eh->top, 0, 0);

      view->texture.changed = 1;
    }
  }
}

void eh_knob_add(view_t* view)
{
  eh_knob_t* eh = mtmem_calloc(sizeof(eh_knob_t), "eh_knob", NULL, NULL);

  view->evt_han_data = eh;
  view->evt_han      = eh_knob_evt;
}

#endif
