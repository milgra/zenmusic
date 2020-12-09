
#ifndef font_h
#define font_h

#include "mtbitmap.c"
#include "mtstring.c"
#include "mtvector.c"
#include "paragraph.c"
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
bm_t*   font_render_text(int width, int height, mtstr_t* string, font_t* the_font, textstyle_t text, glyphmetrics_t* glyphmetrics, mtvec_t* selections);
bm_t*   font_render_glyph(font_t* the_font, uint32_t codepoint, float textsize, uint32_t textcolor, uint32_t backcolor, int* xoff, int* yoff);
int     font_kerning(font_t* the_font, float textsize, uint32_t codepoint_a, uint32_t codepoint_b);

bm_t* font_render_ttext(
    mtstr_t*     ttext,
    mtvec_t*     metrics,
    ttextstyle_t style,
    font_t*      font,
    bm_t*        bitmap);

int font_asc(font_t* the_font, float textsize);
int font_desc(font_t* the_font, float textsize);
int font_lineheight(font_t* the_font, float textsize);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtmemory.c"
#include "paragraph.c"
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

int font_asc(font_t* the_font, float textsize)
{
  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), textsize);
  float asc   = the_font->ascent * scale;
  return (int)roundf(asc);
}

int font_desc(font_t* the_font, float textsize)
{
  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), textsize);
  float desc  = the_font->descent * scale;
  return (int)roundf(desc);
}

int font_lineheight(font_t* the_font, float textsize)
{
  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), textsize);
  float desc  = the_font->descent * scale;
  float asc   = the_font->ascent * scale;
  float gap   = the_font->gap * scale;

  return (int)roundf(asc - desc + gap);
}

int font_kerning(
    font_t*  the_font,
    float    textsize,
    uint32_t codepoint_a,
    uint32_t codepoint_b)
{
  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), textsize);

  int result = stbtt_GetCodepointKernAdvance(
      &(the_font->info),
      codepoint_a,
      codepoint_b);

  return (int)roundf(scale * (float)result);
}

bm_t* font_render_glyph(
    font_t*  the_font,
    uint32_t codepoint,
    float    textsize,
    uint32_t textcolor,
    uint32_t backcolor,
    int*     xoff,
    int*     yoff)
{

  /* generate raw font bitmap */

  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), textsize);
  float asc   = the_font->ascent * scale;

  int gw;
  int gh;

  uint8_t* rawmap = stbtt_GetCodepointBitmap(
      &(the_font->info),
      scale,
      scale,
      codepoint,
      &gw,
      &gh,
      xoff,
      yoff);

  bm_t* result = NULL;

  if (gw > 0 && gh > 0)
  {

    result = bm_new_from_grayscale(
        gw,
        gh,
        backcolor & 0xFFFFFF00,
        textcolor,
        rawmap);
  }
  else
  {
  }

  *yoff += asc;

  if (rawmap != NULL) free(rawmap);

  return result;
}

char          buffer[24 << 20];
unsigned char screen[20][79];

// render text into bitmap
bm_t* font_render_ttext(
    mtstr_t*     ttext,
    mtvec_t*     metrics,
    ttextstyle_t style,
    font_t*      font,
    bm_t*        bitmap)
{

  int   i, j, ascent, baseline, ch = 0;
  float scale, xpos = 2;       // leave a little padding in case the character extends left
  char* text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

  scale = stbtt_ScaleForPixelHeight(&font->info, 15);
  stbtt_GetFontVMetrics(&font->info, &ascent, 0, 0);
  baseline = (int)(ascent * scale);

  printf("rendering text %s scale %f baseline %i\n", text, scale, baseline);

  while (text[ch])
  {
    int   lsb;     // left side bearing, from current pos to the left edge of the glyph
    int   advance; // advance width from current pos to next pos
    int   x0, y0, x1, y1;
    float x_shift = xpos - (float)floor(xpos);

    stbtt_GetCodepointHMetrics(&font->info,
                               text[ch],
                               &advance,
                               &lsb);

    printf("codepoint h metrics %c advance %i left side bearing %i\n", text[ch], advance, lsb);

    stbtt_GetCodepointBitmapBoxSubpixel(&font->info,
                                        text[ch],
                                        scale,
                                        scale,
                                        x_shift,
                                        0,
                                        &x0,
                                        &y0,
                                        &x1,
                                        &y1);

    printf("bitmap subpixel %i x0 %i y0 %i x1 %i y1 %i\n", text[ch], x0, y0, x1, y1);

    stbtt_MakeCodepointBitmapSubpixel(&font->info,
                                      &screen[baseline + y0][(int)xpos + x0],
                                      x1 - x0, // out widht
                                      y1 - y0, // out height
                                      79,      // out stride
                                      scale,   // scale x
                                      scale,   // scale y
                                      x_shift, // shift x
                                      0,       // shift y
                                      text[ch]);

    printf("make subpixel at xpos %f array indexes %i %i x_shift %f\n", xpos, (baseline + y0), (int)xpos + x0, x_shift);

    // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
    // because this API is really for baking character bitmaps into textures. if you want to render
    // a sequence of characters, you really need to render each bitmap to a temp buffer, then
    // "alpha blend" that into the working buffer

    xpos += (advance * scale);
    if (text[ch + 1])
      xpos += scale * stbtt_GetCodepointKernAdvance(&font->info, text[ch], text[ch + 1]);
    ++ch;
  }

  bm_t* bm = bm_new_from_grayscale(79, 20, 0x0F0000FF, 0xFFFFFFFF, &screen);

  return bm;
}

