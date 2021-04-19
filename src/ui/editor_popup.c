#ifndef editor_popup_h
#define editor_popup_h

#include "mtmap.c"
#include "view.c"

void   editor_popup_attach(view_t* view, char* fontpath);
void   editor_popup_set_songs(vec_t* vec, char* libpath);
map_t* editor_popup_get_changed();
vec_t* editor_popup_get_removed();
char*  editor_popup_get_cover();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "editor.c"
#include "mtcstring.c"
#include "mtvector.c"
#include "text.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "vh_list.c"
#include "vh_list_item.c"
#include "vh_textinput.c"
#include <string.h>

struct _editor_popup_t
{
  view_t* listview;
  view_t* headview;
  view_t* coverview;

  map_t* data;
  map_t* temp;

  vec_t* fields;
  vec_t* items;

  map_t* changed;
  vec_t* removed;
  char*  cover;

  view_t*     sel_item;
  textstyle_t textstyle;

} ep = {0};

void editor_popup_input_cell_value_changed(view_t* inputview)
{
  vh_textinput_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(vh_textinput_get_text(inputview));

  printf("editor_popup_input_cell_value_changed key %s text %s\n", key, text);

  MPUT(ep.temp, key, text);
}

void editor_popup_input_cell_edit_finished(view_t* inputview)
{
  vh_textinput_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(vh_textinput_get_text(inputview));

  MPUT(ep.changed, key, text);

  printf("editor_popup_input_cell_edit_finished key %s text %s\n", key, text);

  // remove text view handler

  view_t* textcell = view_new(cstr_fromformat("%s%s", ep.sel_item->id, "val", NULL), inputview->frame.local);
  tg_text_add(textcell);
  tg_text_set(textcell, text, ep.textstyle);
  textcell->blocks_touch = 0;

  // TODO check if previous cell gets released
  vh_litem_rpl_cell(ep.sel_item, key, textcell);

  printf("text cell created id %s\n", textcell->id);
  printf("replacing intpu cell with text cell\n");

  // unlock scrolling
  vh_list_lock_scroll(ep.listview, 0);
}

void editor_popup_select_item(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  printf("editor_popup_select_item %s index %i cell id %s\n", itemview->id, index, cell->id);

  char* key   = ep.fields->data[index];
  char* value = MGET(ep.data, key);

  printf("key for item %s value %s\n", key, value);

  // only value cells are editable
  if (strcmp(cell->id, "val") == 0)
  {
    uint32_t color1 = 0x00FF00AA;

    ep.textstyle.backcolor = color1;
    ep.sel_item            = itemview;

    view_t* inputcell = view_new(cstr_fromformat("%s%s%s", itemview->id, key, "edit", NULL), cell->view->frame.local);
    vh_textinput_add(inputcell, "", value, ep.textstyle, key);                        // add text handler with key as userdata
    vh_textinput_set_on_text(inputcell, editor_popup_input_cell_value_changed);       // listen for text change
    vh_textinput_set_on_deactivate(inputcell, editor_popup_input_cell_edit_finished); // listen for text editing finish
    vh_textinput_activate(inputcell, 1);                                              // activate text input
    ui_manager_activate(inputcell);                                                   // set text input as event receiver
    vh_list_lock_scroll(ep.listview, 1);                                              // lock scrolling of list to avoid going out screen
    vh_litem_rpl_cell(itemview, cell->id, inputcell);                                 // replacing simple text cell with input cell

    // TODO check if previous cell gets released

    printf("input cell created id %s\n", inputcell->id);
    printf("replacing text cell with input cell %s\n", cell->id);

    /* ep.textstyle.backcolor = 0x00FF00AA; */
    /* tg_text_set(vh_litem_get_cell(itemview, "key"), key, ep.textstyle); */
    /* tg_text_set(vh_litem_get_cell(itemview, "val"), value, ep.textstyle); */
    /* tg_text_set(vh_litem_get_cell(itemview, "del"), "Remove", ep.textstyle); */
  }

  if (strcmp(cell->id, "del") == 0)
  {
    // indicate row for deletio
    ep.textstyle.backcolor = 0xFF0000AA;
    tg_text_set(vh_litem_get_cell(itemview, "key"), key, ep.textstyle);
    tg_text_set(vh_litem_get_cell(itemview, "val"), value, ep.textstyle);
    tg_text_set(vh_litem_get_cell(itemview, "del"), "Remove", ep.textstyle);

    VADD(ep.removed, key);
  }
}

view_t* editor_popup_create_item()
{
  static int itemcnt;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "editor_popup_item%i", itemcnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, editor_popup_select_item);

  // first cell is a simple text cell
  view_t* keycell = view_new(cstr_fromformat("%s%s", rowview->id, "key", NULL), (r2_t){0, 0, 100, 35});
  tg_text_add(keycell);

  view_t* valcell = view_new(cstr_fromformat("%s%s", rowview->id, "val", NULL), (r2_t){0, 0, 300, 35});
  tg_text_add(valcell);

  view_t* delcell = view_new(cstr_fromformat("%s%s", rowview->id, "del", NULL), (r2_t){0, 0, 110, 35});
  tg_text_add(delcell);

  vh_litem_add_cell(rowview, "key", 200, keycell);
  vh_litem_add_cell(rowview, "val", 200, valcell);
  vh_litem_add_cell(rowview, "del", 200, delcell);

  return rowview;
}

view_t* editor_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= ep.items->length)
    return NULL; // no more items

  *item_count = ep.items->length;

  return ep.items->data[index];
}

