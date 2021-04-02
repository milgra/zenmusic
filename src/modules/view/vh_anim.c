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

  r2_t sf; // starting frame
  r2_t ef; // ending frame

  r2_t sr; // starting region
  r2_t er; // ending region

  float sa; // starting alpha
  float ea; // ending alpha

  char anim_frame;
  char anim_region;
  char anim_alpha;

  int step;
  int steps;
} vh_anim_t;

void vh_anim_frame(view_t* view, r2_t sf, r2_t ef, int steps, animtype_t type);
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

      if (vh->anim_frame)
      {
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
        view_set_frame(view, cf);
      }

      if (vh->anim_region)
      {
        r2_t sr = vh->sr;
        r2_t cr = sr;
        r2_t er = vh->er;

        if (vh->type == AT_LINEAR)
        {
          // just increase current with delta
          cr.x = sr.x + ((er.x - sr.x) / vh->steps) * vh->step;
          cr.y = sr.y + ((er.y - sr.y) / vh->steps) * vh->step;
          cr.w = sr.w + ((er.w - sr.w) / vh->steps) * vh->step;
          cr.h = sr.h + ((er.h - sr.h) / vh->steps) * vh->step;
        }
        else if (vh->type == AT_EASE)
        {
          // speed function based on cosine ( half circle )
          float angle = 3.14 + (3.14 / vh->steps) * vh->step;
          float delta = (cos(angle) + 1.0) / 2.0;

          cr.x = sr.x + (er.x - sr.x) * delta;
          cr.y = sr.y + (er.y - sr.y) * delta;
          cr.w = sr.w + (er.w - sr.w) * delta;
          cr.h = sr.h + (er.h - sr.h) * delta;
        }
        view_set_region(view, cr);
      }

      if (vh->anim_alpha)
      {
        float sa = vh->sa;
        float ea = vh->ea;
        float ca = sa;

        if (vh->type == AT_LINEAR)
        {
          // just increase current with delta
          ca = sa + ((ea - sa) / vh->steps) * vh->step;
        }
        else if (vh->type == AT_EASE)
        {
          // speed function based on cosine ( half circle )
          float angle = 3.14 + (3.14 / vh->steps) * vh->step;
          float delta = (cos(angle) + 1.0) / 2.0;
          ca          = sa + (ea - sa) * delta;
        }
        view_set_texture_alpha(view, ca, 1);
      }

      if (vh->step == vh->steps - 1)
      {
        vh->anim_frame  = 0;
        vh->anim_region = 0;
        vh->anim_alpha  = 0;
      }
    }
  }
}

void vh_anim_frame(view_t*    view,
                   r2_t       sf,
                   r2_t       ef,
                   int        steps,
                   animtype_t type)
{
  vh_anim_t* vh = view->handler_data;
  vh->sf        = sf;
  vh->ef        = ef;
  vh->step      = 0;
  vh->type      = type;
  vh->steps     = steps;
}

void vh_anim_region(view_t*    view,
                    r2_t       sr,
                    r2_t       er,
                    int        steps,
                    animtype_t type)
{
  vh_anim_t* vh = view->handler_data;
  vh->sr        = sr;
  vh->er        = er;
  vh->step      = 0;
  vh->type      = type;
  vh->steps     = steps;
}

void vh_anim_alpha(view_t*    view,
                   float      sa,
                   float      ea,
                   int        steps,
                   animtype_t type)
{
  vh_anim_t* vh = view->handler_data;
  vh->sa        = sa;
  vh->ea        = ea;
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
