#ifndef vh_picker_h
#define vh_picker_h

#include "view.c"
#include "wm_event.c"
#include "zc_callback.c"

void vh_picker_add(view_t* view, cb_t* on_press);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_number.c"

typedef struct
{
  cb_t* on_press;
} vh_picker_t;

void vh_picker_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_picker_t* vh = view->handler_data;

    int x = ev.x - view->frame.global.x;
    int y = ev.y - view->frame.global.y;

    uint32_t i = (y * view->texture.bitmap->w + x) * 4;
    uint8_t  r = view->texture.bitmap->data[i];
    uint8_t  g = view->texture.bitmap->data[i + 1];
    uint8_t  b = view->texture.bitmap->data[i + 2];

    num_t* num = num_newuint32((r << 24) | (g << 16) | (b << 8) | 0xFF);
    (*vh->on_press->fp)(vh->on_press->userdata, num);
    REL(num);
  }
}

void vh_picker_add(view_t* view, cb_t* on_press)
{
  vh_picker_t* vh = mem_calloc(sizeof(vh_picker_t), "vh_picker", NULL, NULL);

  vh->on_press = on_press;

  view->handler      = vh_picker_evt;
  view->handler_data = vh;
}

#endif
