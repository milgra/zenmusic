#ifndef ui_filter_popup_h
#define ui_filter_popup_h

#include "view.c"

void ui_filter_popup_attach(view_t* baseview);
void ui_filter_popup_show();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "callbacks.c"
#include "config.c"
#include "textlist.c"
#include "tg_text.c"
#include "ui_filter_bar.c"
#include "ui_popup_switcher.c"
#include "ui_songlist.c"
#include "visible.c"
#include "zc_cstring.c"

void ui_filter_popup_on_genre_select(int index);
void ui_filter_popup_on_artist_select(int index);

struct _ui_filter_popup_t
{
  textlist_t* artistlist;
  textlist_t* genrelist;

} ufp = {0};

void ui_filter_popup_attach(view_t* baseview)
{
  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;
  ts.align        = TA_RIGHT;
  ts.margin_right = 20;

  ufp.genrelist  = textlist_new(view_get_subview(baseview, "genrelist"), visible_get_genres(), ts, ui_filter_popup_on_genre_select);
  ts.align       = TA_LEFT;
  ufp.artistlist = textlist_new(view_get_subview(baseview, "artistlist"), visible_get_artists(), ts, ui_filter_popup_on_artist_select);
}

void ui_filter_popup_show()
{
  textlist_update(ufp.genrelist);
  textlist_update(ufp.artistlist);

  ui_popup_switcher_toggle("filters_popup_page");
}

void ui_filter_popup_on_genre_select(int index)
{
  printf("on genre select %i\n", index);

  vec_t* genres = visible_get_genres();
  char*  genre  = genres->data[index];
  // callbacks_call("on_genre_selected", genre);

  char* query = cstr_fromformat(100, "genre is %s", genre);

  visible_set_filter(query);
  ui_songlist_update();
  ui_filter_bar_show_query(query);
}

void ui_filter_popup_on_artist_select(int index)
{
  printf("on artist select %i\n", index);

  vec_t* artists = visible_get_artists();
  char*  artist  = artists->data[index];
  // callbacks_call("on_artist_selected", artist);

  char* query = cstr_fromformat(100, "artist is %s", artist);

  visible_set_filter(query);
  ui_songlist_update();
  ui_filter_bar_show_query(query);
}

#endif
