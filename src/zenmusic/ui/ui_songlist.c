#ifndef ui_songlist_h
#define ui_songlist_h

#include "view.c"
#include "zc_map.c"

void ui_songlist_select(int index);
void ui_songlist_select_all();
void ui_songlist_select_range(int index);
void ui_songlist_get_selected(vec_t* vec);

void ui_songlist_attach(view_t* base);
void ui_songlist_update();
void ui_songlist_refresh();
void ui_songlist_toggle_pause(int state);
void ui_songlist_select_and_show(int index);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "selection.c"
#include "tg_css.c"
#include "tg_text.c"
#include "ui_play_controls.c"
#include "ui_popup_switcher.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"
#include "visible.c"

void on_header_field_select(view_t* view, char* id, ev_t ev);
void on_header_field_insert(view_t* view, int src, int tgt);
void on_header_field_resize(view_t* view, char* id, int size);

view_t* ui_songlist_item_for_index(int index, void* userdata, view_t* listview, int* item_count);
void    ui_songlist_item_recycle(view_t* item);

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

struct ui_songlist_t
{
  view_t*     view;    // table view
  vec_t*      cache;   // item cache
  vec_t*      columns; // fileds in table
  textstyle_t textstyle;

  uint32_t color_s; // color selected
  int32_t  index_s; // index selected

  void (*on_select)(int index);
} sl = {0};

void ui_songlist_attach(view_t* base)
{
  assert(base != NULL);

  sl.view    = view_get_subview(base, "songlist");
  sl.cache   = VNEW();
  sl.columns = VNEW();

  sl.color_s = 0x55FF55FF;

  sl.textstyle.font        = config_get("font_path");
  sl.textstyle.margin_left = 10;
  sl.textstyle.size        = 30.0;
  sl.textstyle.textcolor   = 0x000000FF;
  sl.textstyle.backcolor   = 0xF5F5F5FF;

  // create columns

  VADD(sl.columns, sl_cell_new("index", 50, 0));
  VADD(sl.columns, sl_cell_new("meta/artist", 300, 1));
  VADD(sl.columns, sl_cell_new("meta/album", 200, 2));
  VADD(sl.columns, sl_cell_new("meta/title", 300, 3));
  VADD(sl.columns, sl_cell_new("meta/date", 150, 4));
  VADD(sl.columns, sl_cell_new("meta/genre", 150, 5));
  VADD(sl.columns, sl_cell_new("meta/track", 150, 6));
  VADD(sl.columns, sl_cell_new("meta/disc", 150, 7));
  VADD(sl.columns, sl_cell_new("file/duration", 100, 8));
  VADD(sl.columns, sl_cell_new("file/channels", 100, 9));
  VADD(sl.columns, sl_cell_new("file/bit_rate", 100, 10));
  VADD(sl.columns, sl_cell_new("file/sample_rate", 100, 11));
  VADD(sl.columns, sl_cell_new("file/play_count", 150, 12));
  VADD(sl.columns, sl_cell_new("file/skip_count", 150, 13));
  VADD(sl.columns, sl_cell_new("file/added", 150, 14));
  VADD(sl.columns, sl_cell_new("file/last_played", 150, 15));
  VADD(sl.columns, sl_cell_new("file/last_skipped", 150, 16));
  VADD(sl.columns, sl_cell_new("file/media_type", 150, 17));
  VADD(sl.columns, sl_cell_new("file/container", 150, 18));

  vec_dec_retcount(sl.columns);

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
  while ((cell = VNXT(sl.columns)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", header->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 30});
    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, sl.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);

    REL(id);
  }

  // add list handler to view

  vh_list_add(sl.view,
              ((vh_list_inset_t){30, 200, 0, 10}),
              ui_songlist_item_for_index,
              ui_songlist_item_recycle,
              NULL);
  vh_list_set_header(sl.view, header);

  // select first song by default

  selection_add(0);
}

