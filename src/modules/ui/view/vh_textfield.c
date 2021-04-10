#ifndef vh_textfield_h
#define vh_textfield_h

#include "text.c"
#include "view.c"

typedef struct _vh_textfield_t
{
  view_t* ti; // textinput
  view_t* ph; // placeholder
  char    active;
} vh_textfield_t;

void vh_textfield_add(view_t* view, textstyle_t textstyle);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "tg_text.c"
#include "vh_textinput.c"

void vh_textfield_evt(view_t* view, ev_t ev)
{
  vh_textfield_t* data = view->handler_data;
  if (ev.type == EV_MDOWN)
  {
    r2_t frame = view->frame.global;
    if (ev.x >= frame.x &&
        ev.x <= frame.x + frame.w &&
        ev.y >= frame.y &&
        ev.y <= frame.y + frame.h)
    {
      if (!data->active)
      {
        data->active = 1;
        // activate, remove placeholder
        view_remove(view, data->ph);
        view_add(view, data->ti);
      }
    }
    else
    {
      if (data->active)
      {
        data->active = 0;
        // deactivate, readd placeholder
        view_remove(view, data->ti);
        view_add(view, data->ph);
      }
    }
  }
}

void vh_textfield_add(view_t* view, textstyle_t textstyle)
{
  vh_textfield_t* data = mem_calloc(sizeof(vh_textfield_t), "vh_textfield", NULL, NULL);
  data->active         = 0;

  view->handler_data = data;
  view->handler      = vh_textfield_evt;

  // create text input and placeholder views
  view_t* ti = view_new(cstr_fromformat("%stextinput", view->id, NULL), view->frame.local);
  view_t* ph = view_new(cstr_fromformat("%splaceholder", view->id, NULL), view->frame.local);

  tg_text_add(ph);
  vh_textinput_add(ti, "start", "place", textstyle, NULL);

  tg_text_set(ph, "place", textstyle);

  data->ti = ti;
  data->ph = ph;

  view_set_block_touch(ph, 0, 1);

  view_add(view, ti);
  //view_add(view, ph);
}

#endif
