/*
  Bitmap file texture generator
  Shows bitmap file in view
 */

#ifndef texgen_bitmap_h
#define texgen_bitmap_h

#include "view.c"

typedef struct _tg_bitmap_t
{
  char* path;

} tg_bitmap_t;

void tg_bitmap_add(view_t* view, char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#define STB_IMAGE_IMPLEMENTATION
#include "mtcstring.c"
#include "stb_image.h"

void tg_bitmap_gen(view_t* view)
{
  // load png
  char* path = view->tgdata;

  int components, w, h;

  unsigned char* bytes = stbi_load(path, &w, &h, &components, 4);

  bm_t* bmp = bm_new(w, h);

  memcpy(bmp->data, bytes, bmp->size);

  stbi_image_free(bytes);

  view_set_texture(view, bmp);
}

void tg_bitmap_add(view_t* view, char* filepath)
{
  char* path = mtcstr_fromcstring(filepath);

  view->tex_state = TS_BLANK;
  view->tgdata    = path;
  view->tg        = tg_bitmap_gen;
}

#endif
