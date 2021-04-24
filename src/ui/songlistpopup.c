#ifndef songlistpopup_h
#define songlistpopup_h

#include "mtmap.c"
#include "view.c"

void songlistpopup_attach(view_t* view, char* fontpath, void (*on_select)(int));
void songlistpopup_update();
void songlistpopup_refresh();
void songlistpopup_toggle_pause(int state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "filtered.c"
#include "selection.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"

void songlistpopup_on_header_field_select(view_t* view, char* id, ev_t ev);
void songlistpopup_on_header_field_insert(view_t* view, int src, int tgt);
void songlistpopup_on_header_field_resize(view_t* view, char* id, int size);

struct songlistpopup_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;
  void (*on_select)(int index);
} slp = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* slp_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void songlistpopup_update()
{
  vh_list_reset(slp.view);
}

void songlistpopup_refresh()
{
  vh_list_refresh(slp.view);
}

void songlistpopup_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*slp.on_header_select)(id);
}

void songlistpopup_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = slp.fields->data[src];

  RET(cell);
  VREM(slp.fields, cell);
  vec_ins(slp.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(slp.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void songlistpopup_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < slp.fields->length; i++)
  {
    sl_cell_t* cell = slp.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(slp.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

// items

void songlistpopup_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  if (slp.on_select) (*slp.on_select)(index);
}

view_t* songlistpopupitem_create(int index)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "slpist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 460, 50});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, songlistpopup_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(slp.fields)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", rowview->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 50});
    REL(id);

    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void songlistpopupitem_update_row(view_t* rowview, int index, char* field)
{
  uint32_t color1         = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  slp.textstyle.backcolor = color1;

  vh_litem_upd_index(rowview, index);

  tg_text_set(vh_litem_get_cell(rowview, "field"), field, slp.textstyle);
}

view_t* songlistpopup_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= slp.items->length)
    return NULL; // no more items

  *item_count = slp.items->length;

  return slp.items->data[index];
}

void songlistpopup_attach(view_t* view, char* fontpath, void (*on_select)(int))
{
  assert(fontpath != NULL);

  slp.view      = view;
  slp.fields    = VNEW();
  slp.items     = VNEW();
  slp.on_select = on_select;

  slp.textstyle.font      = fontpath;
  slp.textstyle.align     = TA_CENTER;
  slp.textstyle.margin    = 10;
  slp.textstyle.size      = 30.0;
  slp.textstyle.textcolor = 0x000000FF;
  slp.textstyle.backcolor = 0xF5F5F5FF;

  // create fields

  VADD(slp.fields, slp_cell_new("field", 240, 0));

  vec_dec_retcount(slp.fields);

  // add list handler to view

  vh_list_add(slp.view,
              ((vh_list_inset_t){0, 10, 0, 10}),
              songlistpopup_item_for_index, NULL, NULL);

  // create items

  VADD(slp.items, songlistpopupitem_create(0));
  VADD(slp.items, songlistpopupitem_create(1));
  VADD(slp.items, songlistpopupitem_create(2));
  VADD(slp.items, songlistpopupitem_create(3));
  VADD(slp.items, songlistpopupitem_create(4));

  songlistpopupitem_update_row(slp.items->data[0], 0, "Select/Deselect");
  songlistpopupitem_update_row(slp.items->data[1], 1, "Select Range");
  songlistpopupitem_update_row(slp.items->data[2], 2, "Select All");
  songlistpopupitem_update_row(slp.items->data[3], 3, "Edit Song Info");
  songlistpopupitem_update_row(slp.items->data[4], 4, "Delete Song");
}

#endif
