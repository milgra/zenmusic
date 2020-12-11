#ifndef text_h
#define text_h

#include "mtbitmap.c"
#include "mtstring.c"
#include "mtvector.c"

#include <stdint.h>

typedef enum _textalign_t
{
  TA_LEFT,
  TA_CENTER,
  TA_RIGHT,
  TA_JUSTIFY,
} textalign_t;

typedef enum _autosize_t
{
  AS_FIX,
  AS_AUTO,
} autosize_t;

typedef struct _textstyle_t
{
  char*       font;
  textalign_t align;
  autosize_t  autosize;
  char        multiline;

  float size;
  float margin;

  uint32_t textcolor;
  uint32_t backcolor;
} textstyle_t;

void text_init();

void text_render(
    mtstr_t*    text,
    mtvec_t*    metrics,
    textstyle_t style,
    bm_t*       bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtgraphics.c"
#include "mtmap.c"
#include "mtmath2.c"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

mtmap_t*       fonts  = NULL;
unsigned char* gbytes = NULL; // byte array for glyph baking
size_t         gcount = 0;    // byte array size for glyph baking

void text_init()
{
  fonts  = MNEW();
  gbytes = malloc(sizeof(unsigned char));
  gcount = 1;
}

void text_font_load(char* path)
{
  assert(fonts != NULL);

  struct stat filestat;
  char        succ = stat(path, &filestat);
  if (succ == 0 && filestat.st_size > 0)
  {
    FILE* file = fopen(path, "rb");

    if (file)
    {
      unsigned char* data = malloc((size_t)filestat.st_size);
      fread(data, (size_t)filestat.st_size, 1, file);

      stbtt_fontinfo font;
      stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));

      void* fontp = HEAP(font, "stbtt_fontinfo");
      MPUT(fonts, path, fontp);

      printf("Font loaded %s fonts in file %i\n", path, stbtt_GetNumberOfFonts(data));
    }
    else
      printf("cannot open font %s\n", path);
  }
  else
    printf("cannot open font %s\n", path);
}

void text_render(
    mtstr_t*    text,
    mtvec_t*    metrics,
    textstyle_t style,
    bm_t*       bitmap)
{

  mtgraphics_rect(bitmap, 0, 0, bitmap->w, bitmap->h, style.backcolor, 0);

  stbtt_fontinfo* font = MGET(fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(fonts, style.font);
    if (!font) return;
  }

  int   i, j, ascent, descent, linegap, advancey, baseline, cursorh;
  float scale, xpos = 2; // leave a little padding in case the character extends left

  scale = stbtt_ScaleForPixelHeight(font, style.size);
  stbtt_GetFontVMetrics(font, &ascent, &descent, &linegap);
  baseline = (int)(ascent * scale);
  advancey = ascent - descent + linegap;
  cursorh  = ascent - descent;

  // printf("rendering text %s scale %f ascent %i descent %i linegap %i baseline %i advancey %i\n", text, scale, ascent, descent, linegap, baseline, advancey);

  for (int index = 0; index < text->length; index++)
  {
    int cp  = text->codepoints[index];
    int ncp = index < text->length - 1 ? text->codepoints[index + 1] : 0;

    int   lsb;      // left side bearing, from current pos to the left edge of the glyph
    int   advancex; // advance width from current pos to next pos
    int   x0, y0, x1, y1;
    float x_shift = xpos - (float)floor(xpos); // subpixel shift

    stbtt_GetCodepointHMetrics(font,
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

    stbtt_GetCodepointBitmapBoxSubpixel(font,
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

    int size = w * h;

    // increase glyph baking bitmap size if needed
    if (size > gcount)
    {
      gcount = size;
      gbytes = realloc(gbytes, gcount);
    }

    // don't write bitmap in case of empty glyphs ( space )
    if (w > 0 && h > 0)
    {
      stbtt_MakeCodepointBitmapSubpixel(font,
                                        gbytes,
                                        w,       // out widht
                                        h,       // out height
                                        w,       // out stride
                                        scale,   // scale x
                                        scale,   // scale y
                                        x_shift, // shift x
                                        0,       // shift y
                                        cp);

      bm_blend_8(bitmap, xpos + x0, baseline + y0, style.textcolor, gbytes, w, h);

      // printf("write glyph %c at xpos %f w %i h %i baseline(y) %i (x) %i x_shift %f\n", text[ch], xpos, x1 - x0, y1 - y0, baseline, (int)xpos + x0, x_shift);
    }

    // advance x axis
    xpos += (advancex * scale);

    // advance with kerning
    if (ncp > 0) xpos += scale * stbtt_GetCodepointKernAdvance(font, cp, ncp);
  }
}

void para_metrics(mtstr_t* text, r2_t rect, textstyle_t style, mtvec_t* metrics)
{
}

#endif
