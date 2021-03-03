#ifndef vh_button_h
#define vh_button_h

#include "mtcallback.c"
#include "view.c"

typedef enum _vh_button_type_t
{
  VH_BUTTON_NORMAL,
  VH_BUTTON_TOGGLE
} vh_button_type_t;

typedef enum _vh_button_state_t
{
  VH_BUTTON_UP,
  VH_BUTTON_DOWN
} vh_button_state_t;

typedef struct _vh_button_t
{
  cb_t*             event;
  vh_button_type_t  type;
  vh_button_state_t state;
} vh_button_t;

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event);

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

void vh_button_set_state(view_t* view, vh_button_state_t state)

{
  vh_button_t* vh = view->handler_data;
  vh->state       = state;
}

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event)
{
  vh_button_t* vh = mem_calloc(sizeof(vh_button_t), "vh_button", NULL, NULL);
  vh->event       = event;
  vh->type        = type;
  vh->state       = VH_BUTTON_UP;

  if (event) RET(event);

  view->handler      = vh_button_evt;
  view->handler_data = vh;
}

#endif
