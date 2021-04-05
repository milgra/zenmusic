/*
  CSS texture generator
  Generates texture based on css layout
 */

#ifndef texgen_css_h
#define texgen_css_h

#include "mtbitmap.c"
#include "view.c"

typedef struct _tg_css_t
{
  char* id;
  char* path;
  bm_t* bitmap;
} tg_bitmap_t;

void tg_css_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "editor.c"
#include "mtcstring.c"
#include "mtgraphics.c"

uint32_t tg_css_graycolor = 0;

void tg_css_gen(view_t* view)
{
  if (view->frame.local.w >= 1.0 &&
      view->frame.local.h >= 1.0)
  {
    if (view->layout.background_image)
    {
      bm_t* bm = view->texture.bitmap;

      if (bm == NULL ||
          bm->w != (int)view->frame.local.w ||
          bm->h != (int)view->frame.local.h)
      {
        bm = bm_new((int)view->frame.local.w, (int)view->frame.local.h);
        view_set_texture_bmp(view, bm);
      }

      bm_t* bmap = editor_get_image(view->layout.background_image);
      view_set_texture_bmp(view, bmap);
      REL(bmap);
    }
    else if (view->layout.background_color)
    {
      uint32_t color = view->layout.background_color;

      float w = view->frame.local.w + 2 * view->layout.shadow_blur;
      float h = view->frame.local.h + 2 * view->layout.shadow_blur;

      bm_t* bm = view->texture.bitmap;

      if (bm == NULL ||
          bm->w != (int)w ||
          bm->h != (int)h)
      {
        bm = bm_new((int)w, (int)h);
        view_set_texture_bmp(view, bm);
      }

      bm_reset(bm);

      gfx_rounded_rect(bm,
                       0,
                       0,
                       w,
                       h,
                       view->layout.border_radius,
                       view->layout.shadow_blur,
                       color,
                       view->layout.shadow_color);

      view->texture.changed = 1;
    }
  }
}

void tg_css_add(view_t* view)
{
  view->tex_gen = tg_css_gen;
  view->display = 1;
}

#endif
