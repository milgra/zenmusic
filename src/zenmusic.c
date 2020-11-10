#include "common.c"
#include "eh_drag.c"
#include "eh_list.c"
#include "eh_text.c"
#include "font.c"
#include "mtcstring.c"
#include "mtdrawer.c"
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
view_t*  coverview;

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

    bm_t* bitmap = player_get_album(files->data[index]);
    tg_bitmap_add(coverview, NULL, bitmap);

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
  tg_color_add(songlist, 0xFFFFFFFF);
  eh_list_add(songlist, songlist_item_generator);

  view_set_layout(songlist, (vlayout_t){
                                .margin_right = 300.0,
                                .w_per        = 1.0,
                                .h_per        = 1.0});

  // visualization views

  view_t* visuals = view_new("visuals", (vframe_t){0, 0, 790, 170}, 0);
  view_set_layout(visuals, (vlayout_t){
                               .margin_top = 60.0,
                               .margin     = INT_MAX});

  //visuals->blur   = 1;
  visuals->shadow = 1;

  view_t* visualscolor = view_new("visualscolor", (vframe_t){0, 0, 790, 170}, 0);
  tg_color_add(visualscolor, 0xFFFFFFFF);

  view_t* videoview_left_base = view_new("videoviewleftbase", (vframe_t){10, 0, 300, 150}, 0);
  tg_color_add(videoview_left_base, 0x000000FF);
  view_set_layout(videoview_left_base, (vlayout_t){
                                           .margin_top  = 10.0,
                                           .margin_left = 10.0});

  view_t* videoview_left = view_new("videoviewleft", (vframe_t){0, 0, 300, 150}, 1);
  tg_video_add(videoview_left, 1280, 720);
  view_add(videoview_left_base, videoview_left);

  view_t* videoview_right_base = view_new("videoviewrightbase", (vframe_t){0, 0, 300, 150}, 0);
  tg_color_add(videoview_right_base, 0x000000FF);
  view_set_layout(videoview_right_base, (vlayout_t){
                                            .margin_top   = 10.0,
                                            .margin_right = 10.0});

  view_t* videoview_right = view_new("videoviewright", (vframe_t){0, 0, 300, 150}, 1);
  tg_video_add(videoview_right, 1280, 720);
  view_add(videoview_right_base, videoview_right);

  coverview = view_new("coverview", (vframe_t){0, 0, 150, 150}, 0);
  tg_color_add(coverview, 0x000000FF);
  view_set_layout(coverview, (vlayout_t){
                                 .margin_top  = 10.0,
                                 .margin_left = 320.0});

  view_add(visualscolor, videoview_left_base);
  view_add(visualscolor, videoview_right_base);
  view_add(visualscolor, coverview);
  view_add(visuals, visualscolor);

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

  view_t* seekbarbase = view_new("seekbarbase", (vframe_t){1, -50, 752, 30}, 0);
  tg_color_add(seekbarbase, 0xFFFFFFFF);
  //seekbarbase->blur   = 1;
  seekbarbase->shadow = 1;

  view_t* seekbar = view_new("seekbar", (vframe_t){0, 0, 752, 30}, 0);
  tg_text_add(seekbar, 0xFFFFFF00, 0x000000FF, "1:56 ----|-- 3:21");

  view_add(seekbarbase, seekbar);
  view_add(visuals, seekbarbase);

  // search bar

  view_t* searchbar = view_new("searchbar", (vframe_t){1, 180, 375, 30}, 0);
  eh_text_add(searchbar, "placeholder");
  view_add(visuals, searchbar);

  // events bar

  view_t* eventbar = view_new("eventbar", (vframe_t){377, 180, 375, 30}, 0);
  tg_text_add(eventbar, 0xFEFEFEFF, 0x000000FF, "Event log");
  view_add(visuals, eventbar);

  ui_manager_add(songlist);
  ui_manager_add(visuals);

  bm_t* circle = bm_new(151, 151);
  mtdrawer_circle(circle, 75.5, 75.5, 45.0, 0xFF0000FF);

  view_t* circleview = view_new("circleview", (vframe_t){600, 600, 151, 151}, 0);
  tg_bitmap_add(circleview, NULL, circle);

  bm_t* chessbmp = bm_new(300, 300);
  mtdrawer_tile(chessbmp);

  view_t* chessview = view_new("chessview", (vframe_t){600, 800, 300, 300}, 0);
  tg_bitmap_add(chessview, NULL, chessbmp);

  ui_manager_add(circleview);
  ui_manager_add(chessview);

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
