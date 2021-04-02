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

typedef enum _flexdir_t // flexdir
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
  int         shadow_h;
  int         shadow_w;
  int         shadow_blur;
  int         shadow_color;
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

typedef enum _rentype_t
{
  RT_BACKGROUND,
  RT_IMMEDIATE
} rentype_t;

typedef struct _texture_t
{
  textype_t type;    /* managed or external */
  uint32_t  page;    /* texture page */
  rentype_t rentype; /* background or immediate rendering */
  float     alpha;

  // internal texture

  char*   id;            /* texture id, multiple views can show the same texture */
  texst_t state;         /* render state of texture */
  bm_t*   bitmap;        /* texture bitmap */
  char    changed;       /* texture is changed */
  char    alpha_changed; /* alpha channel is changed */

  // decoration

  char full;
  char blur;
} texture_t;

typedef struct _frame_t
{
  r2_t local;  // local position
  r2_t global; // global position
  r2_t region; // region to show
  char pos_changed;
  char dim_changed;
  char reg_changed;
} frame_t;

typedef struct _view_t view_t;
struct _view_t
{
  char hidden; /* exclude from rendering */
  char vis_changed;
  char overflow; /* enable content outside frame */
  char display;  /* view should be displayed? */
  char masked;   /* view should be used as mask for subviews? */

  char needs_key;     /* accepts key events */
  char needs_text;    /* accepts text events */
  char needs_time;    /* accepts time events */
  char needs_touch;   /* accepts touch events */
  char blocks_touch;  /* blocks touch events */
  char blocks_scroll; /* blocks scroll events */

  char*    id;     /* identifier for handling view */
  vec_t*   views;  /* subviews */
  view_t*  parent; /* parent view */
  uint32_t index;  /* depth */

  frame_t   frame;
  vlayout_t layout;
  texture_t texture;

  void (*handler)(view_t*, ev_t); /* view handler for view */
  void (*tex_gen)(view_t*);       /* texture generator for view */
  void* handler_data;             /* data for event handler */
  void* tex_gen_data;             /* data for texture generator */
};

view_t* view_new(char* id, r2_t frame);
void    view_add(view_t* view, view_t* subview);
void    view_insert(view_t* view, view_t* subview, uint32_t index);
void    view_remove(view_t* view, view_t* subview);

void    view_evt(view_t* view, ev_t ev); /* general event, sending to all views */
void    view_coll_touched(view_t* view, ev_t ev, vec_t* queue);
view_t* view_get_subview(view_t* view, char* id);
void    view_gen_texture(view_t* view);

void view_set_frame(view_t* view, r2_t frame);
void view_set_region(view_t* view, r2_t frame);
void view_set_layout(view_t* view, vlayout_t layout);
void view_set_hidden(view_t* view, char hidden, char recursive);
void view_set_block_touch(view_t* view, char block, char recursive);
void view_set_texture_bmp(view_t* view, bm_t* tex);
void view_set_texture_id(view_t* view, char* id);
void view_set_texture_page(view_t* view, uint32_t page);
void view_set_texture_type(view_t* view, textype_t type);
void view_set_texture_alpha(view_t* view, float alpha, char recur);

void view_desc(void* pointer, int level);
void view_desc_layout(vlayout_t l);
void view_calc_global(view_t* view);

extern char resend;

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtmemory.c"
#include <limits.h>

char resend = 1;

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
  view_t* view        = mem_calloc(sizeof(view_t), "view_t", view_del, view_desc);
  view->id            = cstr_fromcstring(id);
  view->views         = VNEW();
  view->frame.local   = frame;
  view->frame.global  = frame;
  view->texture.page  = -1;
  view->texture.id    = cstr_fromcstring(id);
  view->texture.alpha = 1.0;
  view->needs_touch   = 1;
  view->blocks_touch  = 1;
  view->display       = 0; // by default no display, tex generators will set this to 1

  // reset margins

  view->layout.margin_top    = INT_MAX;
  view->layout.margin_left   = INT_MAX;
  view->layout.margin_right  = INT_MAX;
  view->layout.margin_bottom = INT_MAX;
  view->layout.top           = INT_MAX;
  view->layout.left          = INT_MAX;
  view->layout.right         = INT_MAX;
  view->layout.bottom        = INT_MAX;
  view->layout.shadow_color  = 0x00000033;

  return view;
}

void view_set_masked(view_t* view, char masked)
{
  view->masked = 1;
  for (int i = 0; i < view->views->length; i++)
  {
    view_t* sview = view->views->data[i];
    view_set_masked(sview, masked);
  }
}

