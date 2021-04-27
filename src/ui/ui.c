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
void ui_toggle_pause(int state);
void ui_show_libpath_popup(char* message);
void ui_hide_libpath_popup();
void ui_refresh_songlist();
void ui_reload_songlist();
void ui_show_query(char* text);
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
#include "tg_picker.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "ui_play_controls.c"
#include "ui_song_infos.c"
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
  vec_t* selected; // selected songs from songlist
  char*  fontpath; // font path

  view_t* songlist_filter_bar;
  view_t* sim_pop_txt;
  view_t* set_col_val;
  view_t* chlib_pop_if;
  view_t* set_org_btn_txt;

  textlist_t* artistlist;
  textlist_t* genrelist;

  vec_t* popup_names;
  map_t* popup_views;

  view_t* baseview; // root view

  // set library popup fields

  view_t* slibpopup_textfield_view;
  view_t* slibpopup_inputfield_view;

  // popup pages

  view_t* library_page;

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
void ui_on_roll_in_visu(void* userdata, void* data);
void ui_on_roll_out_visu(void* userdata, void* data);
void ui_on_song_edit(int index);
void ui_remove_from_base(view_t* view, void* userdata);
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

  ui_play_controls_init();

  // view setup

  char* csspath  = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.css", respath);
  char* htmlpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.html", respath);

  vec_t* views = view_gen_load(htmlpath, csspath, respath, callbacks_get_data());

  ui.baseview = vec_head(views);

  ui_play_controls_attach(ui.baseview);

  ui_song_infos_attach(ui.baseview, ui.fontpath);

  cb_t* key_cb = cb_new(ui_on_key_down, ui.baseview);

  vh_key_add(ui.baseview, key_cb);

  view_set_frame(ui.baseview, (r2_t){0.0, 0.0, (float)width, (float)height});
  view_layout(ui.baseview);

  ui_manager_init(width, height);
  ui_manager_add(ui.baseview);

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

  // display views

  ts.margin_right = 0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0x0;

  ts.align = TA_LEFT;

  ts.align  = TA_LEFT;
  ts.margin = 10.0;

  // init activity

  activity_init();
  activity_attach(view_get_subview(ui.baseview, "messagelist"), view_get_subview(ui.baseview, "song_info"), ts);

  cb_t* msg_show_cb = cb_new(ui_on_button_down, NULL);
  vh_button_add(view_get_subview(ui.baseview, "song_info"), VH_BUTTON_NORMAL, msg_show_cb);

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

  editor_popup_attach(view_get_subview(ui.baseview, "song_editor_popup"), ui.fontpath);

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

  // change lib popup

  view_t* chlib_pop_tf = view_get_subview(ui.baseview, "chlib_pop_tf");
  ui.chlib_pop_if      = view_get_subview(ui.baseview, "chlib_pop_if");

  ts.backcolor = 0;

  tg_text_add(chlib_pop_tf);
  tg_text_set(chlib_pop_tf, "Use library at", ts);
  vh_textinput_add(ui.chlib_pop_if, "/home/youruser/Music", "", ts, NULL);

  // simple popup text

  ui.sim_pop_txt = view_get_subview(ui.baseview, "sim_pop_txt");
  tg_text_add(ui.sim_pop_txt);

  // settings

  settingslist_attach(view_get_subview(ui.baseview, "settingslist"), ui.fontpath, ui_show_libpath_popup1, ui_show_liborg_popup, ui_show_simple_popup);

  // about view

  donatelist_attach(view_get_subview(ui.baseview, "aboutlist"), ui.fontpath, ui_show_simple_popup);

  // attach songlistpopup to song_popup_list view

  songlistpopup_attach(view_get_subview(ui.baseview, "song_popup_list"), ui.fontpath, ui_on_songlistpopup_select);

  // popup setup

  ui.popup_names = VNEW();
  ui.popup_views = MNEW();

  VADDR(ui.popup_names, cstr_fromcstring("library_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("song_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("messages_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("ideditor_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("about_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("filters_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("settings_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("decision_popup_page"));
  VADDR(ui.popup_names, cstr_fromcstring("simple_popup_page"));

  for (int index = 0; index < ui.popup_names->length; index++)
  {
    char* name = ui.popup_names->data[index];

    view_t* page_view  = view_get_subview(ui.baseview, name);
    view_t* btn_view   = page_view->views->data[0];
    view_t* popup_view = btn_view->views->data[0];

    vh_anim_add(page_view);
    vh_anim_set_event(page_view, page_view, ui_remove_from_base);
    vh_touch_add(btn_view, cb_new(ui_on_button_down, NULL));

    view_remove(ui.baseview, page_view);

    MPUT(ui.popup_views, name, page_view);
  }

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
  ui_toggle_baseview(MGET(ui.popup_views, "ideditor_popup_page"));

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

void ui_on_filter_activate(view_t* view)
{
  textlist_update(ui.genrelist);
  textlist_update(ui.artistlist);

  ui_toggle_baseview(MGET(ui.popup_views, "filters_popup_page"));
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
  if (strcmp(id, "settingsbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "settings_popup_page"));
  if (strcmp(id, "closesettingsbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "settings_popup_page"));
  if (strcmp(id, "aboutbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "about_popup_page"));
  if (strcmp(id, "editbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "ideditor_popup_page"));

  if (strcmp(id, "song_info") == 0) ui_toggle_baseview(MGET(ui.popup_views, "messages_popup_page"));

  if (strcmp(id, "closefilterbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "filters_popup_page"));
  if (strcmp(id, "closeeditorbtn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "ideditor_popup_page"));
  if (strcmp(id, "accepteditorbtn") == 0) ui_editor_accept();
  if (strcmp(id, "chlib_pop_acc_btn") == 0) ui_change_library();
  if (strcmp(id, "acceptlibbtn") == 0) ui_set_library();
  if (strcmp(id, "library_popup_close_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "library_popup_page"));
  if (strcmp(id, "dec_pop_acc_btn") == 0) ui_set_organize_lib();
  if (strcmp(id, "filterbtn") == 0) ui_on_filter_activate(MGET(ui.popup_views, "filters_popup_page"));
  if (strcmp(id, "visuright_btn") == 0) ui_change_visu();
  if (strcmp(id, "visuleft_btn") == 0) ui_change_visu();
  if (strcmp(id, "clearbtn") == 0) ui_clear_search();

  if (strcmp(id, "song_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "song_popup_page"));
  if (strcmp(id, "messages_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "messages_popup_page"));
  if (strcmp(id, "ideditor_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "ideditor_popup_page"));
  if (strcmp(id, "about_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "about_popup_page"));
  if (strcmp(id, "simple_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "simple_popup_page"));
  if (strcmp(id, "settings_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "settings_popup_page"));
  if (strcmp(id, "filters_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "filters_popup_page"));
  if (strcmp(id, "decision_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "decision_popup_page"));
  if (strcmp(id, "library_popup_page_btn") == 0) ui_toggle_baseview(MGET(ui.popup_views, "library_popup_page"));
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
  ui_toggle_baseview(MGET(ui.popup_views, "song_popup_page"));
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

void ui_toggle_pause(int state)
{
  songlist_toggle_pause(state);
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

  ui_toggle_baseview(MGET(ui.popup_views, "simple_popup_page"));
}

void ui_show_libpath_popup1(char* text)
{
  ui_toggle_baseview(MGET(ui.popup_views, "library_popup_page"));
}

void ui_show_liborg_popup(char* text)
{
  ui_toggle_baseview(MGET(ui.popup_views, "decision_popup_page"));
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
  ui_toggle_baseview(MGET(ui.popup_views, "song_popup_page"));

  if (index == 0) songlist_select(index);
  if (index == 1) songlist_select_range(index);
  if (index == 2) songlist_select_all();
  if (index == 3)
  {
    vec_reset(ui.selected);
    songlist_get_selected(ui.selected);
    editor_popup_set_songs(ui.selected, config_get("lib_path"));
    ui_toggle_baseview(MGET(ui.popup_views, "ideditor_popup_page"));
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
