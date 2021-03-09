#ifndef vh_fade_h
#define vh_fade_h

#include "view.c"

typedef struct _vh_fade_t
{
  float delta;
  int   step;
  int   steps;
  char  recur;
} vh_fade_t;

void vh_fade_set(view_t* view, float alpha, float steps, char recursive);
void vh_fade_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_fade_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    vh_fade_t* vh = view->handler_data;

    if (vh->step <= vh->steps)
    {
      view->texture.alpha += vh->delta;
      vh->step += 1;
      if (vh->step == vh->steps) view->texture.alpha = roundf(view->texture.alpha);

      view_set_texture_alpha(view, view->texture.alpha, vh->recur);
    }
  }
}

void vh_fade_set(view_t* view, float alpha, float steps, char recursive)
{
  vh_fade_t* vh = view->handler_data;
  vh->step      = 0;
  vh->steps     = steps;
  vh->delta     = (alpha - view->texture.alpha) / (float)steps;
  vh->recur     = recursive;
}

void vh_fade_add(view_t* view)
{
  vh_fade_t* vh = mem_calloc(sizeof(vh_fade_t), "vh_fade", NULL, NULL);
  vh->step      = 1;

  view->handler      = vh_fade_evt;
  view->handler_data = vh;
}

#endif
