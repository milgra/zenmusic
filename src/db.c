#ifndef db_h
#define db_h

#include "mtmap.c"

void db_read(mtmap_t* db);
void db_write(mtmap_t* db);
void db_sort(mtmap_t* db, mtvec_t* vec);
void db_filter(mtmap_t* db, char* text, mtvec_t* vec);

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
  mtvec_t* vals = VNEW();
  mtmap_values(db, vals);

  FILE* f = fopen("zenmusic", "w");

  for (int index = 0; index < vals->length; index++)
  {
    mtmap_t* entry = vals->data[index];
    mtvec_t* keys  = VNEW();
    mtmap_keys(entry, keys);
    char* key;
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

int db_comp_artist(void* left, void* right)
{
  mtmap_t* l = left;
  mtmap_t* r = right;

  char* la = MGET(l, "artist");
  char* ra = MGET(r, "artist");

  return strcmp(la, ra);
}

mtvec_t* vec1 = NULL;
mtvec_t* vec2 = NULL;

void db_filter(mtmap_t* db, char* text, mtvec_t* res)
{
  int ei, vi; // entry, value index

  if (!vec1) vec1 = VNEW();
  if (!vec2) vec2 = VNEW();

  mtvec_reset(res);
  mtvec_reset(vec1);
  mtvec_reset(vec2);

  mtmap_values(db, vec1);

  for (ei = 0;
       ei < vec1->length;
       ei++)
  {
    mtmap_t* entry = vec1->data[ei];
    mtvec_reset(vec2);
    mtmap_values(entry, vec2);

    for (vi = 0;
         vi < vec2->length;
         vi++)
    {
      char* val = vec2->data[vi];
      if (strstr(val, text))
      {
        mtvec_add(res, entry);
        break;
      }
    }
  }
  mtvec_sort(res, db_comp_artist);
}

void db_sort(mtmap_t* db, mtvec_t* vec)
{
  mtvec_reset(vec);
  mtmap_values(db, vec);
  mtvec_sort(vec, db_comp_artist);
}

#endif
