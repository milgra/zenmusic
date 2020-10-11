#include "common.c"
#include "evthan_drag.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "musiclist.c"
#include "texgen_color.c"
#include "texgen_text.c"
#include "ui_manager.c"
#include "view.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

view_t* song_list;
view_t* viewB;

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));

  char* respath = SDL_GetBasePath();
  char* path    = mtcstr_fromformat("%s/../res/Terminus.ttf", respath, NULL);

  common_font = font_alloc(path);

  ui_manager_init(width, height);

  song_list = view_new("song_list",
                       (vframe_t){20, 20, 500, 600},
                       musiclist_event,
                       texgen_color,
                       musiclist_new,
                       NULL);
  viewB     = view_new("viewb",
                   (vframe_t){200, 420, 350, 170},
                   evthan_drag,
                   texgen_text,
                   NULL,
                   NULL);

  ui_manager_add(song_list);
  ui_manager_add(viewB);
}

void update(ev_t ev)
{
  ui_manager_event(ev);
}

void render()
{
  ui_manager_render();
}

void destroy()
{
}

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}
