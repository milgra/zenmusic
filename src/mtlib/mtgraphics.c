//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtgraphics_h
#define mtgraphics_h

#include "mtbitmap.c"
#include <math.h>
#include <stdint.h>

void mtgraphics_circle(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c);
void mtgraphics_arc(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c, float as, float ae);
void mtgraphics_rounded_rect(bm_t* bitmap, int x, int y, int w, int h, int r, float edge, uint32_t c1, uint32_t c2);
void mtgraphics_tile(bm_t* bitmap);
void mtgraphics_arc_grad(bm_t*    bm,
                         float    cx,
                         float    cy,
                         float    d1,
                         float    d2,
                         float    a1,
                         float    a2,
                         uint32_t c1,
                         uint32_t c2);
void mtgraphics_grad_v(bm_t* bm, int sx, int sy, int w, int h, uint32_t c1, uint32_t c2);
void mtgraphics_grad_h(bm_t* bm, int sx, int sy, int w, int h, uint32_t c1, uint32_t c2);

#endif

#if __INCLUDE_LEVEL__ == 0

void mtgraphics_circle(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c)
{
  float m = r;
  for (int x = 0; x < bitmap->w; x++)
  {
    for (int y = 0; y < bitmap->h; y++)
    {
      int r = (c >> 24) & 0xFF;
      int g = (c >> 16) & 0xFF;
      int b = (c >> 8) & 0xFF;
      int a = (c >> 0) & 0xFF;

      float    dx = cx - (float)x;
      float    dy = cy - (float)y;
      float    d  = sqrt(dx * dx + dy * dy);
      float    sa = (float)(c & 0xFF) / 255.0;
      uint8_t  ra = a;
      uint32_t fi = 0;
      if (d < m)
      {
        if (d > m - edge)
        {
          float delta = m - d; // (edge - (d - (m - edge)));
          float ratio = delta / edge;
          ra          = (uint8_t)(ratio * sa * 255.0);
        }
        fi = (r << 24) | (g << 16) | (b << 8) | ra;
        bm_blend_pixel(bitmap, x, y, fi);
      }
    }
  }
}

void mtgraphics_arc(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c, float as, float ae)
{
  float m = r;
  for (int x = 0; x < bitmap->w; x++)
  {
    for (int y = 0; y < bitmap->h; y++)
    {
      float    dx = (float)x - cx;
      float    dy = (float)y - cy;
      float    d  = sqrt(dx * dx + dy * dy);
      float    sa = (float)(c & 0xFF) / 255.0;
      uint8_t  ra = c & 0xFF;
      uint32_t fi = 0;
      if (d < m)
      {
        float r = atan2(dy, dx);
        if (r < 0) r += 6.28;
        if (r > as && r < ae)
        {
          if (d > m - edge)
          {
            float delta = m - d; // (edge - (d - (m - edge)));
            float ratio = delta / edge;
            ra          = (uint8_t)(ratio * sa * 255.0);
          }
          bm_blend_pixel(bitmap, x, y, (c & 0xFFFFFF00) | ra);
        }
      }
    }
  }
}

