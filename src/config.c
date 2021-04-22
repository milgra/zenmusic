#ifndef config_h
#define config_h

#include "mtmap.c"

void  config_init(char* respath);
void  config_read();
void  config_write();
void  config_set(char* key, char* value);
char* config_get(char* key);
int   config_get_bool(char* key);
void  config_set_bool(char* key, int val);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "library.c"
#include "mtcstring.c"
#include "mtlog.c"
#include <limits.h>

struct _cfg_t
{
  map_t* data;
} cfg = {0};

void config_init(char* respath)
{
  cfg.data = MNEW();

  MPUT(cfg.data, "organize_db", cstr_fromcstring("false"));
  MPUT(cfg.data, "ui_color", cstr_fromcstring("0xEEEEEEFF"));

#ifndef DEBUG
  respath = "/usr/local/share/zenmusic";
#else
  respath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/../res", respath);
#endif

  // TODO MPUTVAL
  // TODO LEAK!!!
  MPUT(cfg.data, "respath", cstr_fromcstring(respath));

  // we won't release values this time, config map will stay in memory during runtime
}

// TODO rework kvlist, entries should contain a key field which is a hierarchical path so multi-level maps can be stored
void config_read()
{
  char* path = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/.config/zenmusic/config.kvl", getenv("HOME"));

  map_t* container = MNEW();
  kvlist_read(path, container, "id");

  if (container->count > 0)
  {
    map_t* cfdb = MGET(container, "config");
    vec_t* keys = VNEW();
    map_keys(cfdb, keys);

    for (int index = 0; index < keys->length; index++)
    {
      char* key = keys->data[index];
      MPUT(cfg.data, key, MGET(cfdb, key));
    }
  }

  printf("config loaded from %s, entries : %i\n", path, cfg.data->count);
  REL(path);
  REL(container);
}

void config_write()
{
  printf("config_write\n");

  map_t* container = MNEW();
  MPUT(cfg.data, "id", cstr_fromcstring("config"));
  MPUT(container, "id", cfg.data);

  char* dirpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/.config/zenmusic/", getenv("HOME"));
  char* cfgpath = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/.config/zenmusic/config.kvl", getenv("HOME"));

  // TODO move mkpath to some other namespace
  int error = lib_mkpath(dirpath, 0777);

  if (error == 0)
  {
    int res = kvlist_write(cfgpath, container);
    if (res < 0) LOG("ERROR config_write cannot write config\n");
  }
  else
    LOG("ERROR config_write cannot create config path\n");

  REL(dirpath);
  REL(cfgpath);
  REL(container);
}

void config_set(char* key, char* value)
{
  MPUT(cfg.data, key, value);
}

char* config_get(char* key)
{
  return MGET(cfg.data, key);
}

int config_get_bool(char* key)
{
  char* val = MGET(cfg.data, key);
  if (val && strcmp(val, "true"))
    return 1;
  else
    return 0;
}

void config_set_bool(char* key, int val)
{
  if (val)
  {
    char* str = cstr_fromcstring("true");
    MPUT(cfg.data, key, str);
  }
  else
  {
    char* str = cstr_fromcstring("false");
    MPUT(cfg.data, key, str);
  }
}

#endif
