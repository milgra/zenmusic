#ifndef ui_about_popup_h
#define ui_about_popup_h

#include "mtmap.c"
#include "view.c"

void ui_about_popup_attach(view_t* base);
void ui_about_popup_update();
void ui_about_popup_refresh();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "selection.c"
#include "tg_css.c"
#include "tg_text.c"
#include "ui_alert_popup.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_list_head.c"
#include "vh_list_item.c"
#include "visible.c"
#include <limits.h>

void ui_about_popup_on_header_field_select(view_t* view, char* id, ev_t ev);
void ui_about_popup_on_header_field_insert(view_t* view, int src, int tgt);
void ui_about_popup_on_header_field_resize(view_t* view, char* id, int size);

struct ui_about_popup_t
{
  view_t*     view;   // table view
  vec_t*      fields; // fileds in table
  vec_t*      items;
  textstyle_t textstyle;
  void (*popup)(char* text);
} donl = {0};

typedef struct _sl_cell_t
{
  char* id;
  int   size;
  int   index;
} sl_cell_t;

sl_cell_t* donl_cell_new(char* id, int size, int index)
{
  sl_cell_t* cell = mem_calloc(sizeof(sl_cell_t), "sl_cell_t", NULL, NULL);

  cell->id    = cstr_fromcstring(id);
  cell->size  = size;
  cell->index = index;

  return cell;
}

void ui_about_popup_update()
{
  vh_list_reset(donl.view);
}

void ui_about_popup_refresh()
{
  vh_list_refresh(donl.view);
}

void ui_about_popup_on_header_field_select(view_t* view, char* id, ev_t ev)
{
  // (*donl.on_header_select)(id);
}

void ui_about_popup_on_header_field_insert(view_t* view, int src, int tgt)
{
  // update in fields so new items will use updated order
  sl_cell_t* cell = donl.fields->data[src];

  RET(cell);
  VREM(donl.fields, cell);
  vec_ins(donl.fields, cell, tgt);
  REL(cell);

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(donl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_swp_cell(item, src, tgt);
  }
}

void ui_about_popup_on_header_field_resize(view_t* view, char* id, int size)
{
  // update in fields so new items will use updated size
  for (int i = 0; i < donl.fields->length; i++)
  {
    sl_cell_t* cell = donl.fields->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      cell->size = size;
      break;
    }
  }

  // update all items and cache
  view_t* item;
  vec_t*  items = vh_list_items(donl.view);
  while ((item = VNXT(items)))
  {
    vh_litem_upd_cell_size(item, id, size);
  }
}

// items

void ui_about_popup_on_item_select(view_t* itemview, int index, vh_lcell_t* cell, ev_t ev)
{
  switch (index)
  {
  case 1:
    system("xdg-open https://patreon.com/milgra");
    break;
  case 2:
    system("xdg-open https://paypal.me/milgra");
    break;
  case 3:
    system("xdg-open https://github.com/milgra/zenmusic/issues");
    break;
  case 4:
    system("xdg-open https://github.com/milgra/zenmusic");
    break;
  }

  if (index < 5) ui_alert_popup_show("Link is opened in the browser.");
}

view_t* donateitem_create(int index)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "donlist_item%i", item_cnt++);

  float height = 50;
  if (index == 0) height = 170;
  if (index == 5) height = 160;

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 460, height});

  vh_litem_add(rowview, NULL);
  vh_litem_set_on_select(rowview, ui_about_popup_on_item_select);

  sl_cell_t* cell;
  while ((cell = VNXT(donl.fields)))
  {
    char* id = cstr_fromformat(100, "%s%s", rowview->id, cell->id);

    view_t* cellview = view_new(id, (r2_t){0, 0, cell->size, height});

    REL(id);

    if (index == 5)
    {
      view_t* imgview                  = view_new("bsdlogo", ((r2_t){130, 10, 200, 200}));
      char*   respath                  = config_get("res_path");
      char*   imagepath                = cstr_fromformat(100, "%s/freebsd.png", respath);
      imgview->layout.background_image = imagepath;
      tg_css_add(imgview);
      // TODO set image instead of direct set
      // REL(imagepath);
      view_add(cellview, imgview);

      vh_litem_upd_index(rowview, 5);
    }
    else
    {
      tg_text_add(cellview);
    }

    vh_litem_add_cell(rowview, cell->id, cell->size, cellview);
  }

  return rowview;
}

void donateitem_update_row(view_t* rowview, int index, char* field)
{
  uint32_t color1          = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  donl.textstyle.backcolor = color1;

  vh_litem_upd_index(rowview, index);

  tg_text_set(vh_litem_get_cell(rowview, "field"), field, donl.textstyle);
}

view_t* ui_about_popup_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index < 0)
    return NULL; // no items before 0
  if (index >= donl.items->length)
    return NULL; // no more items

  *item_count = donl.items->length;

  return donl.items->data[index];
}

void ui_about_popup_attach(view_t* baseview)
{
  donl.view   = view_get_subview(baseview, "aboutlist");
  donl.fields = VNEW();
  donl.items  = VNEW();

  donl.textstyle.font      = config_get("font_path");
  donl.textstyle.align     = TA_CENTER;
  donl.textstyle.margin    = 10;
  donl.textstyle.size      = 30.0;
  donl.textstyle.textcolor = 0x000000FF;
  donl.textstyle.backcolor = 0xF5F5F5FF;

  // create fields

  VADD(donl.fields, donl_cell_new("field", 460, 0));

  vec_dec_retcount(donl.fields);

  // add list handler to view

  vh_list_add(donl.view,
              ((vh_list_inset_t){0, 0, 0, 0}),
              ui_about_popup_item_for_index, NULL, NULL);

  // create items

  VADD(donl.items, donateitem_create(0));
  VADD(donl.items, donateitem_create(1));
  VADD(donl.items, donateitem_create(2));
  VADD(donl.items, donateitem_create(3));
  VADD(donl.items, donateitem_create(4));
  VADD(donl.items, donateitem_create(5));

  char* version = cstr_fromformat(200, "Zen Music by Milan Toth\nv%i-%i\nFree and Open Source Software.\nIf you like it, please support the development.",
                                  VERSION / 10000,
                                  VERSION % 10000);

  donateitem_update_row(donl.items->data[0], 0, version);
  donateitem_update_row(donl.items->data[1], 1, "Support on Patreon");
  donateitem_update_row(donl.items->data[2], 2, "Donate on Paypal");
  donateitem_update_row(donl.items->data[3], 3, "Report an Issue");
  donateitem_update_row(donl.items->data[4], 4, "GitHub Page");

  REL(version);
}

#endif
