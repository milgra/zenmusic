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
  bm_t*    bm;
  mtmap_t* coords;
  int      cx; // cursor x
  int      cy; // cursor y
  int      ch; // cursor height
  char     is_full;
  char     did_change;
};

tm_t*       tm_new();
void        tm_del(void* p);
void        tm_reset(tm_t* tm);
char        tm_has(tm_t* tm, char* id);
tm_coords_t tm_get(tm_t* tm, char* id);
char        tm_put(tm_t* tm, char* id, bm_t* bm);
void        tm_upd(tm_t* tm, char* id, bm_t* bm);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"

tm_t* tm_new(int w, int h)
{
  tm_t* tm   = mtmem_calloc(sizeof(tm_t), "tm_t", tm_del, NULL);
  tm->bm     = bm_new(w, h);
  tm->coords = mtmap_alloc();

  return tm;
}

void tm_del(void* p)
{
  tm_t* tm = (tm_t*)p;
  REL(tm->bm);
  REL(tm->coords);
}

void tm_reset(tm_t* tm)
{
  mtmap_reset(tm->coords);
  bm_reset(tm->bm);
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

  tm_coords_t* coords = HEAP(((tm_coords_t){.ltx = (float)ncx / (float)tm->bm->w,
                                            .lty = (float)ncy / (float)tm->bm->h,
                                            .rbx = (float)rbx / (float)tm->bm->w,
                                            .rby = (float)rby / (float)tm->bm->h,
                                            .x   = ncx,
                                            .y   = ncy,
                                            .w   = bm->w,
                                            .h   = bm->h}),
                             "float*");

  mtmap_put(tm->coords, id, coords);

  tm->cx = rbx;
  tm->cy = ncy;
  tm->ch = nch;

  return 0; // success
}

void tm_upd(tm_t* tm, char* id, bm_t* bm)
{
  tm_coords_t* coords = mtmap_get(tm->coords, id);

  if (coords)
  {
    if (bm->w == coords->w && bm->h == coords->h)
    {
      bm_insert(tm->bm, bm, coords->x, coords->y);
    }
  }
}

#endif
