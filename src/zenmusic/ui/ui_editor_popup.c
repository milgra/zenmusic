#ifndef ui_editor_popup_h
#define ui_editor_popup_h

#include "mtmap.c"
#include "view.c"

void ui_editor_popup_attach(view_t* view);
void ui_editor_popup_set_songs(vec_t* vec);
void ui_editor_popup_show();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "editor.c"
#include "mtcallback.c"
#include "mtcstring.c"
#include "mtvector.c"
#include "text.c"
#include "tg_text.c"
#include "ui_alert_popup.c"
#include "ui_decision_popup.c"
#include "ui_manager.c"
#include "ui_popup_switcher.c"
#include "ui_songlist.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"
#include "vh_textinput.c"
#include <string.h>

void    ui_editor_popup_create_table();
void    ui_editor_popup_on_accept();
view_t* ui_editor_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count);
void    ui_editor_popup_on_button_down(void* userdata, void* data);
void    ui_editor_popup_on_header_field_select(view_t* view, char* id, ev_t ev);
void    ui_editor_popup_on_header_field_insert(view_t* view, int src, int tgt);
void    ui_editor_popup_on_header_field_resize(view_t* view, char* id, int size);

struct _ui_editor_popup_t
{
  int song_count;

  view_t* listview;
  view_t* headview;
  view_t* coverview;

  map_t* data;
  map_t* temp;

  vec_t* fields;
  vec_t* items;
  vec_t* cols;

  map_t* changed;
  vec_t* removed;
  char*  cover;

  view_t*     sel_item;
  textstyle_t textstyle;

} ep = {0};

// TODO unify these

typedef struct _se_cell_t
{
  char* id;
  int   size;
  int   index;
} se_cell_t;

se_cell_t* uise_cell_new(char* id, int size, int index)
{
  se_cell_t* cell = mem_calloc(sizeof(se_cell_t), "se_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void ui_editor_popup_attach(view_t* view)
{
  view_t* headview    = view_get_subview(view, "song_editor_header");
  view_t* listview    = view_get_subview(view, "editorlist");
  view_t* coverview   = view_get_subview(view, "coverview");
  view_t* acceptbtn   = view_get_subview(view, "editor_popup_accept_btn");
  view_t* rejectbtn   = view_get_subview(view, "editor_popup_reject_btn");
  view_t* uploadbtn   = view_get_subview(view, "uploadbtn");
  view_t* newfieldbtn = view_get_subview(view, "newfieldbtn");
  cb_t*   but_cb      = cb_new(ui_editor_popup_on_button_down, NULL);

  textstyle_t ts = {0};
  ts.font        = config_get("font_path");
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

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
  ep.cols      = VNEW();

  ui_editor_popup_create_table();

  tg_text_add(uploadbtn);
  tg_text_set(uploadbtn, "add new image", ts);

  tg_text_add(newfieldbtn);
  tg_text_set(newfieldbtn, "add new field", ts);

  vh_button_add(acceptbtn, VH_BUTTON_NORMAL, but_cb);
  vh_button_add(rejectbtn, VH_BUTTON_NORMAL, but_cb);

  tg_text_add(headview);
  tg_text_set(headview, "Editing 1 data", ts);
}

void ui_editor_popup_create_table()
{
  VADDR(ep.cols, uise_cell_new("key", 100, 0));
  VADDR(ep.cols, uise_cell_new("value", 200, 1));
  VADDR(ep.cols, uise_cell_new("delete", 80, 2));

  // create header

  view_t* header = view_new("songeditorlist_header", (r2_t){0, 0, 10, 10});

  vh_lhead_add(header);
  vh_lhead_set_on_select(header, ui_editor_popup_on_header_field_select);
  vh_lhead_set_on_insert(header, ui_editor_popup_on_header_field_insert);
  vh_lhead_set_on_resize(header, ui_editor_popup_on_header_field_resize);

  ep.textstyle.align     = TA_LEFT;
  ep.textstyle.backcolor = 0xFFFFFFFF;

  se_cell_t* cell;
  while ((cell = VNXT(ep.cols)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", header->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 30});
    REL(id);

    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, ep.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);
  }

  vh_list_add(ep.listview, ((vh_list_inset_t){30, 10, 0, 10}), ui_editor_popup_item_for_index, NULL, NULL);
  vh_list_set_header(ep.listview, header);
}

// header related

void ui_editor_popup_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*uisp.on_header_select)(id);
}

