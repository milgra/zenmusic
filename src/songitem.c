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

view_t* songitem_new()
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songitem_index);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 1500, 35});
  rowview->hidden = 1;

  snprintf(idbuffer, 100, "index_item%i", songitem_index);
  view_t* indexview = view_new(idbuffer, (r2_t){0, 0, 80, 35});
  tg_text_add(indexview, 0xFFFFFFFF, 0x000000FF, "0", 1);
  indexview->needs_touch = 0;

  snprintf(idbuffer, 100, "name_item%i", songitem_index);
  view_t* nameview = view_new(idbuffer, (r2_t){80, 0, 500, 35});
  tg_text_add(nameview, 0xFFFFFFFF, 0x000000FF, "0", 0);
  nameview->needs_touch = 0;

  snprintf(idbuffer, 100, "type_item%i", songitem_index);
  view_t* typeview = view_new(idbuffer, (r2_t){580, 0, 1000, 35});
  tg_text_add(typeview, 0xFFFFFFFF, 0x000000FF, "0", 1);
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

  tg_text_set(rowview->views->data[0], color1, 0x000000FF, indbuffer, 1);

  tg_text_set(rowview->views->data[1], color1, 0x000000FF, MGET(file, "artist"), 0);

  tg_text_set(rowview->views->data[2], color1, 0x000000FF, MGET(file, "title"), 0);
}

#endif
