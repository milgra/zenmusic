#include "activity.c"
#include "common.c"
#include "cr_text.c"
#include "db.c"
#include "editor.c"
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
#include "vh_list_item.c"
#include "vh_text.c"
#include "view.c"
#include "view_generator.c"
#include "view_layout.c"
#include "view_util.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* libpath = "/home/milgra/Music";

view_t* songlist;
view_t* coverview;
view_t* baseview;
view_t* minuteview;
view_t* secondview;
view_t* display;
view_t* aboutview;
view_t* editorview;
view_t* settingsview;

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

int messageitem_index = 0;

view_t* mainview;
view_t* display;
view_t* messagelistback;
view_t* filterlistback;
view_t* filterlist;

char     song_refr_flag = 0;
uint32_t song_recv_time = 0;

char*  fontpath;
vec_t* files;
map_t* db;
vec_t* songs;
vec_t* genres;
vec_t* artists;
ch_t*  libch;

uint32_t selected_index = UINT32_MAX;
uint32_t selected_color = 0x55FF55FF;

vec_t* songlist_fields;

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

void toggle_pause()
{
  int state = player_toggle_pause();

  if (state)
    selected_color = 0xFF5555FF;
  else
    selected_color = 0x55FF55FF;

  view_t* item = vh_list_item_for_index(songlist, selected_index);

  if (item) songitem_select(item, selected_index, songs->data[selected_index], fontpath, songlist_fields, selected_color);
}

void play_button_pushed(view_t* view)
{
  toggle_pause();
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

void edit_button_pushed(view_t* view, void* data)
{
  if (editorview->parent)
    view_remove(mainview, editorview);
  else
    view_add(mainview, editorview);
}

void prev_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex - 1;
  if (lastindex < 0) lastindex = 0;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = songs->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artist, (char*)MGET(songmap, "artist"), ts);

  player_play(MGET(songmap, "path"));
}

void next_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex + 1;
  if (lastindex == songs->length) lastindex = files->length - 1;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = songs->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artist, (char*)MGET(songmap, "artist"), ts);

  player_play(MGET(songmap, "path"));
}

void rand_button_pushed(view_t* view, void* data)
{
  lastindex = rand() % songs->length;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = songs->data[lastindex];
  tg_text_set(song, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artist, (char*)MGET(songmap, "artist"), ts);

  player_play(MGET(songmap, "path"));
}

void about_button_pushed(view_t* view, void* data)
{
  if (aboutview->parent)
    view_remove(mainview, aboutview);
  else
    view_add(mainview, aboutview);
}

void settings_button_pushed(view_t* view, void* data)
{
  if (settingsview->parent)
    view_remove(mainview, settingsview);
  else
    view_add(mainview, settingsview);
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

void songitem_on_select(view_t* view, uint32_t index, ev_t ev)
{
  printf("select button %i\n", ev.button);

  if (ev.button == 1)
  {

    // deselect prev item
    view_t* olditem = vh_list_item_for_index(songlist, selected_index);

    if (olditem)
    {
      songitem_update(olditem, selected_index, songs->data[selected_index], fontpath, songlist_fields);
    }

    // indicate list item
    view_t* newitem = vh_list_item_for_index(songlist, index);

    if (newitem)
    {
      songitem_select(newitem, index, songs->data[index], fontpath, songlist_fields, selected_color);
    }
    selected_index = index;

    // update display
    map_t* songmap = songs->data[index];

    textstyle_t ts = {0};
    ts.font        = fontpath;
    ts.align       = TA_CENTER;
    ts.size        = 25.0;
    ts.textcolor   = 0x555555FF;
    ts.backcolor   = 0;

    tg_text_set(song, (char*)MGET(songmap, "title"), ts);
    tg_text_set(artist, (char*)MGET(songmap, "artist"), ts);

    char buff[100];
    snprintf(buff, 100, "%s/%s/%s", (char*)MGET(songmap, "date"), (char*)MGET(songmap, "genre"), "192Kb/s");
    tg_text_set(info, buff, ts);

    // LOG started playing xy

    player_play(MGET(songmap, "path"));
  }
  else if (ev.button == 3)
  {
    view_add(mainview, editorview);
  }
}

void on_artistitem_select(view_t* view, uint32_t index, ev_t ev)
{
  printf("on_artistitem_select\n");
}

view_t* artistlist_create_item(view_t* listview)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "artistlist_item%i", messageitem_index++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  //rowview->hidden = 1;

  vh_litem_add(rowview, 35, on_artistitem_select);
  vh_litem_add_cell(rowview, "artist", 230, cr_text_add, cr_text_upd);

  return rowview;
}

