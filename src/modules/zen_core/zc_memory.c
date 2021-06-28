#ifndef zc_memory_h
#define zc_memory_h

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAL(X, Y, Z) mem_calloc(X, Y, Z, __FILE__, __LINE__);
#define RET(X) mem_retain(X, __FILE__, __LINE__)
#define REL(X) mem_release(X, __FILE__, __LINE__)
#define HEAP(X) mem_stack_to_heap(sizeof(X), NULL, NULL, (uint8_t*)&X, __FILE__, __LINE__)

struct mem_head
{
  char     id[2]; // starting bytes for zc_memory managed memory ranges to detect invalid object during retain/release
  uint32_t index; // allocation index for debugging/safety checks

  void (*destructor)(void*);
  void (*descriptor)(void*, int);
  size_t retaincount;
};

void*    mem_alloc(size_t size, void (*destructor)(void*), void (*descriptor)(void*, int), char* file, int line);
void*    mem_calloc(size_t size, void (*destructor)(void*), void (*descriptor)(void*, int), char* file, int line);
void*    mem_realloc(void* pointer, size_t size);
void*    mem_retain(void* pointer, char* file, int line);
char     mem_release(void* pointer, char* file, int line);
char     mem_release_each(void* first, ...);
size_t   mem_retaincount(void* pointer);
void*    mem_stack_to_heap(size_t size, void (*destructor)(void*), void (*descriptor)(void*, int), uint8_t* data, char* file, int line);
uint32_t mem_index_value(void* pointer);
void     mem_describe(void* pointer, int level);
void     mem_exit(char* text, char* file, int line);
void     mem_stat(void* pointer);
void     mem_stats();

#endif

#if __INCLUDE_LEVEL__ == 0

#include <string.h>

/*******DEBUGGING********/

#define ZC_MAX_BLOCKS 100000
#define ZC_TRACEROUTE_ALLOC 0
#define ZC_TRACEROUTE_CALLOC 0
#define ZC_TRACEROUTE_RETAIN 0

struct mem_info
{
  char* file;
  int   line;
  void* ptr;
};

struct mem_info mem_infos[ZC_MAX_BLOCKS] = {0};
uint32_t        mem_index                = 1; /* live object counter for debugging */

/************************/

void* mem_alloc(size_t size,                    /* size of data to store */
                void (*destructor)(void*),      /* optional destructor */
                void (*descriptor)(void*, int), /* optional descriptor for describing memory map*/
                char* file,                     /* caller file name */
                int   line)                       /* caller file line number */
{
  if (size == 0) mem_exit("Ttrying to allocate 0 bytes for", file, line);
  uint8_t* bytes = malloc(sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", file, line);

  struct mem_head* head = (struct mem_head*)bytes;

  head->id[0]       = 'z';
  head->id[1]       = 'c';
  head->index       = mem_index;
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  // for leak checking
  mem_infos[mem_index].file = file;
  mem_infos[mem_index].line = line;
  mem_infos[mem_index].ptr  = bytes + sizeof(struct mem_head);

  // debug
  if (mem_index == ZC_TRACEROUTE_ALLOC) abort();

  mem_index++;

  return bytes + sizeof(struct mem_head);
}

void* mem_calloc(size_t size,                    /* size of data to store */
                 void (*destructor)(void*),      /* optional destructor */
                 void (*descriptor)(void*, int), /* optional descriptor for describing memory map*/
                 char* file,                     /* caller file name */
                 int   line)                       /* caller file line number */
{
  if (size == 0) mem_exit("Ttrying to allocate 0 bytes for", file, line);
  uint8_t* bytes = calloc(1, sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", file, line);

  struct mem_head* head = (struct mem_head*)bytes;

  head->id[0]       = 'z';
  head->id[1]       = 'c';
  head->index       = mem_index;
  head->destructor  = destructor;
  head->descriptor  = descriptor;
  head->retaincount = 1;

  // for leak checking
  mem_infos[mem_index].file = file;
  mem_infos[mem_index].line = line;
  mem_infos[mem_index].ptr  = bytes + sizeof(struct mem_head);

  // debug
  if (mem_index == ZC_TRACEROUTE_CALLOC) abort();

  mem_index++;

  return bytes + sizeof(struct mem_head);
}

void* mem_stack_to_heap(size_t size,
                        void (*destructor)(void*),
                        void (*descriptor)(void*, int),
                        uint8_t* data,
                        char*    file,
                        int      line)
{
  uint8_t* bytes = mem_alloc(size, destructor, descriptor, file, line);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ for", file, line);
  memcpy(bytes, data, size);
  return bytes;
}

void* mem_realloc(void* pointer, size_t size)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  bytes = realloc(bytes, sizeof(struct mem_head) + size);
  if (bytes == NULL) mem_exit("Out of RAM \\_(o)_/ when realloc", "", 0);

  return bytes + sizeof(struct mem_head);
}

void* mem_retain(void* pointer, char* file, int line)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'z' && head->id[1] == 'c');

  // debug
  if (head->index == ZC_TRACEROUTE_RETAIN) abort();

  head->retaincount += 1;
  if (head->retaincount == SIZE_MAX) mem_exit("Maximum retain count reached \\(o)_/ for", "", 0);

  return pointer;
}

char mem_release(void* pointer, char* file, int line)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'z' && head->id[1] == 'c');

  if (head->retaincount == 0) mem_exit("Tried to release already released memory for", "", 0);

  head->retaincount -= 1;

  if (head->retaincount == 0)
  {
    mem_infos[head->index].file = NULL;
    mem_infos[head->index].line = 0;

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
    released &= mem_release(actual, __FILE__, __LINE__);
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
  assert(head->id[0] == 'z' && head->id[1] == 'c');

  return head->retaincount;
}

uint32_t mem_index_value(void* pointer)
{
  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;
  return head->index;
}

void mem_describe(void* pointer, int level)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'z' && head->id[1] == 'c');

  if (head->descriptor != NULL)
  {
    head->descriptor(pointer, ++level);
  }
  else
  {
    printf("no descriptor, index %u", head->index);
  }
}

void mem_exit(char* text, char* file, int line)
{
  printf("%s %s %i\n", text, file, line);
  exit(EXIT_FAILURE);
}

void mem_stat(void* pointer)
{
  assert(pointer != NULL);

  uint8_t* bytes = (uint8_t*)pointer;
  bytes -= sizeof(struct mem_head);
  struct mem_head* head = (struct mem_head*)bytes;

  // check memory range id
  assert(head->id[0] == 'z' && head->id[1] == 'c');

  printf("mem stat %s %i", mem_infos[head->index].file, mem_infos[head->index].line);
}

void mem_stats()
{
  uint32_t count = 0;
  for (int index = 0; index < mem_index; index++)
  {
    char* file = mem_infos[index].file;
    if (file != NULL)
    {
      printf("unreleased block %i at %s %i desc : ", index, mem_infos[index].file, mem_infos[index].line);
      mem_describe(mem_infos[index].ptr, 0);
      printf("\n");
      count++;
    }
  }
  printf("total unreleased blocks : %u\n", count);
}

#endif
