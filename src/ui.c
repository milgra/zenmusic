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
void ui_update_time(double time, double left, double duration);
void ui_toggle_pause(int state);
void ui_show_libpath_popup(char* message);
void ui_hide_libpath_popup();
void ui_refresh_songlist();
void ui_reload_songlist();
void ui_show_query(char* text);
void ui_play_next();
void ui_set_libpath(char* libpath);
void ui_set_org_btn_lbl(char* text);

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
#include "vh_roll.c"
#include "vh_text.c"
#include "view_generator.c"
#include "view_layout.c"
#include "wm_connector.c"
// TODO remove fromm zenmusic.c
#include "ui_generator.c"
#include "ui_manager.c"

view_t* baseview;
view_t* timeview;
view_t* leftview;

view_t* aboutview;
view_t* editorview;
view_t* settingsview;

view_t* infoview;

view_t* visuleft;
view_t* visuright;
view_t* visuvideo;
view_t* visuleftbtn;
view_t* visurightbtn;

view_t* seekknob;
view_t* playbtn;
view_t* volknob;
view_t* mutebtn;

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
view_t* chlib_pop_if;
view_t* set_org_btn_txt;

size_t lastindex = 0;
char*  fontpath;
char*  ui_libpath;

void ui_show_song_info(int index);

struct _ui_t
{
  vec_t* songs;
  int    visu;
} ui = {0};

void ui_play_index(int index)
{
  lastindex = index;
  if (lastindex < 0) lastindex = 0;
  if (lastindex == ui.songs->length) lastindex = ui.songs->length - 1;

  vh_button_set_state(playbtn, VH_BUTTON_DOWN);

  ui_show_song_info(lastindex);
  map_t* songmap = ui.songs->data[lastindex];
  char*  path    = cstr_fromformat("%s%s", ui_libpath, MGET(songmap, "path"), NULL);
  player_play(path);
  REL(path);
}

void ui_play_next()
{
  ui_play_index(lastindex + 1);
}

void ui_toggle_mainview(view_t* view)
{
  if (view->parent)
    view_remove(mainview, view);
  else
    view_add(mainview, view);
}

void ui_editor_accept()
{
  printf("ui_editor_accept\n");
  ui_toggle_mainview(editorview);

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

void ui_change_library()
{
  // get path string
  str_t* path    = vh_text_get_text(chlib_pop_if);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_set_library()
{
  // get path string
  str_t* path    = vh_text_get_text(libinputfield);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_set_organize_lib()
{
  char* text = tg_text_get(set_org_btn_txt);

  callbacks_call("on_change_organize", text);
}

void ui_change_visu()
{
  ui.visu = 1 - ui.visu;
}

void ui_on_play_button_down(view_t* view)
{
  if (player_toggle_pause() < 0)
  {
    ui_play_index(0);
  }
  //ui_toggle_pause();
}

void ui_on_mute_button_down(view_t* view)
{
  player_toggle_mute();
}

void ui_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "maxbtn") == 0) wm_toggle_fullscreen();
  if (strcmp(id, "app_close_btn") == 0) wm_close();
  if (strcmp(id, "playbtn") == 0) ui_on_play_button_down(NULL);
  if (strcmp(id, "mutebtn") == 0) ui_on_mute_button_down(NULL);
  if (strcmp(id, "nextbtn") == 0) ui_play_index(lastindex + 1);
  if (strcmp(id, "shufflebtn") == 0) ui_play_index(rand() % ui.songs->length);
  if (strcmp(id, "prevbtn") == 0) ui_play_index(lastindex - 1);
  if (strcmp(id, "settingsbtn") == 0) ui_toggle_mainview(settingsview);
  if (strcmp(id, "closesettingsbtn") == 0) ui_toggle_mainview(settingsview);
  if (strcmp(id, "aboutbtn") == 0) ui_toggle_mainview(aboutview);
  if (strcmp(id, "editbtn") == 0) ui_toggle_mainview(editorview);
  if (strcmp(id, "closemessagelistbtn") == 0) ui_toggle_mainview(messagelistback);
  if (strcmp(id, "info") == 0) ui_toggle_mainview(messagelistback);
  if (strcmp(id, "closefilterbtn") == 0) ui_toggle_mainview(filterlistback);
  if (strcmp(id, "closeeditorbtn") == 0) ui_toggle_mainview(editorview);
  if (strcmp(id, "accepteditorbtn") == 0) ui_editor_accept();
  if (strcmp(id, "closehomebtn") == 0) ui_toggle_mainview(aboutview);
  if (strcmp(id, "set_lib_btn") == 0) ui_toggle_mainview(ch_lib_popup);
  if (strcmp(id, "set_org_btn") == 0) ui_toggle_mainview(decision_popup);
  if (strcmp(id, "chlib_pop_acc_btn") == 0) ui_change_library();
  if (strcmp(id, "acceptlibbtn") == 0) ui_set_library();
  if (strcmp(id, "chlib_pop_rej_btn") == 0) ui_toggle_mainview(ch_lib_popup);
  if (strcmp(id, "dec_pop_acc_btn") == 0) ui_set_organize_lib();
  if (strcmp(id, "dec_pop_rej_btn") == 0) ui_toggle_mainview(decision_popup);
  if (strcmp(id, "visuright_btn") == 0) ui_change_visu();
  if (strcmp(id, "visuleft_btn") == 0) ui_change_visu();
}

