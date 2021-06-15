#ifndef ui_inputfield_popup_h
#define ui_inputfield_popup_h

#include "view.c"
#include "zc_callback.c"

void ui_inputfield_popup_attach(view_t* baseview);
void ui_inputfield_popup_show(char* text, cb_t* acc_cb, cb_t* rej_cb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "tg_text.c"
#include "ui_manager.c"
#include "ui_popup_switcher.c"
#include "vh_button.c"
#include "vh_list.c"
#include "vh_textinput.c"
#include "zc_cstring.c"
#include "zc_vector.c"

void    ui_inputfield_popup_accept(void* userdata, void* data);
void    ui_inputfield_popup_reject(void* userdata, void* data);
void    ui_inputfield_popup_enter(view_t* view);
void    ui_inputfield_on_button_down(void* userdata, void* data);
view_t* ui_inputfield_item_for_index(int index, void* userdata, view_t* listview, int* item_count);
void    ui_inputfield_on_text(view_t* view);

struct _ui_inputfield_popup_t
{
  char        attached;
  vec_t*      requests;
  view_t*     listview;
  view_t*     textfield;
  view_t*     inputfield;
  textstyle_t textstyle;
} uip = {0};

void ui_inputfield_popup_attach(view_t* baseview)
{
  if (uip.attached) return;

  textstyle_t ts = {0};
  ts.font        = config_get("font_path");
  ts.size        = 30.0;
  ts.margin      = 5;
  ts.align       = TA_LEFT;
  ts.textcolor   = 0x000000FF;
  ts.backcolor   = 0;
  ts.multiline   = 0;
  ts.autosize    = AS_AUTO;

  view_t* acc_btn = view_get_subview(baseview, "inp_popup_accept_btn");
  view_t* rej_btn = view_get_subview(baseview, "inp_popup_reject_btn");
  view_t* clr_btn = view_get_subview(baseview, "inp_popup_clear_btn");

  view_t* listview   = view_get_subview(baseview, "inp_popup_listview");
  view_t* textfield  = view_get_subview(baseview, "inp_popup_textfield");
  view_t* inputfield = view_get_subview(baseview, "inp_popup_inputfield");

  tg_text_add(textfield);

  vh_textinput_add(inputfield, "/home/youruser/Music", "", ts, NULL);
  vh_textinput_set_on_text(inputfield, ui_inputfield_on_text);
  vh_textinput_set_on_return(inputfield, ui_inputfield_popup_enter);

  ts.autosize  = AS_FIX;
  ts.multiline = 1;
  ts.align     = TA_CENTER;

  cb_t* cb_btn_press = cb_new(ui_inputfield_on_button_down, NULL);

  vh_button_add(acc_btn, VH_BUTTON_NORMAL, cb_btn_press);
  vh_button_add(rej_btn, VH_BUTTON_NORMAL, cb_btn_press);
  vh_button_add(clr_btn, VH_BUTTON_NORMAL, cb_btn_press);

  uip.textstyle  = ts;
  uip.listview   = listview;
  uip.textfield  = textfield;
  uip.inputfield = inputfield;
  uip.attached   = 1;
  uip.requests   = VNEW();

  vh_list_add(uip.listview,
              ((vh_list_inset_t){0}),
              ui_inputfield_item_for_index,
              NULL,
              NULL);

  RET(inputfield);
  view_remove(view_get_subview(baseview, "inp_popup_inputfield_back"), inputfield);

  REL(cb_btn_press);
}

view_t* ui_inputfield_item_for_index(int index, void* userdata, view_t* listview, int* item_count)
{
  if (index == 0)
    return uip.inputfield;
  else
    return NULL;
}

void ui_inputfield_on_text(view_t* view)
{
  if (uip.inputfield->frame.local.w > uip.listview->frame.local.w)
  {
    vh_list_set_item_width(uip.listview, uip.inputfield->frame.local.w);
    vh_list_scroll_to_x_poisiton(uip.listview, uip.listview->frame.local.w - uip.inputfield->frame.local.w);
  }
}

void ui_inputfield_on_button_down(void* userdata, void* data)
{
  char* id = ((view_t*)data)->id;

  if (strcmp(id, "inp_popup_accept_btn") == 0) ui_inputfield_popup_accept(NULL, NULL);
  if (strcmp(id, "inp_popup_reject_btn") == 0) ui_inputfield_popup_reject(NULL, NULL);
  if (strcmp(id, "inp_popup_clear_btn") == 0)
  {
    vh_list_set_item_width(uip.listview, uip.inputfield->frame.local.w);
    vh_list_scroll_to_x_poisiton(uip.listview, 0);
    vh_textinput_set_text(uip.inputfield, "");
    vh_textinput_activate(uip.inputfield, 1);
    ui_manager_activate(uip.inputfield);
  }
}

void ui_inputfield_popup_shownext()
{
  if (uip.requests->length > 0)
  {
    map_t* request = vec_tail(uip.requests);
    char*  text    = MGET(request, "text");
    tg_text_set(uip.textfield, text, uip.textstyle);
  }
  else
    ui_popup_switcher_toggle("inp_popup_page");
}

void ui_inputfield_popup_show(char* text, cb_t* acc_cb, cb_t* rej_cb)
{
  map_t* request = MNEW();
  MPUTR(request, "text", cstr_fromcstring(text));
  if (acc_cb) MPUT(request, "acc_cb", acc_cb);
  if (rej_cb) MPUT(request, "rej_cb", rej_cb);

  VADD(uip.requests, request);

  tg_text_set(uip.textfield, text, uip.textstyle);
  ui_popup_switcher_toggle("inp_popup_page");

  vh_textinput_activate(uip.inputfield, 1); // activate text input
  ui_manager_activate(uip.inputfield);      // set text input as event receiver
}

void ui_inputfield_popup_enter(view_t* view)
{
  ui_inputfield_popup_accept(NULL, NULL);
}

void ui_inputfield_popup_accept(void* userdata, void* data)
{
  map_t* request = vec_tail(uip.requests);
  if (request)
  {
    char* text   = MGET(request, "text");
    cb_t* acc_cb = MGET(request, "acc_cb");

    str_t* inputstr = vh_textinput_get_text(uip.inputfield);

    if (acc_cb) (*acc_cb->fp)(acc_cb->userdata, inputstr);
    vec_rem(uip.requests, request);
  }

  ui_inputfield_popup_shownext();
}

void ui_inputfield_popup_reject(void* userdata, void* data)
{
  map_t* request = vec_tail(uip.requests);
  if (request)
  {
    char* text   = MGET(request, "text");
    cb_t* rej_cb = MGET(request, "rej_cb");

    if (rej_cb) (*rej_cb->fp)(rej_cb->userdata, NULL);
    vec_rem(uip.requests, request);
  }

  ui_inputfield_popup_shownext();
}

#endif
