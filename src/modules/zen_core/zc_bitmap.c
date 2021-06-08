#ifndef zc_bitmap_h
#define zc_bitmap_h

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
bm_t* bm_flip_y(bm_t* bm);
void  bm_describe(void* p, int level);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_memory.c"
#include <assert.h>
#include <string.h>

void bm_del(void* pointer)
{
  bm_t* bm = pointer;

  if (bm->data != NULL) mem_release(bm->data); // REL 1
}

bm_t* bm_new(int the_w, int the_h)
{
  bm_t* bm = mem_calloc(sizeof(bm_t), "zc_bitmap", bm_del, bm_describe); // REL 0

  bm->w = the_w;
  bm->h = the_h;

  bm->size = 4 * the_w * the_h;
  bm->data = mem_calloc(bm->size * sizeof(unsigned char), "uint8_t*", NULL, NULL); // REL 1

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

bm_t* bm_flip_y(bm_t* bm)
{
  bm_t* tmp = bm_new(bm->w, bm->h);
  for (int y = 0; y < bm->h; y++)
  {
    int src_y = bm->h - y - 1;
    memcpy(tmp->data + y * bm->w * 4, bm->data + src_y * bm->w * 4, bm->w * 4);
  }
  return tmp;
}

void bm_describe(void* p, int level)
{
  bm_t* bm = p;
  printf("width %i height %i size %u", bm->w, bm->h, bm->size);
}

#endif
