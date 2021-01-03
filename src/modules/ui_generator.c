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
void     ui_generator_render(uint32_t);
void     ui_generator_cleanup();
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
  vec_t*      views;
  SDL_Thread* thread;
  ch_t*       channel;
} uig = {0};

int ui_generator_init(int width, int height)
{
  ui_compositor_init(width, height);

  uig.views   = VNEW();
  uig.channel = ch_new(50);
  uig.thread  = SDL_CreateThread(ui_generator_workloop, "generator", NULL);

  return (uig.thread != NULL);
}

void ui_generator_cleanup()
{
  ui_compositor_rewind();
  vec_reset(uig.views);
}

void ui_generator_add(view_t* view)
{
  VADD(uig.views, view);

  // assign pages if view is new
  if (view->texture.page == -1)
  {
    // use a texture map texture page
    if (view->texture.type == TT_MANAGED) view_set_texture_page(view, ui_compositor_map_texture());
    if (view->texture.type == TT_EXTERNAL) view_set_texture_page(view, ui_compositor_new_texture());
  }

  ui_compositor_upd(view->id,
                    view->hidden,
                    view->frame.global,       // frame
                    view->layout.shadow_blur, // view border
                    view->texture.page,       // texture page
                    view->texture.full,       // needs full texture
                    view->texture.page > 0,   // external texture
                    view->texture.id);        // texture id
}

void ui_generator_render(uint32_t time)
{
  for (int i = 0; i < uig.views->length; i++)
  {
    view_t* view = uig.views->data[i];

    if (view->texture.type == TT_MANAGED && view->texture.state == TS_BLANK)
    {
      if (view->texture.rentype == RT_BACKGROUND)
      {
        if (ch_send(uig.channel, view)) view->texture.state = TS_PENDING;
      }
      else
      {
        view_gen_texture(view);
      }
    }

    if (view->frame.pos_changed)
    {
      ui_compositor_upd_pos(i, view->frame.global, view->layout.shadow_blur);

      view->frame.pos_changed = 0;
    }

    if (view->texture.changed)
    {
      ui_compositor_upd_bmp(i, view->frame.global, view->layout.shadow_blur, view->texture.id, view->texture.bitmap);

      view->frame.dim_changed = 0;
      view->texture.changed   = 0;
    }
  }

  ui_compositor_render(time);
}

int ui_generator_workloop()
{
  view_t* view;

  while (1)
  {
    while ((view = ch_recv(uig.channel)))
    {
      view_gen_texture(view);
    }
    SDL_Delay(16);
  }
}

void ui_generator_resize(int width, int height)
{
  ui_compositor_resize(width, height);

  // TODO DO WE NEED TO RESET?!?!
  ui_compositor_reset();

  for (int i = 0; i < uig.views->length; i++)
  {
    view_t* view = uig.views->data[i];
    ui_compositor_upd(view->id,
                      view->hidden,
                      view->frame.global,       // frame
                      view->layout.shadow_blur, // view border
                      view->texture.page,       // texture page
                      view->texture.full,       // needs full texture
                      view->texture.page > 0,   // external texture
                      view->texture.id);        // texture id

    if (view->texture.state == TS_READY)
      ui_compositor_upd_bmp(i, view->frame.global, view->layout.shadow_blur, view->texture.id, view->texture.bitmap);
  }
}

#endif
