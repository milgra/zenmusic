#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtvector.c"
#include "wm_event.c"

typedef struct _vframe_t vframe_t;
struct _vframe_t
{
  int x;
  int y;
  int w;
  int h;
};

typedef struct _view_t view_t;
struct _view_t
{
  char*    id;     /* identifier for handling view */
  void*    data;   /* data for event handler and bitmap generator */
  mtvec_t* views;  /* subviews */
  view_t*  parent; /* parent view */

  char attached;

  vframe_t frame;         /* position and dimensions */
  char     frame_changed; /* frame changed */

  bm_t* bmp;         /* bitmap of view */
  char  bmp_changed; /* bitmap changed */
  char  bmp_state;   /* 0 - blank , 1 - pending , 2 - ready to render, 3 - added to compositor */

  void (*evt)(view_t*, ev_t); /* event handler for view */
  void (*tex)(view_t*);       /* texture generator for view */
};

view_t* view_new(char* id, vframe_t frame, void (*evt)(struct _view_t*, ev_t), void (*tex)(struct _view_t*), void (*new)(struct _view_t*, void* arg), void* arg);
void    view_tex(view_t* view);
void    view_evt(view_t* view, ev_t ev);
void    view_setframe(view_t* view, vframe_t frame);
void    view_setbmp(view_t* view, bm_t* bmp);
void    view_add(view_t* view, view_t* subview);
void    view_rem(view_t* view, view_t* subview);
void    view_desc(void* pointer);
void    view_setdata(view_t* view, void* data);

extern char view_needs_resend;

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "mtcstring.c"
#include "mtmemory.c"
#include "text.c"

char view_needs_resend = 1;

void view_del(void* pointer)
{
  view_t* view = (view_t*)pointer;
  REL(view->id);
  REL(view->bmp);
  REL(view->views);
}

view_t* view_new(char*    id,                              /* view id */
                 vframe_t frame,                           /* view frame */
                 void (*pevt)(struct _view_t*, ev_t),      /* event handles for view */
                 void (*ptex)(struct _view_t*),            /* texture generator for view */
                 void (*pnew)(struct _view_t*, void* arg), /* event handler initializer for view */
                 void* arg)                                /* event handler initializer argument for view */
{
  view_t* view = mtmem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id     = mtcstr_fromcstring(id);
  view->bmp    = NULL;
  view->evt    = pevt;
  view->tex    = ptex;
  view->views  = VNEW();
  view->frame  = frame;

  if (pnew) (*pnew)(view, arg);

  return view;
}

void view_evt(view_t* view, ev_t ev)
{
  if (*view->evt) (*view->evt)(view, ev);
}

void view_tex(view_t* view)
{
  if (*view->tex) (*view->tex)(view);
}

void view_setframe(view_t* view, vframe_t frame)
{
  view->frame         = frame;
  view->frame_changed = 1;
}

void view_setbmp(view_t* view, bm_t* bmp)
{
  RPL(view->bmp, bmp);
  view->bmp_state   = 2;
  view->bmp_changed = 1;
}

void view_setdata(view_t* view, void* data)
{
  view->data = data;
}

void view_add(view_t* view, view_t* subview)
{
  VADD(view->views, subview);
  subview->parent   = view;
  view_needs_resend = 1;
}

void view_rem(view_t* view, view_t* subview)
{
  VREM(view->views, subview);
  subview->parent   = NULL;
  view_needs_resend = 1;
}

void view_desc(void* pointer)
{
  view_t* view = (view_t*)pointer;
  printf("id %s frame %i %i %i %i\n", view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h);
}

#endif
