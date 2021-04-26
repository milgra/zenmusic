#ifndef mtcallbacks_h
#define mtcallbacks_h

#include "mtcallback.c"

void   callbacks_init();
void   callbacks_set(char* id, cb_t* cb);
cb_t*  callbacks_get(char* id);
void   callbacks_call(char* id, void* data);
map_t* callbacks_get_data();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtlog.c"
#include "mtmap.c"

map_t* callbacks;

void callbacks_init()
{
  callbacks = MNEW();
}

void callbacks_set(char* id, cb_t* cb)
{
  MPUT(callbacks, id, cb);
}

cb_t* callbacks_get(char* id)
{
  return MGET(callbacks, id);
}

void callbacks_call(char* id, void* data)
{
  cb_t* cb = MGET(callbacks, id);
  if (cb)
    (*cb->fp)(cb->userdata, data);
  else
    LOG("ERROR callback %s doesn't exist", id);
}

map_t* callbacks_get_data()
{
  return callbacks;
}

#endif
