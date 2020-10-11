#include "common.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "musiclist.c"
#include "tex/color.c"
#include "tex/text.c"
#include "ui_manager.c"
#include "view.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));

  char* respath = SDL_GetBasePath();
  char* path    = mtcstr_fromformat("%s/../res/Terminus.ttf", respath, NULL);

  common_font = font_alloc(path);

  ui_manager_init(width, height);

  view_t* header = view_new("header",
                            (vframe_t){0, 0, 600, 100},
                            NULL,
                            text_gen,
                            NULL,
                            NULL);

  view_t* song_list = view_new("song_list",
                               (vframe_t){0, 100, 600, 600},
                               musiclist_event,
                               color_gen,
                               musiclist_new,
                               NULL);

  ui_manager_add(header);
  ui_manager_add(song_list);
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
