#ifndef mtbm_h
#define mtbm_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _bm_t bm_t;
struct _bm_t
{
  int w;
  int h;

  uint8_t* data;
  uint32_t size;
};

bm_t* bm_new(int the_w, int the_h);
bm_t* bm_clone(bm_t* bm);
void  bm_reset(bm_t* bm);
void  bm_describe(void* p, int level);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtmemory.c"
#include <assert.h>
#include <string.h>

void bm_del(void* pointer)
{
  bm_t* bm = pointer;
  if (bm->data != NULL) mtmem_release(bm->data);
}

bm_t* bm_new(int the_w, int the_h)
{
  bm_t* bm = mtmem_calloc(sizeof(bm_t), "mtbitmap", bm_del, bm_describe);

  bm->w = the_w;
  bm->h = the_h;

  bm->size = 4 * the_w * the_h;
  bm->data = mtmem_calloc(bm->size * sizeof(unsigned char), "uint8_t*", NULL, NULL);

  return bm;
}

bm_t* bm_clone(bm_t* the_bm)
{
  bm_t* bm = bm_new(the_bm->w, the_bm->h);
  memcpy(bm->data, the_bm->data, the_bm->size);
  return bm;
}

void bm_reset(bm_t* bm)
{
  memset(bm->data, 0, bm->size);
}

void bm_describe(void* p, int level)
{
  bm_t* bm = p;
  printf("width %i height %i size %u", bm->w, bm->h, bm->size);
}

#endif
