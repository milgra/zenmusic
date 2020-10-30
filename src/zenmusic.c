#include "common.c"
#include "eh_drag.c"
#include "eh_list.c"
#include "font.c"
#include "mtcstring.c"
#include "mtmath4.c"
#include "player.c"
#include "songitem.c"
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
#include <float.h>
#include <ftw.h>
#include <limits.h>
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

  mtvec_add(files, mtcstr_fromcstring((char*)fpath));

  return 0; /* To tell nftw() to continue */
}

void songitem_event(ev_t ev, void* data)
{
  if (ev.type == EV_MDOWN)
  {
    int index = (int)data;
    printf("songitem event %i %i %s\n", ev.type, index, (char*)files->data[index]);
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
    nftw("/usr/home/milgra/Music", display_info, 20, flags);
    printf("file count %i\n", files->length);
  }
  if (index < 0) return NULL; // no items over 0
  if (rowview == NULL) rowview = songitem_new();
  songitem_update(rowview, index, files->data[index], songitem_event);
  return rowview;
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));

  char* respath = SDL_GetBasePath();
  char* path    = mtcstr_fromformat("%s/../res/Ubuntu-Regular.ttf", respath, NULL);

  common_font = font_alloc(path);

  ui_manager_init(width, height);
  ui_manager_set_layout((vlayout_t){
      .display = LD_FLEX});

  // songlist view
  view_t* songlist = view_new("songlist", (vframe_t){0, 0, 500, 500}, 0);
  tg_color_add(songlist, 0x222222FF);
  eh_list_add(songlist, songlist_item_generator);

  view_set_layout(songlist, (vlayout_t){
                                .margin_right = 300.0,
                                .w_per        = 1.0,
                                .h_per        = 1.0});

  view_t* seekview = view_new("seekview", (vframe_t){0, 305, 290, 15}, 0);
  view_set_layout(seekview, (vlayout_t){
                                .margin = INT_MAX});
  tg_color_add(seekview, 0xFFFFFF66);

  view_t* volview = view_new("volview", (vframe_t){0, 325, 290, 15}, 0);
  view_set_layout(volview, (vlayout_t){
                               .margin = INT_MAX});
  tg_color_add(volview, 0xFFFFFF66);

  // visualization views

  view_t* visuals = view_new("visuals", (vframe_t){0, 0, 758, 238}, 0);
  tg_color_add(visuals, 0x99999955);
  view_set_layout(visuals, (vlayout_t){
                               .margin_top = 20.0,
                               .margin     = INT_MAX});

  view_t* videoview_left = view_new("videoviewleft", (vframe_t){0, 50, 300, 150}, 1);
  tg_video_add(videoview_left, 1280, 720);
  view_set_layout(videoview_left, (vlayout_t){
                                      .margin_top  = 44.0,
                                      .margin_left = 2.0});

  view_t* videoview_right = view_new("videoviewright", (vframe_t){0, 50, 300, 150}, 1);
  tg_video_add(videoview_right, 1280, 720);
  view_set_layout(videoview_right, (vlayout_t){
                                       .margin_top   = 44.0,
                                       .margin_right = 2.0});

  view_t* coverview = view_new("coverview", (vframe_t){0, 44, 150, 150}, 0);
  tg_color_add(coverview, 0xAAAAAAFF);
  view_set_layout(coverview, (vlayout_t){
                                 .margin_top  = 44.0,
                                 .margin_left = 304.0});

  view_add(visuals, videoview_left);
  view_add(visuals, videoview_right);
  view_add(visuals, coverview);

  // buttons

  char*   playpath = mtcstr_fromformat("%s/../res/play.png", respath, NULL);
  view_t* playbtn  = view_new("playbtnview", (vframe_t){0, 40, 80, 80}, 0);
  tg_bitmap_add(playbtn, playpath, NULL);
  view_set_layout(playbtn, (vlayout_t){
                               .margin = INT_MAX});

  eh_touch_add(playbtn, NULL, playbtn_event);

  view_add(coverview, playbtn);

  view_t* nextbtn = view_new("nextbtnview", (vframe_t){0, 60, 45, 45}, 0);
  tg_bitmap_add(nextbtn, playpath, NULL);
  view_add(coverview, nextbtn);

  view_t* prevbtn = view_new("prevbtnview", (vframe_t){110, 60, 45, 45}, 0);
  tg_bitmap_add(prevbtn, playpath, NULL);
  view_add(coverview, prevbtn);

  view_t* repeatbtn = view_new("repeatbtnview", (vframe_t){0, 0, 45, 45}, 0);
  tg_bitmap_add(repeatbtn, playpath, NULL);
  view_add(coverview, repeatbtn);

  view_t* shufflebtn = view_new("shufflebtnview", (vframe_t){110, 0, 45, 45}, 0);
  tg_bitmap_add(shufflebtn, playpath, NULL);
  view_add(coverview, shufflebtn);

  // seek bar

  view_t* seekbar = view_new("seekbar", (vframe_t){2, 2, 754, 40}, 0);
  tg_text_add(seekbar, 0xFEFEFEFF, 0x000000FF, "1:56 ----|-- 3:21");

  view_add(visuals, seekbar);

  // search bar

  view_t* searchbar = view_new("searchbar", (vframe_t){2, 196, 376, 40}, 0);
  tg_text_add(searchbar, 0xFEFEFEFF, 0x000000FF, "Search...");
  view_add(visuals, searchbar);

  // evets bar

  view_t* eventbar = view_new("eventbar", (vframe_t){380, 196, 376, 40}, 0);
  tg_text_add(eventbar, 0xFEFEFEFF, 0x000000FF, "Event log");
  view_add(visuals, eventbar);

  ui_manager_add(songlist);
  ui_manager_add(visuals);

  /* view_t*   chessview = view_new("chessview", (vframe_t){100, 100, 300, 300}, 0); */
  /* bm_t*     chessbmp  = bm_new(300, 300); */
  /* uint32_t* data      = (uint32_t*)chessbmp->data; */
  /* for (int col = 0; col < 300; col++) */
  /* { */
  /*   for (int row = 0; row < 300; row++) */
  /*   { */
  /*     uint32_t index = row * 300 + col; */
  /*     uint32_t color = (row % 2 == 0 && col % 2 == 0) ? 0xFFFFFFFF : 0x000000FF; */
  /*     data[index]    = color; */
  /*   } */
  /* } */
  /* view_set_texture(chessview, chessbmp); */
  /* eh_drag_add(chessview); */

  /* view_t* texmapview = view_new("texmapview", (vframe_t){50, 500, 200, 200}, 0); */
  /* eh_drag_add(texmapview); */
  /* tg_texmap_add(texmapview); */

  //ui_manager_add(texmapview);
  //ui_manager_add(chessview);
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
