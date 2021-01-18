#ifndef textlist_h
#define textlist_h

#include "text.c"
#include "view.c"

typedef struct _textlist_t
{
  vec_t*      items;
  view_t*     view;
  textstyle_t textstyle;

  void (*on_select)(int index);
} textlist_t;

textlist_t* textlist_new(view_t* view, vec_t* items, textstyle_t textstyle, void (*on_select)(int));
void        textlist_update(textlist_t* tl);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_text.c"
#include "vh_list.c"
#include "vh_list_item.c"

view_t* textlist_create_item(view_t* listview, void* data);
int     textlist_update_item(view_t* listview, void* data, view_t* item, int index, int* item_count);

void textlist_del(void* p)
{
  textlist_t* tl = p;
}

textlist_t* textlist_new(view_t* view, vec_t* items, textstyle_t textstyle, void (*on_select)(int))
{
  textlist_t* tl = mem_calloc(sizeof(textlist_t), "textlist", textlist_del, NULL);

  tl->items = items;

  vh_list_add(view, textlist_create_item, textlist_update_item, tl);

  tl->view      = view;
  tl->textstyle = textstyle;

  return tl;
}

void textlist_update(textlist_t* tl)
{
  vh_list_reset(tl->view);
}

void on_textitem_select(view_t* itemview)
{
  vh_litem_t* vh = itemview->handler_data;

  textlist_t* tl = vh->userdata;
  (*tl->on_select)(vh->index);
}

view_t* textlist_create_item(view_t* listview, void* data)
{
  textlist_t* tl = data;

  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "textlist_item%i", item_cnt++);

  view_t* rowview  = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->display = 0;
  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, on_textitem_select);

  view_t* cell = view_new(cstr_fromformat("%s%s", rowview->id, "cell", NULL), (r2_t){0, 0, 200, 35});
  tg_text_add(cell);

  vh_litem_add_cell(rowview, "cell", 200, cell);

  return rowview;
}

int textlist_update_item(view_t* listview, void* data, view_t* item, int index, int* item_count)
{
  textlist_t* tl = data;
  if (index < 0)
    return 1; // no items before 0
  if (index >= tl->items->length)
    return 1; // no more items

  *item_count = tl->items->length;

  vh_litem_upd_index(item, index);
  tg_text_set(vh_litem_get_cell(item, "cell"), tl->items->data[index], tl->textstyle);

  return 0;
}

#endif
