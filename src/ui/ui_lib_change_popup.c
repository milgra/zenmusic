#ifndef ui_lib_change_popup_h
#define ui_lib_change_popup_h

#include "view.c"

void ui_lib_change_popup_attach(view_t* baseview);
void ui_lib_change_popup_show();

void ui_show_libpath_popup1(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "config.c"
#include "text.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_textinput.c"

struct _ui_lib_change_popup_t
{
  view_t* lib_change_inputfield;
} ulcp = {0};

void ui_lib_change_on_button_down(void* userdata, void* data);

void ui_lib_change_popup_attach(view_t* baseview)
{
  view_t* lib_change_textfield = view_get_subview(baseview, "lib_change_textfield");

  ulcp.lib_change_inputfield = view_get_subview(baseview, "lib_change_inputfield");

  cb_t* cb_btn_press = cb_new(ui_lib_change_on_button_down, NULL);

  vh_button_add(view_get_subview(baseview, "lib_change_accept_btn"), VH_BUTTON_NORMAL, cb_btn_press);

  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  tg_text_add(lib_change_textfield);
  tg_text_set(lib_change_textfield, "Use library at", ts);
  vh_textinput_add(ulcp.lib_change_inputfield, "/home/youruser/Music", "", ts, NULL);
}

void ui_change_library()
{
  // get path string
  str_t* path    = vh_textinput_get_text(ulcp.lib_change_inputfield);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_lib_change_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "lib_change_accept_btn") == 0) ui_change_library();
}

#endif
