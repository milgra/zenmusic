#ifndef eh_sbar_h
#define ev_sbar_h

#include "view.c"

typedef enum _sbartype_t
{
  SBAR_H,
  SBAR_V,
} sbartype_t;

void eh_sbar_add(view_t* view, sbartype_t type, int steps);
void eh_sbar_open(view_t* view);
void eh_sbar_close(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"
#include <stdio.h>

typedef struct _eh_sbar_t
{
  sbartype_t type;
  int        step;
  int        steps;
  int        delta;
} eh_sbar_t;

void eh_sbar_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    eh_sbar_t* eh = view->evt_han_data;

    // animation is not ready
    if (eh->step > 0 && eh->step < eh->steps)
    {
      // avoid invalid bitmaps
      if (view->frame.local.w >= 1.0 &&
          view->frame.local.h >= 1.0)
      {
        bm_t* bm = view->texture.bitmap;

        if (bm == NULL ||
            bm->w != (int)view->frame.local.w ||
            bm->h != (int)view->frame.local.h)
        {
          bm = bm_new((int)view->frame.local.w, (int)view->frame.local.h);
          view_set_texture_bmp(view, bm);
        }

        float ratio;
        if (eh->step < eh->steps / 3)
        {
          // dot state
          ratio = (float)eh->step / ((float)eh->steps / 3.0);
          bm_reset(bm);

          if (eh->type == SBAR_V)
          {
            float radius = view->frame.local.w * ratio * 0.5;
            gfx_circle(bm, bm->w / 2, bm->h / 2, radius, 1, 0x000000FF);
          }
          else
          {
            float radius = view->frame.local.h * ratio * 0.5;
            gfx_circle(bm, bm->w / 2, bm->h / 2, radius, 1, 0x000000FF);
          }
          view->texture.changed = 1;
        }
        else
        {
          // bar state
          ratio = (float)(eh->step - eh->steps / 3) / (float)(eh->steps / 3 * 2);
          bm_reset(bm);

          if (eh->type == SBAR_V)
          {
            float height = (view->frame.local.h - view->frame.local.w) * ratio;

            gfx_circle(bm, bm->w / 2, bm->h / 2 - height / 2, bm->w / 2, 1, 0x000000FF);
            gfx_circle(bm, bm->w / 2, bm->h / 2 + height / 2, bm->w / 2, 1, 0x000000FF);
            gfx_rect(bm, 1, bm->h / 2 - height / 2, bm->w - 2, height, 0x000000FF, 0);
          }
          else
          {
            float width = (view->frame.local.w - view->frame.local.h) * ratio;

            gfx_circle(bm, bm->w / 2 - width / 2, bm->h / 2, bm->h / 2, 1, 0x000000FF);
            gfx_circle(bm, bm->w / 2 + width / 2, bm->h / 2, bm->h / 2, 1, 0x000000FF);
            gfx_rect(bm, bm->w / 2 - width / 2, 1, width, bm->h - 2, 0x000000FF, 0);
          }
          view->texture.changed = 1;
        }

        eh->step += eh->delta;
      }
    }
  }
}

void eh_sbar_add(view_t* view, sbartype_t type, int steps)
{
  eh_sbar_t* eh = mem_calloc(sizeof(eh_sbar_t), "eh_sbar", NULL, NULL);

  eh->type  = type;
  eh->steps = steps;

  view->evt_han_data = eh;
  view->evt_han      = eh_sbar_evt;
}

void eh_sbar_open(view_t* view)
{
  eh_sbar_t* eh = view->evt_han_data;
  eh->delta     = 1;
  eh->step      = 1;
}

void eh_sbar_close(view_t* view)
{
  eh_sbar_t* eh = view->evt_han_data;
  eh->delta     = -1;
  eh->step      = eh->steps - 1;
}

#endif
