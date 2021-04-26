#ifndef ui_h
#define ui_h

#include "mtmap.c"
#include "view.c"

void ui_init();
void ui_load(float width,
             float height,
             char* respath);
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
void ui_play_prev();
void ui_play_pause();
void ui_set_org_btn_lbl(char* text);
void ui_show_simple_popup(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "activity.c"
#include "callbacks.c"
#include "config.c"
#include "database.c"
#include "donatelist.c"
#include "editor.c"
#include "editor_popup.c"
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
#include "ui_manager.c"
#include "vh_anim.c"
#include "vh_button.c"
#include "vh_key.c"
#include "vh_knob.c"
#include "vh_list_item.c"
#include "vh_picker.c"
#include "vh_roll.c"
#include "vh_textinput.c"
#include "vh_touch.c"
#include "view_generator.c"
#include "view_layout.c"
#include "visible.c"
#include "wm_connector.c"
#include <limits.h>

struct _ui_t
{
  vec_t* songs;
  int    visu;
  char   shuffle;
  vec_t* selected; // selected songs from songlist
  char*  fontpath; // font path
  size_t lastindex;

  view_t* songlist_filter_bar;
  view_t* sim_pop_txt;
  view_t* song_popup_list;
  view_t* set_col_val;
  view_t* chlib_pop_if;
  view_t* set_org_btn_txt;

  textlist_t* artistlist;
  textlist_t* genrelist;

  view_t* baseview; // root view

  // set library popup fields

  view_t* slibpopup_textfield_view;
  view_t* slibpopup_inputfield_view;

  // play info views

  view_t* song_info_view;
  view_t* song_remaining_view;
  view_t* song_time_view;
  view_t* song_length_view;

  // controls

  view_t* playbtn;
  view_t* mutebtn;
  view_t* volknob;
  view_t* seekknob;

  // popup pages

  view_t* library_page;
  view_t* song_popup_page;
  view_t* about_popup_page;
  view_t* editor_popup_page;
  view_t* simple_popup_page;
  view_t* library_popup_page;
  view_t* filters_popup_page;
  view_t* messages_popup_page;
  view_t* decision_popup_page;
  view_t* settings_popup_page;

  // visualizer views

  view_t* visuleft;
  view_t* visuright;
  view_t* visuvideo;
  view_t* visuleftbtn;
  view_t* visurightbtn;
  view_t* visuleftbtnbck;
  view_t* visurightbtnbck;
} ui = {0};

void ui_on_button_down(void* userdata, void* data);
void ui_on_play_button_down(view_t* view);
void ui_on_mute_button_down(view_t* view);
void ui_on_position_change(view_t* view, float angle);
void ui_on_volume_change(view_t* view, float angle);
void ui_on_roll_in_visu(void* userdata, void* data);
void ui_on_roll_out_visu(void* userdata, void* data);
void ui_play_index(int index);
void ui_on_song_edit(int index);
void ui_remove_from_base(view_t* view, void* userdata);
void ui_show_song_info(int index);
void ui_on_key_down(void* userdata, void* data);
void ui_on_song_header(char* id);
void ui_on_genre_select(int index);
void ui_on_artist_select(int index);
void ui_filter(view_t* view);
void ui_show_libpath_popup1(char* text);
void ui_show_liborg_popup(char* text);
void ui_on_songlistpopup_select(int index);

void ui_init()
{
}

void ui_load(float width,
             float height,
             char* respath)
{
  ui.songs    = visible_get_songs();
  ui.selected = VNEW();

  // init text

  text_init();

  ui.fontpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/Baloo.ttf", respath);

  textstyle_t ts  = {0};
  ts.font         = ui.fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  // event setup

  callbacks_set("on_button_press", cb_new(ui_on_button_down, NULL));

  // view setup

  char* csspath  = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.css", respath);
  char* htmlpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.html", respath);

  vec_t* views = view_gen_load(htmlpath, csspath, respath, callbacks_get_data());

  ui.baseview = vec_head(views);

  cb_t* key_cb = cb_new(ui_on_key_down, ui.baseview);

  vh_key_add(ui.baseview, key_cb);

  view_set_frame(ui.baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(ui.baseview);

  ui_manager_init(width, height);
  ui_manager_add(ui.baseview);

  // buttons

  ui.seekknob = view_get_subview(ui.baseview, "seekknob");
  ui.playbtn  = view_get_subview(ui.baseview, "playbtn");
  ui.volknob  = view_get_subview(ui.baseview, "volknob");
  ui.mutebtn  = view_get_subview(ui.baseview, "mutebtn");

  tg_knob_add(ui.seekknob);
  vh_knob_add(ui.seekknob, ui_on_position_change, ui_on_play_button_down);

  tg_knob_add(ui.volknob);
  vh_knob_add(ui.volknob, ui_on_volume_change, ui_on_mute_button_down);

  cb_t* msg_play_pause_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(ui.playbtn, VH_BUTTON_TOGGLE, msg_play_pause_cb);
  vh_button_add(ui.mutebtn, VH_BUTTON_TOGGLE, msg_play_pause_cb);

  // get visualizer views

  ui.visuleft        = view_get_subview(ui.baseview, "visuleft");
  ui.visuright       = view_get_subview(ui.baseview, "visuright");
  ui.visuvideo       = view_get_subview(ui.baseview, "visuvideo");
  ui.visuleftbtn     = view_get_subview(ui.visuleft, "visuleft_btn");
  ui.visurightbtn    = view_get_subview(ui.visuright, "visuright_btn");
  ui.visuleftbtnbck  = view_get_subview(ui.visuleft, "visuleft_btn_bck");
  ui.visurightbtnbck = view_get_subview(ui.visuright, "visuright_btn_bck");

  vh_anim_add(ui.visuleftbtnbck);
  vh_anim_add(ui.visurightbtnbck);

  /* view_remove(ui.visuleft, ui.visuleftbtn); */
  /* view_remove(ui.visuright, ui.visurightbtn); */

  // visualise roll over

  cb_t* cb_roll_in_visu  = cb_new(ui_on_roll_in_visu, NULL);
  cb_t* cb_roll_out_visu = cb_new(ui_on_roll_out_visu, NULL);

  vh_roll_add(ui.visuleft, cb_roll_in_visu, cb_roll_out_visu);
  vh_roll_add(ui.visuright, cb_roll_in_visu, cb_roll_out_visu);

  vh_anim_alpha(ui.visuleftbtnbck, 1.0, 0.0, 10, AT_LINEAR);
  vh_anim_alpha(ui.visurightbtnbck, 1.0, 0.0, 10, AT_LINEAR);

  // list setup

  songlist_attach(ui.baseview, ui.fontpath, ui_play_index, ui_on_song_edit, ui_on_song_header);

  ts.align        = TA_RIGHT;
  ts.margin_right = 20;

  ui.genrelist = textlist_new(view_get_subview(ui.baseview, "genrelist"), visible_get_genres(), ts, ui_on_genre_select);

  ts.align      = TA_LEFT;
  ui.artistlist = textlist_new(view_get_subview(ui.baseview, "artistlist"), visible_get_artists(), ts, ui_on_artist_select);

  ui.messages_popup_page          = view_get_subview(ui.baseview, "messages_popup_page");
  view_t* messagelist             = view_get_subview(ui.baseview, "messagelist");
  view_t* messages_popup_page_btn = view_get_subview(ui.baseview, "messages_popup_page_btn");

  vh_anim_add(ui.messages_popup_page);
  vh_anim_set_event(ui.messages_popup_page, ui.messages_popup_page, ui_remove_from_base);
  vh_touch_add(messages_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_remove(ui.baseview, ui.messages_popup_page);

  ui.filters_popup_page          = view_get_subview(ui.baseview, "filters_popup_page");
  view_t* filters_popup          = view_get_subview(ui.baseview, "filters_popup");
  view_t* filters_popup_page_btn = view_get_subview(ui.baseview, "filters_popup_page_btn");

  vh_anim_add(ui.filters_popup_page);
  vh_anim_set_event(ui.filters_popup_page, ui.filters_popup_page, ui_remove_from_base);
  vh_touch_add(filters_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_remove(ui.baseview, ui.filters_popup_page);

  // display views

  ui.song_time_view      = view_get_subview(ui.baseview, "time");
  ui.song_remaining_view = view_get_subview(ui.baseview, "left");
  ui.song_length_view    = view_get_subview(ui.baseview, "length");

  tg_text_add(ui.song_time_view);
  tg_text_add(ui.song_remaining_view);
  tg_text_add(ui.song_length_view);

  ts.margin_right = 0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0x0;

  ts.align = TA_LEFT;

  ui.song_info_view = view_get_subview(ui.baseview, "info");

  tg_text_add(ui.song_info_view);
  tg_text_set(ui.song_info_view, "-", ts);

  ts.align  = TA_LEFT;
  ts.margin = 10.0;

  cb_t* msg_show_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(ui.song_info_view, VH_BUTTON_NORMAL, msg_show_cb);

  // init activity

  activity_init();
  activity_attach(messagelist, ui.song_info_view, ts);

  // query field

  view_t* main      = view_get_subview(ui.baseview, "main");
  main->needs_touch = 0;

  ui.songlist_filter_bar                          = view_get_subview(ui.baseview, "filterfield");
  ui.songlist_filter_bar->layout.background_color = 0xFFFFFFFF;

  ts.align     = TA_LEFT;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;

  vh_textinput_add(ui.songlist_filter_bar, "", "Search/Filter", ts, NULL);
  vh_textinput_set_on_text(ui.songlist_filter_bar, ui_filter);
  //vh_textinput_set_on_activate(ui.songlist_filter_bar, ui_on_filter_activate);

  // vh_textfield_add(ui.songlist_filter_bar, ts);

  // song editor

  ui.editor_popup_page          = view_get_subview(ui.baseview, "ideditor_popup_page");
  view_t* editor_popup          = view_get_subview(ui.baseview, "song_editor_popup");
  view_t* editor_popup_page_btn = view_get_subview(ui.baseview, "ideditor_popup_page_btn");

  editor_popup_attach(editor_popup, ui.fontpath);

  vh_anim_add(ui.editor_popup_page);
  vh_anim_set_event(ui.editor_popup_page, ui.editor_popup_page, ui_remove_from_base);
  vh_touch_add(editor_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_remove(ui.baseview, ui.editor_popup_page);

  // lib input popup

  ui.library_page              = view_get_subview(ui.baseview, "library_page");
  ui.slibpopup_textfield_view  = view_get_subview(ui.baseview, "libtextfield");
  ui.slibpopup_inputfield_view = view_get_subview(ui.baseview, "libinputfield");

  ts.backcolor = 0;

  tg_text_add(ui.slibpopup_textfield_view);
  vh_textinput_add(ui.slibpopup_inputfield_view, "/home/youruser/Music", "", ts, NULL);

  view_remove(ui.baseview, ui.library_page);

  // decision popup

  ts.multiline       = 1;
  view_t* dec_pop_tf = view_get_subview(ui.baseview, "dec_pop_tf");
  tg_text_add(dec_pop_tf);
  tg_text_set(dec_pop_tf, "Files will be renamed and moved to different folders based on artist, album, track number and title, are you sure?", ts);

  ui.decision_popup_page          = view_get_subview(ui.baseview, "decision_popup_page");
  view_t* decision_popup          = view_get_subview(ui.baseview, "decision_popup");
  view_t* decision_popup_page_btn = view_get_subview(ui.baseview, "decision_popup_page_btn");

  vh_anim_add(ui.decision_popup_page);
  vh_anim_set_event(ui.decision_popup_page, ui.decision_popup_page, ui_remove_from_base);
  vh_touch_add(decision_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_remove(ui.baseview, ui.decision_popup_page);

  // change lib popup

  view_t* chlib_pop_tf = view_get_subview(ui.baseview, "chlib_pop_tf");
  ui.chlib_pop_if      = view_get_subview(ui.baseview, "chlib_pop_if");

  ts.backcolor = 0;

  tg_text_add(chlib_pop_tf);
  tg_text_set(chlib_pop_tf, "Use library at", ts);
  vh_textinput_add(ui.chlib_pop_if, "/home/youruser/Music", "", ts, NULL);

  ui.library_popup_page          = view_get_subview(ui.baseview, "library_popup_page");
  view_t* library_popup          = view_get_subview(ui.baseview, "library_popup");
  view_t* library_popup_page_btn = view_get_subview(ui.baseview, "library_popup_page_btn");

  vh_anim_add(ui.library_popup_page);
  vh_anim_set_event(ui.library_popup_page, ui.library_popup_page, ui_remove_from_base);
  vh_touch_add(library_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_remove(ui.baseview, ui.library_popup_page);

  // simple popup

  ui.simple_popup_page          = view_get_subview(ui.baseview, "simple_popup_page");
  view_t* simple_popup          = view_get_subview(ui.baseview, "simple_popup");
  ui.sim_pop_txt                = view_get_subview(ui.baseview, "sim_pop_txt");
  view_t* simple_popup_page_btn = view_get_subview(ui.baseview, "simple_popup_page_btn");

  vh_anim_add(ui.simple_popup_page);
  vh_anim_set_event(ui.simple_popup_page, ui.simple_popup_page, ui_remove_from_base);
  vh_touch_add(simple_popup_page_btn, cb_new(ui_on_button_down, NULL));

  tg_text_add(ui.sim_pop_txt);
  view_remove(ui.baseview, ui.simple_popup_page);

  // settings

  ui.settings_popup_page          = view_get_subview(ui.baseview, "settings_popup_page");
  view_t* settings_popup_page_btn = view_get_subview(ui.baseview, "settings_popup_page_btn");

  vh_anim_add(ui.settings_popup_page);
  vh_anim_set_event(ui.settings_popup_page, ui.settings_popup_page, ui_remove_from_base);
  vh_touch_add(settings_popup_page_btn, cb_new(ui_on_button_down, NULL));

  view_t* settingslist = view_get_subview(ui.baseview, "settingslist");
  settingslist_attach(settingslist, ui.fontpath, ui_show_libpath_popup1, ui_show_liborg_popup, ui_show_simple_popup);

  view_remove(ui.baseview, ui.settings_popup_page);

  // about view

  ui.about_popup_page          = view_get_subview(ui.baseview, "about_popup_page");
  view_t* aboutlist            = view_get_subview(ui.baseview, "aboutlist");
  view_t* about_popup_page_btn = view_get_subview(ui.baseview, "about_popup_page_btn");

  vh_anim_add(ui.about_popup_page);
  vh_anim_set_event(ui.about_popup_page, ui.about_popup_page, ui_remove_from_base);
  vh_touch_add(about_popup_page_btn, cb_new(ui_on_button_down, NULL));

  donatelist_attach(aboutlist, ui.fontpath, ui_show_simple_popup);

  view_remove(ui.baseview, ui.about_popup_page);

  // song popup

  ui.song_popup_page          = view_get_subview(ui.baseview, "song_popup_page");
  ui.song_popup_list          = view_get_subview(ui.baseview, "song_popup_list");
  view_t* song_popup_page_btn = view_get_subview(ui.baseview, "song_popup_page_btn");

  vh_anim_add(ui.song_popup_page);
  vh_anim_set_event(ui.song_popup_page, ui.song_popup_page, ui_remove_from_base);
  vh_touch_add(song_popup_page_btn, cb_new(ui_on_button_down, NULL));

  songlistpopup_attach(ui.song_popup_list, ui.fontpath, ui_on_songlistpopup_select);
  //donatelist_attach(aboutlist, ui.fontpath, ui_show_simple_popup);

  view_remove(ui.baseview, ui.song_popup_page);

  // set glossy effect on header

  /* view_t* header = view_get_subview(ui.baseview, "header"); */
  /* header->texture.blur = 1; */
  /* header->texture.shadow = 1; */

  // show texture map for debug

  /* view_t* texmap        = view_new("texmap", ((r2_t){0, 0, 300, 300})); */
  /* texmap->needs_touch   = 0; */
  /* texmap->exclude       = 0; */
  /* texmap->texture.full  = 1; */
  /* texmap->layout.right  = 1; */
  /* texmap->layout.bottom = 1; */

  /* ui_manager_add(texmap); */
}

void ui_play_index(int index)
{
  ui.lastindex = index;
  if (ui.lastindex < 0) ui.lastindex = 0;
  if (ui.lastindex < visible_song_count())
  {
    vh_button_set_state(ui.playbtn, VH_BUTTON_DOWN);

    ui_show_song_info(ui.lastindex);
    map_t* songmap = ui.songs->data[ui.lastindex];
    char*  path    = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", config_get("lib_path"), MGET(songmap, "file/path"));
    player_play(path);
    player_set_volume(0.9);
    REL(path);
  }
}

void ui_play_next()
{
  if (ui.shuffle)
    ui_play_index(rand() % visible_song_count());
  else
    ui_play_index(ui.lastindex + 1);

  songlist_select_and_show(ui.lastindex);
}

void ui_play_prev()
{
  if (ui.shuffle)
    ui_play_index(rand() % visible_song_count());
  else
    ui_play_index(ui.lastindex - 1);

  songlist_select_and_show(ui.lastindex);
}

void ui_play_pause()
{
  player_toggle_pause();
}

void ui_toggle_shuffle()
{
  ui.shuffle = !ui.shuffle;
}

void ui_remove_from_base(view_t* view, void* userdata)
{
  if (view->texture.alpha < 1.0) view_remove(ui.baseview, view);
}

void ui_toggle_baseview(view_t* view)
{
  if (view->parent)
  {
    view->texture.alpha = 1.0;
    vh_anim_alpha(view, 1.0, 0.0, 20, AT_LINEAR);
  }
  else
  {
    r2_t basef = ui.baseview->frame.local;

    view_t* btnview = view->views->data[0];
    view_t* popview = btnview->views->data[0];

    r2_t viewf = popview->frame.local;
    viewf.x    = (basef.w - viewf.w) / 2;
    viewf.y    = (basef.h - viewf.h) / 2;
    view_set_frame(popview, viewf);

    view_add(ui.baseview, view);
    view->texture.alpha = 0.0;
    vh_anim_alpha(view, 0.0, 1.0, 20, AT_LINEAR);
  }
}

void ui_editor_accept()
{
  ui_toggle_baseview(ui.editor_popup_page);

  map_t* changed = editor_popup_get_changed();
  vec_t* removed = editor_popup_get_removed();
  char*  cover   = editor_popup_get_cover();

  ui_show_simple_popup("ARE YOU SURE?");

  /* char* libpath = config_get("lib_path"); */

  /* editor_update_metadata(libpath, ui.selected, changed, removed, cover); */

  // update metadata in media files

  // update metadata in database

  // organize library ( if path has to change )

  /* map_t* old_data = editor_popup_get_old_data(); */
  /* map_t* new_data = editor_popup_get_new_data(); */

  /* // update modified entity in database */
  /* vec_t* keys = VNEW(); */
  /* map_keys(new_data, keys); */
  /* for (int index = 0; index < keys->length; index++) */
  /* { */
  /*   char* key    = keys->data[index]; */
  /*   char* oldval = MGET(old_data, key); */
  /*   char* newval = MGET(new_data, key); */

  /*   if (strcmp(oldval, newval) != 0) */
  /*   { */
  /*     printf("%s changed from %s to %s, writing to db\n", key, oldval, newval); */
  /*     MPUT(old_data, key, newval); */
  /*   } */
  /* } */

  /* // notify main namespace to organize and save metadata and database */
  /* callbacks_call("on_save_entry", old_data); */

  /* // reload song list */
  /* songlist_refresh(); */
}

void ui_change_library()
{
  // get path string
  str_t* path    = vh_textinput_get_text(ui.chlib_pop_if);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_set_library()
{
  // get path string
  str_t* path    = vh_textinput_get_text(ui.slibpopup_inputfield_view);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_set_organize_lib()
{
  //char* text = tg_text_get(ui.set_org_btn_txt);

  // TODO fix
  callbacks_call("on_change_organize", "Enable");
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

void ui_on_filter_activate(view_t* view)
{
  textlist_update(ui.genrelist);
  textlist_update(ui.artistlist);

  ui_toggle_baseview(ui.filters_popup_page);
}

void ui_clear_search()
{
  vh_textinput_set_text(ui.songlist_filter_bar, "");
  ui_manager_activate(ui.songlist_filter_bar);
}

void ui_on_key_down(void* userdata, void* data)
{
  if (userdata == ui.baseview)
  {
    ev_t* ev = (ev_t*)data;
    if (ev->keycode == SDLK_SPACE)
    {
      int state = player_toggle_pause();
      ui_toggle_pause(state);
    }
  }
}

void ui_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  printf("button down %s\n", id);

  if (strcmp(id, "maxbtn") == 0) wm_toggle_fullscreen();
  if (strcmp(id, "app_close_btn") == 0) wm_close();
  if (strcmp(id, "playbtn") == 0) ui_on_play_button_down(NULL);
  if (strcmp(id, "mutebtn") == 0) ui_on_mute_button_down(NULL);
  if (strcmp(id, "nextbtn") == 0) ui_play_next();
  if (strcmp(id, "shufflebtn") == 0) ui_toggle_shuffle();
  if (strcmp(id, "prevbtn") == 0) ui_play_index(ui.lastindex - 1);
  if (strcmp(id, "settingsbtn") == 0) ui_toggle_baseview(ui.settings_popup_page);
  if (strcmp(id, "closesettingsbtn") == 0) ui_toggle_baseview(ui.settings_popup_page);
  if (strcmp(id, "aboutbtn") == 0) ui_toggle_baseview(ui.about_popup_page);
  if (strcmp(id, "editbtn") == 0) ui_toggle_baseview(ui.editor_popup_page);
  if (strcmp(id, "info") == 0) ui_toggle_baseview(ui.messages_popup_page);
  if (strcmp(id, "closefilterbtn") == 0) ui_toggle_baseview(ui.filters_popup_page);
  if (strcmp(id, "closeeditorbtn") == 0) ui_toggle_baseview(ui.editor_popup_page);
  if (strcmp(id, "accepteditorbtn") == 0) ui_editor_accept();
  if (strcmp(id, "chlib_pop_acc_btn") == 0) ui_change_library();
  if (strcmp(id, "acceptlibbtn") == 0) ui_set_library();
  if (strcmp(id, "library_popup_close_btn") == 0) ui_toggle_baseview(ui.library_popup_page);
  if (strcmp(id, "dec_pop_acc_btn") == 0) ui_set_organize_lib();
  if (strcmp(id, "filterbtn") == 0) ui_on_filter_activate(ui.filters_popup_page);
  if (strcmp(id, "visuright_btn") == 0) ui_change_visu();
  if (strcmp(id, "visuleft_btn") == 0) ui_change_visu();
  if (strcmp(id, "clearbtn") == 0) ui_clear_search();

  if (strcmp(id, "song_popup_page_btn") == 0) ui_toggle_baseview(ui.song_popup_page);
  if (strcmp(id, "messages_popup_page_btn") == 0) ui_toggle_baseview(ui.messages_popup_page);
  if (strcmp(id, "ideditor_popup_page_btn") == 0) ui_toggle_baseview(ui.editor_popup_page);
  if (strcmp(id, "about_popup_page_btn") == 0) ui_toggle_baseview(ui.about_popup_page);
  if (strcmp(id, "simple_popup_page_btn") == 0) ui_toggle_baseview(ui.simple_popup_page);
  if (strcmp(id, "settings_popup_page_btn") == 0) ui_toggle_baseview(ui.settings_popup_page);
  if (strcmp(id, "filters_popup_page_btn") == 0) ui_toggle_baseview(ui.filters_popup_page);
  if (strcmp(id, "decision_popup_page_btn") == 0) ui_toggle_baseview(ui.decision_popup_page);
  if (strcmp(id, "library_popup_page_btn") == 0) ui_toggle_baseview(ui.library_popup_page);
}

void ui_on_color_select(void* userdata, void* data)
{
  num_t* val = data;

  /* char text[10] = {0}; */
  /* snprintf(text, 10, "%.2x%.2x%.2x", r, g, b); */
  /* tg_text_set(ui.set_col_val, text, ts); */
}

void ui_on_song_edit(int index)
{
  ui_toggle_baseview(ui.song_popup_page);
}

void ui_on_song_header(char* id)
{
  callbacks_call("on_song_header", id);
}

void ui_on_genre_select(int index)
{
  printf("on genre select %i\n", index);

  vec_t* genres = visible_get_genres();
  char*  genre  = genres->data[index];
  callbacks_call("on_genre_selected", genre);
}

void ui_on_artist_select(int index)
{
  printf("on artist select %i\n", index);

  vec_t* artists = visible_get_artists();
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
  vh_anim_alpha(ui.visuleftbtnbck, 0.0, 1.0, 10, AT_LINEAR);
  vh_anim_alpha(ui.visurightbtnbck, 1.0, 0.0, 10, AT_LINEAR);
}

void ui_on_roll_out_visu(void* userdata, void* data)
{
  vh_anim_alpha(ui.visuleftbtnbck, 1.0, 0.0, 10, AT_LINEAR);
  vh_anim_alpha(ui.visurightbtnbck, 1.0, 0.0, 10, AT_LINEAR);
}

void ui_set_org_btn_lbl(char* text)
{
  // TODO get style from css
  textstyle_t ts = {0};
  ts.font        = ui.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 25.0;
  ts.textcolor   = 0x333333FF;
  ts.backcolor   = 0;

  tg_text_set(ui.set_org_btn_txt, text, ts);
}

void ui_show_song_info(int index)
{
  textstyle_t ts = {0};
  ts.font        = ui.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 28.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  map_t* songmap = ui.songs->data[index];

  char* sample = MGET(songmap, "file/sample_rate");
  char* bit    = MGET(songmap, "file/bit_rate");
  int   sr     = atoi(sample);
  int   br     = atoi(bit);

  printf("sr br %i %i\n", sr, br);

  char* infostr = mem_calloc(100, "char*", NULL, NULL);

  snprintf(infostr, 100, "%s\n%s\n%s/%iKHz/%iKbit/%s channels",
           (char*)MGET(songmap, "meta/title"),
           (char*)MGET(songmap, "meta/artist"),
           (char*)MGET(songmap, "meta/genre"),
           sr / 1000,
           br / 1000,
           (char*)MGET(songmap, "file/channels"));

  tg_text_set(ui.song_info_view, infostr, ts);
}

void ui_toggle_pause(int state)
{
  songlist_toggle_pause(state);
}

void ui_update_position(float ratio)
{
  tg_knob_set_angle(ui.seekknob, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_volume(float ratio)
{
  tg_knob_set_angle(ui.volknob, ratio * 6.28 - 3.14 / 2.0);
}

void ui_update_visualizer()
{
  if (ui.visu)
  {
    player_draw_rdft(0, ui.visuleft->texture.bitmap, 3);
    player_draw_rdft(1, ui.visuright->texture.bitmap, 3);
  }
  else
  {
    player_draw_waves(0, ui.visuleft->texture.bitmap, 3);
    player_draw_waves(1, ui.visuright->texture.bitmap, 3);
  }

  ui.visuleft->texture.changed  = 1;
  ui.visuright->texture.changed = 1;
}

void ui_update_video()
{
  player_draw_video(ui.visuvideo->texture.bitmap, 3);
  ui.visuvideo->texture.changed = 1;
}

void ui_update_time(double time, double left, double dur)
{
  textstyle_t ts = {0};
  ts.font        = ui.fontpath;
  ts.align       = TA_RIGHT;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;
  ts.margin_left = 17;

  char timebuff[20];

  int tmin = (int)floor(time / 60.0);
  int tsec = (int)time % 60;
  int lmin = (int)floor(left / 60.0);
  int lsec = (int)left % 60;
  int dmin = (int)floor(dur / 60.0);
  int dsec = (int)dur % 60;

  ts.align = TA_LEFT;
  snprintf(timebuff, 20, "%.2i:%.2i", dmin, dsec);
  tg_text_set(ui.song_length_view, timebuff, ts);
  snprintf(timebuff, 20, "%.2i:%.2i", tmin, tsec);
  tg_text_set(ui.song_time_view, timebuff, ts);
  snprintf(timebuff, 20, "%.2i:%.2i", lmin, lsec);
  tg_text_set(ui.song_remaining_view, timebuff, ts);
}

void ui_hide_libpath_popup()
{
  if (ui.library_page->parent)
  {
    ui_manager_remove(ui.library_page);
    ui_manager_add(ui.baseview);
  }
}

void ui_show_simple_popup(char* text)
{
  textstyle_t ts = {0};
  ts.font        = ui.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(ui.sim_pop_txt, text, ts);

  ui_toggle_baseview(ui.simple_popup_page);
}

void ui_show_libpath_popup1(char* text)
{
  ui_toggle_baseview(ui.library_popup_page);
}

void ui_show_liborg_popup(char* text)
{
  ui_toggle_baseview(ui.decision_popup_page);
}

void ui_show_libpath_popup(char* text)
{
  textstyle_t ts = {0};
  ts.font        = ui.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(ui.slibpopup_textfield_view, text, ts);

  if (!ui.library_page->parent)
  {
    ui_manager_remove(ui.baseview);
    ui_manager_add(ui.library_page);
  }

  vh_textinput_activate(ui.slibpopup_inputfield_view, 1); // activate text input
  ui_manager_activate(ui.slibpopup_inputfield_view);      // set text input as event receiver
}

void ui_on_songlistpopup_select(int index)
{
  ui_toggle_baseview(ui.song_popup_page);

  if (index == 0) songlist_select(index);
  if (index == 1) songlist_select_range(index);
  if (index == 2) songlist_select_all();
  if (index == 3)
  {
    vec_reset(ui.selected);
    songlist_get_selected(ui.selected);
    editor_popup_set_songs(ui.selected, config_get("lib_path"));
    ui_toggle_baseview(ui.editor_popup_page);
  }
  if (index == 4)
  {
    ui_show_simple_popup("Are you sure you want to delete x items?");
  }
}

void ui_filter(view_t* view)
{
  str_t* text = vh_textinput_get_text(view);

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
  vh_textinput_set_text(ui.songlist_filter_bar, text);
}

#endif
