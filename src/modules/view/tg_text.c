/*
  Text texture generator
  Shows text in view
 */

#ifndef texgen_text_h
#define texgen_text_h

#include "text.c"
#include "view.c"

typedef struct _tg_text_t
{
  char*       text;
  textstyle_t style;
} tg_text_t;

void tg_text_add(view_t* view);
void tg_text_set(view_t* view, char* text, textstyle_t style);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtbitmap.c"
#include "mtcstring.c"
#include "mtstring.c"

int tg_text_index = 0;

void tg_text_gen(view_t* view)
{
  tg_text_t* gen = view->tex_gen_data;
  if (view->frame.local.w > 0 && view->frame.local.h > 0 && gen->text)
  {

    str_t* str = str_new();
    str_addbytearray(str, gen->text);

    bm_t* fontmap = bm_new((int)view->frame.local.w, (int)view->frame.local.h);

    text_render(
        str,
        gen->style,
        fontmap);

    view_set_texture_bmp(view, fontmap);
  }
}

void tg_text_add(view_t* view)
{
  tg_text_t* gen = mem_calloc(sizeof(tg_text_t), "tg_text_t", NULL, NULL);

  view->tex_gen_data = gen;
  view->tex_gen      = tg_text_gen;
  view->display      = 1;
}

void tg_text_set(view_t* view, char* text, textstyle_t style)
{
  tg_text_t* gen = view->tex_gen_data;

  if (gen->text) REL(gen->text);
  gen->text           = cstr_fromcstring(text);
  gen->style          = style;
  view->texture.state = TS_BLANK;
}

#endif
