/*
  Text texture generator
  Shows text in view
 */

#ifndef texgen_text_h
#define texgen_text_h

#include "view.c"

typedef struct _tg_text_t
{
  char*    text;
  uint32_t fc;
  uint32_t bc;

} tg_text_t;

void tg_text_add(view_t* view, uint32_t bc, uint32_t fc, char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "font.c"
#include "mtbitmap.c"
#include "mtcstring.c"
#include "mtstring.c"

int tg_text_index = 0;

void tg_text_gen(view_t* view)
{
  tg_text_t* gen = view->tex_gen_data;

  mtstr_t* str = mtstr_frombytes(gen->text);

  textstyle_t ts =
      {
          .align      = 0,
          .editable   = 0,
          .selectable = 0,
          .multiline  = 0,
          .autosize   = 0,
          .uppercase  = 0,

          .textsize   = 20.0,
          .marginsize = 5.0,
          .cursorsize = 15.0,

          .textcolor = gen->fc,
          .backcolor = gen->bc,
      };

  char idbuffer[100] = {0};
  snprintf(idbuffer, 20, "text %i", tg_text_index++);

  bm_t* bmp = font_render_text((int)view->frame.local.w, (int)view->frame.local.h, str, common_font, ts, NULL, NULL);
  view_set_texture(view, bmp, idbuffer);
}

void tg_text_add(view_t* view, uint32_t bc, uint32_t fc, char* text)
{
  tg_text_t* gen = mtmem_alloc(sizeof(tg_text_t), "tg_text_t", NULL, NULL);

  gen->fc   = fc;
  gen->bc   = bc;
  gen->text = mtcstr_fromcstring(text);

  view->texture.state = TS_BLANK;
  view->tex_gen_data  = gen;
  view->tex_gen       = tg_text_gen;
}

#endif
