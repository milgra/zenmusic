#ifndef ui_settings_popup_h
#define ui_settings_popup_h

#include "mtmap.c"
#include "view.c"

void ui_settings_popup_attach(view_t* view);
void ui_settings_popup_update();
void ui_settings_popup_refresh();
void ui_settings_popup_show();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "config.c"
#include "mtcallback.c"
#include "selection.c"
#include "tg_css.c"
#include "tg_text.c"
#include "ui_alert_popup.c"
#include "ui_decision_popup.c"
#include "ui_lib_change_popup.c"
#include "ui_popup_switcher.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"

void ui_settings_popup_on_header_field_select(view_t* view, char* id, ev_t ev);
void ui_settings_popup_on_header_field_insert(view_t* view, int src, int tgt);
void ui_settings_popup_on_header_field_resize(view_t* view, char* id, int size);

view_t* ui_settings_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count);
view_t* settingsitem_create();
void    settingsitem_update_row(view_t* rowview, int index, char* field, char* value);

struct ui_settings_popup_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;
} uisp = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* uisp_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void ui_settings_popup_attach(view_t* baseview)
{
  uisp.view   = view_get_subview(baseview, "settingslist");
  uisp.fields = VNEW();
  uisp.items  = VNEW();

  uisp.textstyle.font        = config_get("font_path");
  uisp.textstyle.align       = 0;
  uisp.textstyle.margin_left = 10;
  uisp.textstyle.size        = 30.0;
  uisp.textstyle.textcolor   = 0x000000FF;
  uisp.textstyle.backcolor   = 0xF5F5F5FF;

  // create fields

  VADDR(uisp.fields, uisp_cell_new("key", 200, 0));
  VADDR(uisp.fields, uisp_cell_new("value", 460, 1));

  // add header handler

  view_t* header = view_new("settingslist_header", (r2_t){0, 0, 10, 30});
  /* header->layout.background_color = 0x333333FF; */
  /* header->layout.shadow_blur      = 3; */
  /* header->layout.border_radius    = 3; */
  tg_css_add(header);

  vh_lhead_add(header);
  vh_lhead_set_on_select(header, ui_settings_popup_on_header_field_select);
  vh_lhead_set_on_insert(header, ui_settings_popup_on_header_field_insert);
  vh_lhead_set_on_resize(header, ui_settings_popup_on_header_field_resize);

  sl_cell_t* cell;
  while ((cell = VNXT(uisp.fields)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", header->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 30});
    REL(id);

    tg_text_add(cellview);
    tg_text_set(cellview, cell->id, uisp.textstyle);

    vh_lhead_add_cell(header, cell->id, cell->size, cellview);
  }

  // add list handler to view

  vh_list_add(uisp.view,
              ((vh_list_inset_t){30, 0, 0, 0}),
              ui_settings_popup_item_for_index, NULL, NULL);
  vh_list_set_header(uisp.view, header);

  // create items

  VADD(uisp.items, settingsitem_create());
  VADD(uisp.items, settingsitem_create());
  // VADD(uisp.items, settingsitem_create());
  VADD(uisp.items, settingsitem_create());
  VADD(uisp.items, settingsitem_create());
  VADD(uisp.items, settingsitem_create());

  settingsitem_update_row(uisp.items->data[0], 0, "Library Path", "/home/user/milgra/Music");
  settingsitem_update_row(uisp.items->data[1], 1, "Organize Library", "Disabled");
  //  settingsitem_update_row(uisp.items->data[2], 2, "Dark Mode", "Disabled");
  settingsitem_update_row(uisp.items->data[2], 2, "Remote Control", "Disabled");
  settingsitem_update_row(uisp.items->data[3], 3, "Config Path", "/home/.config/zenmusic/config");
  settingsitem_update_row(uisp.items->data[4], 4, "Style Path", "/usr/local/share/zenmusic");
}

void ui_settings_popup_show()
{
  settingsitem_update_row(uisp.items->data[0], 0, "Library Path", config_get("lib_path"));
  settingsitem_update_row(uisp.items->data[1], 1, "Organize Library", config_get("organize_lib"));
  //  settingsitem_update_row(uisp.items->data[2], 2, "Dark Mode", config_get("dark_mode"));
  settingsitem_update_row(uisp.items->data[2], 2, "Remote Control", config_get("remote_enabled"));
  settingsitem_update_row(uisp.items->data[3], 3, "Config Path", config_get("cfg_path"));
  settingsitem_update_row(uisp.items->data[4], 4, "HTML/Style Path", config_get("res_path"));

  ui_popup_switcher_toggle("settings_popup_page");
}

void ui_settings_popup_update()
{
  vh_list_reset(uisp.view);
}

void ui_settings_popup_refresh()
{
  vh_list_refresh(uisp.view);
}

void ui_settings_popup_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*uisp.on_header_select)(id);
}

