#ifndef evthan_button_h
#define evthan_button_h

#include "view.c"

typedef struct _eh_button_t
{
  void* data;
  void (*event)(view_t* view, void* data); /* event handler for view */
} eh_button_t;

void eh_button_add(view_t* view, void* data, void (*event)(view_t* view, void* data));

#endif

#if __INCLUDE_LEVEL__ == 0

void eh_button_evt(view_t* view, ev_t ev)
{
  eh_button_t* eh = view->evt_han_data;
  if (ev.type == EV_MDOWN)
  {
    if (ev.x < view->frame.global.x + view->frame.global.w &&
        ev.x > view->frame.global.x &&
        ev.y < view->frame.global.y + view->frame.global.h &&
        ev.y > view->frame.global.y)
    {
      (*eh->event)(view, eh->data);
    }
  }
}

void eh_button_add(view_t* view, void* data, void (*event)(view_t* view, void* data))
{
  eh_button_t* eh = mtmem_calloc(sizeof(eh_button_t), "eh_button", NULL, NULL);
  eh->data        = data;
  eh->event       = event;

  view->evt_han      = eh_button_evt;
  view->evt_han_data = eh;
}

#endif
