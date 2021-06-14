#ifndef ui_lib_init_popup_h
#define ui_lib_init_popup_h

#include "view.c"

void ui_lib_init_popup_attach(view_t* baseview);
void ui_lib_init_popup_show(char* text);
void ui_lib_init_popup_hide();
void ui_lib_init_popup_set_library();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "config.c"
#include "text.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "vh_button.c"
#include "vh_textinput.c"
#include "wm_connector.c"
#include "zc_callback.c"

struct _ui_lib_init_popup_t
{
  char*   fontpath;
  view_t* baseview;
  view_t* textfield;
  view_t* inputfield;
  view_t* view;
} ulip = {0};

void ui_lib_init_on_button_down(void* userdata, void* data);
void ui_lib_init_popup_set_library(view_t* view);

void ui_lib_init_popup_attach(view_t* baseview)
{
  ulip.fontpath   = config_get("font_path");
  ulip.baseview   = baseview;
  ulip.view       = view_get_subview(baseview, "lib_init_page");
  ulip.textfield  = view_get_subview(baseview, "lib_init_textfield");
  ulip.inputfield = view_get_subview(baseview, "lib_init_inputfield");

  cb_t* cb_btn_press = cb_new(ui_lib_init_on_button_down, NULL);

  vh_button_add(view_get_subview(baseview, "lib_init_accept_btn"), VH_BUTTON_NORMAL, cb_btn_press);
  vh_button_add(view_get_subview(baseview, "lib_init_reject_btn"), VH_BUTTON_NORMAL, cb_btn_press);
  vh_button_add(view_get_subview(baseview, "lib_init_clear_btn"), VH_BUTTON_NORMAL, cb_btn_press);

  textstyle_t ts  = {0};
  ts.font         = ulip.fontpath;
  ts.align        = TA_LEFT;
  ts.margin_left  = 10;
  ts.margin_right = 10;
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  tg_text_add(ulip.textfield);

  ts.backcolor = 0xFFFFFFFF;
  vh_textinput_add(ulip.inputfield, "/home/youruser/Music", "", ts, NULL);

  vh_textinput_set_on_return(ulip.inputfield, ui_lib_init_popup_set_library);

  view_remove(baseview, ulip.view);
}

void ui_lib_init_popup_set_library(view_t* view)
{
  // get path string
  str_t* path    = vh_textinput_get_text(ulip.inputfield);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_lib_init_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  printf("ID %s\n", id);

  if (strcmp(id, "lib_init_accept_btn") == 0) ui_lib_init_popup_set_library(NULL);
  if (strcmp(id, "lib_init_reject_btn") == 0) wm_close();
  if (strcmp(id, "lib_init_clear_btn") == 0) vh_textinput_set_text(ulip.inputfield, "");
}

void ui_lib_init_popup_show(char* text)
{
  textstyle_t ts = {0};
  ts.font        = ulip.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(ulip.textfield, text, ts);

  if (!ulip.view->parent)
  {
    ui_manager_remove(ulip.baseview);
    ui_manager_add(ulip.view);
  }

  vh_textinput_activate(ulip.inputfield, 1); // activate text input
  ui_manager_activate(ulip.inputfield);      // set text input as event receiver
}

void ui_lib_init_popup_hide()
{
  if (ulip.view->parent)
  {
    ui_manager_remove(ulip.view);
    ui_manager_add(ulip.baseview);
  }
}

#endif
