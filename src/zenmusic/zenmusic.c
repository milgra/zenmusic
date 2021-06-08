#include "callbacks.c"
#include "coder.c"
#include "config.c"
#include "database.c"
#include "evrecorder.c"
#include "files.c"
#include "library.c"
#include "player.c"
#include "remote.c"
#include "tg_css.c"
#include "ui.c"
#include "ui_compositor.c"
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
#include "zc_callback.c"
#include "zc_channel.c"
#include "zc_cstring.c"
#include "zc_cstrpath.c"
#include "zc_log.c"
#include "zc_map.c"
#include "zc_string.c"
#include <SDL.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(int width, int height, char* path);
void update(ev_t ev);
void render(uint32_t time);
void destroy();

void load_library();
void on_change_remote(void* userdata, void* data);
void on_change_library(void* userdata, void* data);
void on_change_organize(void* userdata, void* data);
void get_analyzed_songs();
void get_remote_events();
void update_player();
void save_screenshot();

struct
{
  double last_step; // last timestep
  ch_t*  lib_ch;    // library channel
  ch_t*  rem_ch;    // remote channel

  char* cfg_par; // config path parameter
  char* res_par; // resources path parameter

  char* rec_par; // record parameter
  char* rep_par; // replay parameter

  view_t* rep_cur; // replay cursor
} zm = {0};

int main(int argc, char* argv[])
{
  const struct option long_options[] =
      {
          {"resources", optional_argument, 0, 'r'},
          {"record", optional_argument, 0, 's'},
          {"replay", optional_argument, 0, 'p'},
          {"config", optional_argument, 0, 'c'},
          {"frame", optional_argument, 0, 'f'},
          {0, 0, 0, 0},
      };

  int option       = 0;
  int option_index = 0;

  while ((option = getopt_long(argc, argv, "c:l:r:s:p:", long_options, &option_index)) != -1)
  {
    if (option != '?') printf("parsing option %c value: %s\n", option, optarg);
    if (option == 'c') zm.cfg_par = cstr_fromcstring(optarg); // REL 0
    if (option == 'r') zm.res_par = cstr_fromcstring(optarg); // REL 1
    if (option == 's') zm.rec_par = cstr_fromcstring(optarg); // REL 2
    if (option == 'p') zm.rep_par = cstr_fromcstring(optarg); // REL 3
    if (option == '?')
    {
      printf("zenmusic v210505 by Milan Toth\nCommand line options:\n");
      printf("-c --config= [config file] \t use config file for session\n");
      printf("-r --resources= [resources folder] \t use resources dir for session\n");
      printf("-s --record= [recorder file] \t record session to file\n");
      printf("-p --replay= [recorder file] \t replay session from file\n");
      printf("-f --frame= [widthxheight] \t initial window dimension\n");
    }
  }

  wm_init(init, update, render, destroy);
  return 0;
}

void init(int width, int height, char* path)
{
  srand((unsigned int)time(NULL));

  zm.lib_ch = ch_new(100); // comm channel for library entries
  zm.rem_ch = ch_new(10);  // remote channel

  db_init();
  config_init();
  player_init();
  visible_init();
  callbacks_init();
  if (zm.rec_par) evrec_init_recorder(zm.rec_par);
  if (zm.rep_par) evrec_init_player(zm.rep_par);

  // init callbacks

  callbacks_set("on_change_remote", cb_new(on_change_remote, NULL));
  callbacks_set("on_change_library", cb_new(on_change_library, NULL));
  callbacks_set("on_change_organize", cb_new(on_change_organize, NULL));

  // init paths

  char* wrk_path = cstr_path_normalize(path, NULL); // REL 0

#ifndef DEBUG
  char* res_path = zm.res_par ? cstr_path_normalize(zm.res_par, wrk_path) : cstr_fromcstring("/usr/local/share/zenmusic"); // REL 1
#else
  char* res_path = zm.res_par ? cstr_path_normalize(zm.res_par, wrk_path) : cstr_path_normalize("../res", wrk_path); // REL 1
#endif

  char* cfgdir_path = zm.cfg_par ? cstr_path_normalize(zm.cfg_par, wrk_path) : cstr_path_normalize("~/.config/zenmusic", getenv("HOME")); // REL 2
  char* css_path    = cstr_path_append(res_path, "main.css");                                                                             // REL 3
  char* html_path   = cstr_path_append(res_path, "main.html");                                                                            // REL 4
  char* font_path   = cstr_path_append(res_path, "Baloo.ttf");                                                                            // REL 5
  char* cfg_path    = cstr_path_append(cfgdir_path, "config.kvl");                                                                        // REL 6

  // print path info to console

  printf("working path     : %s\n", wrk_path);
  printf("config path   : %s\n", cfg_path);
  printf("resource path : %s\n", res_path);
  printf("css path      : %s\n", css_path);
  printf("html path     : %s\n", html_path);
  printf("font path     : %s\n", font_path);

  // init config

  config_set("remote_enabled", "false");
  config_set("remote_port", "23723");
  config_set("organize_lib", "false");
  config_set("dark_mode", "false");
  config_set("res_path", res_path);

  // read config, it overwrites defaults if exists

  config_read(cfg_path);

  // init non-configurable defaults

  config_set("wrk_path", wrk_path);
  config_set("cfg_path", cfg_path);
  config_set("css_path", css_path);
  config_set("html_path", html_path);
  config_set("font_path", font_path);

  // load ui from descriptors

  ui_load(width, height);

  // start listening for remote control events if set

  if (config_get("remote_enabled") && config_get_bool("remote_enabled")) remote_listen(zm.rem_ch, config_get_int("remote_port"));

  // show library popup if no lib path is saved yet or load library

  if (config_get("lib_path") == NULL)
    ui_lib_init_popup_show("Please enter the location of your music library folder.");
  else
    load_library();

  if (zm.rec_par) printf("***RECORDING SESSION***\n");

  // init cursor if replay

  if (zm.rep_par)
  {
    printf("***REPLAYING SESSION***\n");
    zm.rep_cur                          = view_new("rep_cur", ((r2_t){10, 10, 10, 10}));
    zm.rep_cur->exclude                 = 0;
    zm.rep_cur->layout.background_color = 0xFF0000FF;
    zm.rep_cur->needs_touch             = 0;
    tg_css_add(zm.rep_cur);
    ui_manager_add(zm.rep_cur);
  }

  // cleanup

  REL(res_path);
  REL(css_path);
  REL(html_path);
  REL(font_path);
}

