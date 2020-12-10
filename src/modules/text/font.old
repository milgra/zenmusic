
#ifndef font_h
#define font_h

#include "mtbitmap.c"
#include "mtstring.c"
#include "mtvector.c"
#include "stb_truetype.h"
#include "text.c"
#include <stdint.h>

typedef struct _font_t font_t;
struct _font_t
{
  stbtt_fontinfo info;
  uint8_t*       buffer;
  int            gap;
  int            ascent;
  int            descent;
};

font_t* font_alloc(char* the_font_path);
void    font_dealloc(void* the_font);

void font_render_ttext(
    mtstr_t*    ttext,
    mtvec_t*    metrics,
    textstyle_t style,
    font_t*     font,
    bm_t*       bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtmemory.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#define STB_TRUETYPE_IMPLEMENTATION // force following include to generate implementation
#include "stb_truetype.h"

/* alloc font */

font_t* font_alloc(char* font_path)
{
  struct stat filestat;
  char        success = stat(font_path, &filestat);

  if (success == 0 && filestat.st_size > 0)
  {
    FILE* font_file = fopen(font_path, "rb");

    if (font_file != NULL)
    {
      // read file

      font_t* font = mtmem_calloc(sizeof(font_t), "font_t", font_dealloc, NULL);
      font->buffer = mtmem_calloc((size_t)filestat.st_size, "uint8_t*", NULL, NULL);

      fread(font->buffer, (size_t)filestat.st_size, 1, font_file);

      // create font

      char success = stbtt_InitFont(&(font->info), font->buffer, stbtt_GetFontOffsetForIndex(font->buffer, 0));
      if (success == 1)
      {
        // get font_ascent font_descent and line font_gap

        stbtt_GetFontVMetrics(&(font->info), &(font->ascent), &(font->descent), &(font->gap));

        return font;
      }
      else
      {
        mtmem_release(font->buffer);
        mtmem_release(font);
      }

      fclose(font_file);
    }
  }

  return NULL;
}

/* dealloc font */

void font_dealloc(void* pointer)
{
  font_t* the_font = pointer;
  mtmem_release(the_font->buffer);
}

// render text into bitmap
void font_render_ttext(
    mtstr_t*    ttext,
    mtvec_t*    metrics,
    textstyle_t style,
    font_t*     font,
    bm_t*       bitmap)
{

  int   i, j, ascent, descent, linegap, advancey, baseline, cursorh, ch = 0;
  float scale, xpos = 2; // leave a little padding in case the character extends left

  scale = stbtt_ScaleForPixelHeight(&font->info, 15);
  stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
  baseline = (int)(ascent * scale);
  advancey = ascent - descent + linegap;
  cursorh  = ascent - descent;

  // printf("rendering text %s scale %f ascent %i descent %i linegap %i baseline %i advancey %i\n", text, scale, ascent, descent, linegap, baseline, advancey);

  for (int index = 0; index < ttext->length; index++)
  {
    int cp  = ttext->codepoints[index];
    int ncp = index < ttext->length - 1 ? ttext->codepoints[index + 1] : 0;

    int   lsb;      // left side bearing, from current pos to the left edge of the glyph
    int   advancex; // advance width from current pos to next pos
    int   x0, y0, x1, y1;
    float x_shift = xpos - (float)floor(xpos); // subpixel shift

    stbtt_GetCodepointHMetrics(&font->info,
                               cp,
                               &advancex, // advance from base x pos to next base pos
                               &lsb);     // left side bearing

    // increase xpos with left side bearing if first character in line
    if (xpos == 0 && lsb < 0)
    {
      xpos    = (float)-lsb * scale;
      x_shift = xpos - (float)floor(xpos); // subpixel shift
    }

    // printf("codepoint h metrics %c advance %i left side bearing %i\n", cp, advancex, lsb);

    stbtt_GetCodepointBitmapBoxSubpixel(&font->info,
                                        cp,
                                        scale,
                                        scale,
                                        x_shift, // x axis subpixel shift
                                        0,       // y axis subpixel shift
                                        &x0,     // left edge of the glyph from origin
                                        &y0,     // top edge of the glyph from origin
                                        &x1,     // right edge of the glyph from origin
                                        &y1);    // bottom edge of the glyph from origin

    // printf("bitmap subpixel %c x0 %i y0 %i x1 %i y1 %i\n", cp, x0, y0, x1, y1);

    int w = x1 - x0;
    int h = y1 - y0;

    // don't write bitmap in case of empty glyphs ( space )
    if (w > 0 && h > 0)
    {
      // TODO use one tmp bitmap for all glyphs
      unsigned char* tmpbmp = calloc(1, sizeof(unsigned char) * w * h);

      stbtt_MakeCodepointBitmapSubpixel(&font->info,
                                        tmpbmp,
                                        w,       // out widht
                                        h,       // out height
                                        w,       // out stride
                                        scale,   // scale x
                                        scale,   // scale y
                                        x_shift, // shift x
                                        0,       // shift y
                                        cp);

      bm_t* tmpbm = bm_new_from_grayscale(w, h, 0x00000000, 0x000000FF, tmpbmp);

      // TODO replace with blend_alpha with int calculations
      bm_insert_blend(bitmap, tmpbm, xpos + x0, baseline + y0);

      // cleanup
      free(tmpbmp);
      REL(tmpbm);

      // printf("write glyph %c at xpos %f w %i h %i baseline(y) %i (x) %i x_shift %f\n", text[ch], xpos, x1 - x0, y1 - y0, baseline, (int)xpos + x0, x_shift);
    }

    // advance x axis
    xpos += (advancex * scale);
    // advance with kerning
    if (ncp > 0)
      xpos += scale * stbtt_GetCodepointKernAdvance(&font->info, cp, ncp);
    ++ch;
  }
}

#endif
