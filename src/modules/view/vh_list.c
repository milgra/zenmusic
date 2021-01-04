#ifndef vh_list_h
#define vh_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _vh_list_t
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

  view_t* (*create_item)(view_t* listview);
  int (*update_item)(view_t* listview, view_t* item, int index, int* item_count);
} vh_list_t;

void    vh_list_add(view_t* view,
                    view_t* (*create_item)(view_t* listview),
                    int (*update_item)(view_t* listview, view_t* item, int index, int* item_count));
vec_t*  vh_list_items(view_t* view);
vec_t*  vh_list_cache(view_t* view);
void    vh_list_fill(view_t* view);
void    vh_list_reset(view_t* view);
view_t* vh_list_item_for_index(view_t* view, int index);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_text.c"
#include "vh_anim.c"
#include "vh_sbar.c"
#include <math.h>

#define PRELOAD_DISTANCE 100.0

void vh_list_move(view_t* view, float dy)
{
  view_t* sview;

  vh_list_t* vh = view->handler_data;
  while ((sview = VNXT(vh->items)))
  {
    r2_t frame = sview->frame.local;

    frame.x = vh->item_pos;
    frame.y += dy;

    view_set_frame(sview, frame);
  }

  float sr; // size ratio
  float pr; // position ratio
  float s;  // size
  float p;  // position

  // vertical scroller
  sr = (float)(vh->tail_index - vh->head_index) / (float)vh->item_count;
  pr = (float)(vh->tail_index) / (float)(vh->item_count - (vh->tail_index - vh->head_index));

  if (sr < 0.1) sr = 0.1;
  s = view->frame.local.h * sr;
  p = (view->frame.local.h - s) * pr;

  view_set_frame(vh->vscr, (r2_t){view->frame.local.w - 16.0, p, 15.0, s});

  // horizontal scroller
  sr = view->frame.local.w / vh->item_wth;
  pr = -vh->item_pos / (vh->item_wth - view->frame.global.w);

  if (sr < 0.1) sr = 0.1;
  s = view->frame.local.w * sr;
  p = (view->frame.local.w - s) * pr;

  view_set_frame(vh->hscr, (r2_t){p, view->frame.local.h - 15.0, s, 15.0});
}

view_t* vh_list_get_item(view_t* view)
{
  vh_list_t* vh = view->handler_data;
  if (vh->cache->length == 0)
  {
    view_t* item = (*vh->create_item)(view);
    VADD(vh->cache, item);
    view_insert(view, item, 0);
    //view_set_hidden(item, 1, 1);
    view_set_frame(item, (r2_t){0, -item->frame.local.h, item->frame.local.w, item->frame.local.h});
  }
  return vec_head(vh->cache);
}

