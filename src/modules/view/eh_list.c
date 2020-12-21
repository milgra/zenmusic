#ifndef eh_list_h
#define eh_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _eh_list_t
{
  vec_t* items;
  vec_t* cache;

  int head_index;
  int tail_index;
  int item_count;
  int full;

  float item_wth;
  float item_pos;

  // scrollers

  view_t* vscr;
  view_t* hscr;

  uint32_t vtimeout;
  uint32_t htimeout;

  view_t* (*row_generator)(view_t* listview, view_t* rowview, int index, int* item_count); /* event handler for view */
} eh_list_t;

void eh_list_add(view_t* view, view_t* (*row_generator)(view_t* listview, view_t* rowview, int index, int* item_count));
void eh_list_fill(view_t* view);
void eh_list_reset(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "eh_anim.c"
#include "mtcstring.c"
#include "mtstring.c"
#include "tg_css.c"
#include "tg_text.c"
#include <math.h>

#define PRELOAD_DISTANCE 100.0

void eh_list_move(view_t* view, float dy)
{
  view_t* sview;

  eh_list_t* eh = view->evt_han_data;
  while ((sview = VNXT(eh->items)))
  {
    r2_t frame = sview->frame.local;

    frame.x = eh->item_pos;
    frame.y += dy;

    view_set_frame(sview, frame);
  }

  float sr; // size ratio
  float pr; // position ratio
  float s;  // size
  float p;  // position

  // vertical scroller
  sr = (float)(eh->tail_index - eh->head_index) / (float)eh->item_count;
  pr = (float)(eh->tail_index) / (float)(eh->item_count - (eh->tail_index - eh->head_index));

  if (sr < 0.1) sr = 0.1;
  s = view->frame.local.h * sr;
  p = (view->frame.local.h - s) * pr;

  view_set_frame(eh->vscr, (r2_t){view->frame.local.w - 16.0, p, 15.0, s});

  // horizontal scroller
  sr = view->frame.local.w / eh->item_wth;
  pr = -eh->item_pos / (eh->item_wth - view->frame.global.w);

  if (sr < 0.1) sr = 0.1;
  s = view->frame.local.w * sr;
  p = (view->frame.local.w - s) * pr;

  view_set_frame(eh->hscr, (r2_t){p, view->frame.local.h - 15.0, s, 15.0});
}

void eh_list_evt(view_t* view, ev_t ev)
{
  eh_list_t* eh = view->evt_han_data;
  if (ev.type == EV_TIME)
  {
    // fill up if needed
    while (eh->full == 0)
    {
      if (eh->items->length == 0)
      {
        view_t* cacheitem = vec_head(eh->cache);
        view_t* rowitem   = (*eh->row_generator)(view, cacheitem, 0, &eh->item_count);

        if (rowitem)
        {
          eh->item_wth = rowitem->frame.global.w; // store maximum width
          VREM(eh->cache, rowitem);
          VADD(eh->items, rowitem);
          if (rowitem->parent == NULL) view_insert(view, rowitem, 0);
        }
        else
          eh->full = 1;
      }
      else
      {
        view_t* head = vec_head(eh->items);
        view_t* tail = vec_tail(eh->items);

        // add items if needed

        if (head->frame.local.y > 0.0 - PRELOAD_DISTANCE)
        {
          view_t* cacheitem = vec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->head_index - 1, &eh->item_count);

          if (rowitem)
          {
            eh->full     = 0;                       // there is probably more to come
            eh->item_wth = rowitem->frame.global.w; // store maximum width

            VREM(eh->cache, rowitem);
            vec_addatindex(eh->items, rowitem, 0);

            if (rowitem->parent == NULL) view_insert(view, rowitem, 0);
            view_set_frame(rowitem, (r2_t){0, head->frame.local.y - rowitem->frame.local.h, rowitem->frame.local.w, rowitem->frame.local.h});

            eh->head_index -= 1;
          }
          else
            eh->full = 1;
        }
        else
          eh->full = 1;

        if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h + PRELOAD_DISTANCE)
        {
          view_t* cacheitem = vec_head(eh->cache);
          view_t* rowitem   = (*eh->row_generator)(view, cacheitem, eh->tail_index + 1, &eh->item_count);

          if (rowitem)
          {
            eh->full     = 0;                       // there is probably more to come
            eh->item_wth = rowitem->frame.global.w; // store maximum width

            VREM(eh->cache, rowitem);
            VADD(eh->items, rowitem);

            if (rowitem->parent == NULL) view_insert(view, rowitem, view->views->length - 3);
            view_set_frame(rowitem, (r2_t){0, tail->frame.local.y + tail->frame.local.h, rowitem->frame.local.w, rowitem->frame.local.h});

            eh->tail_index += 1;
          }
          else
            eh->full &= 1;
        }
        else
          eh->full &= 1;

        // remove items if needed

        if (head->frame.local.y + head->frame.local.h < 0.0 - PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, head);
          VREM(eh->items, head);

          eh->head_index += 1;
        }

        if (tail->frame.local.y > view->frame.local.h + PRELOAD_DISTANCE && eh->items->length > 1)
        {
          VADD(eh->cache, tail);
          VREM(eh->items, tail);

          eh->tail_index -= 1;
        }
      }
    }
    // scroll bounce if needed
    if (eh->items->length > 0)
    {
      view_t* head = vec_head(eh->items);
      view_t* tail = vec_tail(eh->items);

      // horizontal bounce

      if (eh->item_pos > 0.0001)
        eh->item_pos += -eh->item_pos / 5.0;
      else if (eh->item_pos < -0.0001 && eh->item_pos + eh->item_wth < view->frame.local.w)
        eh->item_pos += (view->frame.local.w - eh->item_wth - eh->item_pos) / 5.0;

      // vertical bounce

      if (head->frame.local.y > 0.0001)
        eh_list_move(view, -head->frame.local.y / 5.0);
      else if (head->frame.local.y < -0.0001 && tail->frame.local.y + tail->frame.local.h < view->frame.local.h)
        eh_list_move(view, (view->frame.local.h - tail->frame.local.h - tail->frame.local.y) / 5.0);
      else if (eh->item_pos > 0.0001 || eh->item_pos < -0.0001)
        eh_list_move(view, 0);
    }
    // close scrollbars
    if (eh->vtimeout > 0 && eh->vtimeout < ev.time)
    {
      eh->vtimeout = 0;

      r2_t sf = eh->vscr->frame.local;
      r2_t ef = sf;
      ef.y    = ef.y + ef.h / 2.0;
      ef.h    = 0.0;

      eh_anim_set(eh->vscr, sf, ef, 10, AT_LINEAR);
    }
    if (eh->htimeout > 0 && eh->htimeout < ev.time)
    {
      eh->htimeout = 0;

      r2_t sf = eh->hscr->frame.local;
      r2_t ef = sf;
      ef.x    = ef.x + ef.w / 2.0;
      ef.w    = 0.0;

      eh_anim_set(eh->hscr, sf, ef, 10, AT_LINEAR);
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    if (eh->items->length > 0)
    {
      if (ev.dx != 0.0)
      {
        eh->item_pos += ev.dx;

        if (eh->htimeout == 0)
        {
          eh->htimeout = ev.time + 1000;

          r2_t ef = eh->hscr->frame.local;
          r2_t sf = ef;
          sf.x    = sf.x + sf.w / 2.0;
          sf.w    = 0.0;

          eh_anim_set(eh->hscr, sf, ef, 10, AT_LINEAR);
        }
        else
          eh->htimeout = ev.time + 1000;
      }

      if (ev.dy != 0.0)
      {
        eh_list_move(view, ev.dy);
        eh->full = 0;

        if (eh->vtimeout == 0)
        {
          eh->vtimeout = ev.time + 1000;

          r2_t ef = eh->vscr->frame.local;
          r2_t sf = ef;
          sf.y    = sf.y + sf.h / 2.0;
          sf.h    = 0.0;

          eh_anim_set(eh->vscr, sf, ef, 10, AT_LINEAR);
        }
        else
          eh->vtimeout = ev.time + 1000;
      }
      else if (ev.dx != 0)
        eh_list_move(view, 0);
    }
  }
  else if (ev.type == EV_RESIZE)
  {
    eh->full = 0;
  }
}

