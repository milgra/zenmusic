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

void ui_show_song_info(int index);

struct _ui_t
{
  vec_t* songs;
} ui = {0};

// button events

void ui_on_max_button_down(view_t* view, void* data)
{
  wm_toggle_fullscreen();
}

void ui_on_close_button_down(view_t* view, void* data)
{
  wm_close();
}

void ui_on_filterbtn_down(view_t* view, void* data)
{
  if (filterlistback->parent)
    view_remove(mainview, filterlistback);
  else
    view_add(mainview, filterlistback);
}

void ui_on_messagebtn_down(view_t* view, void* data)
{
  if (messagelistback->parent)
    view_remove(mainview, messagelistback);
  else
    view_add(mainview, messagelistback);
}

void ui_on_play_button_down(view_t* view)
{
  // toggle_pause();
}

void ui_on_mute_button_down(view_t* view)
{
  player_toggle_mute();
}

void ui_on_edit_button_down(view_t* view, void* data)
{
  if (editorview->parent)
    view_remove(mainview, editorview);
  else
    view_add(mainview, editorview);
}

void ui_on_prev_button_down(view_t* view, void* data)
{
  lastindex = lastindex - 1;
  if (lastindex < 0) lastindex = 0;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_next_button_down(view_t* view, void* data)
{
  lastindex = lastindex + 1;
  // if (lastindex == ui.songs->length) lastindex = files->length - 1;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_rand_button_down(view_t* view, void* data)
{
  lastindex = rand() % ui.songs->length;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_about_button_down(view_t* view, void* data)
{
  if (aboutview->parent)
    view_remove(mainview, aboutview);
  else
    view_add(mainview, aboutview);
}

void ui_on_settings_button_down(view_t* view, void* data)
{
  if (settingsview->parent)
    view_remove(mainview, settingsview);
  else
    view_add(mainview, settingsview);
}

void ui_on_song_select(int index)
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

void ui_on_song_edit(int index)
{
  map_t* songmap = ui.songs->data[index];

  editor_set_song(songmap);
  view_add(mainview, editorview);
}

void ui_on_song_header(char* id)
{
  printf("on song header %s\n", id);
  // filter db by field id
  // sort(id);
}

void ui_on_genre_select(int index)
{
  printf("on genre select %i\n", index);
}

void ui_on_artist_select(int index)
{
  printf("on artist select %i\n", index);
}

void ui_on_position_change(view_t* view, float angle)
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

void ui_on_volume_change(view_t* view, float angle)
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

// api functions

void ui_show_song_info(int index)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = ui.songs->data[index];
  tg_text_set(songview, (char*)MGET(songmap, "title"), ts);
  tg_text_set(artistview, (char*)MGET(songmap, "artist"), ts);
}

void ui_toggle_pause(int state)
{
  songlist_toggle_selected(state);
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

  // init text

  text_init();

  fontpath = cstr_fromformat("%s/../res/Avenir.ttc", respath, NULL);

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 25.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0xFFFFFFFF;

  // view setup

  char* csspath  = cstr_fromformat("%s/../res/main.css", respath, NULL);
  char* htmlpath = cstr_fromformat("%s/../res/main.html", respath, NULL);

  vec_t* views = view_gen_load(htmlpath, csspath, respath);

  baseview = vec_head(views);

  view_set_frame(baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(baseview);

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  mainview = view_get_subview(baseview, "main");

  // buttons

  view_t* maxbtn   = view_get_subview(baseview, "maxicon");
  view_t* closebtn = view_get_subview(baseview, "closeicon");

  view_t* prevbtn = view_get_subview(baseview, "previcon");
  view_t* nextbtn = view_get_subview(baseview, "nexticon");
  view_t* randbtn = view_get_subview(baseview, "shuffleicon");

  view_t* configbtn = view_get_subview(baseview, "settingsicon");
  view_t* aboutbtn  = view_get_subview(baseview, "abouticon");
  view_t* editbtn   = view_get_subview(baseview, "eventsicon");

  view_t* filterbtn   = view_get_subview(baseview, "genrebtn");
  view_t* messagesbtn = view_get_subview(baseview, "messagesbtn");

  playbtn = view_get_subview(baseview, "playbtn");
  volbtn  = view_get_subview(baseview, "volbtn");

  vh_button_add(maxbtn, NULL, ui_on_max_button_down);
  vh_button_add(closebtn, NULL, ui_on_close_button_down);

  tg_knob_add(playbtn);
  vh_knob_add(playbtn, ui_on_position_change, ui_on_play_button_down);

  tg_knob_add(volbtn);
  vh_knob_add(volbtn, ui_on_volume_change, ui_on_mute_button_down);

  vh_button_add(prevbtn, NULL, ui_on_prev_button_down);
  vh_button_add(nextbtn, NULL, ui_on_next_button_down);
  vh_button_add(randbtn, NULL, ui_on_rand_button_down);

  vh_button_add(configbtn, NULL, ui_on_settings_button_down);
  vh_button_add(editbtn, NULL, ui_on_edit_button_down);
  vh_button_add(aboutbtn, NULL, ui_on_about_button_down);

  vh_button_add(filterbtn, NULL, ui_on_filterbtn_down);
  vh_button_add(messagesbtn, NULL, ui_on_messagebtn_down);

  tg_text_add(filterbtn);
  tg_text_set(filterbtn, "filters", ts);

  tg_text_add(messagesbtn);
  tg_text_set(messagesbtn, "activity", ts);

  // get visualizer views

  visuleft  = view_get_subview(baseview, "visuleft");
  visuright = view_get_subview(baseview, "visuright");
  visuvideo = view_get_subview(baseview, "visuvideo");

  // list setup

  songlist_attach(baseview, ui.songs, fontpath, ui_on_song_select, ui_on_song_edit, ui_on_song_header);

  ts.align              = TA_RIGHT;
  textlist_t* genrelist = textlist_new(view_get_subview(baseview, "genrelist"), genres, ts, ui_on_genre_select);

  ts.align               = TA_LEFT;
  textlist_t* artistlist = textlist_new(view_get_subview(baseview, "artistlist"), artists, ts, ui_on_artist_select);

  messagelistback     = view_get_subview(baseview, "messagelistback");
  view_t* messagelist = view_get_subview(baseview, "messagelist");

  activity_attach(messagelist, fontpath);

  view_remove(mainview, messagelistback);

  filterlistback = view_get_subview(baseview, "filterlistback");
  view_remove(mainview, filterlistback);

  // display views

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

  // query field

  view_t* main      = view_get_subview(baseview, "main");
  main->needs_touch = 0;

  view_t* filterbar = view_get_subview(baseview, "filterfield");

  filterbar->layout.background_color = 0xFFFFFFFF;
  vh_text_add(filterbar, "Search/Query", fontpath, ui_filter, NULL);

  settingsview = view_get_subview(baseview, "settingsback");

  ts.align     = TA_CENTER;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;

  view_t* settings = view_get_subview(baseview, "settings");

  char* settingsinfo = "Library location:\n Change library\n Organize library X\n Change color\n";

  tg_text_add(settings);
  tg_text_set(settings, settingsinfo, ts);

  view_remove(main, settingsview);

  aboutview = view_get_subview(baseview, "aboutback");

  char* info = "Zen Music by Milan Toth\nFree and Open Source Software.\n"
               "If you like Zen Music, please consider donating\n www.paypal.me/milgra\n"
               "Powered by FreeBSD, Emacs and C";

  view_t* about = view_get_subview(baseview, "about");
  tg_text_add(about);
  tg_text_set(about, info, ts);

  view_remove(main, aboutview);

  editorview     = view_get_subview(baseview, "ideditorback");
  view_t* editor = view_get_subview(baseview, "ideditor");

  view_remove(main, editorview);

  editor_attach(editor, fontpath);

  // additional setup
  // TODO control these from CSS

  view_t* playicon = view_get_subview(baseview, "playicon");
  view_t* muteicon = view_get_subview(baseview, "muteicon");
  view_t* footer   = view_get_subview(baseview, "footer");

  playicon->needs_touch = 0;
  muteicon->needs_touch = 0;
  footer->display       = 0;

  // set glossy effect on header

  /* view_t* header = view_get_subview(baseview, "header"); */
  /* header->texture.blur = 1; */
  /* header->texture.shadow = 1; */

  // show texture map for debug

  /* view_t* texmap        = view_new("texmap", ((r2_t){0, 0, 300, 300})); */
  /* texmap->needs_touch   = 0; */
  /* texmap->display       = 1; */
  /* texmap->texture.full  = 1; */
  /* texmap->layout.right  = 1; */
  /* texmap->layout.bottom = 1; */

  /* ui_manager_add(texmap); */
}

#endif