void vh_list_evt(view_t* view, ev_t ev)
{
  vh_list_t* vh = view->handler_data;
  if (ev.type == EV_TIME)
  {
    // fill up if needed
    while (vh->full == 0)
    {
      if (vh->items->length == 0)
      {
        view_t* item = vh_list_get_item(view);
        int     full = (*vh->update_item)(view, item, 0, &vh->item_count);

        if (!full)
        {
          VREM(vh->cache, item);
          VADD(vh->items, item);
          //view_set_hidden(item, 0, 1);

          vh->item_wth = item->frame.global.w; // store maximum width

          view_set_frame(item, (r2_t){0, 0, item->frame.local.w, item->frame.local.h});
        }
        else
          vh->full = 1;
      }
      else
      {
        view_t* head = vec_head(vh->items);
        view_t* tail = vec_tail(vh->items);

        // add items if needed

        if (head->frame.local.y > 0.0 - PRELOAD_DISTANCE)
        {
          view_t* item = vh_list_get_item(view);
          int     full = (*vh->update_item)(view, item, vh->head_index - 1, &vh->item_count);

          if (!full)
          {
            VREM(vh->cache, item);
            vec_addatindex(vh->items, item, 0);
            //view_set_hidden(item, 0, 1);

            vh->full     = 0;                    // there is probably more to come
            vh->item_wth = item->frame.global.w; // store maximum width
            vh->head_index -= 1;                 // decrease head index

            view_set_frame(item, (r2_t){0, head->frame.local.y - item->frame.local.h, item->frame.local.w, item->frame.local.h});
          }
          else
            vh->full = 1;
        }
        else
          vh->full = 1;

        if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h + PRELOAD_DISTANCE)
        {
          view_t* item = vh_list_get_item(view);
          int     full = (*vh->update_item)(view, item, vh->tail_index + 1, &vh->item_count);

          if (!full)
          {
            VREM(vh->cache, item);
            VADD(vh->items, item);
            //view_set_hidden(item, 0, 1);

            vh->full     = 0;                    // there is probably more to come
            vh->item_wth = item->frame.global.w; // store maximum width
            vh->tail_index += 1;                 // increase tail index

            view_set_frame(item, (r2_t){0, tail->frame.local.y + tail->frame.local.h, item->frame.local.w, item->frame.local.h});
          }
          else
            vh->full &= 1; // don't set to full if previously item is added
        }
        else
          vh->full &= 1; // don't set to full if previously item is added

        // remove items if needed

        if (head->frame.local.y + head->frame.local.h < 0.0 - PRELOAD_DISTANCE && vh->items->length > 1)
        {
          VADD(vh->cache, head);
          VREM(vh->items, head);
          vh->head_index += 1;
          //view_set_hidden(head, 1, 1);
        }

        if (tail->frame.local.y > view->frame.local.h + PRELOAD_DISTANCE && vh->items->length > 1)
        {
          VADD(vh->cache, tail);
          VREM(vh->items, tail);
          vh->tail_index -= 1;
          //view_set_hidden(tail, 1, 1);
        }
      }
    }
    // scroll bounce if needed
    if (vh->items->length > 0)
    {
      view_t* head = vec_head(vh->items);
      view_t* tail = vec_tail(vh->items);

      // horizontal bounce

      if (vh->item_pos > 0.0001)
        vh->item_pos += -vh->item_pos / 5.0;
      else if (vh->item_pos < -0.0001 && vh->item_pos + vh->item_wth < view->frame.local.w)
        vh->item_pos += (view->frame.local.w - vh->item_wth - vh->item_pos) / 5.0;

      // vertical bounce

      if (head->frame.local.y > 0.0001)
        vh_list_move(view, -head->frame.local.y / 5.0);
      else if (head->frame.local.y < -0.0001 && tail->frame.local.y + tail->frame.local.h < view->frame.local.h)
        vh_list_move(view, (view->frame.local.h - tail->frame.local.h - tail->frame.local.y) / 5.0);
      else if (vh->item_pos > 0.0001 || vh->item_pos < -0.0001)
        vh_list_move(view, 0);
    }
    // close scrollbars
    if (vh->vtimeout > 0 && vh->vtimeout < ev.time)
    {
      vh->vtimeout = 0;
      vh_sbar_close(vh->vscr);
    }
    if (vh->htimeout > 0 && vh->htimeout < ev.time)
    {
      vh->htimeout = 0;
      vh_sbar_close(vh->hscr);
    }
  }
  else if (ev.type == EV_SCROLL)
  {
    if (vh->items->length > 0)
    {
      if (ev.dx != 0.0)
      {
        vh->item_pos += ev.dx;

        if (vh->htimeout == 0)
        {
          vh->htimeout = ev.time + 1000;
          vh_sbar_open(vh->hscr);
        }
        else
          vh->htimeout = ev.time + 1000;
      }

      if (ev.dy != 0.0)
      {
        vh_list_move(view, ev.dy);
        vh->full = 0;

        if (vh->vtimeout == 0)
        {
          vh->vtimeout = ev.time + 1000;
          vh_sbar_open(vh->vscr);
        }
        else
          vh->vtimeout = ev.time + 1000;
      }
      else if (ev.dx != 0)
        vh_list_move(view, 0);
    }
  }
  else if (ev.type == EV_RESIZE)
  {
    vh->full = 0;
  }
}

void vh_list_del(void* p)
{
  vh_list_t* vh = (vh_list_t*)p;
  REL(vh->items);
}

vec_t* vh_list_items(view_t* view)
{
  vh_list_t* vh = view->handler_data;
  return vh->items;
}

vec_t* vh_list_cache(view_t* view)
{
  vh_list_t* vh = view->handler_data;
  return vh->cache;
}

void vh_list_fill(view_t* view)
{
  vh_list_t* vh = view->handler_data;
  vh->full      = 0;
}

void vh_list_reset(view_t* view)
{
  vh_list_t* vh = view->handler_data;

  vec_reset(vh->items);

  vh->head_index = 0;
  vh->tail_index = 0;
  vh->item_count = 0;
  vh->full       = 0;
}

void vh_list_add(view_t* view,
                 view_t* (*create_item)(view_t* listview),
                 int (*update_item)(view_t* listview, view_t* item, int index, int* item_count))
{
  vh_list_t* vh   = mem_calloc(sizeof(vh_list_t), "vh_list", vh_list_del, NULL);
  vh->items       = VNEW();
  vh->cache       = VNEW();
  vh->create_item = create_item;
  vh->update_item = update_item;

  view_t* vscr = view_new("vscr", (r2_t){0, 0, 15, 0});
  view_t* hscr = view_new("hscr", (r2_t){0, 10, 0, 10});

  vh_sbar_add(vscr, SBAR_V, 30);
  vh_sbar_add(hscr, SBAR_H, 30);

  view_add(view, vscr);
  view_add(view, hscr);

  vh->vscr = vscr;
  vh->hscr = hscr;

  view->needs_scroll = 1;
  view->handler_data = vh;
  view->handler      = vh_list_evt;
}

view_t* vh_list_item_for_index(view_t* view, int index)
{
  vh_list_t* vh = view->handler_data;

  if (index > vh->head_index && index < vh->tail_index)
  {
    int diff = index - vh->head_index;
    return vh->items->data[diff];
  }

  return NULL;
}

#endif
