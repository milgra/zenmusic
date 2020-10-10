#ifndef evthan_drag_h
#define evthan_drag_h

#include "view.c"
#include "wm_event.c"

void evthan_drag(view_t* view, ev_t event);

#endif

#if __INCLUDE_LEVEL__ == 0

void evthan_drag(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE && ev.drag)
  {
    view_setpos(view, (v2_t){ev.x, ev.y});
  }
}

#endif
