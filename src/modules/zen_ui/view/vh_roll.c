#ifndef vh_roll_h
#define vh_roll_h

#include "view.c"
#include "wm_event.c"
#include "zc_callback.c"

typedef struct _vh_roll_t
{
  char  active;
  cb_t* roll_in;
  cb_t* roll_out;
} vh_roll_t;

void vh_roll_add(view_t* view, cb_t* roll_in, cb_t* roll_out);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_roll_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE)
  {
    vh_roll_t* vh    = view->handler_data;
    r2_t       frame = view->frame.global;

    if (!vh->active)
    {
      if (ev.x >= frame.x &&
          ev.x <= frame.x + frame.w &&
          ev.y >= frame.y &&
          ev.y <= frame.y + frame.h)
      {
        vh->active = 1;
        (*vh->roll_in->fp)(vh->roll_in->userdata, view);
      }
    }
  }
  else if (ev.type == EV_MMOVE_OUT)
  {
    vh_roll_t* vh    = view->handler_data;
    r2_t       frame = view->frame.global;

    if (vh->active)
    {
      if (ev.x < frame.x ||
          ev.x > frame.x + frame.w ||
          ev.y < frame.y ||
          ev.y > frame.y + frame.h)
      {
        vh->active = 0;
        (*vh->roll_out->fp)(vh->roll_out->userdata, view);
      }
    }
  }
}

void vh_roll_add(view_t* view, cb_t* roll_in, cb_t* roll_out)
{
  vh_roll_t* vh = mem_calloc(sizeof(vh_roll_t), "vh_roll", NULL, NULL);
  vh->roll_in   = roll_in;
  vh->roll_out  = roll_out;

  view->handler_data = vh;
  view->handler      = vh_roll_evt;
}

#endif
