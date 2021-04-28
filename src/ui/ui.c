#ifndef ui_h
#define ui_h

#include "view.c"

void ui_load(float width, float height);

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
#include "ui_filter_bar.c"
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

void ui_on_button_down(void* userdata, void* data);
void ui_on_key_down(void* userdata, void* data);

void ui_load(float width, float height)
{
  text_init();

  // callbacks setup for simple buttons

  callbacks_set("on_button_press", cb_new(ui_on_button_down, NULL));

  // setup callbacks in subnamespaces

  ui_song_infos_init();
  ui_visualizer_init();
  ui_filter_bar_init();
  ui_alert_popup_init();
  ui_filter_popup_init();
  ui_play_controls_init();
  ui_activity_popup_init();
  ui_popup_switcher_init();
  ui_lib_init_popup_init();
  ui_lib_change_popup_init();

  // view setup with inited callbacks

  vec_t* views = view_gen_load(config_get("html_path"),
                               config_get("css_path"),
                               config_get("res_path"),
                               callbacks_get_data());

  view_t* baseview = vec_head(views);

  // initial layout of views

  view_set_frame(baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(baseview);

  // setup ui manager

  ui_manager_init(width, height);
  ui_manager_add(baseview);

  // attach ui components

  ui_songlist_attach(baseview);
  ui_song_infos_attach(baseview);
  ui_visualizer_attach(baseview);
  ui_filter_bar_attach(baseview);
  ui_alert_popup_attach(baseview);
  ui_filter_popup_attach(baseview);
  ui_editor_popup_attach(baseview);
  ui_donate_popup_attach(baseview);
  ui_play_controls_attach(baseview);
  ui_lib_init_popup_attach(baseview);
  ui_activity_popup_attach(baseview);
  ui_settings_popup_attach(baseview);
  ui_song_menu_popup_attach(baseview);
  ui_lib_change_popup_attach(baseview);

  // setup views

  view_t* mainview = view_get_subview(baseview, "main");
  cb_t*   key_cb   = cb_new(ui_on_key_down, baseview);
  cb_t*   but_cb   = cb_new(ui_on_button_down, NULL);

  mainview->needs_touch = 0;                                                        // don't cover events from songlist
  vh_key_add(baseview, key_cb);                                                     // listen on baseview for shortcuts
  vh_button_add(view_get_subview(mainview, "song_info"), VH_BUTTON_NORMAL, but_cb); // show messages on song info click

  // finally attach and remove popups, it removes views so it has to be the last command

  ui_popup_switcher_attach(baseview);

  // set glossy effect on header

  /* view_t* header = view_get_subview(baseview, "header"); */
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

// key event from base view

void ui_on_key_down(void* userdata, void* data)
{
  ev_t* ev = (ev_t*)data;
  if (ev->keycode == SDLK_SPACE)
  {
    int state = player_toggle_pause();
    ui_songlist_toggle_pause(state);
  }
}

// button down event from descriptor html

void ui_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "app_close_btn") == 0) wm_close();
  if (strcmp(id, "app_maximize_btn") == 0) wm_toggle_fullscreen();

  // todo sanitize button names

  if (strcmp(id, "editbtn") == 0) ui_popup_switcher_toggle("ideditor_popup_page");
  if (strcmp(id, "aboutbtn") == 0) ui_popup_switcher_toggle("about_popup_page");
  if (strcmp(id, "song_info") == 0) ui_popup_switcher_toggle("messages_popup_page");
  if (strcmp(id, "settingsbtn") == 0) ui_popup_switcher_toggle("settings_popup_page");
  if (strcmp(id, "closefilterbtn") == 0) ui_popup_switcher_toggle("filters_popup_page");
  if (strcmp(id, "closeeditorbtn") == 0) ui_popup_switcher_toggle("ideditor_popup_page");
  if (strcmp(id, "closesettingsbtn") == 0) ui_popup_switcher_toggle("settings_popup_page");
  if (strcmp(id, "library_popup_close_btn") == 0) ui_popup_switcher_toggle("library_popup_page");
}

#endif
