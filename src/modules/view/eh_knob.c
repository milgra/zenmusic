#ifndef eh_knob_h
#define ev_knob_h

#include "view.c"

void eh_knob_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"
#include "tg_knob.c"
#include <stdio.h>

typedef struct _eh_knob_t
{
  float angle;
  char* id;
  bm_t* top;
  bm_t* bottom;
} eh_knob_t;

void eh_knob_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
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

      tg_knob_set_angle(view, angle);
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