void ui_settings_popup_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = uisp.fields->data[src];

  RET(cell);
  VREM(uisp.fields, cell);
  vec_ins(uisp.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(uisp.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void ui_settings_popup_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < uisp.fields->length; i++)
  {
    sl_cell_t* cell = uisp.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(uisp.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

void ui_settings_popup_on_accept(void* userdata, void* data)
{
  int enabled = config_get_bool("organize_lib");
  if (enabled)
    config_set_bool("organize_lib", 0);
  else
    config_set_bool("organize_lib", 1);
  config_write(config_get("cfg_path"));
  settingsitem_update_row(uisp.items->data[1], 1, "Organize Library", config_get("organize_lib"));
  callbacks_call("on_change_organize", NULL);
}

void ui_settings_popup_on_accept_remote(void* userdata, void* data)
{
  int enabled = config_get_bool("remote_enabled");
  if (enabled)
    config_set_bool("remote_enabled", 0);
  else
    config_set_bool("remote_enabled", 1);
  config_write(config_get("cfg_path"));
  settingsitem_update_row(uisp.items->data[2], 1, "Organize Library", config_get("remote_enabled"));
  callbacks_call("on_change_remote", NULL);
}

// items

void ui_settings_popup_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  printf("item select %i\n", index);

  // ui_popup_switcher_toggle("decision_popup_page");

  switch (index)
  {
  case 0:
    ui_lib_change_popup_show();
    break;
  case 1:
  {
    int   enabled = config_get_bool("organize_lib");
    cb_t* acc_cb  = cb_new(ui_settings_popup_on_accept, NULL);
    if (enabled)
      ui_decision_popup_show("Are you sure you want to switch off automatic organization?", acc_cb, NULL);
    else
      ui_decision_popup_show("Music files will be renamed/reorganized automatically under your library folder, based on artist, title and track number. Are you sure?", acc_cb, NULL);
    REL(acc_cb);
    break;
  }
  case 2:
  {
    int   enabled = config_get_bool("remote_enabled");
    cb_t* acc_cb  = cb_new(ui_settings_popup_on_accept_remote, NULL);
    if (enabled)
      ui_decision_popup_show("Are you sure you want to switch off remote control?", acc_cb, NULL);
    else
    {
      char* message = cstr_fromformat(200, "You can remote control Zen Music by sending 0x00(play/pause) 0x01(prev song) 0x02(next song) to UDP port %s. "
                                           "Would you like to enable it?",
                                      config_get("remote_port"));
      ui_decision_popup_show(message, acc_cb, NULL);
    }
    REL(acc_cb);
    break;
  }
  case 3:
    ui_alert_popup_show("You cannot set the config path");
    break;
  case 4:
    ui_alert_popup_show("You cannot set the style path");
    break;
  }
}

view_t* settingsitem_create()
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "uispist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 640, 50});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, ui_settings_popup_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(uisp.fields)))
  {
    char*   id       = cstr_fromformat(100, "%s%s", rowview->id, cell->id);
    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, 50});
    REL(id);

    tg_text_add(cellview);

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void settingsitem_update_row(view_t* rowview, int index, char* field, char* value)
{
  uint32_t color1          = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uisp.textstyle.backcolor = color1;

  vh_litem_upd_index(rowview, index);

  tg_text_set(vh_litem_get_cell(rowview, "key"), field, uisp.textstyle);
  tg_text_set(vh_litem_get_cell(rowview, "value"), value, uisp.textstyle);
}

view_t* ui_settings_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= uisp.items->length)
    return NULL; // no more items

  *item_count = uisp.items->length;

  return uisp.items->data[index];
}

#endif
