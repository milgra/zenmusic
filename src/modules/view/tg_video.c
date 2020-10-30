/*
  Video texture generator
  Shows individual texture in view
 */

#ifndef texgen_video_h
#define texgen_video_h

#include "view.c"

typedef struct _tg_video_t
{
  int width;
  int height;

} tg_video_t;

void tg_video_add(view_t* view, int w, int h);

#endif

#if __INCLUDE_LEVEL__ == 0

void tg_video_add(view_t* view, int w, int h)
{
  tg_video_t* tg = mtmem_calloc(sizeof(tg_video_t), "tg_video", NULL, NULL);
  tg->width      = w;
  tg->height     = h;

  view->tex_state = TS_READY;
}

#endif
