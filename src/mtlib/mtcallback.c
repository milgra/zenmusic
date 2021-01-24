#ifndef mtcallback_h
#define mtcallback_h

#include "mtmap.c"

typedef struct _cb_t cb_t;
struct _cb_t
{
  void (*fp)(void* userdata, map_t* map);
  void* userdata;
};

cb_t* cb_new(void (*fp)(void*, map_t*), void* userdata);

#endif

#if __INCLUDE_LEVEL__ == 0

cb_t* cb_new(void (*fp)(void*, map_t*), void* userdata)
{
  cb_t* cb     = mem_calloc(sizeof(cb_t), "mtcallback", NULL, NULL);
  cb->fp       = fp;
  cb->userdata = userdata;

  return cb;
}

#endif
