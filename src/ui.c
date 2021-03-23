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
#include "database.c"
#include "donatelist.c"
#include "editor.c"
#include "filtered.c"
#include "itemlist.c"
#include "mtcstring.c"
#include "mtnumber.c"
#include "player.c"
#include "settingslist.c"
#include "songlist.c"
#include "songlistpopup.c"
#include "textlist.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_picker.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_fade.c"
#include "vh_knob.c"
#include "vh_list_item.c"
#include "vh_picker.c"
#include "vh_roll.c"
#include "vh_text.c"
#include "view_generator.c"
#include "view_layout.c"
#include "wm_connector.c"
#include <limits.h>
// TODO remove fromm zenmusic.c
#include "ui_generator.c"
#include "ui_manager.c"

view_t* baseview;
view_t* timeview;
view_t* leftview;

view_t* about_popup;
view_t* editor_popup;
view_t* settings_popup;

view_t* infoview;

view_t* visuleft;
view_t* visuright;
view_t* visuvideo;
view_t* visuleftbtn;
view_t* visurightbtn;
view_t* visuleftbtnbck;
view_t* visurightbtnbck;

view_t* seekknob;
view_t* playbtn;
view_t* volknob;
view_t* mutebtn;
view_t* simple_popup;
view_t* sim_pop_txt;
view_t* song_popup;
view_t* song_popup_list;

view_t* mainview;
view_t* messages_popup;
view_t* filters_popup;
view_t* filterbar;

view_t* decision_popup;

view_t* library_page;
view_t* libtextfield;
view_t* libinputfield;

view_t* set_col_val;
view_t* chlib_pop_if;
view_t* library_popup;
view_t* set_org_btn_txt;

size_t lastindex = 0;
char*  fontpath;
char*  ui_libpath;

void ui_show_song_info(int index);

struct _ui_t
{
  vec_t* songs;
  int    visu;
  char   shuffle;
  vec_t* selected;
} ui = {0};

void ui_play_index(int index)
{
  lastindex = index;
  if (lastindex < 0) lastindex = 0;
  if (lastindex < filtered_song_count())
  {
    vh_button_set_state(playbtn, VH_BUTTON_DOWN);

    ui_show_song_info(lastindex);
    map_t* songmap = ui.songs->data[lastindex];
    char*  path    = cstr_fromformat("%s%s", ui_libpath, MGET(songmap, "path"), NULL);
    player_play(path);
    REL(path);
  }
}

void ui_play_next()
{
  if (ui.shuffle)
    ui_play_index(rand() % filtered_song_count());
  else
    ui_play_index(lastindex + 1);
}

void ui_toggle_shuffle()
{
  ui.shuffle = !ui.shuffle;
}

void ui_remove_from_main(void* p)
{
  view_t* view = p;
  if (view->texture.alpha < 1.0) view_remove(mainview, view);
}

void ui_toggle_mainview(view_t* view)
{
  if (view->parent)
  {
    view->texture.alpha = 1.0;
    vh_fade_set(view, 0.0, 20.0, 1);
  }
  else
  {
    r2_t basef = baseview->frame.local;
    r2_t viewf = view->frame.local;
    viewf.x    = (basef.w - viewf.w) / 2;
    viewf.y    = (basef.h - viewf.h) / 2;
    view_set_frame(view, viewf);
    view_add(mainview, view);

    view->texture.alpha = 0.0;
    vh_fade_set(view, 1.0, 20.0, 1);
  }
}

void ui_editor_accept()
{
  ui_toggle_mainview(editor_popup);

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
  if (strcmp(id, "nextbtn") == 0) ui_play_next();
  if (strcmp(id, "shufflebtn") == 0) ui_toggle_shuffle();
  if (strcmp(id, "prevbtn") == 0) ui_play_index(lastindex - 1);
  if (strcmp(id, "settingsbtn") == 0) ui_toggle_mainview(settings_popup);
  if (strcmp(id, "closesettingsbtn") == 0) ui_toggle_mainview(settings_popup);
  if (strcmp(id, "aboutbtn") == 0) ui_toggle_mainview(about_popup);
  if (strcmp(id, "editbtn") == 0) ui_toggle_mainview(editor_popup);
  if (strcmp(id, "closemessagelistbtn") == 0) ui_toggle_mainview(messages_popup);
  if (strcmp(id, "info") == 0) ui_toggle_mainview(messages_popup);
  if (strcmp(id, "closefilterbtn") == 0) ui_toggle_mainview(filters_popup);
  if (strcmp(id, "closeeditorbtn") == 0) ui_toggle_mainview(editor_popup);
  if (strcmp(id, "accepteditorbtn") == 0) ui_editor_accept();
  if (strcmp(id, "closehomebtn") == 0) ui_toggle_mainview(about_popup);
  if (strcmp(id, "chlib_pop_acc_btn") == 0) ui_change_library();
  if (strcmp(id, "acceptlibbtn") == 0) ui_set_library();
  if (strcmp(id, "library_popup_close_btn") == 0) ui_toggle_mainview(library_popup);
  if (strcmp(id, "dec_pop_acc_btn") == 0) ui_set_organize_lib();
  if (strcmp(id, "dec_pop_rej_btn") == 0) ui_toggle_mainview(decision_popup);
  if (strcmp(id, "close_sim_pop_btn") == 0) ui_toggle_mainview(simple_popup);
  if (strcmp(id, "close_song_popup_btn") == 0) ui_toggle_mainview(song_popup);
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
  ui_toggle_mainview(filters_popup);
}

