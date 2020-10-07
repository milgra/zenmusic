#include "mtch.c"
#include "ui_compositor.c"
#include "ui_connector.c"
#include "wm_connector.c"
#include <SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

ch_t* zench;

void* brender(void* mypointer)
{
  ch_t* mych = (ch_t*)mypointer;
  while (1)
  {
    ch_send(mych, "anyad");
    sleep(5);
  }
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  ui_connector_init(width, height);

  // start generator in background thread

  /* this variable is our reference to the second thread */
  pthread_t thread;
  zench = ch_new(10);

  int success = pthread_create(&thread, NULL, brender, zench);

  printf("thread created: %i\n", success);
}

void update(int x, int y)
{
  ui_compositor_update(x, y);
}

void render()
{
  char* data = (char*)ch_recv(zench);
  if (data)
    printf("render, recv data %s\n", data);

  ui_compositor_render();
}

void destroy()
{
  printf("zenmusic destroy\n");
}

int main(int argc, char* args[])
{

  wm_init(init, update, render, destroy);

  return 0;
}
