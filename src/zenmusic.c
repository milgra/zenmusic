#include "common.c"
#include "cr_text.c"
#include "db.c"
#include "lib.c"
#include "mtchannel.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "player.c"
#include "songitem.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "vh_button.c"
#include "vh_knob.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_text.c"
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

view_t* song;
view_t* artist;
view_t* info;

view_t* visuleft;
view_t* visuright;
view_t* visuvideo;
double  lasttime = 0.0;
view_t* playbtn;
view_t* volbtn;
size_t  lastindex = 0;
int     loop_all  = 0;

char     song_refr_flag = 0;
uint32_t song_recv_time = 0;

char*  fontpath;
vec_t* files;
map_t* db;
vec_t* vec_srt;
ch_t*  libch;

vec_t* songlist_fields;
vec_t* songitem_cache;

void songitem_on_select(view_t* view, uint32_t index)
{
  map_t* songmap = vec_srt->data[index];

  tg_text_set(song, (char*)MGET(songmap, "title"));
  tg_text_set(artist, (char*)MGET(songmap, "artist"));

  // LOG started playing xy

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

  map_t* songmap = vec_srt->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"));
  tg_text_set(artist, (char*)MGET(songmap, "artist"));

  player_play(MGET(songmap, "path"));
}

void next_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex + 1;
  if (lastindex == vec_srt->length) lastindex = files->length - 1;

  map_t* songmap = vec_srt->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"));
  tg_text_set(artist, (char*)MGET(songmap, "artist"));

  player_play(MGET(songmap, "path"));
}

void rand_button_pushed(view_t* view, void* data)
{
  lastindex = rand() % vec_srt->length;

  map_t* songmap = vec_srt->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"));
  tg_text_set(artist, (char*)MGET(songmap, "artist"));

  player_play(MGET(songmap, "path"));
}

void loop_button_pushed(view_t* view, void* data)
{
  loop_all = !loop_all;
}

void max_button_pushed(view_t* view, void* data)
{
  wm_toggle_fullscreen();
}
void close_button_pushed(view_t* view, void* data)
{
  wm_close();
}

void songlist_item_recycler(view_t* listview, view_t* rowview)
{
  VADD(songitem_cache, rowview);
}

view_t* songlist_item_generator(view_t* listview, int index, int* count)
{
  if (index < 0)
    return NULL; // no items over 0
  if (index >= vec_srt->length)
    return NULL;

  view_t* rowview = vec_head(songitem_cache);
  if (rowview)
    VREM(songitem_cache, rowview);
  else
    rowview = songitem_new(fontpath, songitem_on_select);

  *count = vec_srt->length;

  songitem_update(rowview, index, vec_srt->data[index], fontpath);
  return rowview;
}

void on_select(view_t* view, char* id)
{
  printf("on_select %s\n", id);
  // filter db by field id
}

void on_insert(view_t* view, char* src_id, char* tgt_id)
{
  printf("on_insert %s %s\n", src_id, tgt_id);
}

void on_resize(view_t* view, char* id, int width)
{
  printf("on_resize %s %i\n", id, width);
  for (int i = 0; i < songlist_fields->length; i++)
  {
    sitem_cell_t* cell = songlist_fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = width;
      break;
    }
  }
}

void filter(view_t* view, str_t* text)
{
  char* word = str_cstring(text);
  db_filter(db, word, vec_srt);
  REL(word);
  vh_list_reset(songlist);
}