int artistlist_update_item(view_t* listview, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= artists->length)
    return 1; // no more items

  *item_count = artists->length;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  vh_litem_upd_cell(item, "artist", &((cr_text_data_t){.style = ts, .text = artists->data[index]}));

  return 0;
}

void on_genreitem_select(view_t* view, uint32_t index, ev_t ev)
{
  printf("on_genreitem_select\n");
}

view_t* genrelist_create_item(view_t* listview)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "genrelist_item%i", messageitem_index++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, on_genreitem_select);
  vh_litem_add_cell(rowview, "genre", 230, cr_text_add, cr_text_upd);

  return rowview;
}

int genrelist_update_item(view_t* listview, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= genres->length)
    return 1; // no more items

  *item_count = genres->length;

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_RIGHT;
  ts.margin_right = 20;
  ts.size         = 25.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0xFFFFFFFF;

  vh_litem_upd_cell(item, "genre", &((cr_text_data_t){.style = ts, .text = genres->data[index]}));

  return 0;
}

view_t* songlist_create_item(view_t* listview)
{
  return songitem_new(fontpath, songitem_on_select, songlist_fields);
}

int songlist_update_item(view_t* listview, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= songs->length)
    return 1; // no more items

  *item_count = songs->length;

  if (selected_index == index)
  {
    songitem_select(item, index, songs->data[index], fontpath, songlist_fields, selected_color);
  }
  else
  {
    songitem_update(item, index, songs->data[index], fontpath, songlist_fields);
  }
  return 0;
}

void sort(char* field)
{
  db_sort(db, songs, field);
  db_genres(db, genres);
  db_artists(songs, artists);
  //vh_list_fill(songlist);
  // remove cache as subviews
  /* view_t* row; */
  /* while ((row = VNXT(songitem_cache))) view_remove(songlist, row); */
  //vec_reset(songitem_cache);
  vh_list_reset(songlist);
}

void on_header_field_select(view_t* view, char* id, ev_t ev)
{
  printf("on_header_field_select %s\n", id);
  // filter db by field id
  sort(id);
}

