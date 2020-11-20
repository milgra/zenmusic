#ifndef songitem_h
#define songitem_h

#include "eh_touch.c"
#include "view.c"

view_t* songitem_new();
void    songitem_update(view_t* rowview, int index, char* filename, void (*event)(ev_t ev, void* data));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"
#include "tg_text.c"

uint32_t songitem_index = 0;

view_t* songitem_new()
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songitem_index);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 1500, 35});

  rowview->layout.background_color = 0x00000022;
  tg_css_add(rowview);

  snprintf(idbuffer, 100, "index_item%i", songitem_index);
  view_t* indexview = view_new(idbuffer, (r2_t){0, 0, 80, 35});

  view_add(rowview, indexview);

  snprintf(idbuffer, 100, "name_item%i", songitem_index);
  view_t* nameview = view_new(idbuffer, (r2_t){80, 0, 1000, 35});

  view_add(rowview, nameview);

  snprintf(idbuffer, 100, "type_item%i", songitem_index);
  view_t* typeview = view_new(idbuffer, (r2_t){1080, 0, 1000, 35});

  view_add(rowview, typeview);

  songitem_index++;

  return rowview;
}

void songitem_update(view_t* rowview, int index, char* filename, void (*event)(ev_t ev, void* data))
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xEFEFEFFF;

  size_t sind = index;
  eh_touch_add(rowview, (void*)sind, event);

  char indbuffer[6];
  if (index > -1)
    snprintf(indbuffer, 6, "%i.", index);
  else
    snprintf(indbuffer, 6, "No.");

  tg_text_add(rowview->views->data[0], color1, 0x000000FF, indbuffer, 1);

  tg_text_add(rowview->views->data[1], color1, 0x000000FF, filename, 0);

  tg_text_add(rowview->views->data[2], color1, 0x000000FF, "mp3", 0);
}

#endif
