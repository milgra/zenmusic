#ifndef ui_activity_popup_h
#define ui_activity_popup_h

#include "text.c"
#include "view.c"

void ui_activity_popup_init();
void ui_activity_popup_attach(view_t* logview, view_t* notifview, textstyle_t style);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtlog.c"
#include "textlist.c"
#include "tg_text.c"
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

  if (last && log[0] == ' ' && last[0] == ' ')
  {
    printf("length %i\n", act.logs->length);
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

void ui_activity_popup_init()
{
  act.logs = VNEW();
  log_set_proxy(ui_activity_popup_log);
}

void ui_activity_popup_attach(view_t* logview, view_t* notifview, textstyle_t style)
{
  act.style = style;
  act.list  = textlist_new(logview, act.logs, style, ui_activity_popup_select);
  act.info  = notifview;

  act.style.align = TA_CENTER;
}

#endif