void ui_on_song_edit(int index)
{
  ui_toggle_mainview(song_popup);
}

void ui_on_song_header(char* id)
{
  callbacks_call("on_song_header", id);
}

void ui_on_genre_select(int index)
{
  printf("on genre select %i\n", index);

  vec_t* genres = filtered_get_genres();
  char*  genre  = genres->data[index];
  callbacks_call("on_genre_selected", genre);
}

void ui_on_artist_select(int index)
{
  printf("on artist select %i\n", index);

  vec_t* artists = filtered_get_artists();
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
  vh_fade_set(visuleftbtnbck, 1.0, 10.0, 1);
  vh_fade_set(visurightbtnbck, 1.0, 10.0, 1);
}

void ui_on_roll_out_visu(void* userdata, void* data)
{
  vh_fade_set(visuleftbtnbck, 0.0, 10.0, 1);
  vh_fade_set(visurightbtnbck, 0.0, 10.0, 1);
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
  songlist_toggle_pause(state);
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

void ui_hide_libpath_popup()
{
  if (library_page->parent)
  {
    ui_manager_remove(library_page);
    ui_manager_add(baseview);
  }
}

void ui_show_simple_popup(char* text)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(sim_pop_txt, text, ts);

  ui_toggle_mainview(simple_popup);
}

void ui_show_libpath_popup1(char* text)
{
  ui_toggle_mainview(library_popup);
}

void ui_show_liborg_popup(char* text)
{
  ui_toggle_mainview(decision_popup);
}

void ui_show_libpath_popup(char* text)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(libtextfield, text, ts);

  if (!library_page->parent)
  {
    ui_manager_remove(baseview);
    ui_manager_add(library_page);
  }

  vh_text_activate(libinputfield, 1); // activate text input
  ui_manager_activate(libinputfield); // set text input as event receiver
}

