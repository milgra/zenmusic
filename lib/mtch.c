//  Created by Milan Toth milgra@milgra.com Public Domain

#ifndef mtch_h
#define mtch_h

#include "mtmem.c"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

typedef struct ch_t ch_t;
struct ch_t
{
  char* flags;
  void** boxes;

  uint32_t size;
  uint32_t read_index;
  uint32_t write_index;
};

ch_t* ch_new(uint32_t size);
void ch_del(void* pointer);
char ch_send(ch_t* boxes, void* data);
void* ch_recv(ch_t* boxes);
void ch_test(void);

#endif

#if __INCLUDE_LEVEL__ == 0

ch_t* ch_new(uint32_t size)
{
  ch_t* boxes = mtmem_calloc(sizeof(ch_t), ch_del);

  boxes->flags = mtmem_calloc(sizeof(char) * size, NULL);
  boxes->boxes = mtmem_calloc(sizeof(void*) * size, NULL);
  boxes->size = size;
  boxes->read_index = 0;
  boxes->write_index = 0;

  return boxes;
}

void ch_del(void* pointer)
{
  assert(pointer != NULL);

  ch_t* boxes = pointer;

  mtmem_release(boxes->flags);
  mtmem_release(boxes->boxes);
}

char ch_send(ch_t* boxes, void* data)
{
  assert(boxes != NULL);
  assert(data != NULL);

  // wait for the box to get empty

  if (boxes->flags[boxes->write_index] == 0)
  {
    boxes->boxes[boxes->write_index] = data;
    boxes->flags[boxes->write_index] = 1; // set flag, it doesn't have to be atomic, only the last bit counts
    boxes->write_index += 1;              // increment write index, doesn't have to be atomic, this thread uses it only
    if (boxes->write_index == boxes->size) boxes->write_index = 0;

    return 1;
  }

  return 0;
}

void* ch_recv(ch_t* boxes)
{
  assert(boxes != NULL);

  if (boxes->flags[boxes->read_index] == 1)
  {
    void* result = boxes->boxes[boxes->read_index];

    boxes->boxes[boxes->read_index] = NULL; // empty box
    boxes->flags[boxes->read_index] = 0;    // set flag, it doesn't have to be atomic, only the last bit counts
    boxes->read_index += 1;                 // increment read index, it doesn't have to be atomic, this thread

    if (boxes->read_index == boxes->size) boxes->read_index = 0;

    return result;
  }

  return NULL;
}

//
//  TEST
//

#define kBoxesTestThreads 10

void send_test(ch_t* boxes)
{
  uint32_t counter = 0;
  while (1)
  {
    uint32_t* number = mtmem_calloc(sizeof(uint32_t), NULL);
    *number = counter;
    char success = ch_send(boxes, number);
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

void recv_test(ch_t* boxes)
{
  uint32_t last = 0;
  while (1)
  {
    uint32_t* number = ch_recv(boxes);
    if (number != NULL)
    {
      if (*number != last)
        printf("index error!!!");
      mtmem_release(number);
      last += 1;
      if (last == UINT32_MAX - 1)
        last = 0;
      if (last % 100000 == 0)
        printf("%zx OK %u %u", (size_t)boxes, last, UINT32_MAX);
      //                struct timespec time;
      //                time.tv_sec = 0;
      //                time.tv_nsec = rand() % 100000;
      //                nanosleep(&time , (struct timespec *)NULL);
    }
  }
}

ch_t** testarray;

void ch_test()
{
  testarray = mtmem_calloc(sizeof(ch_t) * kBoxesTestThreads, NULL);

  for (int index = 0; index < kBoxesTestThreads; index++)
  {
    testarray[index] = ch_new(100);
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)send_test, testarray[index]);
    pthread_create(&thread, NULL, (void*)recv_test, testarray[index]);
  }
}

#endif
