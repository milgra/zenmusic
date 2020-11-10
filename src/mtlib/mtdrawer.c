//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtdrawer_h
#define mtdrawer_h

#include "mtbitmap.c"
#include <math.h>
#include <stdint.h>

void mtdrawer_circle(bm_t* bitmap, float cx, float cy, float r, uint32_t c);
void mtdrawer_tile(bm_t* bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

void mtdrawer_circle(bm_t* bitmap, float cx, float cy, float r, uint32_t c)
{
  printf("CIRCLE %i %f %f %f\n", bitmap->w, cx, cy, r);
  float m = r;
  for (int x = 0; x < bitmap->w; x++)
  {
    for (int y = 0; y < bitmap->h; y++)
    {
      int     dx = cx - (float)x;
      int     dy = cy - (float)y;
      float   d  = sqrt(dx * dx + dy * dy);
      uint8_t a  = 255;
      if (d < m - 2.0)
      {
        if (d > m - 4.0)
        {
          float delta = d - 2.0 - m;
          a           = (uint8_t)(((2.0 - delta) / 2.0) * 255.0);
        }
        bm_fill(bitmap, x, y, x + 1, y + 1, (c & 0xFFFFFF00) | a);
      }
    }
  }
}

// tiled bitmap, mainly for testing opengl rendering
void mtdrawer_tile(bm_t* bitmap)
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

#endif
