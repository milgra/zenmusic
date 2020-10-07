#include "mtpipe.c"
#include "ui_connector.c"
#include "wm_connector.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

mtpipe_t* zenpipe;

void* brender(void* mypointer)
{
  mtpipe_t* mypipe = (mtpipe_t*)mypointer;
  while (1)
  {

    mtpipe_send(mypipe, "anyad");
    sleep(5);
  }
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  ui_init(width, height);

  // start generator in background thread

  /* this variable is our reference to the second thread */
  pthread_t thread;
  zenpipe = mtpipe_alloc(10);

  int success = pthread_create(&thread, NULL, brender, zenpipe);

  printf("thread created: %i\n", success);
}

void render()
{

  char* data = (char*)mtpipe_recv(zenpipe);
  if (data)
    printf("render, recv data %s\n", data);

  ui_render();
}

void destroy()
{
  printf("zenmusic destroy\n");
}

int main(int argc, char* args[])
{

  wm_init(init, render, destroy);

  return 0;
}
