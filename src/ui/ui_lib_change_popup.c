#ifndef ui_lib_change_popup_h
#define ui_lib_change_popup_h

#include "view.c"

void ui_lib_change_popup_init();
void ui_lib_change_popup_attach(view_t* baseview, char* fontpath);
void ui_lib_change_popup_show();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "text.c"
#include "tg_text.c"

struct _ui_lib_change_popup_t
{
  char* id;
} ulcp = {0};

void ui_lib_change_popup_init()
{
}
void ui_lib_change_popup_attach(view_t* baseview, char* fontpath)
{
}
void ui_lib_change_popup_show()
{
}

#endif
