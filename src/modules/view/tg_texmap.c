/*
  Texture map texture generator
  Shows given texture map in view
 */

#ifndef texgen_texmap_h
#define texgen_texmap_h

#include "view.c"

void tg_texmap_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

void tg_texmap_add(view_t* view)
{
  // force upload to compositor without bitmap
  view->texture.page  = 1; // we will show the whole tilemap
  view->texture.state = TS_EXTERN;
}

#endif
