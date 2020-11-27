#ifndef db_h
#define db_h

#include "mtmap.c"

void db_read(mtmap_t* db);
void db_write(mtmap_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"

void db_read(mtmap_t* db)
{
  char* dbstr = mtcstr_fromfile("zenmusic");

  if (dbstr)
  {
    char*    token = strtok(dbstr, "\n");
    char*    key   = NULL;
    mtmap_t* map   = MNEW();

    while (token)
    {
      if (key)
      {
        char* val = mtcstr_fromcstring(token);
        MPUT(map, key, val);
        REL(key);
        REL(val);
        key = NULL;
      }
      else
      {
        if (token[0] == '-')
        {
          char* path = MGET(map, "path");
          MPUT(db, path, map);
          REL(map);
          map = MNEW();
        }
        else
          key = mtcstr_fromcstring(token);
      }
      token = strtok(NULL, "\n");
    }

    printf("LOG db loaded\n");
  }
  else
  {
    printf("LOG No db found.\n");
  }
}

void db_write(mtmap_t* db)
{
  mtvec_t* vals = mtmap_values(db);

  FILE* f = fopen("zenmusic", "w");

  for (int index = 0; index < vals->length; index++)
  {
    mtmap_t* entry = vals->data[index];
    mtvec_t* keys  = mtmap_keys(entry);
    char*    key;
    while ((key = VNXT(keys)))
    {
      char* val = MGET(entry, key);
      fprintf(f, "%s\n", key);
      fprintf(f, "%s\n", val);
    }
    fprintf(f, "-\n");
    REL(keys);
  }

  fclose(f);
  REL(vals);

  printf("LOG db saved");
}

#endif
