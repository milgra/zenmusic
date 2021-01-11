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

void cr_text_add(view_t* view);
void cr_text_upd(view_t* view, void* p);

#endif

#if __INCLUDE_LEVEL__ == 0

void cr_text_add(view_t* view)
{
  tg_text_add(view);
}

void cr_text_upd(view_t* view, void* p)
{
  cr_text_data_t data = *(cr_text_data_t*)p;

  if (view->tex_gen_data == NULL) tg_text_add(view);

  tg_text_set(view, data.text, data.style);
}

#endif
