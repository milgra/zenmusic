#ifndef zc_memory_h
#define zc_memory_h

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define RPL(X, Y) mem_replace((void**)&X, Y)
#define SET(X, Y) X = Y
#define RET(X) mem_retain(X)
#define REL(X) mem_release(X)
#define HEAP(X, T) mem_stack_to_heap(sizeof(X), T, NULL, NULL, (uint8_t*)&X)

struct mem_head
{
  char id[2]; // starting bytes for zc_memory managed memory ranges to detect invalid object during retain/release
  char type[10];
  void (*destructor)(void*);
  void (*descriptor)(void*, int);
  size_t retaincount;
};

void*  mem_alloc(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int));
void*  mem_calloc(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int));
void*  mem_realloc(void* pointer, size_t size);
void*  mem_retain(void* pointer);
char   mem_release(void* pointer);
char   mem_releaseeach(void* first, ...);
size_t mem_retaincount(void* pointer);
void   mem_replace(void** address, void* data);
void*  mem_stack_to_heap(size_t size, char* type, void (*destructor)(void*), void (*descriptor)(void*, int), uint8_t* data);
char*  mem_type(void* pointer);
void   mem_describe(void* pointer, int level);
void   mem_exit(char* text, char* type);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <string.h>

void* mem_alloc(size_t size,                    /* size of data to store */
                char*  type,                    /* short descriptoon of data to show for describing memory map*/
                void (*destructor)(void*),      /* optional destructor */
                void (*descriptor)(void*, int)) /* optional descriptor for describing memory map*/
{
  if (size == 0) mem_exit("Ttrying to allocate 0 bytes for", type);
  uint8_t* bytes = malloc(sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", type);

  struct mem_head* head = (struct mem_head*)bytes;

  head->id[0] = 'm';
  head->id[1] = 't';
  memcpy(head->type, type, 9);
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  return bytes + sizeof(struct mem_head);
}

void* mem_calloc(size_t size,                    /* size of data to store */
                 char*  type,                    /* short descriptoon of data to show for describing memory map*/
                 void (*destructor)(void*),      /* optional destructor */
                 void (*descriptor)(void*, int)) /* optional descriptor for describing memory map*/
{
  if (size == 0) mem_exit("Ttrying to allocate 0 bytes for", type);
  uint8_t* bytes = calloc(1, sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", type);

  struct mem_head* head = (struct mem_head*)bytes;

  head->id[0] = 'm';
  head->id[1] = 't';
  memcpy(head->type, type, 9);
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  return bytes + sizeof(struct mem_head);
}

void* mem_stack_to_heap(size_t size,
                        char*  type,
                        void (*destructor)(void*),
                        void (*descriptor)(void*, int),
                        uint8_t* data)
{
  uint8_t* bytes = mem_alloc(size, type, destructor, descriptor);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", type);
  memcpy(bytes, data, size);
  return bytes;
}

void* mem_realloc(void* pointer, size_t size)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  bytes = realloc(bytes, sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ when realloc", "");

  return bytes + sizeof(struct mem_head);
}

void* mem_retain(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'm' && head->id[1] == 't');

  head->retaincount += 1;
  if (head->retaincount == SIZE_MAX) mem_exit("Maximum retain count reached \\(o)_/ for", head->type);

  return pointer;
}

char mem_release(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'm' && head->id[1] == 't');

  if (head->retaincount == 0) mem_exit("Tried to release already released memory for", head->type);

  head->retaincount -= 1;

  if (head->retaincount == 0)
  {
    if (head->destructor != NULL) head->destructor(pointer);
    // zero out bytes that will be deallocated so it will be easier to detect re-using of released zc_memory areas
    memset(bytes, 0, sizeof(struct mem_head));
    free(bytes);
    return 1;
  }

  return 0;
}

char mem_releaseeach(void* first, ...)
{
  va_list ap;
  void*   actual;
  char    released = 1;
  va_start(ap, first);
  for (actual = first; actual != NULL; actual = va_arg(ap, void*))
  {
    released &= mem_release(actual);
  }
  va_end(ap);
  return released;
}

size_t mem_retaincount(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'm' && head->id[1] == 't');

  return head->retaincount;
}

char* mem_type(void* pointer)
{
  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;
  return head->type;
}

void mem_replace(void** address, void* data)
{
  if (*address != NULL) mem_release(*address);
  mem_retain(data);
  *address = data;
}

void mem_describe(void* pointer, int level)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'm' && head->id[1] == 't');

  if (head->descriptor != NULL)
  {
    head->descriptor(pointer, ++level);
  }
  else
  {
    printf("(%s)", head->type);
  }
}

void mem_exit(char* text, char* type)
{
  printf("%s %s\n", text, type);
  exit(EXIT_FAILURE);
}

#endif
