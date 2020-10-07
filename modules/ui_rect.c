#ifndef ui_rect_h
#define ui_rect_h

typedef struct _uir_t uir_t;
struct _uir_t
{
  float vertexes[16];
  char* id;
};

uir_t* uir_new();
void uir_del(void* rect);

#endif

#if __INCLUDE_LEVEL__ == 0

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

  rect->vertexes[0] = x;
  rect->vertexes[1] = y;
  rect->vertexes[3] = tx;
  rect->vertexes[4] = ty;

  rect->vertexes[5] = x;
  rect->vertexes[6] = y + h;
  rect->vertexes[7] = tx;
  rect->vertexes[8] = tw;

  rect->vertexes[9] = x + w;
  rect->vertexes[10] = y + h;
  rect->vertexes[11] = tz;
  rect->vertexes[12] = tw;

  rect->vertexes[13] = x + w;
  rect->vertexes[14] = y + h;
  rect->vertexes[15] = tz;
  rect->vertexes[16] = tw;

  rect->vertexes[17] = x + w;
  rect->vertexes[18] = y + h;
  rect->vertexes[19] = tz;
  rect->vertexes[20] = tw;

  rect->vertexes[21] = x + w;
  rect->vertexes[22] = y;
  rect->vertexes[23] = tz;
  rect->vertexes[24] = ty;
}

void uir_set_dim(uir_t* rect, float x, float y, float w, float h)
{
  rect->vertexes[0] = x;
  rect->vertexes[1] = y;

  rect->vertexes[5] = x;
  rect->vertexes[6] = y + h;

  rect->vertexes[9] = x + w;
  rect->vertexes[10] = y + h;

  rect->vertexes[13] = x + w;
  rect->vertexes[14] = y + h;

  rect->vertexes[17] = x + w;
  rect->vertexes[18] = y + h;

  rect->vertexes[21] = x + w;
  rect->vertexes[22] = y;
}

void uir_set_tex(uir_t* rect, float tx, float ty, float tz, float tw)
{
  rect->vertexes[3] = tx;
  rect->vertexes[4] = ty;

  rect->vertexes[7] = tx;
  rect->vertexes[8] = tw;

  rect->vertexes[11] = tz;
  rect->vertexes[12] = tw;

  rect->vertexes[15] = tz;
  rect->vertexes[16] = tw;

  rect->vertexes[19] = tz;
  rect->vertexes[20] = tw;

  rect->vertexes[23] = tz;
  rect->vertexes[24] = ty;
}

#endif
