#ifndef config_h
#define config_h

#include "mtmap.c"

void  config_init();
void  config_read(char* path);
void  config_write(char* path);
void  config_set(char* key, char* value);
char* config_get(char* key);
int   config_get_int(char* key);
int   config_get_bool(char* key);
void  config_set_bool(char* key, int val);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "files.c"
#include "kvlist.c"
#include "library.c"
#include "mtcstring.c"
#include "mtcstrpath.c"
#include "mtlog.c"
#include <limits.h>

map_t* confmap;

void config_init()
{
  confmap = MNEW();
}

void config_read(char* path)
{
  map_t* data = MNEW();

  kvlist_read(path, data, "id");

  map_t* cfdb = MGET(data, "config");

  if (cfdb)
  {
    vec_t* keys = VNEW();
    map_keys(cfdb, keys);

    for (int index = 0; index < keys->length; index++)
    {
      char* key = keys->data[index];
      MPUT(confmap, key, MGET(cfdb, key));
    }

    printf("config loaded from %s, entries : %i\n", path, confmap->count);
  }

  REL(path);
  REL(data);
}

void config_write(char* path)
{
  map_t* data    = MNEW();
  char*  dirpath = cstr_remove_last_path_component(path);

  printf("CONFIG DIR %s FILE %s\n", dirpath, path);

  MPUT(confmap, "id", cstr_fromcstring("config")); // put id in config db
  MPUT(data, "id", confmap);                       // put config db in final data with same id

  int error = files_mkpath(dirpath, 0777);

  if (error == 0)
  {
    int res = kvlist_write(path, data);
    if (res < 0)
      LOG("ERROR config_write cannot write config\n");
    else
      LOG("config written");
  }
  else
    LOG("ERROR config_write cannot create config path\n");

  REL(dirpath);
  REL(data);
}

void config_set(char* key, char* value)
{
  char* str = cstr_fromcstring(value);
  MPUT(confmap, key, str);
  REL(str);
}

char* config_get(char* key)
{
  return MGET(confmap, key);
}

int config_get_bool(char* key)
{
  char* val = MGET(confmap, key);
  if (val && strcmp(val, "true") == 0)
    return 1;
  else
    return 0;
}

int config_get_int(char* key)
{
  char* val = MGET(confmap, key);
  if (val)
    return atoi(val);
  else
    return 0;
}

void config_set_bool(char* key, int val)
{
  if (val)
  {
    char* str = cstr_fromcstring("true");
    MPUT(confmap, key, str);
  }
  else
  {
    char* str = cstr_fromcstring("false");
    MPUT(confmap, key, str);
  }
}

#endif
