#ifndef vh_sbar_h
#define vh_sbar_h

#include "view.c"

typedef enum _sbartype_t
{
  SBAR_H,
  SBAR_V,
} sbartype_t;

void vh_sbar_add(view_t* view, sbartype_t type, int steps, void (*scroll)(view_t* view, void* userdata, float ratio), void* userdata);
void vh_sbar_open(view_t* view);
void vh_sbar_close(view_t* view);
void vh_sbar_update(view_t* view, float pos, float size);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_graphics.c"
#include <stdio.h>

typedef struct _vh_sbar_t
{
  sbartype_t type;
  int        step;
  int        steps;
  int        delta;
  float      pos;
  float      size;
  float      fpos; // final pos
  float      fsize;
  void (*scroll)(view_t*, void*, float);
  void* userdata;
} vh_sbar_t;

void vh_sbar_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    vh_sbar_t* vh = view->handler_data;

    // animation is not ready
    if (vh->step > 0 && vh->step < vh->steps)
    {
      vh->pos += (vh->fpos - vh->pos) / 5.0;
      vh->size += (vh->fsize - vh->size) / 5.0;

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
          if (vh->delta > 0) gfx_rect(bm, 0, 0, bm->w, bm->h, 0x00000055, 0);

          if (vh->type == SBAR_V)
          {
            float radius = view->frame.local.w * ratio;
            gfx_circle(bm, bm->w, vh->pos + vh->size / 2, radius, 1, 0x000000BB);
          }
          else
          {
            float radius = view->frame.local.h * ratio * 0.5;
            gfx_circle(bm, vh->pos + vh->size / 2, bm->h, radius, 1, 0x000000BB);
          }
          view->texture.changed = 1;
        }
        else
        {
          // bar state
          ratio = (float)(vh->step - vh->steps / 3) / (float)(vh->steps / 3 * 2);
          bm_reset(bm);
          if (vh->delta > 0) gfx_rect(bm, 0, 0, bm->w, bm->h, 0x00000055, 0);

          float size = vh->size * ratio;
          float pos  = vh->pos + vh->size / 2 - size / 2;

          if (vh->type == SBAR_V)
          {
            gfx_circle(bm, bm->w, pos, bm->w + 1, 1, 0x000000BB);
            gfx_circle(bm, bm->w, pos + size, bm->w + 1, 1, 0x000000BB);
            gfx_rect(bm, 0, pos, bm->w, size, 0x000000BB, 0);
          }
          else
          {
            gfx_circle(bm, pos, bm->h, bm->h + 1, 1, 0x000000BB);
            gfx_circle(bm, pos + size, bm->h, bm->h + 1, 1, 0x000000BB);
            gfx_rect(bm, pos, 0, size, bm->h, 0x000000BB, 0);
          }
          view->texture.changed = 1;
        }

        vh->step += vh->delta;
      }
    }
    else if (vh->step >= vh->steps)
    {
      vh->pos += (vh->fpos - vh->pos) / 5.0;
      vh->size += (vh->fsize - vh->size) / 5.0;

      bm_t* bm = view->texture.bitmap;
      bm_reset(bm);

      gfx_rect(bm, 0, 0, bm->w, bm->h, 0x00000055, 0);

      if (vh->type == SBAR_V)
      {
        gfx_circle(bm, bm->w, vh->pos, bm->w + 1, 1, 0x000000BB);
        gfx_circle(bm, bm->w, vh->pos + vh->size, bm->w + 1, 1, 0x000000BB);
        gfx_rect(bm, 0, vh->pos, bm->w, vh->size, 0x000000BB, 0);
      }
      else
      {
        gfx_circle(bm, vh->pos, bm->h, bm->h + 1, 1, 0x000000BB);
        gfx_circle(bm, vh->pos + vh->size, bm->h, bm->h + 1, 1, 0x000000BB);
        gfx_rect(bm, vh->pos, 0, vh->size, bm->h, 0x000000BB, 0);
      }
      view->texture.changed = 1;
    }
  }
  else if (ev.type == EV_MMOVE)
  {
    vh_sbar_t* vh = view->handler_data;

    if (ev.drag)
    {
      if (vh->step == 0) vh_sbar_open(view);

      float ratio;
      if (vh->type == SBAR_V) ratio = (ev.y - view->frame.global.y) / view->frame.local.h;
      if (vh->type == SBAR_H) ratio = (ev.x - view->frame.global.x) / view->frame.local.w;

      if (vh->scroll) (*vh->scroll)(view, vh->userdata, ratio);
    }
  }
  else if (ev.type == EV_MDOWN)
  {
  }
}

void vh_sbar_add(view_t* view, sbartype_t type, int steps, void (*scroll)(view_t* view, void* userdata, float ratio), void* userdata)
{
  vh_sbar_t* vh = mem_calloc(sizeof(vh_sbar_t), "vh_sbar", NULL, NULL);

  vh->type     = type;
  vh->steps    = steps;
  vh->scroll   = scroll;
  vh->userdata = userdata;

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

void vh_sbar_update(view_t* view, float pr, float sr)
{
  vh_sbar_t* vh = view->handler_data;

  if (sr < 0.1) sr = 0.1;

  if (vh->type == SBAR_V)
  {
    float max = view->frame.local.h - view->frame.local.w;

    vh->fsize = max * sr;
    vh->fpos  = view->frame.local.w / 2 + (max - vh->size) * pr;
  }
  else
  {
    float max = view->frame.local.w - view->frame.local.h;

    vh->fsize = max * sr;
    vh->fpos  = view->frame.local.h / 2 + (max - vh->size) * pr;
  }
}

#endif
