#ifndef editor_h
#define editor_h

#include "mtmap.c"
#include "view.c"

void editor_attach(view_t* view, char* fontpath);
void editor_set_song(map_t* map);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "text.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "vh_list.c"
#include "vh_list_item.c"
#include "vh_text.c"

struct _editor_t
{
  view_t*     view;
  textstyle_t textstyle;
  int         ind;
  map_t*      song;
  vec_t*      fields;

  view_t* sel_item;
  char*   sel_cell;
  char*   sel_key;
} editor = {0};

void editor_value_changed(view_t* view)
{
  printf("editor_value_changed\n");

  vh_text_t* data = view->handler_data;

  char* text = str_cstring(data->text);
  MPUT(editor.song, editor.sel_key, text);
}

void editor_edit_finished(view_t* view)
{
  printf("editor_value_finished\n");

  // lock scrolling
  vh_list_lock_scroll(editor.view, 0);

  // remove text view handler

  view_t* valcell = view_new(cstr_fromformat("%s%s", editor.sel_item->id, "val", NULL), view->frame.local);
  tg_text_add(valcell);

  char* value = MGET(editor.song, editor.sel_key);

  tg_text_set(valcell, value, editor.textstyle);

  valcell->blocks_touch = 0;

  vh_litem_rpl_cell(editor.sel_item, editor.sel_cell, valcell);
}

void editor_select(view_t* itemview)
{
  printf("select\n");
  vh_litem_t* vh = itemview->handler_data;

  if (vh->sel_cell && strcmp(vh->sel_cell->id, "val") == 0)
  {
    char* key   = editor.fields->data[vh->index];
    char* value = MGET(editor.song, key);

    editor.sel_item = itemview;
    editor.sel_cell = vh->sel_cell->id;
    editor.sel_key  = key;

    printf("select %i %s\n", vh->index, key);

    view_t* valcell = view_new(cstr_fromformat("%s%s%s", itemview->id, key, "edit", NULL), vh->sel_cell->view->frame.local);

    printf("valcell id %s sel cell id %s\n", valcell->id, vh->sel_cell->view->id);

    uint32_t color1            = (vh->index % 2 == 0) ? 0xFEFEFEFF : 0xEFEFEFFF;
    editor.textstyle.backcolor = color1;

    vh_text_add(valcell, value, editor.textstyle);

    vh_text_set_on_text(valcell, editor_value_changed);
    vh_text_set_on_deactivate(valcell, editor_edit_finished);

    // activate text input
    vh_text_activate(valcell, 1);

    // set text input as event receiver
    ui_manager_activate(valcell);

    // lock scrolling
    vh_list_lock_scroll(editor.view, 1);

    // swap text cell to input cell

    printf("replace cell\n");

    vh_litem_rpl_cell(itemview, vh->sel_cell->id, valcell);
  }
}

view_t* editor_create_item(view_t* listview, void* userdata)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "editor_item%i", editor.ind++);

  view_t* rowview  = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->display = 0;
  vh_litem_add(rowview, NULL, editor_select);

  // first cell is a simple text cell
  view_t* keycell = view_new(cstr_fromformat("%s%s", rowview->id, "key", NULL), (r2_t){0, 0, 200, 35});
  tg_text_add(keycell);

  view_t* valcell = view_new(cstr_fromformat("%s%s", rowview->id, "val", NULL), (r2_t){0, 0, 300, 35});
  tg_text_add(valcell);

  vh_litem_add_cell(rowview, "key", 200, keycell);
  vh_litem_add_cell(rowview, "val", 200, valcell);

  return rowview;
}

int editor_update_item(view_t* listview, void* userdata, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= editor.fields->length)
    return 1; // no more items

  *item_count = editor.fields->length;

  char* key   = editor.fields->data[index];
  char* value = MGET(editor.song, key);

  uint32_t color1            = (index % 2 == 0) ? 0xFEFEFEFF : 0xEFEFEFFF;
  editor.textstyle.backcolor = color1;

  vh_litem_upd_index(item, index);
  tg_text_set(vh_litem_get_cell(item, "key"), key, editor.textstyle);

  uint32_t color2            = (index % 2 == 0) ? 0xF8F8F8FF : 0xE8E8E8FF;
  editor.textstyle.backcolor = color2;

  tg_text_set(vh_litem_get_cell(item, "val"), value, editor.textstyle);

  return 0;
}

void editor_set_song(map_t* map)
{
  vec_reset(editor.fields);
  editor.song = map;
  map_keys(map, editor.fields);
}

void editor_attach(view_t* view, char* fontpath)
{
  vh_list_add(view, editor_create_item, editor_update_item, NULL);

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  editor.view      = view;
  editor.fields    = VNEW();
  editor.textstyle = ts;
}

#endif
