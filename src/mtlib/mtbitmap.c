#ifndef mtbm_h
#define mtbm_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _bm_t bm_t;
struct _bm_t
{
  int w;
  int h;

  uint8_t* data;
  uint32_t size;
};

bm_t* bm_new(int the_w, int the_h);
bm_t* bm_clone(bm_t* bm);
void  bm_reset(bm_t* bm);
void  bm_describe(void* p, int level);

void bm_insert(bm_t* the_base, bm_t* bm, int the_x, int the_y);
void bm_insert_rgb(bm_t* the_base, uint8_t* bm, int w, int h, int the_x, int the_y);
void bm_blend_pixel(bm_t* bm, int x, int y, uint32_t color);
void bm_blend_8(bm_t* bm, int nx, int ny, uint32_t color, unsigned char* ndata, int nw, int nh);
void bm_blend_rgba(bm_t* bm, int nx, int ny, bm_t* nbm);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"
#include <assert.h>
#include <string.h>

void bm_del(void* pointer)
{
  bm_t* bm = pointer;
  if (bm->data != NULL) mtmem_release(bm->data);
}

bm_t* bm_new(int the_w, int the_h)
{
  bm_t* bm = mtmem_calloc(sizeof(bm_t), "mtbitmap", bm_del, bm_describe);

  bm->w = the_w;
  bm->h = the_h;

  bm->size = 4 * the_w * the_h;
  bm->data = mtmem_calloc(bm->size * sizeof(unsigned char), "uint8_t*", NULL, NULL);

  return bm;
}

bm_t* bm_clone(bm_t* the_bm)
{
  bm_t* bm = bm_new(the_bm->w, the_bm->h);
  memcpy(bm->data, the_bm->data, the_bm->size);
  return bm;
}

void bm_reset(bm_t* bm)
{
  memset(bm->data, 0, bm->size);
}

void bm_describe(void* p, int level)
{
  bm_t* bm = p;
  printf("width %i height %i size %u", bm->w, bm->h, bm->size);
}

///

void bm_blend_pixel(bm_t* bm, int x, int y, uint32_t color)
{
  if (x > bm->w) return;
  if (y > bm->h) return;

  uint8_t* data = bm->data;
  int      i    = (y * bm->w + x) * 4;

  int dr = data[i];
  int dg = data[i + 1];
  int db = data[i + 2];
  int da = data[i + 3];

  int sr = (color >> 24) & 0xFF;
  int sg = (color >> 16) & 0xFF;
  int sb = (color >> 8) & 0xFF;
  int sa = color & 0xFF;

  int a = sa + da * (255 - sa) / 255;
  int r = (sr * sa / 255 + dr * da / 255 * (255 - sa) / 255);
  int g = (sg * sa / 255 + dg * da / 255 * (255 - sa) / 255);
  int b = (sb * sa / 255 + db * da / 255 * (255 - sa) / 255);

  if (a > 0)
  {
    r = r * 255 / a;
    g = g * 255 / a;
    b = b * 255 / a;
  }

  data[i]     = (uint8_t)(r & 0xFF);
  data[i + 1] = (uint8_t)(g & 0xFF);
  data[i + 2] = (uint8_t)(b & 0xFF);
  data[i + 3] = (uint8_t)(a & 0xFF);
}