void on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sitem_cell_t* cell = songlist_fields->data[src];
  RET(cell);
  VREM(songlist_fields, cell);
  vec_addatindex(songlist_fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(songlist);
  while ((item = VNXT(cache)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
  vec_t* items = vh_list_items(songlist);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < songlist_fields->length; i++)
  {
    sitem_cell_t* cell = songlist_fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }
  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(songlist);
  while ((item = VNXT(cache)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
  vec_t* items = vh_list_items(songlist);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

void filter(view_t* view, str_t* text)
{
  char* word = str_cstring(text);
  db_filter(db, word, songs);
  REL(word);
  vh_list_reset(songlist);
}

void filter_onactivate(view_t* view)
{
}

void genrebtn_pushed(view_t* view, void* data)
{
  if (filterlistback->parent)
    view_remove(mainview, filterlistback);
  else
    view_add(mainview, filterlistback);
}

void messagesbtn_pushed(view_t* view, void* data)
{
  if (messagelistback->parent)
    view_remove(mainview, messagelistback);
  else
    view_add(mainview, messagelistback);
}

void init(int width, int height)
{
  srand((unsigned int)time(NULL));

  char* respath  = SDL_GetBasePath();
  char* csspath  = cstr_fromformat("%s/../res/main.css", respath, NULL);
  char* htmlpath = cstr_fromformat("%s/../res/main.html", respath, NULL);
  fontpath       = cstr_fromformat("%s/../res/Avenir.ttc", respath, NULL);

  vec_t* views = view_gen_load(htmlpath, csspath, respath);
  baseview     = vec_head(views);

  // layout to starter size
  view_set_frame(baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(baseview);

  common_respath = respath;

  text_init();

  activity_init();

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  songlist = view_get_subview(baseview, "songlist");

  vh_list_add(songlist, songlist_create_item, songlist_update_item);

  songlist_fields = VNEW();
  VADD(songlist_fields, sitem_cell_new("index", 50, 0));
  VADD(songlist_fields, sitem_cell_new("artist", 300, 1));
  VADD(songlist_fields, sitem_cell_new("title", 300, 2));
  VADD(songlist_fields, sitem_cell_new("date", 150, 3));
  VADD(songlist_fields, sitem_cell_new("genre", 150, 4));
  VADD(songlist_fields, sitem_cell_new("track", 150, 5));
  VADD(songlist_fields, sitem_cell_new("disc", 150, 6));
  VADD(songlist_fields, sitem_cell_new("plays", 150, 7));
  VADD(songlist_fields, sitem_cell_new("added", 150, 8));
  VADD(songlist_fields, sitem_cell_new("last played", 150, 9));
  VADD(songlist_fields, sitem_cell_new("last skipped", 150, 10));

  //display         = view_get_subview(baseview, "display");
  messagelistback     = view_get_subview(baseview, "messagelistback");
  view_t* messagelist = view_get_subview(baseview, "messagelist");

  activity_attach(messagelist, fontpath);

  view_t* genrelist = view_get_subview(baseview, "genrelist");
  vh_list_add(genrelist, genrelist_create_item, genrelist_update_item);

  view_t* artistlist = view_get_subview(baseview, "artistlist");
  vh_list_add(artistlist, artistlist_create_item, artistlist_update_item);

  mainview = view_get_subview(baseview, "main");

  view_remove(mainview, messagelistback);

  filterlistback = view_get_subview(baseview, "filterlistback");
  view_remove(mainview, filterlistback);

  /* filterlist = view_get_subview(baseview, "filterlist"); */

  /* view_remove(mainview, filterlist); */

  // decrease retain count of cells because of inline allocation
  vec_dec_retcount(songlist_fields);

  minuteview              = view_get_subview(baseview, "minute");
  minuteview->needs_touch = 0;

  secondview              = view_get_subview(baseview, "second");
  secondview->needs_touch = 0;

  tg_text_add(minuteview);

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_RIGHT;
  ts.size        = 25.0;
  ts.textcolor   = 0x555555FF;
  ts.backcolor   = 0x0;

  tg_text_set(minuteview, "00:", ts);

  tg_text_add(secondview);

  ts.align = TA_LEFT;

  tg_text_set(secondview, "00", ts);

  song              = view_get_subview(baseview, "song");
  song->needs_touch = 0;

  ts.size = 25.0;

  tg_text_add(song);
  tg_text_set(song, "-", ts);

  artist              = view_get_subview(baseview, "artist");
  artist->needs_touch = 0;

  tg_text_add(artist);
  tg_text_set(artist, "-", ts);

  info = view_get_subview(baseview, "info");
  tg_text_add(info);
  tg_text_set(info, "-", ts);

  ts.size = 20.0;

  //tg_text_add(info, "-", ts);

  view_t* filterbar = view_get_subview(baseview, "filterfield");
  tg_css_add(filterbar);

  filterbar->layout.background_color = 0xFFFFFFFF;
  vh_text_add(filterbar, "Search/Query", fontpath, filter, filter_onactivate);

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

  view_t* main      = view_get_subview(baseview, "main");
  main->needs_touch = 0;

  view_t* settingsbtn = view_get_subview(baseview, "settingsicon");
  vh_button_add(settingsbtn, NULL, settings_button_pushed);

  settingsview = view_get_subview(baseview, "settingsback");

  ts.align     = TA_CENTER;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;
  ts.size      = 25.0;

  view_t* settings = view_get_subview(baseview, "settings");

  char* settingsinfo = "Library location:\n Change library\n Organize library X\n Change color\n";

  tg_text_add(settings);
  tg_text_set(settings, settingsinfo, ts);

  view_remove(main, settingsview);

  view_t* editbtn = view_get_subview(baseview, "eventsicon");
  vh_button_add(editbtn, NULL, edit_button_pushed);

  view_t* aboutbtn = view_get_subview(baseview, "abouticon");
  aboutview        = view_get_subview(baseview, "aboutback");

  vh_button_add(aboutbtn, NULL, about_button_pushed);

  ts.align     = TA_CENTER;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;
  ts.size      = 25.0;

  view_t* about = view_get_subview(baseview, "about");

  char* info = "Zen Music by Milan Toth\nFree and Open Source Software.\n"
               "If you like Zen Music, please consider donating\n www.paypal.me/milgra\n"
               "Powered by FreeBSD, Emacs and C";

  tg_text_add(about);
  tg_text_set(about, info, ts);

  view_remove(main, aboutview);

  editorview     = view_get_subview(baseview, "ideditorback");
  view_t* editor = view_get_subview(baseview, "ideditor");

  view_remove(main, editorview);

  editor_attach(editor, fontpath);

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
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0x00000000;

  view_t* maxbtn   = view_get_subview(baseview, "maxicon");
  view_t* closebtn = view_get_subview(baseview, "closeicon");

  vh_button_add(maxbtn, NULL, max_button_pushed);
  vh_button_add(closebtn, NULL, close_button_pushed);

  ts.align = TA_CENTER;

  view_t* genrebtn = view_get_subview(baseview, "genrebtn");
  vh_button_add(genrebtn, NULL, genrebtn_pushed);
  tg_text_add(genrebtn);
  tg_text_set(genrebtn, "filters", ts);

  view_t* messagesbtn = view_get_subview(baseview, "messagesbtn");
  vh_button_add(messagesbtn, NULL, messagesbtn_pushed);
  tg_text_add(messagesbtn);
  tg_text_set(messagesbtn, "activity", ts);

  view_t* songlistheader = view_get_subview(baseview, "songlistheader");

  ts.font        = fontpath;
  ts.align       = 0;
  ts.margin_left = 10;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xEFEFEFFF;

  vh_lhead_add(songlistheader, 30, on_header_field_select, on_header_field_insert, on_header_field_resize);

  sitem_cell_t* cell;
  while ((cell = VNXT(songlist_fields)))
  {
    vh_lhead_add_cell(songlistheader, cell->id, cell->size, cr_text_upd);
    vh_lhead_upd_cell(songlistheader, cell->id, cell->size, &((cr_text_data_t){.style = ts, .text = cell->id}));
  }

  /* view_t* texmap        = view_new("texmap", ((r2_t){0, 0, 300, 300})); */
  /* texmap->needs_touch   = 0; */
  /* texmap->display       = 1; */
  /* texmap->texture.full  = 1; */
  /* texmap->layout.right  = 1; */
  /* texmap->layout.bottom = 1; */

  /* ui_manager_add(texmap); */

  view_t* footer  = view_get_subview(baseview, "footer");
  footer->display = 0;

  db    = MNEW();
  libch = ch_new(100);

  songs   = VNEW();
  artists = VNEW();
  genres  = VNEW();

  LOG("using database : %s", libpath);

  // read db
  db_read(libpath, db);

  LOG("database loaded, entries : %i", db->count);

  // read library
  lib_read(libpath);

  LOG("library scanned, files : %i", lib_entries());

  // remove existing
  lib_remove_duplicates(db);

  LOG("new files detected : %i", lib_entries());

  if (lib_entries() > 0)
  {
    LOG("analyzing entires...");
    // start analyzing new entries
    lib_analyze(libch);
  }

  sort("artist");
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

      textstyle_t ts = {0};
      ts.font        = fontpath;
      ts.align       = TA_RIGHT;
      ts.size        = 25.0;
      ts.textcolor   = 0x555555FF;
      ts.backcolor   = 0;

      char timebuff[20];
      snprintf(timebuff, 20, "%.2i:", (int)floor(lasttime / 60.0));
      tg_text_set(minuteview, timebuff, ts);
      snprintf(timebuff, 20, "%.2i", (int)lasttime % 60);
      ts.align = TA_LEFT;
      tg_text_set(secondview, timebuff, ts);

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

      sort("artist");

      db_write(libpath, db);

      LOG("done, saving database");
      LOG("done, organizing database");

      int succ = lib_organize(libpath, db);
      if (succ == 0)
      {
        // save db
        db_write(libpath, db);
        LOG("done, database saved");
      }
    }
  }

  else if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_SPACE)
    {
      toggle_pause();
    }
  }

  // update ui
  ui_manager_event(ev);
}

void render(uint32_t time)
{
  ui_manager_render(time);
}

void destroy()
{
}

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}
