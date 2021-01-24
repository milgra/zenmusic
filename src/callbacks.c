#ifndef mtcallbacks_h
#define mtcallbacks_h

#include "mtcallback.c"

void  callbacks_init();
void  callbacks_set(char* id, cb_t* cb);
cb_t* callbacks_get(char* id);
void  callbacks_call(char* id, map_t* map);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmap.c"

struct _callbacks_t
{
  map_t* data;
} callbacks = {0};

void callbacks_init()
{
  callbacks.data = MNEW();
}

void callbacks_set(char* id, cb_t* cb)
{
  MPUT(callbacks.data, id, cb);
}

cb_t* callbacks_get(char* id)
{
  return MGET(callbacks.data, id);
}

void callbacks_call(char* id, map_t* data)
{
  cb_t* cb = MGET(callbacks.data, id);
  (*cb->fp)(cb->userdata, data);
}

#endif
