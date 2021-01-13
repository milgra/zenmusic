#ifndef songlist_h
#define songlist_h

typedef struct _sitem_cell_t
{
  char* id;
  int   size;
  int   index;
} sitem_cell_t;

#include "mtmap.c"
#include "view.c"

sitem_cell_t* sitem_cell_new(char* id, int size, int index);

void songlist_attach(view_t* base, vec_t* songs, char* fontpath, void (*on_select)(int), void (*on_edit)(int), void (*on_header_select)(char*));
void songlist_update();
void songlist_toggle_selected(int state);

view_t* songlist_item_new(char* fontpath, void (*on_select)(view_t* view, uint32_t index, ev_t ev), vec_t* fields);
void    songlist_item_update(view_t* rowview, int index, map_t* file, char* fontpath, vec_t* fields);
void    songlist_item_select(view_t* rowview, int index, map_t* file, char* fontpath, vec_t* fields, uint32_t color);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cr_text.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"
#include "view_util.c"

uint32_t songlist_index = 0;
vec_t*   songlist_fields;
view_t*  songlist;
char*    songlist_fontpath;
vec_t*   songlist_songs;
void (*songlist_item_on_select)(int index);
void (*songlist_item_on_edit)(int index);
void (*songlist_header_on_select)(char* id);

uint32_t selected_index = UINT32_MAX;
uint32_t selected_color = 0x55FF55FF;

void on_header_field_select(view_t* view, char* id, ev_t ev)
{
  printf("on_header_field_select %s\n", id);
  (*songlist_header_on_select)(id);
}

