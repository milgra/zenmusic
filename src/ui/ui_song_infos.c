#ifndef ui_song_infos_h
#define ui_song_infos_h

#include "view.c"

void ui_song_infos_attach(view_t* baseview);
void ui_song_infos_update_time(double time, double left, double duration);
void ui_song_infos_show(int index);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "text.c"
#include "tg_text.c"
#include "visible.c"

struct ui_song_infos_t
{
  view_t*     song_info_view;
  view_t*     song_remaining_view;
  view_t*     song_time_view;
  view_t*     song_length_view;
  textstyle_t textstyle;
  char*       fontpath;
} uisi;

void ui_song_infos_attach(view_t* baseview)
{

  uisi.fontpath            = config_get("font_path");
  uisi.song_info_view      = view_get_subview(baseview, "song_info");
  uisi.song_time_view      = view_get_subview(baseview, "song_info_time");
  uisi.song_length_view    = view_get_subview(baseview, "song_info_length");
  uisi.song_remaining_view = view_get_subview(baseview, "song_info_remaining");

  textstyle_t ts  = {0};
  ts.font         = uisi.fontpath;
  ts.align        = TA_CENTER;
  ts.margin_right = 0;
  ts.size         = 30.0;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;

  uisi.textstyle = ts;

  tg_text_add(uisi.song_info_view);
  tg_text_set(uisi.song_info_view, "-", ts);
  tg_text_add(uisi.song_time_view);
  tg_text_add(uisi.song_length_view);
  tg_text_add(uisi.song_remaining_view);

  ui_song_infos_update_time(0.0, 0.0, 0.0);
}

void ui_song_infos_update_time(double time, double left, double dur)
{
  textstyle_t ts = {0};
  ts.font        = uisi.fontpath;
  ts.align       = TA_RIGHT;
  ts.size        = 30.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;
  ts.margin_left = 17;

  char timebuff[20];

  int tmin = (int)floor(time / 60.0);
  int tsec = (int)time % 60;
  int lmin = (int)floor(left / 60.0);
  int lsec = (int)left % 60;
  int dmin = (int)floor(dur / 60.0);
  int dsec = (int)dur % 60;

  ts.align = TA_LEFT;
  snprintf(timebuff, 20, "%.2i:%.2i", dmin, dsec);
  tg_text_set(uisi.song_length_view, timebuff, ts);
  snprintf(timebuff, 20, "%.2i:%.2i", tmin, tsec);
  tg_text_set(uisi.song_time_view, timebuff, ts);
  snprintf(timebuff, 20, "%.2i:%.2i", lmin, lsec);
  tg_text_set(uisi.song_remaining_view, timebuff, ts);
}

void ui_song_infos_show(int index)
{
  textstyle_t ts = {0};
  ts.font        = uisi.fontpath;
  ts.align       = TA_CENTER;
  ts.size        = 28.0;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;

  vec_t* songs   = visible_get_songs();
  map_t* songmap = songs->data[index];

  char* sample = MGET(songmap, "file/sample_rate");
  char* bit    = MGET(songmap, "file/bit_rate");
  int   sr     = atoi(sample);
  int   br     = atoi(bit);

  char* infostr = mem_calloc(100, "char*", NULL, NULL);

  snprintf(infostr, 100, "%s\n%s\n%s/%iKHz/%iKbit/%s channels",
           (char*)MGET(songmap, "meta/title"),
           (char*)MGET(songmap, "meta/artist"),
           (char*)MGET(songmap, "meta/genre"),
           sr / 1000,
           br / 1000,
           (char*)MGET(songmap, "file/channels"));

  tg_text_set(uisi.song_info_view, infostr, ts);
}

#endif
