#ifndef donatelist_h
#define donatelist_h

#include "mtmap.c"
#include "view.c"

void donatelist_attach(view_t* base, char* fontpath);
void donatelist_update();
void donatelist_refresh();
void donatelist_toggle_pause(int state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "filtered.c"
#include "selected.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"

void donatelist_on_header_field_select(view_t* view, char* id, ev_t ev);
void donatelist_on_header_field_insert(view_t* view, int src, int tgt);
void donatelist_on_header_field_resize(view_t* view, char* id, int size);

struct donatelist_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;
} donl = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* donl_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void donatelist_update()
{
  vh_list_reset(donl.view);
}

void donatelist_refresh()
{
  vh_list_refresh(donl.view);
}

void donatelist_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*donl.on_header_select)(id);
}

void donatelist_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = donl.fields->data[src];

  RET(cell);
  VREM(donl.fields, cell);
  vec_ins(donl.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(donl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void donatelist_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < donl.fields->length; i++)
  {
    sl_cell_t* cell = donl.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(donl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

// items

void donatelist_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  if (ev.button == 1)
  {
    if (!ev.ctrl_down && !ev.shift_down) selected_res();

    if (ev.shift_down)
    {
      selected_rng(index);
    }
    else
    {
      selected_add(index);
    }

    // if (ev.dclick && donl.on_select) (*donl.on_select)(index);

    vh_list_refresh(donl.view);
  }
  else if (ev.button == 3)
  {
    //if (donl.on_edit) (*donl.on_edit)(index);
  }
}

view_t* donateitem_create()
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "donlist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 50});
  rowview->hidden = 1;

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, donatelist_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(donl.fields)))
  {
    view_t* cellview = view_new(cstr_fromformat("%s%s", rowview->id, cell->id, NULL), (r2_t){0, 0, cell->size, 50});
    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void donateitem_update_row(view_t* rowview, int index, char* field, char* value)
{
  uint32_t color1          = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  donl.textstyle.backcolor = color1;

  tg_text_set(vh_litem_get_cell(rowview, "key"), field, donl.textstyle);
  tg_text_set(vh_litem_get_cell(rowview, "value"), value, donl.textstyle);
}

view_t* donatelist_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= donl.items->length)
    return NULL; // no more items

  *item_count = donl.items->length;

  return donl.items->data[index];
}

void donatelist_attach(view_t* view,
                       char*   fontpath)
{
  assert(fontpath != NULL);

  donl.view   = view;
  donl.fields = VNEW();
  donl.items  = VNEW();

  donl.textstyle.font        = fontpath;
  donl.textstyle.align       = 0;
  donl.textstyle.margin_left = 10;
  donl.textstyle.size        = 30.0;
  donl.textstyle.textcolor   = 0x000000FF;
  donl.textstyle.backcolor   = 0xF5F5F5FF;

  // create fields

  VADD(donl.fields, donl_cell_new("key", 300, 0));
  VADD(donl.fields, donl_cell_new("value", 340, 1));

  vec_dec_retcount(donl.fields);

  // add header handler

  view_t* header = view_new("donatelist_header", (r2_t){0, 0, 10, 30});
  /* header->layout.background_color = 0x333333FF; */
  /* header->layout.shadow_blur      = 3; */
  /* header->layout.border_radius    = 3; */
  tg_css_add(header);

  vh_lhead_add(header);
  vh_lhead_set_on_select(header, donatelist_on_header_field_select);
  vh_lhead_set_on_insert(header, donatelist_on_header_field_insert);
  vh_lhead_set_on_resize(header, donatelist_on_header_field_resize);

  sl_cell_t* cell;
  while ((cell = VNXT(donl.fields)))
  {
    view_t* cellview = view_new(cstr_fromformat("%s%s", header->id, cell->id, NULL), (r2_t){0, 0, cell->size, 30});
    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, donl.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);
  }

  // add list handler to view

  vh_list_add(donl.view, donatelist_item_for_index, NULL, NULL);
  vh_list_set_header(donl.view, header);

  // create items

  VADD(donl.items, donateitem_create());
  VADD(donl.items, donateitem_create());
  VADD(donl.items, donateitem_create());

  donateitem_update_row(donl.items->data[0], 0, "Library Path", "/home/user/milgra/Music");
  donateitem_update_row(donl.items->data[1], 1, "Keep Library Organized", "Disabled");
  donateitem_update_row(donl.items->data[2], 2, "Dark Mode", "Disabled");
}

#endif