void update(ev_t ev)
{
  if (ev.type == EV_TIME)
  {
    get_analyzed_songs();
    get_remote_events();
    update_player();

    if (zm.rep_par)
    {
      // get recorded events
      ev_t* recev;
      while ((recev = evrec_replay(ev.time)) != NULL)
      {
        ui_manager_event(*recev);
        view_set_frame(zm.rep_cur, (r2_t){recev->x, recev->y, 10, 10});
      }
    }
  }
  else
  {
    if (zm.rec_par)
    {
      // record all events besides time
      evrec_record(ev);
    }
  }

  // in case of replay only send time events
  if (!zm.rep_par || ev.type == EV_TIME) ui_manager_event(ev);

  /* if (zm.rep_par || zm.rec_par) */
  /* { */
  if (ev.type == EV_KDOWN && ev.keycode == SDLK_PRINTSCREEN) save_screenshot();
  /* } */
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
  /* ui_unload(); */
  /* callbacks_destroy(); */
  /* visible_destroy(); */
  /* player_destroy(); */
  /* config_destroy(); */
  /* db_destroy(); */
}

void load_library()
{
  assert(config_get("lib_path") != NULL);

  db_reset();
  db_read(config_get("lib_path"));

  map_t* files = MNEW();                         // REL 0
  lib_read_files(config_get("lib_path"), files); // read all files under library path
  db_update(files);                              // remove deleted files from db, remove existing files from files

  if (files->count > 0)
  {
    LOG("new files detected : %i", files->count);
    lib_analyze_files(zm.lib_ch, files); // start analyzing new entries
  }

  visible_set_sortfield("meta/artist", 0);

  REL(files); // REL 0
}

void on_change_remote(void* userdata, void* data)
{
  if (config_get_bool("remote_enabled"))
    remote_listen(zm.rem_ch, config_get_int("remote_port"));
  else
    remote_close();
}

void on_change_library(void* userdata, void* data)
{
  char* new_path = data;
  char* lib_path = NULL;

  // construct path if needed

  lib_path = cstr_path_normalize(new_path, config_get("wrk_path"));

  printf("new path %s lib path %s\n", new_path, lib_path);

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
  if (config_get_bool("organize_lib"))
  {
    int succ = db_organize(config_get("lib_path"), db_get_db());
    if (succ == 0) db_write(config_get("lib_path"));
    ui_songlist_refresh();
  }
}

void get_analyzed_songs()
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

      if (config_get_bool("organize_lib"))
      {
        // organize db if needed

        int succ = db_organize(config_get("lib_path"), db_get_db());
        if (succ == 0) db_write(config_get("lib_path"));
      }

      visible_set_sortfield("meta/artist", 0);
      ui_songlist_refresh();
    }

    // cleanup, ownership was passed with the channel from analyzer

    REL(entry);
  }
}

void get_remote_events()
{
  char* buffer = NULL;
  if ((buffer = ch_recv(zm.rem_ch)))
  {
    if (buffer[0] == '0') ui_play_pause();
    if (buffer[0] == '1') ui_play_prev();
    if (buffer[0] == '2') ui_play_next();
  }
}

void update_player()
{
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

void save_screenshot()
{
  static int cnt    = 0;
  view_t*    root   = ui_manager_get_root();
  r2_t       frame  = root->frame.local;
  bm_t*      screen = bm_new(frame.w, frame.h); // REL 0
  ui_compositor_render_to_bmp(screen);
  char* name = cstr_fromformat(20, "screenshot%.3i.png", cnt++); // REL 1
  char* path = cstr_path_append(config_get("lib_path"), name);   // REL 2
  coder_write_png(path, screen);
  REL(screen); // REL 0
  REL(name);   // REL 1
  REL(path);   // REL 2
}
