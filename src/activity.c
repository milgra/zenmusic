#ifndef activity_h
#define activity_h

#include "text.c"
#include "view.c"

void activity_init();
void activity_attach(view_t* logview, view_t* notifview, textstyle_t style);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtlog.c"
#include "textlist.c"
#include "tg_text.c"
#include <pthread.h>

struct _activity_t
{
  vec_t*          logs;
  int             ind;
  textlist_t*     list;
  view_t*         info;
  pthread_mutex_t lock;
  textstyle_t     style;
} act = {0};

void activity_select(int index)
{
  printf("on_activityitem_select\n");
}

void activity_log(char* log)
{
  pthread_mutex_lock(&act.lock); // have to be thread safe

  char* last = act.logs->data[0];

  if (last && last[0] == log[0])
    vec_replaceatindex(act.logs, log, 0);
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

void activity_init()
{
  act.logs = VNEW();
  log_set_proxy(activity_log);
}

void activity_attach(view_t* logview, view_t* notifview, textstyle_t style)
{
  act.style = style;
  act.list  = textlist_new(logview, act.logs, style, activity_select);
  act.info  = notifview;
}

#endif
