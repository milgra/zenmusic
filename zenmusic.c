#include "ui_compositor.c"
#include "ui_connector.c"
#include "wm_connector.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  ui_connector_init(width, height);
}

void update(int x, int y)
{
  ui_connector_update(x, y);
}

void render()
{
  ui_connector_render();
}

void destroy()
{
}

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}
