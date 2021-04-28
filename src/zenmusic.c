#include "callbacks.c"
#include "config.c"
#include "database.c"
#include "editor.c"
#include "files.c"
#include "library.c"
#include "mtcallback.c"
#include "mtchannel.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtstring.c"
#include "player.c"
#include "remote.c"
#include "ui.c"
#include "ui_filter_bar.c"
#include "ui_lib_init_popup.c"
#include "ui_manager.c"
#include "ui_play_controls.c"
#include "ui_song_infos.c"
#include "ui_songlist.c"
#include "ui_visualizer.c"
#include "visible.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(int width, int height, char* path);
void update(ev_t ev);
void render(uint32_t time);
void destroy();

void load_library();

void on_change_library(void* userdata, void* data);
void on_change_organize(void* userdata, void* data);
void on_song_header(void* userdata, void* data);
void on_genre_select(void* userdata, void* data);
void on_artist_select(void* userdata, void* data);
void on_filter_songs(void* userdata, void* data);

struct
{
  double last_step; // last timestep
  ch_t*  lib_ch;    // library channel
  ch_t*  rem_ch;    // remote channel
} zm = {0};

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}

void init(int width, int height, char* path)
{
  srand((unsigned int)time(NULL));

  zm.lib_ch = ch_new(100); // comm channel for library entries
  zm.rem_ch = ch_new(10);  // remote channel

  db_init();
  ui_init();
  config_init();
  player_init();
  visible_init();
  callbacks_init();

  // init callbacks

  callbacks_set("on_song_header", cb_new(on_song_header, NULL));
  callbacks_set("on_change_library", cb_new(on_change_library, NULL));
  callbacks_set("on_change_organize", cb_new(on_change_organize, NULL));
  callbacks_set("on_filter_songs", cb_new(on_filter_songs, NULL));
  callbacks_set("on_genre_selected", cb_new(on_genre_select, NULL));
  callbacks_set("on_artist_selected", cb_new(on_artist_select, NULL));

  // init paths

#ifndef DEBUG
  char* res_path = cstr_fromstring("/usr/local/share/zenmusic");
#else
  char* res_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s../res", path);
#endif
  char* cfg_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/.config/zenmusic/config.kvl", getenv("HOME"));

  char* css_path  = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.css", res_path);
  char* html_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/main.html", res_path);
  char* font_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/Baloo.ttf", res_path);

  // print path info to console

  printf("config path   : %s\n", cfg_path);
  printf("resource path : %s\n", res_path);
  printf("css path      : %s\n", res_path);
  printf("html path     : %s\n", html_path);
  printf("font path     : %s\n", font_path);

  // init config

  config_set("organize_db", "false");
  config_set("remote_enabled", "false");
  config_set("ui_color", "0xEEEEEEFF");
  config_set("res_path", res_path);

  // read config, it overwrites defaults if exists

  config_read(cfg_path);

  // init non-configurable defaults

  config_set("cfg_path", cfg_path);
  config_set("css_path", css_path);
  config_set("html_path", html_path);
  config_set("font_path", font_path);

  // load ui from descriptors

  ui_load(width, height);

  // show library popup if no lib path is saved yet or load library

  if (config_get("lib_path") == NULL)
    ui_lib_init_popup_show("Please enter the location of your music library folder.");
  else
    load_library();

  // start listening for remote control events if set

  if (config_get("remote_enabled"))
  {
    if (config_get_bool("remote_enabled")) remote_listen(zm.rem_ch);
  }

  REL(res_path);
  REL(css_path);
  REL(html_path);
  REL(font_path);
}

