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
} editor = {0};

void editor_value_changed(view_t* view, str_t* text)
{
}

void editor_select(view_t* itemview)
{
  vh_litem_t* vh = itemview->handler_data;

  if (vh->sel_cell && strcmp(vh->sel_cell->id, "val") == 0)
  {
    char* key   = editor.fields->data[vh->index];
    char* value = MGET(editor.song, key);

    printf("select %i %s\n", vh->index, key);

    view_t* valcell = view_new(cstr_fromformat("%s%s%s", itemview->id, key, "edit", NULL), vh->sel_cell->view->frame.local);

    printf("valcell id %s sel cell id %s\n", valcell->id, vh->sel_cell->view->id);

    vh_text_add(valcell, value, editor.textstyle, editor_value_changed, NULL);

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
