#ifndef vh_touch_h
#define vh_touch_h

#include "view.c"
#include "zc_callback.c"

void vh_touch_add(view_t* view, cb_t* event);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "wm_event.c"

typedef struct _vh_touch_t
{
  cb_t* event;
} vh_touch_t;

void vh_touch_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_touch_t* vh = view->handler_data;
    if (vh->event) (*vh->event->fp)(vh->event->userdata, view);
  }
}

void vh_touch_add(view_t* view, cb_t* event)
{
  vh_touch_t* vh = mem_calloc(sizeof(vh_touch_t), "vh_touch", NULL, NULL);
  vh->event      = event;

  if (event) RET(event);

  view->handler      = vh_touch_evt;
  view->handler_data = vh;
}

#endif
