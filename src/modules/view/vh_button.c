#ifndef vh_button_h
#define vh_button_h

#include "mtcallback.c"
#include "view.c"

typedef struct _vh_button_t
{
  cb_t* event;
} vh_button_t;

void vh_button_add(view_t* view, cb_t* event);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_button_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_button_t* vh = view->handler_data;
    if (vh->event) (*vh->event->fp)(vh->event->userdata, view);
  }
}

void vh_button_add(view_t* view, cb_t* event)
{
  vh_button_t* vh = mem_calloc(sizeof(vh_button_t), "vh_button", NULL, NULL);
  vh->event       = event;

  if (event) RET(event);

  view->handler      = vh_button_evt;
  view->handler_data = vh;
}

#endif
