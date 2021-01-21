#ifndef activity_h
#define activity_h

#include "view.c"

void activity_init();
void activity_attach(view_t* logview, view_t* notifview, char* fontpath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtlog.c"
#include "text.c"
#include "textlist.c"
#include <pthread.h>

struct _activity_t
{
  vec_t*          logs;
  int             ind;
  textlist_t*     list;
  pthread_mutex_t lock;
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

  if (act.list) textlist_update(act.list);

  pthread_mutex_unlock(&act.lock);
}

void activity_init()
{
  act.logs = VNEW();
  log_set_proxy(activity_log);
}

void activity_attach(view_t* logview, view_t* notifview, char* fontpath)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  ts.align = TA_LEFT;
  act.list = textlist_new(logview, act.logs, ts, activity_select);
}

#endif
