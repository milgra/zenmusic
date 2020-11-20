//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtgraphics_h
#define mtgraphics_h

#include "mtbitmap.c"
#include <math.h>
#include <stdint.h>

void mtgraphics_circle(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c);
void mtgraphics_arc(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c, float as, float ae);
void mtgraphics_rounded_rect(bm_t* bitmap, int x, int y, int w, int h, int r, uint32_t c);
void mtgraphics_tile(bm_t* bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

void mtgraphics_circle(bm_t* bitmap, float cx, float cy, float r, float edge, uint32_t c)
{
  float m = r;
  for (int x = 0; x < bitmap->w; x++)
  {
    for (int y = 0; y < bitmap->h; y++)
    {
      float   dx = cx - (float)x;
      float   dy = cy - (float)y;
      float   d  = sqrt(dx * dx + dy * dy);
      uint8_t a  = 255;
      if (d < m + edge)
      {
        if (d > m)
        {
          float delta = m + edge - d;
          a           = (uint8_t)(delta / edge * 255.0);
        }
        bm_fill(bitmap, x, y, x + 1, y + 1, (c & 0xFFFFFF00) | a);
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
      float   dx = (float)x - cx;
      float   dy = (float)y - cy;
      float   d  = sqrt(dx * dx + dy * dy);
      uint8_t a  = 255;
      if (d < m + edge)
      {
        float r = atan2(dy, dx);
        if (r < 0) r += 6.28;
        if (r > as && r < ae)
        {
          if (d > m)
          {
            float delta = m + edge - d;
            a           = (uint8_t)(delta / edge * 255.0);
          }
          bm_fill(bitmap, x, y, x + 1, y + 1, (c & 0xFFFFFF00) | a);
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

void mtgraphics_rounded_rect(bm_t* bitmap, int x, int y, int w, int h, int r, uint32_t c)
{
  // draw corners
  mtgraphics_circle(bitmap, x + r, y + r, r, 1.0, c);
  mtgraphics_circle(bitmap, x + w - r - 1.0, y + r, r, 1.0, c);
  mtgraphics_circle(bitmap, x + r, y + h - r - 1.0, r, 1.0, c);
  mtgraphics_circle(bitmap, x + w - r - 1.0, y + h - r - 1.0, r, 1.0, c);
  bm_fill(bitmap, x + r, y, x + w - r + 1, y + h + 1, c);
  bm_fill(bitmap, x, y + r, x + w + 1, y + h - r + 1, c);
}

#endif