void eh_list_del(void* p)
{
  eh_list_t* eh = (eh_list_t*)p;
  REL(eh->items);
}

void eh_list_fill(view_t* view)
{
  eh_list_t* eh = view->evt_han_data;
  eh->full      = 0;
}

void eh_list_reset(view_t* view)
{
  eh_list_t* eh = view->evt_han_data;

  // add all items to cache
  vec_addinvector(eh->cache, eh->items);
  vec_reset(eh->items);

  eh->head_index = 0;
  eh->tail_index = 0;
  eh->item_count = 0;
  eh->full       = 0;

  // move cache items out of screen
  for (int index = 0;
       index < eh->cache->length;
       index++)
  {
    view_t* view  = eh->cache->data[index];
    r2_t    frame = view->frame.local;
    frame.y       = -frame.h;
    view_set_frame(view, frame);
  }
}

void eh_list_add(view_t* view,
                 view_t* (*row_generator)(view_t* listview, view_t* rowview, int index, int* item_count))
{
  eh_list_t* eh     = mem_calloc(sizeof(eh_list_t), "eh_list", eh_list_del, NULL);
  eh->items         = VNEW();
  eh->cache         = VNEW();
  eh->row_generator = row_generator;

  view_t* vscr = view_new("vscr", (r2_t){0, 0, 15, 0});
  view_t* hscr = view_new("hscr", (r2_t){0, 10, 0, 10});

  tg_css_add(hscr);
  tg_css_add(vscr);

  eh_anim_add(vscr);
  eh_anim_add(hscr);

  vscr->layout.background_color = 0x000000AA;
  //vscr->layout.border_radius    = 5;
  //vscr->layout.shadow_blur = 3;

  hscr->layout.background_color = 0x000000AA;
  //hscr->layout.border_radius    = 5;
  //hscr->layout.shadow_blur = 3;

  view_add(view, vscr);
  view_add(view, hscr);

  eh->vscr = vscr;
  eh->hscr = hscr;

  view->needs_scroll = 1;
  view->evt_han_data = eh;
  view->evt_han      = eh_list_evt;
}

#endif
