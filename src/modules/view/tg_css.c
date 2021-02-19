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

void     tg_css_add(view_t* view);
void     tg_css_set_graycolor(uint32_t color);
uint32_t tg_css_get_graycolor();
uint32_t tg_css_get_newcolor(uint32_t color);

#endif

#if __INCLUDE_LEVEL__ == 0

#define STB_IMAGE_IMPLEMENTATION
#include "mtcstring.c"
#include "mtgraphics.c"
#include "stb_image.h"

uint32_t tg_css_graycolor = 0;

void tg_css_set_graycolor(uint32_t color)
{
  tg_css_graycolor = color;
}

uint32_t tg_css_get_graycolor()
{
  return tg_css_graycolor;
}

uint32_t tg_css_get_newcolor(uint32_t color)
{
  int cr = (color >> 24) & 0xFF;
  int cg = (color >> 16) & 0xFF;
  int cb = (color >> 8) & 0xFF;
  int ca = color & 0xFF;

  int nr = (tg_css_graycolor >> 24) & 0xFF;
  int ng = (tg_css_graycolor >> 16) & 0xFF;
  int nb = (tg_css_graycolor >> 8) & 0xFF;

  int fr = nr - (255 - cr);
  int fg = ng - (255 - cg);
  int fb = nb - (255 - cb);

  if (cr == 255) fr += 50;
  if (cg == 255) fg += 50;
  if (cb == 255) fb += 50;

  int light = (nr + ng + nb) / 2 > 130;

  if (!light)
  {
    fr = nr + (255 - cr);
    fg = ng + (255 - cg);
    fb = nb + (255 - cb);
  }

  if (fr < 0) fr = 0;
  if (fr > 255) fr = 255;
  if (fg < 0) fg = 0;
  if (fg > 255) fg = 255;
  if (fb < 0) fb = 0;
  if (fb > 255) fb = 255;

  uint32_t rescolor = (fr << 24) | (fg << 16) | (fb << 8) | ca;

  return rescolor;
}

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

      int            c, w, h;
      unsigned char* imagebytes = stbi_load(view->layout.background_image, &w, &h, &c, 4);
      if (imagebytes != NULL)
      {
        bm_t* imagebmp = bm_new(w, h);
        memcpy(imagebmp->data, imagebytes, imagebmp->size);
        stbi_image_free(imagebytes);
        gfx_insert(bm, imagebmp, 0, 0);
        REL(imagebmp);
        view->texture.changed = 1;
      }
      else
        printf("invalid image : %s\n", view->layout.background_image);
    }
    else if (view->layout.background_color)
    {
      uint32_t color = view->layout.background_color;

      if (tg_css_graycolor > 0) color = tg_css_get_newcolor(color);

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
