/*
  songlist table and events handler
 */
#ifndef songlist_h
#define songlist_h

#include "mtmap.c"
#include "view.c"

void songlist_select(int index);
void songlist_select_all();
void songlist_select_range(int index);
void songlist_get_selected(vec_t* vec);

void songlist_attach(view_t* base, char* fontpath, void (*on_select)(int), void (*on_edit)(int), void (*on_header_select)(char*));
void songlist_update();
void songlist_refresh();
void songlist_toggle_pause(int state);
void songlist_select_and_show(int index);

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

void on_header_field_select(view_t* view, char* id, ev_t ev);
void on_header_field_insert(view_t* view, int src, int tgt);
void on_header_field_resize(view_t* view, char* id, int size);

struct songlist_t
{
  view_t*     view;   // table view
  vec_t*      cache;  // item cache
  vec_t*      fields; // fileds in table
  textstyle_t textstyle;

  uint32_t color_s; // color selected
  int32_t  index_s; // index selected

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

void songlist_update()
{
  vh_list_reset(sl.view);
}

void songlist_refresh()
{
  vh_list_refresh(sl.view);
}

void songlist_toggle_pause(int state)
{
  if (state)
    sl.color_s = 0xFF5555FF;
  else
    sl.color_s = 0x55FF55FF;

  // view_t* item = vh_list_item_for_index(sl.view, sl.index_s);

  // if (item) songitem_update_row(item, sl.index_s, sl.songs->data[sl.index_s], sl.color_s);
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
  vec_ins(sl.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(sl.view);
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
  vec_t*  items = vh_list_items(sl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

void songlist_select(int index)
{
  selected_res();
  selected_add(sl.index_s);
  vh_list_refresh(sl.view);
}

void songlist_select_range(int index)
{
  selected_rng(sl.index_s);
  vh_list_refresh(sl.view);
}

void songlist_select_all()
{
  selected_res();
  selected_add(0);
  selected_rng(filtered_song_count());
  vh_list_refresh(sl.view);
}

void songlist_get_selected(vec_t* vec)
{
  // if selection is empty, select current index
  if (selected_cnt() < 2) selected_add(sl.index_s);
  selected_add_selected(filtered_get_songs(), vec);
}

void songlist_select_and_show(int index)
{
  selected_res();
  selected_add(index);
  vh_list_scroll_to_index(sl.view, index);
}

// items

void songlist_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  sl.index_s = index;
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
    printf("%i %i\n", index, ev.dclick);
    if (ev.dclick && sl.on_select) (*sl.on_select)(index);

    vh_list_refresh(sl.view);
  }
  else if (ev.button == 3)
  {
    if (sl.on_edit) (*sl.on_edit)(index);
    if (selected_cnt() < 2)
    {
      selected_res();
      selected_add(index);
      vh_list_refresh(sl.view);
    }
  }
}

view_t* songitem_create()
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, songlist_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    view_t* cellview = view_new(cstr_fromformat("%s%s", rowview->id, cell->id, NULL), (r2_t){0, 0, cell->size, 35});
    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void songitem_update_row(view_t* rowview, int index, map_t* file, uint32_t color)
{
  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  sl.textstyle.textcolor = 0x000000FF;
  sl.textstyle.backcolor = color;

  vh_litem_upd_index(rowview, index);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    if (MGET(file, cell->id))
    {
      tg_text_set(vh_litem_get_cell(rowview, cell->id), MGET(file, cell->id), sl.textstyle);
    }
    else
    {
      tg_text_set(vh_litem_get_cell(rowview, cell->id), "-", sl.textstyle);
    }
  }

  tg_text_set(vh_litem_get_cell(rowview, "index"), indbuffer, sl.textstyle);
}

void songlist_item_recycle(view_t* item)
{
  VADD(sl.cache, item);
}

view_t* songlist_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= filtered_song_count())
    return NULL; // no more items

  *item_count = filtered_song_count();

  view_t* item;
  if (sl.cache->length > 0)
    item = sl.cache->data[0];
  else
    item = songitem_create();

  VREM(sl.cache, item);

  if (selected_has(index))
  {
    songitem_update_row(item, index, filtered_song_at_index(index), sl.color_s);
  }
  else
  {
    uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
    uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

    songitem_update_row(item, index, filtered_song_at_index(index), color1);
  }

  return item;
}

void songlist_attach(view_t* base,
                     char*   fontpath,
                     void (*on_select)(int),
                     void (*on_edit)(int),
                     void (*on_header_select)(char*))
{
  assert(base != NULL);
  assert(fontpath != NULL);

  sl.view   = view_get_subview(base, "songlist");
  sl.cache  = VNEW();
  sl.fields = VNEW();

  sl.color_s = 0x55FF55FF;

  sl.on_edit          = on_edit;
  sl.on_select        = on_select;
  sl.on_header_select = on_header_select;

  sl.textstyle.font        = fontpath;
  sl.textstyle.align       = 0;
  sl.textstyle.margin_left = 10;
  sl.textstyle.size        = 30.0;
  sl.textstyle.textcolor   = 0x000000FF;
  sl.textstyle.backcolor   = 0xF5F5F5FF;

  // create fields

  VADD(sl.fields, sl_cell_new("index", 50, 0));
  VADD(sl.fields, sl_cell_new("meta/artist", 300, 1));
  VADD(sl.fields, sl_cell_new("meta/album", 200, 2));
  VADD(sl.fields, sl_cell_new("meta/title", 300, 3));
  VADD(sl.fields, sl_cell_new("meta/date", 150, 4));
  VADD(sl.fields, sl_cell_new("meta/genre", 150, 5));
  VADD(sl.fields, sl_cell_new("meta/track", 150, 6));
  VADD(sl.fields, sl_cell_new("meta/disc", 150, 7));
  VADD(sl.fields, sl_cell_new("file/duration", 100, 8));
  VADD(sl.fields, sl_cell_new("file/channels", 100, 9));
  VADD(sl.fields, sl_cell_new("file/bit_rate", 100, 10));
  VADD(sl.fields, sl_cell_new("file/sample_rate", 100, 11));
  VADD(sl.fields, sl_cell_new("file/play_count", 150, 12));
  VADD(sl.fields, sl_cell_new("file/skip_count", 150, 13));
  VADD(sl.fields, sl_cell_new("file/added", 150, 14));
  VADD(sl.fields, sl_cell_new("file/last_played", 150, 15));
  VADD(sl.fields, sl_cell_new("file/last_skipped", 150, 16));

  vec_dec_retcount(sl.fields);

  // add header handler

  view_t* header = view_new("songlist_header", (r2_t){0, 0, 10, 30});
  /* header->layout.background_color = 0x333333FF; */
  /* header->layout.shadow_blur      = 3; */
  /* header->layout.border_radius    = 3; */
  tg_css_add(header);

  vh_lhead_add(header);
  vh_lhead_set_on_select(header, on_header_field_select);
  vh_lhead_set_on_insert(header, on_header_field_insert);
  vh_lhead_set_on_resize(header, on_header_field_resize);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.fields)))
  {
    view_t* cellview = view_new(cstr_fromformat("%s%s", header->id, cell->id, NULL), (r2_t){0, 0, cell->size, 30});
    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, sl.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);
  }

  // add list handler to view

  vh_list_add(sl.view,
              ((vh_list_inset_t){30, 200, 0, 10}),
              songlist_item_for_index,
              songlist_item_recycle,
              NULL);
  vh_list_set_header(sl.view, header);
}

#endif