void ui_editor_popup_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  se_cell_t* cell = ep.cols->data[src];

  RET(cell);
  VREM(ep.cols, cell);
  vec_ins(ep.cols, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(ep.listview);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void ui_editor_popup_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < ep.cols->length; i++)
  {
    se_cell_t* cell = ep.cols->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(ep.listview);
  while ((item = VNXT(items))) vh_litem_upd_cell_size(item, id, size);
}

// item related

view_t* ui_editor_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0) return NULL;                 // no items before 0
  if (index >= ep.items->length) return NULL; // no more items

  *item_count = ep.items->length;

  return ep.items->data[index];
}

// cell related

void ui_editor_popup_input_cell_value_changed(view_t* inputview)
{
  vh_textinput_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(vh_textinput_get_text(inputview));

  printf("ui_editor_popup_input_cell_value_changed key %s text %s\n", key, text);

  MPUT(ep.temp, key, text);
}

void ui_editor_popup_input_cell_edit_finished(view_t* inputview)
{
  vh_textinput_t* data = inputview->handler_data;

  char* key  = data->userdata;
  char* text = str_cstring(vh_textinput_get_text(inputview));

  MPUT(ep.changed, key, text);

  printf("ui_editor_popup_input_cell_edit_finished key %s text %s\n", key, text);

  // remove text view handler
  char* id = cstr_fromformat(100, "%s%s", ep.sel_item->id, "val");

  view_t* textcell = view_new(id, inputview->frame.local);
  REL(id);

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

void ui_editor_popup_select_item(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  printf("ui_editor_popup_select_item %s index %i cell id %s\n", itemview->id, index, cell->id);

  char* key   = ep.fields->data[index];
  char* value = MGET(ep.data, key);

  printf("key for item %s value %s\n", key, value);

  // only value cells are editable
  if (strcmp(cell->id, "val") == 0)
  {
    uint32_t color1 = 0x00FF00AA;

    ep.textstyle.backcolor = color1;
    ep.sel_item            = itemview;

    char* id = cstr_fromformat(100, "%s%s%s", itemview->id, key, "edit");

    view_t* inputcell = view_new(id, cell->view->frame.local);
    REL(id);

    vh_textinput_add(inputcell, "", value, ep.textstyle, key);                           // add text handler with key as userdata
    vh_textinput_set_on_text(inputcell, ui_editor_popup_input_cell_value_changed);       // listen for text change
    vh_textinput_set_on_deactivate(inputcell, ui_editor_popup_input_cell_edit_finished); // listen for text editing finish
    vh_textinput_activate(inputcell, 1);                                                 // activate text input
    ui_manager_activate(inputcell);                                                      // set text input as event receiver
    vh_list_lock_scroll(ep.listview, 1);                                                 // lock scrolling of list to avoid going out screen
    vh_litem_rpl_cell(itemview, cell->id, inputcell);                                    // replacing simple text cell with input cell

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
    tg_text_set(vh_litem_get_cell(itemview, "value"), value, ep.textstyle);
    tg_text_set(vh_litem_get_cell(itemview, "delete"), "Remove", ep.textstyle);

    VADD(ep.removed, key);
  }
}

view_t* ui_editor_popup_create_item()
{
  static int itemcnt;

  char*   id      = cstr_fromformat(100, "editor_popup_item%i", itemcnt++);
  view_t* rowview = view_new(id, (r2_t){0, 0, 0, 35});
  REL(id);

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, ui_editor_popup_select_item);

  se_cell_t* cell;
  while ((cell = VNXT(ep.cols)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", rowview->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 30});
    REL(id);

    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, ep.textstyle);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

int ui_editor_popup_comp_text(void* left, void* right)
{
  char* la = left;
  char* ra = right;

  return strcmp(la, ra);
}

void ui_editor_popup_remove_str(vec_t* vec, char* str)
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

void ui_editor_popup_set_song()
{
  // reset temporary fields containers
  map_reset(ep.temp);
  vec_reset(ep.fields);
  vec_reset(ep.items);

  // store song and extract fields
  map_keys(ep.data, ep.fields);

  // sort fields
  vec_sort(ep.fields, VSD_DSC, ui_editor_popup_comp_text);

  ui_editor_popup_remove_str(ep.fields, "meta/artist");
  ui_editor_popup_remove_str(ep.fields, "meta/album");
  ui_editor_popup_remove_str(ep.fields, "meta/title");

  vec_ins(ep.fields, cstr_fromcstring("meta/title"), 0);
  vec_ins(ep.fields, cstr_fromcstring("meta/album"), 0);
  vec_ins(ep.fields, cstr_fromcstring("meta/artist"), 0);

  // reset list handler
  vh_list_reset(ep.listview);

  // create items

  for (int index = 0; index < ep.fields->length; index++)
  {
    view_t* item = ui_editor_popup_create_item();

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

    tg_text_set(vh_litem_get_cell(item, "value"), value, ep.textstyle);

    if (key[0] != 'f') tg_text_set(vh_litem_get_cell(item, "delete"), "Delete", ep.textstyle);

    VADD(ep.items, item);
  }
}

void ui_editor_popup_on_button_down(void* userdata, void* data)
{
  view_t* view = data;
  if (strcmp(view->id, "editor_popup_accept_btn") == 0)
  {
    char* message = cstr_fromformat(150, "%i fields will be changed %i fields will be removed cover will be %s on %i items, are you sure you want these modifications?", ep.changed->count, ep.removed->length, ep.cover, ep.song_count);
    cb_t* acc_cb  = cb_new(ui_editor_popup_on_accept, NULL);
    ui_decision_popup_show(message, acc_cb, NULL);
    REL(message);
  }
  if (strcmp(view->id, "editor_popup_reject_btn") == 0)
  {
    ui_popup_switcher_toggle("song_editor_popup_page");
  }
}

void ui_editor_popup_set_songs(vec_t* vec)
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
    ui_editor_popup_set_song();

    REL(fields);
    REL(values);
  }

  char text[100];
  snprintf(text, 100, "Editing %i song(s)", vec->length);

  ep.song_count = vec->length;

  // tg_text_add(ep.headview);

  ep.textstyle.backcolor = 0;
  tg_text_set(ep.headview, text, ep.textstyle);

  // load cover

  if (ep.coverview->texture.bitmap)
  {

    map_t* song = vec->data[0];
    char*  path = MGET(song, "file/path");
    char*  file = cstr_fromformat(100, "%s%s", config_get("lib_path"), path);

    editor_get_album(file, ep.coverview->texture.bitmap);

    REL(file);
    ep.coverview->texture.changed = 1;
  }
}

