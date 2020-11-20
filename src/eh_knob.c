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
} eh_knob_t;

void eh_knob_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    if (view->texture.bitmap == NULL && view->frame.local.w > 0 && view->frame.local.h > 0)
    {
      bm_t* bmp = bm_new(view->frame.local.w, view->frame.local.h);

      mtgraphics_circle(bmp, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, (view->frame.local.w / 2.0 - 2.0), 1.0, 0x999999FF);
      mtgraphics_circle(bmp, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 30.0, 1.0, 0x666666FF);

      char idbuffer[100] = {0};
      snprintf(idbuffer, 20, "knob %i", rand());

      view_set_texture(view, bmp, idbuffer);
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
      float dx    = ev.x - (view->frame.global.x + view->frame.global.w / 2.0);
      float dy    = ev.y - (view->frame.global.y + view->frame.global.h / 2.0);
      float angle = atan2(dy, dx);

      if (angle < 0) angle += 6.28;

      printf("angle %f\n", angle * 180 / 3.14);
      mtgraphics_circle(view->texture.bitmap, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, (view->frame.local.w / 2.0 - 2.0), 1.0, 0x999999FF);

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

      mtgraphics_circle(view->texture.bitmap,
                        (view->frame.local.w - 1.0) / 2.0,
                        (view->frame.local.h - 1.0) / 2.0,
                        30.0,
                        2.0,
                        0x666666FF);

      view->texture.changed = 1;

      eh_knob_t* eh = view->evt_han_data;
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
