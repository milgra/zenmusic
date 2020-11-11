#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtvector.c"
#include "wm_event.c"

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

typedef enum _texst_t // texture loading state
{
  TS_BLANK,   /* texture is empty */
  TS_PENDING, /* texture is under generation */
  TS_READY,   /* texture is generated */
  TS_EXTERN,  /* texture is handled outside ui compositor */
} texst_t;

typedef struct _texture_t
{
  texst_t  state;
  uint32_t index;
  bm_t*    bitmap;
  char     changed;

  char blur;
  char shadow;
} texture_t;

typedef struct _frame_t
{
  vframe_t local;
  vframe_t global;
  char     changed;
} frame_t;

typedef struct _view_t view_t;
struct _view_t
{
  char hidden;    /* exclude from rendering */
  char overflow;  /* enable content outside frame */
  char connected; /* view is added to connector */

  char*    id;     /* identifier for handling view */
  mtvec_t* views;  /* subviews */
  view_t*  parent; /* parent view */
  uint32_t index;  /* depth */

  frame_t   frame;
  vlayout_t layout;
  texture_t texture;

  void (*evt_han)(view_t*, ev_t); /* event handler for view */
  void (*tex_gen)(view_t*);       /* texture generator for view */
  void* evt_han_data;             /* data for event handler */
  void* tex_gen_data;             /* data for texture generator */
};

view_t* view_new(char* id, vframe_t frame);
void    view_add(view_t* view, view_t* subview);
void    view_insert(view_t* view, view_t* subview, uint32_t index);
void    view_remove(view_t* view, view_t* subview);
void    view_evt(view_t* view, ev_t ev);
void    view_set_frame(view_t* view, vframe_t frame);
void    view_set_layout(view_t* view, vlayout_t layout);
void    view_set_texture(view_t* view, bm_t* tex);
void    view_set_texture_index(view_t* view, uint32_t index);
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
  REL(view->texture.bitmap);
  REL(view->views);
}

view_t* view_new(char* id, vframe_t frame)
{
  view_t* view       = mtmem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id           = mtcstr_fromcstring(id);
  view->views        = VNEW();
  view->frame.local  = frame;
  view->frame.global = frame;

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
  if (view->parent != NULL) frame_parent = view->parent->frame.global;
  view->frame.global.x = frame_parent.x + view->frame.local.x;
  view->frame.global.y = frame_parent.y + view->frame.local.y;
  view->frame.changed  = 1;

  view_t* v;
  while ((v = VNXT(view->views)))
    view_calc_global(v);
}

void view_set_frame(view_t* view, vframe_t frame)
{
  view->frame.local  = frame;
  view->frame.global = frame;

  view_calc_global(view);
}

void view_set_texture(view_t* view, bm_t* bitmap)
{
  RPL(view->texture.bitmap, bitmap);
  view->texture.state   = TS_READY;
  view->texture.changed = 1;
}

void view_set_texture_index(view_t* view, uint32_t index)
{
  view->texture.index = index;
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
  printf("id %s frame %f %f %f %f\n", view->id, view->frame.local.x, view->frame.local.y, view->frame.local.w, view->frame.local.h);
}

#endif
