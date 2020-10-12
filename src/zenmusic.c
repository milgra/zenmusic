#include "common.c"
#include "eh_songs.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "tg_color.c"
#include "tg_text.c"
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

  // header_texture_generator = text_gen("Header",0x000000FF,0xFFFFFFFF );
  // header_event_handler =
  // musiclist_texture_generatpr = color_gen(0x000000FF)
  // musiclist_event_handler = musiclist_ev_gen

  view_t* header = view_new("header", (vframe_t){0, 0, 600, 100});

  tg_text_add(header, 0x000000FF, 0xFFFFFFFF, "Header");

  view_t* songlist = view_new("songlist", (vframe_t){0, 100, 600, 600});

  tg_color_add(songlist, 0x00FF00FF);
  eh_songs_add(songlist);

  ui_manager_add(header);
  ui_manager_add(songlist);
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
