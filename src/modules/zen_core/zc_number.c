#ifndef zc_number_h
#define zc_number_h

#include <stdint.h>

typedef union
{
  float    floatv;
  int      intv;
  uint32_t uint32v;
} num_t;

num_t* num_new_float(float val);
num_t* num_new_int(int val);
num_t* num_new_uint32(uint32_t val);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_memory.c"

num_t* num_new_float(float val)
{
  num_t* res  = mem_calloc(sizeof(num_t), "num_t", NULL, NULL);
  res->floatv = val;
  return res;
}

num_t* num_new_int(int val)
{
  num_t* res = mem_calloc(sizeof(num_t), "num_t", NULL, NULL);
  res->intv  = val;
  return res;
}

num_t* num_new_uint32(uint32_t val)
{
  num_t* res   = mem_calloc(sizeof(num_t), "num_t", NULL, NULL);
  res->uint32v = val;
  return res;
}

#endif
