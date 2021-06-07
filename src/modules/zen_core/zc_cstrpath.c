#ifndef zc_cstrpath_h
#define zc_cstrpath_h

#include "zc_cstring.c"

char* cstr_path_append(char* root, char* component);
char* cstr_remove_last_path_component(char* path);
char* cstr_path_extension(char* path);
char* cstr_path_filename(char* path);
char* cstr_path_normalize(char* path, char* execpath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_memory.c"
#include <limits.h>
#include <string.h>

char* cstr_path_append(char* root, char* component)
{
  if (root[strlen(root) - 1] == '/')
    return cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", root, component);
  else
    return cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s", root, component);
}

char* cstr_remove_last_path_component(char* path)
{
  int index;
  for (index = strlen(path) - 1; index > -1; --index)
  {
    if (path[index] == '/')
    {
      index++;
      break;
    }
  }
  char* str = mem_calloc(index + 1, "char*", NULL, NULL);
  memcpy(str, path, index);
  return str;
}

char* cstr_path_extension(char* path)
{
  int index;
  for (index = strlen(path) - 1; index > -1; --index)
  {
    if (path[index] == '.')
    {
      index++;
      break;
    }
  }

  int   len = strlen(path) - index;
  char* ext = mem_calloc(len + 1, "char*", NULL, NULL);
  memcpy(ext, path + index, len);
  return ext;
}

char* cstr_path_filename(char* path)
{
  int dotindex;
  for (dotindex = strlen(path) - 1; dotindex > -1; --dotindex)
  {
    if (path[dotindex] == '.') break;
  }

  if (dotindex == -1) dotindex = strlen(path) - 1;

  int slashindex;
  for (slashindex = strlen(path) - 1; slashindex > -1; --slashindex)
  {
    if (path[slashindex] == '/')
    {
      slashindex++;
      break;
    }
  }
  if (slashindex == -1) slashindex = 0;
  int   len   = dotindex - slashindex;
  char* title = mem_calloc(len + 1, "char*", NULL, NULL);
  memcpy(title, path + slashindex, len);
  return title;
}

char* cstr_path_normalize(char* path, char* execpath)
{
  char* result = NULL;

  if (path[0] == '~') // if starts with tilde, insert home dir
    result = cstr_fromformat(PATH_MAX + NAME_MAX, "%s%s", getenv("HOME"), path + 1);
  else if (path[0] != '/') // if doesn't start with / insert base dir
    result = cstr_fromformat(PATH_MAX + NAME_MAX, "%s/%s", execpath, path);
  else
    result = cstr_fromcstring(path);

  // if ends with '/' remove it
  if (result[strlen(result) - 1] == '/') result[strlen(result) - 1] = '\0';

  return result;
}

#endif
