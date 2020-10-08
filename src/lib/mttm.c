#ifndef __KineticUI__tm__
#define __KineticUI__tm__

#include "mttm.c"
#include "math4.c"
#include "mtbm.c"
#include "mtmap.c"

typedef struct _tm_t tm_t;
struct _tm_t
{
  bm_t* bm;
  mtmap_t* coords;
  int cx; // cursor x
  int cy; // cursor y
  int ch; // cursor height
  char is_full;
  char did_change;
};

tm_t* tm_new();
void tm_del(tm_t* tm);
void tm_reset(tm_t* tm);
v4_t tm_get(tm_t* tm, char* id);
char tm_put(tm_t* tm, char* id, bm_t* bm);
void tm_upd(tm_t* tm, char* id, bm_t* bm);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "math4.c"
#include "mtbm.c"
#include "mtmap.c"
#include "mtmem.c"
#include "mttm.c"

tm_t* tm_new()
{
  tm_t* tm = mtmem_calloc(sizeof(tm_t), NULL);
  tm->bm = bm_new(1024, 1024);
  tm->coords = mtmap_alloc();

  return tm;
}

void tm_del(tm_t* tm)
{
  REL(tm->bm);
  REL(tm);
}

void tm_reset(tm_t* tm)
{
  mtmap_reset(tm->coords);
  bm_reset(tm->bm);
  tm->cx = 0;
  tm->cy = 0;
  tm->ch = 0;
  tm->is_full = 0;
  tm->did_change = 0;
}

v4_t tm_get(tm_t* tm, char* id)
{
  v4_t* coords = mtmap_get(tm->coords, id);
  if (coords) return *coords;
  return (v4_t){0};
}

char tm_put(tm_t* tm, char* id, bm_t* bm)
{

  if (bm->w > tm->bm->w || bm->h > tm->bm->h) return -1; // too big bitmap

  int nch = tm->ch; // new cursor height

  if (tm->cx + bm->w > tm->bm->w)
  {
    nch = bm->h;
  }
  else if (bm->h > tm->ch)
  {
    nch = bm->h;
  }

  int ncy = tm->cy; // new cursor y

  if (tm->cx + bm->w > tm->bm->w)
  {
    ncy = tm->cy + tm->ch;
  }

  int ncx = tm->cx; // new cursor x

  if (tm->cx + bm->w > tm->bm->w)
  {
    ncx = 0;
  }

  int rbx = ncx + bm->w;
  int rby = ncy + bm->h;

  char is_full = nch > tm->bm->h;

  if (is_full) return -2; // tilemap is full

  bm_insert(tm->bm, bm, ncx, ncy);

  float* coords = HEAP(((v4_t){(float)ncx / (float)tm->bm->w,
                               (float)ncy / (float)tm->bm->h,
                               (float)rbx / (float)tm->bm->w,
                               (float)rby / (float)tm->bm->h}));

  mtmap_put(tm->coords, id, coords);

  tm->cx = rbx;
  tm->cy = ncy;
  tm->ch = nch;

  return 0; // success
}

void tm_upd(tm_t* tm, char* id, bm_t* bm)
{
  v4_t* coords = mtmap_get(tm->coords, id);

  if (coords)
  {
    v4_t c = *coords;
    int w = c.z - c.x;
    int h = c.w - c.y;

    if (bm->w == w && bm->h == h)
    {
      bm_insert(tm->bm, bm, c.x, c.y);
    }
  }
}

#endif