int editor_popup_comp_text(void* left, void* right)
{
  char* la = left;
  char* ra = right;

  return strcmp(la, ra);
}

void editor_popup_remove_str(vec_t* vec, char* str)
{
  for (int index = 0; index < vec->length; index++)
  {
    char* val = (char*)vec->data[index];
    if (strcmp(val, str) == 0)
    {
      vec_rematindex(vec, index);
      return;
    }
  }
}

void editor_popup_set_song()
{
  // reset temporary fields containers
  map_reset(ep.temp);
  vec_reset(ep.fields);
  vec_reset(ep.items);

  // store song and extract fields
  map_keys(ep.data, ep.fields);

  // sort fields
  vec_sort(ep.fields, VSD_DSC, editor_popup_comp_text);

  editor_popup_remove_str(ep.fields, "meta/artist");
  editor_popup_remove_str(ep.fields, "meta/album");
  editor_popup_remove_str(ep.fields, "meta/title");

  vec_ins(ep.fields, cstr_fromcstring("meta/title"), 0);
  vec_ins(ep.fields, cstr_fromcstring("meta/album"), 0);
  vec_ins(ep.fields, cstr_fromcstring("meta/artist"), 0);

  // reset list handler
  vh_list_reset(ep.listview);

  // create items

  for (int index = 0; index < ep.fields->length; index++)
  {
    view_t* item = editor_popup_create_item();

    char* key   = ep.fields->data[index];
    char* value = MGET(ep.data, key);

    uint32_t color1 = (index % 2 == 0) ? 0xFEFEFEFF : 0xEFEFEFFF;

    // "file" fields are non-editable
    if (key[0] == 'f') color1 &= 0xFFDDDDFF;

    ep.textstyle.backcolor = color1;

    vh_litem_upd_index(item, index);
    tg_text_set(vh_litem_get_cell(item, "key"), key + 5, ep.textstyle); // show last key name component

    uint32_t color2 = (index % 2 == 0) ? 0xF8F8F8FF : 0xE8E8E8FF;

    // "file" fields are non-editable
    if (key[0] == 'f') color2 &= 0xFFDDDDFF;

    ep.textstyle.backcolor = color2;

    tg_text_set(vh_litem_get_cell(item, "val"), value, ep.textstyle);

    if (key[0] != 'f') tg_text_set(vh_litem_get_cell(item, "del"), "Remove", ep.textstyle);

    VADD(ep.items, item);
  }
}

void editor_popup_attach(view_t* view, char* fontpath)
{
  view_t* listview  = view_get_subview(view, "editorlist");
  view_t* headview  = view_get_subview(view, "ideditorheader");
  view_t* coverview = view_get_subview(view, "coverview");

  vh_list_add(listview,
              ((vh_list_inset_t){0, 10, 0, 10}),
              editor_popup_item_for_index, NULL, NULL);

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  ep.headview  = headview;
  ep.listview  = listview;
  ep.coverview = coverview;
  ep.temp      = MNEW();
  ep.fields    = VNEW();
  ep.textstyle = ts;
  ep.items     = VNEW();
  ep.data      = MNEW();
  ep.changed   = MNEW();
  ep.removed   = VNEW();
  ep.cover     = NULL;

  ts.backcolor = 0;

  view_t* uploadbtn = view_get_subview(view, "uploadbtn");
  tg_text_add(uploadbtn);
  tg_text_set(uploadbtn, "add new image", ts);

  view_t* newfieldbtn = view_get_subview(view, "newfieldbtn");
  tg_text_add(newfieldbtn);
  tg_text_set(newfieldbtn, "add new field", ts);

  ts.align = TA_CENTER;
  tg_text_add(headview);
  tg_text_set(headview, "Editing 1 data", ts);
}

map_t* editor_popup_get_changed()
{
  return ep.changed;
}

vec_t* editor_popup_get_removed()
{
  return ep.removed;
}

char* editor_popup_get_cover()
{
  return ep.cover;
}

void editor_popup_set_songs(vec_t* vec, char* libpath)
{
  if (vec->length > 0)
  {
    map_reset(ep.data);
    map_reset(ep.changed);
    vec_reset(ep.removed);

    vec_t* fields = VNEW();
    vec_t* values = VNEW();

    for (int index = 0; index < vec->length; index++)
    {
      map_t* song = vec->data[index];
      vec_reset(fields);
      vec_reset(values);
      map_keys(song, fields);

      for (int fi = 0; fi < fields->length; fi++)
      {
        char* field = fields->data[fi];
        char* value = MGET(song, field);
        char* curr  = MGET(ep.data, field);

        if (curr == NULL)
        {
          MPUT(ep.data, field, value);
        }
        else
        {
          if (strcmp(curr, value) != 0) MPUT(ep.data, field, cstr_fromcstring("MULTIPLE"));
        }
      }
    }
    editor_popup_set_song();

    REL(fields);
    REL(values);
  }

  char text[100];
  snprintf(text, 100, "Editing %i song(s)", vec->length);
  tg_text_add(ep.headview);
  tg_text_set(ep.headview, text, ep.textstyle);

  // load cover

  if (ep.coverview->texture.bitmap)
  {

    map_t* song = vec->data[0];
    char*  path = MGET(song, "file/path");
    char*  file = cstr_fromformat("%s%s", libpath, path, NULL);

    editor_get_album(file, ep.coverview->texture.bitmap);
    ep.coverview->texture.changed = 1;
  }
}

#endif
