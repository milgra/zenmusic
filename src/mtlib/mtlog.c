#ifndef log_h
#define log_h

#define LOG(...) log_log(__VA_ARGS__)

void log_log(char* fmt, ...);
void log_set_proxy(void (*proxy)(char*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtvector.c"

struct _mtlog_t
{
  void (*proxy)(char*);
} mtlog = {0};

void log_set_proxy(void (*proxy)(char*))
{
  mtlog.proxy = proxy;
}

void log_log(char* fmt, ...)
{
  va_list arglist;
  char*   str = mem_calloc(100, "char*", NULL, NULL);
  va_start(arglist, fmt);
  vsnprintf(str, 150, fmt, arglist);
  va_end(arglist);

  printf("LOG %s\n", str);

  if (mtlog.proxy != NULL) (*mtlog.proxy)(str);

  REL(str);
}

#endif
