
#ifndef paragraph_h
#define paragraph_h

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

typedef struct _ttextstyle_t
{
  textalign_t align;
  autosize_t  autosize;
  char        multiline;

  float size;
  float margin;

  uint32_t textcolor;
  uint32_t backcolor;
} ttextstyle_t;

typedef struct _paragraph_t paragraph_t;
struct _paragraph_t
{
};

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmath2.c"
#include "mtstring.c"

void para_metrics(mtstr_t* text, r2_t rect, ttextstyle_t style, mtvec_t* metrics)
{
}

#endif
