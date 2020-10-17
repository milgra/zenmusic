#include "common.c"
#include "eh_songs.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "player.c"
#include "tg_color.c"
#include "tg_texmap.c"
#include "tg_text.c"
#include "tg_video.c"
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

  view_t* header = view_new("header", (vframe_t){0, 0, 600, 100});

  tg_text_add(header, 0xFFFFFFFF, 0x000000FF, "Zen Music Player");

  view_t* songlist = view_new("songlist", (vframe_t){0, 100, 600, 600});

  tg_color_add(songlist, 0x222222FF);
  eh_songs_add(songlist);

  view_t* videoview = view_new("videoview", (vframe_t){400, 400, 320, 240});

  tg_video_add(videoview);
  eh_drag_add(videoview);

  view_t* texmapview = view_new("texmapview", (vframe_t){50, 500, 400, 400});

  tg_texmap_add(texmapview);

  ui_manager_add(header);
  ui_manager_add(songlist);
  ui_manager_add(videoview);
  ui_manager_add(texmapview);

  player_init();
}

void update(ev_t ev)
{
  ui_manager_event(ev);
}

void render()
{
  player_draw();
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
