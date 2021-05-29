#ifndef ui_filter_bar_h
#define ui_filter_bar_h

#include "view.c"

void ui_filter_bar_attach(view_t* baseview);
void ui_filter_bar_clear_search();
void ui_filter_bar_show_query(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

void ui_filter_bar_show_filters(void* userdata, void* data);
void ui_filter_bar_clear_search(void* userdata, void* data);

#include "callbacks.c"
#include "config.c"
#include "ui_filter_popup.c"
#include "ui_manager.c"
#include "ui_songlist.c"
#include "vh_button.c"
#include "vh_textinput.c"
#include "visible.c"

void ui_filter_bar_filter(view_t* view);

struct _ui_filter_bar_t
{
  view_t* songlist_filter_bar;

} ufb = {0};

void ui_filter_bar_attach(view_t* baseview)
{
  ufb.songlist_filter_bar                          = view_get_subview(baseview, "filterfield");
  ufb.songlist_filter_bar->layout.background_color = 0xFFFFFFFF;

  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.size         = 30.0;
  ts.margin_right = 20;
  ts.align        = TA_LEFT;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0xFFFFFFFF;

  view_t* clearbtn  = view_get_subview(baseview, "clearbtn");
  view_t* filterbtn = view_get_subview(baseview, "filterbtn");

  cb_t* filter_cb = cb_new(ui_filter_bar_show_filters, NULL);
  cb_t* clear_cb  = cb_new(ui_filter_bar_clear_search, NULL);

  vh_button_add(filterbtn, VH_BUTTON_NORMAL, filter_cb);
  vh_button_add(clearbtn, VH_BUTTON_NORMAL, clear_cb);

  REL(filter_cb);
  REL(clear_cb);

  vh_textinput_add(ufb.songlist_filter_bar, "", "Search/Filter", ts, NULL);
  vh_textinput_set_on_text(ufb.songlist_filter_bar, ui_filter_bar_filter);
  //vh_textinput_set_on_activate(ufb.songlist_filter_bar, ui_on_filter_activate);
}

void ui_filter_bar_filter(view_t* view)
{
  str_t* text = vh_textinput_get_text(view);

  // callbacks_call("on_filter_songs", text);

  char* ctext = str_cstring(text);

  visible_set_filter(ctext);
  ui_songlist_update();
}

void ui_filter_bar_show_filters(void* userdata, void* data)
{
  ui_filter_popup_show();
}
void ui_filter_bar_clear_search(void* userdata, void* data)
{
  printf("CLEAR\n");
  vh_textinput_set_text(ufb.songlist_filter_bar, "");
  ui_manager_activate(ufb.songlist_filter_bar);
}

void ui_filter_bar_show_query(char* text)
{
  vh_textinput_set_text(ufb.songlist_filter_bar, text);
}

#endif
