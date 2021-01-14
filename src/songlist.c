/*
  songlist table and events handler
 */

#ifndef songlist_h
#define songlist_h

#include "mtmap.c"
#include "view.c"

void songlist_attach(view_t* base, vec_t* songs, char* fontpath, void (*on_select)(int), void (*on_edit)(int), void (*on_header_select)(char*));
void songlist_update();
void songlist_toggle_selected(int state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cr_text.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"

view_t* songlist_create_item(view_t* listview, void* userdata);
int     songlist_update_item(view_t* listview, void* userdata, view_t* item, int index, int* item_count);
void    songlist_update_row(view_t* rowview, int index, map_t* file, uint32_t color);

void on_header_field_select(view_t* view, char* id, ev_t ev);
void on_header_field_insert(view_t* view, int src, int tgt);
void on_header_field_resize(view_t* view, char* id, int size);

struct songlist_t
{
  view_t*     view;   // table view
  vec_t*      songs;  // songs to be shown in list
  vec_t*      fields; // fileds in table
  textstyle_t textstyle;

  uint32_t index_s; // selected index
  uint32_t color_s; // color selected

  void (*on_edit)(int index);
  void (*on_select)(int index);
  void (*on_header_select)(char* id);
} sl = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* sl_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void songlist_attach(view_t* base,
                     vec_t*  songs,
                     char*   fontpath,
                     void (*on_select)(int),
                     void (*on_edit)(int),
                     void (*on_header_select)(char*))
{
  assert(base != NULL);
  assert(songs != NULL);
  assert(fontpath != NULL);

  sl.view   = view_get_subview(base, "songlist");
  sl.songs  = songs;
  sl.fields = VNEW();

  sl.index_s = 0;
  sl.color_s = 0x55FF55FF;

  sl.on_edit          = on_edit;
  sl.on_select        = on_select;
  sl.on_header_select = on_header_select;

  sl.textstyle.font        = fontpath;
  sl.textstyle.align       = 0;
  sl.textstyle.margin_left = 10;
  sl.textstyle.size        = 25.0;
  sl.textstyle.textcolor   = 0x000000FF;
  sl.textstyle.backcolor   = 0xEFEFEFFF;

  // add list handler to view

  vh_list_add(sl.view, songlist_create_item, songlist_update_item, NULL);

  // create fields

  VADD(sl.fields, sl_cell_new("index", 50, 0));
  VADD(sl.fields, sl_cell_new("artist", 300, 1));
  VADD(sl.fields, sl_cell_new("title", 300, 2));
  VADD(sl.fields, sl_cell_new("date", 150, 3));
  VADD(sl.fields, sl_cell_new("genre", 150, 4));
  VADD(sl.fields, sl_cell_new("track", 150, 5));
  VADD(sl.fields, sl_cell_new("disc", 150, 6));
  VADD(sl.fields, sl_cell_new("plays", 150, 7));
  VADD(sl.fields, sl_cell_new("added", 150, 8));
  VADD(sl.fields, sl_cell_new("last played", 150, 9));
  VADD(sl.fields, sl_cell_new("last skipped", 150, 10));

  vec_dec_retcount(sl.fields);

  // add header handler

  view_t* header = view_get_subview(base, "songlistheader");

  vh_lhead_add(header, 30, on_header_field_select, on_header_field_insert, on_header_field_resize);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    vh_lhead_add_cell(header, cell->id, cell->size, cr_text_upd);
    vh_lhead_upd_cell(header, cell->id, cell->size, &((cr_text_data_t){.style = sl.textstyle, .text = cell->id}));
  }
}

void songlist_update()
{
  vh_list_reset(sl.view);
}

void songlist_toggle_selected(int state)
{
  if (state)
    sl.color_s = 0xFF5555FF;
  else
    sl.color_s = 0x55FF55FF;

  view_t* item = vh_list_item_for_index(sl.view, sl.index_s);

  if (item) songlist_update_row(item, sl.index_s, sl.songs->data[sl.index_s], sl.color_s);
}

// header

void on_header_field_select(view_t* view, char* id, ev_t ev)
{
  (*sl.on_header_select)(id);
}

void on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = sl.fields->data[src];

  RET(cell);
  VREM(sl.fields, cell);
  vec_addatindex(sl.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(sl.view);
  while ((item = VNXT(cache)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
  vec_t* items = vh_list_items(sl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < sl.fields->length; i++)
  {
    sl_cell_t* cell = sl.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(sl.view);
  while ((item = VNXT(cache)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
  vec_t* items = vh_list_items(sl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

// items

void songlist_on_select(view_t* view, void* userdata, int index, ev_t ev)
{

  if (ev.button == 1)
  {
    // deselect prev item
    view_t* olditem = vh_list_item_for_index(sl.view, sl.index_s);

    if (olditem)
    {
      uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;

      songlist_update_row(olditem, sl.index_s, sl.songs->data[sl.index_s], color1);
    }

    // indicate list item
    view_t* newitem = vh_list_item_for_index(sl.view, index);

    if (newitem)
    {
      songlist_update_row(newitem, sl.index_s, sl.songs->data[sl.index_s], sl.color_s);
    }
    sl.index_s = index;

    (*sl.on_select)(index);
  }
  else if (ev.button == 3)
  {
    (*sl.on_edit)(index);
  }
}

view_t* songlist_create_item(view_t* listview, void* data)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, songlist_on_select, NULL);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    vh_litem_add_cell(rowview, cell->id, cell->size, cr_text_add, cr_text_upd);
  }

  return rowview;
}

void songlist_update_row(view_t* rowview, int index, map_t* file, uint32_t color)
{
  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  sl.textstyle.textcolor = 0x000000FF;
  sl.textstyle.backcolor = color;

  vh_litem_upd(rowview, index);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    if (MGET(file, cell->id))
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = sl.textstyle, .text = MGET(file, cell->id)}));
    else
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = sl.textstyle, .text = "-"}));
  }

  vh_litem_upd_cell(rowview, "index", &((cr_text_data_t){.style = sl.textstyle, .text = indbuffer}));
}

int songlist_update_item(view_t* listview, void* userdata, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= sl.songs->length)
    return 1; // no more items

  *item_count = sl.songs->length;

  if (sl.index_s == index)
  {
    songlist_update_row(item, index, sl.songs->data[index], sl.color_s);
  }
  else
  {
    uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
    uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

    songlist_update_row(item, index, sl.songs->data[index], color1);
  }
  return 0;
}

#endif
