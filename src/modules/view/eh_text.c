#ifndef evthan_text_h
#define evthan_text_h

#include "mtstring.c"
#include "view.c"
#include "wm_event.c"

void eh_text_add(view_t* view, char* text, void (*ontext)(view_t* view, str_t* text));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "eh_anim.c"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_css.c"
#include "tg_text.c"

typedef struct _eh_text_t
{
  str_t*  text;
  vec_t*  paragraphs; // a paragraph contains multiple glyph views or one big pre-rendered paragraph view
  vec_t*  glyphs;
  vec_t*  animators;
  char    editing;
  view_t* crsr;
  void (*ontext)(view_t* view, str_t* text);
} eh_text_t;

void eh_text_evt(view_t* view, ev_t ev)
{
  eh_text_t* data = view->evt_han_data;
  if (ev.type == EV_MMOVE)
  {
    // activate text field, add cursor
    if (ev.x < view->frame.global.x + view->frame.global.w &&
        ev.x > view->frame.global.x &&
        ev.y < view->frame.global.y + view->frame.global.h &&
        ev.y > view->frame.global.y)
    {
      if (!data->editing)
      {
        data->editing = 1;

        r2_t sf = data->crsr->frame.local;
        r2_t ef = sf;
        ef.y    = 8.0;
        ef.h    = 20.0;

        eh_anim_set(data->crsr, sf, ef, 10, AT_LINEAR);
      }
    }
    else
    {
      data->editing = 0;

      r2_t sf = data->crsr->frame.local;
      r2_t ef = sf;
      ef.y    = 18.0;
      ef.h    = 0.0;

      eh_anim_set(data->crsr, sf, ef, 10, AT_LINEAR);
    }
  }
  else if (ev.type == EV_TEXT)
  {

    str_addbytearray(data->text, ev.text);
    printf("text %s\n", str_cstring(data->text));

    // add new glyph view/update paragraph view

    char idbuffer[100] = {0};
    snprintf(idbuffer, 100, "glyphview %s", ev.text);

    textstyle_t ts = {0};
    ts.align       = 0;
    ts.size        = 15.0;
    ts.textcolor   = 0xFFFFFFFF;
    ts.backcolor   = 0x00000022;

    // TODO get glyph width first
    view_t* glyphview = view_new(idbuffer, (r2_t){0, 0, 20, 20});
    tg_text_add(glyphview, ev.text, ts);

    view_add(view, glyphview);

    VADD(data->glyphs, glyphview);

    view_t* gview;
    float   pos = 0;

    while ((gview = VNXT(data->glyphs)))
    {
      r2_t frame = gview->frame.local;
      pos += frame.w;
    }

    r2_t sf = (r2_t){pos, 0, 1, 20};

    view_set_frame(glyphview, sf);

    r2_t ef = sf;
    ef.x += 10.0;
    ef.w = 20.0;

    eh_anim_add(glyphview);
    eh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

    (*data->ontext)(view, data->text);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE)
    {
      view_t* last = vec_tail(data->glyphs);
      view_remove(view, last);
      VREM(data->glyphs, last);
    }
  }
  else if (ev.type == EV_TIME)
  {
    // animate glyphs
  }
}

void eh_text_add(view_t* view, char* text, void (*ontext)(view_t* view, str_t* text))
{
  eh_text_t* data = mem_calloc(sizeof(eh_text_t), "eh_text", NULL, NULL);
  data->text      = str_new();
  data->glyphs    = vec_alloc();
  data->ontext    = ontext;

  str_addbytearray(data->text, text);

  view->needs_key     = 1;
  view->needs_text    = 1;
  view->evt_han       = eh_text_evt;
  view->evt_han_data  = data;
  view->texture.state = TS_BLANK;

  // add cursor

  view_t* crsr = view_new("crsr", (r2_t){50, 12, 2, 0});

  crsr->layout.background_color = 0x000000FF;
  tg_css_add(crsr);
  eh_anim_add(crsr);
  view_add(view, crsr);

  data->crsr = crsr;

  // generate text
  for (int index = 0; index < data->text->length; index++)
  {
    uint32_t cp = data->text->codepoints[index];

    str_t* str = str_new();
    str_addcodepoint(str, cp);

    char idbuffer[100] = {0};
    snprintf(idbuffer, 100, "glyphview %s", str_cstring(str));

    textstyle_t ts = {0};
    ts.align       = 0;
    ts.size        = 15.0;
    ts.textcolor   = 0xFFFFFFFF;
    ts.backcolor   = 0x00000022;

    // TODO get glyph width first
    view_t* glyphview = view_new(idbuffer, (r2_t){0, 0, 20, 20});
    tg_text_add(glyphview, str_cstring(str), ts);

    view_add(view, glyphview);

    VADD(data->glyphs, glyphview);
  }

  // arrange
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

#endif
