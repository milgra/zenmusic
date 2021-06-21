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

  if (tl->items) REL(tl->items); // REL 0
}

void textlist_update(textlist_t* tl)
{
  vh_list_refresh(tl->view);
}

void textlist_set_datasource(textlist_t* tl, vec_t* items)
{
  if (tl->items) REL(tl->items);
  tl->items = items; // REL 0
  RET(items);
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

  char* item_id = cstr_new_format(100, "tetlist_item%i", item_cnt++); // REL 0
  char* cell_id = cstr_new_format(100, "%s%s", item_id, "cell");      // REL 1

  view_t* item_view = view_new(item_id, (r2_t){0, 0, 0, 35});                       // REL 2
  view_t* cell_view = view_new(cell_id, (r2_t){0, 0, tl->view->frame.local.w, 35}); // REL 3

  tg_text_add(cell_view);

  vh_litem_add(item_view, tl);
  vh_litem_set_on_select(item_view, on_textitem_select);
  vh_litem_add_cell(item_view, "cell", 200, cell_view);

  REL(item_id);   // REL 0
  REL(cell_id);   // REL 1
  REL(item_view); // REL 2
  REL(cell_view); // REL 3

  return item_view;
}

view_t* textlist_item_for_index(int index, void* data, view_t* listview, int* item_count)
{
  textlist_t* tl = data;

  if (index < 0) return NULL;                  // no items before 0
  if (index >= tl->items->length) return NULL; // no more items

  *item_count = tl->items->length;

  view_t*  item  = textlist_create_item(tl); // REL 0
  uint32_t color = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;

  tl->textstyle.backcolor = color;

  str_t* str = str_new(); // REL 1
  str_addbytearray(str, tl->items->data[index]);
  int nw;
  int nh;
  text_measure(str, tl->textstyle, item->frame.local.w, item->frame.local.h, &nw, &nh);

  if (nh < 35) nh = 35;

  view_t* cell = vh_litem_get_cell(item, "cell");

  r2_t frame = item->frame.local;
  frame.h    = nh;

  view_set_frame(item, frame);

  frame   = cell->frame.local;
  frame.h = nh;

  view_set_frame(cell, frame);

  vh_litem_upd_index(item, index);
  tg_text_set(cell, tl->items->data[index], tl->textstyle);

  REL(str); // REL 1

  return item;
}

#endif
