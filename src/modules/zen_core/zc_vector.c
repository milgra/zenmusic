#ifndef mtvec_h
#define mtvec_h

#include "zc_memory.c"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VNEW() vec_alloc()
#define VADD(VEC, OBJ) vec_add(VEC, OBJ)
#define VADDR(VEC, OBJ) vec_add_rel(VEC, OBJ)
#define VREM(VEC, OBJ) vec_rem(VEC, OBJ)
#define VNXT(VEC) vec_next(VEC)

#define REL_VEC_ITEMS(X) vec_decrese_item_retcount(X)

typedef enum _vsdir_t
{
  VSD_ASC,
  VSD_DSC
} vsdir_t;

typedef struct _vec_t vec_t;
struct _vec_t
{
  void**   data;
  void**   next;
  uint32_t pos;
  uint32_t length;
  uint32_t length_real;
};

vec_t*   vec_alloc(void);
void     vec_dealloc(void* vector);
void     vec_reset(vec_t* vector);
void     vec_dec_retcount(vec_t* vector);
void     vec_add(vec_t* vector, void* data);
void     vec_add_rel(vec_t* vector, void* data);
void     vec_ins(vec_t* vector, void* data, size_t index);
void*    vec_next(vec_t* vector);
void     vec_addinvector(vec_t* vec_a, vec_t* vec_b);
void     vec_adduniquedata(vec_t* vector, void* data);
void     vec_adduniquedataatindex(vec_t* vector, void* data, size_t index);
void     vec_replaceatindex(vec_t* vector, void* data, size_t index);
char     vec_rem(vec_t* vector, void* data);
char     vec_rematindex(vec_t* vector, uint32_t index);
void     vec_reminrange(vec_t* vector, uint32_t start, uint32_t end);
void     vec_reminvector(vec_t* vec_a, vec_t* vec_b);
void     vec_reverse(vec_t* vector);
void*    vec_head(vec_t* vector);
void*    vec_tail(vec_t* vector);
uint32_t vec_indexofdata(vec_t* vector, void* data);
void     vec_sort(vec_t* vector, vsdir_t dir, int (*comp)(void* left, void* right));

void vec_describe(void* p, int level);

#endif
#if __INCLUDE_LEVEL__ == 0

/* creates new vector */

vec_t* vec_alloc()
{
  vec_t* vector       = mem_calloc(sizeof(vec_t), "vec_t", vec_dealloc, vec_describe);
  vector->data        = mem_calloc(sizeof(void*) * 10, "void**", NULL, NULL);
  vector->pos         = 0;
  vector->length      = 0;
  vector->length_real = 10;
  return vector;
}

/* deletes vector */

void vec_dealloc(void* pointer)
{
  vec_t* vector = pointer;
  for (uint32_t index = 0; index < vector->length; index++)
    mem_release(vector->data[index]);
  mem_release(vector->data);
}

/* resets vector */

void vec_reset(vec_t* vector)
{
  for (uint32_t index = 0; index < vector->length; index++)
    mem_release(vector->data[index]);
  vector->length = 0;
}

// iterates through all items, resets position when ran out for next loop

void* vec_next(vec_t* vector)
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

void vec_dec_retcount(vec_t* vector)
{
  for (uint32_t index = 0; index < vector->length; index++)
    mem_release(vector->data[index]);
}

/* expands storage */

void vec_expand(vec_t* vector)
{
  if (vector->length == vector->length_real)
  {
    vector->length_real += 10;
    vector->data = mem_realloc(vector->data, sizeof(void*) * vector->length_real);
  }
}

/* adds single data */

void vec_add(vec_t* vector, void* data)
{
  mem_retain(data);
  vec_expand(vector);
  vector->data[vector->length] = data;
  vector->length += 1;
}

/* adds and releases single data, for inline use */

void vec_add_rel(vec_t* vector, void* data)
{
  vec_add(vector, data);
  REL(data);
}

/* adds data at given index */

void vec_ins(vec_t* vector, void* data, size_t index)
{
  if (index > vector->length) index = vector->length;
  mem_retain(data);
  vec_expand(vector);
  memmove(vector->data + index + 1, vector->data + index, (vector->length - index) * sizeof(void*));
  vector->data[index] = data;
  vector->length += 1;
}

/* adds all items in vector to vector */

void vec_addinvector(vec_t* vec_a, vec_t* vec_b)
{
  for (uint32_t index = 0; index < vec_b->length; index++)
    mem_retain(vec_b->data[index]);
  vec_a->length_real += vec_b->length_real;
  vec_a->data = mem_realloc(vec_a->data, sizeof(void*) * vec_a->length_real);
  memcpy(vec_a->data + vec_a->length, vec_b->data, vec_b->length * sizeof(void*));
  vec_a->length += vec_b->length;
}

