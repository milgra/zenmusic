#ifndef songitem_h
#define songitem_h

#include "mtmap.c"
#include "view.c"

view_t* songitem_new(char* fontpath, void (*on_select)(view_t* view, uint32_t index));
void    songitem_update(view_t* rowview, int index, map_t* file, char* fontpath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"

#include "cr_text.c"
#include "vh_list_item.c"

uint32_t songitem_index = 0;

view_t* songitem_new(char* fontpath, void (*on_select)(view_t* view, uint32_t index))
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

  vh_litem_add(rowview, 35, on_select);

  vh_litem_add_cell(rowview, "index", 50, cr_text_upd);
  vh_litem_add_cell(rowview, "artist", 300, cr_text_upd);
  vh_litem_add_cell(rowview, "title", 300, cr_text_upd);

  return rowview;
}

void songitem_update(view_t* rowview, int index, map_t* file, char* fontpath)
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

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

  vh_litem_upd(rowview, index);
  vh_litem_upd_cell(rowview, "index", 30, &((cr_text_data_t){.style = ts, .text = indbuffer}));
  vh_litem_upd_cell(rowview, "artist", 200, &((cr_text_data_t){.style = ts, .text = MGET(file, "artist")}));
  vh_litem_upd_cell(rowview, "title", 300, &((cr_text_data_t){.style = ts, .text = MGET(file, "title")}));
}

#endif
