#ifndef text_h
#define text_h

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
  textalign_t align;
  autosize_t  autosize;
  char        multiline;

  float size;
  float margin;

  uint32_t textcolor;
  uint32_t backcolor;
} textstyle_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"
#include "mtmath2.c"
#include "mtstring.c"

mtmap_t* fonts;

void text_init()
{
  fonts = MNEW();
}

void para_metrics(mtstr_t* text, r2_t rect, textstyle_t style, mtvec_t* metrics)
{
}

#endif