void view_add(view_t* view, view_t* subview)
{
  resend = 1;

  for (int i = 0; i < view->views->length; i++)
  {
    view_t* sview = view->views->data[i];
    if (strcmp(sview->id, subview->id) == 0)
    {
      printf("DUPLICATE SUBVIEW %s\n", subview->id);
      return;
    }
  }

  VADD(view->views, subview);
  subview->parent = view;

  if (view->masked) view_set_masked(subview, 1);

  view_calc_global(view);
}

void view_insert(view_t* view, view_t* subview, uint32_t index)
{
  resend = 1;

  for (int i = 0; i < view->views->length; i++)
  {
    view_t* sview = view->views->data[i];
    if (strcmp(sview->id, subview->id) == 0)
    {
      printf("DUPLICATE SUBVIEW %s\n", subview->id);
      return;
    }
  }

  vec_ins(view->views, subview, index);
  subview->parent = view;

  if (view->masked) view_set_masked(subview, 1);

  view_calc_global(view);
}

void view_remove(view_t* view, view_t* subview)
{
  resend = 1;

  VREM(view->views, subview);

  subview->parent = NULL;
}

void view_coll_touched(view_t* view, ev_t ev, vec_t* queue)
{
  if (ev.x <= view->frame.global.x + view->frame.global.w &&
      ev.x >= view->frame.global.x &&
      ev.y <= view->frame.global.y + view->frame.global.h &&
      ev.y >= view->frame.global.y)
  {
    VADD(queue, view);
    for (int i = 0; i < view->views->length; i++)
    {
      view_t* v = view->views->data[i];
      view_coll_touched(v, ev, queue);
    }
  }
}

view_t* view_get_subview(view_t* view, char* id)
{
  if (strcmp(view->id, id) == 0) return view;
  for (int i = 0; i < view->views->length; i++)
  {
    view_t* sv = view->views->data[i];
    view_t* re = view_get_subview(sv, id);
    if (re) return re;
  }
  return NULL;
}

void view_evt(view_t* view, ev_t ev)
{
  view_t* v;
  while ((v = VNXT(view->views)))
    view_evt(v, ev);

  if (view->handler) (*view->handler)(view, ev);
}

void view_calc_global(view_t* view)
{
  r2_t frame_parent = {0};
  if (view->parent != NULL) frame_parent = view->parent->frame.global;

  r2_t frame_local  = view->frame.local;
  r2_t frame_global = frame_local;

  frame_global.x = roundf(frame_parent.x) + roundf(view->frame.local.x);
  frame_global.y = roundf(frame_parent.y) + roundf(view->frame.local.y);

  r2_t old_global = view->frame.global;

  if (frame_global.w != old_global.w ||
      frame_global.h != old_global.h)
  {
    view->frame.dim_changed = 1;
    if (frame_global.w >= 1.0 &&
        frame_global.h >= 1.0)
    {
      if (view->texture.type == TT_MANAGED) view->texture.state = TS_BLANK;
    }
  }
  else
    view->frame.pos_changed = 1;

  view->frame.global = frame_global;

  view_t* v;
  while ((v = VNXT(view->views))) view_calc_global(v);
}

void view_set_frame(view_t* view, r2_t frame)
{
  view->frame.local = frame;
  view_calc_global(view);
}

void view_set_region(view_t* view, r2_t region)
{
  view->frame.region      = region;
  view->frame.reg_changed = 1;
}

void view_set_hidden(view_t* view, char hidden, char recursive)
{
  view->hidden      = hidden;
  view->vis_changed = 1;

  if (recursive)
  {
    view_t* v;
    while ((v = VNXT(view->views))) view_set_hidden(v, hidden, recursive);
  }
}

void view_set_block_touch(view_t* view, char block, char recursive)
{
  view->blocks_touch = block;

  if (recursive)
  {
    view_t* v;
    while ((v = VNXT(view->views))) view_set_block_touch(v, block, recursive);
  }
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
  view->texture.id = cstr_fromcstring(id);
}

void view_set_texture_page(view_t* view, uint32_t page)
{
  view->texture.page = page;
}

void view_set_texture_type(view_t* view, textype_t type)
{
  view->texture.type = type;
}

void view_set_texture_alpha(view_t* view, float alpha, char recur)
{
  view->texture.alpha         = alpha;
  view->texture.alpha_changed = 1;

  if (recur)
  {
    view_t* v;
    while ((v = VNXT(view->views))) view_set_texture_alpha(v, alpha, recur);
  }
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
