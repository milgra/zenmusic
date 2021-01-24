#ifndef config_h
#define config_h

#include "mtmap.c"

void  config_init();
void  config_read();
void  config_write();
void  config_set(char* key, char* value);
char* config_get(char* key);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "lib.c"
#include "mtcstring.c"
#include "mtlog.c"

struct _cfg_t
{
  map_t* data;
} cfg = {0};

void config_init()
{
  printf("config_init\n");

  cfg.data = MNEW();

  MPUT(cfg.data, "organize_db", cstr_fromcstring("false"));
  MPUT(cfg.data, "ui_color", cstr_fromcstring("0xEEEEEEFF"));

  // we won't release values this time, config map will stay in memory during runtime
}

// TODO rework kvlist, entries should contain a key field which is a hierarchical path so multi-level maps can be stored
void config_read()
{
  char* path = cstr_fromformat("%s/.config/zenmusic/config.kvl", getenv("HOME"), NULL);

  map_t* container = MNEW();
  kvlist_read(path, container, "id");

  printf("container\n");
  mem_describe(container, 0);

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

  char* dirpath = cstr_fromformat("%s/.config/zenmusic/", getenv("HOME"), NULL);
  char* cfgpath = cstr_fromformat("%s/.config/zenmusic/config.kvl", getenv("HOME"), NULL);

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

#endif
