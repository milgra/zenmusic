#ifndef evthan_touch_h
#define evthan_touch_h

#include "view.c"

typedef struct _eh_touch_t
{
  void* data;
  void (*event)(ev_t ev, void* data); /* event handler for view */
} eh_touch_t;

void eh_touch_add(view_t* view, void* data, void (*event)(ev_t event, void* data));

#endif

#if __INCLUDE_LEVEL__ == 0

void eh_touch_evt(view_t* view, ev_t ev)
{
  eh_touch_t* eh = view->evt_han_data;
  if (ev.type == EV_MDOWN)
  {
    if (ev.x < view->frame_global.x + view->frame_global.w &&
        ev.x > view->frame_global.x &&
        ev.y < view->frame_global.y + view->frame_global.h &&
        ev.y > view->frame_global.y)
    {
      (*eh->event)(ev, eh->data);
    }
  }
}

void eh_touch_add(view_t* view, void* data, void (*event)(ev_t ev, void* data))
{
  eh_touch_t* eh = mtmem_calloc(sizeof(eh_touch_t), "eh_touch", NULL, NULL);
  eh->data       = data;
  eh->event      = event;

  view->evt_han      = eh_touch_evt;
  view->evt_han_data = eh;
}

#endif
