#ifndef itemlist_h
#define itemlist_h

#include "text.c"
#include "view.c"

typedef struct _itemlist_t
{
  vec_t*  items;
  view_t* view;
} itemlist_t;

itemlist_t* itemlist_new(view_t* view, vec_t* items);
void        itemlist_update(itemlist_t* tl);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "vh_list.c"

view_t* itemlist_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  itemlist_t* il = userdata;
  if (index < il->items->length && index > 0) return il->items->data[index];

  return NULL;
}

void itemlist_del(void* p)
{
  itemlist_t* il = (itemlist_t*)p;
  REL(il->view);
  REL(il->items);
}

itemlist_t* itemlist_new(view_t* view, vec_t* items)
{
  itemlist_t* il = mem_calloc(sizeof(itemlist_t), "itemlist", itemlist_del, NULL);
  il->view       = view;
  il->items      = items;

  vh_list_add(view, itemlist_item_for_index, NULL, NULL);

  return il;
}

void itemlist_update(itemlist_t* il)
{
  vh_list_refresh(il->view);
}

#endif