void mtgraphics_arc_grad(bm_t*    bm,
                         float    cx,
                         float    cy,
                         float    d1,
                         float    d2,
                         float    a1,
                         float    a2,
                         uint32_t c1,
                         uint32_t c2)
{
  int sx = (int)cx - d2 - 1;
  int sy = (int)cy - d2 - 1;
  int ex = (int)cx + d2 + 1;
  int ey = (int)cy + d2 + 1;

  if (sx < 0) sx = 0;
  if (ex > bm->w) ex = bm->w - 1;
  if (sy < 0) sy = 0;
  if (ey > bm->h) ey = bm->h - 1;

  int r1 = (c1 >> 24) & 0xFF;
  int g1 = (c1 >> 16) & 0xFF;
  int b1 = (c1 >> 8) & 0xFF;
  int p1 = c1 & 0xFF;

  int r2 = (c2 >> 24) & 0xFF;
  int g2 = (c2 >> 16) & 0xFF;
  int b2 = (c2 >> 8) & 0xFF;
  int p2 = c2 & 0xFF;

  float ds = d2 - d1;
  float dr = (float)(r2 - r1) / ds;
  float dg = (float)(g2 - g1) / ds;
  float db = (float)(b2 - b1) / ds;
  float dp = (float)(p2 - p1) / ds;

  for (int x = sx; x < ex; x++)
  {
    for (int y = sy; y < ey; y++)
    {
      float dx = (float)x - cx;
      float dy = (float)y - cy;
      float di = sqrt(dx * dx + dy * dy); // distance from center

      if (di > d1 && di < d2) // we are between the two distances
      {
        float an = atan2(dy, dx);
        if (an < 0) an += 6.28;
        if (an > a1 && an < a2) // we are between the two angles
        {
          float di1 = di - d1;
          float di2 = d2 - di;

          float pr = 1.0; // alpha ratio

          if (di1 < 1.0) pr = di1;
          if (di2 < 1.0) pr = di2;

          int r = r1 + (int)(di1 * dr);
          int g = g1 + (int)(di1 * dg);
          int b = b1 + (int)(di1 * db);
          int p = (int)((float)(p1 + (int)(di1 * dp)) * pr);

          uint32_t fi = ((r & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (p & 0xFF);

          bm_blend_pixel(bm, x, y, fi);
        }
      }
    }
  }
}

// tiled bitmap, mainly for testing opengl rendering
void mtgraphics_tile(bm_t* bitmap)
{
  for (int col = 0; col < bitmap->w; col++)
  {
    for (int row = 0; row < bitmap->h; row++)
    {
      uint32_t index = row * bitmap->w + col;
      uint32_t color = (row % 2 == 0 && col % 2 == 1) ? 0xFFFFFFFF : 0x000000FF;
      bm_fill(bitmap, col, row, col + 1, row + 1, color);
    }
  }
}

void mtgraphics_grad_h(bm_t* bm, int sx, int sy, int w, int h, uint32_t c1, uint32_t c2)
{
  int r1 = (c1 >> 24) & 0xFF;
  int g1 = (c1 >> 16) & 0xFF;
  int b1 = (c1 >> 8) & 0xFF;
  int a1 = c1 & 0xFF;

  int r2 = (c2 >> 24) & 0xFF;
  int g2 = (c2 >> 16) & 0xFF;
  int b2 = (c2 >> 8) & 0xFF;
  int a2 = c2 & 0xFF;

  float dr = (float)(r2 - r1) / (float)w;
  float dg = (float)(g2 - g1) / (float)w;
  float db = (float)(b2 - b1) / (float)w;
  float da = (float)(a2 - a1) / (float)w;

  int ex = sx + w;
  if (ex > bm->w) ex = bm->w;
  int ey = sy + h;
  if (ey > bm->h) ey = bm->h;

  uint8_t* data = bm->data;

  for (int y = sy; y < ey; y++)
  {
    for (int x = sx; x < ex; x++)
    {
      int s = x - sx;
      int i = (y * bm->w + x) * 4;
      int r = r1 + (int)(s * dr);
      int g = g1 + (int)(s * dg);
      int b = b1 + (int)(s * db);
      int a = a1 + (int)(s * da);

      data[i]     = r;
      data[i + 1] = g;
      data[i + 2] = b;
      data[i + 3] = a;
    }
  }
}

void mtgraphics_grad_v(bm_t* bm, int sx, int sy, int w, int h, uint32_t c1, uint32_t c2)
{
  int r1 = (c1 >> 24) & 0xFF;
  int g1 = (c1 >> 16) & 0xFF;
  int b1 = (c1 >> 8) & 0xFF;
  int a1 = c1 & 0xFF;

  int r2 = (c2 >> 24) & 0xFF;
  int g2 = (c2 >> 16) & 0xFF;
  int b2 = (c2 >> 8) & 0xFF;
  int a2 = c2 & 0xFF;

  float dr = (float)(r2 - r1) / (float)h;
  float dg = (float)(g2 - g1) / (float)h;
  float db = (float)(b2 - b1) / (float)h;
  float da = (float)(a2 - a1) / (float)h;

  int ex = sx + w;
  if (ex > bm->w) ex = bm->w;
  int ey = sy + h;
  if (ey > bm->h) ey = bm->h;

  uint8_t* data = bm->data;

  for (int y = sy; y < ey; y++)
  {
    for (int x = sx; x < ex; x++)
    {
      int s = y - sy;
      int i = (y * bm->w + x) * 4;
      int r = r1 + (int)(s * dr);
      int g = g1 + (int)(s * dg);
      int b = b1 + (int)(s * db);
      int a = a1 + (int)(s * da);

      data[i]     = r;
      data[i + 1] = g;
      data[i + 2] = b;
      data[i + 3] = a;
    }
  }
}

void mtgraphics_rounded_rect(bm_t* bitmap, int x, int y, int w, int h, int r, float edge, uint32_t c1, uint32_t c2)
{
  float e = edge;

  if (r > 0)
  {
    mtgraphics_arc_grad(bitmap, x + r, y + r, r - e, r, 3.14, 3.14 * 3 / 2.0, c2, c2 & 0xFFFFFF00);
    mtgraphics_arc_grad(bitmap, x + w - r - 1, y + r, r - e, r, 3.14 * 3 / 2.0, 3.14 * 2, c2, c2 & 0xFFFFFF00);
    mtgraphics_arc_grad(bitmap, x + r, y + h - r - 1, r - e, r, 3.14 / 2.0, 3.14, c2, c2 & 0xFFFFFF00);
    mtgraphics_arc_grad(bitmap, x + w - r - 1, y + h - r - 1, r - e, r, 0, 3.14 / 2.0, c2, c2 & 0xFFFFFF00);

    mtgraphics_arc_grad(bitmap, x + r, y + r, 0, r - e + 1, 3.14, 3.14 * 3 / 2.0, c1, c1);
    mtgraphics_arc_grad(bitmap, x + w - r - 1, y + r, 0, r - e + 1, 3.14 * 3 / 2.0, 3.14 * 2, c1, c1);
    mtgraphics_arc_grad(bitmap, x + r, y + h - r - 1, 0, r - e + 1, 3.14 / 2.0, 3.14, c1, c1);
    mtgraphics_arc_grad(bitmap, x + w - r - 1, y + h - r - 1, 0, r - e + 1, 0, 3.14 / 2.0, c1, c1);
  }

  mtgraphics_grad_h(bitmap, x, y + r, e, h - r - r, c2 & 0xFFFFFF00, c2);             // left vertical grad
  mtgraphics_grad_h(bitmap, x + w - e, y + r, e - 1, h - r - r, c2, c2 & 0xFFFFFF00); // right vertical grad
  mtgraphics_grad_v(bitmap, x + r, y, w - r - r, e, c2 & 0xFFFFFF00, c2);             // top horizontal grad
  mtgraphics_grad_v(bitmap, x + r, y + h - e, w - r - r, e - 1, c2, c2 & 0xFFFFFF00); // bottom horizontal grad

  bm_fill(bitmap, x + e, y + r, x + w - e, y + h - r, c1); // center
  bm_fill(bitmap, x + r, y + e, x + w - r, y + h - e, c1); // center
}

#endif
