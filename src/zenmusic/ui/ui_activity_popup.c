#ifndef ui_activity_popup_h
#define ui_activity_popup_h

#include "text.c"
#include "view.c"

void ui_activity_popup_attach(view_t* baseview);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "textlist.c"
#include "tg_text.c"
#include "zc_log.c"
#include <pthread.h>

struct _ui_activity_popup_t
{
  vec_t*          logs;
  int             ind;
  textlist_t*     list;
  view_t*         info;
  pthread_mutex_t lock;
  textstyle_t     style;
} act = {0};

void ui_activity_popup_select(int index)
{
  printf("on_ui_activity_popupitem_select\n");
}

void ui_activity_popup_log(char* log)
{
  pthread_mutex_lock(&act.lock); // have to be thread safe

  char* last = act.logs->data[0];

  if (last && strstr(log, "progress"))
  {
    vec_replaceatindex(act.logs, log, 0);
  }
  else
    vec_ins(act.logs, log, 0);

  if (act.logs->length > 100) vec_rematindex(act.logs, 100);

  if (act.list)
  {
    textlist_update(act.list);

    tg_text_set(act.info, log, act.style);
  }

  pthread_mutex_unlock(&act.lock);
}

void ui_activity_popup_attach(view_t* baseview)
{
  act.logs = VNEW();
  log_set_proxy(ui_activity_popup_log);

  view_t* listview = view_get_subview(baseview, "messages_popup_list");
  view_t* infoview = view_get_subview(baseview, "song_info");

  textstyle_t ts = {0};
  ts.font        = config_get("font_path");
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0x0;
  ts.align       = TA_LEFT;
  ts.margin      = 10.0;

  act.style = ts;
  act.list  = textlist_new(listview, ts, ui_activity_popup_select);
  act.info  = infoview;

  act.style.align = TA_CENTER;

  textlist_set_datasource(act.list, act.logs);
}

#endif
