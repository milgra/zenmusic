#ifndef evthan_drag_h
#define evthan_drag_h

#include "view.c"
#include "wm_event.c"

void eh_drag_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void eh_drag_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE && ev.drag)
  {
    vframe_t frame = view->frame;
    frame.x        = ev.x;
    frame.y        = ev.y;
    view_set_frame(view, frame);
  }
}

void eh_drag_add(view_t* view)
{
  view->eh = eh_drag_evt;
}

#endif
