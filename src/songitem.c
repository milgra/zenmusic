#ifndef songitem_h
#define songitem_h

#include "mtmap.c"
#include "view.c"

view_t* songitem_new();
void    songitem_update(view_t* rowview, int index, map_t* file, void (*event)(view_t* view, void* data), char* fontpath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "eh_button.c"
#include "tg_css.c"
#include "tg_text.c"

#include "cr_text.c"
#include "eh_item.c"

uint32_t songitem_index = 0;

view_t* songitem_new(char* fontpath)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = 0;
  ts.size        = 25.0;
  ts.textcolor   = 0xFFFFFFFF;
  ts.backcolor   = 0x00000022;

  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songitem_index++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  eh_item_add(rowview, 35);

  eh_item_add_cell(rowview, "index", 50, cr_text_upd);
  eh_item_add_cell(rowview, "artist", 300, cr_text_upd);
  eh_item_add_cell(rowview, "title", 300, cr_text_upd);

  return rowview;
}

void songitem_update(view_t* rowview, int index, map_t* file, void (*event)(view_t* view, void* data), char* fontpath)
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

  /* size_t sind = index; */
  /* eh_button_add(rowview, (void*)sind, event); */

  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = 0;
  ts.size        = 25.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = color1;

  eh_item_upd_cell(rowview, "index", 30, &((cr_text_data_t){.style = ts, .text = indbuffer}));
  eh_item_upd_cell(rowview, "artist", 200, &((cr_text_data_t){.style = ts, .text = MGET(file, "artist")}));
  eh_item_upd_cell(rowview, "title", 300, &((cr_text_data_t){.style = ts, .text = MGET(file, "title")}));
}

#endif
