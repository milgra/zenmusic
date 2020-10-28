#ifndef songitem_h
#define songitem_h

#include "view.c"

view_t* songitem_new();
void    songitem_update(view_t* rowitem, int index, char* filename);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_color.c"
#include "tg_text.c"

uint32_t songitem_index = 0;

view_t* songitem_new()
{
  char idbuffer[100] = {0};
  snprintf(idbuffer, 100, "list_item%i", songitem_index);

  view_t* rowview = view_new(idbuffer, (vframe_t){0, 0, 1500, 35}, 0);

  tg_color_add(rowview, 0x00000022);

  snprintf(idbuffer, 100, "index_item%i", songitem_index);
  view_t* indexview = view_new(idbuffer, (vframe_t){0, 0, 80, 35}, 0);

  view_add(rowview, indexview);

  snprintf(idbuffer, 100, "name_item%i", songitem_index);
  view_t* nameview = view_new(idbuffer, (vframe_t){80, 0, 500, 35}, 0);

  view_add(rowview, nameview);

  snprintf(idbuffer, 100, "type_item%i", songitem_index);
  view_t* typeview = view_new(idbuffer, (vframe_t){580, 0, 80, 35}, 0);

  view_add(rowview, typeview);

  songitem_index++;

  return rowview;
}

void songitem_update(view_t* rowview, int index, char* filename)
{
  uint32_t color1 = (index % 2 == 0) ? 0xEFEFEFFF : 0xE5E5E5FF;
  uint32_t color2 = (index % 2 == 0) ? 0xE5E5E5FF : 0xDFDFDFFF;

  char indbuffer[6];
  snprintf(indbuffer, 6, "%i.", index);
  tg_text_add(rowview->views->data[0], color2, 0x000000FF, indbuffer);

  tg_text_add(rowview->views->data[1], color1, 0x000000FF, filename);

  tg_text_add(rowview->views->data[2], color2, 0x000000FF, "mp3");
}

#endif
