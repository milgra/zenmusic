#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtvector.c"
#include "wm_event.c"

typedef enum _texst_t
{
  TS_BLANK,
  TS_PENDING,
  TS_READY,
} texst_t;

typedef struct _vframe_t vframe_t;
struct _vframe_t
{
  float x;
  float y;
  float w;
  float h;
};

typedef struct _view_t view_t;
struct _view_t
{
  char*    id;     /* identifier for handling view */
  mtvec_t* views;  /* subviews */
  view_t*  parent; /* parent view */

  vframe_t frame;         /* position and dimensions */
  char     frame_changed; /* frame changed */

  bm_t*   tex;         /* texture of view */
  texst_t tex_state;   /* texture state */
  char    tex_changed; /* texture changed */
  int     tex_channel; /* texture channel if texture is external */

  void (*eh)(view_t*, ev_t); /* event handler for view */
  void (*tg)(view_t*);       /* texture generator for view */
  void* ehdata;              /* data for event handler */
  void* tgdata;              /* data for texture generator */
};

view_t* view_new(char* id, vframe_t frame, int texture_channel);
void    view_add(view_t* view, view_t* subview);
void    view_rem(view_t* view, view_t* subview);
void    view_evt(view_t* view, ev_t ev);
void    view_set_frame(view_t* view, vframe_t frame);
void    view_set_texture(view_t* view, bm_t* tex);
void    view_gen_texture(view_t* view);
void    view_desc(void* pointer);

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
  REL(view->tex);
  REL(view->views);
}

view_t* view_new(char*    id, /* view id */
                 vframe_t frame,
                 int      texture_channel) /* view frame */
{
  view_t* view      = mtmem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id          = mtcstr_fromcstring(id);
  view->views       = VNEW();
  view->frame       = frame;
  view->tex_channel = texture_channel;
  view->tex_state   = texture_channel == 0 ? TS_BLANK : TS_READY;

  return view;
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

void view_evt(view_t* view, ev_t ev)
{
  if (view->eh) (*view->eh)(view, ev);
}

void view_set_frame(view_t* view, vframe_t frame)
{
  view->frame         = frame;
  view->frame_changed = 1;
}

void view_set_texture(view_t* view, bm_t* tex)
{
  RPL(view->tex, tex);
  view->tex_state   = TS_READY;
  view->tex_changed = 1;
}

void view_gen_texture(view_t* view)
{
  if (view->tg) (*view->tg)(view);
}

void view_desc(void* pointer)
{
  view_t* view = (view_t*)pointer;
  printf("id %s frame %f %f %f %f\n", view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h);
}

#endif
