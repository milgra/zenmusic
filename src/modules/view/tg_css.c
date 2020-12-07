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

#define STB_IMAGE_IMPLEMENTATION
#include "mtcstring.c"
#include "mtgraphics.c"
#include "stb_image.h"

void tg_css_gen(view_t* view)
{
  if (view->frame.local.w >= 1.0 &&
      view->frame.local.h >= 1.0)
  {
    if (view->layout.background_image)
    {
      int            c, w, h;
      unsigned char* imagebytes = stbi_load(view->layout.background_image, &w, &h, &c, 4);
      if (imagebytes != NULL)
      {
        bm_t* imagebmp = bm_new(w, h);
        memcpy(imagebmp->data, imagebytes, imagebmp->size);
        stbi_image_free(imagebytes);
        bm_t* texbmp = bm_new(view->frame.local.w, view->frame.local.h);
        bm_insert(texbmp, imagebmp, 0, 0);
        REL(imagebmp);
        view_set_texture_bmp(view, texbmp);
      }
      else
        printf("invalid image : %s\n", view->layout.background_image);
    }
    else if (view->layout.background_color)
    {
      char idbuffer[100] = {0};
      snprintf(idbuffer, 20, "color %i %i", view->layout.background_color, view->layout.border_radius);

      float w = view->frame.local.w + 2 * view->layout.shadow_blur;
      float h = view->frame.local.h + 2 * view->layout.shadow_blur;

      bm_t* bmp = bm_new(w, h);
      //bm_fill(bmp, 0, 0, bmp->w, bmp->h, 0x00000044);

      mtgraphics_rounded_rect(bmp,
                              0,
                              0,
                              w,
                              h,
                              view->layout.border_radius,
                              view->layout.shadow_blur,
                              view->layout.background_color,
                              0x00000033);

      view_set_texture_bmp(view, bmp);
    }
  }
}

void tg_css_add(view_t* view)
{
  view->tex_gen = tg_css_gen;
}

#endif
