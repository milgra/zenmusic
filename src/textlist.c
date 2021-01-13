#ifndef textlist_h
#define textlist_h

#include "cr_text.c"
#include "view.c"

typedef struct _textlist_t
{
  vec_t*      items;
  view_t*     view;
  textstyle_t textstyle;

  void (*on_select)(int index);
} textlist_t;

textlist_t* textlist_new(view_t* view, vec_t* items, textstyle_t textstyle, void (*on_select)(int));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "vh_list.c"
#include "vh_list_item.c"
#include "view_util.c"

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

void on_textitem_select(view_t* view, void* data, int index, ev_t ev)
{
  textlist_t* tl = data;
  (*tl->on_select)(index);
}

view_t* textlist_create_item(view_t* listview, void* data)
{
  textlist_t* tl = data;

  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "textlist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, on_textitem_select, tl);
  vh_litem_add_cell(rowview, "item", 230, cr_text_add, cr_text_upd);

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

  vh_litem_upd_cell(item, "item", &((cr_text_data_t){.style = tl->textstyle, .text = tl->items->data[index]}));

  return 0;
}

#endif
