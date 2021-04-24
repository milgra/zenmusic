#ifndef donatelist_h
#define donatelist_h

#include "mtmap.c"
#include "view.c"

void donatelist_attach(view_t* base, char* fontpath, void (*popup)(char* text));
void donatelist_update();
void donatelist_refresh();
void donatelist_toggle_pause(int state);

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

void donatelist_on_header_field_select(view_t* view, char* id, ev_t ev);
void donatelist_on_header_field_insert(view_t* view, int src, int tgt);
void donatelist_on_header_field_resize(view_t* view, char* id, int size);

struct donatelist_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;
  void (*popup)(char* text);
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
  if (donl.popup) (*donl.popup)("The link is opened in the browser.");

  switch (index)
  {
  case 1:
    system("xdg-open https://paypal.me/milgra");
    break;
  case 2:
    system("xdg-open https://patreon.com/milgra");
    break;
  case 3:
    system("xdg-open https://github.com/milgra/zenmusic");
    break;
  case 4:
    system("xdg-open https://yotubue.com/milgra");
    break;
  }
}

view_t* donateitem_create(int index)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "donlist_item%i", item_cnt++);

  float height = index == 0 ? 150 : 50;

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 460, height});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, donatelist_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(donl.fields)))
  {
    char* id = cstr_fromformat(100, "%s%s", rowview->id, cell->id);

    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, height});

    REL(id);
    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void donateitem_update_row(view_t* rowview, int index, char* field)
{
  uint32_t color1          = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  donl.textstyle.backcolor = color1;

  vh_litem_upd_index(rowview, index);

  tg_text_set(vh_litem_get_cell(rowview, "field"), field, donl.textstyle);
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

void donatelist_attach(view_t* view, char* fontpath, void (*popup)(char* text))
{
  assert(fontpath != NULL);

  donl.view   = view;
  donl.fields = VNEW();
  donl.items  = VNEW();
  donl.popup  = popup;

  donl.textstyle.font      = fontpath;
  donl.textstyle.align     = TA_CENTER;
  donl.textstyle.margin    = 10;
  donl.textstyle.size      = 30.0;
  donl.textstyle.textcolor = 0x000000FF;
  donl.textstyle.backcolor = 0xF5F5F5FF;

  // create fields

  VADD(donl.fields, donl_cell_new("field", 460, 0));

  vec_dec_retcount(donl.fields);

  // add list handler to view

  vh_list_add(donl.view,
              ((vh_list_inset_t){0, 0, 0, 0}),
              donatelist_item_for_index, NULL, NULL);

  // create items

  VADD(donl.items, donateitem_create(0));
  VADD(donl.items, donateitem_create(1));
  VADD(donl.items, donateitem_create(2));
  VADD(donl.items, donateitem_create(3));
  VADD(donl.items, donateitem_create(4));
  VADD(donl.items, donateitem_create(5));

  donateitem_update_row(donl.items->data[0], 0, "Zen Music v0.8\n by Milan Toth\nFree and Open Source Software.\nIf you like it, please support the development.");
  donateitem_update_row(donl.items->data[1], 1, "Donate on Paypal");
  donateitem_update_row(donl.items->data[2], 2, "Support on Patreon");
  donateitem_update_row(donl.items->data[3], 3, "GitHub Page");
  donateitem_update_row(donl.items->data[4], 4, "Youtube Channel");
  donateitem_update_row(donl.items->data[5], 5, "Report an Issue");
}

#endif
