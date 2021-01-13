/*
  genrelist table and events handler
 */

#ifndef genrelist_h
#define genrelist_h

#include "view.c"

void genrelist_attach(view_t* base, vec_t* genres, char* fontpath, void (*on_select)(char*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cr_text.c"
#include "vh_list.c"
#include "vh_list_item.c"
#include "view_util.c"

struct _genrelist_t
{
  vec_t*      genres;
  view_t*     view;
  textstyle_t textstyle;

  void (*on_select)(char* id);
} grel = {0};

view_t* genrelist_create_item(view_t* listview);
int     genrelist_update_item(view_t* listview, view_t* item, int index, int* item_count);

void genrelist_attach(view_t* base, vec_t* genres, char* fontpath, void (*on_select)(char*))
{
  grel.genres = genres;

  view_t* genrelist = view_get_subview(base, "genrelist");
  vh_list_add(genrelist, genrelist_create_item, genrelist_update_item);

  grel.view                   = genrelist;
  grel.textstyle.font         = fontpath;
  grel.textstyle.align        = TA_RIGHT;
  grel.textstyle.margin_right = 20;
  grel.textstyle.size         = 25.0;
  grel.textstyle.textcolor    = 0x000000FF;
  grel.textstyle.backcolor    = 0xFFFFFFFF;
}

void genrelist_update()
{
  vh_list_reset(grel.view);
}

void on_genreitem_select(view_t* view, uint32_t index, ev_t ev)
{
  printf("on_genreitem_select\n");
  (*grel.on_select)(grel.genres->data[index]);
}

view_t* genrelist_create_item(view_t* listview)
{
  static int item_cnt      = 0;
  char       idbuffer[100] = {0};
  snprintf(idbuffer, 100, "genrelist_item%i", item_cnt++);

  view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35});
  rowview->hidden = 1;

  vh_litem_add(rowview, 35, on_genreitem_select);
  vh_litem_add_cell(rowview, "genre", 230, cr_text_add, cr_text_upd);

  return rowview;
}

int genrelist_update_item(view_t* listview, view_t* item, int index, int* item_count)
{
  if (index < 0)
    return 1; // no items before 0
  if (index >= grel.genres->length)
    return 1; // no more items

  *item_count = grel.genres->length;

  vh_litem_upd_cell(item, "genre", &((cr_text_data_t){.style = grel.textstyle, .text = grel.genres->data[index]}));

  return 0;
}

#endif
