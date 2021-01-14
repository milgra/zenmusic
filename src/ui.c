#ifndef ui_h
#define ui_h

#include "view.c"

void ui_init(float width, float height, char* respath, vec_t* songs, vec_t* genres, vec_t* artists);
void ui_update_position(float ratio);
void ui_update_volume(float ratio);
void ui_update_visualizer();
void ui_update_video();
void ui_update_time(double time);
void ui_toggle_pause(int state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "activity.c"
#include "editor.c"
#include "mtcstring.c"
#include "player.c"
#include "songlist.c"
#include "textlist.c"
#include "tg_knob.c"
#include "vh_button.c"
#include "vh_knob.c"
#include "vh_text.c"
#include "view_generator.c"
#include "view_layout.c"
#include "wm_connector.c"
// TODO remove fromm zenmusic.c
#include "ui_manager.c"

view_t* baseview;
view_t* minuteview;
view_t* secondview;

view_t* aboutview;
view_t* editorview;
view_t* settingsview;

view_t* songview;
view_t* infoview;
view_t* artistview;

view_t* visuleft;
view_t* visuright;
view_t* visuvideo;

view_t* playbtn;
view_t* volbtn;

view_t* mainview;
view_t* messagelistback;
view_t* filterlistback;

size_t lastindex = 0;
char*  fontpath;

struct _ui_t
{
  vec_t* songs;
} ui = {0};

void filterbtn_pushed(view_t* view, void* data)
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

void on_song_select(int index)
{
  // update display
  map_t* songmap = ui.songs->data[index];

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x555555FF;
  ts.backcolor   = 0;

  tg_text_set(songview, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artistview, (char*)MGET(songmap, "artist"), ts);

  char buff[100];
  snprintf(buff, 100, "%s/%s/%s", (char*)MGET(songmap, "date"), (char*)MGET(songmap, "genre"), "192Kb/s");
  tg_text_set(infoview, buff, ts);

  // LOG started playing xy

  player_play(MGET(songmap, "path"));
}

void on_song_edit(int index)
{
  map_t* songmap = ui.songs->data[index];

  editor_set_song(songmap);
  view_add(mainview, editorview);
}

void on_song_header(char* id)
{
  printf("on song header %s\n", id);
  // filter db by field id
  // sort(id);
}

void on_genre_select(int index)
{
  printf("on genre select %i\n", index);
}

void on_artist_select(int index)
{
  printf("on artist select %i\n", index);
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

void ui_toggle_pause(int state)
{
  songlist_toggle_selected(state);
}

void play_button_pushed(view_t* view)
{
  // toggle_pause();
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

  map_t* songmap = ui.songs->data[lastindex];
  tg_text_set(songview, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artistview, (char*)MGET(songmap, "artist"), ts);

  player_play(MGET(songmap, "path"));
}

void next_button_pushed(view_t* view, void* data)
{
  lastindex = lastindex + 1;
  // if (lastindex == ui.songs->length) lastindex = files->length - 1;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = ui.songs->data[lastindex];
  tg_text_set(songview, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artistview, (char*)MGET(songmap, "artist"), ts);

  player_play(MGET(songmap, "path"));
}

void rand_button_pushed(view_t* view, void* data)
{
  lastindex = rand() % ui.songs->length;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = ui.songs->data[lastindex];
  tg_text_set(songview, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artistview, (char*)MGET(songmap, "artist"), ts);

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

void max_button_pushed(view_t* view, void* data)
{
  wm_toggle_fullscreen();
}

void close_button_pushed(view_t* view, void* data)
{
  wm_close();
}

void ui_update_position(float ratio)
{
  tg_knob_set_angle(playbtn, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_volume(float ratio)
{
  tg_knob_set_angle(volbtn, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_visualizer()
{
  player_draw_waves(0, visuleft->texture.bitmap, 3);
  player_draw_waves(1, visuright->texture.bitmap, 3);

  visuleft->texture.changed  = 1;
  visuright->texture.changed = 1;
}

void ui_update_video()
{
  player_draw_video(visuvideo->texture.bitmap, 3);
  visuvideo->texture.changed = 1;
}

void ui_update_time(double time)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_RIGHT;
  ts.size        = 25.0;
  ts.textcolor   = 0x555555FF;
  ts.backcolor   = 0;

  char timebuff[20];

  snprintf(timebuff, 20, "%.2i:", (int)floor(time / 60.0));
  tg_text_set(minuteview, timebuff, ts);

  ts.align = TA_LEFT;

  snprintf(timebuff, 20, "%.2i", (int)time % 60);
  tg_text_set(secondview, timebuff, ts);
}

void ui_filter(view_t* view, str_t* text)
{
}

void ui_init(float width, float height, char* respath, vec_t* songs, vec_t* genres, vec_t* artists)
{
  ui.songs = songs;

  char* csspath  = cstr_fromformat("%s/../res/main.css", respath, NULL);
  char* htmlpath = cstr_fromformat("%s/../res/main.html", respath, NULL);

  fontpath = cstr_fromformat("%s/../res/Avenir.ttc", respath, NULL);

  // init text rendering

  text_init();

  // view setup

  vec_t* views = view_gen_load(htmlpath, csspath, respath);

  baseview = vec_head(views);

  view_set_frame(baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(baseview);

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_RIGHT;
  ts.margin_right = 20;
  ts.size         = 25.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0xFFFFFFFF;

  mainview = view_get_subview(baseview, "main");

  songlist_attach(baseview, ui.songs, fontpath, on_song_select, on_song_edit, on_song_header);

  textlist_t* genrelist = textlist_new(view_get_subview(baseview, "genrelist"), genres, ts, on_genre_select);

  ts.align = TA_LEFT;

  textlist_t* artistlist = textlist_new(view_get_subview(baseview, "artistlist"), artists, ts, on_artist_select);

  messagelistback     = view_get_subview(baseview, "messagelistback");
  view_t* messagelist = view_get_subview(baseview, "messagelist");

  activity_attach(messagelist, fontpath);

  view_remove(mainview, messagelistback);

  filterlistback = view_get_subview(baseview, "filterlistback");
  view_remove(mainview, filterlistback);

  minuteview = view_get_subview(baseview, "minute");
  secondview = view_get_subview(baseview, "second");

  ts.margin_right = 0;
  ts.textcolor    = 0x555555FF;
  ts.backcolor    = 0x0;

  tg_text_add(minuteview);
  tg_text_set(minuteview, "00:", ts);

  ts.align = TA_LEFT;

  tg_text_add(secondview);
  tg_text_set(secondview, "00", ts);

  songview              = view_get_subview(baseview, "song");
  songview->needs_touch = 0;

  ts.size = 25.0;

  tg_text_add(songview);
  tg_text_set(songview, "-", ts);

  artistview              = view_get_subview(baseview, "artist");
  artistview->needs_touch = 0;

  tg_text_add(artistview);
  tg_text_set(artistview, "-", ts);

  infoview = view_get_subview(baseview, "info");
  tg_text_add(infoview);
  tg_text_set(infoview, "-", ts);

  ts.size = 20.0;

  view_t* filterbar = view_get_subview(baseview, "filterfield");

  filterbar->layout.background_color = 0xFFFFFFFF;
  vh_text_add(filterbar, "Search/Query", fontpath, ui_filter, NULL);

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

  view_t* filterbtn = view_get_subview(baseview, "genrebtn");
  vh_button_add(filterbtn, NULL, filterbtn_pushed);
  tg_text_add(filterbtn);
  tg_text_set(filterbtn, "filters", ts);

  view_t* messagesbtn = view_get_subview(baseview, "messagesbtn");
  vh_button_add(messagesbtn, NULL, messagesbtn_pushed);
  tg_text_add(messagesbtn);
  tg_text_set(messagesbtn, "activity", ts);

  /* view_t* texmap        = view_new("texmap", ((r2_t){0, 0, 300, 300})); */
  /* texmap->needs_touch   = 0; */
  /* texmap->display       = 1; */
  /* texmap->texture.full  = 1; */
  /* texmap->layout.right  = 1; */
  /* texmap->layout.bottom = 1; */

  /* ui_manager_add(texmap); */

  view_t* footer  = view_get_subview(baseview, "footer");
  footer->display = 0;
}

#endif
