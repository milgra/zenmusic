#ifndef vh_button_h
#define vh_button_h

#include "view.c"

typedef struct _vh_button_t
{
  void* data;
  void (*event)(view_t* view, void* data); /* event handler for view */
} vh_button_t;

void vh_button_add(view_t* view, void* data, void (*event)(view_t* view, void* data));

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_button_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_button_t* vh = view->handler_data;
    (*vh->event)(view, vh->data);
  }
}

void vh_button_add(view_t* view, void* data, void (*event)(view_t* view, void* data))
{
  vh_button_t* vh = mem_calloc(sizeof(vh_button_t), "vh_button", NULL, NULL);
  vh->data        = data;
  vh->event       = event;

  view->handler      = vh_button_evt;
  view->handler_data = vh;
}

#endif
