#ifndef evthan_text_h
#define evthan_text_h

#include "view.c"
#include "wm_event.c"

void eh_text_add(view_t* view, char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_text.c"

typedef struct _eh_text_t
{
  char*    text;
  mtvec_t* paragraphs; // a paragraph contains multiple glyph views or one big pre-rendered paragraph view
  mtvec_t* glyphs;
  mtvec_t* animators;
} eh_text_t;

void eh_text_evt(view_t* view, ev_t ev)
{
  eh_text_t* data = view->evt_han_data;
  if (ev.type == EV_TEXT)
  {
    printf("text %s\n", ev.text);
    // add new glyph view/update paragraph view

    char idbuffer[100] = {0};
    snprintf(idbuffer, 100, "glyphview %s", ev.text);

    view_t* glyphview = view_new(idbuffer, (r2_t){0, 0, 20, 20});
    tg_text_add(glyphview, 0xFEFEFEFF, 0x000000FF, ev.text, 0);
    view_add(view, glyphview);

    VADD(data->glyphs, glyphview);

    view_t* gview;
    float   pos = 0;

    while ((gview = VNXT(data->glyphs)))
    {
      r2_t frame = gview->frame.local;
      frame.x    = pos;
      pos += frame.w;
      view_set_frame(gview, frame);
    }
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE)
    {
      view_t* last = mtvec_tail(data->glyphs);
      view_remove(view, last);
      VREM(data->glyphs, last);
    }
  }
  else if (ev.type == EV_TIME)
  {
    // animate glyphs
  }
}

void eh_text_add(view_t* view, char* text)
{
  eh_text_t* data = mtmem_calloc(sizeof(eh_text_t), "eh_text", NULL, NULL);
  data->text      = mtcstr_fromcstring(text);
  data->glyphs    = mtvec_alloc();

  view->evt_han       = eh_text_evt;
  view->evt_han_data  = data;
  view->texture.state = TS_BLANK;
}

#endif
