#ifndef evthan_anim_h
#define evthan_anim_h

#include "mtmath2.c"
#include "view.c"

typedef enum _animtype_t
{
  AT_LINEAR,
  AT_EASE,
  AT_EASE_IN,
  AT_EASE_OUT
} animtype_t;

typedef struct _eh_anim_t
{
  animtype_t type;
  r2_t       sf; // starting frame
  r2_t       ef; // ending frame
  r2_t       cf; // current frame
  int        step;
  int        steps;
} eh_anim_t;

void eh_anim_set(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type);
void eh_anim_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void eh_anim_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    eh_anim_t* eh = view->evt_han_data;

    if (eh->step < eh->steps)
    {
      eh->step += 1;

      r2_t sf = eh->sf;
      r2_t cf = sf;
      r2_t ef = eh->ef;

      if (eh->type == AT_LINEAR)
      {
        // just increase current with delta
        cf.x = sf.x + ((ef.x - sf.x) / eh->steps) * eh->step;
        cf.y = sf.y + ((ef.y - sf.y) / eh->steps) * eh->step;
        cf.w = sf.w + ((ef.w - sf.w) / eh->steps) * eh->step;
        cf.h = sf.h + ((ef.h - sf.h) / eh->steps) * eh->step;
      }
      else if (eh->type == AT_EASE)
      {
        // speed function based on cosine ( half circle )
        float angle = 3.14 + (3.14 / eh->steps) * eh->step;
        float delta = (cos(angle) + 1.0) / 2.0;

        cf.x = sf.x + (ef.x - sf.x) * delta;
        cf.y = sf.y + (ef.y - sf.y) * delta;
        cf.w = sf.w + (ef.w - sf.w) * delta;
        cf.h = sf.h + (ef.h - sf.h) * delta;
      }

      eh->cf = cf;
      view_set_frame(view, cf);
    }
  }
}

void eh_anim_set(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type)
{
  eh_anim_t* eh = view->evt_han_data;
  eh->sf        = sf;
  eh->cf        = sf;
  eh->ef        = ef;
  eh->step      = 0;
  eh->type      = type;
  eh->steps     = steps;
}

void eh_anim_add(view_t* view)
{
  eh_anim_t* eh = mtmem_calloc(sizeof(eh_anim_t), "eh_anim", NULL, NULL);

  view->evt_han      = eh_anim_evt;
  view->evt_han_data = eh;
}

#endif
