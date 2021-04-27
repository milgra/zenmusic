#ifndef ui_lib_init_popup_h
#define ui_lib_init_popup_h

#include "view.c"

void ui_lib_init_popup_init();
void ui_lib_init_popup_attach(view_t* baseview, char* fontpath);
void ui_lib_init_popup_show(char* text);
void ui_lib_init_popup_hide();
void ui_lib_init_popup_set_library();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "text.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "vh_textinput.c"

struct _ui_lib_init_popup_t
{
  char*   fontpath;
  view_t* baseview;
  view_t* lib_init_textfield_view;
  view_t* lib_init_inputfield_view;
  view_t* lib_init_popup;
} ulip = {0};

void ui_lib_init_on_button_down(void* userdata, void* data);

void ui_lib_init_popup_init()
{
  callbacks_set("on_lib_init_button_press", cb_new(ui_lib_init_on_button_down, NULL));
}

void ui_lib_init_popup_attach(view_t* baseview, char* fontpath)
{
  ulip.fontpath                 = fontpath;
  ulip.baseview                 = baseview;
  ulip.lib_init_popup           = view_get_subview(baseview, "lib_init_page");
  ulip.lib_init_textfield_view  = view_get_subview(baseview, "lib_init_textfield");
  ulip.lib_init_inputfield_view = view_get_subview(baseview, "lib_init_inputfield");

  textstyle_t ts  = {0};
  ts.font         = fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  tg_text_add(ulip.lib_init_textfield_view);
  vh_textinput_add(ulip.lib_init_inputfield_view, "/home/youruser/Music", "", ts, NULL);

  view_remove(baseview, ulip.lib_init_popup);
}

void ui_lib_init_popup_set_library()
{
  // get path string
  str_t* path    = vh_textinput_get_text(ulip.lib_init_inputfield_view);
  char*  path_ch = str_cstring(path);

  callbacks_call("on_change_library", path_ch);
  REL(path_ch);
}

void ui_lib_init_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "acceptlibbtn") == 0) ui_lib_init_popup_set_library();
}

void ui_lib_init_popup_show(char* text)
{
  textstyle_t ts = {0};
  ts.font        = ulip.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(ulip.lib_init_textfield_view, text, ts);

  if (!ulip.lib_init_popup->parent)
  {
    ui_manager_remove(ulip.baseview);
    ui_manager_add(ulip.lib_init_popup);
  }

  vh_textinput_activate(ulip.lib_init_inputfield_view, 1); // activate text input
  ui_manager_activate(ulip.lib_init_inputfield_view);      // set text input as event receiver
}

void ui_lib_init_popup_hide()
{
  if (ulip.lib_init_popup->parent)
  {
    ui_manager_remove(ulip.lib_init_popup);
    ui_manager_add(ulip.baseview);
  }
}

#endif
