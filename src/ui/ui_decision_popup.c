#ifndef ui_decision_popup_h
#define ui_decision_popup_h

#include "view.c"

void ui_decision_popup_init();
void ui_decision_popup_attach(view_t* baseview);
void ui_decision_popup_show(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "tg_text.c"

struct _ui_decision_popup_t
{
  view_t* dec_pop_tf;
} udp = {0};

void ui_decision_popup_init()
{
}

void ui_decision_popup_attach(view_t* baseview)
{
  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.size         = 30.0;
  ts.margin_right = 20;
  ts.align        = TA_LEFT;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0xFFFFFFFF;
  ts.multiline    = 1;

  view_t* dec_pop_tf = view_get_subview(baseview, "dec_pop_tf");
  tg_text_add(dec_pop_tf);
  tg_text_set(dec_pop_tf, "Files will be renamed and moved to different folders based on artist, album, track number and title, are you sure?", ts);
}

#endif
