/*  
    Created by Milan Toth milgra@milgra.com Public Domain
    One-way non-locking communication channel between threads
    If mtch_send returns 0, channel is full, send data again later
    If mtch_recv returns 0, channel is empty
 */

#ifndef mtch_h
#define mtch_h

#include "mtmemory.c"
#include <SDL.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

typedef struct mtch_t mtch_t;
struct mtch_t
{
  char*  flags;
  void** boxes;

  uint32_t size;
  uint32_t rpos; // read position
  uint32_t wpos; // write position
};

mtch_t* mtch_new(uint32_t size);
void    mtch_del(void* pointer);
char    mtch_send(mtch_t* ch, void* data);
void*   mtch_recv(mtch_t* ch);
void    mtch_test(void);

#endif

#if __INCLUDE_LEVEL__ == 0

mtch_t* mtch_new(uint32_t size)
{
  mtch_t* ch = mtmem_calloc(sizeof(mtch_t), "mtchannel", mtch_del, NULL);

  ch->flags = mtmem_calloc(sizeof(char) * size, "char*", NULL, NULL);
  ch->boxes = mtmem_calloc(sizeof(void*) * size, "void**", NULL, NULL);
  ch->size  = size;
  ch->rpos  = 0;
  ch->wpos  = 0;

  return ch;
}

void mtch_del(void* pointer)
{
  assert(pointer != NULL);

  mtch_t* ch = pointer;

  mtmem_release(ch->flags);
  mtmem_release(ch->boxes);
}

char mtch_send(mtch_t* ch, void* data)
{
  assert(ch != NULL);
  assert(data != NULL);

  // wait for the box to get empty

  if (ch->flags[ch->wpos] == 0)
  {
    ch->boxes[ch->wpos] = data;
    ch->flags[ch->wpos] = 1; // set flag, it doesn't have to be atomic, only the last bit counts
    ch->wpos += 1;           // increment write index, doesn't have to be atomic, this thread uses it only
    if (ch->wpos == ch->size) ch->wpos = 0;

    return 1;
  }

  return 0;
}

void* mtch_recv(mtch_t* ch)
{
  assert(ch != NULL);

  if (ch->flags[ch->rpos] == 1)
  {
    void* result = ch->boxes[ch->rpos];

    ch->boxes[ch->rpos] = NULL; // empty box
    ch->flags[ch->rpos] = 0;    // set flag, it doesn't have to be atomic, only the last bit counts
    ch->rpos += 1;              // increment read index, it doesn't have to be atomic, this thread

    if (ch->rpos == ch->size) ch->rpos = 0;

    return result;
  }

  return NULL;
}

//
//  TEST
//

#define kChTestThreads 10

void send_test(mtch_t* ch)
{
  uint32_t counter = 0;
  while (1)
  {
    uint32_t* number = mtmem_calloc(sizeof(uint32_t), "uint32_t", NULL, NULL);
    *number          = counter;
    char success     = mtch_send(ch, number);
    if (success == 0)
      mtmem_release(number);
    else
      counter += 1;
    if (counter == UINT32_MAX - 1)
      counter = 0;
    //            struct timespec time;
    //            time.tv_sec = 0;
    //            time.tv_nsec = rand() % 100000;
    //            nanosleep(&time , (struct timespec *)NULL);
  }
}

void recv_test(mtch_t* ch)
{
  uint32_t last = 0;
  while (1)
  {
    uint32_t* number = mtch_recv(ch);
    if (number != NULL)
    {
      if (*number != last)
        printf("index error!!!");
      mtmem_release(number);
      last += 1;
      if (last == UINT32_MAX - 1)
        last = 0;
      if (last % 100000 == 0)
        printf("%zx OK %u %u", (size_t)ch, last, UINT32_MAX);
      //                struct timespec time;
      //                time.tv_sec = 0;
      //                time.tv_nsec = rand() % 100000;
      //                nanosleep(&time , (struct timespec *)NULL);
    }
  }
}

mtch_t** testarray;

void mtch_test()
{
  testarray = mtmem_calloc(sizeof(mtch_t) * kChTestThreads, "mtch_t**", NULL, NULL);

  for (int index = 0; index < kChTestThreads; index++)
  {
    testarray[index] = mtch_new(100);
    pthread_t thread;
    /* pthread_create(&thread, NULL, (void*)send_test, testarray[index]); */
    /* pthread_create(&thread, NULL, (void*)recv_test, testarray[index]); */
  }
}

#endif
