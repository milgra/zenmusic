#ifndef ui_decision_popup_h
#define ui_decision_popup_h

#include "mtcallback.c"
#include "view.c"

void ui_decision_popup_attach(view_t* baseview);
void ui_decision_popup_show(char* text, cb_t* callback);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "mtcstring.c"
#include "mtvector.c"
#include "tg_text.c"
#include "ui_popup_switcher.c"
#include "vh_button.c"

void ui_decision_popup_accept(void* userdata, void* data);

struct _ui_decision_popup_t
{
  char        attached;
  vec_t*      textqueue;
  vec_t*      callqueue;
  view_t*     tf;
  view_t*     btn;
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

  view_t* btn = view_get_subview(baseview, "dec_pop_acc_btn");
  view_t* tf  = view_get_subview(baseview, "dec_pop_tf");

  cb_t* acc = cb_new(ui_decision_popup_accept, NULL);

  tg_text_add(tf);
  vh_button_add(btn, VH_BUTTON_NORMAL, acc);

  REL(acc);

  udp.ts        = ts;
  udp.tf        = tf;
  udp.btn       = btn;
  udp.attached  = 1;
  udp.textqueue = VNEW();
  udp.callqueue = VNEW();
}

void ui_decision_popup_show(char* text, cb_t* callback)
{
  VADDR(udp.textqueue, cstr_fromcstring(text));
  VADD(udp.callqueue, callback);

  tg_text_set(udp.tf, text, udp.ts);
  ui_popup_switcher_toggle("decision_popup_page");
}

void ui_decision_popup_accept(void* userdata, void* data)
{
  ui_popup_switcher_toggle("decision_popup_page");

  cb_t* callback = vec_tail(udp.callqueue);
  if (callback)
  {
    (*callback->fp)(callback->userdata, NULL);
    vec_rem(udp.callqueue, callback);
    char* text = vec_tail(udp.textqueue);
    vec_rem(udp.textqueue, text);
  }
  if (udp.textqueue->length > 0)
  {
    char* text = vec_tail(udp.textqueue);
    tg_text_set(udp.tf, text, udp.ts);
  }
}

void ui_decision_popup_reject(void* userdata, void* data)
{
}

#endif
