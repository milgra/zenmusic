#ifndef vhand_text_h
#define vh_text_h

#include "mtstring.c"
#include "text.c"
#include "view.c"
#include "wm_event.c"

typedef struct _vh_text_t
{
  str_t*      text;
  textstyle_t style;
  char        editing;

  r2_t     crsr_f;
  view_t*  crsr_v;
  uint32_t crsr_i;

  view_t* pgraph;
  void (*on_text)(view_t* view);
  void (*on_activate)(view_t* view);
  void (*on_deactivate)(view_t* view);
} vh_text_t;

void vh_text_add(view_t*     view,
                 char*       text,
                 textstyle_t textstyle);
void vh_text_activate(view_t* view, char state);
void vh_text_set_on_text(view_t* view, void (*event)(view_t*));
void vh_text_set_on_deactivate(view_t* view, void (*event)(view_t*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"

void vh_text_upd(view_t* view)
{
  vh_text_t* data  = view->handler_data;
  str_t*     text  = data->text;
  r2_t       frame = view->frame.local;

  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length);
  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];

  text_layout(glyphs, text->length, data->style, frame.w, frame.h);

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

  free(glyphs);

  //text_render_glyphs(glyphs, text->length, style, bitmap);
  //vh_anim_add(glyphview);
  //vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

  char* cstr = str_cstring(data->text);

  tg_text_set(data->pgraph, cstr, data->style);

  REL(cstr);
}

void vh_text_activate(view_t* view, char state)
{
  vh_text_t* data = view->handler_data;

  if (state)
  {
    if (!data->editing)
    {
      data->editing = 1;

      r2_t sf = data->crsr_f;
      r2_t ef = sf;
      sf.h    = 0;
      sf.y    = ef.y + ef.h / 2.0;

      vh_anim_set(data->crsr_v, sf, ef, 10, AT_LINEAR);
    }
  }
  else
  {
    if (data->editing)
    {
      data->editing = 0;

      r2_t sf = data->crsr_f;
      r2_t ef = sf;
      ef.h    = 0;
      ef.y    = sf.y + sf.h / 2.0;

      vh_anim_set(data->crsr_v, sf, ef, 10, AT_LINEAR);
    }
  }
}

void vh_text_evt(view_t* view, ev_t ev)
{
  vh_text_t* data = view->handler_data;
  if (ev.type == EV_MDOWN)
  {
    vh_text_activate(view, 1);
    if (data->on_activate) (*data->on_activate)(view);
  }
  else if (ev.type == EV_MDOWN_OUTSIDE)
  {
    vh_text_activate(view, 0);
    if (data->on_deactivate) (*data->on_deactivate)(view);
  }
  else if (ev.type == EV_TEXT)
  {
    str_addbytearray(data->text, ev.text);

    vh_text_upd(view);

    if (data->on_text) (*data->on_text)(view);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE && data->text->length > 0)
    {
      str_removecodepointatindex(data->text, data->text->length - 1);
      vh_text_upd(view);
      if (data->on_text) (*data->on_text)(view);
    }
  }
  else if (ev.type == EV_TIME)
  {
    // animate glyphs
  }
}

void vh_text_add(view_t*     view,
                 char*       text,
                 textstyle_t textstyle)
{
  printf("vh_text_add %s\n", view->id);

  vh_text_t* data = mem_calloc(sizeof(vh_text_t), "vh_text", NULL, NULL);
  data->text      = str_new();
  data->style     = textstyle;
  data->crsr_i    = 0; // cursor index

  str_addbytearray(data->text, text);

  view->needs_key    = 1;
  view->needs_text   = 1;
  view->handler      = vh_text_evt;
  view->handler_data = data;

  // add default/first paragraph

  r2_t frame = view->frame.local;
  frame.x    = 0;
  frame.y    = 0;

  char*   pgraphid = cstr_fromformat("%s%s", view->id, "pgraph");
  view_t* pgraph   = view_new(pgraphid, frame);
  REL(pgraphid);

  pgraph->layout.w_per = 1.0;
  pgraph->layout.h_per = 1.0;
  pgraph->needs_touch  = 0;

  tg_text_add(pgraph);
  tg_text_set(pgraph, text, textstyle);

  view_add(view, pgraph);

  data->pgraph = pgraph;

  // add cursor

  char*   crsrid = cstr_fromformat("%s%s", view->id, "crsr");
  view_t* crsr_v = view_new(crsrid, (r2_t){50, 12, 2, 0});
  REL(crsrid);

  crsr_v->layout.background_color = 0x666666FF;
  tg_css_add(crsr_v);
  vh_anim_add(crsr_v);
  view_add(view, crsr_v);

  data->crsr_v = crsr_v;

  // udpate

  vh_text_upd(view);
}

void vh_text_set_on_text(view_t* view, void (*event)(view_t*))
{
  vh_text_t* data = view->handler_data;
  data->on_text   = event;
}

void vh_text_set_on_deactivate(view_t* view, void (*event)(view_t*))
{
  vh_text_t* data     = view->handler_data;
  data->on_deactivate = event;
}

#endif
