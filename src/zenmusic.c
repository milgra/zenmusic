#include "callbacks.c"
#include "config.c"
#include "database.c"
#include "editor.c"
#include "files.c"
#include "filtered.c"
#include "library.c"
#include "mtcallback.c"
#include "mtchannel.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "mtstring.c"
#include "player.c"
#include "remote.c"
#include "ui.c"
#include "ui_manager.c"
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
void on_save_entry(void* userdata, void* data);
void on_song_header(void* userdata, void* data);
void on_change_organize(void* userdata, void* data);
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
  filtered_init();
  callbacks_init();

  // init callbacks

  callbacks_set("om_save_entry", cb_new(on_save_entry, NULL));
  callbacks_set("on_song_header", cb_new(on_song_header, NULL));
  callbacks_set("on_change_library", cb_new(on_change_library, NULL));
  callbacks_set("on_change_organize", cb_new(on_change_organize, NULL));
  callbacks_set("on_filter_songs", cb_new(on_filter_songs, NULL));
  callbacks_set("on_genre_selected", cb_new(on_genre_select, NULL));
  callbacks_set("on_artist_selected", cb_new(on_artist_select, NULL));

  // init config

#ifndef DEBUG
  char* res_path = cstr_fromstring("/usr/local/share/zenmusic");
#else
  char* res_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/../res", path);
#endif

  config_set("organize_db", "false");
  config_set("remote_enabled", "false");
  config_set("ui_color", "0xEEEEEEFF");
  config_set("res_path", res_path);

  // read config, it overwrites defaults if exists

  config_read();

  // load ui from descriptors

  ui_load(width, height, config_get("res_path"), config_get("lib_path"));

  // show library popup if no lib path is saved yet or load library

  if (config_get("lib_path") == NULL)
    ui_show_libpath_popup("Please enter the location of your music library folder.");
  else
    load_library();

  // start listening for remote control events if set

  if (config_get("remote_enabled"))
  {
    if (config_get_bool("remote_enabled")) remote_listen(zm.rem_ch);
  }

  REL(res_path);
}

void update(ev_t ev)
{
  // get key events

  if (ev.type == EV_KDOWN)
  {
    if (ev.keycode == SDLK_SPACE)
    {
      int state = player_toggle_pause();
      ui_toggle_pause(state);
    }
  }

  if (ev.type == EV_TIME)
  {
    // get analyzed song entries

    map_t* entry;
    while ((entry = ch_recv(zm.lib_ch)))
    {
      char* path = MGET(entry, "file/path");

      if (strcmp(path, "//////") != 0)
      {
        db_add_entry(path, entry);
        //MPUT(db, path, entry); // store entry
        //VADD(songs, entry);
        if (db_count() % 100 == 0)
        {
          filtered_set_sortfield("meta/artist", 0);
          ui_refresh_songlist();
        }
      }
      else
      {
        // analyzing is finished, sort and store database

        db_write(config_get("lib_path"));

        /* if (config_get_bool("organize_db")) */
        /* { */
        /*   int succ = lib_organize(zm.lib_path, db_get_db()); */
        /*   if (succ == 0) db_write(zm.lib_path); */
        /* } */

        filtered_set_sortfield("meta/artist", 0);
        ui_refresh_songlist();
      }
      // cleanup, ownership was passed with the channel from analyzer
      REL(entry);
    }

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
        char* new_play_count = mem_calloc(10, "char*", NULL, NULL);
        snprintf(new_play_count, 10, "%i", play_count_i);
        MPUT(entry, "file/play_count", new_play_count);
        REL(new_play_count);

        db_write(config_get("lib_path"));
      }

      // play next song
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
    // update timer
    if (floor(phead) != zm.last_step)
    {
      zm.last_step = floor(phead);

      double posratio = phead / player_duration();
      double volratio = player_volume();

      ui_update_time(zm.last_step, player_duration() - zm.last_step, player_duration());
      ui_update_position(posratio);
      ui_update_volume(volratio);
    }

    ui_update_visualizer();
    ui_update_video();
  }
  else
  {
    if (zm.last_step < time)
    {
      zm.last_step = time + 1000;

      ui_update_time(0.0, 0.0, 0.0);
      ui_update_volume(0.9);
    }
  }

  ui_manager_render(time);
}

void destroy()
{
  printf("zenmusic destroy\n");
}

void load_library()
{
  assert(config_get("lib_path") != NULL);

  db_reset();
  db_read(config_get("lib_path")); // read db

  lib_read(config_get("lib_path"));   // read library
  lib_remove_duplicates(db_get_db()); // remove existing

  if (lib_entries() > 0) lib_analyze(zm.lib_ch); // start analyzing new entries

  filtered_set_sortfield("meta/artist", 0);
}

void on_change_library(void* userdata, void* data)
{
  char* new_path = data;
  char* lib_path = NULL;

  if (new_path[0] == '~')
    lib_path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", getenv("HOME"), new_path + 1); // replace tilde's with home
  else
    lib_path = cstr_fromcstring(new_path);

  // remove slash if needed

  if (lib_path[strlen(lib_path) - 1] == '/') lib_path[strlen(lib_path) - 1] = '\0';

  if (files_path_exists(lib_path))
  {
    printf("CHANGING LIBRARY %s\n", lib_path);

    config_set("lib_path", lib_path);
    config_write();

    load_library();
    ui_set_libpath(lib_path);
    ui_hide_libpath_popup();
  }
  else
    ui_show_libpath_popup("Location doesn't exists, please enter valid location.");

  REL(lib_path);
}

void on_save_entry(void* userdata, void* data)
{
  map_t* entry = data;
  // update metadata in file
  //editor_set_metadata(entry, "king.jpg");

  // move song to new place if needed
  //lib_organize_entry(zm.lib_path, db_get_db(), entry);

  // save database
  //db_write(zm.lib_path);
}

void on_song_header(void* userdata, void* data)
{
  char* id = data;

  filtered_set_sortfield(id, 1);
  // todo filtered should notify ui
  ui_refresh_songlist();
}

void on_filter_songs(void* userdata, void* data)
{
  char* text = str_cstring((str_t*)data);

  filtered_set_filter(text);
  ui_reload_songlist();
}

void on_genre_select(void* userdata, void* data)
{
  char* genre = data;
  char* query = cstr_fromformat(100, "genre is %s", genre);

  // genre select should narrow artist selector

  filtered_set_filter(query);
  ui_reload_songlist();
  ui_show_query(query);
}

void on_artist_select(void* userdata, void* data)
{
  char* artist = data;
  char* query  = cstr_fromformat(100, "artist is %s", artist);

  filtered_set_filter(query);
  ui_reload_songlist();
  ui_show_query(query);
}

void on_change_organize(void* userdata, void* data)
{
  char* value = data;
  char  flag  = strcmp(value, "Enable") == 0;

  config_set_bool("organize_db", flag);

  // ui_set_org_btn_lbl(organize ? "Disable" : "Enable");

  if (config_get_bool("organize_db"))
  {
    int succ = lib_organize(config_get("lib_path"), db_get_db());
    if (succ == 0) db_write(config_get("lib_path"));
    ui_refresh_songlist();
  }
}
