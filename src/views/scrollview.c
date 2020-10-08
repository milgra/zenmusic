#ifndef scrollview_h
#define scrollview_h

#include "math4.c"
#include "mtbm.c"

typedef struct _scrollview_t
{
  char* id;
  bm_t* bmp;
  v4_t  frame;

  int   overflow; // enable or disable overflow
  float position; // top or bottom position of content

} scrollview_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#endif
