#ifndef view_h
#define view_h

#include "event.c"
#include "math2.c"
#include "math4.c"
#include "mtbm.c"
#include "mtvec.c"

typedef struct _view_t
{
  char*    id;    // identifier for handling view
  void*    data;  // data for event handler and bitmap generator
  mtvec_t* views; // subviews

  v4_t frame;       // position and dimensions
  char dim_changed; // frame changed

  bm_t* bmp;          // bitmap of view
  char  bmp_changed;  // bitmap changed
  char  bitmap_state; // 0 - blank , 1 - pending , 2 - ready to render, 3 - added to compositor

  void (*evt)(struct _view_t*, ev_t); // event handler for view
  void (*gen)(struct _view_t*);       // bitmap generator for view

} view_t;

view_t* view_new(char* id, v4_t frame);
void    view_gen(view_t* view);
void    view_setpos(view_t* view, v2_t pos);
void    view_setdim(view_t* view, v2_t dim);
void    view_setbmp(view_t* view, bm_t* bmp);

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

view_t* view_new(char* id, v4_t frame)
{
  view_t* view = mtmem_calloc(sizeof(view_t), view_del);
  view->id     = mtcstr_fromcstring(id);
  view->bmp    = bm_new(frame.z, frame.w);
  view->views  = VNEW();
  view->frame  = frame;

  return view;
}

void view_gen(view_t* view)
{
  bm_fill(view->bmp,
          0,
          0,
          (int)view->frame.z,
          (int)view->frame.w,
          0xFF0000FF);
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

void view_setpos(view_t* view, v2_t pos)
{
  view->frame.x     = pos.x;
  view->frame.y     = pos.y;
  view->dim_changed = 1;
}

void view_setdim(view_t* view, v2_t dim)
{
  view->frame.z     = dim.x;
  view->frame.w     = dim.y;
  view->dim_changed = 1;
}

#endif
