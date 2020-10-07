#ifndef ui_rect_h
#define ui_rect_h

typedef struct _uir_t uir_t;
struct _uir_t
{
  float vertexes[24];
  char* id;
};

uir_t* uir_new(char* id,
               float x,
               float y,
               float w,
               float h,
               float tx,
               float ty,
               float tz,
               float tw);
void uir_del(void* rect);
void uir_desc(uir_t* rect);
void uir_set_dim(uir_t* rect, float x, float y, float w, float h);
void uir_set_tex(uir_t* rect, float tx, float ty, float tz, float tw);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstr.c"
#include "mtmem.c"

void uir_del(void* pointer)
{
  uir_t* rect = (uir_t*)pointer;
  REL(rect->id);
}

uir_t* uir_new(char* id,
               float x,
               float y,
               float w,
               float h,
               float tx,
               float ty,
               float tz,
               float tw)
{
  uir_t* rect = mtmem_calloc(sizeof(uir_t), uir_del);

  rect->id = mtcstr_fromcstring(id);

  rect->vertexes[0] = x;
  rect->vertexes[1] = y;
  rect->vertexes[2] = tx;
  rect->vertexes[3] = ty;

  rect->vertexes[4] = x + w;
  rect->vertexes[5] = y + h;
  rect->vertexes[6] = tz;
  rect->vertexes[7] = tw;

  rect->vertexes[8] = x;
  rect->vertexes[9] = y + h;
  rect->vertexes[10] = tx;
  rect->vertexes[11] = tw;

  rect->vertexes[12] = x + w;
  rect->vertexes[13] = y;
  rect->vertexes[14] = tz;
  rect->vertexes[15] = ty;

  rect->vertexes[16] = x;
  rect->vertexes[17] = y;
  rect->vertexes[18] = tx;
  rect->vertexes[19] = ty;

  rect->vertexes[20] = x + w;
  rect->vertexes[21] = y + h;
  rect->vertexes[22] = tz;
  rect->vertexes[23] = tw;

  return rect;
}

void uir_set_dim(uir_t* rect, float x, float y, float w, float h)
{
  rect->vertexes[0] = x;
  rect->vertexes[1] = y;

  rect->vertexes[4] = x + w;
  rect->vertexes[5] = y + h;

  rect->vertexes[8] = x;
  rect->vertexes[9] = y + h;

  rect->vertexes[12] = x + w;
  rect->vertexes[13] = y;

  rect->vertexes[16] = x;
  rect->vertexes[17] = y;

  rect->vertexes[20] = x + w;
  rect->vertexes[21] = y + h;
}

void uir_set_tex(uir_t* rect, float tx, float ty, float tz, float tw)
{
  rect->vertexes[2] = tx;
  rect->vertexes[3] = ty;

  rect->vertexes[6] = tz;
  rect->vertexes[7] = tw;

  rect->vertexes[10] = tx;
  rect->vertexes[11] = tw;

  rect->vertexes[14] = tz;
  rect->vertexes[15] = ty;

  rect->vertexes[18] = tx;
  rect->vertexes[19] = ty;

  rect->vertexes[22] = tz;
  rect->vertexes[23] = tw;
}

void uir_desc(uir_t* rect)
{
  for (int index = 0; index < 24; index++)
  {
    if (index % 4 == 0) printf("\n");
    printf("%f ", rect->vertexes[index]);
  }
  printf("\n");
}

#endif