void update(ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    map_t* entry;

    // get analyzed song entries

    while ((entry = ch_recv(zm.lib_ch)))
    {
      char* path = MGET(entry, "file/path");

      if (strcmp(path, "//////") != 0)
      {
        // store entry in db

        db_add_entry(path, entry);

        if (db_count() % 100 == 0)
        {
          // filter and sort current db and show in ui partial analysis result

          visible_set_sortfield("meta/artist", 0);
          ui_songlist_refresh();
        }
      }
      else
      {
        db_write(config_get("lib_path"));

        if (config_get_bool("organize_db"))
        {
          // organize db if needed

          int succ = lib_organize(config_get("lib_path"), db_get_db());
          if (succ == 0) db_write(config_get("lib_path"));
        }

        visible_set_sortfield("meta/artist", 0);
        ui_songlist_refresh();
      }

      // cleanup, ownership was passed with the channel from analyzer

      REL(entry);
    }

    // get remote events

    char* buffer = NULL;
    if ((buffer = ch_recv(zm.rem_ch)))
    {
      if (buffer[0] == '0') ui_play_pause();
      if (buffer[0] == '1') ui_play_prev();
      if (buffer[0] == '2') ui_play_next();
    }

    // update player

    int finished = player_refresh();

    if (finished)
    {
      // increase play count of song

      char*  path  = player_get_path();
      map_t* entry = MGET(db_get_db(), path);

      if (entry)
      {
        char* play_count_s = MGET(entry, "file/play_count");
        int   play_count_i = 0;

        if (play_count_s != NULL) play_count_i = atoi(play_count_s);
        play_count_i += 1;
        MPUTR(entry, "file/play_count", cstr_fromformat(10, "%i", play_count_i));
        db_write(config_get("lib_path"));
      }

      ui_play_next();
    }
  }

  ui_manager_event(ev);
}

// render, called once per frame

void render(uint32_t time)
{
  double phead = player_time();

  if (phead > 0.0)
  {
    if (floor(phead) != zm.last_step)
    {
      zm.last_step = floor(phead);

      double posratio = phead / player_duration();
      double volratio = player_volume();

      ui_song_infos_update_time(zm.last_step, player_duration() - zm.last_step, player_duration());
      ui_play_update_position(posratio);
      ui_play_update_volume(volratio);
    }

    ui_visualizer_update();
    ui_visualizer_update_video();
  }

  ui_manager_render(time);
}

void destroy()
{
}

void load_library()
{
  assert(config_get("lib_path") != NULL);

  db_reset();
  db_read(config_get("lib_path"));

  lib_read(config_get("lib_path"));
  lib_remove_duplicates(db_get_db());

  if (lib_entries() > 0) lib_analyze(zm.lib_ch); // start analyzing new entries

  visible_set_sortfield("meta/artist", 0);
}

void on_change_library(void* userdata, void* data)
{
  char* new_path = data;
  char* lib_path = NULL;

  // construct path if needed

  if (new_path[0] == '~')
    lib_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", getenv("HOME"), new_path + 1); // replace tilde's with home
  else
    lib_path = cstr_fromcstring(new_path);

  // remove slash if needed

  if (lib_path[strlen(lib_path) - 1] == '/') lib_path[strlen(lib_path) - 1] = '\0';

  // change library if exists

  if (files_path_exists(lib_path))
  {
    config_set("lib_path", lib_path);
    config_write(config_get("cfg_path"));

    load_library();

    ui_lib_init_popup_hide();
  }
  else
    ui_lib_init_popup_show("Location doesn't exists, please enter valid location.");

  REL(lib_path);
}

void on_change_organize(void* userdata, void* data)
{
  char* value = data;
  char  flag  = strcmp(value, "Enable") == 0;

  config_set_bool("organize_db", flag);

  // ui_set_org_btn_lbl(flag ? "Disable" : "Enable");

  if (config_get_bool("organize_db"))
  {
    int succ = lib_organize(config_get("lib_path"), db_get_db());
    if (succ == 0) db_write(config_get("lib_path"));
    ui_songlist_refresh();
  }
}

void on_song_header(void* userdata, void* data)
{
  char* id = data;

  visible_set_sortfield(id, 1);
  ui_songlist_refresh();
}

void on_filter_songs(void* userdata, void* data)
{
  char* text = str_cstring((str_t*)data);

  visible_set_filter(text);
  ui_songlist_update();
}

void on_genre_select(void* userdata, void* data)
{
  char* genre = data;
  char* query = cstr_fromformat(100, "genre is %s", genre);

  visible_set_filter(query);
  ui_songlist_update();
  ui_filter_bar_show_query(query);
}

void on_artist_select(void* userdata, void* data)
{
  char* artist = data;
  char* query  = cstr_fromformat(100, "artist is %s", artist);

  visible_set_filter(query);
  ui_songlist_update();
  ui_filter_bar_show_query(query);
}
