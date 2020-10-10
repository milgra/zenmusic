#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtmath2.c"
#include "mtmath4.c"
#include "mtvector.c"
#include "wm_event.c"

typedef struct _view_t
{
  char*    id;    // identifier for handling view
  void*    data;  // data for event handler and bitmap generator
  mtvec_t* views; // subviews

  v4_t frame;         // position and dimensions
  char frame_changed; // frame changed

  bm_t* bmp;         // bitmap of view
  char  bmp_changed; // bitmap changed
  char  bmp_state;   // 0 - blank , 1 - pending , 2 - ready to render, 3 - added to compositor

  void (*evt)(struct _view_t*, ev_t); // event handler for view
  void (*tex)(struct _view_t*);       // texture generator for view

} view_t;

view_t* view_new(char* id, v4_t frame, void (*evt)(struct _view_t*, ev_t), void (*tex)(struct _view_t*), void* data);
void    view_tex(view_t* view);
void    view_evt(view_t* view, ev_t ev);
void    view_setpos(view_t* view, v2_t pos);
void    view_setdim(view_t* view, v2_t dim);
void    view_setbmp(view_t* view, bm_t* bmp);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "mtcstring.c"
#include "mtmemory.c"
#include "text.c"

void view_del(void* pointer)
{
  view_t* view = (view_t*)pointer;
  REL(view->id);
  REL(view->bmp);
  REL(view->views);
}

view_t* view_new(char* id, v4_t frame, void (*pevt)(struct _view_t*, ev_t), void (*ptex)(struct _view_t*), void* data)
{
  view_t* view = mtmem_calloc(sizeof(view_t), view_del);
  view->id     = mtcstr_fromcstring(id);
  view->bmp    = NULL;
  view->evt    = pevt;
  view->tex    = ptex;
  view->data   = data;
  view->views  = VNEW();
  view->frame  = frame;

  return view;
}

void view_evt(view_t* view, ev_t ev)
{
  (*view->evt)(view, ev);
}

void view_tex(view_t* view)
{
  (*view->tex)(view);
}

void view_setpos(view_t* view, v2_t pos)
{
  view->frame.x       = pos.x;
  view->frame.y       = pos.y;
  view->frame_changed = 1;
}

void view_setdim(view_t* view, v2_t dim)
{
  view->frame.z       = dim.x;
  view->frame.w       = dim.y;
  view->frame_changed = 1;
}

void view_setbmp(view_t* view, bm_t* bmp)
{
  RPL(view->bmp, bmp);
  view->bmp_state   = 2;
  view->bmp_changed = 1;
}

#endif
