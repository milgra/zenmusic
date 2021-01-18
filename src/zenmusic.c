#include "config.c"
#include "db.c"
#include "lib.c"
#include "mtchannel.c"
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

char*  libpath  = "/home/milgra/Music";
double lasttime = 0.0;

ch_t*  ch;
map_t* db;
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

  // move song to new place if needed
  lib_organize_entry(libpath, db, entry);

  // save database
  db_write(libpath, db);

  // reload song list
}

void init(int width, int height, char* respath)
{
  srand((unsigned int)time(NULL));

  db = MNEW();      // database
  ch = ch_new(100); // comm channel for library entries

  songs   = VNEW();
  genres  = VNEW();
  artists = VNEW();

  ui_init(width,
          height,
          respath,
          songs,
          genres,
          artists,
          save_db); // init ui

  config_init(); // init config

  db_read(libpath, db);                   // read db
  lib_read(libpath);                      // read library
  lib_remove_duplicates(db);              // remove existing
  if (lib_entries() > 0) lib_analyze(ch); // start analyzing new entries

  sort("artist");
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
    }
    else
    {
      // analyzing is finished, sort and store database

      sort("artist");
      db_write(libpath, db);

      int succ = lib_organize(libpath, db);
      if (succ == 0) db_write(libpath, db);
    }
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
