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
  char* data;
} tg_bitmap_t;

void tg_bitmap_add(view_t* view, char* filepath, uint8_t* data);

#endif

#if __INCLUDE_LEVEL__ == 0

#define STB_IMAGE_IMPLEMENTATION
#include "mtcstring.c"
#include "stb_image.h"

void tg_bitmap_gen(view_t* view)
{
  tg_bitmap_t* tg = view->tgdata;
  // load png
  char* path = tg->path;

  int components, w, h;

  unsigned char* bytes = stbi_load(path, &w, &h, &components, 4);

  bm_t* bmp = bm_new(w, h);

  memcpy(bmp->data, bytes, bmp->size);

  stbi_image_free(bytes);

  view_set_texture(view, bmp);
}

void tg_bitmap_add(view_t* view, char* filepath, uint8_t* data)
{
  tg_bitmap_t* tg = mtmem_calloc(sizeof(tg_bitmap_t), "tg_bitmap", NULL, NULL);
  tg->path        = mtcstr_fromcstring(filepath);
  tg->data        = data;

  view->tex_state = TS_BLANK;
  view->tgdata    = tg;
  view->tg        = tg_bitmap_gen;
}

#endif