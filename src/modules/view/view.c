#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtvector.c"
#include "wm_event.c"

typedef enum _texst_t // texture loading state
{
  TS_BLANK,
  TS_PENDING,
  TS_READY,
} texst_t;

typedef enum _laypos_t // layout position
{
  LP_STATIC = 0,
  LP_FIXED,
  LP_ABSOLUTE,
} laypos_t;

typedef enum _laydis_t // layout display
{
  LD_NONE = 0,
  LD_FLEX,
} laydis_t;

typedef struct _vlayout_t vlayout_t; // view layout
struct _vlayout_t
{
  laypos_t position;
  laydis_t display;
  float    w_per;
  float    h_per;
  int      width;
  int      height;
  int      margin;
  int      margin_top;
  int      margin_left;
  int      margin_right;
  int      margin_bottom;
};

typedef struct _vframe_t vframe_t; // view frame
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
  char connected; /* view is added to connector */

  char*     id;     /* identifier for handling view */
  mtvec_t*  views;  /* subviews */
  view_t*   parent; /* parent view */
  uint32_t  index;  /* depth */
  vlayout_t layout;

  vframe_t frame;         /* parent local position and dimensions */
  vframe_t frame_global;  /* global position and dimensions */
  char     frame_changed; /* frame changed */

  bm_t*   tex;         /* texture of view */
  texst_t tex_state;   /* texture state */
  int     tex_channel; /* texture channel if texture is external */
  char    tex_changed; /* texture changed */

  char hidden;
  char shadow;
  char blur;

  void (*evt_han)(view_t*, ev_t); /* event handler for view */
  void (*tex_gen)(view_t*);       /* texture generator for view */
  void* evt_han_data;             /* data for event handler */
  void* tex_gen_data;             /* data for texture generator */
};

view_t* view_new(char* id, vframe_t frame, int texture_channel);
void    view_add(view_t* view, view_t* subview);
void    view_insert(view_t* view, view_t* subview, uint32_t index);
void    view_remove(view_t* view, view_t* subview);
void    view_evt(view_t* view, ev_t ev);
void    view_set_frame(view_t* view, vframe_t frame);
void    view_set_layout(view_t* view, vlayout_t layout);
void    view_set_texture(view_t* view, bm_t* tex);
void    view_gen_texture(view_t* view);
void    view_desc(void* pointer);
void    view_calc_global(view_t* view);

extern char reindex;

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "mtcstring.c"
#include "mtmemory.c"
#include "text.c"

char reindex = 1;

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
  view_t* view       = mtmem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id           = mtcstr_fromcstring(id);
  view->views        = VNEW();
  view->frame        = frame;
  view->frame_global = frame;
  view->tex_channel  = texture_channel;
  view->tex_state    = texture_channel == 0 ? TS_BLANK : TS_READY;

  return view;
}

void view_add(view_t* view, view_t* subview)
{
  reindex = 1;

  VADD(view->views, subview);
  subview->parent = view;

  view_calc_global(view);
}

void view_insert(view_t* view, view_t* subview, uint32_t index)
{
  reindex = 1;

  mtvec_addatindex(view->views, subview, index);
  subview->parent = view;

  view_calc_global(view);
}

void view_remove(view_t* view, view_t* subview)
{
  reindex = 1;

  // TODO!!!
  // remove all subviews recursively so ui_connector will know what to cleanup
  /* view_t* v; */
  /* while ((v = VNXT(subview->views))) */
  /* { */
  /*   view_remove(subview, v); */
  /* } */

  VREM(view->views, subview);
  subview->parent = NULL;
}

void view_evt(view_t* view, ev_t ev)
{
  view_t* v;
  while ((v = VNXT(view->views)))
    view_evt(v, ev);

  if (view->evt_han) (*view->evt_han)(view, ev);
}

void view_calc_global(view_t* view)
{
  vframe_t frame_parent = {0};
  if (view->parent != NULL) frame_parent = view->parent->frame_global;
  view->frame_global.x = frame_parent.x + view->frame.x;
  view->frame_global.y = frame_parent.y + view->frame.y;
  view->frame_changed  = 1;

  view_t* v;
  while ((v = VNXT(view->views)))
    view_calc_global(v);
}

void view_set_frame(view_t* view, vframe_t frame)
{
  view->frame        = frame;
  view->frame_global = frame;

  view_calc_global(view);
}

void view_set_texture(view_t* view, bm_t* tex)
{
  RPL(view->tex, tex);
  view->tex_state   = TS_READY;
  view->tex_changed = 1;
}

void view_set_layout(view_t* view, vlayout_t layout)
{
  view->layout = layout;
}

void view_gen_texture(view_t* view)
{
  if (view->tex_gen) (*view->tex_gen)(view);
}

void view_desc(void* pointer)
{
  view_t* view = (view_t*)pointer;
  printf("id %s frame %f %f %f %f\n", view->id, view->frame.x, view->frame.y, view->frame.w, view->frame.h);
}

#endif
