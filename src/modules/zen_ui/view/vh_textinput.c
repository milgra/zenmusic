#ifndef vh_textinput_h
#define vh_textinput_h

#include "text.c"
#include "view.c"
#include "wm_event.c"
#include "zc_string.c"

typedef struct _vh_textinput_t
{
  str_t*  text_s;   // text string
  vec_t*  glyph_v;  // glpyh views
  view_t* cursor_v; // cursor view
  view_t* holder_v; // placeholder text view

  int         glyph_index;
  textstyle_t style;
  char        active;
  void*       userdata;

  void (*on_text)(view_t* view);
  void (*on_return)(view_t* view);
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
void   vh_textinput_set_on_return(view_t* view, void (*event)(view_t*));
void   vh_textinput_set_on_activate(view_t* view, void (*event)(view_t*));
void   vh_textinput_set_on_deactivate(view_t* view, void (*event)(view_t*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "SDL.h"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"
#include "zc_cstring.c"
#include "zc_graphics.c"
#include "zc_vector.c"

void vh_textinput_upd(view_t* view)
{
  vh_textinput_t* data   = view->handler_data;
  str_t*          text_s = data->text_s;
  r2_t            frame  = view->frame.local;

  if (text_s->length > 0)
  {
    glyph_t* glyphs = malloc(sizeof(glyph_t) * text_s->length);
    for (int i = 0; i < text_s->length; i++) glyphs[i].cp = text_s->codepoints[i];
    text_layout(glyphs, text_s->length, data->style, frame.w, frame.h);

    for (int i = 0; i < text_s->length; i++)
    {
      glyph_t g = glyphs[i];

      if (i < data->glyph_v->length)
      {

        view_t* gv = data->glyph_v->data[i];

        if (g.w > 0 && g.h > 0)
        {
          r2_t f  = gv->frame.local;
          r2_t nf = (r2_t){g.x, g.y, g.w, g.h};
          if (f.w == 0 || f.h == 0)
          {
            bm_t* texture = bm_new(g.w, g.h);

            text_render_glyph(g, data->style, texture);

            view_set_texture_bmp(gv, texture);

            gv->exclude = 0; // do we have to have 0 as default?!?!

            view_add(view, gv);

            view_set_frame(gv, nf);

            // open
            r2_t sf = nf;
            sf.x    = 0.0;
            sf.y    = 0.0;
            nf.x    = 0.0;
            nf.y    = 0.0;
            sf.w    = 0.0;

            vh_anim_region(gv, sf, nf, 10, AT_EASE);

            view_set_region(gv, sf);
          }
          else
          {
            r2_t rf = nf;
            rf.x    = 0;
            rf.y    = 0;
            view_set_region(gv, rf);
            vh_anim_finish(gv);
            vh_anim_frame(gv, gv->frame.local, nf, 10, AT_EASE);
          }
        }
      }
      else
        printf("glyph and string count mismatch\n");
    }

    // update cursor position

    glyph_t last = {0};
    if (text_s->length > 0) last = glyphs[text_s->length - 1];

    r2_t crsr_f = {0};
    crsr_f.x    = last.x + last.w + 1;
    crsr_f.y    = last.base_y - last.asc - last.desc / 2.0;
    crsr_f.w    = 2;
    crsr_f.h    = last.asc;

    vh_anim_finish(data->cursor_v);
    vh_anim_frame(data->cursor_v, data->cursor_v->frame.local, crsr_f, 10, AT_EASE);

    // view_set_frame(data->cursor_v, crsr_f);

    free(glyphs);
  }
  else
  {
    // move cursor to starting position based on textstyle

    // get line height
    glyph_t glyph;
    glyph.cp = ' ';
    text_layout(&glyph, 1, data->style, frame.w, frame.h);

    r2_t crsr_f = {0};
    crsr_f.w    = 2;
    crsr_f.h    = glyph.asc;

    if (data->style.align == TA_LEFT)
    {
      crsr_f.x = data->style.margin;
      crsr_f.y = data->style.margin;
      if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
      if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
    }
    if (data->style.align == TA_RIGHT)
    {
      crsr_f.x = frame.w - data->style.margin_right - crsr_f.w;
      crsr_f.y = data->style.margin_top;
      if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
      if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
    }
    if (data->style.align == TA_CENTER)
    {
      crsr_f.x = frame.w / 2 - crsr_f.w / 2;
      crsr_f.y = data->style.margin_top;
      if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
      if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
    }

    view_set_frame(data->cursor_v, crsr_f);
  }

  //textinput_render_glyphs(glyphs, text_s->length, style, bitmap);
  //vh_anim_add(glyphview);
  //vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

  // show text as texture
  //char* cstr = str_cstring(text_s);
  //tg_text_set(view, cstr, data->style);
  //REL(cstr);
}

void vh_textinput_activate(view_t* view, char state)
{
  assert(view && view->handler_data != NULL && strcmp(mem_type(view->handler_data), "vh_text") == 0);

  vh_textinput_t* data = view->handler_data;

  if (state)
  {
    if (!data->active)
    {
      data->active = 1;

      if (data->text_s->length == 0)
      {
        vh_anim_alpha(data->holder_v, 1.0, 0.0, 10, AT_LINEAR);
      }
      vh_anim_alpha(data->cursor_v, 0.0, 1.0, 10, AT_LINEAR);
    }
  }
  else
  {
    if (data->active)
    {
      data->active = 0;

      if (data->text_s->length == 0)
      {
        vh_anim_alpha(data->holder_v, 0.0, 1.0, 10, AT_LINEAR);
      }
      vh_anim_alpha(data->cursor_v, 1.0, 0.0, 10, AT_LINEAR);
    }
  }

  vh_textinput_upd(view);
}

void vh_textinput_on_glyph_close(view_t* view, void* userdata)
{
  view_t* textview = userdata;
  view_remove(textview, view);
}

void vh_textinput_evt(view_t* view, ev_t ev)
{
  vh_textinput_t* data = view->handler_data;
  if (ev.type == EV_TIME)
  {
  }
  else if (ev.type == EV_MDOWN)
  {
    r2_t frame = view->frame.global;

    vh_textinput_activate(view, 1);
    if (data->on_activate) (*data->on_activate)(view);
  }
  else if (ev.type == EV_MDOWN_OUT)
  {
    r2_t frame = view->frame.global;

    if (ev.x < frame.x ||
        ev.x > frame.x + frame.w ||
        ev.y < frame.y ||
        ev.y > frame.y + frame.h)
    {
      vh_textinput_activate(view, 0);
      if (data->on_deactivate) (*data->on_deactivate)(view);
    }
  }
  else if (ev.type == EV_TEXT)
  {
    str_addbytearray(data->text_s, ev.text);

    // create view for glyph

    char view_id[100];
    snprintf(view_id, 100, "%sglyph%i", view->id, data->glyph_index++);
    view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0});
    vh_anim_add(glyph_view);
    glyph_view->texture.resizable = 0;

    VADD(data->glyph_v, glyph_view);

    // append or break-insert new glyph(s)

    vh_textinput_upd(view);

    if (data->on_text) (*data->on_text)(view);
  }
  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_BACKSPACE && data->text_s->length > 0)
    {
      str_removecodepointatindex(data->text_s, data->text_s->length - 1);

      view_t* glyph_view = vec_tail(data->glyph_v);
      VREM(data->glyph_v, glyph_view);

      r2_t sf = glyph_view->frame.local;
      r2_t ef = sf;
      sf.x    = 0.0;
      sf.y    = 0.0;
      ef.x    = 0.0;
      ef.y    = 0.0;
      ef.w    = 0.0;

      vh_anim_region(glyph_view, sf, ef, 10, AT_EASE);
      vh_anim_set_event(glyph_view, view, vh_textinput_on_glyph_close);

      vh_textinput_upd(view);
      if (data->on_text) (*data->on_text)(view);
    }
    if (ev.keycode == SDLK_RETURN)
    {
      if (data->on_return) (*data->on_return)(view);
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
  char* id_c = cstr_fromformat(100, "%s%s", view->id, "crsr");
  char* id_h = cstr_fromformat(100, "%s%s", view->id, "holder");

  vh_textinput_t* data = mem_calloc(sizeof(vh_textinput_t), "vh_text", NULL, NULL);

  textstyle.backcolor = 0;

  data->text_s  = str_new();
  data->glyph_v = VNEW();

  data->style    = textstyle;
  data->userdata = userdata;

  view->needs_key  = 1; // backspace event
  view->needs_text = 1; // unicode text event

  view->handler      = vh_textinput_evt;
  view->handler_data = data;

  // cursor

  data->cursor_v                          = view_new(id_c, (r2_t){50, 12, 2, 0});
  data->cursor_v->layout.background_color = 0x666666FF;

  tg_css_add(data->cursor_v);
  vh_anim_add(data->cursor_v);

  view_set_texture_alpha(data->cursor_v, 0.0, 1);
  view_add(view, data->cursor_v);

  // placeholder

  textstyle_t phts = textstyle;
  phts.align       = TA_CENTER;
  phts.textcolor   = 0x888888FF;
  data->holder_v   = view_new(id_h, (r2_t){0, 0, view->frame.local.w, view->frame.local.h});
  tg_text_add(data->holder_v);
  tg_text_set(data->holder_v, phtext, phts);
  vh_anim_add(data->holder_v);

  data->holder_v->blocks_touch = 0;

  view_add(view, data->holder_v);

  // view setup

  tg_text_add(view);

  // add placeholder view

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
  REL(id_h);
}

void vh_textinput_set_text(view_t* view, char* text)
{
  vh_textinput_t* data = view->handler_data;

  str_reset(data->text_s);

  // remove glyphs

  for (int i = 0; i < data->glyph_v->length; i++)
  {
    view_t* gv = data->glyph_v->data[i];
    view_remove(view, gv);
  }
  vec_reset(data->glyph_v);

  // text_s setup
  // TODO create function from this to reuse

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

  vh_textinput_upd(view);

  if (data->on_text) (*data->on_text)(view);
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

void vh_textinput_set_on_return(view_t* view, void (*event)(view_t*))
{
  vh_textinput_t* data = view->handler_data;
  data->on_return      = event;
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
