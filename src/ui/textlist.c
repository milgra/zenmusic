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

view_t* textlist_item_for_index(int index, void* data, view_t* listview, int* item_count);

void textlist_del(void* p)
{
  textlist_t* tl = p;
}

textlist_t* textlist_new(view_t* view, vec_t* items, textstyle_t textstyle, void (*on_select)(int))
{
  textlist_t* tl = mem_calloc(sizeof(textlist_t), "textlist", textlist_del, NULL);

  mem_describe(items, 0);

  tl->items = items;

  tl->view      = view;
  tl->textstyle = textstyle;
  tl->on_select = on_select;

  vh_list_add(view,
              ((vh_list_inset_t){0, 10, 0, 10}),
              textlist_item_for_index, NULL, tl);

  return tl;
}

void textlist_update(textlist_t* tl)
{
  vh_list_refresh(tl->view);
}

void on_textitem_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  vh_litem_t* vh = itemview->handler_data;
  textlist_t* tl = vh->userdata;
  (*tl->on_select)(vh->index);
}

view_t* textlist_create_item(textlist_t* tl)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "textlist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});

  vh_litem_add(rowview, tl);
  vh_litem_set_on_select(rowview, on_textitem_select);

  view_t* cell = view_new(cstr_fromformat("%s%s", rowview->id, "cell", NULL), (r2_t){0, 0, tl->view->frame.local.w, 35});
  tg_text_add(cell);

  vh_litem_add_cell(rowview, "cell", 200, cell);

  return rowview;
}

view_t* textlist_item_for_index(int index, void* data, view_t* listview, int* item_count)
{
  textlist_t* tl = data;
  if (index < 0)
    return NULL; // no items before 0
  if (index >= tl->items->length)
    return NULL; // no more items

  *item_count = tl->items->length;

  view_t* item = textlist_create_item(tl);

  uint32_t color = (index % 2 == 0) ? 0xFFFFFFFF : 0xFAFAFAFF;

  tl->textstyle.backcolor = color;

  vh_litem_upd_index(item, index);

  tg_text_set(vh_litem_get_cell(item, "cell"), tl->items->data[index], tl->textstyle);

  return item;
}

#endif
