#ifndef vh_anim_h
#define vh_anim_h

#include "mtmath2.c"
#include "view.c"

typedef enum _animtype_t
{
  AT_LINEAR,
  AT_EASE,
  AT_EASE_IN,
  AT_EASE_OUT
} animtype_t;

typedef struct _vh_anim_t
{
  animtype_t type;
  r2_t       sf; // starting frame
  r2_t       ef; // ending frame
  r2_t       cf; // current frame
  int        step;
  int        steps;
} vh_anim_t;

void vh_anim_set(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type);
void vh_anim_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_anim_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    vh_anim_t* vh = view->handler_data;

    if (vh->step < vh->steps)
    {
      vh->step += 1;

      r2_t sf = vh->sf;
      r2_t cf = sf;
      r2_t ef = vh->ef;

      if (vh->type == AT_LINEAR)
      {
        // just increase current with delta
        cf.x = sf.x + ((ef.x - sf.x) / vh->steps) * vh->step;
        cf.y = sf.y + ((ef.y - sf.y) / vh->steps) * vh->step;
        cf.w = sf.w + ((ef.w - sf.w) / vh->steps) * vh->step;
        cf.h = sf.h + ((ef.h - sf.h) / vh->steps) * vh->step;
      }
      else if (vh->type == AT_EASE)
      {
        // speed function based on cosine ( half circle )
        float angle = 3.14 + (3.14 / vh->steps) * vh->step;
        float delta = (cos(angle) + 1.0) / 2.0;

        cf.x = sf.x + (ef.x - sf.x) * delta;
        cf.y = sf.y + (ef.y - sf.y) * delta;
        cf.w = sf.w + (ef.w - sf.w) * delta;
        cf.h = sf.h + (ef.h - sf.h) * delta;
      }

      vh->cf = cf;
      view_set_frame(view, cf);
    }
  }
}

void vh_anim_set(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type)
{
  vh_anim_t* vh = view->handler_data;
  vh->sf        = sf;
  vh->cf        = sf;
  vh->ef        = ef;
  vh->step      = 0;
  vh->type      = type;
  vh->steps     = steps;
}

void vh_anim_add(view_t* view)
{
  vh_anim_t* vh = mem_calloc(sizeof(vh_anim_t), "vh_anim", NULL, NULL);

  view->handler      = vh_anim_evt;
  view->handler_data = vh;
}

#endif
