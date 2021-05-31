#ifndef ui_decision_popup_h
#define ui_decision_popup_h

#include "view.c"
#include "zc_callback.c"

void ui_decision_popup_attach(view_t* baseview);
void ui_decision_popup_show(char* text, cb_t* acc_cb, cb_t* rej_cb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "tg_text.c"
#include "ui_popup_switcher.c"
#include "vh_button.c"
#include "zc_cstring.c"
#include "zc_vector.c"

void ui_decision_popup_accept(void* userdata, void* data);
void ui_decision_popup_reject(void* userdata, void* data);

struct _ui_decision_popup_t
{
  char        attached;
  vec_t*      requests;
  view_t*     tf;
  textstyle_t ts;
} udp = {0};

void ui_decision_popup_attach(view_t* baseview)
{
  if (udp.attached) return;

  textstyle_t ts  = {0};
  ts.font         = config_get("font_path");
  ts.size         = 30.0;
  ts.margin_right = 20;
  ts.align        = TA_CENTER;
  ts.textcolor    = 0x000000FF;
  ts.backcolor    = 0;
  ts.multiline    = 1;

  view_t* acc_btn = view_get_subview(baseview, "dec_pop_acc_btn");
  view_t* rej_btn = view_get_subview(baseview, "dec_pop_rej_btn");
  view_t* tf      = view_get_subview(baseview, "dec_pop_tf");

  cb_t* acc_cb = cb_new(ui_decision_popup_accept, NULL);
  cb_t* rej_cb = cb_new(ui_decision_popup_reject, NULL);

  tg_text_add(tf);

  vh_button_add(acc_btn, VH_BUTTON_NORMAL, acc_cb);
  vh_button_add(rej_btn, VH_BUTTON_NORMAL, rej_cb);

  REL(acc_cb);
  REL(rej_cb);

  udp.ts       = ts;
  udp.tf       = tf;
  udp.attached = 1;
  udp.requests = VNEW();
}

void ui_decision_popup_shownext()
{
  if (udp.requests->length > 0)
  {
    map_t* request = vec_tail(udp.requests);
    char*  text    = MGET(request, "text");
    tg_text_set(udp.tf, text, udp.ts);
  }
  else
    ui_popup_switcher_toggle("decision_popup_page");
}

void ui_decision_popup_show(char* text, cb_t* acc_cb, cb_t* rej_cb)
{
  map_t* request = MNEW();
  MPUTR(request, "text", cstr_fromcstring(text));
  if (acc_cb) MPUT(request, "acc_cb", acc_cb);
  if (rej_cb) MPUT(request, "rej_cb", rej_cb);

  VADD(udp.requests, request);

  tg_text_set(udp.tf, text, udp.ts);
  ui_popup_switcher_toggle("decision_popup_page");
}

void ui_decision_popup_accept(void* userdata, void* data)
{
  map_t* request = vec_tail(udp.requests);
  if (request)
  {
    char* text   = MGET(request, "text");
    cb_t* acc_cb = MGET(request, "acc_cb");

    if (acc_cb) (*acc_cb->fp)(acc_cb->userdata, NULL);
    vec_rem(udp.requests, request);
  }

  ui_decision_popup_shownext();
}

void ui_decision_popup_reject(void* userdata, void* data)
{
  map_t* request = vec_tail(udp.requests);
  if (request)
  {
    char* text   = MGET(request, "text");
    cb_t* rej_cb = MGET(request, "rej_cb");

    if (rej_cb) (*rej_cb->fp)(rej_cb->userdata, NULL);
    vec_rem(udp.requests, request);
  }

  ui_decision_popup_shownext();
}

#endif
