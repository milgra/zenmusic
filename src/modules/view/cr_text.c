#ifndef cellrenderer_text_h
#define cellrenderer_text_h

#include "text.c"
#include "tg_text.c"
#include "view.c"

typedef struct _cr_text_data_t
{
  textstyle_t style;
  char*       text;
} cr_text_data_t;

void cr_text_upd(view_t* view, void* p);

#endif

#if __INCLUDE_LEVEL__ == 0

void cr_text_upd(view_t* view, void* p)
{
  cr_text_data_t data = *(cr_text_data_t*)p;

  if (view->tex_gen_data == NULL)
  {
    // TODO tg_text_add shouldn't have initial text, _set should have textstyle
    tg_text_add(view, data.text, data.style);
  }

  tg_text_set(view, data.text);
}

#endif
