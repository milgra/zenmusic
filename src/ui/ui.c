#ifndef ui_h
#define ui_h

#include "mtmap.c"
#include "view.c"

void ui_init();
void ui_load(float width, float height);
void ui_show_query(char* text);        // show query in search/query field
void ui_set_org_btn_lbl(char* text);   // sets label of organize library button
void ui_toggle_baseview(view_t* view); // shows/hides subview on baseview

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "config.c"
#include "itemlist.c"
#include "mtcstring.c"
#include "mtnumber.c"
#include "player.c"
#include "tg_text.c"
#include "ui_activity_popup.c"
#include "ui_alert_popup.c"
#include "ui_donate_popup.c"
#include "ui_editor_popup.c"
#include "ui_filter_popup.c"
#include "ui_lib_change_popup.c"
#include "ui_lib_init_popup.c"
#include "ui_manager.c"
#include "ui_play_controls.c"
#include "ui_popup_switcher.c"
#include "ui_settings_popup.c"
#include "ui_song_infos.c"
#include "ui_song_menu_popup.c"
#include "ui_songlist.c"
#include "ui_visualizer.c"
#include "vh_button.c"
#include "vh_key.c"
#include "vh_textinput.c"
#include "vh_touch.c"
#include "view_generator.c"
#include "view_layout.c"
#include "visible.c"
#include "wm_connector.c"
#include <limits.h>

struct _ui_t
{
  view_t* baseview;
  view_t* songlist_filter_bar;
} ui = {0};

void ui_toggle_pause(int state);
void ui_on_button_down(void* userdata, void* data);
void ui_on_song_edit(int index);
void ui_remove_from_base(view_t* view, void* userdata);
void ui_on_key_down(void* userdata, void* data);
void ui_on_song_header(char* id);
void ui_filter(view_t* view);
void ui_show_liborg_popup(char* text);

void ui_init()
{
}

