#include "common.c"
#include "eh_knob.c"
#include "eh_list.c"
#include "eh_text.c"
#include "eh_video.c"
#include "font.c"
#include "mtcstring.c"
#include "player.c"
#include "songitem.c"
#include "tg_knob.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "view.c"
#include "view_generator.c"
#include "view_util.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <float.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

mtvec_t* files;
view_t*  coverview;
view_t*  baseview;
view_t*  timeview;
view_t*  visuleft;
view_t*  visuright;
double   lasttime = 0.0;
view_t*  playbtn;
view_t*  volbtn;

static int display_info(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
{
  /* printf("%-3s %2d %7jd   %-40s %d %s\n", */
  /*        (tflag == FTW_D) ? "d" : (tflag == FTW_DNR) ? "dnr" : (tflag == FTW_DP) ? "dp" : (tflag == FTW_F) ? "f" : (tflag == FTW_NS) ? "ns" : (tflag == FTW_SL) ? "sl" : (tflag == FTW_SLN) ? "sln" : "???", */
  /*        ftwbuf->level, */
  /*        (intmax_t)sb->st_size, */
  /*        fpath, */
  /*        ftwbuf->base, */
  /*        fpath + ftwbuf->base); */

  mtvec_add(files, mtcstr_fromcstring((char*)fpath));

  return 0; /* To tell nftw() to continue */
}

void songitem_event(ev_t ev, void* data)
{
  if (ev.type == EV_MDOWN)
  {
    size_t index = (size_t)data;
    // printf("songitem event %i %i %s\n", ev.type, index, (char*)files->data[index]);

    view_t* song = view_get_subview(baseview, "song");
    tg_text_add(song, 0x00000000, 0x000000FF, (char*)files->data[index], 0);

    bm_t* bitmap = player_get_album(files->data[index]);
    //tg_bitmap_add(coverview, NULL, bitmap, "album");

    player_play(files->data[index]);
  }
}

void playbtn_event(ev_t ev, void* data)
{
  if (ev.type == EV_MDOWN)
  {
    player_stop();
  }
}

view_t* songlist_item_generator(view_t* listview, view_t* rowview, int index)
{
  if (files == NULL)
  {
    files     = mtvec_alloc();
    int flags = 0;
    //flags |= FTW_DEPTH;
    flags |= FTW_PHYS;
    //nftw("/usr/home/milgra/Projects/zenmusic/res/med", display_info, 20, flags);
    nftw("/usr/home/milgra/Music", display_info, 20, flags);
    //printf("file count %i\n", files->length);
  }
  if (index < 0)
    return NULL; // no items over 0
  if (index > files->length)
    return NULL;
  if (rowview == NULL)
    rowview = songitem_new();
  songitem_update(rowview, index, files->data[index], songitem_event);
  return rowview;
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));

  char* respath  = SDL_GetBasePath();
  char* csspath  = mtcstr_fromformat("%s/../res/main.css", respath, NULL);
  char* htmlpath = mtcstr_fromformat("%s/../res/main.html", respath, NULL);
  char* fontpath = mtcstr_fromformat("%s/../res/Ubuntu-Regular.ttf", respath, NULL);

  mtvec_t* views = view_gen_load(htmlpath, csspath, respath);
  baseview       = mtvec_head(views);

  common_respath = respath;
  common_font    = font_alloc(fontpath);

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  view_t* songlist = view_get_subview(baseview, "songlist");
  eh_list_add(songlist, songlist_item_generator);

  view_t* video = view_get_subview(baseview, "video");
  eh_video_add(video);

  timeview = view_get_subview(baseview, "time");
  tg_text_add(timeview, 0x00000000, 0x000000FF, "00:00", 0);

  view_t* song = view_get_subview(baseview, "song");
  tg_text_add(song, 0x00000000, 0x000000FF, "-", 1);

  view_t* songlistheader = view_get_subview(baseview, "songlistheader");

  view_t* filterbar = view_get_subview(baseview, "filterbar");
  tg_text_add(filterbar, 0xFFFFFFFF, 0x000000FF, "Search/Filter", 1);
  eh_text_add(filterbar, "placeholder");

  view_t* headeritem = songitem_new();
  songitem_update(headeritem, -1, "Artist", NULL);
  view_add(songlistheader, headeritem);

  playbtn = view_get_subview(baseview, "playbtn");
  eh_knob_add(playbtn);
  tg_knob_add(playbtn);

  volbtn = view_get_subview(baseview, "volbtn");
  eh_knob_add(volbtn);
  tg_knob_add(volbtn);

  view_t* header = view_get_subview(baseview, "header");
  //header->texture.blur = 1;
  //header->texture.shadow = 1;

  visuleft  = view_get_subview(baseview, "visuleft");
  visuright = view_get_subview(baseview, "visuright");

  /* view_t* texmapview       = view_new("texmapview", (r2_t){500, 500, 200, 200}); */
  /* texmapview->texture.full = 1; */
  /* ui_manager_add(texmapview); */
}

void update(ev_t ev)
{
  double time = player_time();
  if (time > 0.0)
  {
    // update timer
    if (floor(time) != lasttime)
    {
      lasttime = floor(time);

      char timebuff[20];
      snprintf(timebuff, 20, "%.2i:%.2i", (int)floor(lasttime / 60.0), (int)lasttime % 60);
      tg_text_set(timeview, 0x00000000, 0x000000FF, timebuff, 0);

      double posratio = time / player_duration();
      tg_knob_set_angle(playbtn, posratio * 3.14 - 3.14 / 2.0);
    }

    // update visualizer
    player_draw_waves(visuleft->texture.page, 0, visuleft->texture.bitmap);
    player_draw_waves(visuright->texture.page, 1, visuright->texture.bitmap);
    visuleft->texture.changed  = 1;
    visuright->texture.changed = 1;
  }
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