/* adds single unique data */

void vec_adduniquedata(vec_t* vector, void* data)
{
  if (vec_indexofdata(vector, data) == UINT32_MAX) vec_add(vector, data);
}

/* adds single unique data at index */

void vec_adduniquedataatindex(vec_t* vector, void* data, size_t index)
{
  if (vec_indexofdata(vector, data) == UINT32_MAX) vec_ins(vector, data, index);
}

/* replaces data at given index */

void vec_replaceatindex(vec_t* vector, void* data, size_t index)
{
  mem_release(vector->data[index]);
  mem_retain(data);
  vector->data[index] = data;
}

/* removes single data, returns 1 if data is removed and released during removal */

char vec_rem(vec_t* vector, void* data)
{
  uint32_t index = vec_indexofdata(vector, data);
  if (index < UINT32_MAX)
  {
    vec_rematindex(vector, index);
    return 1;
  }
  return 0;
}

/* removes single data at index, returns 1 if data is removed and released during removal */

char vec_rematindex(vec_t* vector, uint32_t index)
{
  if (index < vector->length)
  {
    mem_release(vector->data[index]);

    if (index < vector->length - 1)
    {
      // have to shift elements after element to left
      memmove(vector->data + index, vector->data + index + 1, (vector->length - index - 1) * sizeof(void*));
    }

    vector->length -= 1;
    return 1;
  }
  return 0;
}

/* removes data in range */

void vec_reminrange(vec_t* vector, uint32_t start, uint32_t end)
{
  for (uint32_t index = start; index < end; index++)
    mem_release(vector->data[index]);
  memmove(vector->data + start, vector->data + end + 1, (vector->length - end - 1) * sizeof(void*));
  vector->length -= end - start + 1;
}

/* removes data in vector */

void vec_reminvector(vec_t* vec_a, vec_t* vec_b)
{
  for (int index = 0; index < vec_b->length; index++)
  {
    vec_rem(vec_a, vec_b->data[index]);
  }
}

/* reverses item order */

void vec_reverse(vec_t* vector)
{
  int length = vector->length;
  for (int index = length - 1; index > -1; index--)
  {
    vec_add(vector, vector->data[index]);
  }
  vec_reminrange(vector, 0, length - 1);
}

/* returns head item of vector */

void* vec_head(vec_t* vector)
{
  if (vector->length > 0)
    return vector->data[0];
  else
    return NULL;
}

/* returns tail item of vector */

void* vec_tail(vec_t* vector)
{
  if (vector->length > 0)
    return vector->data[vector->length - 1];
  else
    return NULL;
}

/* returns index of data or UINT32_MAX if not found */

uint32_t vec_indexofdata(vec_t* vector, void* data)
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

// mt vector node for sorting

typedef struct _mtvn_t mtvn_t;
struct _mtvn_t
{
  void*   c; // content
  mtvn_t* l; // left
  mtvn_t* r; // right
};

// TODO use node pool

void vec_sort_ins(mtvn_t* node, void* data, vsdir_t dir, int (*comp)(void* left, void* right))
{
  if (node->c == NULL)
  {
    node->c = data;
  }
  else
  {
    int smaller = comp(data, node->c) < 0;
    if (dir == VSD_DSC) smaller = 1 - smaller;

    if (smaller)
    {
      if (node->l == NULL) node->l = mem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
      vec_sort_ins(node->l, data, dir, comp);
    }
    else
    {
      if (node->r == NULL) node->r = mem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
      vec_sort_ins(node->r, data, dir, comp);
    }
  }
}

void vec_sort_ord(mtvn_t* node, vec_t* vector, int* index)
{
  if (node->l) vec_sort_ord(node->l, vector, index);
  vector->data[*index] = node->c;
  *index += 1;

  // cleanup node
  mtvn_t* right = node->r;
  REL(node);

  if (right) vec_sort_ord(right, vector, index);
}

// sorts values in vector, needs a comparator function
// or just use strcmp for strings

void vec_sort(vec_t* vector, vsdir_t dir, int (*comp)(void* left, void* right))
{
  mtvn_t* node = mem_calloc(sizeof(mtvn_t), "mtvn_t", NULL, NULL);
  for (int index = 0; index < vector->length; index++)
  {
    vec_sort_ins(node, vector->data[index], dir, comp);
  }
  int index = 0;
  vec_sort_ord(node, vector, &index);
}

void vec_describe(void* pointer, int level)
{
  vec_t* vector = pointer;
  for (uint32_t index = 0; index < vector->length; index++)
  {
    mem_describe(vector->data[index], level + 1);
    printf("\n");
  }
}

#endif
