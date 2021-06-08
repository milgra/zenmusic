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

textlist_t* textlist_new(view_t* view, textstyle_t textstyle, void (*on_select)(int));
void        textlist_update(textlist_t* tl);
void        textlist_set_datasource(textlist_t* tl, vec_t* items);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_text.c"
#include "vh_list.c"
#include "vh_list_item.c"

void    textlist_del(void* p);
view_t* textlist_item_for_index(int index, void* data, view_t* listview, int* item_count);

textlist_t* textlist_new(view_t* view, textstyle_t textstyle, void (*on_select)(int))
{
  textlist_t* tl = mem_calloc(sizeof(textlist_t), "textlist", textlist_del, NULL);

  tl->view      = view;
  tl->textstyle = textstyle;
  tl->on_select = on_select;

  vh_list_add(view, ((vh_list_inset_t){0, 10, 0, 10}), textlist_item_for_index, NULL, tl);

  return tl;
}

void textlist_del(void* p)
{
  textlist_t* tl = p;
}

void textlist_update(textlist_t* tl)
{
  vh_list_refresh(tl->view);
}

void textlist_set_datasource(textlist_t* tl, vec_t* items)
{
  tl->items = items;
}

void on_textitem_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  vh_litem_t* vh = itemview->handler_data;
  textlist_t* tl = vh->userdata;

  (*tl->on_select)(vh->index);
}

view_t* textlist_create_item(textlist_t* tl)
{
  static int item_cnt = 0;

  char* item_id = cstr_fromformat(100, "tetlist_item%i", item_cnt++);
  char* cell_id = cstr_fromformat(100, "%s%s", item_id, "cell");

  view_t* item_view = view_new(item_id, (r2_t){0, 0, 0, 35});
  view_t* cell_view = view_new(cell_id, (r2_t){0, 0, tl->view->frame.local.w, 35});

  REL(item_id);
  REL(cell_id);

  tg_text_add(cell_view);

  vh_litem_add(item_view, tl);
  vh_litem_set_on_select(item_view, on_textitem_select);
  vh_litem_add_cell(item_view, "cell", 200, cell_view);

  return item_view;
}

view_t* textlist_item_for_index(int index, void* data, view_t* listview, int* item_count)
{
  textlist_t* tl = data;

  if (index < 0) return NULL;                  // no items before 0
  if (index >= tl->items->length) return NULL; // no more items

  *item_count = tl->items->length;

  view_t*  item  = textlist_create_item(tl);
  uint32_t color = (index % 2 == 0) ? 0xFFFFFFFF : 0xFAFAFAFF;

  tl->textstyle.backcolor = color;

  int    h;
  str_t* str = str_new(); // REL 0
  str_addbytearray(str, tl->items->data[index]);
  text_measure(str, tl->textstyle, item->frame.local.w, &h);
  REL(str);

  if (h < 35) h = 35;

  printf("ITEM %i %s\n", h, tl->items->data[index]);

  view_t* cell = vh_litem_get_cell(item, "cell");

  r2_t frame = item->frame.local;
  frame.h    = h;

  view_set_frame(item, frame);

  frame   = cell->frame.local;
  frame.h = h;

  view_set_frame(cell, frame);

  vh_litem_upd_index(item, index);
  tg_text_set(cell, tl->items->data[index], tl->textstyle);

  return item;
}

#endif
