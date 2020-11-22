#ifndef __KineticUI__tm__
#define __KineticUI__tm__

#include "mtbitmap.c"
#include "mtmap.c"
#include "mtmath4.c"

typedef struct _tm_coords_t tm_coords_t;
struct _tm_coords_t
{
  float ltx;
  float lty;
  float rbx;
  float rby;
  int   x;
  int   y;
  int   w;
  int   h;
};

typedef struct _tm_t tm_t;
struct _tm_t
{
  mtmap_t* coords;
  int      cx; // cursor x
  int      cy; // cursor y
  int      ch; // cursor height
  char     is_full;
  char     did_change;
  int      width;
  int      height;
};

tm_t*       tm_new(int w, int h);
void        tm_del(void* p);
void        tm_reset(tm_t* tm);
char        tm_has(tm_t* tm, char* id);
tm_coords_t tm_get(tm_t* tm, char* id);
int         tm_put(tm_t* tm, char* id, int w, int h);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"

tm_t* tm_new(int w, int h)
{
  tm_t* tm   = mtmem_calloc(sizeof(tm_t), "tm_t", tm_del, NULL);
  tm->coords = mtmap_alloc();
  tm->width  = w;
  tm->height = h;

  return tm;
}

void tm_del(void* p)
{
  tm_t* tm = (tm_t*)p;
  REL(tm->coords);
}

void tm_reset(tm_t* tm)
{
  mtmap_reset(tm->coords);
  tm->cx         = 0;
  tm->cy         = 0;
  tm->ch         = 0;
  tm->is_full    = 0;
  tm->did_change = 0;
}

char tm_has(tm_t* tm, char* id)
{
  v4_t* coords = mtmap_get(tm->coords, id);
  if (coords) return 1;
  return 0;
}

tm_coords_t tm_get(tm_t* tm, char* id)
{
  tm_coords_t* coords = mtmap_get(tm->coords, id);
  if (coords) return *coords;
  return (tm_coords_t){0};
}

int tm_put(tm_t* tm, char* id, int w, int h)
{

  if (w > tm->width || h > tm->height) return -1; // too big bitmap

  int nch = tm->ch; // new cursor height

  if (tm->cx + w > tm->width)
  {
    nch = h;
  }
  else if (h > tm->ch)
  {
    nch = h;
  }

  int ncy = tm->cy; // new cursor y

  if (tm->cx + w > tm->width)
  {
    ncy = tm->cy + tm->ch;
  }

  int ncx = tm->cx; // new cursor x

  if (tm->cx + w > tm->width)
  {
    ncx = 0;
  }

  int rbx = ncx + w;
  int rby = ncy + h;

  char is_full = nch > tm->height;

  if (is_full) return -2; // tilemap is full

  tm_coords_t* coords = HEAP(((tm_coords_t){.ltx = (float)ncx / (float)tm->width,
                                            .lty = (float)ncy / (float)tm->height,
                                            .rbx = (float)rbx / (float)tm->width,
                                            .rby = (float)rby / (float)tm->height,
                                            .x   = ncx,
                                            .y   = ncy,
                                            .w   = w,
                                            .h   = h}),
                             "float*");

  mtmap_put(tm->coords, id, coords);

  tm->cx = rbx;
  tm->cy = ncy;
  tm->ch = nch;

  return 0; // success
}

#endif
