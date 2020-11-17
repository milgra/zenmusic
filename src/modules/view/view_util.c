#ifndef view_util_h
#define view_util_h

#include "view.c"

view_t* view_get_subview(view_t* view, char* id);

#endif

#if __INCLUDE_LEVEL__ == 0

view_t* view_get_subview(view_t* view, char* id)
{
  if (strcmp(view->id, id) == 0) return view;
  for (int i = 0; i < view->views->length; i++)
  {
    view_t* sv = view->views->data[i];
    view_t* re = view_get_subview(sv, id);
    if (re) return re;
  }
  return NULL;
}

#endif
