#ifndef editor_h
#define editor_h

#include "mtmap.c"
#include "view.c"

void   editor_attach(view_t* view, char* fontpath);
void   editor_set_song(map_t* map);
map_t* editor_get_old_data();
map_t* editor_get_new_data();

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
  map_t*      song;
  map_t*      temp;
  vec_t*      fields;
  view_t*     sel_item;
  textstyle_t textstyle;
} editor = {0};

void editor_input_cell_value_changed(view_t* inputview)
{
  vh_text_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(data->text);

  printf("editor_input_cell_value_changed key %s text %s\n", key, text);

  MPUT(editor.temp, key, text);
}

void editor_input_cell_edit_finished(view_t* inputview)
{
  vh_text_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(data->text);

  printf("editor_input_cell_edit_finished key %s text %s\n", key, text);

  // remove text view handler

  view_t* textcell = view_new(cstr_fromformat("%s%s", editor.sel_item->id, "val", NULL), inputview->frame.local);
  tg_text_add(textcell);
  tg_text_set(textcell, text, editor.textstyle);
  textcell->blocks_touch = 0;

  // TODO check if previous cell gets released
  vh_litem_rpl_cell(editor.sel_item, key, textcell);

  printf("text cell created id %s\n", textcell->id);
  printf("replacing intpu cell with text cell\n");

  // unlock scrolling
  vh_list_lock_scroll(editor.view, 0);
}

void editor_select_item(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  printf("editor_select_item %s index %i cell id %s\n", itemview->id, index, cell->id);

  // only value cells are editable
  if (strcmp(cell->id, "val") == 0)
  {
    char* key   = editor.fields->data[index];
    char* value = MGET(editor.song, key);

    printf("key for item %s value %s\n", key, value);

    uint32_t color1 = (index % 2 == 0) ? 0xFEFEFEFF : 0xEFEFEFFF;

    editor.textstyle.backcolor = color1;
    editor.sel_item            = itemview;

    view_t* inputcell = view_new(cstr_fromformat("%s%s%s", itemview->id, key, "edit", NULL), cell->view->frame.local);
    vh_text_add(inputcell, "", value, editor.textstyle, key);              // add text handler with key as userdata
    vh_text_set_on_text(inputcell, editor_input_cell_value_changed);       // listen for text change
    vh_text_set_on_deactivate(inputcell, editor_input_cell_edit_finished); // listen for text editing finish
    vh_text_activate(inputcell, 1);                                        // activate text input
    ui_manager_activate(inputcell);                                        // set text input as event receiver
    vh_list_lock_scroll(editor.view, 1);                                   // lock scrolling of list to avoid going out screen
    vh_litem_rpl_cell(itemview, cell->id, inputcell);                      // replacing simple text cell with input cell

    // TODO check if previous cell gets released

    printf("input cell created id %s\n", inputcell->id);
    printf("replacing text cell with input cell %s\n", cell->id);
  }
}

view_t* editor_create_item(view_t* listview, void* userdata)
{
  static int itemcnt;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "editor_item%i", itemcnt++);

  view_t* rowview  = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->display = 0;
  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, editor_select_item);

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
  printf("editor set song\n");

  // reset temporary fields containers
  map_reset(editor.temp);
  vec_reset(editor.fields);

  // store song and extract fields
  editor.song = map;
  map_keys(map, editor.fields);

  // reset list handler
  vh_list_reset(editor.view);
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
  editor.temp      = MNEW();
  editor.fields    = VNEW();
  editor.textstyle = ts;
}

map_t* editor_get_old_data()
{
  return editor.song;
}

map_t* editor_get_new_data()
{
  return editor.temp;
}

#endif
