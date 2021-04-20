#ifndef kvlist_h
#define kvlist_h

#include "mtmap.c"
#include <stdio.h>

int kvlist_read(char* libpath, map_t* db, char* keyfield);
int kvlist_write(char* libpath, map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include <limits.h>

int kvlist_read(char* libpath, map_t* db, char* keyfield)
{
  printf("kvlist read libpath %s keyfield %s\n", libpath, keyfield);

  int   retv  = -1;
  char* dbstr = cstr_fromfile(libpath);

  if (dbstr)
  {
    retv = 0;

    char*  token = strtok(dbstr, "\n");
    char*  key   = NULL;
    map_t* map   = MNEW();

    while (token)
    {
      if (key)
      {
        char* val = cstr_fromcstring(token);
        MPUT(map, key, val);
        REL(key);
        REL(val);
        key = NULL;
      }
      else
      {
        if (token[0] == '-')
        {
          key = MGET(map, keyfield);
          MPUT(db, key, map);
          REL(map);
          map = MNEW();
          key = NULL;
        }
        else
          key = cstr_fromcstring(token);
      }
      token = strtok(NULL, "\n");
    }

    REL(dbstr);
  }
  else
    printf("ERROR kvlist_read cannot read file %s\n", libpath);

  return retv;
}

int kvlist_write(char* libpath, map_t* db)
{
  printf("kvlist write %s\n", libpath);

  int   retv = -1;
  char* path = cstr_fromformat(PATH_MAX + NAME_MAX, "%snew", libpath);
  FILE* file = fopen(path, "w");

  if (file)
  {
    retv        = 0;
    vec_t* vals = VNEW();
    map_values(db, vals);

    for (int vali = 0; vali < vals->length; vali++)
    {
      map_t* entry = vals->data[vali];
      vec_t* keys  = VNEW();

      map_keys(entry, keys);

      for (int keyi = 0; keyi < keys->length; keyi++)
      {
        char* key = keys->data[keyi];
        char* val = MGET(entry, key);

        if (fprintf(file, "%s\n", key) < 0) retv = -1;
        if (fprintf(file, "%s\n", val) < 0) retv = -1;
      }

      if (fprintf(file, "-\n") < 0) retv = -1;

      REL(keys);

      if (retv < 0) break;
    }

    if (fclose(file) == EOF) retv = -1;

    REL(vals);

    if (retv == 0)
    {
      if (rename(path, libpath) != 0) retv = -1;
    }
    else
      printf("ERROR kvlist_write cannot write file\n");
  }
  else
    printf("ERROR kvlist_write cannot open file %s\n", path);

  REL(path);

  return retv;
}

#endif
