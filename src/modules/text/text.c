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

typedef enum _vertalign_t
{
  VA_CENTER,
  VA_TOP,
  VA_BOTTOM,
} vertalign_t;

typedef enum _autosize_t
{
  AS_FIX,
  AS_AUTO,
} autosize_t;

typedef struct _textstyle_t
{
  char*       font;
  textalign_t align;
  vertalign_t valign;
  autosize_t  autosize;
  char        multiline;

  float size;
  float margin;

  uint32_t textcolor;
  uint32_t backcolor;
} textstyle_t;

typedef struct _glyph_t
{
  int      x;
  int      y;
  int      w;
  int      h;
  float    x_scale;
  float    y_scale;
  float    x_shift;
  float    y_shift;
  uint32_t cp;
} glyph_t;

void text_init();

void text_render(
    str_t*      text,
    vec_t*      metrics,
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

map_t*         fonts  = NULL;
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

// breaks text into lines in given rect
// first step for further alignment
void text_break_glyphs(
    glyph_t*    glyphs,
    int         count,
    textstyle_t style,
    int         w,
    int         h)
{

  stbtt_fontinfo* font = MGET(fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(fonts, style.font);
    if (!font) return;
  }

  int   asc, desc, lgap;     // glyph ascent, descent, linegap
  int   lsb, advx;           // left side bearing, glyph advance
  int   basey, fonth, lineh; // baseline position, base height, line height
  float scale, xpos;         // font scale, position

  stbtt_GetFontVMetrics(font, &asc, &desc, &lgap);
  scale = stbtt_ScaleForPixelHeight(font, style.size);

  fonth = asc - desc;
  lineh = fonth + lgap;

  basey = (int)(asc * scale);

  for (int index = 0; index < count; index++)
  {
    glyph_t glyph = glyphs[index];

    int cp  = glyphs[index].cp;
    int ncp = index < count - 1 ? glyphs[index + 1].cp : 0;

    int   x0, y0, x1, y1;
    float x_shift = xpos - (float)floor(xpos); // subpixel shift

    stbtt_GetCodepointHMetrics(font,
                               cp,
                               &advx, // advance from base x pos to next base pos
                               &lsb); // left side bearing

    // increase xpos with left side bearing if first character in line
    if (xpos == 0 && lsb < 0)
    {
      xpos    = (float)-lsb * scale;
      x_shift = xpos - (float)floor(xpos); // subpixel shift
    }

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
      glyph.x       = xpos + x0;
      glyph.y       = basey + y0;
      glyph.w       = w;
      glyph.h       = h;
      glyph.x_scale = scale;
      glyph.y_scale = scale;
      glyph.x_shift = x_shift;
      glyph.y_shift = 0;
      glyph.cp      = cp;

      // printf("write glyph %c at xpos %f w %i h %i basey(y) %i (x) %i x_shift %f\n", text[ch], xpos, x1 - x0, y1 - y0, basey, (int)xpos + x0, x_shift);
    }
    else
    {
      glyph.x = xpos + x0;
      glyph.y = basey + y0;
      glyph.w = 0;
      glyph.h = 0;
    }

    // advance x axis
    xpos += (advx * scale);

    // advance with kerning
    if (ncp > 0) xpos += scale * stbtt_GetCodepointKernAdvance(font, cp, ncp);

    // line break
    if (cp == '\n' || cp == '\r')
    {
      xpos = 0;
      basey += lineh;
    }
  }
}

void text_align_glyphs(glyph_t*    glyphs,
                       int         count,
                       textstyle_t style,
                       int         w,
                       int         h)
{
  for (int i = 0; i < count; i++)
  {
    glyph_t g = glyphs[i];
    float   x = g.x;
    float   y = g.y;
    // get last glyph in row for row width
    float ex = x; // end x
    float rw = 0; // row width
    int   ri;     // row index
    int   sc;     // space count
    for (ri = i; ri < count; ri++)
    {
      glyph_t rg = glyphs[ri];
      if (rg.y != y)
      {
        rw = ex - x;
        break;
      }
      ex = rg.x + rg.w;
      if (rg.cp == ' ') sc += 1; // count spaces
    }
    // align row
    float s = 0; // space
    if (style.align == TA_RIGHT) s = (float)w - rw;
    if (style.align == TA_CENTER) s = ((float)w - rw) / 2.0; // space
    if (style.align == TA_JUSTIFY) s = ((float)w - rw) / sc; // space
    for (int ni = i; ni < ri; ni++) glyphs[ni].x += s;
    // jump to next row
    i = ri;
  }
}

void text_render_glyphs(glyph_t*    glyphs,
                        int         count,
                        textstyle_t style,
                        bm_t*       bitmap)
{
  gfx_rect(bitmap, 0, 0, bitmap->w, bitmap->h, style.backcolor, 0);

  // get or load font
  stbtt_fontinfo* font = MGET(fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(fonts, style.font);
    if (!font) return;
  }

  // draw glyphs
  for (int i = 0; i < count; i++)
  {
    glyph_t g = glyphs[i];

    // don't write bitmap in case of empty glyphs ( space )
    if (g.w > 0 && g.h > 0)
    {
      int size = g.w * g.h;

      // increase glyph baking bitmap size if needed
      if (size > gcount)
      {
        gcount = size;
        gbytes = realloc(gbytes, gcount);
      }

      stbtt_MakeCodepointBitmapSubpixel(font,
                                        gbytes,
                                        g.w,       // out widht
                                        g.h,       // out height
                                        g.w,       // out stride
                                        g.x_scale, // scale x
                                        g.y_scale, // scale y
                                        g.x_shift, // shift x
                                        g.y_shift, // shift y
                                        g.cp);

      gfx_blend_8(bitmap,
                  g.x,
                  g.y,
                  style.textcolor,
                  gbytes,
                  g.w,
                  g.h);
    }
  }
}

void text_render_a(
    str_t*      text,
    textstyle_t style,
    bm_t*       bitmap)
{
  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length);
  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];
  text_break_glyphs(glyphs, text->length, style, bitmap->w, bitmap->h);
  text_align_glyphs(glyphs, text->length, style, bitmap->w, bitmap->h);
  text_render_glyphs(glyphs, text->length, style, bitmap);
}

void text_render(
    str_t*      text,
    vec_t*      metrics,
    textstyle_t style,
    bm_t*       bitmap)
{

  gfx_rect(bitmap, 0, 0, bitmap->w, bitmap->h, style.backcolor, 0);

  stbtt_fontinfo* font = MGET(fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(fonts, style.font);
    if (!font) return;
  }

  int   ascent, descent, linegap, advancey, baseline, baseheight;
  float scale, xpos = 0; // leave a little padding in case the character extends left

  scale = stbtt_ScaleForPixelHeight(font, style.size);
  stbtt_GetFontVMetrics(font, &ascent, &descent, &linegap);
  baseheight = ascent - descent;

  baseline = (int)(ascent * scale);

  if (style.valign == VA_CENTER) baseline += (bitmap->h - (scale * baseheight)) / 2;

  advancey = ascent - descent + linegap;

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

      gfx_blend_8(bitmap, xpos + x0, baseline + y0, style.textcolor, gbytes, w, h);

      // printf("write glyph %c at xpos %f w %i h %i baseline(y) %i (x) %i x_shift %f\n", text[ch], xpos, x1 - x0, y1 - y0, baseline, (int)xpos + x0, x_shift);
    }

    // advance x axis
    xpos += (advancex * scale);

    // advance with kerning
    if (ncp > 0) xpos += scale * stbtt_GetCodepointKernAdvance(font, cp, ncp);
  }
}

#endif