void ui_on_songlistpopup_select(int index)
{
  ui_toggle_mainview(song_popup);
  if (index == 0) songlist_select(index);
  if (index == 1) songlist_select_range(index);
  if (index == 2) songlist_select_all();
  if (index == 3)
  {
    vec_reset(ui.selected);
    songlist_get_selected(ui.selected);
    editor_set_songs(ui.selected);
    ui_toggle_mainview(editor_popup);
  }
  if (index == 4)
  {
    ui_show_simple_popup("Are you sure you want to delete x items?");
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
  ui_libpath  = cstr_fromcstring(libpath);
  ui.songs    = filtered_get_songs();
  ui.selected = VNEW();

  // init text

  text_init();

  fontpath = cstr_fromformat("%s/Baloo.ttf", respath, NULL);

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 30.0;
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

  visuleft        = view_get_subview(baseview, "visuleft");
  visuright       = view_get_subview(baseview, "visuright");
  visuvideo       = view_get_subview(baseview, "visuvideo");
  visuleftbtn     = view_get_subview(visuleft, "visuleft_btn");
  visurightbtn    = view_get_subview(visuright, "visuright_btn");
  visuleftbtnbck  = view_get_subview(visuleft, "visuleft_btn_bck");
  visurightbtnbck = view_get_subview(visuright, "visuright_btn_bck");

  vh_fade_add(visuleftbtnbck, NULL, NULL);
  vh_fade_add(visurightbtnbck, NULL, NULL);

  /* view_remove(visuleft, visuleftbtn); */
  /* view_remove(visuright, visurightbtn); */

  // visualise roll over

  cb_t* cb_roll_in_visu  = cb_new(ui_on_roll_in_visu, NULL);
  cb_t* cb_roll_out_visu = cb_new(ui_on_roll_out_visu, NULL);

  vh_roll_add(visuleft, cb_roll_in_visu, cb_roll_out_visu);
  vh_roll_add(visuright, cb_roll_in_visu, cb_roll_out_visu);

  vh_fade_set(visuleftbtnbck, 0.0, 10.0, 1);
  vh_fade_set(visurightbtnbck, 0.0, 10.0, 1);

  // list setup

  songlist_attach(baseview, fontpath, ui_play_index, ui_on_song_edit, ui_on_song_header);

  ts.align        = TA_RIGHT;
  ts.margin_right = 20;

  textlist_t* genrelist = textlist_new(view_get_subview(baseview, "genrelist"), filtered_get_genres(), ts, ui_on_genre_select);

  ts.align               = TA_LEFT;
  textlist_t* artistlist = textlist_new(view_get_subview(baseview, "artistlist"), filtered_get_artists(), ts, ui_on_artist_select);

  messages_popup      = view_get_subview(baseview, "messages_popup");
  view_t* messagelist = view_get_subview(baseview, "messagelist");

  vh_fade_add(messages_popup, messages_popup, ui_remove_from_main);

  view_remove(mainview, messages_popup);

  filters_popup = view_get_subview(baseview, "filters_popup");

  vh_fade_add(filters_popup, filters_popup, ui_remove_from_main);

  view_remove(mainview, filters_popup);

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

  // song editor

  editor_popup = view_get_subview(baseview, "song_editor_popup");

  editor_attach(editor_popup, fontpath);

  vh_fade_add(editor_popup, editor_popup, ui_remove_from_main);

  view_remove(main, editor_popup);

  // lib input popup

  library_page  = view_get_subview(baseview, "library_page");
  libtextfield  = view_get_subview(baseview, "libtextfield");
  libinputfield = view_get_subview(baseview, "libinputfield");

  ts.backcolor = 0;

  tg_text_add(libtextfield);
  vh_text_add(libinputfield, "/home/youruser/Music", "", ts, NULL);

  view_remove(baseview, library_page);

  // decision popup

  ts.multiline       = 1;
  view_t* dec_pop_tf = view_get_subview(baseview, "dec_pop_tf");
  tg_text_add(dec_pop_tf);
  tg_text_set(dec_pop_tf, "Files will be renamed and moved to different folders based on artist, album, track number and title, are you sure?", ts);

  decision_popup = view_get_subview(baseview, "decision_popup");

  vh_fade_add(decision_popup, decision_popup, ui_remove_from_main);

  view_remove(main, decision_popup);

  // change lib popup

  view_t* chlib_pop_tf = view_get_subview(baseview, "chlib_pop_tf");
  chlib_pop_if         = view_get_subview(baseview, "chlib_pop_if");

  ts.backcolor = 0;

  tg_text_add(chlib_pop_tf);
  tg_text_set(chlib_pop_tf, "Use library at", ts);
  vh_text_add(chlib_pop_if, "/home/youruser/Music", "", ts, NULL);

  library_popup = view_get_subview(baseview, "library_popup");

  vh_fade_add(library_popup, library_popup, ui_remove_from_main);

  view_remove(main, library_popup);

  // simple popup

  simple_popup = view_get_subview(baseview, "simple_popup");
  sim_pop_txt  = view_get_subview(baseview, "sim_pop_txt");

  vh_fade_add(simple_popup, simple_popup, ui_remove_from_main);

  tg_text_add(sim_pop_txt);
  view_remove(main, simple_popup);

  // settings

  settings_popup = view_get_subview(baseview, "settings_popup");

  vh_fade_add(settings_popup, settings_popup, ui_remove_from_main);

  view_t* settingslist = view_get_subview(baseview, "settingslist");
  settingslist_attach(settingslist, fontpath, ui_show_libpath_popup1, ui_show_liborg_popup, ui_show_simple_popup);

  view_remove(main, settings_popup);

  // about view

  about_popup       = view_get_subview(baseview, "about_popup");
  view_t* aboutlist = view_get_subview(baseview, "aboutlist");

  vh_fade_add(about_popup, about_popup, ui_remove_from_main);

  donatelist_attach(aboutlist, fontpath, ui_show_simple_popup);

  view_remove(main, about_popup);

  // song popup

  song_popup      = view_get_subview(baseview, "song_popup");
  song_popup_list = view_get_subview(baseview, "song_popup_list");

  vh_fade_add(song_popup, song_popup, ui_remove_from_main);

  songlistpopup_attach(song_popup_list, fontpath, ui_on_songlistpopup_select);
  //donatelist_attach(aboutlist, fontpath, ui_show_simple_popup);

  view_remove(main, song_popup);

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
