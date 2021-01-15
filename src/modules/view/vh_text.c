#ifndef vhand_text_h
#define vh_text_h

#include "mtstring.c"
#include "text.c"
#include "view.c"
#include "wm_event.c"

void vh_text_add(view_t*     view,
                 char*       text,
                 textstyle_t textstyle,
                 void (*ontext)(view_t* view, str_t* text),
                 void (*onactivate)(view_t* view));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"

typedef struct _vh_text_t
{
  str_t*      text;
  textstyle_t style;
  char        editing;

  r2_t     crsr_f;
  view_t*  crsr_v;
  uint32_t crsr_i;

  view_t* para;
  void (*ontext)(view_t* view, str_t* text);
  void (*onactivate)(view_t* view);
} vh_text_t;

void vh_text_upd(view_t* view)
{
  vh_text_t* data  = view->handler_data;
  str_t*     text  = data->text;
  r2_t       frame = view->frame.local;

  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length);

  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];

  text_break_glyphs(glyphs, text->length, data->style, frame.w, frame.h);
  text_align_glyphs(glyphs, text->length, data->style, frame.w, frame.h);

  // update cursor position

  glyph_t last = {0};
  if (text->length > 0) last = glyphs[text->length - 1];

  r2_t crsr_f = {0};
  crsr_f.x    = last.x + last.w + 1;
  crsr_f.y    = last.base_y - last.asc;
  crsr_f.w    = 2;
  crsr_f.h    = last.asc - last.desc;

  data->crsr_f = crsr_f;
  view_set_frame(data->crsr_v, crsr_f);

  //text_render_glyphs(glyphs, text->length, style, bitmap);
  //vh_anim_add(glyphview);
  //vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

  tg_text_set(data->para, str_cstring(data->text), data->style);
}

void vh_text_evt(view_t* view, ev_t ev)
{
  vh_text_t* data = view->handler_data;
  if (ev.type == EV_MDOWN)
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

        r2_t sf = data->crsr_f;
        r2_t ef = sf;
        sf.h    = 0;
        sf.y    = ef.y + ef.h / 2.0;

        vh_anim_set(data->crsr_v, sf, ef, 10, AT_LINEAR);

        if (data->onactivate) (*data->onactivate)(view);
      }
    }
    else
    {
      data->editing = 0;

      r2_t sf = data->crsr_f;
      r2_t ef = sf;
      ef.h    = 0;
      ef.y    = sf.y + sf.h / 2.0;

      vh_anim_set(data->crsr_v, sf, ef, 10, AT_LINEAR);
    }
  }
  else if (ev.type == EV_TEXT)
  {
    str_addbytearray(data->text, ev.text);

    vh_text_upd(view);

    (*data->ontext)(view, data->text);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE && data->text->length > 0)
    {
      str_removecodepointatindex(data->text, data->text->length - 1);
      vh_text_upd(view);
      (*data->ontext)(view, data->text);
    }
  }
  else if (ev.type == EV_TIME)
  {
    // animate glyphs
  }
}

void vh_text_add(view_t*     view,
                 char*       text,
                 textstyle_t textstyle,
                 void (*ontext)(view_t* view, str_t* text),
                 void (*onactivate)(view_t* view))
{
  vh_text_t* data  = mem_calloc(sizeof(vh_text_t), "vh_text", NULL, NULL);
  data->text       = str_new();
  data->ontext     = ontext;
  data->onactivate = onactivate;
  data->style      = textstyle;
  data->crsr_i     = 0; // cursor index

  str_addbytearray(data->text, text);

  view->needs_key    = 1;
  view->needs_text   = 1;
  view->handler      = vh_text_evt;
  view->handler_data = data;

  // add default/first paragraph

  r2_t frame = view->frame.local;
  frame.x    = 0;
  frame.y    = 0;

  view_t* para = view_new("para", frame);

  para->layout.w_per = 1.0;
  para->layout.h_per = 1.0;
  para->needs_touch  = 0;

  tg_text_add(para);
  tg_text_set(para, text, textstyle);

  view_add(view, para);

  data->para = para;

  // add cursor

  view_t* crsr_v = view_new("crsr_v", (r2_t){50, 12, 2, 0});

  crsr_v->layout.background_color = 0x666666FF;
  tg_css_add(crsr_v);
  vh_anim_add(crsr_v);
  view_add(view, crsr_v);

  data->crsr_v = crsr_v;

  // udpate

  vh_text_upd(view);
}

#endif
