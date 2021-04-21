#include "callbacks.c"
#include "config.c"
#include "database.c"
#include "editor.c"
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

double lasttime = 0.0;
char*  libpath  = NULL;
int    organize = 0;
ch_t*  ch;     // library channel
ch_t*  rem_ch; // remote channel
char   ui_cleared = 0;

void load_library();

void on_save_entry(void* userdata, void* data)
{
  map_t* entry = data;
  // update metadata in file
  //editor_set_metadata(entry, "king.jpg");

  // move song to new place if needed
  //lib_organize_entry(libpath, db_get_db(), entry);

  // save database
  //db_write(libpath);
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

void on_change_library(void* userdata, void* data)
{
  char* path = data;

  if (libpath) REL(libpath);

  if (path[0] == '~')
    libpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", getenv("HOME"), path + 1); // replace tilde's with home
  else
    libpath = cstr_fromcstring(path);

  if (path[strlen(path) - 1] != '/')
    libpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/", path); // add closing slash

  if (lib_exists(libpath))
  {
    printf("CHANGE LIBRARY %s\n", libpath);
    config_set("library_path", libpath);
    config_write();

    libpath = config_get("library_path");
    load_library();
    ui_set_libpath(libpath);
    ui_hide_libpath_popup();
  }
  else
    ui_show_libpath_popup("Location doesn't exists, please enter valid location.");
}

void on_change_organize(void* userdata, void* data)
{
  char* value = data;
  organize    = strcmp(value, "Enable") == 0;

  char* newval = cstr_fromcstring(organize ? "true" : "false");
  config_set("organize_db", newval);
  REL(newval);

  // ui_set_org_btn_lbl(organize ? "Disable" : "Enable");

  if (organize)
  {
    int succ = lib_organize(libpath, db_get_db());
    if (succ == 0) db_write(libpath);
    ui_refresh_songlist();
  }
}

void load_library()
{
  db_reset();

  db_read(libpath);                       // read db
  lib_read(libpath);                      // read library
  lib_remove_duplicates(db_get_db());     // remove existing
  if (lib_entries() > 0) lib_analyze(ch); // start analyzing new entries

  filtered_set_sortfield("meta/artist", 0);
}

void init(int width, int height, char* respath)
{
  srand((unsigned int)time(NULL));

  ch     = ch_new(100); // comm channel for library entries
  rem_ch = ch_new(10);  // remote channel

  remote_listen(rem_ch);

  db_init();
  player_init();
  config_init();
  config_read();
  filtered_init();

  callbacks_init();
  callbacks_set("om_save_entry", cb_new(on_save_entry, NULL));
  callbacks_set("on_song_header", cb_new(on_song_header, NULL));
  callbacks_set("on_change_library", cb_new(on_change_library, NULL));
  callbacks_set("on_change_organize", cb_new(on_change_organize, NULL));
  callbacks_set("on_filter_songs", cb_new(on_filter_songs, NULL));
  callbacks_set("on_genre_selected", cb_new(on_genre_select, NULL));
  callbacks_set("on_artist_selected", cb_new(on_artist_select, NULL));

#ifndef DEBUG
  respath = "/usr/local/share/zenmusic";
#else
  respath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/../res", respath);
#endif
  libpath = config_get("library_path");

  char* orgstr = config_get("organize_db");
  organize     = strcmp(orgstr, "true") == 0;

  ui_init(width, height, respath, libpath);

  if (!libpath)
  {
    ui_show_libpath_popup("Please enter the location of your music library folder.");
  }
  else
    load_library();
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
    while ((entry = ch_recv(ch)))
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

        db_write(libpath);

        /* if (organize) */
        /* { */
        /*   int succ = lib_organize(libpath, db_get_db()); */
        /*   if (succ == 0) db_write(libpath); */
        /* } */

        filtered_set_sortfield("meta/artist", 0);
        ui_refresh_songlist();
      }
      // cleanup, ownership was passed with the channel from analyzer
      REL(entry);
    }

    char* buffer = NULL;
    if ((buffer = ch_recv(rem_ch)))
    {
      if (buffer[0] == '0') ui_play_pause();
      if (buffer[0] == '1') ui_play_next();
      if (buffer[0] == '2') ui_play_prev();
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

        db_write(libpath);
      }

      // play next song
      ui_play_next();
    }
  }

  ui_manager_event(ev);
}

void render(uint32_t time)
{
  double phead = player_time();
  if (phead > 0.0)
  {
    // update timer
    if (floor(phead) != lasttime)
    {
      lasttime = floor(phead);

      double posratio = phead / player_duration();
      double volratio = player_volume();

      ui_update_time(lasttime, player_duration() - lasttime, player_duration());
      ui_update_position(posratio);
      ui_update_volume(volratio);
    }

    ui_update_visualizer();
    ui_update_video();

    ui_cleared = 0;
  }
  else
  {
    if (ui_cleared == 0)
    {
      ui_update_time(0.0, 0.0, 0.0);
      ui_update_volume(0.9);
      ui_cleared = 1;
    }
  }

  ui_manager_render(time);
}

void destroy()
{
  printf("zenmusic destroy\n");
}

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}
