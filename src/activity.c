#ifndef activity_h
#define activity_h

#include "view.c"

void activity_init();
void activity_attach(view_t* view, char* fontpath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtlog.c"
#include "text.c"
#include "textlist.c"

struct _activity_t
{
  vec_t*      logs;
  int         ind;
  textlist_t* list;
} act = {0};

void activity_select(int index)
{
  printf("on_activityitem_select\n");
}

void activity_log(char* log)
{
  vec_ins(act.logs, log, 0);
  if (act.logs->length > 100) vec_rematindex(act.logs, 100);

  if (act.list) textlist_update(act.list);
}

void activity_init()
{
  act.logs = VNEW();
  log_set_proxy(activity_log);
}

void activity_attach(view_t* view, char* fontpath)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  ts.align = TA_LEFT;
  act.list = textlist_new(view, act.logs, ts, activity_select);
}

#endif
