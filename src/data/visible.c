#ifndef visible_h
#define visible_h

#include "mtmap.c"

void visible_init();

map_t* visible_song_at_index(int index);
int    visible_song_count();

vec_t* visible_get_songs();
vec_t* visible_get_genres();
vec_t* visible_get_artists();

void visible_set_sortfield(char* field, int toggle);
void visible_set_filter(char* text);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "database.c"
#include "mtcstring.c"

struct _visible_t
{
  vec_t* songs;
  vec_t* genres;
  vec_t* artists;

  char*   filter;
  char*   sort_field;
  vsdir_t sort_dir;

  vec_t* tmp1;
  vec_t* tmp2;

} vis = {0};

void visible_init()
{
  vis.songs   = VNEW();
  vis.genres  = VNEW();
  vis.artists = VNEW();

  vis.tmp1 = VNEW();
  vis.tmp2 = VNEW();

  vis.filter     = NULL;
  vis.sort_field = "meta/artist";
}

map_t* visible_song_at_index(int index)
{
  return vis.songs->data[index];
}

int visible_song_count()
{
  return vis.songs->length;
}

vec_t* visible_get_songs()
{
  return vis.songs;
}

vec_t* visible_get_genres()
{
  return vis.genres;
}

vec_t* visible_get_artists()
{
  return vis.artists;
}

int visible_comp_entry(void* left, void* right)
{
  map_t* l = left;
  map_t* r = right;

  char* la = MGET(l, vis.sort_field);
  char* ra = MGET(r, vis.sort_field);

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

int visible_comp_text(void* left, void* right)
{
  char* la = left;
  char* ra = right;

  return strcmp(la, ra);
}

void visible_gen_genres()
{
  int ei, gi; // entry, genre index

  vec_reset(vis.genres);

  for (ei = 0;
       ei < vis.songs->length;
       ei++)
  {
    map_t* entry = vis.songs->data[ei];
    char*  genre = MGET(entry, "meta/genre");

    if (genre)
    {
      char found = 0;
      for (gi = 0; gi < vis.genres->length; gi++)
      {
        char* act_genre = vis.genres->data[gi];
        if (strcmp(genre, act_genre) == 0)
        {
          found = 1;
          break;
        }
      }
      if (!found) VADD(vis.genres, genre);
    }
  }
}

void visible_gen_artists()
{
  int ei;

  vec_reset(vis.artists);

  map_t* artists = MNEW();

  for (ei = 0;
       ei < vis.songs->length;
       ei++)
  {
    map_t* entry  = vis.songs->data[ei];
    char*  artist = MGET(entry, "meta/artist");

    if (artist) MPUT(artists, artist, artist);
  }

  map_values(artists, vis.artists);
  vec_sort(vis.artists, VSD_ASC, visible_comp_text);

  REL(artists);
}

int visible_nextword(char* text, char* part)
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

map_t* visible_query_fields(char* text)
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
      if (visible_nextword(text + i, " is") && key == NULL)
      {
        key = mem_calloc(i - last + 1, "char", NULL, NULL);
        memcpy(key, text + last, i - last);
        i += 4;
        last = i;
        printf("key %s\n", key);
      }

      if (visible_nextword(text + i, " and") && val == NULL)
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

void visible_filter()
{
  int ei, ki; // entry, key index

  map_t* fields = visible_query_fields(vis.filter);
  char*  value  = NULL;
  char*  query  = NULL;

  vec_reset(vis.songs);

  vec_reset(vis.tmp1);
  vec_reset(vis.tmp2);

  map_values(db_get_db(), vis.tmp1);

  for (ei = 0;
       ei < vis.tmp1->length;
       ei++)
  {
    map_t* entry = vis.tmp1->data[ei];
    vec_reset(vis.tmp2);

    if (fields)
    {
      // use query fields
      map_keys(fields, vis.tmp2);
    }
    else
    {
      // use all fields
      map_keys(entry, vis.tmp2);
    }

    for (ki = 0;
         ki < vis.tmp2->length;
         ki++)
    {
      char* field = vis.tmp2->data[ki];
      value       = MGET(entry, field);

      if (value)
      {
        if (fields)
          query = MGET(fields, field);
        else
          query = vis.filter;

        if (strstr(value, query))
        {
          vec_add(vis.songs, entry);
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

  visible_gen_genres();
  visible_gen_artists();
}

void visible_update()
{
  if (vis.filter)
    visible_filter();
  else
  {
    vec_reset(vis.songs);
    map_values(db_get_db(), vis.songs);
    visible_gen_genres();
    visible_gen_artists();
  }

  vec_sort(vis.songs, vis.sort_dir, visible_comp_entry);
}

void visible_set_sortfield(char* field, int toggle)
{
  if (field != NULL)
  {
    if (toggle == 1 && vis.sort_field != NULL && strcmp(field, vis.sort_field) == 0)
    {
      if (vis.sort_dir == VSD_ASC)
        vis.sort_dir = VSD_DSC;
      else
        vis.sort_dir = VSD_ASC;
    }
    else
    {
      vis.sort_dir = VSD_ASC;
    }

    vis.sort_field = field;
    visible_update();
  }
}

void visible_set_filter(char* text)
{
  vis.filter = text;
  visible_update();
}

#endif
