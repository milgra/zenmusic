#include "common.c"
#include "db.c"
#include "eh_button.c"
#include "eh_knob.c"
#include "eh_list.c"
#include "eh_text.c"
#include "eh_video.c"
#include "font.c"
#include "lib.c"
#include "mtchannel.c"
#include "mtcstring.c"
#include "mtmap.c"
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

view_t* songlist;
view_t* coverview;
view_t* baseview;
view_t* timeview;
view_t* visuleft;
view_t* visuright;
double  lasttime = 0.0;
view_t* playbtn;
view_t* volbtn;
size_t  lastindex = 0;
int     loop_all  = 0;

char     song_refr_flag = 0;
uint32_t song_recv_time = 0;

mtvec_t* files;
mtmap_t* db;
mtvec_t* sorted;
mtch_t*  libch;

void songitem_event(view_t* view, void* data)
{
  lastindex = (size_t)data;
  // printf("songitem event %i %i %s\n", ev.type, index, (char*)files->data[index]);

  mtmap_t* songmap = sorted->data[lastindex];

  view_t* song = view_get_subview(baseview, "song");
  tg_text_add(song, 0x00000000, 0x000000FF, (char*)MGET(songmap, "title"), 0);

  //bm_t* bitmap = player_get_album(files->data[lastindex]);
  //tg_bitmap_add(coverview, NULL, bitmap, "album");

  player_play(MGET(songmap, "path"));
}

void seek_ratio_changed(view_t* view, float angle)
{
  float ratio = 0.0;
  if (angle > 0 && angle < 3.14 * 3 / 2)
  {
    ratio = angle / 6.28 + 0.25;
  }
  else if (angle > 3.14 * 3 / 2)
  {
    ratio = (angle - (3.14 * 3 / 2)) / 6.28;
  }

  player_set_position(ratio);
}

void play_button_pushed(view_t* view)
{

  player_toggle_pause();
}

void vol_ratio_changed(view_t* view, float angle)
{
  float ratio = 0.0;
  if (angle > 0 && angle < 3.14 * 3 / 2)
  {
    ratio = angle / 6.28 + 0.25;
  }
  else if (angle > 3.14 * 3 / 2)
  {
    ratio = (angle - (3.14 * 3 / 2)) / 6.28;
  }

  player_set_volume(ratio);
}

void mute_button_pushed(view_t* view)
{
  player_toggle_mute();
}

void prev_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex - 1;
  if (lastindex < 0) lastindex = 0;

  mtmap_t* songmap = sorted->data[lastindex];

  view_t* song = view_get_subview(baseview, "song");
  tg_text_add(song, 0x00000000, 0x000000FF, (char*)MGET(songmap, "title"), 0);

  player_play(MGET(songmap, "path"));
}

void next_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex + 1;
  if (lastindex == sorted->length) lastindex = files->length - 1;

  mtmap_t* songmap = sorted->data[lastindex];

  view_t* song = view_get_subview(baseview, "song");
  tg_text_add(song, 0x00000000, 0x000000FF, (char*)MGET(songmap, "title"), 0);

  player_play(MGET(songmap, "path"));
}

void rand_button_pushed(view_t* view, void* data)
{
  lastindex = rand() % sorted->length;

  mtmap_t* songmap = sorted->data[lastindex];

  view_t* song = view_get_subview(baseview, "song");
  tg_text_add(song, 0x00000000, 0x000000FF, (char*)MGET(songmap, "title"), 0);

  player_play(MGET(songmap, "path"));
}

void loop_button_pushed(view_t* view, void* data)
{
  printf("LOOP\n");
  loop_all = !loop_all;
}

int comp_artist(void* left, void* right)
{
  mtmap_t* l = left;
  mtmap_t* r = right;

  char* la = MGET(l, "artist");
  char* ra = MGET(r, "artist");

  return strcmp(la, ra);
}

view_t* songlist_item_generator(view_t* listview, view_t* rowview, int index)
{
  if (index < 0)
    return NULL; // no items over 0
  if (index >= sorted->length)
    return NULL;
  if (rowview == NULL)
    rowview = songitem_new();

  songitem_update(rowview, index, sorted->data[index], songitem_event);
  return rowview;
}

void sort()
{
  if (sorted) REL(sorted);
  sorted = mtmap_values(db);
  mtvec_sort(sorted, comp_artist);
  eh_list_fill(songlist);
  printf("COUNT %i\n", sorted->length);
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

  songlist = view_get_subview(baseview, "songlist");
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
  //songitem_update(headeritem, -1, "Artist", NULL);
  view_add(songlistheader, headeritem);

  playbtn = view_get_subview(baseview, "playbtn");
  tg_knob_add(playbtn);
  eh_knob_add(playbtn, seek_ratio_changed, play_button_pushed);

  volbtn = view_get_subview(baseview, "volbtn");
  tg_knob_add(volbtn);
  eh_knob_add(volbtn, vol_ratio_changed, mute_button_pushed);

  view_t* prevbtn = view_get_subview(baseview, "prevbtn");
  view_t* nextbtn = view_get_subview(baseview, "nextbtn");
  view_t* randbtn = view_get_subview(baseview, "randbtn");
  view_t* loopbtn = view_get_subview(baseview, "loopbtn");

  eh_button_add(prevbtn, NULL, prev_button_pushed);
  eh_button_add(nextbtn, NULL, next_button_pushed);
  eh_button_add(randbtn, NULL, rand_button_pushed);
  eh_button_add(loopbtn, NULL, loop_button_pushed);

  view_t* main      = view_get_subview(baseview, "main");
  main->needs_touch = 0;

  view_t* header = view_get_subview(baseview, "header");
  //header->texture.blur = 1;
  //header->texture.shadow = 1;

  visuleft  = view_get_subview(baseview, "visuleft");
  visuright = view_get_subview(baseview, "visuright");

  /* view_t* texmapview       = view_new("texmapview", (r2_t){500, 500, 200, 200}); */
  /* texmapview->texture.full = 1; */
  /* ui_manager_add(texmapview); */

  db    = MNEW();
  libch = mtch_new(100);

  // read db
  db_read(db);

  // read library
  lib_read();

  // remove existing
  lib_remove_duplicates(db);

  // start analyzing new entries
  lib_analyze(libch);

  sort();
}

void update(ev_t ev)
{
  // update time, knobs and visualizer
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
      tg_knob_set_angle(playbtn, posratio * 6.28 - 3.14 / 2.0);

      // TODO remove hack
      // TODO use pi constant
      double volume = player_volume() - 0.0001; // hack for knob limit
      tg_knob_set_angle(volbtn, volume * 6.28 - 3.14 / 2.0);
    }

    // update visualizer
    player_draw_waves(visuleft->texture.page, 0, visuleft->texture.bitmap);
    player_draw_waves(visuright->texture.page, 1, visuright->texture.bitmap);
    visuleft->texture.changed  = 1;
    visuright->texture.changed = 1;
  }
  // get analyzed song entries
  mtmap_t* entry;
  while ((entry = mtch_recv(libch)))
  {
    char* path = MGET(entry, "path");
    MPUT(db, path, entry);
    REL(entry);
    song_refr_flag = 1;
    song_recv_time = ev.time;
  }
  // reload
  if (song_refr_flag)
  {
    if (ev.time > song_recv_time + 3000)
    {
      song_refr_flag = 0;
      sort();
      db_write(db);
    }
  }
  // update ui
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
