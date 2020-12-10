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
bm_t* bm_new_from_bm(bm_t* bm);
bm_t* bm_new_from_grayscale(int the_w, int the_h, uint32_t backcolor, uint32_t fontcolor, unsigned char* bm);
bm_t* bm_from3(bm_t*    bm,
               uint8_t* src);
bm_t* bm_fill(bm_t* bm, int the_sx, int the_sy, int the_ex, int the_ey, uint32_t color);
bm_t* bm_fill_rgb(bm_t* bm, int the_sx, int the_sy, int the_ex, int the_ey, uint32_t color);
void  bm_del(void* bm);
void  bm_reset(bm_t* bm);
void  bm_insert(bm_t* the_base, bm_t* bm, int the_x, int the_y);
void  bm_insert_rgb(bm_t* the_base, uint8_t* bm, int w, int h, int the_x, int the_y);
void  bm_insert_blend(bm_t* the_base, bm_t* bm, int the_x, int the_y);
void  bm_blend_pixel(bm_t* bm, int x, int y, uint32_t color);
void  bm_describe(void* p, int level);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"
#include <assert.h>
#include <string.h>

bm_t* bm_new(int the_w, int the_h)
{
  bm_t* bm = mtmem_calloc(sizeof(bm_t), "mtbitmap", bm_del, bm_describe);

  bm->w = the_w;
  bm->h = the_h;

  bm->size = 4 * the_w * the_h;
  bm->data = mtmem_calloc(bm->size * sizeof(unsigned char), "uint8_t*", NULL, NULL);

  return bm;
}

void bm_del(void* pointer)
{
  bm_t* bm = pointer;
  if (bm->data != NULL) mtmem_release(bm->data);
}

void bm_reset(bm_t* bm)
{
  memset(bm->data, 0, bm->size);
}

bm_t* bm_new_from_bm(bm_t* the_bm)
{
  bm_t* bm = bm_new(the_bm->w, the_bm->h);
  memcpy(bm->data, the_bm->data, the_bm->size);
  return bm;
}

bm_t* bm_from3(bm_t*    bm,
               uint8_t* src)
{
  for (int y = 0; y < bm->h; y++)
  {
    for (int x = 0; x < bm->w; x++)
    {
      int sposition = (y * bm->w + x) * 3;
      int tposition = (y * bm->w + x) * 4;

      bm->data[tposition]     = src[sposition];
      bm->data[tposition + 1] = src[sposition + 1];
      bm->data[tposition + 2] = src[sposition + 2];
      bm->data[tposition + 3] = 0xFF;
    }
  }

  return bm;
}

bm_t* bm_new_from_grayscale(int            the_w,
                            int            the_h,
                            uint32_t       bc, // background
                            uint32_t       fc, // frontcolor
                            unsigned char* bm)
{
  if (the_w == 0 || the_h == 0) return NULL;

  float br = (float)(bc >> 24 & 0xFF);
  float bg = (float)(bc >> 16 & 0xFF);
  float bb = (float)(bc >> 8 & 0xFF);
  float ba = (float)(bc & 0xFF);

  float fr = (float)(fc >> 24 & 0xFF);
  float fg = (float)(fc >> 16 & 0xFF);
  float fb = (float)(fc >> 8 & 0xFF);
  float fa = (float)(fc & 0xFF);

  bm_t* nbm = bm_new(the_w, the_h);

  for (int i = 0; i < the_w * the_h; i++)
  {
    float ratio          = (float)bm[i] / 255.0;
    nbm->data[i * 4]     = (int)(fr * ratio + br * (1.0 - ratio));
    nbm->data[i * 4 + 1] = (int)(fg * ratio + bg * (1.0 - ratio));
    nbm->data[i * 4 + 2] = (int)(fb * ratio + bb * (1.0 - ratio));
    nbm->data[i * 4 + 3] = (int)(fa * ratio + ba * (1.0 - ratio));
  }

  return nbm;
}

bm_t* bm_fill(bm_t*    bm,
              int      sx,
              int      sy,
              int      ex,
              int      ey,
              uint32_t color)
{
  if (ex < sx) return bm;
  if (ey < sy) return bm;
  if (sx < 0) sx = 0;
  if (sy < 0) sy = 0;
  if (ex >= bm->w) ex = bm->w;
  if (ey >= bm->h) ey = bm->h;

  int r = color >> 24 & 0xFF;
  int g = color >> 16 & 0xFF;
  int b = color >> 8 & 0xFF;
  int a = color & 0xFF;

  for (int y = sy; y < ey; y++)
  {
    for (int x = sx; x < ex; x++)
    {
      int position = (y * bm->w + x) * 4;

      bm->data[position]     = r;
      bm->data[position + 1] = g;
      bm->data[position + 2] = b;
      bm->data[position + 3] = a;
    }
  }

  return bm;
}

bm_t* bm_fill_rgb(bm_t*    bm,
                  int      sx,
                  int      sy,
                  int      ex,
                  int      ey,
                  uint32_t color)
{
  if (ex < sx) return bm;
  if (ey < sy) return bm;
  if (sx < 0) sx = 0;
  if (sy < 0) sy = 0;
  if (ex >= bm->w) ex = bm->w;
  if (ey >= bm->h) ey = bm->h;

  int r = color >> 24 & 0xFF;
  int g = color >> 16 & 0xFF;
  int b = color >> 8 & 0xFF;

  for (int y = sy; y < ey; y++)
  {
    for (int x = sx; x < ex; x++)
    {
      int position = (y * bm->w + x) * 4;

      bm->data[position]     = r;
      bm->data[position + 1] = g;
      bm->data[position + 2] = b;
    }
  }

  return bm;
}

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

void bm_insert_blend(bm_t* base, bm_t* src, int sx, int sy)
{
  assert(base->w > 0);
  assert(base->h > 0);
  assert(src->w > 0);
  assert(src->h > 0);
  int bx = sx + src->w;
  if (bx > base->w) bx = base->w;
  int by = sy + src->h;
  if (by > base->h) by = base->h;

  uint8_t* sdata = src->data;  // src data
  uint8_t* bdata = base->data; // base data

  for (int y = sy; y < by; y++)
  {
    for (int x = sx; x < bx; x++)
    {
      int si = ((y - sy) * src->w + (x - sx)) * 4; // src index
      int bi = (y * base->w + x) * 4;              // base index

      float srcR = (float)sdata[si] / 255.0;
      float srcG = (float)sdata[si + 1] / 255.0;
      float srcB = (float)sdata[si + 2] / 255.0;
      float srcA = (float)sdata[si + 3] / 255.0;

      float dstR = (float)bdata[bi] / 255.0;
      float dstG = (float)bdata[bi + 1] / 255.0;
      float dstB = (float)bdata[bi + 2] / 255.0;
      float dstA = (float)bdata[bi + 3] / 255.0;

      float outA = srcA + dstA * (1 - srcA);
      float outR = (srcR * srcA + dstR * dstA * (1 - srcA)) / outA;
      float outG = (srcG * srcA + dstG * dstA * (1 - srcA)) / outA;
      float outB = (srcB * srcA + dstB * dstA * (1 - srcA)) / outA;

      bdata[bi]     = (int)(outR * 255.0);
      bdata[bi + 1] = (int)(outG * 255.0);
      bdata[bi + 2] = (int)(outB * 255.0);
      bdata[bi + 3] = (int)(outA * 255.0);
    }
  }
}

void bm_describe(void* p, int level)
{
  bm_t* bm = p;
  printf("width %i height %i size %u", bm->w, bm->h, bm->size);
}

#endif
