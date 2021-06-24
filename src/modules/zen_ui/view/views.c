#ifndef views_h
#define views_h

#include "zc_map.c"

typedef struct _views_t
{
  vec_t* list;
  int    arrange;
} views_t;

extern views_t views;

void views_init();
void views_destroy();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "view.c"

views_t views = {0};

void views_init()
{
  views.list    = VNEW();
  views.arrange = 0;
}

void views_destroy()
{
  // flatten view
  for (int index = 0; index < views.list->length; index++)
  {
    view_t* view = views.list->data[index];
    view_remove_from_parent(view);
  }
  // release list
  REL(views.list);
  views.list = NULL;
}

#endif
