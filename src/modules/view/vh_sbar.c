#ifndef vh_sbar_h
#define vh_sbar_h

#include "view.c"

typedef enum _sbartype_t
{
  SBAR_H,
  SBAR_V,
} sbartype_t;

void vh_sbar_add(view_t* view, sbartype_t type, int steps);
void vh_sbar_open(view_t* view);
void vh_sbar_close(view_t* view);
void vh_sbar_update(view_t* view, float pos, float size);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"
#include <stdio.h>

typedef struct _vh_sbar_t
{
  sbartype_t type;
  int        step;
  int        steps;
  int        delta;
  float      pos;
  float      size;
} vh_sbar_t;

void vh_sbar_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    vh_sbar_t* vh = view->handler_data;

    // animation is not ready
    if (vh->step > 0 && vh->step < vh->steps)
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
        if (vh->step < vh->steps / 3)
        {
          // dot state
          ratio = (float)vh->step / ((float)vh->steps / 3.0);
          bm_reset(bm);

          if (vh->type == SBAR_V)
          {
            float radius = view->frame.local.w * ratio * 0.5;
            gfx_circle(bm, bm->w / 2, vh->pos + vh->size / 2, radius, 1, 0x000000FF);
          }
          else
          {
            float radius = view->frame.local.h * ratio * 0.5;
            gfx_circle(bm, vh->pos + vh->size / 2, bm->h / 2, radius, 1, 0x000000FF);
          }
          view->texture.changed = 1;
        }
        else
        {
          // bar state
          ratio = (float)(vh->step - vh->steps / 3) / (float)(vh->steps / 3 * 2);
          bm_reset(bm);

          if (vh->type == SBAR_V)
          {
            float height = vh->size * ratio;
            float pos    = vh->pos + vh->size / 2 - height / 2;

            gfx_circle(bm, bm->w / 2, pos, bm->w / 2, 1, 0x000000FF);
            gfx_circle(bm, bm->w / 2, pos + height, bm->w / 2, 1, 0x000000FF);
            gfx_rect(bm, 1, pos, bm->w - 2, height, 0x000000FF, 0);
          }
          else
          {
            float width = vh->size * ratio;
            float pos   = vh->pos + vh->size / 2 - width / 2;

            gfx_circle(bm, pos, bm->h / 2, bm->h / 2, 1, 0x000000FF);
            gfx_circle(bm, pos + width, bm->h / 2, bm->h / 2, 1, 0x000000FF);
            gfx_rect(bm, pos, 1, width, bm->h - 2, 0x000000FF, 0);
          }
          view->texture.changed = 1;
        }

        vh->step += vh->delta;
      }
    }
  }
}

void vh_sbar_add(view_t* view, sbartype_t type, int steps)
{
  vh_sbar_t* vh = mem_calloc(sizeof(vh_sbar_t), "vh_sbar", NULL, NULL);

  vh->type  = type;
  vh->steps = steps;

  view->handler_data = vh;
  view->handler      = vh_sbar_evt;
}

void vh_sbar_open(view_t* view)
{
  vh_sbar_t* vh = view->handler_data;
  vh->delta     = 1;
  vh->step      = 1;
}

void vh_sbar_close(view_t* view)
{
  vh_sbar_t* vh = view->handler_data;
  vh->delta     = -1;
  vh->step      = vh->steps - 1;
}

void vh_sbar_update(view_t* view, float pos, float size)
{
  vh_sbar_t* vh = view->handler_data;
  vh->pos       = pos;
  vh->size      = size;

  if (vh->step >= vh->steps)
  {
    bm_t* bm = view->texture.bitmap;
    bm_reset(bm);
    if (vh->type == SBAR_V)
    {
      gfx_circle(bm, bm->w / 2, vh->pos, bm->w / 2, 1, 0x000000FF);
      gfx_circle(bm, bm->w / 2, vh->pos + vh->size, bm->w / 2, 1, 0x000000FF);
      gfx_rect(bm, 1, vh->pos, bm->w - 2, vh->size, 0x000000FF, 0);
    }
    else
    {
      gfx_circle(bm, vh->pos, bm->h / 2, bm->h / 2, 1, 0x000000FF);
      gfx_circle(bm, vh->pos + vh->size, bm->h / 2, bm->h / 2, 1, 0x000000FF);
      gfx_rect(bm, vh->pos, 1, vh->size, bm->h - 2, 0x000000FF, 0);
    }
    view->texture.changed = 1;
  }
}

#endif
