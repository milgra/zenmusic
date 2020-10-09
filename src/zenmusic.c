#include "common.c"
#include "font.c"
#include "math2.c"
#include "mtbm.c"
#include "mtcstr.c"
#include "mtstr.c"
#include "ui_compositor.c"
#include "ui_connector.c"
#include "view.c"
#include "wm_connector.c"
#include "wm_event.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void gen_color(view_t* view)
{
  printf("gen_color %s\n", view->id);
  bm_t* bmp = bm_new(200, 200);
  bm_fill(bmp,
          0,
          0,
          200,
          200,
          0xFF0000FF);
  view_setbmp(view, bmp);
}

void gen_text(view_t* view)
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

view_t* viewA;
view_t* viewB;

void init(int width, int height)
{
  printf("zenmusic init %i %i\n", width, height);

  srand((unsigned int)time(NULL));
  char* respath = SDL_GetBasePath();

  char* path = mtcstr_fromformat("%s/../res/Terminus.ttf", respath, NULL);

  common_font = font_alloc(path);

  ui_connector_init(width, height);

  viewA = view_new("viewa", (v4_t){20.0, 20.0, 150.0, 70.0}, NULL, gen_color, NULL);
  viewB = view_new("viewb", (v4_t){200.0, 420.0, 350.0, 170.0}, NULL, gen_text, NULL);

  ui_connector_add(viewA);
  ui_connector_add(viewB);
}

void update(ev_t ev)
{

  if (ev.type == EV_MMOVE && ev.drag)
  {
    view_setpos(viewA, (v2_t){ev.x, ev.y});
  }
}

void render()
{
  ui_connector_render();
}

void destroy()
{
}

int main(int argc, char* args[])
{
  wm_init(init, update, render, destroy);

  return 0;
}
