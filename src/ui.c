#ifndef ui_h
#define ui_h

#include "mtmap.c"
#include "view.c"

void ui_init(float width,
             float height,
             char* respath,
             char* libpath);
void ui_update_position(float ratio);
void ui_update_volume(float ratio);
void ui_update_visualizer();
void ui_update_video();
void ui_update_time(double time);
void ui_toggle_pause(int state);
void ui_show_libpath_popup(char* message);
void ui_hide_libpath_popup();
void ui_refresh_songlist();
void ui_reload_songlist();
void ui_show_query(char* text);
void ui_on_next_button_down(void* userdata, void* data);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "activity.c"
#include "callbacks.c"
#include "db.c"
#include "editor.c"
#include "mtcstring.c"
#include "mtnumber.c"
#include "player.c"
#include "songlist.c"
#include "textlist.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_picker.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_knob.c"
#include "vh_picker.c"
#include "vh_text.c"
#include "view_generator.c"
#include "view_layout.c"
#include "wm_connector.c"
// TODO remove fromm zenmusic.c
#include "ui_generator.c"
#include "ui_manager.c"

view_t* baseview;
view_t* minuteview;
view_t* secondview;

view_t* aboutview;
view_t* editorview;
view_t* settingsview;

view_t* infoview;

view_t* visuleft;
view_t* visuright;
view_t* visuvideo;

view_t* playbtn;
view_t* volbtn;

view_t* mainview;
view_t* messagelistback;
view_t* filterlistback;
view_t* filterbar;

view_t* decision_popup;
view_t* ch_lib_popup;

view_t* libpopuppage;
view_t* libtextfield;
view_t* libinputfield;

view_t* set_col_val;

size_t lastindex = 0;
char*  fontpath;
char*  ui_libpath;

void ui_show_song_info(int index);

struct _ui_t
{
  vec_t* songs;
} ui = {0};

// button events

void ui_on_max_button_down(void* userdata, void* data)
{
  wm_toggle_fullscreen();
}

void ui_on_close_button_down(void* userdata, void* data)
{
  wm_close();
}