void bm_blend_8(bm_t* bm, int nx, int ny, uint32_t color, unsigned char* ndata, int nw, int nh)
{
  int ex = nx + nw;
  if (ex > bm->w) ex = bm->w;
  int ey = ny + nh;
  if (ey > bm->h) ey = bm->h;

  int nr = (color >> 24) & 0xFF;
  int ng = (color >> 16) & 0xFF;
  int nb = (color >> 8) & 0xFF;

  uint8_t* odata = bm->data;

  for (int y = ny; y < ey; y++)
  {
    for (int x = nx; x < ex; x++)
    {
      int ni = (y - ny) * nw + (x - nx); // new map index
      int oi = (y * bm->w + x) * 4;      // old map index

      int or = odata[oi];
      int og = odata[oi + 1];
      int ob = odata[oi + 2];
      int oa = odata[oi + 3];

      unsigned char na = ndata[ni];

      int a = na + oa * (255 - na) / 255;
      int r = nr * na / 255 + or *oa / 255 * (255 - na) / 255;
      int g = ng * na / 255 + og * oa / 255 * (255 - na) / 255;
      int b = nb * na / 255 + ob * oa / 255 * (255 - na) / 255;

      if (a > 0)
      {
        r = r * 255 / a;
        g = g * 255 / a;
        b = b * 255 / a;
      }

      odata[oi]     = (uint8_t)(r & 0xFF);
      odata[oi + 1] = (uint8_t)(g & 0xFF);
      odata[oi + 2] = (uint8_t)(b & 0xFF);
      odata[oi + 3] = (uint8_t)(a & 0xFF);
    }
  }
}

void bm_insert(bm_t* base, bm_t* src, int sx, int sy)
{
  if (sx < 0) sx = 0;
  if (sy < 0) sy = 0;

  int w = src->w;
  int h = src->h;

  if (sx + w > base->w) return; // w = base->w - sx;
  if (sy + h > base->h) return; // h = base->h - sy;

  if (w <= 0 || h <= 0) return;

  for (int y = sy; y < sy + src->h; y++)
  {
    int bi = (y * base->w + sx) * 4;
    int si = (y - sy) * src->w * 4;
    memcpy(base->data + bi, src->data + si, w * 4);
  }
}

// TODO maybe move this function elsewhere
void bm_insert_rgb(bm_t* base, uint8_t* src, int w, int h, int sx, int sy)
{
  int bx = sx + w;
  if (bx > base->w) bx = base->w;
  int by = sy + h;
  if (by > base->h) by = base->h;

  uint8_t* sdata = src;        // src data
  uint8_t* bdata = base->data; // base data

  for (int y = sy; y < by; y++)
  {
    for (int x = sx; x < bx; x++)
    {
      int si = ((y - sy) * w + (x - sx)) * 4; // src index
      int bi = (y * base->w + x) * 4;         // base index

      uint8_t r = sdata[si];
      uint8_t g = sdata[si + 1];
      uint8_t b = sdata[si + 2];

      bdata[bi]     = r;
      bdata[bi + 1] = g;
      bdata[bi + 2] = b;
    }
  }
}

void bm_blend_rgba(bm_t* bm, int nx, int ny, bm_t* nbm)
{
  int ex = nx + nbm->w;
  if (ex > bm->w) ex = bm->w;
  int ey = ny + nbm->h;
  if (ey > bm->h) ey = bm->h;

  uint8_t* odata = bm->data;
  uint8_t* ndata = nbm->data;

  for (int y = ny; y < ey; y++)
  {
    for (int x = nx; x < ex; x++)
    {
      int ni = ((y - ny) * nbm->w + (x - nx)) * 4; // new map index
      int oi = (y * bm->w + x) * 4;                // old map index

      int nr = ndata[ni];
      int ng = ndata[ni + 1];
      int nb = ndata[ni + 2];
      int na = ndata[ni + 3];

      int or = odata[oi];
      int og = odata[oi + 1];
      int ob = odata[oi + 2];
      int oa = odata[oi + 3];

      int a = na + oa * (255 - na) / 255;
      int r = nr * na / 255 + or *oa / 255 * (255 - na) / 255;
      int g = ng * na / 255 + og * oa / 255 * (255 - na) / 255;
      int b = nb * na / 255 + ob * oa / 255 * (255 - na) / 255;

      if (a > 0)
      {
        r = r * 255 / a;
        g = g * 255 / a;
        b = b * 255 / a;
      }

      odata[oi]     = (uint8_t)(r & 0xFF);
      odata[oi + 1] = (uint8_t)(g & 0xFF);
      odata[oi + 2] = (uint8_t)(b & 0xFF);
      odata[oi + 3] = (uint8_t)(a & 0xFF);
    }
  }
}

#endif
