#ifndef vh_textinput_h
#define vh_textinput_h

#include "mtstring.c"
#include "text.c"
#include "view.c"
#include "wm_event.c"

typedef struct _vh_textinput_t
{
  str_t*      text;
  vec_t*      glyph_views;
  view_t*     cursor;
  textstyle_t style;
  char        active; // we are under editing
  void*       userdata;

  void (*on_text)(view_t* view);
  void (*on_activate)(view_t* view);
  void (*on_deactivate)(view_t* view);
} vh_textinput_t;

void vh_textinput_add(view_t*     view,
                      char*       text,
                      char*       phtext,
                      textstyle_t textstyle,
                      void*       userdata);

str_t* vh_textinput_get_text(view_t* view);
void   vh_textinput_set_text(view_t* view, char* text);
void   vh_textinput_activate(view_t* view, char state);
void   vh_textinput_set_on_text(view_t* view, void (*event)(view_t*));
void   vh_textinput_set_on_activate(view_t* view, void (*event)(view_t*));
void   vh_textinput_set_on_deactivate(view_t* view, void (*event)(view_t*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"

void vh_textinput_upd(view_t* view)
{
  vh_textinput_t* data  = view->handler_data;
  str_t*          text  = data->text;
  r2_t            frame = view->frame.local;

  if (data->active)
  {
    data->style.textcolor = 0x000000FF;
    // update text
    // update cursor
  }

  char empty = text->length == 0;
  if (empty) str_addbytearray(text, " ");

  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length);
  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];

  text_layout(glyphs, text->length, data->style, frame.w, frame.h);

  // update cursor position

  glyph_t last = {0};
  if (text->length > 0) last = glyphs[text->length - 1];

  r2_t crsr_f = {0};
  crsr_f.x    = last.x + last.w + 1;
  crsr_f.y    = last.base_y - last.asc - last.desc / 2.0;
  crsr_f.w    = 2;
  crsr_f.h    = last.asc;

  vh_anim_set(data->cursor, data->cursor->frame.local, crsr_f, 10, AT_LINEAR);

  // view_set_frame(data->cursor, crsr_f);

  free(glyphs);

  //textinput_render_glyphs(glyphs, text->length, style, bitmap);
  //vh_anim_add(glyphview);
  //vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

  char* cstr = str_cstring(text);

  tg_text_set(view, cstr, data->style);

  REL(cstr);

  if (empty) str_removecodepointatindex(text, 0);
}

void vh_textinput_activate(view_t* view, char state)
{
  vh_textinput_t* data = view->handler_data;

  if (state)
  {
    if (!data->active)
    {
      data->active = 1;
      view_add(view, data->cursor);
    }
  }
  else
  {
    if (data->active)
    {
      data->active = 0;
      view_remove(view, data->cursor);
    }
  }

  vh_textinput_upd(view);
}

void vh_textinput_evt(view_t* view, ev_t ev)
{
  vh_textinput_t* data = view->handler_data;
  if (ev.type == EV_TIME)
  {
    if (data->active)
    {
    }
  }
  else if (ev.type == EV_MDOWN)
  {
    vh_textinput_activate(view, 1);
    if (data->on_activate) (*data->on_activate)(view);
    r2_t frame = view->frame.global;

    if (ev.x >= frame.x &&
        ev.x <= frame.x + frame.w &&
        ev.y >= frame.y &&
        ev.y <= frame.y + frame.h &&
        data->on_deactivate) (*data->on_deactivate)(view);
  }
  else if (ev.type == EV_TEXT)
  {
    str_addbytearray(data->text, ev.text);

    vh_textinput_upd(view);

    if (data->on_text) (*data->on_text)(view);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE && data->text->length > 0)
    {
      str_removecodepointatindex(data->text, data->text->length - 1);
      vh_textinput_upd(view);
      if (data->on_text) (*data->on_text)(view);
    }
  }
  else if (ev.type == EV_TIME)
  {
    // animate glyphs
  }
}

void vh_textinput_add(view_t*     view,
                      char*       text,
                      char*       phtext,
                      textstyle_t textstyle,
                      void*       userdata)
{
  printf("vh_textinput_add %s %s\n", view->id, phtext);

  vh_textinput_t* data = mem_calloc(sizeof(vh_textinput_t), "vh_text", NULL, NULL);
  data->text           = str_new();
  data->style          = textstyle;
  data->userdata       = userdata;

  if (text) str_addbytearray(data->text, text);

  view->needs_key    = 1;
  view->needs_text   = 1;
  view->handler      = vh_textinput_evt;
  view->handler_data = data;

  // add default/first paragraph

  r2_t frame = view->frame.local;
  frame.x    = 0;
  frame.y    = 0;

  tg_text_add(view);

  // add cursor

  char*   crsrid = cstr_fromformat("%s%s", view->id, "crsr", NULL);
  view_t* cursor = view_new(crsrid, (r2_t){50, 12, 2, 0});
  REL(crsrid);

  cursor->layout.background_color = 0x666666FF;
  tg_css_add(cursor);
  vh_anim_add(cursor);

  data->cursor = cursor;

  // udpate

  vh_textinput_upd(view);
}

void vh_textinput_set_text(view_t* view, char* text)
{
  vh_textinput_t* data = view->handler_data;

  str_reset(data->text);
  str_addbytearray(data->text, text);
  vh_textinput_upd(view);
}

str_t* vh_textinput_get_text(view_t* view)
{
  vh_textinput_t* data = view->handler_data;
  return data->text;
}

void vh_textinput_set_on_text(view_t* view, void (*event)(view_t*))
{
  vh_textinput_t* data = view->handler_data;
  data->on_text        = event;
}

void vh_textinput_set_on_activate(view_t* view, void (*event)(view_t*))
{
  vh_textinput_t* data = view->handler_data;
  data->on_activate    = event;
}

void vh_textinput_set_on_deactivate(view_t* view, void (*event)(view_t*))
{
  vh_textinput_t* data = view->handler_data;
  data->on_deactivate  = event;
}

#endif