/* render text TODO !!! CLEANUP, REFACTOR */

bm_t* font_render_text(int width, int height, mtstr_t* string, font_t* the_font, textstyle_t text, glyphmetrics_t* glyphmetrics, mtvec_t* selections)
{
  /* get font metrics */

  float scale = stbtt_ScaleForPixelHeight(&(the_font->info), text.textsize);
  float desc  = the_font->descent * scale;
  float asc   = the_font->ascent * scale;
  float gap   = the_font->gap * scale;

  /* return empty bitmap in case of no text */

  v2_t            dimensions = v2_init(width, asc + -desc);
  glyphmetrics_t* metrics    = glyphmetrics;

  if (glyphmetrics == NULL)
  {
    metrics = mtmem_calloc(sizeof(glyphmetrics_t) * (string == NULL ? 2 : string->length + 2), "glyphmetrics", NULL, NULL);
  }

  if (string == NULL || string->length == 0)
  {
    /* align empty text to init metrics */

    text_align(metrics, text, &dimensions, string == NULL ? 0 : string->length, asc, desc, gap);

    int xoff = 0;
    int yoff = 0;
    if (dimensions.x < width) xoff = (width - dimensions.x) / 2;
    if (dimensions.y < height) yoff = (height - dimensions.y) / 2;
    if (text.autosize == 1 && text.multiline == 1) yoff = 0;

    metrics[0].x += xoff;
    metrics[0].y += yoff;

    /* create empty bitmap */

    bm_t* result = bm_new(width, height);
    bm_fill(result, 0, 0, result->w, result->h, text.backcolor);

    if (glyphmetrics == NULL) mtmem_release(metrics);
    return result;
  }

  /* bitmap array with maximum possible characters */

  bm_t* bitmaps[string->length];

  /* generate glyphs and get metrics */

  for (int index = 0; index < string->length; index++)
  {
    int gw, gh, gxo, gyo, kern;

    /* default font colorts */

    uint32_t backcolor = text.backcolor & 0xFFFFFF00;
    uint32_t textcolor = text.textcolor;
    uint8_t  selected  = 0;

    /* if given index is selected, use selection colors as font colors */

    if (selections != NULL && selections->length > 0)
    {
      for (int si = 0; si < selections->length; si++)
      {
        textselection_t* selection = selections->data[si];
        if (selection->startindex <= index && selection->endindex > index)
        {
          //backcolor = selection->backcolor;
          textcolor = selection->textcolor;
          selected  = 1;
        }
      }
    }

    /* generate raw font bitmap */

    uint8_t* rawmap = stbtt_GetCodepointBitmap(&(the_font->info), scale, scale, string->codepoints[index], &gw, &gh, &gxo, &gyo);

    /* get kerning */

    if (index > 0)
    {
      kern = scale * stbtt_GetCodepointKernAdvance(&(the_font->info), string->codepoints[index - 1], string->codepoints[index]);
    }
    else
      kern = 0;

    /* space needs plus width */

    if (string->codepoints[index] == 32)
    {
      int aw, db;
      stbtt_GetCodepointHMetrics(&(the_font->info), string->codepoints[index], &aw, &db);
      gw = (aw + db) * scale;
    }

    /* make carriage return and line feed invisible */

    if (string->codepoints[index] == 10 || string->codepoints[index] == 13) memset(rawmap, 0, gw * gh);

    /* generate and store final bitmap */

    bm_t* bitmap;
    if (gw > 0 && gh > 0)
    {
      bitmap = bm_new_from_grayscale(gw, gh, backcolor & 0xFFFFFF00, textcolor, rawmap);
      free(rawmap);
    }
    else
    {
      bitmap = bm_new(1, 1);
    }

    bitmaps[index] = bitmap;

    /* store metrics at index + 1 because 0 is a 0 width char */

    metrics[index + 1] = glyphmetrics_init(string->codepoints[index], gw, gh, gxo, gyo, (float)kern * scale, selected);
  }

  /* align text */

  text_align(metrics, text, &dimensions, string->length, asc, desc, gap);

  /* draw all glyphs, create the base bitmap first */

  bm_t* result = bm_new(width, height);

  bm_fill(result, 0, 0, result->w, result->h, text.backcolor);

  int xoff = 0;
  int yoff = 0;

  /* center horizontally and vertically */

  if (dimensions.x < width) xoff = (width - dimensions.x) / 2;
  if (dimensions.y < height) yoff = (height - dimensions.y) / 2;

  metrics[0].x += xoff;
  metrics[0].y += yoff;

  int prevx = metrics[0].x;
  int prevy = metrics[0].y;
  int prevr = -1;

  for (int index = 0; index < string->length; index++)
  {
    bm_t*           bitmap = bitmaps[index];
    glyphmetrics_t* glyph  = &metrics[index + 1];

    glyph->x += xoff;
    glyph->y += yoff;

    if (glyph->selected == 1)
    {
      if (glyph->row != prevr)
      {
        prevx = glyph->x;
        prevy = glyph->y;
      }
      bm_fill(result, prevx, prevy - asc, glyph->x + glyph->width, glyph->y - desc, 0xBBBBBBFF);
    }

    if (bitmap->data != NULL) bm_insert_blend(result, bitmap, glyph->x, glyph->y + glyph->yoff);

    prevx = glyph->x + glyph->width;
    prevy = glyph->y;
    prevr = glyph->row;

    mtmem_release(bitmap);
  }

  if (glyphmetrics == NULL) mtmem_release(metrics);

  return result;
}

#endif
