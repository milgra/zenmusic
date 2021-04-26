/*
  settingslist table and events handler
 */
#ifndef settingslist_h
#define settingslist_h

#include "mtmap.c"
#include "view.c"

void settingslist_attach(view_t* view,
                         char*   fontpath,
                         void (*libpath_popup)(char* text),
                         void (*liborg_popup)(char* text),
                         void (*info_popup)(char* text));
void settingslist_update();
void settingslist_refresh();
void settingslist_toggle_pause(int state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "selection.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"

void settingslist_on_header_field_select(view_t* view, char* id, ev_t ev);
void settingslist_on_header_field_insert(view_t* view, int src, int tgt);
void settingslist_on_header_field_resize(view_t* view, char* id, int size);

struct settingslist_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;

  void (*libpath_popup)(char* text);
  void (*liborg_popup)(char* text);
  void (*info_popup)(char* text);
} setl = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* setl_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void settingslist_update()
{
  vh_list_reset(setl.view);
}

void settingslist_refresh()
{
  vh_list_refresh(setl.view);
}

void settingslist_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*setl.on_header_select)(id);
}

void settingslist_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = setl.fields->data[src];

  RET(cell);
  VREM(setl.fields, cell);
  vec_ins(setl.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(setl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void settingslist_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < setl.fields->length; i++)
  {
    sl_cell_t* cell = setl.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(setl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

// items

void settingslist_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  printf("item select %i\n", index);

  switch (index)
  {
  case 0:
    (*setl.libpath_popup)(NULL);
    break;
  case 1:
    (*setl.liborg_popup)(NULL);
    break;
  case 3:
    (*setl.info_popup)("You cannot set the config path");
    break;
  case 4:
    (*setl.info_popup)("You cannot set the style path");
    break;
  }
}

view_t* settingsitem_create()
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "setlist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 640, 50});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, settingslist_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(setl.fields)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", rowview->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 50});
    REL(id);

    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void settingsitem_update_row(view_t* rowview, int index, char* field, char* value)
{
  uint32_t color1          = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  setl.textstyle.backcolor = color1;

  vh_litem_upd_index(rowview, index);

  tg_text_set(vh_litem_get_cell(rowview, "key"), field, setl.textstyle);
  tg_text_set(vh_litem_get_cell(rowview, "value"), value, setl.textstyle);
}

view_t* settingslist_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= setl.items->length)
    return NULL; // no more items

  *item_count = setl.items->length;

  return setl.items->data[index];
}

void settingslist_attach(view_t* view,
                         char*   fontpath,
                         void (*libpath_popup)(char* text),
                         void (*liborg_popup)(char* text),
                         void (*info_popup)(char* text))
{
  assert(fontpath != NULL);

  setl.view   = view;
  setl.fields = VNEW();
  setl.items  = VNEW();

  setl.libpath_popup = libpath_popup;
  setl.liborg_popup  = liborg_popup;
  setl.info_popup    = info_popup;

  setl.textstyle.font        = fontpath;
  setl.textstyle.align       = 0;
  setl.textstyle.margin_left = 10;
  setl.textstyle.size        = 30.0;
  setl.textstyle.textcolor   = 0x000000FF;
  setl.textstyle.backcolor   = 0xF5F5F5FF;

  // create fields

  VADD(setl.fields, setl_cell_new("key", 300, 0));
  VADD(setl.fields, setl_cell_new("value", 340, 1));

  vec_dec_retcount(setl.fields);

  // add header handler

  view_t* header = view_new("settingslist_header", (r2_t){0, 0, 10, 30});
  /* header->layout.background_color = 0x333333FF; */
  /* header->layout.shadow_blur      = 3; */
  /* header->layout.border_radius    = 3; */
  tg_css_add(header);

  vh_lhead_add(header);
  vh_lhead_set_on_select(header, settingslist_on_header_field_select);
  vh_lhead_set_on_insert(header, settingslist_on_header_field_insert);
  vh_lhead_set_on_resize(header, settingslist_on_header_field_resize);

  sl_cell_t* cell;
  while ((cell = VNXT(setl.fields)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", header->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 30});
    REL(id);

    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, setl.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);
  }

  // add list handler to view

  vh_list_add(setl.view,
              ((vh_list_inset_t){30, 0, 0, 0}),
              settingslist_item_for_index, NULL, NULL);
  vh_list_set_header(setl.view, header);

  // create items

  VADD(setl.items, settingsitem_create());
  VADD(setl.items, settingsitem_create());
  VADD(setl.items, settingsitem_create());
  VADD(setl.items, settingsitem_create());
  VADD(setl.items, settingsitem_create());
  VADD(setl.items, settingsitem_create());

  settingsitem_update_row(setl.items->data[0], 0, "Library Path", "/home/user/milgra/Music");
  settingsitem_update_row(setl.items->data[1], 1, "Organize Library", "Disabled");
  settingsitem_update_row(setl.items->data[2], 2, "Dark Mode", "Disabled");
  settingsitem_update_row(setl.items->data[3], 3, "Remote Control", "Disabled");
  settingsitem_update_row(setl.items->data[4], 4, "Config Path", "/home/.config/zenmusic/config");
  settingsitem_update_row(setl.items->data[5], 5, "Style Path", "/usr/local/share/zenmusic");
}

#endif
