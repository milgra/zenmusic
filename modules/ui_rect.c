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

uir_t*
uir_new(float x,
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
}

#endif