void sort()
{
  db_sort(db, vec_srt);
  vh_list_fill(songlist);
}

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));

  char* respath  = SDL_GetBasePath();
  char* csspath  = cstr_fromformat("%s/../res/main.css", respath, NULL);
  char* htmlpath = cstr_fromformat("%s/../res/main.html", respath, NULL);
  fontpath       = cstr_fromformat("%s/../res/Avenir.ttc", respath, NULL);

  vec_t* views = view_gen_load(htmlpath, csspath, respath);
  baseview     = vec_head(views);

  common_respath = respath;

  text_init();

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  songlist = view_get_subview(baseview, "songlist");

  vh_list_add(songlist, songlist_item_generator, songlist_item_recycler);

  songitem_cache = VNEW();

  songlist_fields = VNEW();
  VADD(songlist_fields, sitem_cell_new("index", 50, 0));
  VADD(songlist_fields, sitem_cell_new("artist", 300, 1));
  VADD(songlist_fields, sitem_cell_new("title", 300, 2));
  VADD(songlist_fields, sitem_cell_new("date", 150, 3));
  VADD(songlist_fields, sitem_cell_new("track", 150, 4));
  VADD(songlist_fields, sitem_cell_new("disc", 150, 5));
  // decrease retain count of cells because of inline allocation
  vec_dec_retcount(songlist_fields);

  timeview = view_get_subview(baseview, "time");

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 35.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_add(timeview, "00:00", ts);

  song = view_get_subview(baseview, "song");

  ts.size = 25.0;

  tg_text_add(song, "-", ts);

  artist = view_get_subview(baseview, "artist");

  ts.size = 25.0;

  tg_text_add(artist, "-", ts);

  //info = view_get_subview(baseview, "info");

  ts.size = 20.0;

  //tg_text_add(info, "-", ts);

  view_t* filterbar = view_get_subview(baseview, "filterfield");
  tg_css_add(filterbar);
  filterbar->layout.background_color = 0xFFFFFFFF;
  vh_text_add(filterbar, "", fontpath, filter);
  //vh_text_add(filterbar, "Search/Filter (x)", filter);

  //view_t* headeritem = songitem_new();
  //songitem_update(headeritem, -1, "Artist", NULL);
  //view_add(songlistheader, headeritem);

  playbtn = view_get_subview(baseview, "playbtn");
  tg_knob_add(playbtn);
  vh_knob_add(playbtn, seek_ratio_changed, play_button_pushed);

  volbtn = view_get_subview(baseview, "volbtn");
  tg_knob_add(volbtn);
  vh_knob_add(volbtn, vol_ratio_changed, mute_button_pushed);

  view_t* prevbtn = view_get_subview(baseview, "previcon");
  view_t* nextbtn = view_get_subview(baseview, "nexticon");
  view_t* randbtn = view_get_subview(baseview, "shuffleicon");

  vh_button_add(prevbtn, NULL, prev_button_pushed);
  vh_button_add(nextbtn, NULL, next_button_pushed);
  vh_button_add(randbtn, NULL, rand_button_pushed);

  view_t* settingsbtn = view_get_subview(baseview, "settingsicon");
  view_t* donatebtn   = view_get_subview(baseview, "donateicon");
  view_t* eventsbtn   = view_get_subview(baseview, "eventsicon");

  view_t* main      = view_get_subview(baseview, "main");
  main->needs_touch = 0;

  view_t* header = view_get_subview(baseview, "header");
  //header->texture.blur = 1;
  //header->texture.shadow = 1;

  visuleft  = view_get_subview(baseview, "visuleft");
  visuright = view_get_subview(baseview, "visuright");
  visuvideo = view_get_subview(baseview, "visuvideo");

  view_t* playicon = view_get_subview(baseview, "playicon");
  view_t* muteicon = view_get_subview(baseview, "muteicon");

  playicon->needs_touch = 0;
  muteicon->needs_touch = 0;

  /* view_t* texmapview       = view_new("texmapview", (r2_t){500, 500, 200, 200}); */
  /* texmapview->texture.full = 1; */
  /* ui_manager_add(texmapview); */

  ts.size      = 25.0;
  ts.backcolor = 0xEFEFEFFF;

  view_t* genrebtn  = view_get_subview(baseview, "genrebtn");
  view_t* artistbtn = view_get_subview(baseview, "artistbtn");

  tg_text_add(genrebtn, "genre/tags", ts);
  tg_text_add(artistbtn, "artists", ts);

  view_t* maxbtn   = view_get_subview(baseview, "maxicon");
  view_t* closebtn = view_get_subview(baseview, "closeicon");

  vh_button_add(maxbtn, NULL, max_button_pushed);
  vh_button_add(closebtn, NULL, close_button_pushed);

  view_t* songlistheader = view_get_subview(baseview, "songlistheader");

  ts.font      = fontpath;
  ts.align     = 0;
  ts.size      = 25.0;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xEFEFEFFF;

  vh_lhead_add(songlistheader, 30, on_select, on_insert, on_resize);

  vh_lhead_add_cell(songlistheader, "index", 50, cr_text_upd);
  vh_lhead_add_cell(songlistheader, "artist", 300, cr_text_upd);
  vh_lhead_add_cell(songlistheader, "title", 300, cr_text_upd);

  vh_lhead_upd_cell(songlistheader, "index", 30, &((cr_text_data_t){.style = ts, .text = "index"}));
  vh_lhead_upd_cell(songlistheader, "artist", 200, &((cr_text_data_t){.style = ts, .text = "artist"}));
  vh_lhead_upd_cell(songlistheader, "title", 300, &((cr_text_data_t){.style = ts, .text = "title"}));

  db    = MNEW();
  libch = ch_new(100);

  vec_srt = VNEW();

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
  player_refresh();

  double time = player_time();
  if (time > 0.0)
  {
    // update timer
    if (floor(time) != lasttime)
    {
      lasttime = floor(time);

      char timebuff[20];
      snprintf(timebuff, 20, "%.2i:%.2i", (int)floor(lasttime / 60.0), (int)lasttime % 60);
      tg_text_set(timeview, timebuff);

      double posratio = time / player_duration();
      tg_knob_set_angle(playbtn, posratio * 6.28 - 3.14 / 2.0);

      // TODO remove hack
      // TODO use pi constant
      double volume = player_volume() - 0.0001; // hack for knob limit
      tg_knob_set_angle(volbtn, volume * 6.28 - 3.14 / 2.0);
    }

    // update visualizer
    player_draw_waves(0, visuleft->texture.bitmap, 3);
    player_draw_waves(1, visuright->texture.bitmap, 3);
    visuleft->texture.changed  = 1;
    visuright->texture.changed = 1;

    player_draw_video(visuvideo->texture.bitmap, 3);
    visuvideo->texture.changed = 1;
  }

  // get analyzed song entries
  map_t* entry;
  while ((entry = ch_recv(libch)))
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

  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_SPACE)
    {
      // play/pause
      player_toggle_pause();
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
