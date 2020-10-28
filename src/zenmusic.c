#include "common.c"
#include "eh_drag.c"
#include "eh_list.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "player.c"
#include "tg_bitmap.c"
#include "tg_color.c"
#include "tg_texmap.c"
#include "tg_text.c"
#include "tg_video.c"
#include "ui_manager.c"
#include "view.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

mtvec_t* files;

static int display_info(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
{
  /* printf("%-3s %2d %7jd   %-40s %d %s\n", */
  /*        (tflag == FTW_D) ? "d" : (tflag == FTW_DNR) ? "dnr" : (tflag == FTW_DP) ? "dp" : (tflag == FTW_F) ? "f" : (tflag == FTW_NS) ? "ns" : (tflag == FTW_SL) ? "sl" : (tflag == FTW_SLN) ? "sln" : "???", */
  /*        ftwbuf->level, */
  /*        (intmax_t)sb->st_size, */
  /*        fpath, */
  /*        ftwbuf->base, */
  /*        fpath + ftwbuf->base); */

  mtvec_add(files, mtcstr_fromcstring((char*)(fpath + ftwbuf->base)));

  return 0; /* To tell nftw() to continue */
}

view_t* row_generator(view_t* listview, view_t* rowview, int index)
{

  if (index < 0) return NULL; // no items over 0

  if (rowview == NULL)
  {
    char idbuffer[100];
    snprintf(idbuffer, 100, "list_item%i", index);
    rowview = view_new(idbuffer, (vframe_t){0, 0, 1500, 40}, 0);

    snprintf(idbuffer, 100, "index_item%i", index);
    view_t* indexview = view_new(idbuffer, (vframe_t){0, 0, 50, 40}, 0);
    snprintf(idbuffer, 100, "%i", index);
    tg_text_add(indexview, 0xFF004400, 0x000000FF, idbuffer);

    view_add(rowview, indexview);
  }
  else
  {
    view_set_frame(rowview, (vframe_t){0, 0, 1500, 40});

    rowview->tgdata = NULL;
    rowview->tg     = NULL;
    rowview->ehdata = NULL;
    rowview->eh     = NULL;
  }

  uint32_t color = (index % 2 == 0) ? 0xFFEFEFEF : 0xFFDEDEDE;
  tg_text_add(rowview, color, 0x000000FF, files->data[index]);

  return rowview;
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  files = mtvec_alloc();

  int flags = 0;
  //flags |= FTW_DEPTH;
  flags |= FTW_PHYS;
  nftw("/usr/home/milgra/Music", display_info, 20, flags);

  printf("file count %i\n", files->length);

  srand((unsigned int)time(NULL));

  char* respath = SDL_GetBasePath();
  char* path    = mtcstr_fromformat("%s/../res/Avenir.ttc", respath, NULL);

  common_font = font_alloc(path);

  ui_manager_init(width, height);

  char*   playpath    = mtcstr_fromformat("%s/../res/play.png", respath, NULL);
  view_t* playbtnview = view_new("playbtnview", (vframe_t){5, 5, 90, 90}, 0);
  tg_bitmap_add(playbtnview, playpath);

  view_t* header = view_new("header", (vframe_t){0, 900, 1200, 100}, 0);
  tg_text_add(header, 0xFFFFFFFF, 0x000000FF, "Zen Music Player");
  view_add(header, playbtnview);

  view_t* songlist = view_new("songlist", (vframe_t){0, 0, 1500, 1000}, 0);
  tg_color_add(songlist, 0xFF222222);
  eh_list_add(songlist, row_generator);

  view_t* videoview = view_new("videoview", (vframe_t){400, 400, 800, 600}, 1);
  tg_video_add(videoview);

  view_t*   chessview = view_new("chessview", (vframe_t){100, 100, 300, 300}, 0);
  bm_t*     chessbmp  = bm_new(300, 300);
  uint32_t* data      = (uint32_t*)chessbmp->data;
  for (int col = 0; col < 300; col++)
  {
    for (int row = 0; row < 300; row++)
    {
      uint32_t index = row * 300 + col;
      uint32_t color = (row % 2 == 0 && col % 2 == 0) ? 0xFFFFFFFF : 0xFF000000;
      data[index]    = color;
    }
  }
  view_set_texture(chessview, chessbmp);
  eh_drag_add(chessview);

  view_t* texmapview = view_new("texmapview", (vframe_t){50, 500, 200, 200}, 0);
  eh_drag_add(texmapview);
  tg_texmap_add(texmapview);

  ui_manager_add(songlist);
  ui_manager_add(videoview);
  ui_manager_add(texmapview);
  ui_manager_add(header);
  //ui_manager_add(chessview);

  //player_init();
}

void update(ev_t ev)
{
  ui_manager_event(ev);
}

void render()
{
  //player_draw();
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