void ui_editor_popup_on_accept()
{
  ui_popup_switcher_toggle("song_editor_popup_page");

  /* char* libpath = config_get("lib_path"); */

  /* editor_update_metadata(libpath, ui.selected, changed, removed, cover); */

  // update metadata in media files

  // update metadata in database

  // organize library ( if path has to change )

  /* map_t* old_data = ui_editor_popup_get_old_data(); */
  /* map_t* new_data = ui_editor_popup_get_new_data(); */

  /* // update modified entity in database */
  /* vec_t* keys = VNEW(); */
  /* map_keys(new_data, keys); */
  /* for (int index = 0; index < keys->length; index++) */
  /* { */
  /*   char* key    = keys->data[index]; */
  /*   char* oldval = MGET(old_data, key); */
  /*   char* newval = MGET(new_data, key); */

  /*   if (strcmp(oldval, newval) != 0) */
  /*   { */
  /*     printf("%s changed from %s to %s, writing to db\n", key, oldval, newval); */
  /*     MPUT(old_data, key, newval); */
  /*   } */
  /* } */

  /* // notify main namespace to organize and save metadata and database */
  /* callbacks_call("on_save_entry", old_data); */

  /* // reload song list */
  /* ui_songlist_refresh(); */
}

void ui_editor_popup_show()
{
  vec_t* selected = VNEW();
  ui_songlist_get_selected(selected);
  ui_editor_popup_set_songs(selected);
  REL(selected);
  ui_popup_switcher_toggle("song_editor_popup_page");
}

#endif