void ui_on_prev_button_down(void* userdata, void* data)
{
  lastindex = lastindex - 1;
  if (lastindex < 0) lastindex = 0;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_next_button_down(void* userdata, void* data)
{
  lastindex = lastindex + 1;
  // if (lastindex == ui.songs->length) lastindex = files->length - 1;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_rand_button_down(void* userdata, void* data)
{
  lastindex = rand() % ui.songs->length;

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  player_play(MGET(songmap, "path"));
}

void ui_on_edit_button_down(void* userdata, void* data)
{
  if (editorview->parent)
    view_remove(mainview, editorview);
  else
    view_add(mainview, editorview);
}

void ui_on_about_button_down(void* userdata, void* data)
{
  if (aboutview->parent)
    view_remove(mainview, aboutview);
  else
    view_add(mainview, aboutview);
}

void ui_on_settings_button_down(void* userdata, void* data)
{
  if (settingsview->parent)
    view_remove(mainview, settingsview);
  else
    view_add(mainview, settingsview);
}

void ui_on_editor_reject(void* userdata, void* data)
{
  ui_on_edit_button_down(NULL, data);
}

void ui_on_editor_accept(void* userdata, void* data)
{
  printf("on_editor_accept\n");
  ui_on_edit_button_down(NULL, data);

  map_t* old_data = editor_get_old_data();
  map_t* new_data = editor_get_new_data();

  // update modified entity in database
  vec_t* keys = VNEW();
  map_keys(new_data, keys);
  for (int index = 0; index < keys->length; index++)
  {
    char* key    = keys->data[index];
    char* oldval = MGET(old_data, key);
    char* newval = MGET(new_data, key);

    if (strcmp(oldval, newval) != 0)
    {
      printf("%s changed from %s to %s, writing to db\n", key, oldval, newval);
      MPUT(old_data, key, newval);
    }
  }

  // notify main namespace to organize and save metadata and database

  callbacks_call("on_save_entry", old_data);

  // reload song list
  songlist_refresh();
}

void ui_on_editor_upload(void* userdata, void* data)
{
  printf("editor upload\n");
}

void ui_on_home_close(void* userdata, void* data)
{
  if (aboutview->parent)
    view_remove(mainview, aboutview);
  else
    view_add(mainview, aboutview);
}

void ui_on_settings_close(void* userdata, void* data)
{
  if (settingsview->parent)
    view_remove(mainview, settingsview);
  else
    view_add(mainview, settingsview);
}

void ui_on_messages_close(void* userdata, void* data)
{
  if (messagelistback->parent)
    view_remove(mainview, messagelistback);
  else
    view_add(mainview, messagelistback);
}

void ui_on_filter_close(void* userdata, void* data)
{
  if (filterlistback->parent)
    view_remove(mainview, filterlistback);
  else
    view_add(mainview, filterlistback);
}

void ui_on_accept_libpath(void* userdata, void* data)
{
  // get path string
  str_t* path    = vh_text_get_text(libinputfield);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);

  REL(path_ch);
}

void ui_on_color_select(void* userdata, void* data)
{
  num_t* val = data;

  uint32_t c = val->uint32v;
  uint8_t  r = (c >> 24) & 0xFF;
  uint8_t  g = (c >> 16) & 0xFF;
  uint8_t  b = (c >> 8) & 0xFF;
  uint8_t  a = c & 0xFF;

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  char text[10] = {0};
  snprintf(text, 10, "%.2x%.2x%.2x", r, g, b);
  tg_text_set(set_col_val, text, ts);

  tg_css_set_graycolor(c);
  ui_generator_rerender();
}

void ui_on_filter_activate(view_t* view)
{
  if (filterlistback->parent)
    view_remove(mainview, filterlistback);
  else
    view_add(mainview, filterlistback);
}

void ui_on_play_button_down(view_t* view)
{
  // toggle_pause();
}

void ui_on_mute_button_down(view_t* view)
{
  player_toggle_mute();
}

void ui_on_song_select(int index)
{
  ui_show_song_info(index);

  map_t* songmap = ui.songs->data[index];

  char* path = cstr_fromformat("%s%s", ui_libpath, MGET(songmap, "path"));

  player_play(path);

  REL(path);
}

void ui_on_song_edit(int index)
{
  map_t* songmap = ui.songs->data[index];

  editor_set_song(songmap);
  view_add(mainview, editorview);
}

void ui_on_song_header(char* id)
{
  callbacks_call("on_song_header", id);
}

void ui_on_genre_select(int index)
{
  printf("on genre select %i\n", index);

  vec_t* genres = db_get_genres();
  char*  genre  = genres->data[index];
  callbacks_call("on_genre_selected", genre);
}

void ui_on_artist_select(int index)
{
  printf("on artist select %i\n", index);

  vec_t* artists = db_get_artists();
  char*  artist  = artists->data[index];
  callbacks_call("on_artist_selected", artist);
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
  ts.textcolor   = 0x333333FF;
  ts.backcolor   = 0;

  map_t* songmap = ui.songs->data[index];

  char* infostr = cstr_fromformat("%s\n%s\n%s %s",
                                  (char*)MGET(songmap, "title"),
                                  (char*)MGET(songmap, "artist"),
                                  (char*)MGET(songmap, "genre"),
                                  "192Kb/s",
                                  NULL);

  tg_text_set(infoview, infostr, ts);
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

void ui_show_libpath_popup(char* text)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(libtextfield, text, ts);

  if (!libpopuppage->parent)
  {
    ui_manager_remove(baseview);
    ui_manager_add(libpopuppage);
  }

  vh_text_activate(libinputfield, 1); // activate text input
  ui_manager_activate(libinputfield); // set text input as event receiver
}

void ui_hide_libpath_popup()
{
  if (libpopuppage->parent)
  {
    ui_manager_remove(libpopuppage);
    ui_manager_add(baseview);
  }
}

void ui_filter(view_t* view)
{
  str_t* text = vh_text_get_text(view);

  callbacks_call("on_filter_songs", text);
}

void ui_refresh_songlist()
{
  songlist_refresh();
}

void ui_reload_songlist()
{
  songlist_update();
}

void ui_show_query(char* text)
{
  vh_text_set_text(filterbar, text);
}

void ui_init(float width,
             float height,
             char* respath,
             char* libpath)
{
  printf("ui_init %s %s\n", respath, libpath);

  ui_libpath = cstr_fromcstring(libpath);

  ui.songs = db_get_songs();

  // init text

  text_init();

  fontpath = cstr_fromformat("%s/Avenir.ttc", respath, NULL);

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 25.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  // event setup

  callbacks_set("on_maximize_press", cb_new(ui_on_max_button_down, NULL));
  callbacks_set("on_close_press", cb_new(ui_on_close_button_down, NULL));

  callbacks_set("on_prev_press", cb_new(ui_on_prev_button_down, NULL));
  callbacks_set("on_next_press", cb_new(ui_on_next_button_down, NULL));
  callbacks_set("on_shuffle_press", cb_new(ui_on_rand_button_down, NULL));

  callbacks_set("on_config_press", cb_new(ui_on_settings_button_down, NULL));
  callbacks_set("on_home_press", cb_new(ui_on_about_button_down, NULL));
  callbacks_set("on_edit_press", cb_new(ui_on_edit_button_down, NULL));

  callbacks_set("on_reject_edit_press", cb_new(ui_on_editor_reject, NULL));
  callbacks_set("on_accept_edit_press", cb_new(ui_on_editor_accept, NULL));

  callbacks_set("on_upload_press", cb_new(ui_on_editor_upload, NULL));

  callbacks_set("on_close_home_press", cb_new(ui_on_home_close, NULL));
  callbacks_set("on_close_settings_press", cb_new(ui_on_settings_close, NULL));
  callbacks_set("on_close_filter_press", cb_new(ui_on_filter_close, NULL));
  callbacks_set("on_close_messages_press", cb_new(ui_on_messages_close, NULL));

  callbacks_set("on_accept_libpath_press", cb_new(ui_on_accept_libpath, NULL));

  // view setup

  char* csspath  = cstr_fromformat("%s/main.css", respath, NULL);
  char* htmlpath = cstr_fromformat("%s/main.html", respath, NULL);

  vec_t* views = view_gen_load(htmlpath, csspath, respath, callbacks_get_data());

  baseview = vec_head(views);

  view_set_frame(baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(baseview);

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  mainview = view_get_subview(baseview, "main");

  // buttons

  view_t* uploadbtn = view_get_subview(baseview, "uploadbtn");
  tg_text_add(uploadbtn);
  tg_text_set(uploadbtn, "add new image", ts);

  playbtn = view_get_subview(baseview, "playbtn");
  volbtn  = view_get_subview(baseview, "volbtn");

  tg_knob_add(playbtn);
  vh_knob_add(playbtn, ui_on_position_change, ui_on_play_button_down);

  tg_knob_add(volbtn);
  vh_knob_add(volbtn, ui_on_volume_change, ui_on_mute_button_down);

  // get visualizer views

  visuleft  = view_get_subview(baseview, "visuleft");
  visuright = view_get_subview(baseview, "visuright");
  visuvideo = view_get_subview(baseview, "visuvideo");

  // list setup

  songlist_attach(baseview, fontpath, ui_on_song_select, ui_on_song_edit, ui_on_song_header);

  ts.align        = TA_RIGHT;
  ts.margin_right = 20;

  textlist_t* genrelist = textlist_new(view_get_subview(baseview, "genrelist"), db_get_genres(), ts, ui_on_genre_select);

  ts.align               = TA_LEFT;
  textlist_t* artistlist = textlist_new(view_get_subview(baseview, "artistlist"), db_get_artists(), ts, ui_on_artist_select);

  messagelistback     = view_get_subview(baseview, "messagelistback");
  view_t* messagelist = view_get_subview(baseview, "messagelist");

  view_remove(mainview, messagelistback);

  filterlistback = view_get_subview(baseview, "filterlistback");
  view_remove(mainview, filterlistback);

  // display views

  minuteview = view_get_subview(baseview, "minute");
  secondview = view_get_subview(baseview, "second");

  ts.margin_right = 0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0x0;

  ts.align = TA_RIGHT;

  tg_text_add(minuteview);
  tg_text_set(minuteview, "00:", ts);

  ts.align = TA_LEFT;

  tg_text_add(secondview);
  tg_text_set(secondview, "00", ts);

  infoview = view_get_subview(baseview, "info");

  tg_text_add(infoview);
  tg_text_set(infoview, "-", ts);

  ts.align  = TA_LEFT;
  ts.margin = 10.0;

  cb_t* msg_show_cb = cb_new(ui_on_messages_close, NULL);
  vh_button_add(infoview, msg_show_cb);

  // init activity

  activity_init();
  activity_attach(messagelist, infoview, ts);

  // query field

  view_t* main      = view_get_subview(baseview, "main");
  main->needs_touch = 0;

  filterbar                          = view_get_subview(baseview, "filterfield");
  filterbar->layout.background_color = 0xFFFFFFFF;

  ts.align     = TA_CENTER;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;

  vh_text_add(filterbar, "", "Search/Query", ts, NULL);
  vh_text_set_on_text(filterbar, ui_filter);
  vh_text_set_on_activate(filterbar, ui_on_filter_activate);

  editorview     = view_get_subview(baseview, "ideditorback");
  view_t* editor = view_get_subview(baseview, "editorlist");

  view_remove(main, editorview);

  editor_attach(editor, fontpath);

  // lib input popup

  libpopuppage  = view_get_subview(baseview, "libpopuppage");
  libtextfield  = view_get_subview(baseview, "libtextfield");
  libinputfield = view_get_subview(baseview, "libinputfield");

  ts.backcolor = 0;

  tg_text_add(libtextfield);
  vh_text_add(libinputfield, "/home/youruser/Music", "", ts, NULL);

  view_remove(baseview, libpopuppage);

  // decision popup

  decision_popup = view_get_subview(baseview, "dec_pop_bck");
  view_remove(main, decision_popup);

  // change lib popup

  ch_lib_popup = view_get_subview(baseview, "chlib_pop_bck");
  view_remove(main, ch_lib_popup);

  // settings

  view_t* set_lib_text    = view_get_subview(baseview, "set_lib_txt");
  view_t* set_lib_path    = view_get_subview(baseview, "set_lib_pth");
  view_t* set_lib_btn_txt = view_get_subview(baseview, "set_lib_btn_txt");
  view_t* set_org_text    = view_get_subview(baseview, "set_org_text");
  view_t* set_org_val     = view_get_subview(baseview, "set_org_val");
  view_t* set_org_btn_txt = view_get_subview(baseview, "set_org_btn_txt");
  view_t* set_col_text    = view_get_subview(baseview, "set_col_text");
  set_col_val             = view_get_subview(baseview, "set_col_val");
  view_t* set_col_sel     = view_get_subview(baseview, "set_col_sel");

  tg_text_add(set_lib_text);
  tg_text_set(set_lib_text, "Music Library Location", ts);

  tg_text_add(set_lib_path);
  tg_text_set(set_lib_path, libpath, ts);

  tg_text_add(set_lib_btn_txt);
  tg_text_set(set_lib_btn_txt, "Use Other Library", ts);

  tg_text_add(set_org_text);
  tg_text_set(set_org_text, "Keep Library Organized", ts);

  tg_text_add(set_org_val);
  tg_text_set(set_org_val, "Disabled", ts);

  tg_text_add(set_org_btn_txt);
  tg_text_set(set_org_btn_txt, "Enable", ts);

  tg_text_add(set_col_text);
  tg_text_set(set_col_text, "User Interface Color", ts);

  tg_text_add(set_col_val);
  tg_text_set(set_col_val, "EFEFEF", ts);

  cb_t* col_sel_cb = cb_new(ui_on_color_select, NULL);

  tg_picker_add(set_col_sel);
  vh_picker_add(set_col_sel, col_sel_cb);

  settingsview = view_get_subview(baseview, "settingsback");
  view_remove(main, settingsview);

  aboutview = view_get_subview(baseview, "aboutback");

  view_t* home_lib_text    = view_get_subview(baseview, "home_lib_txt");
  view_t* home_lib_btn_txt = view_get_subview(baseview, "home_lib_btn_txt");

  view_t* home_org_text    = view_get_subview(baseview, "home_org_txt");
  view_t* home_org_btn_txt = view_get_subview(baseview, "home_org_btn_txt");

  tg_text_add(home_lib_text);
  tg_text_set(home_lib_text, "Zen Music v0.8\n by Milan Toth\nFree and Open Source Software.\nIf you like it, please support the development.", ts);

  tg_text_add(home_lib_btn_txt);
  tg_text_set(home_lib_btn_txt, "Donate on Paypal", ts);

  tg_text_add(home_org_btn_txt);
  tg_text_set(home_org_btn_txt, "Support on Patreon", ts);

  view_remove(main, aboutview);

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
