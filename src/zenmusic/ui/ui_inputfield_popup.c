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
#include "vh_textinput.c"
#include "zc_cstring.c"
#include "zc_vector.c"

void ui_inputfield_popup_accept(void* userdata, void* data);
void ui_inputfield_popup_reject(void* userdata, void* data);
void ui_inputfield_popup_enter(view_t* view);

struct _ui_inputfield_popup_t
{
  char        attached;
  vec_t*      requests;
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
  ts.multiline   = 1;

  view_t* acc_btn    = view_get_subview(baseview, "inp_popup_accept_btn");
  view_t* rej_btn    = view_get_subview(baseview, "inp_popup_reject_btn");
  view_t* textfield  = view_get_subview(baseview, "inp_popup_textfield");
  view_t* inputfield = view_get_subview(baseview, "inp_popup_inputfield");

  cb_t* acc_cb = cb_new(ui_inputfield_popup_accept, NULL);
  cb_t* rej_cb = cb_new(ui_inputfield_popup_reject, NULL);

  tg_text_add(textfield);

  vh_textinput_add(inputfield, "/home/youruser/Music", "", ts, NULL);
  vh_textinput_set_on_return(inputfield, ui_inputfield_popup_enter);

  ts.align = TA_CENTER;

  vh_button_add(acc_btn, VH_BUTTON_NORMAL, acc_cb);
  vh_button_add(rej_btn, VH_BUTTON_NORMAL, rej_cb);

  REL(acc_cb);
  REL(rej_cb);

  uip.textstyle  = ts;
  uip.textfield  = textfield;
  uip.inputfield = inputfield;
  uip.attached   = 1;
  uip.requests   = VNEW();
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