void ui_songlist_update()
{
  vh_list_reset(sl.view);
}

void ui_songlist_refresh()
{
  vh_list_refresh(sl.view);
}

void ui_songlist_toggle_pause(int state)
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
  visible_set_sortfield(id, 1);
  ui_songlist_refresh();
}

void on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in columns so new items will use updated order
  sl_cell_t* cell = sl.columns->data[src];

  RET(cell);
  VREM(sl.columns, cell);
  vec_ins(sl.columns, cell, tgt);
  REL(cell);

  // update all items and cache
  vec_t* items = vh_list_items(sl.view);
  for (int index = 0; index < items->length; index++)
  {
    view_t* item = items->data[index];
    vh_litem_swp_cell(item, src, tgt);
  }

  items = sl.cache;
  for (int index = 0; index < items->length; index++)
  {
    view_t* item = items->data[index];
    vh_litem_swp_cell(item, src, tgt);
  }
}

void on_header_field_resize(view_t* view, char* id, int size)
{
  // update in columns so new items will use updated size
  for (int i = 0; i < sl.columns->length; i++)
  {
    sl_cell_t* cell = sl.columns->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  vec_t* items = vh_list_items(sl.view);

  for (int index = 0; index < items->length; index++)
  {
    view_t* item = items->data[index];
    vh_litem_upd_cell_size(item, id, size);
  }

  items = sl.cache;
  for (int index = 0; index < items->length; index++)
  {
    view_t* item = items->data[index];
    vh_litem_upd_cell_size(item, id, size);
  }
}

void ui_songlist_select(int index)
{
  selection_res();
  selection_add(sl.index_s);
  vh_list_refresh(sl.view);
}

void ui_songlist_select_range(int index)
{
  selection_rng(sl.index_s);
  vh_list_refresh(sl.view);
}

void ui_songlist_select_all()
{
  selection_res();
  selection_add(0);
  selection_rng(visible_song_count());
  vh_list_refresh(sl.view);
}

void ui_songlist_get_selected(vec_t* vec)
{
  // if selection is empty, select current index
  if (selection_cnt() < 2) selection_add(sl.index_s);
  selection_add_selection(visible_get_songs(), vec);
}

void ui_songlist_select_and_show(int index)
{
  selection_res();
  selection_add(index);
  vh_list_scroll_to_index(sl.view, index);
}

// items

void ui_songlist_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  sl.index_s = index;
  if (ev.button == 1)
  {
    if (!ev.ctrl_down && !ev.shift_down) selection_res();

    if (ev.shift_down)
    {
      selection_rng(index);
    }
    else
    {
      selection_add(index);
    }

    printf("%i %i\n", index, ev.dclick);
    if (ev.dclick)
    {
      ui_play_index(index);
    }

    vh_list_refresh(sl.view);
  }
  else if (ev.button == 3)
  {

    ui_popup_switcher_toggle("song_popup_page");

    if (selection_cnt() < 2)
    {
      selection_res();
      selection_add(index);
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
  vh_litem_set_on_select(rowview, ui_songlist_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(sl.columns)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", rowview->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 35});
    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
    REL(id);
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
  while ((cell = VNXT(sl.columns)))
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

void ui_songlist_item_recycle(view_t* item)
{
  VADD(sl.cache, item);
}

view_t* ui_songlist_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0) return NULL;                     // no items before 0
  if (index >= visible_song_count()) return NULL; // no more items

  *item_count = visible_song_count();

  view_t* item;
  if (sl.cache->length > 0)
    item = sl.cache->data[0];
  else
    item = songitem_create();

  VREM(sl.cache, item);

  if (selection_has(index))
  {
    songitem_update_row(item, index, visible_song_at_index(index), sl.color_s);
  }
  else
  {
    uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
    uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

    songitem_update_row(item, index, visible_song_at_index(index), color1);
  }

  return item;
}

#endif
