#ifndef view_h
#define view_h

#include "mtbitmap.c"
#include "mtmath2.c"
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

typedef enum _exdir_t // flexdir
{
  FD_ROW = 0,
  FD_COL,
} flexdir_t;

typedef enum _itemalign_t // flexdir
{
  IA_NONE = 0,
  IA_CENTER,
} itemalign_t;

typedef enum _cjustify_t // justify content
{
  JC_NONE = 0,
  JC_CENTER,
} cjustify_t;

typedef struct _vlayout_t vlayout_t; // view layout
struct _vlayout_t
{
  laypos_t    position;
  laydis_t    display;
  flexdir_t   flexdir;
  itemalign_t itemalign;
  cjustify_t  cjustify;
  float       w_per;
  float       h_per;
  int         width;
  int         height;
  int         margin;
  int         margin_top;
  int         margin_left;
  int         margin_right;
  int         margin_bottom;
  int         top;
  int         left;
  int         right;
  int         bottom;
  int         border_radius;
  uint32_t    background_color;
  char*       background_image;
};

typedef enum _texst_t // texture loading state
{
  TS_BLANK,   /* texture is empty */
  TS_PENDING, /* texture is under generation */
  TS_READY,   /* texture is generated */
} texst_t;

typedef enum _textype_t
{
  TT_MANAGED,
  TT_EXTERNAL
} textype_t;

typedef struct _texture_t
{
  textype_t type; /* managed or external */
  uint32_t  page; /* texture page */

  // internal texture

  char*   id;      /* texture id, multiple views can show the same texture */
  texst_t state;   /* render state of texture */
  bm_t*   bitmap;  /* texture bitmap */
  char    changed; /* texture is changed */

  // decoration

  char full;
  char blur;
  char shadow;
} texture_t;

typedef struct _frame_t
{
  r2_t local;
  r2_t global;
  char changed;
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

view_t* view_new(char* id, r2_t frame);
void    view_add(view_t* view, view_t* subview);
void    view_insert(view_t* view, view_t* subview, uint32_t index);
void    view_remove(view_t* view, view_t* subview);

void view_evt(view_t* view, ev_t ev);
void view_gen_texture(view_t* view);

void view_set_frame(view_t* view, r2_t frame);
void view_set_layout(view_t* view, vlayout_t layout);
void view_set_texture_bmp(view_t* view, bm_t* tex);
void view_set_texture_id(view_t* view, char* id);
void view_set_texture_page(view_t* view, uint32_t page);
void view_set_texture_type(view_t* view, textype_t type);

void view_desc(void* pointer, int level);
void view_desc_layout(vlayout_t l);
void view_calc_global(view_t* view);

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

  if (view->layout.background_image != NULL) REL(view->layout.background_image);

  REL(view->id);
  REL(view->texture.bitmap);
  REL(view->views);
}

view_t* view_new(char* id, r2_t frame)
{
  view_t* view       = mtmem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id           = mtcstr_fromcstring(id);
  view->views        = VNEW();
  view->frame.local  = frame;
  view->frame.global = frame;
  view->texture.page = -1;
  view->texture.id   = mtcstr_fromcstring(id);

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
  r2_t frame_parent = {0};
  if (view->parent != NULL) frame_parent = view->parent->frame.global;
  view->frame.global.x = roundf(frame_parent.x) + roundf(view->frame.local.x);
  view->frame.global.y = roundf(frame_parent.y) + roundf(view->frame.local.y);
  view->frame.changed  = 1;

  view_t* v;
  while ((v = VNXT(view->views)))
    view_calc_global(v);
}

void view_set_frame(view_t* view, r2_t frame)
{
  // force rerender
  // TODO this will cause problems when eh's render textures instead of tg's

  if (view->frame.local.w != frame.w || view->frame.local.h != frame.h)
  {
    if (view->texture.type == TT_MANAGED) view->texture.state = TS_BLANK;
  }

  view->frame.local  = frame;
  view->frame.global = frame;

  view_calc_global(view);
}

void view_set_texture_bmp(view_t* view, bm_t* bitmap)
{
  RPL(view->texture.bitmap, bitmap);
  view->texture.state   = TS_READY;
  view->texture.changed = 1;
}

void view_set_texture_id(view_t* view, char* id)
{
  REL(view->texture.id);
  view->texture.id = mtcstr_fromcstring(id);
}

void view_set_texture_page(view_t* view, uint32_t page)
{
  view->texture.page = page;
}

void view_set_texture_type(view_t* view, textype_t type)
{
  view->texture.type = type;
}

void view_set_layout(view_t* view, vlayout_t layout)
{
  view->layout = layout;
}

void view_gen_texture(view_t* view)
{
  if (view->tex_gen) (*view->tex_gen)(view);
}

void view_desc(void* pointer, int level)
{
  view_t* view = (view_t*)pointer;
  printf("%*.sid %s frame %.1f %.1f %.1f %.1f tex %i\n", level, " ",
         view->id,
         view->frame.local.x,
         view->frame.local.y,
         view->frame.local.w,
         view->frame.local.h,
         view->texture.page);

  for (int i = 0; i < view->views->length; i++)
  {
    view_desc(view->views->data[i], level + 1);
  }
}

void view_desc_layout(vlayout_t l)
{
  printf("position %i\n"
         "display %i\n"
         "flexdir %i\n"
         "itemalign %i\n"
         "cjustify %i\n"
         "w_per %f\n"
         "h_per %f\n"
         "width %i\n"
         "height %i\n"
         "margin %i\n"
         "margin_top %i\n"
         "margin_left %i\n"
         "margin_right %i\n"
         "margin_bottom %i\n"
         "top %i\n"
         "left %i\n"
         "right %i\n"
         "bottom %i\n"
         "border_radius %i\n"
         "background_color %x\n"
         "background_image %s\n",
         l.position,
         l.display,
         l.flexdir,
         l.itemalign,
         l.cjustify,
         l.w_per,
         l.h_per,
         l.width,
         l.height,
         l.margin,
         l.margin_top,
         l.margin_left,
         l.margin_right,
         l.margin_bottom,
         l.top,
         l.left,
         l.right,
         l.bottom,
         l.border_radius,
         l.background_color,
         l.background_image == NULL ? "" : l.background_image);
}

#endif
