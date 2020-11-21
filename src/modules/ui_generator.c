/*
  UI Generator Module for Zen Multimedia Desktop System 
  Monitors views for changes, renders their content in the background, updates compositor state based on view state.
  All views have to be added to ui_generator, hierarchy is not handled.

  views  ->
  events -> ui_generator -> ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_generator_h
#define ui_generator_h

#include "view.c"

int      ui_generator_init(int, int);
void     ui_generator_reset();
void     ui_generator_render();
void     ui_generator_add(view_t* view);
void     ui_generator_remove(view_t* view);
void     ui_generator_set_index(view_t* view);
void     ui_generator_resize(int width, int height);
uint32_t ui_generate_create_texture();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtchannel.c"
#include "mtmap.c"
#include "mtvector.c"
#include "ui_compositor.c"
#include "view.c"
#include <SDL.h>
#include <unistd.h>

int ui_generator_workloop(void* mypointer);

struct _ui_generator_t
{
  mtvec_t*    views;
  SDL_Thread* thread;
  mtch_t*     channel;
} uig = {0};

int ui_generator_init(int width, int height)
{
  ui_compositor_init(width, height);

  uig.views   = VNEW();
  uig.channel = mtch_new(50);
  uig.thread  = SDL_CreateThread(ui_generator_workloop, "generator", NULL);

  return (uig.thread != NULL);
}

void ui_generator_reset()
{
  ui_compositor_reset();
  mtvec_reset(uig.views);
}

void ui_generator_add(view_t* view)
{
  VADD(uig.views, view);

  ui_compositor_add(view->id,
                    view->frame.global,
                    view->index);

  view->connected = 1;

  if (view->texture.type == TT_MANAGED)
  {
    int has_tex = ui_compositor_has_texture(view->texture.id);
    if (has_tex)
    {
      // no need for texture rendering
      view->texture.state = TS_READY;
      // update texture in compositor
      ui_compositor_set_texture(view->id,
                                view->texture.id,
                                view->texture.bitmap,
                                view->texture.page,
                                view->texture.shadow,
                                view->texture.blur,
                                view->texture.full);
    }
    else
    {
      // use a texture map texture page
      view_set_texture_page(view, ui_compositor_map_texture());
    }
  }
  else
  {
    // request new texture page if needed
    if (view->texture.page == 0)
    {
      view_set_texture_page(view, ui_compositor_new_texture());
      // update texture in compositor
      ui_compositor_set_texture(view->id,
                                view->texture.id,
                                view->texture.bitmap,
                                view->texture.page,
                                view->texture.shadow,
                                view->texture.blur,
                                view->texture.full);
    }
  }
}

void ui_generator_set_index(view_t* view)
{
  ui_compositor_set_index(view->id, view->index);
}

void ui_generator_render()
{
  view_t* view;

  while ((view = VNXT(uig.views)))
  {
    if (view->texture.type == TT_MANAGED && view->texture.state == TS_BLANK)
    {
      // send to renderer if needed
      if (mtch_send(uig.channel, view)) view->texture.state = TS_PENDING;
    }
    if (view->frame.changed)
    {
      // update frame in compositor
      ui_compositor_set_frame(view->id, view->frame.global);
      view->frame.changed = 0;
    }
    if (view->texture.changed)
    {
      // update texture in compositor
      ui_compositor_set_texture(view->id,
                                view->texture.id,
                                view->texture.bitmap,
                                view->texture.page,
                                view->texture.shadow,
                                view->texture.blur,
                                view->texture.full);

      view->texture.changed = 0;
    }
  }

  ui_compositor_render();
}

int ui_generator_workloop()
{
  view_t* view;

  while (1)
  {
    while ((view = mtch_recv(uig.channel)))
    {
      view_gen_texture(view);
    }
    SDL_Delay(16);
  }
}

void ui_generator_resize(int width, int height)
{
  ui_compositor_resize(width, height);

  // reset ui compositor and re-add views

  ui_compositor_reset();
  view_t* view;
  while ((view = VNXT(uig.views)))
  {
    ui_compositor_add(view->id,
                      view->frame.global,
                      view->index);

    if (view->texture.type == TT_EXTERNAL || view->texture.state == TS_READY)
    {
      ui_compositor_set_texture(view->id,
                                view->texture.id,
                                view->texture.bitmap,
                                view->texture.page,
                                view->texture.shadow,
                                view->texture.blur,
                                view->texture.full);
    }

    ui_compositor_set_frame(view->id, view->frame.global);
  }
}

#endif
