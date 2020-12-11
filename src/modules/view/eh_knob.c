#ifndef eh_knob_h
#define ev_knob_h

#include "view.c"

void eh_knob_add(view_t* view,
                 void (*ratio_changed)(view_t* view, float ratio),
                 void (*button_pushed)(view_t* view));

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

  void (*ratio_changed)(view_t* view, float ratio);
  void (*button_pushed)(view_t* view);
} eh_knob_t;

void eh_knob_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN || (ev.type == EV_MMOVE && ev.drag))
  {
    eh_knob_t* eh = view->evt_han_data;

    float dx    = ev.x - (view->frame.global.x + view->frame.global.w / 2.0);
    float dy    = ev.y - (view->frame.global.y + view->frame.global.h / 2.0);
    float angle = atan2(dy, dx);
    float r     = sqrt(dx * dx + dy * dy);

    if (angle < 0) angle += 6.28;

    if (r < 30.0)
    {
      (*eh->button_pushed)(view);
    }
    else if (r < view->frame.global.w / 2.0)
    {
      tg_knob_set_angle(view, angle);
      (*eh->ratio_changed)(view, angle);
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    eh_knob_t* eh = view->evt_han_data;
    tg_knob_t* tg = view->tex_gen_data;

    float angle = tg->angle - ev.dy / 50.0;
    if (angle < 0) angle += 6.28;
    if (angle > 6.28) angle -= 6.28;
    tg_knob_set_angle(view, angle);
    (*eh->ratio_changed)(view, angle);
  }
}

void eh_knob_add(view_t* view,
                 void (*ratio_changed)(view_t* view, float ratio),
                 void (*button_pushed)(view_t* view))
{
  eh_knob_t* eh = mem_calloc(sizeof(eh_knob_t), "eh_knob", NULL, NULL);

  eh->ratio_changed = ratio_changed;
  eh->button_pushed = button_pushed;

  view->needs_scroll = 1;
  view->evt_han_data = eh;
  view->evt_han      = eh_knob_evt;
}

#endif
