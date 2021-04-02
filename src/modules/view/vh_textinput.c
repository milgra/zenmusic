#ifndef vh_textinput_h
#define vh_textinput_h

#include "mtstring.c"
#include "text.c"
#include "view.c"
#include "wm_event.c"

typedef struct _vh_textinput_t
{
  str_t*  text_s;
  vec_t*  glyph_v;
  view_t* cursor_v;

  int         glyph_index;
  textstyle_t style;
  char        active;
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
#include "mtgraphics.c"
#include "mtvector.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"

void vh_textinput_upd(view_t* view)
{
  vh_textinput_t* data   = view->handler_data;
  str_t*          text_s = data->text_s;
  r2_t            frame  = view->frame.local;

  glyph_t* glyphs = malloc(sizeof(glyph_t) * text_s->length);
  for (int i = 0; i < text_s->length; i++) glyphs[i].cp = text_s->codepoints[i];
  text_layout(glyphs, text_s->length, data->style, frame.w, frame.h);

  for (int i = 0; i < text_s->length; i++)
  {
    glyph_t g  = glyphs[i];
    view_t* gv = data->glyph_v->data[i];

    if (g.w > 0 && g.h > 0)
    {
      r2_t f  = gv->frame.local;
      r2_t nf = (r2_t){g.x, g.y, g.w, g.h};
      if (f.w == 0 || f.h == 0)
      {
        printf("adding texture to glyph\n");
        bm_t* texture = bm_new(g.w, g.h);
        //gfx_rect(texture, 0, 0, g.w, g.h, 0xFF0000FF, 0);
        text_render_glyph(g, data->style, texture);
        view_set_texture_bmp(gv, texture);

        gv->display = 1; // do we have to have 0 as default?!?!
        view_add(view, gv);

        // fade in
        r2_t sf = nf;
        sf.w    = 0.0;
        vh_anim_start(gv, nf, nf, 0.0, 1.0, 30, AT_LINEAR);
      }
      else
      {
        vh_anim_start(gv, gv->frame.local, nf, 1.0, 1.0, 10, AT_LINEAR);
      }
    }
  }

  // update cursor position

  glyph_t last = {0};
  if (text_s->length > 0) last = glyphs[text_s->length - 1];

  r2_t crsr_f = {0};
  crsr_f.x    = last.x + last.w + 1;
  crsr_f.y    = last.base_y - last.asc - last.desc / 2.0;
  crsr_f.w    = 2;
  crsr_f.h    = last.asc;

  vh_anim_start(data->cursor_v, data->cursor_v->frame.local, crsr_f, 1.0, 1.0, 10, AT_LINEAR);

  // view_set_frame(data->cursor_v, crsr_f);

  free(glyphs);

  //textinput_render_glyphs(glyphs, text_s->length, style, bitmap);
  //vh_anim_add(glyphview);
  //vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

  char* cstr = str_cstring(text_s);

  //tg_text_set(view, cstr, data->style);

  REL(cstr);
}

void vh_textinput_activate(view_t* view, char state)
{
  vh_textinput_t* data = view->handler_data;

  if (state)
  {
    if (!data->active)
    {
      data->active = 1;
      view_add(view, data->cursor_v);
    }
  }
  else
  {
    if (data->active)
    {
      data->active = 0;
      view_remove(view, data->cursor_v);
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
    str_addbytearray(data->text_s, ev.text);

    // create view for glyph

    char view_id[100];
    snprintf(view_id, 100, "%sglyph%i", view->id, data->glyph_index++);
    view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0});
    vh_anim_add(glyph_view);

    VADD(data->glyph_v, glyph_view);

    printf("string length %u glyphs length %u\n", data->text_s->length, data->glyph_v->length);

    // append or break-insert new glyph(s)

    vh_textinput_upd(view);

    if (data->on_text) (*data->on_text)(view);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE && data->text_s->length > 0)
    {
      str_removecodepointatindex(data->text_s, data->text_s->length - 1);

      vec_rematindex(data->glyph_v, data->glyph_v->length - 1);

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

  char* id_c = cstr_fromformat("%s%s", view->id, "crsr", NULL);

  vh_textinput_t* data = mem_calloc(sizeof(vh_textinput_t), "vh_text", NULL, NULL);

  textstyle.backcolor = 0;

  data->text_s   = str_new();
  data->glyph_v  = VNEW();
  data->cursor_v = view_new(id_c, (r2_t){50, 12, 2, 0});

  data->style    = textstyle;
  data->userdata = userdata;

  view->needs_key  = 1; // backspace event
  view->needs_text = 1; // unicode text event

  view->handler      = vh_textinput_evt;
  view->handler_data = data;

  // cursor

  data->cursor_v->layout.background_color = 0x666666FF;
  tg_css_add(data->cursor_v);
  vh_anim_add(data->cursor_v);

  // view setup

  tg_text_add(view);

  // text_s setup

  if (text)
  {
    str_addbytearray(data->text_s, text);

    for (int i = 0; i < data->text_s->length; i++)
    {
      str_t* charstr = str_new();
      str_addcodepoint(charstr, data->text_s->codepoints[i]);
      char view_id[100];
      snprintf(view_id, 100, "%sglyph%i", view->id, data->glyph_index++);
      view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0});
      vh_anim_add(glyph_view);

      VADD(data->glyph_v, glyph_view);

      REL(charstr);
    }
  }
  // update text

  vh_textinput_upd(view);

  // cleanup

  REL(id_c);
}

void vh_textinput_set_text(view_t* view, char* text)
{
  vh_textinput_t* data = view->handler_data;

  str_reset(data->text_s);
  str_addbytearray(data->text_s, text);
  vh_textinput_upd(view);
}

str_t* vh_textinput_get_text(view_t* view)
{
  vh_textinput_t* data = view->handler_data;
  return data->text_s;
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
