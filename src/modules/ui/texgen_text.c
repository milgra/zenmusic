#ifndef texgen_text_h
#define texgen_text_h

#include "view.c"

void texgen_text(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "common.c"
#include "font.c"
#include "mtbitmap.c"
#include "mtstring.c"

void texgen_text(view_t* view)
{
  printf("gen_text %s\n", view->id);
  mtstr_t* str = mtstr_frombytes("KUTYAFASZA");

  textstyle_t ts =
      {
          .align      = 0,
          .editable   = 0,
          .selectable = 0,
          .multiline  = 1,
          .autosize   = 1,
          .uppercase  = 0,

          .textsize   = 25.0,
          .marginsize = 10.0,
          .cursorsize = 15.0,

          .textcolor = 0xFFFFFFFF,
          .backcolor = 0x000000FF,
      };

  bm_t* bmp = font_render_text((int)view->frame.z, (int)view->frame.w, str, common_font, ts, NULL, NULL);
  view_setbmp(view, bmp);
}

#endif
