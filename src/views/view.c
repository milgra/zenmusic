#ifndef view_h
#define view_h

#include "math4.c"
#include "mtbm.c"

typedef struct _view_t
{
  char*    id;
  bm_t*    bmp;
  char*    type;
  v4_t     frame;
  uint32_t color;
  char     bitmap_state; // 0 - blank , 1 - pending , 2 - ready to render, 3 - added to compositor
  char     dim_changed;
  char     bmp_changed;
} view_t;

view_t* view_new(char* id, v4_t frame, uint32_t color);
void    view_gen(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "mtbm.c"
#include "mtcstr.c"
#include "mtmem.c"
#include "text.c"

void view_del(void* pointer)
{
  view_t* view = (view_t*)pointer;
  REL(view->id);
  REL(view->bmp);
}

view_t* view_new(char* id, v4_t frame, uint32_t color)
{
  view_t* view = mtmem_calloc(sizeof(view_t), view_del);
  view->color  = color;
  view->frame  = frame;
  view->type   = "default";
  view->bmp    = bm_new(frame.z, frame.w);
  view->id     = mtcstr_fromcstring(id);

  return view;
}

void view_gen(view_t* view)
{
  bm_fill(view->bmp,
          0,
          0,
          (int)view->frame.z,
          (int)view->frame.w,
          view->color);
  view->bitmap_state = 2;

  mtstr_t* str = mtstr_frombytes("KUTYAFASZA");

  textstyle_t ts =
      {
          .align      = 0,
          .editable   = 0,
          .selectable = 0,
          .multiline  = 1,
          .autosize   = 1,
          .uppercase  = 0,

          .textsize   = 25.0,
          .marginsize = 10.0,
          .cursorsize = 15.0,

          .textcolor = 0xFFFFFFFF,
          .backcolor = 0x000000FF,
      };

  bm_t* bitmap = font_render_text((int)view->frame.z, (int)view->frame.w, str, common_font, ts, NULL, NULL);
  view->bmp    = bitmap;
}

#endif
