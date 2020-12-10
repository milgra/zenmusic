#ifndef songitem_h
#define songitem_h

#include "mtmap.c"
#include "view.c"

view_t* songitem_new();
void    songitem_update(view_t* rowview, int index, mtmap_t* file, void (*event)(view_t* view, void* data));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "eh_button.c"
#include "tg_css.c"
#include "tg_text.c"

uint32_t songitem_index = 0;

view_t* songitem_new(char* fontpath)
{
  textstyle_t ts = {0};
  ts.font        = fontpath;
  ts.align       = 0;
  ts.size        = 20.0;
  ts.textcolor   = 0xFFFFFFFF;
  ts.backcolor   = 0x00000000;

  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songitem_index);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 1500, 35});
  rowview->hidden = 1;

  snprintf(idbuffer, 100, "index_item%i", songitem_index);
  view_t* indexview = view_new(idbuffer, (r2_t){0, 0, 80, 35});
  tg_text_add(indexview, "index", ts);
  indexview->needs_touch = 0;

  snprintf(idbuffer, 100, "name_item%i", songitem_index);
  view_t* nameview = view_new(idbuffer, (r2_t){80, 0, 500, 35});
  tg_text_add(nameview, "name", ts);
  nameview->needs_touch = 0;

  snprintf(idbuffer, 100, "type_item%i", songitem_index);
  view_t* typeview = view_new(idbuffer, (r2_t){580, 0, 1000, 35});
  tg_text_add(typeview, "type", ts);
  typeview->needs_touch = 0;

  view_add(rowview, indexview);
  view_add(rowview, nameview);
  view_add(rowview, typeview);

  songitem_index++;

  return rowview;
}

void songitem_update(view_t* rowview, int index, mtmap_t* file, void (*event)(view_t* view, void* data))
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

  size_t sind = index;
  eh_button_add(rowview, (void*)sind, event);

  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  view_t* idview     = rowview->views->data[0];
  view_t* artistview = rowview->views->data[1];
  view_t* titleview  = rowview->views->data[2];

  tg_text_t* tg       = idview->tex_gen_data;
  tg->style.align     = 1;
  tg->style.backcolor = color1;
  tg->style.textcolor = 0x000000FF;

  tg                  = artistview->tex_gen_data;
  tg->style.backcolor = color1;
  tg->style.textcolor = 0x000000FF;

  tg                  = titleview->tex_gen_data;
  tg->style.backcolor = color1;
  tg->style.textcolor = 0x000000FF;

  tg_text_set(idview, indbuffer);

  tg_text_set(artistview, MGET(file, "artist"));

  tg_text_set(titleview, MGET(file, "title"));
}

#endif
