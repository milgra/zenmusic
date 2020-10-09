#include "common.c"
#include "event.c"
#include "font.c"
#include "math2.c"
#include "mtbm.c"
#include "mtcstr.c"
#include "mtstr.c"
#include "ui_compositor.c"
#include "ui_connector.c"
#include "view.c"
#include "wm_connector.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

view_t* viewA;
view_t* viewB;

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  char* path = mtcstr_fromformat("%s/../res/Terminus.ttf", respath, NULL);

  common_font = font_alloc(path);

  ui_connector_init(width, height);

  viewA = view_new("viewa", (v4_t){20.0, 20.0, 150.0, 70.0});
  viewB = view_new("viewb", (v4_t){200.0, 420.0, 350.0, 170.0});

  ui_connector_add(viewA);
  ui_connector_add(viewB);
}

void update(ev_t ev)
{

  if (ev.type == EV_MMOVE && ev.drag)
  {
    view_setpos(viewA, (v2_t){ev.x, ev.y});
  }
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
