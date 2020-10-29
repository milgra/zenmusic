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

  mtvec_add(files, mtcstr_fromcstring((char*)(fpath + ftwbuf->base)));

  return 0; /* To tell nftw() to continue */
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
  songitem_update(rowview, index, files->data[index]);
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
                                .margin_right = 350.0,
                                .w_per        = 1.0,
                                .h_per        = 1.0});

  // header view
  view_t* header = view_new("header", (vframe_t){0, 0, 600, 100}, 0);
  tg_color_add(header, 0xCDCDCDFF);
  view_set_layout(header, (vlayout_t){
                              .h_per        = 1.0,
                              .margin_right = 1.0});

  char*   playpath    = mtcstr_fromformat("%s/../res/play.png", respath, NULL);
  view_t* playbtnview = view_new("playbtnview", (vframe_t){0, 210, 90, 90}, 0);
  tg_bitmap_add(playbtnview, playpath);
  view_set_layout(playbtnview, (vlayout_t){
                                   .margin = INT_MAX});

  view_add(header, playbtnview);

  view_t* seekview = view_new("seekview", (vframe_t){0, 160, 580, 10}, 0);
  view_set_layout(seekview, (vlayout_t){
                                .margin = INT_MAX});
  tg_color_add(seekview, 0xFFFFFF66);

  view_t* volview = view_new("volview", (vframe_t){0, 178, 400, 10}, 0);
  view_set_layout(volview, (vlayout_t){
                               .margin = INT_MAX});
  tg_color_add(volview, 0xFFFFFF66);

  // video views
  view_t* videoview_left = view_new("videoviewleft", (vframe_t){0, 0, 296, 150}, 0);
  tg_color_add(videoview_left, 0xDDDDDDFF);
  view_set_layout(videoview_left, (vlayout_t){
                                      .margin_top  = 2.0,
                                      .margin_left = 2.0});

  view_t* videoview_right = view_new("videoviewright", (vframe_t){0, 0, 296, 150}, 0);
  tg_color_add(videoview_right, 0xDDDDDDFF);
  view_set_layout(videoview_right, (vlayout_t){
                                       .margin_top   = 2.0,
                                       .margin_right = 2.0});

  view_add(header, videoview_left);
  view_add(header, videoview_right);

  view_add(header, seekview);
  view_add(header, volview);

  ui_manager_add(songlist);
  ui_manager_add(header);

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
