#ifndef ui_popup_switcher_h
#define ui_popup_switcher_h

#include "view.c"

void ui_popup_switcher_init();
void ui_popup_switcher_attach(view_t* baseview);
void ui_popup_switcher_toggle(char* id);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "vh_anim.c"
#include "vh_touch.c"

struct _ui_popup_switcher_t
{
  view_t* baseview;
  vec_t*  popup_names;
  map_t*  popup_views;
} ups = {0};

void ui_popup_switcher_remove(view_t* view, void* userdata);
void ui_popup_switcher_on_button_down(void* userdata, void* data);

void ui_popup_switcher_init()
{
}

void ui_popup_switcher_attach(view_t* baseview)
{
  ups.baseview    = baseview;
  ups.popup_names = VNEW();
  ups.popup_views = MNEW();

  VADDR(ups.popup_names, cstr_fromcstring("lib_change_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("song_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("messages_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("ideditor_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("about_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("filters_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("settings_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("decision_popup_page"));
  VADDR(ups.popup_names, cstr_fromcstring("simple_popup_page"));

  for (int index = 0; index < ups.popup_names->length; index++)
  {
    char* name = ups.popup_names->data[index];

    view_t* page_view  = view_get_subview(ups.baseview, name);
    view_t* btn_view   = page_view->views->data[0];
    view_t* popup_view = btn_view->views->data[0];

    vh_anim_add(page_view);
    vh_anim_set_event(page_view, page_view, ui_popup_switcher_remove);
    vh_touch_add(btn_view, cb_new(ui_popup_switcher_on_button_down, NULL));

    view_remove(ups.baseview, page_view);

    MPUT(ups.popup_views, name, page_view);
  }
}

void ui_popup_switcher_remove(view_t* view, void* userdata)
{
  if (view->texture.alpha < 1.0) view_remove(ups.baseview, view);
}

void ui_popup_switcher_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "song_popup_page_btn") == 0) ui_popup_switcher_toggle("song_popup_page");
  if (strcmp(id, "messages_popup_page_btn") == 0) ui_popup_switcher_toggle("messages_popup_page");
  if (strcmp(id, "ideditor_popup_page_btn") == 0) ui_popup_switcher_toggle("ideditor_popup_page");
  if (strcmp(id, "about_popup_page_btn") == 0) ui_popup_switcher_toggle("about_popup_page");
  if (strcmp(id, "simple_popup_page_btn") == 0) ui_popup_switcher_toggle("simple_popup_page");
  if (strcmp(id, "settings_popup_page_btn") == 0) ui_popup_switcher_toggle("settings_popup_page");
  if (strcmp(id, "filters_popup_page_btn") == 0) ui_popup_switcher_toggle("filters_popup_page");
  if (strcmp(id, "decision_popup_page_btn") == 0) ui_popup_switcher_toggle("decision_popup_page");
  if (strcmp(id, "library_popup_page_btn") == 0) ui_popup_switcher_toggle("library_popup_page");
}

void ui_popup_switcher_toggle_baseview(view_t* view)
{
  if (view->parent)
  {
    view->texture.alpha = 1.0;
    vh_anim_alpha(view, 1.0, 0.0, 20, AT_LINEAR);
  }
  else
  {
    r2_t basef = ups.baseview->frame.local;

    view_t* btnview = view->views->data[0];
    view_t* popview = btnview->views->data[0];

    r2_t viewf = popview->frame.local;
    viewf.x    = (basef.w - viewf.w) / 2;
    viewf.y    = (basef.h - viewf.h) / 2;
    view_set_frame(popview, viewf);

    view_add(ups.baseview, view);
    view->texture.alpha = 0.0;
    vh_anim_alpha(view, 0.0, 1.0, 20, AT_LINEAR);
  }
}

void ui_popup_switcher_toggle(char* id)
{
  view_t* view = MGET(ups.popup_views, id);
  if (view) ui_popup_switcher_toggle_baseview(view);
}

#endif
