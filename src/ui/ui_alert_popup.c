#ifndef ui_alert_popup_h
#define ui_alert_popup_h

#include "view.c"

void ui_alert_popup_attach(view_t* baseview);
void ui_alert_popup_show(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "text.c"
#include "tg_text.c"

struct _ui_alert_popup_t
{
  view_t* sim_pop_txt;
  char*   fontpath;
} uap = {0};

void ui_alert_popup_attach(view_t* baseview)
{
  uap.sim_pop_txt = view_get_subview(baseview, "sim_pop_txt");
  tg_text_add(uap.sim_pop_txt);
  uap.fontpath = config_get("font_path");
}

void ui_alert_popup_show(char* text)
{
  textstyle_t ts = {0};
  ts.font        = uap.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  tg_text_set(uap.sim_pop_txt, text, ts);

  // ui_toggle_baseview(MGET(uap.popup_views, "simple_popup_page"));
}

#endif
