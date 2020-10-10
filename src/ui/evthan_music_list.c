#ifndef evthan_music_list_h
#define evthan_music_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _mlist_t
{
  mtvec_t* items;
} mlist_t;

mlist_t* evthan_music_list_init();
void     evthan_music_list(view_t* view, ev_t event);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtstring.c"

void evthan_music_list_del(void* pointer)
{
  mlist_t* list = (mlist_t*)pointer;
  REL(list->items);
}

mlist_t* evthan_music_list_new()
{
  mlist_t* data = mtmem_alloc(sizeof(mlist_t), evthan_music_list_del);
  data->items   = VNEW();

  VADD(data->items, mtstr_frombytes("ITEM 0"));
  VADD(data->items, mtstr_frombytes("ITEM 1"));
  VADD(data->items, mtstr_frombytes("ITEM 2"));
  VADD(data->items, mtstr_frombytes("ITEM 3"));

  //view_t* viewB = view_new("viewb", (v4_t){200.0, 420.0, 350.0, 170.0}, evthan_drag, texgen_text, NULL);

  return data;
}

void evthan_music_list(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE && ev.drag)
  {
  }
}

#endif
