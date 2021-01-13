#ifndef editor_h
#define editor_h

#include "mtmap.c"
#include "view.c"

void editor_attach(view_t* view, char* fontpath);
void editor_set_song(map_t* map);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cr_text.c"
#include "text.c"
#include "vh_list.c"
#include "vh_list_item.c"

struct _editor_t
{
  view_t* view;
  char*   fontpath;
  int     ind;
  map_t*  song;
  vec_t*  fields;
} editor = {0};

void editor_select(view_t* view, void* userdata, int index, ev_t ev)
{
  printf("on_editoritem_select\n");
}

view_t* editor_create_item(view_t* listview, void* userdata)
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "editor_item%i", editor.ind++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, editor_select, NULL);
  vh_litem_add_cell(rowview, "key", 200, cr_text_add, cr_text_upd);
  vh_litem_add_cell(rowview, "value", 200, cr_text_add, cr_text_upd);

  return rowview;
}

int editor_update_item(view_t* listview, void* userdata, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= editor.fields->length)
    return 1; // no more items

  printf("editor update %i\n", index);

  *item_count = editor.fields->length;

  textstyle_t ts = {0};
  ts.font        = editor.fontpath;
  ts.margin      = 10.0;
  ts.align       = TA_LEFT;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0xFFFFFFFF;

  char* key   = editor.fields->data[index];
  char* value = MGET(editor.song, key);

  vh_litem_upd_cell(item, "key", &((cr_text_data_t){.style = ts, .text = key}));
  vh_litem_upd_cell(item, "value", &((cr_text_data_t){.style = ts, .text = value}));

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

  editor.view     = view;
  editor.fontpath = fontpath;
  editor.fields   = VNEW();
}

#endif
