
#ifndef text_h
#define text_h

#include "math2.c"

#define kTextAlignLeft 0
#define kTextAlignCenter 1
#define kTextAlignRight 2
#define kTextAlignJustify 3

typedef struct _glyphmetrics_t
{
  float x;
  float y;

  float xoff;
  float yoff;
  float width;
  float kerning;

  uint32_t codepoint;
  uint32_t row;
  uint8_t  selected;

} glyphmetrics_t;

glyphmetrics_t glyphmetrics_init(
    uint32_t codepoint,
    float    width,
    float    height,
    int      xoff,
    int      yoff,
    int      kerning,
    uint8_t  selected);

typedef struct _textstyle_t
{
  char align;
  char editable;
  char selectable;
  char multiline;
  char autosize;
  char uppercase;

  float textsize;
  float marginsize;
  float cursorsize;

  uint32_t textcolor;
  uint32_t backcolor;

} textstyle_t;

void text_align(
    glyphmetrics_t* glyphs,
    textstyle_t     style,
    v2_t*           dimensions,
    uint32_t        count,
    int             asc,
    int             desc,
    int             gap);

typedef struct _textselection_t
{
  uint32_t startindex;
  uint32_t endindex;
  uint32_t textcolor;
  uint32_t backcolor;

} textselection_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtstr.c"

/* init glyph metrics */

glyphmetrics_t glyphmetrics_init(uint32_t codepoint, float width, float height, int xoff, int yoff, int kerning, uint8_t selected)
{
  glyphmetrics_t result =
      {
          .codepoint = codepoint,
          .width     = width,
          .xoff      = xoff,
          .yoff      = yoff,
          .kerning   = kerning,
          .selected  = selected};
  return result;
}

/* align text */

void text_align(glyphmetrics_t* glyphs, textstyle_t style, v2_t* dimensions, uint32_t count, int asc, int desc, int gap)
{
  float linewidth = style.marginsize;
  float wordwidth = 0.0;
  float realwidth[count + 1];

  uint32_t laststart = 0;

  int row    = 0;
  int spaces = 0;
  int spacecount[count + 1];

  /* arrange glyps into words and calculate row widths */

  for (uint32_t index = 0; index < count; index++)
  {
    glyphmetrics_t* glyph = &(glyphs[index + 1]);

    /* count spaces */

    if (glyph->codepoint == ' ') spaces++;

    /* check for new word */

    if (glyph->codepoint == ' ' || glyph->codepoint == '\n' || glyph->codepoint == '\r')
    {
      wordwidth = 0.0;
      laststart = index;
    }

    if (style.multiline == 1)
    {
      /* instant new line */

      if (glyph->codepoint == '\n' || glyph->codepoint == '\r')
      {
        realwidth[row]  = linewidth;
        spacecount[row] = spaces;
        laststart       = index;

        spaces    = 0;
        linewidth = style.marginsize;
        wordwidth = 0.0;

        row += 1;
      }

      /* forced new line */

      if (linewidth > dimensions->x - 2 * style.marginsize)
      {
        if (wordwidth > dimensions->x - 2 * style.marginsize)
        {
          realwidth[row] = linewidth;
          laststart      = index;
        }
        else
        {
          realwidth[row] = linewidth - wordwidth;
          // !!! TODO hosszu sor tordeles
          // index = laststart;
          laststart = index;
        }

        spacecount[row] = spaces;
        spaces          = 0;

        linewidth = style.marginsize;
        wordwidth = 0.0;

        row += 1;
      }
    }

    linewidth += glyph->xoff + glyph->kerning + glyph->width;
    wordwidth += glyph->xoff + glyph->kerning + glyph->width;

    glyph->row = row;
  }

  realwidth[row] = linewidth;

  if (style.autosize == 1 && style.multiline == 0) dimensions->x = linewidth;

  /* position glyphs */

  float xpos       = 0.0;
  float ypos       = 0.0;
  float spacewidth = 0.0;

  /* set starting metrics, glyphs should be at least 1 length */

  xpos = style.marginsize;
  ypos = style.marginsize + asc;

  if (style.align == 1) xpos = dimensions->x / 2.0;
  if (style.align == 2) xpos = dimensions->x - style.marginsize;

  glyphmetrics_t* glyph = &(glyphs[0]);
  glyph->x              = xpos;
  glyph->y              = ypos;

  row = -1;

  for (uint32_t index = 0; index < count; index++)
  {
    glyphmetrics_t* glyph = &(glyphs[index + 1]);

    if (row != glyph->row)
    {
      row  = glyph->row;
      xpos = style.marginsize;
      ypos = style.marginsize + asc + row * (asc - desc + gap);

      if (style.align == 1) xpos = (dimensions->x - realwidth[glyph->row]) / 2.0;
      if (style.align == 2) xpos = (dimensions->x - style.marginsize - realwidth[glyph->row]);

      spacewidth = (dimensions->x - 2 * style.marginsize - realwidth[glyph->row]) / spacecount[glyph->row];
      if (xpos < 0.0) xpos = 0.0;
    }

    if (xpos > 0.0) xpos += glyph->kerning + glyph->xoff;

    glyph->x = xpos;
    glyph->y = ypos;

    xpos += glyph->width;
    if (glyph->codepoint == ' ' && style.align == 3) xpos += spacewidth;

    // printf( "%c to %f %f space %f\n" , glyph->codepoint , glyph->x , glyph->y , spacewidth);
  }

  glyph    = &(glyphs[count + 1]);
  glyph->x = xpos;
  glyph->y = ypos;

  dimensions->y = ypos + -desc + style.marginsize;
}

#endif
