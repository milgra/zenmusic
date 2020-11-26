//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtvec_h
#define mtvec_h

#include "mtmemory.c"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VNEW() mtvec_alloc()
#define VADD(VEC, OBJ) mtvec_add(VEC, OBJ)
#define VREM(VEC, OBJ) mtvec_rem(VEC, OBJ)
#define VNXT(VEC) mtvec_next(VEC)

#define REL_VEC_ITEMS(X) mtvec_decrese_item_retcount(X)

typedef struct mtvec_t mtvec_t;
struct mtvec_t
{
  void**   data;
  void**   next;
  uint32_t pos;
  uint32_t length;
  uint32_t length_real;
};

mtvec_t* mtvec_alloc(void);
void     mtvec_dealloc(void* vector);
void     mtvec_reset(mtvec_t* vector);
void     mtvec_decrese_item_retcount(mtvec_t* vector);
void     mtvec_add(mtvec_t* vector, void* data);
void*    mtvec_next(mtvec_t* vector);
void     mtvec_addatindex(mtvec_t* vector, void* data, size_t index);
void     mtvec_addinvector(mtvec_t* mtvec_a, mtvec_t* mtvec_b);
void     mtvec_adduniquedata(mtvec_t* vector, void* data);
void     mtvec_adduniquedataatindex(mtvec_t* vector, void* data, size_t index);
void     mtvec_replaceatindex(mtvec_t* vector, void* data, size_t index);
char     mtvec_rem(mtvec_t* vector, void* data);
char     mtvec_rematindex(mtvec_t* vector, uint32_t index);
void     mtvec_reminrange(mtvec_t* vector, uint32_t start, uint32_t end);
void     mtvec_reminvector(mtvec_t* mtvec_a, mtvec_t* mtvec_b);
void     mtvec_reverse(mtvec_t* vector);
void*    mtvec_head(mtvec_t* vector);
void*    mtvec_tail(mtvec_t* vector);
uint32_t mtvec_indexofdata(mtvec_t* vector, void* data);
void     mtvec_sort(mtvec_t* vector, int (*comp)(void* left, void* right));

void mtvec_describe(void* p, int level);

#endif
#if __INCLUDE_LEVEL__ == 0

/* creates new vector */

mtvec_t* mtvec_alloc()
{
  mtvec_t* vector     = mtmem_calloc(sizeof(mtvec_t), "mtvec_t", mtvec_dealloc, mtvec_describe);
  vector->data        = mtmem_calloc(sizeof(void*) * 10, "void**", NULL, NULL);
  vector->pos         = 0;
  vector->length      = 0;
  vector->length_real = 10;
  return vector;
}

/* deletes vector */

void mtvec_dealloc(void* pointer)
{
  mtvec_t* vector = pointer;
  for (uint32_t index = 0; index < vector->length; index++)
    mtmem_release(vector->data[index]);
  mtmem_release(vector->data);
}

/* resets vector */

void mtvec_reset(mtvec_t* vector)
{
  for (uint32_t index = 0; index < vector->length; index++)
    mtmem_release(vector->data[index]);
  vector->length = 0;
}

// iterates through all items, resets position when ran out for next loop

void* mtvec_next(mtvec_t* vector)
{
  if (vector->pos < vector->length)
  {
    vector->pos += 1;
    return vector->data[vector->pos - 1];
  }
  else
  {
    vector->pos = 0;
    return NULL;
  }
}

/* decreases retain count of items. use when you add items inline and don't want to release every item
        one by one. Be careful with it, don't release them til dealloc!*/

void mtvec_decrese_item_retcount(mtvec_t* vector)
{
  for (uint32_t index = 0; index < vector->length; index++)
    mtmem_release(vector->data[index]);
}

/* expands storage */

void mtvec_expand(mtvec_t* vector)
{
  if (vector->length == vector->length_real)
  {
    vector->length_real += 10;
    vector->data = mtmem_realloc(vector->data, sizeof(void*) * vector->length_real);
  }
}

/* adds single data */

void mtvec_add(mtvec_t* vector, void* data)
{
  mtmem_retain(data);
  mtvec_expand(vector);
  vector->data[vector->length] = data;
  vector->length += 1;
}

/* adds data at given index */

void mtvec_addatindex(mtvec_t* vector, void* data, size_t index)
{
  if (index > vector->length) index = vector->length;
  mtmem_retain(data);
  mtvec_expand(vector);
  memmove(vector->data + index + 1, vector->data + index, (vector->length - index) * sizeof(void*));
  vector->data[index] = data;
  vector->length += 1;
}

/* adds all items in vector to vector */

void mtvec_addinvector(mtvec_t* mtvec_a, mtvec_t* mtvec_b)
{
  for (uint32_t index = 0; index < mtvec_b->length; index++)
    mtmem_retain(mtvec_b->data[index]);
  mtvec_a->length_real += mtvec_b->length_real;
  mtvec_a->data = mtmem_realloc(mtvec_a->data, sizeof(void*) * mtvec_a->length_real);
  memcpy(mtvec_a->data + mtvec_a->length, mtvec_b->data, mtvec_b->length * sizeof(void*));
  mtvec_a->length += mtvec_b->length;
}

