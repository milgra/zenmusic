#include "config.c"
#include "db.c"
#include "lib.c"
#include "mtchannel.c"
#include "mtcstring.c"
#include "mtmap.c"
#include "player.c"
#include "ui.c"
#include "ui_manager.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double lasttime = 0.0;
char*  libpath  = NULL;

ch_t*  ch;
map_t* db;
map_t* cfg;
vec_t* songs;
vec_t* genres;
vec_t* artists;

void sort(char* field)
{
  db_sort(db, songs, field);
  db_genres(db, genres);
  db_artists(songs, artists);
}

void filter(view_t* view, char* text)
{
  db_filter(db, text, songs);
}

void save_db(map_t* entry)
{
  // update metadata in file
  player_set_metadata(entry, "king.jpg");

  // move song to new place if needed
  lib_organize_entry(libpath, db, entry);

  // save database
  db_write(libpath, db);
}

void load_lib()
{
  printf("load_lib %s\n", libpath);

  db_read(libpath, db);                   // read db
  lib_read(libpath);                      // read library
  lib_remove_duplicates(db);              // remove existing
  if (lib_entries() > 0) lib_analyze(ch); // start analyzing new entries

  sort("artist");
}

void save_lib(char* path)
{
  printf("save_lib %s\n", path);

  if (libpath) REL(libpath);

  if (path[0] == '~')
    libpath = cstr_fromformat("%s%s", getenv("HOME"), path + 1, NULL); // replace tilde's with home
  else
    libpath = cstr_fromcstring(path);

  if (lib_exists(libpath))
  {
    MPUT(cfg, "library_path", libpath);
    config_write(cfg);
    load_lib();
    ui_hide_libpath_popup();
  }
  else
    ui_show_libpath_popup("Location doesn't exists, please enter valid location.");
}

void init(int width, int height, char* respath)
{
  srand((unsigned int)time(NULL));

  db  = MNEW();      // database
  ch  = ch_new(100); // comm channel for library entries
  cfg = MNEW();      // config map

  songs   = VNEW();
  genres  = VNEW();
  artists = VNEW();

  ui_init(width,
          height,
          respath,
          songs,
          genres,
          artists,
          save_db,
          save_lib);

  config_read(cfg);

  libpath = MGET(cfg, "library_path");

  if (!libpath)
  {
    config_init(cfg);
    ui_show_libpath_popup("Please enter the location of your music library folder.");
  }
  else
    load_lib();
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

  // get analyzed song entries

  map_t* entry;
  while ((entry = ch_recv(ch)))
  {
    char* path = MGET(entry, "path");

    if (strcmp(path, "//////") != 0)
    {
      MPUT(db, path, entry); // store entry
      VADD(songs, entry);
      if (db->count % 100 == 0) ui_refresh_songlist();
    }
    else
    {
      // analyzing is finished, sort and store database

      sort("artist");
      db_write(libpath, db);

      int succ = lib_organize(libpath, db);
      if (succ == 0) db_write(libpath, db);

      ui_refresh_songlist();
    }
    // cleanup, ownership was passed with the channel from analyzer
    REL(entry);
  }

  // update player

  player_refresh();

  // update ui

  double time = player_time();
  if (time > 0.0)
  {
    // update timer
    if (floor(time) != lasttime)
    {
      lasttime = floor(time);

      double posratio = time / player_duration();
      double volratio = player_volume();

      ui_update_time(lasttime);
      ui_update_position(posratio);
      ui_update_volume(volratio);
    }

    ui_update_visualizer();
    ui_update_video();
  }

  ui_manager_event(ev);
}

void render(uint32_t time)
{
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
