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

  char    inited;
  view_t* offview;
  view_t* onview;

  float ratio;
  float target;
} vh_button_t;

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_button_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    vh_button_t* vh = view->handler_data;
    if (!vh->inited)
    {
      vh->inited = 1;
      if (view->views->length > 0) vh->offview = view->views->data[0];
      if (view->views->length > 1) vh->onview = view->views->data[1];
      if (vh->offview) view_set_texture_alpha(vh->offview, 1.0);
      if (vh->onview) view_set_texture_alpha(vh->onview, 0.0);
    }

    if ((vh->target == 1.0 && vh->ratio < vh->target) ||
        (vh->target == 0.0 && vh->ratio > vh->target))
    {
      if (vh->ratio < vh->target)
        vh->ratio += 0.1;
      else
        vh->ratio -= 0.1;

      if (vh->offview) view_set_texture_alpha(vh->offview, 1.0 - vh->ratio);
      if (vh->onview) view_set_texture_alpha(vh->onview, vh->ratio);
    }
  }
  else if (ev.type == EV_MDOWN)
  {
    vh_button_t* vh = view->handler_data;

    if (vh->type == VH_BUTTON_NORMAL) vh->target = 1.0;
  }
  else if (ev.type == EV_MUP)
  {
    vh_button_t* vh = view->handler_data;

    if (vh->type == VH_BUTTON_TOGGLE)
    {
      if (vh->target == 1.0)
        vh->target = 0.0;
      else
      {
        vh->target = 1.0;
        if (vh->event) (*vh->event->fp)(vh->event->userdata, view);
      }
    }

    if (vh->type == VH_BUTTON_NORMAL && vh->event)
    {
      vh->target = 0.0;
      (*vh->event->fp)(vh->event->userdata, view);
    }
  }
}

void vh_button_set_state(view_t* view, vh_button_state_t state)

{
  vh_button_t* vh = view->handler_data;
  vh->state       = state;
}

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event)
{
  printf("vh button add %i\n", view->views->length);

  vh_button_t* vh = mem_calloc(sizeof(vh_button_t), "vh_button", NULL, NULL);
  vh->event       = event;
  vh->type        = type;
  vh->state       = VH_BUTTON_UP;

  vh->ratio  = 0.5;
  vh->target = 0.0;

  if (event) RET(event);

  view->handler      = vh_button_evt;
  view->handler_data = vh;
}

#endif
