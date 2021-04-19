// stores filteres and sorted song entries for use for songlist

#ifndef filtered_h
#define filtered_h

#include "mtmap.c"

void filtered_init();

map_t* filtered_song_at_index(int index);
int    filtered_song_count();

vec_t* filtered_get_songs();
vec_t* filtered_get_genres();
vec_t* filtered_get_artists();

void filtered_set_sortfield(char* field, int toggle);
void filtered_set_filter(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "database.c"
#include "mtcstring.c"

struct _filtered_t
{
  vec_t* songs;
  vec_t* genres;
  vec_t* artists;

  char*   filter;
  char*   sort_field;
  vsdir_t sort_dir;

  vec_t* tmp1;
  vec_t* tmp2;

} flt = {0};

void filtered_init()
{
  flt.songs   = VNEW();
  flt.genres  = VNEW();
  flt.artists = VNEW();

  flt.tmp1 = VNEW();
  flt.tmp2 = VNEW();

  flt.filter     = NULL;
  flt.sort_field = "meta/artist";
}

map_t* filtered_song_at_index(int index)
{
  return flt.songs->data[index];
}

int filtered_song_count()
{
  return flt.songs->length;
}

vec_t* filtered_get_songs()
{
  return flt.songs;
}

vec_t* filtered_get_genres()
{
  return flt.genres;
}

vec_t* filtered_get_artists()
{
  return flt.artists;
}

int filtered_comp_entry(void* left, void* right)
{
  map_t* l = left;
  map_t* r = right;

  char* la = MGET(l, flt.sort_field);
  char* ra = MGET(r, flt.sort_field);

  if (strcmp(la, ra) == 0)
  {
    // todo make this controllable from header fields
    char* nla = MGET(l, "meta/album");
    char* nra = MGET(r, "meta/album");

    if (nla && nra)
    {
      if (strcmp(nla, nra) == 0)
      {
        nla = MGET(l, "meta/track");
        nra = MGET(r, "meta/track");

        if (nla && nra)
        {
          la = nla;
          ra = nra;
        }
      }
      else
      {
        la = nla;
        ra = nra;
      }
    }
  }

  return strcmp(la, ra);
}

int filtered_comp_text(void* left, void* right)
{
  char* la = left;
  char* ra = right;

  return strcmp(la, ra);
}

void filtered_gen_genres()
{
  int ei, gi; // entry, genre index

  vec_reset(flt.genres);

  for (ei = 0;
       ei < flt.songs->length;
       ei++)
  {
    map_t* entry = flt.songs->data[ei];
    char*  genre = MGET(entry, "meta/genre");

    if (genre)
    {
      char found = 0;
      for (gi = 0; gi < flt.genres->length; gi++)
      {
        char* act_genre = flt.genres->data[gi];
        if (strcmp(genre, act_genre) == 0)
        {
          found = 1;
          break;
        }
      }
      if (!found) VADD(flt.genres, genre);
    }
  }
}

void filtered_gen_artists()
{
  int ei;

  vec_reset(flt.artists);

  map_t* artists = MNEW();

  for (ei = 0;
       ei < flt.songs->length;
       ei++)
  {
    map_t* entry  = flt.songs->data[ei];
    char*  artist = MGET(entry, "meta/artist");

    if (artist) MPUT(artists, artist, artist);
  }

  map_values(artists, flt.artists);
  vec_sort(flt.artists, VSD_ASC, filtered_comp_text);

  REL(artists);
}

int filtered_nextword(char* text, char* part)
{
  if (strlen(part) < strlen(text))
  {
    for (int i = 0; i < strlen(part); i++)
    {
      if (text[i] != part[i]) return 0;
    }
    return 1;
  }
  return 0;
}

map_t* filtered_query_fields(char* text)
{
  if (strstr(text, " "))
  {
    map_t* fields = MNEW();
    char*  key    = NULL;
    char*  val    = NULL;
    int    last   = 0;
    int    i      = 0;

    for (i = 0; i < strlen(text); i++)
    {
      if (filtered_nextword(text + i, " is") && key == NULL)
      {
        key = mem_calloc(i - last + 1, "char", NULL, NULL);
        memcpy(key, text + last, i - last);
        i += 4;
        last = i;
        printf("key %s\n", key);
      }

      if (filtered_nextword(text + i, " and") && val == NULL)
      {
        val = mem_calloc(i - last + 1, "char", NULL, NULL);
        memcpy(val, text + last, i - last);
        i += 5;
        last = i;
        printf("val %s\n", val);

        cstr_tolower(key);
        MPUT(fields, key, val);
        REL(key);
        REL(val);
        key = NULL;
        val = NULL;
      }
    }
    if (last < i && val == NULL && key != NULL)
    {
      val = mem_calloc(i - last, "char", NULL, NULL);
      memcpy(val, text + last, i - last);
      printf("val %s\n", val);

      cstr_tolower(key);
      MPUT(fields, key, val);
      REL(key);
      REL(val);
      key = NULL;
      val = NULL;
    }

    return fields;
  }
  return NULL;
}

void filtered_filter()
{
  int ei, ki; // entry, key index

  map_t* fields = filtered_query_fields(flt.filter);
  char*  value  = NULL;
  char*  query  = NULL;

  vec_reset(flt.songs);

  vec_reset(flt.tmp1);
  vec_reset(flt.tmp2);

  map_values(db_get_db(), flt.tmp1);

  for (ei = 0;
       ei < flt.tmp1->length;
       ei++)
  {
    map_t* entry = flt.tmp1->data[ei];
    vec_reset(flt.tmp2);

    if (fields)
    {
      // use query fields
      map_keys(fields, flt.tmp2);
    }
    else
    {
      // use all fields
      map_keys(entry, flt.tmp2);
    }

    for (ki = 0;
         ki < flt.tmp2->length;
         ki++)
    {
      char* field = flt.tmp2->data[ki];
      value       = MGET(entry, field);

      if (value)
      {
        if (fields)
          query = MGET(fields, field);
        else
          query = flt.filter;

        if (strstr(value, query))
        {
          vec_add(flt.songs, entry);
          break;
        }
      }
      else
      {
        printf("NO %s field in entry :\n", field);
        mem_describe(entry, 0);
        printf("\n");
      }
    }
  }

  if (fields) REL(fields);

  filtered_gen_genres();
  filtered_gen_artists();
}

void filtered_update()
{
  if (flt.filter)
    filtered_filter();
  else
  {
    vec_reset(flt.songs);
    map_values(db_get_db(), flt.songs);
    filtered_gen_genres();
    filtered_gen_artists();
  }

  vec_sort(flt.songs, flt.sort_dir, filtered_comp_entry);
}

void filtered_set_sortfield(char* field, int toggle)
{
  if (field != NULL)
  {
    if (toggle == 1 && flt.sort_field != NULL && strcmp(field, flt.sort_field) == 0)
    {
      if (flt.sort_dir == VSD_ASC)
        flt.sort_dir = VSD_DSC;
      else
        flt.sort_dir = VSD_ASC;
    }
    else
    {
      flt.sort_dir = VSD_ASC;
    }

    flt.sort_field = field;
    filtered_update();
  }
}

void filtered_set_filter(char* text)
{
  flt.filter = text;
  filtered_update();
}

#endif
