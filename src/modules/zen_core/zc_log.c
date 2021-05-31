#ifndef zc_log_h
#define zc_log_h

#define LOG(...) log_log(__VA_ARGS__)

void log_log(char* fmt, ...);
void log_set_proxy(void (*proxy)(char*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_vector.c"

#define LOG_SIZE 150

struct _zc_log_t
{
  void (*proxy)(char*);
} zc_log = {0};

void log_set_proxy(void (*proxy)(char*))
{
  zc_log.proxy = proxy;
}

void log_log(char* fmt, ...)
{
  va_list arglist;
  char*   str = mem_calloc(LOG_SIZE, "char*", NULL, NULL);
  va_start(arglist, fmt);
  vsnprintf(str, LOG_SIZE, fmt, arglist);
  va_end(arglist);

  printf("LOG %s\n", str);

  if (zc_log.proxy != NULL) (*zc_log.proxy)(str);

  REL(str);
}

#endif