/* adds single unique data */

void mtvec_adduniquedata(mtvec_t* vector, void* data)
{
  if (mtvec_indexofdata(vector, data) == UINT32_MAX) mtvec_add(vector, data);
}

/* adds single unique data at index */

void mtvec_adduniquedataatindex(mtvec_t* vector, void* data, size_t index)
{
  if (mtvec_indexofdata(vector, data) == UINT32_MAX) mtvec_addatindex(vector, data, index);
}

/* replaces data at given index */

void mtvec_replaceatindex(mtvec_t* vector, void* data, size_t index)
{
  mtmem_release(vector->data[index]);
  mtmem_retain(data);
  vector->data[index] = data;
}

/* removes single data, returns 1 if data is removed and released during removal */

char mtvec_rem(mtvec_t* vector, void* data)
{
  uint32_t index = mtvec_indexofdata(vector, data);
  if (index < UINT32_MAX)
  {
    mtvec_rematindex(vector, index);
    return 1;
  }
  return 0;
}

/* removes single data at index, returns 1 if data is removed and released during removal */

char mtvec_rematindex(mtvec_t* vector, uint32_t index)
{
  if (index < vector->length)
  {
    mtmem_release(vector->data[index]);
    memmove(vector->data + index, vector->data + index + 1, (vector->length - index - 1) * sizeof(void*));
    vector->length -= 1;
    return 1;
  }
  return 0;
}

/* removes data in range */

void mtvec_reminrange(mtvec_t* vector, uint32_t start, uint32_t end)
{
  for (uint32_t index = start; index < end; index++)
    mtmem_release(vector->data[index]);
  memmove(vector->data + start, vector->data + end + 1, (vector->length - end - 1) * sizeof(void*));
  vector->length -= end - start + 1;
}

/* removes data in vector */

void mtvec_reminvector(mtvec_t* mtvec_a, mtvec_t* mtvec_b)
{
  for (int index = 0; index < mtvec_b->length; index++)
  {
    mtvec_rem(mtvec_a, mtvec_b->data[index]);
  }
}

/* reverses item order */

void mtvec_reverse(mtvec_t* vector)
{
  int length = vector->length;
  for (int index = length - 1; index > -1; index--)
  {
    mtvec_add(vector, vector->data[index]);
  }
  mtvec_reminrange(vector, 0, length - 1);
}

/* returns head item of vector */

void* mtvec_head(mtvec_t* vector)
{
  if (vector->length > 0)
    return vector->data[0];
  else
    return NULL;
}

/* returns tail item of vector */

void* mtvec_tail(mtvec_t* vector)
{
  if (vector->length > 0)
    return vector->data[vector->length - 1];
  else
    return NULL;
}

/* returns index of data or UINT32_MAX if not found */

uint32_t mtvec_indexofdata(mtvec_t* vector, void* data)
{
  void**   actual = vector->data;
  uint32_t index  = 0;
  while (index < vector->length)
  {
    if (*actual == data) return index;
    index++;
    actual += 1;
  }
  return UINT32_MAX;
}

typedef struct _mtvn_t mtvn_t;
struct _mtvn_t
{
  void*   c; // content
  mtvn_t* l; // left
  mtvn_t* r; // right
};

// TODO use node pool

void mtvec_sort_ins(mtvn_t* node, void* data, int (*comp)(void* left, void* right))
{
  if (node->c == NULL)
  {
    node->c = data;
  }
  else
  {
    if (comp(data, node->c) < 0)
    {
      if (node->l == NULL) node->l = mtmem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
      mtvec_sort_ins(node->l, data, comp);
    }
    else
    {
      if (node->r == NULL) node->r = mtmem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
      mtvec_sort_ins(node->r, data, comp);
    }
  }
}

void mtvec_sort_ord(mtvn_t* node, mtvec_t* vector, int* index)
{
  if (node->l) mtvec_sort_ord(node->l, vector, index);
  vector->data[*index] = node->c;
  *index += 1;

  // cleanup node
  mtvn_t* right = node->r;
  REL(node);

  if (right) mtvec_sort_ord(right, vector, index);
}

void mtvec_sort(mtvec_t* vector, int (*comp)(void* left, void* right))
{
  mtvn_t* node = mtmem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
  for (int index = 0; index < vector->length; index++)
  {
    mtvec_sort_ins(node, vector->data[index], comp);
  }
  int index = 0;
  mtvec_sort_ord(node, vector, &index);
}

void mtvec_describe(void* pointer, int level)
{
  mtvec_t* vector = pointer;
  for (uint32_t index = 0; index < vector->length; index++)
  {
    mtmem_describe(vector->data[index], level + 1);
    printf("\n");
  }
}

#endif
