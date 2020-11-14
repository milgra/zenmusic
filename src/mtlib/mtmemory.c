//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtmem_h
#define mtmem_h

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define RPL(X, Y) mtmem_replace((void**)&X, Y)
#define SET(X, Y) X = Y
#define RET(X) mtmem_retain(X)
#define REL(X) mtmem_release(X)
#define HEAP(X, T) mtmem_stack_to_heap(sizeof(X), T, NULL, NULL, (uint8_t*)&X)

struct mtmem_head
{
  char type[10];
  void (*destructor)(void*);
  void (*descriptor)(void*, int);
  size_t retaincount;
};

void*  mtmem_alloc(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int));
void*  mtmem_calloc(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int));
void*  mtmem_realloc(void* pointer, size_t size);
void*  mtmem_retain(void* pointer);
char   mtmem_release(void* pointer);
char   mtmem_releaseeach(void* first, ...);
size_t mtmem_retaincount(void* pointer);
void   mtmem_replace(void** address, void* data);
void*  mtmem_stack_to_heap(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int), uint8_t* data);
void   mtmem_describe(void* pointer, int level);
void   mtmem_exit(char* text, char* type);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <string.h>

void* mtmem_alloc(size_t size,                    /* size of data to store */
                  char*  type,                    /* short descriptoon of data to show for describing memory map*/
                  void (*destructor)(void*),      /* optional destructor */
                  void (*descriptor)(void*, int)) /* optional descriptor for describing memory map*/
{
  if (size == 0) mtmem_exit("Ttrying to allocate 0 bytes for", type);
  uint8_t* bytes = malloc(sizeof(struct mtmem_head) + size);
  if (bytes == NULL) mtmem_exit("Out of RAM \\_(o)_/ for", type);

  struct mtmem_head* head = (struct mtmem_head*)bytes;

  memcpy(head->type, type, 9);
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  return bytes + sizeof(struct mtmem_head);
}

void* mtmem_calloc(size_t size,                    /* size of data to store */
                   char*  type,                    /* short descriptoon of data to show for describing memory map*/
                   void (*destructor)(void*),      /* optional destructor */
                   void (*descriptor)(void*, int)) /* optional descriptor for describing memory map*/
{
  if (size == 0) mtmem_exit("Ttrying to allocate 0 bytes for", type);
  uint8_t* bytes = calloc(1, sizeof(struct mtmem_head) + size);
  if (bytes == NULL) mtmem_exit("Out of RAM \\_(o)_/ for", type);

  struct mtmem_head* head = (struct mtmem_head*)bytes;

  memcpy(head->type, type, 9);
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  return bytes + sizeof(struct mtmem_head);
}

void* mtmem_stack_to_heap(size_t size,
                          char*  type,
                          void (*destructor)(void*),
                          void (*descriptor)(void*, int),
                          uint8_t* data)
{
  uint8_t* bytes = mtmem_alloc(size, type, destructor, descriptor);
  if (bytes == NULL) mtmem_exit("Out of RAM \\_(o)_/ for", type);
  memcpy(bytes, data, size);
  return bytes;
}

void* mtmem_realloc(void* pointer, size_t size)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mtmem_head);
  bytes = realloc(bytes, sizeof(struct mtmem_head) + size);
  if (bytes == NULL) mtmem_exit("Out of RAM \\_(o)_/ when realloc", "");

  return bytes + sizeof(struct mtmem_head);
}

void* mtmem_retain(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mtmem_head);
  struct mtmem_head* head = (struct mtmem_head*)bytes;

  head->retaincount += 1;
  if (head->retaincount == SIZE_MAX) mtmem_exit("Maximum retain count reached \\(o)_/ for", head->type);

  return pointer;
}

char mtmem_release(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mtmem_head);
  struct mtmem_head* head = (struct mtmem_head*)bytes;

  if (head->retaincount == 0) mtmem_exit("Tried to release already released memory for", head->type);

  head->retaincount -= 1;

  if (head->retaincount == 0)
  {
    if (head->destructor != NULL) head->destructor(pointer);
    free(bytes);
    return 1;
  }

  return 0;
}

char mtmem_releaseeach(void* first, ...)
{
  va_list ap;
  void*   actual;
  char    released = 1;
  va_start(ap, first);
  for (actual = first; actual != NULL; actual = va_arg(ap, void*))
  {
    released &= mtmem_release(actual);
  }
  va_end(ap);
  return released;
}

size_t mtmem_retaincount(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mtmem_head);
  struct mtmem_head* head = (struct mtmem_head*)bytes;

  return head->retaincount;
}

void mtmem_replace(void** address, void* data)
{
  if (*address != NULL) mtmem_release(*address);
  mtmem_retain(data);
  *address = data;
}

void mtmem_describe(void* pointer, int level)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mtmem_head);
  struct mtmem_head* head = (struct mtmem_head*)bytes;

  if (head->descriptor != NULL)
  {
    head->descriptor(pointer, ++level);
  }
  else
  {
    printf("(%s)", head->type);
  }
}

void mtmem_exit(char* text, char* type)
{
  printf("%s %s\n", text, type);
  exit(EXIT_FAILURE);
}

#endif