void ui_load(float width, float height)
{
  text_init();

  // event setup

  callbacks_set("on_button_press", cb_new(ui_on_button_down, NULL));

  // setup callbacks in subnamespaces

  ui_play_controls_init();
  ui_song_infos_init();
  ui_visualizer_init();
  ui_alert_popup_init();
  ui_popup_switcher_init();
  ui_lib_init_popup_init();
  ui_lib_change_popup_init();
  ui_filter_popup_init();
  ui_activity_popup_init();

  // view setup with inited callbacks

  vec_t* views = view_gen_load(config_get("html_path"),
                               config_get("css_path"),
                               config_get("res_path"),
                               callbacks_get_data());

  ui.baseview = vec_head(views);

  ui_play_controls_attach(ui.baseview);
  ui_song_infos_attach(ui.baseview, config_get("font_path"));
  ui_visualizer_attach(ui.baseview);
  ui_alert_popup_attach(ui.baseview, config_get("font_path"));
  ui_lib_init_popup_attach(ui.baseview, config_get("font_path"));
  ui_lib_change_popup_attach(ui.baseview, config_get("font_path"));
  ui_filter_popup_attach(ui.baseview);
  ui_activity_popup_attach(view_get_subview(ui.baseview, "messagelist"), view_get_subview(ui.baseview, "song_info"), config_get("font_path"));

  cb_t* key_cb = cb_new(ui_on_key_down, ui.baseview);

  vh_key_add(ui.baseview, key_cb);

  view_set_frame(ui.baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(ui.baseview);

  ui_manager_init(width, height);
  ui_manager_add(ui.baseview);

  // list setup

  ui_songlist_attach(ui.baseview, config_get("font_path"), ui_play_index, ui_on_song_edit, ui_on_song_header);

  // display views

  cb_t* msg_show_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(view_get_subview(ui.baseview, "song_info"), VH_BUTTON_NORMAL, msg_show_cb);

  // query field

  view_t* main      = view_get_subview(ui.baseview, "main");
  main->needs_touch = 0;

  ui.songlist_filter_bar                          = view_get_subview(ui.baseview, "filterfield");
  ui.songlist_filter_bar->layout.background_color = 0xFFFFFFFF;

  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;
  ts.align        = TA_RIGHT;
  ts.margin_right = 20;

  ts.align     = TA_LEFT;
  ts.textcolor = 0x000000FF;
  ts.backcolor = 0xFFFFFFFF;

  vh_textinput_add(ui.songlist_filter_bar, "", "Search/Filter", ts, NULL);
  vh_textinput_set_on_text(ui.songlist_filter_bar, ui_filter);
  //vh_textinput_set_on_activate(ui.songlist_filter_bar, ui_on_filter_activate);

  // vh_textfield_add(ui.songlist_filter_bar, ts);

  // song editor

  ui_editor_popup_attach(view_get_subview(ui.baseview, "song_editor_popup"), config_get("font_path"));

  // lib input popup

  // decision popup

  ts.multiline       = 1;
  view_t* dec_pop_tf = view_get_subview(ui.baseview, "dec_pop_tf");
  tg_text_add(dec_pop_tf);
  tg_text_set(dec_pop_tf, "Files will be renamed and moved to different folders based on artist, album, track number and title, are you sure?", ts);

  ui_settings_popup_attach(view_get_subview(ui.baseview, "settingslist"), config_get("font_path"), NULL, ui_show_liborg_popup, NULL);

  ui_donate_popup_attach(view_get_subview(ui.baseview, "aboutlist"), config_get("font_path"), NULL);

  ui_song_menu_popup_attach(view_get_subview(ui.baseview, "song_popup_list"), config_get("font_path"), NULL);

  // popup setup, it removes views so it has to be the last command

  ui_popup_switcher_attach(ui.baseview);

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

void ui_editor_accept()
{
  ui_popup_switcher_toggle("ideditor_popup_page");

  map_t* changed = ui_editor_popup_get_changed();
  vec_t* removed = ui_editor_popup_get_removed();
  char*  cover   = ui_editor_popup_get_cover();

  ui_alert_popup_show("ARE YOU SURE?");

  /* char* libpath = config_get("lib_path"); */

  /* editor_update_metadata(libpath, ui.selected, changed, removed, cover); */

  // update metadata in media files

  // update metadata in database

  // organize library ( if path has to change )

  /* map_t* old_data = ui_editor_popup_get_old_data(); */
  /* map_t* new_data = ui_editor_popup_get_new_data(); */

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
  /* ui_songlist_refresh(); */
}

void ui_set_organize_lib()
{
  //char* text = tg_text_get(ui.set_org_btn_txt);

  // TODO fix
  callbacks_call("on_change_organize", "Enable");
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

  if (strcmp(id, "maxbtn") == 0) wm_toggle_fullscreen();
  if (strcmp(id, "app_close_btn") == 0) wm_close();

  if (strcmp(id, "dec_pop_acc_btn") == 0) ui_set_organize_lib();
  if (strcmp(id, "clearbtn") == 0) ui_clear_search();
  if (strcmp(id, "accepteditorbtn") == 0) ui_editor_accept();

  // if (strcmp(id, "filterbtn") == 0) ui_on_filter_activate(MGET(ui.popup_views, "filters_popup_page"));

  if (strcmp(id, "settingsbtn") == 0) ui_popup_switcher_toggle("settings_popup_page");
  if (strcmp(id, "closesettingsbtn") == 0) ui_popup_switcher_toggle("settings_popup_page");
  if (strcmp(id, "aboutbtn") == 0) ui_popup_switcher_toggle("about_popup_page");
  if (strcmp(id, "editbtn") == 0) ui_popup_switcher_toggle("ideditor_popup_page");
  if (strcmp(id, "song_info") == 0) ui_popup_switcher_toggle("messages_popup_page");
  if (strcmp(id, "closefilterbtn") == 0) ui_popup_switcher_toggle("filters_popup_page");
  if (strcmp(id, "closeeditorbtn") == 0) ui_popup_switcher_toggle("ideditor_popup_page");
  if (strcmp(id, "library_popup_close_btn") == 0) ui_popup_switcher_toggle("library_popup_page");
}

void ui_on_song_edit(int index)
{
  ui_popup_switcher_toggle("song_popup_page");
}

void ui_on_song_header(char* id)
{
  callbacks_call("on_song_header", id);
}

void ui_set_org_btn_lbl(char* text)
{
}

void ui_toggle_pause(int state)
{
  ui_songlist_toggle_pause(state);
}

void ui_show_libpath_popup1(char* text)
{
  ui_popup_switcher_toggle("library_popup_page");
}

void ui_show_liborg_popup(char* text)
{
  ui_popup_switcher_toggle("decision_popup_page");
}

void ui_filter(view_t* view)
{
  str_t* text = vh_textinput_get_text(view);

  callbacks_call("on_filter_songs", text);
}

void ui_show_query(char* text)
{
  vh_textinput_set_text(ui.songlist_filter_bar, text);
}

#endif