void ui_on_color_select(void* userdata, void* data)
{
  num_t* val = data;

  /* char text[10] = {0}; */
  /* snprintf(text, 10, "%.2x%.2x%.2x", r, g, b); */
  /* tg_text_set(set_col_val, text, ts); */
}

void ui_on_filter_activate(view_t* view)
{
  ui_toggle_mainview(filterlistback);
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

void ui_on_roll_in_visu(void* userdata, void* data)
{
  view_add(visuleft, visuleftbtn);
  view_add(visuright, visurightbtn);
}

void ui_on_roll_out_visu(void* userdata, void* data)
{
  view_remove(visuleft, visuleftbtn);
  view_remove(visuright, visurightbtn);
}

// api functions

void ui_set_libpath(char* libpath)
{
  if (ui_libpath) REL(ui_libpath);
  ui_libpath = cstr_fromcstring(libpath);
}

void ui_set_org_btn_lbl(char* text)
{
  // TODO get style from css
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x333333FF;
  ts.backcolor   = 0;

  tg_text_set(set_org_btn_txt, text, ts);
}

void ui_show_song_info(int index)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 28.0;
  ts.textcolor   = 0x000000FF;
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
  tg_knob_set_angle(seekknob, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_volume(float ratio)
{
  tg_knob_set_angle(volknob, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_visualizer()
{
  if (ui.visu)
  {
    player_draw_rdft(0, visuleft->texture.bitmap, 3);
    player_draw_rdft(1, visuright->texture.bitmap, 3);
  }
  else
  {
    player_draw_waves(0, visuleft->texture.bitmap, 3);
    player_draw_waves(1, visuright->texture.bitmap, 3);
  }

  visuleft->texture.changed  = 1;
  visuright->texture.changed = 1;
}

void ui_update_video()
{
  player_draw_video(visuvideo->texture.bitmap, 3);
  visuvideo->texture.changed = 1;
}

void ui_update_time(double time, double left, double dur)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_RIGHT;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  char timebuff[20];

  int tmin = (int)floor(time / 60.0);
  int tsec = (int)time % 60;
  int lmin = (int)floor(left / 60.0);
  int lsec = (int)left % 60;
  int dmin = (int)floor(dur / 60.0);
  int dsec = (int)dur % 60;

  ts.align = TA_CENTER;
  snprintf(timebuff, 20, "%.2i:%.2i / %.2i:%.2i", dmin, dsec, tmin, tsec);
  tg_text_set(timeview, timebuff, ts);
  snprintf(timebuff, 20, "%.2i:%.2i / %.2i:%.2i", dmin, dsec, lmin, lsec);
  tg_text_set(leftview, timebuff, ts);
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
  ui_libpath = cstr_fromcstring(libpath);
  ui.songs   = db_get_songs();

  // init text

  text_init();

  fontpath = cstr_fromformat("%s/Baloo.ttf", respath, NULL);

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 25.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  // event setup

  callbacks_set("on_button_press", cb_new(ui_on_button_down, NULL));

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

  seekknob = view_get_subview(baseview, "seekknob");
  playbtn  = view_get_subview(baseview, "playbtn");
  volknob  = view_get_subview(baseview, "volknob");
  mutebtn  = view_get_subview(baseview, "mutebtn");

  tg_knob_add(seekknob);
  vh_knob_add(seekknob, ui_on_position_change, ui_on_play_button_down);

  tg_knob_add(volknob);
  vh_knob_add(volknob, ui_on_volume_change, ui_on_mute_button_down);

  cb_t* msg_play_pause_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(playbtn, VH_BUTTON_TOGGLE, msg_play_pause_cb);
  vh_button_add(mutebtn, VH_BUTTON_TOGGLE, msg_play_pause_cb);

  // get visualizer views

  visuleft     = view_get_subview(baseview, "visuleft");
  visuright    = view_get_subview(baseview, "visuright");
  visuvideo    = view_get_subview(baseview, "visuvideo");
  visuleftbtn  = view_get_subview(visuleft, "visuleft_btn");
  visurightbtn = view_get_subview(visuright, "visuright_btn");

  view_remove(visuleft, visuleftbtn);
  view_remove(visuright, visurightbtn);

  // visualise roll over

  cb_t* cb_roll_in_visu  = cb_new(ui_on_roll_in_visu, NULL);
  cb_t* cb_roll_out_visu = cb_new(ui_on_roll_out_visu, NULL);

  vh_roll_add(visuleft, cb_roll_in_visu, cb_roll_out_visu);
  vh_roll_add(visuright, cb_roll_in_visu, cb_roll_out_visu);

  // list setup

  songlist_attach(baseview, fontpath, ui_play_index, ui_on_song_edit, ui_on_song_header);

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

  view_t* timeback = view_get_subview(baseview, "timebck");
  timeview         = view_get_subview(baseview, "time");
  leftview         = view_get_subview(baseview, "left");

  vh_button_add(timeback, VH_BUTTON_TOGGLE, NULL);

  ts.margin_right = 0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0x0;

  ts.align = TA_LEFT;

  tg_text_add(timeview);
  tg_text_set(timeview, "00:", ts);

  ts.align = TA_RIGHT;

  tg_text_add(leftview);
  tg_text_set(leftview, "00:", ts);

  ts.align = TA_LEFT;

  infoview = view_get_subview(baseview, "info");

  tg_text_add(infoview);
  tg_text_set(infoview, "-", ts);

  ts.align  = TA_LEFT;
  ts.margin = 10.0;

  cb_t* msg_show_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(infoview, VH_BUTTON_NORMAL, msg_show_cb);

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

  vh_text_add(filterbar, "", "Search/Filter", ts, NULL);
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

  ts.multiline       = 1;
  view_t* dec_pop_tf = view_get_subview(baseview, "dec_pop_tf");
  tg_text_add(dec_pop_tf);
  tg_text_set(dec_pop_tf, "Files will be renamed and moved to different folders based on artist, album, track number and title, are you sure?", ts);

  decision_popup = view_get_subview(baseview, "dec_pop_bck");
  view_remove(main, decision_popup);

  // change lib popup

  view_t* chlib_pop_tf = view_get_subview(baseview, "chlib_pop_tf");
  chlib_pop_if         = view_get_subview(baseview, "chlib_pop_if");

  ts.backcolor = 0;

  tg_text_add(chlib_pop_tf);
  tg_text_set(chlib_pop_tf, "Use library at", ts);
  vh_text_add(chlib_pop_if, "/home/youruser/Music", "", ts, NULL);

  ch_lib_popup = view_get_subview(baseview, "chlib_pop_bck");
  view_remove(main, ch_lib_popup);

  // settings

  view_t* set_lib_text    = view_get_subview(baseview, "set_lib_txt");
  view_t* set_lib_path    = view_get_subview(baseview, "set_lib_pth");
  view_t* set_lib_btn_txt = view_get_subview(baseview, "set_lib_btn_txt");
  view_t* set_org_text    = view_get_subview(baseview, "set_org_text");
  view_t* set_org_val     = view_get_subview(baseview, "set_org_val");
  set_org_btn_txt         = view_get_subview(baseview, "set_org_btn_txt");
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

  view_t* home_info_text       = view_get_subview(baseview, "home_info_txt");
  view_t* home_donate_btn_txt  = view_get_subview(baseview, "home_donate_btn_txt");
  view_t* home_support_btn_txt = view_get_subview(baseview, "home_support_btn_txt");
  view_t* home_src_btn_txt     = view_get_subview(baseview, "home_src_btn_txt");
  view_t* home_yt_btn_txt      = view_get_subview(baseview, "home_yt_btn_txt");

  tg_text_add(home_info_text);
  tg_text_set(home_info_text, "Zen Music v0.8\n by Milan Toth\nFree and Open Source Software.\nIf you like it, please support the development.", ts);

  tg_text_add(home_donate_btn_txt);
  tg_text_set(home_donate_btn_txt, "Donate on Paypal", ts);

  tg_text_add(home_support_btn_txt);
  tg_text_set(home_support_btn_txt, "Support on Patreon", ts);

  tg_text_add(home_src_btn_txt);
  tg_text_set(home_src_btn_txt, "Open Source", ts);

  tg_text_add(home_yt_btn_txt);
  tg_text_set(home_yt_btn_txt, "Open Youtube Channel", ts);

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
