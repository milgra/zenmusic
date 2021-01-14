#ifndef activity_h
#define activity_h

#include "view.c"

void activity_init();
void activity_attach(view_t* view, char* fontpath);
void activity_log(char* msg, ...);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cr_text.c"
#include "text.c"
#include "vh_list.c"
#include "vh_list_item.c"

struct _activity_t
{
  vec_t*  logs;
  int     ind;
  char*   fontpath;
  view_t* view;
} act = {0};

void activity_select(view_t* view, void* userdata, int index, ev_t ev)
{
  printf("on_activityitem_select\n");
}

view_t* activity_create_item(view_t* listview, void* userdata)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "activity_item%i", act.ind++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, activity_select, NULL);
  vh_litem_add_cell(rowview, "message", 460, cr_text_add, cr_text_upd);

  return rowview;
}

int activity_update_item(view_t* listview, void* userdata, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= act.logs->length)
    return 1; // no more items

  printf("activity update %i\n", index);

  *item_count = act.logs->length;

  textstyle_t ts = {0};
  ts.font        = act.fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  vh_litem_upd_cell(item, "message", &((cr_text_data_t){.style = ts, .text = act.logs->data[index]}));

  return 0;
}

void activity_init()
{
  act.logs = VNEW();
}

void activity_attach(view_t* view, char* fontpath)
{
  vh_list_add(view, activity_create_item, activity_update_item, NULL);

  act.view     = view;
  act.fontpath = fontpath;
}

void activity_log(char* fmt, ...)
{
  va_list arglist;
  char*   str = mem_calloc(100, "char*", NULL, NULL);
  va_start(arglist, fmt);
  vsnprintf(str, 100, fmt, arglist);
  va_end(arglist);

  VADD(act.logs, str);

  if (act.view) vh_list_fill(act.view);

  printf("LOG %s\n", str);
}

#endif
