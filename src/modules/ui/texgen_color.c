#ifndef texgen_color_h
#define texgen_color_h

#include "view.c"

void texgen_color(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void texgen_color(view_t* view)
{
  bm_t* bmp = bm_new(200, 200);
  bm_fill(bmp,
          0,
          0,
          200,
          200,
          0xFF0000FF);
  view_setbmp(view, bmp);
}

#endif