void on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sitem_cell_t* cell = songlist_fields->data[src];
  RET(cell);
  VREM(songlist_fields, cell);
  vec_addatindex(songlist_fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(songlist);
  while ((item = VNXT(cache)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
  vec_t* items = vh_list_items(songlist);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < songlist_fields->length; i++)
  {
    sitem_cell_t* cell = songlist_fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }
  // update all items and cache
  view_t* item;
  vec_t*  cache = vh_list_cache(songlist);
  while ((item = VNXT(cache)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
  vec_t* items = vh_list_items(songlist);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

void songlist_on_select(view_t* view, uint32_t index, ev_t ev)
{
  printf("select button %i\n", ev.button);

  if (ev.button == 1)
  {
    // deselect prev item
    view_t* olditem = vh_list_item_for_index(songlist, selected_index);

    if (olditem)
    {
      songlist_item_update(olditem, selected_index, songlist_songs->data[selected_index], songlist_fontpath, songlist_fields);
    }

    // indicate list item
    view_t* newitem = vh_list_item_for_index(songlist, index);

    if (newitem)
    {
      songlist_item_select(newitem, index, songlist_songs->data[index], songlist_fontpath, songlist_fields, selected_color);
    }
    selected_index = index;

    (*songlist_item_on_select)(index);
  }
  else if (ev.button == 3)
  {
    (*songlist_item_on_edit)(index);
  }
}

view_t* songlist_create_item(view_t* listview)
{
  return songlist_item_new(songlist_fontpath, songlist_on_select, songlist_fields);
}

int songlist_update_item(view_t* listview, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= songlist_songs->length)
    return 1; // no more items

  *item_count = songlist_songs->length;

  if (selected_index == index)
  {
    songlist_item_select(item, index, songlist_songs->data[index], songlist_fontpath, songlist_fields, selected_color);
  }
  else
  {
    songlist_item_update(item, index, songlist_songs->data[index], songlist_fontpath, songlist_fields);
  }
  return 0;
}

void songlist_attach(view_t* base, vec_t* songs, char* fontpath, void (*on_select)(int), void (*on_edit)(int), void (*on_header_select)(char*))
{
  songlist_fontpath = fontpath;

  songlist = view_get_subview(base, "songlist");

  songlist_songs = songs;

  songlist_item_on_select   = on_select;
  songlist_item_on_edit     = on_edit;
  songlist_header_on_select = on_header_select;

  vh_list_add(songlist, songlist_create_item, songlist_update_item);

  songlist_fields = VNEW();
  VADD(songlist_fields, sitem_cell_new("index", 50, 0));
  VADD(songlist_fields, sitem_cell_new("artist", 300, 1));
  VADD(songlist_fields, sitem_cell_new("title", 300, 2));
  VADD(songlist_fields, sitem_cell_new("date", 150, 3));
  VADD(songlist_fields, sitem_cell_new("genre", 150, 4));
  VADD(songlist_fields, sitem_cell_new("track", 150, 5));
  VADD(songlist_fields, sitem_cell_new("disc", 150, 6));
  VADD(songlist_fields, sitem_cell_new("plays", 150, 7));
  VADD(songlist_fields, sitem_cell_new("added", 150, 8));
  VADD(songlist_fields, sitem_cell_new("last played", 150, 9));
  VADD(songlist_fields, sitem_cell_new("last skipped", 150, 10));

  // decrease retain count of cells because of inline allocation
  vec_dec_retcount(songlist_fields);

  view_t* songlistheader = view_get_subview(base, "songlistheader");

  textstyle_t ts = {0};

  ts.font        = fontpath;
  ts.align       = 0;
  ts.margin_left = 10;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xEFEFEFFF;

  vh_lhead_add(songlistheader, 30, on_header_field_select, on_header_field_insert, on_header_field_resize);

  sitem_cell_t* cell;
  while ((cell = VNXT(songlist_fields)))
  {
    vh_lhead_add_cell(songlistheader, cell->id, cell->size, cr_text_upd);
    vh_lhead_upd_cell(songlistheader, cell->id, cell->size, &((cr_text_data_t){.style = ts, .text = cell->id}));
  }
}

void songlist_update()
{
  vh_list_reset(songlist);
}

void songlist_toggle_selected(int state)
{
  if (state)
    selected_color = 0xFF5555FF;
  else
    selected_color = 0x55FF55FF;

  view_t* item = vh_list_item_for_index(songlist, selected_index);

  if (item) songlist_item_select(item, selected_index, songlist_songs->data[selected_index], songlist_fontpath, songlist_fields, selected_color);
}

sitem_cell_t* sitem_cell_new(char* id, int size, int index)
{
  sitem_cell_t* cell = mem_calloc(sizeof(sitem_cell_t), "sitem_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

view_t* songlist_item_new(char* fontpath, void (*on_select)(view_t* view, uint32_t index, ev_t ev), vec_t* fields)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songlist_index++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, on_select);

  sitem_cell_t* cell;
  while ((cell = VNXT(fields)))
  {
    vh_litem_add_cell(rowview, cell->id, cell->size, cr_text_add, cr_text_upd);
  }

  return rowview;
}

void songlist_item_update(view_t* rowview, int index, map_t* file, char* fontpath, vec_t* fields)
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = 0;
  ts.margin_left = 10;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = color1;

  vh_litem_upd(rowview, index);

  sitem_cell_t* cell;
  while ((cell = VNXT(fields)))
  {
    if (MGET(file, cell->id))
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = ts, .text = MGET(file, cell->id)}));
    else
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = ts, .text = "-"}));
  }

  vh_litem_upd_cell(rowview, "index", &((cr_text_data_t){.style = ts, .text = indbuffer}));
}

void songlist_item_select(view_t* rowview, int index, map_t* file, char* fontpath, vec_t* fields, uint32_t color)
{
  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = 0;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = color;

  vh_litem_upd(rowview, index);

  sitem_cell_t* cell;
  while ((cell = VNXT(fields)))
  {
    if (MGET(file, cell->id))
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = ts, .text = MGET(file, cell->id)}));
    else
      vh_litem_upd_cell(rowview, cell->id, &((cr_text_data_t){.style = ts, .text = "-"}));
  }

  vh_litem_upd_cell(rowview, "index", &((cr_text_data_t){.style = ts, .text = indbuffer}));
}

#endif
