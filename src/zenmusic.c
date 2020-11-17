#include "common.c"
#include "eh_drag.c"
#include "eh_list.c"
#include "eh_text.c"
#include "eh_video.c"
#include "eh_visu.c"
#include "font.c"
#include "mtcstring.c"
#include "mtgraphics.c"
#include "mtmath4.c"
#include "player.c"
#include "songitem.c"
#include "tg_bitmap.c"
#include "tg_color.c"
#include "tg_texmap.c"
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
    nftw("/usr/home/milgra/Projects/zenmusic/res/med", display_info, 20, flags);
    printf("file count %i\n", files->length);
  }
  if (index < 0)
    return NULL; // no items over 0
  if (index > 2)
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

  mtvec_t* views = view_gen_load(htmlpath, csspath);
  view_t*  base  = mtvec_head(views);

  common_font = font_alloc(fontpath);

  ui_manager_init(width, height);
  ui_manager_add(base);

  view_t* texmapview       = view_new("texmapview", (vframe_t){50, 50, 200, 200});
  texmapview->texture.full = 1;
  ui_manager_add(texmapview);

  view_t* songlist = view_get_subview(base, "songlist");
  eh_list_add(songlist, songlist_item_generator);

  view_t* video = view_get_subview(base, "video");
  eh_video_add(video);

  view_t* left = view_get_subview(base, "left");
  eh_visu_add(left, 0);

  view_t* right = view_get_subview(base, "right");
  eh_visu_add(left, 1);

  view_t* header = view_get_subview(base, "header");
  //header->texture.blur = 1;
  //header->texture.shadow = 1;

  /* mtmap_describe(view_structure); */
  /* mtmap_describe(view_styles); */

  /* coverview = view_new("coverview", (vframe_t){0, 0, 150, 150}); */
  /* tg_color_add(coverview, 0x000000FF); */
  /* view_set_layout(coverview, (vlayout_t){.margin_top = 10.0, .margin_left = 320.0}); */

  /* char*   playpath = mtcstr_fromformat("%s/../res/play.png", respath, NULL); */
  /* view_t* playbtn  = view_new("playbtnview", (vframe_t){0, 40, 80, 80}); */
  /* tg_bitmap_add(playbtn, playpath, NULL, "play.png"); */
  /* view_set_layout(playbtn, (vlayout_t){.margin = INT_MAX}); */

  /* eh_touch_add(playbtn, NULL, playbtn_event); */

  /* view_add(coverview, playbtn); */

  /* view_t* nextbtn = view_new("nextbtnview", (vframe_t){0, 60, 45, 45}); */
  /* tg_bitmap_add(nextbtn, playpath, NULL, "next.png"); */
  /* view_add(coverview, nextbtn); */

  /* view_t* prevbtn = view_new("prevbtnview", (vframe_t){110, 60, 45, 45}); */
  /* tg_bitmap_add(prevbtn, playpath, NULL, "prev.png"); */
  /* view_add(coverview, prevbtn); */

  /* view_t* repeatbtn = view_new("repeatbtnview", (vframe_t){0, 0, 45, 45}); */
  /* tg_bitmap_add(repeatbtn, playpath, NULL, "repeat.png"); */
  /* view_add(coverview, repeatbtn); */

  /* view_t* shufflebtn = view_new("shufflebtnview", (vframe_t){110, 0, 45, 45}); */
  /* tg_bitmap_add(shufflebtn, playpath, NULL, "shuffle.png"); */
  /* view_add(coverview, shufflebtn); */

  /* // seek bar */

  /* view_t* seekbarbase = view_new("seekbarbase", (vframe_t){1, -50, 752, 30}); */
  /* tg_color_add(seekbarbase, 0xFFFFFFFF); */
  /* //seekbarbase->blur   = 1; */
  /* seekbarbase->texture.shadow = 1; */

  /* view_t* seekbar = view_new("seekbar", (vframe_t){0, 0, 752, 30}); */
  /* tg_text_add(seekbar, 0xFFFFFF00, 0x000000FF, "1:56 ----|-- 3:21"); */

  /* view_add(seekbarbase, seekbar); */
  /* view_add(visuals, seekbarbase); */

  /* // search bar */

  /* view_t* searchbar = view_new("searchbar", (vframe_t){1, 180, 375, 30}); */
  /* eh_text_add(searchbar, "placeholder"); */
  /* view_add(visuals, searchbar); */

  /* // events bar */

  /* view_t* eventbar = view_new("eventbar", (vframe_t){377, 180, 375, 30}); */
  /* tg_text_add(eventbar, 0xFEFEFEFF, 0x000000FF, "Event log"); */
  /* view_add(visuals, eventbar); */

  /* ui_manager_add(songlist); */
  /* ui_manager_add(visuals); */
  /* ui_manager_add(header); */

  /* bm_t* circle = bm_new(500, 400); */
  /* mtgraphics_rounded_rect(circle, 30, 30, 300, 200, 10, 0xFAFAFAFF); */

  /* view_t* circleview = view_new("circleview", (vframe_t){100, 600, 500, 400}); */
  /* tg_bitmap_add(circleview, NULL, circle, "red circle"); */

  /* ui_manager_add(circleview); */

  /* view_t* videoviewbase = view_new("videoviewbase", (vframe_t){800, 600, 300, 300}); */
  /* tg_color_add(videoviewbase, 0x000000FF); */

  /* view_t* videoview = view_new("videoview", (vframe_t){0, 0, 300, 300}); */
  /* eh_video_add(videoview); */

  /* view_add(videoviewbase, videoview); */
  /* ui_manager_add(videoviewbase); */

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
