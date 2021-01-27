#ifndef vh_list_h
#define vh_list_h

#include "mtvector.c"
#include "view.c"
#include "wm_event.c"

typedef struct _vh_list_t
{
  void* userdata;

  vec_t* items;
  vec_t* cache;

  int head_index; // index of top element
  int tail_index; // index of bottom element
  int top_index;  // index of upper visible element
  int bot_index;  // index of bottom visible element
  int item_count; // all elements in data source
  int full;       // list is full, no more elements needed

  char lock_scroll;

  float item_wth; // width of all items
  float item_pos; // horizontal position of all items

  // scrollers

  view_t* vscr;   // vertical scroller
  view_t* hscr;   // horizontal scroller
  view_t* header; // header view
  float   header_size;

  uint32_t vtimeout; // vertical scroller timeout
  uint32_t htimeout; // horizontal scroller timeout

  view_t* (*create_item)(view_t* listview, void* userdata);                                       // pointer to item generator function
  int (*update_item)(view_t* listview, void* userdata, view_t* item, int index, int* item_count); // pointer to item updater function
} vh_list_t;

void    vh_list_add(view_t* view,
                    view_t* (*create_item)(view_t* listview, void* userdata),
                    int (*update_item)(view_t* listview, void* userdata, view_t* item, int index, int* item_count),
                    void* userdata);
void    vh_list_set_header(view_t* view, view_t* headerview);
vec_t*  vh_list_items(view_t* view);
vec_t*  vh_list_cache(view_t* view);
void    vh_list_fill(view_t* view);
void    vh_list_reset(view_t* view);
void    vh_list_refresh(view_t* view);
view_t* vh_list_item_for_index(view_t* view, int index);
void    vh_list_lock_scroll(view_t* view, char state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtcstring.c"
#include "mtstring.c"
#include "tg_css.c"
#include "tg_text.c"
#include "vh_anim.c"
#include "vh_sbar.c"
#include <math.h>

#define PRELOAD_DISTANCE 100.0

void vh_list_update_scrollbars(view_t* view)
{
  vh_list_t* vh = view->handler_data;

  float sr; // size ratio
  float pr; // position ratio
  float s;  // size
  float p;  // position

  /* // vertical scroller */

  sr = 1.0;
  pr = 0.0;

  if (vh->bot_index - vh->top_index + 1 < vh->item_count)
  {
    sr = (float)(vh->bot_index - vh->top_index + 1) / (float)vh->item_count;
    pr = (float)(vh->top_index) / (float)(vh->item_count - (vh->bot_index - vh->top_index + 1));
  }

  vh_sbar_update(vh->vscr, pr, sr);

  // horizontal scroller

  sr = 1.0;
  pr = 0.0;

  if (vh->item_wth > view->frame.local.w)
  {
    sr = view->frame.local.w / vh->item_wth;
    pr = -vh->item_pos / (vh->item_wth - view->frame.global.w);
  }

  vh_sbar_update(vh->hscr, pr, sr);
}

void vh_list_move(view_t* view, float dy)
{
  vh_list_t* vh = view->handler_data;

  char inside  = 0;
  char outside = 0;

  for (int index = 0; index < vh->items->length; index++)
  {
    view_t* sview = vh->items->data[index];
    r2_t    frame = sview->frame.local;

    frame.x = vh->item_pos;
    frame.y += dy;

    view_set_frame(sview, frame);

    if (frame.y >= 0.0 && !inside)
    {
      inside        = 1;
      vh->top_index = vh->head_index + index;
    }

    if (frame.y >= view->frame.local.h && !outside)
    {
      outside       = 1;
      vh->bot_index = vh->head_index + index;
    }
  }

  if (vh->header)
  {
    r2_t frame = vh->header->frame.local;
    frame.x    = vh->item_pos;
    view_set_frame(vh->header, frame);
  }

  if (!outside) vh->bot_index = vh->tail_index;
  if (!inside) vh->top_index = vh->head_index;

  vh_list_update_scrollbars(view);
}

view_t* vh_list_get_item(view_t* view)
{
  vh_list_t* vh = view->handler_data;
  if (vh->cache->length == 0)
  {
    view_t* item = (*vh->create_item)(view, vh->userdata);
    view_set_block_touch(item, 0, 1);
    VADD(vh->cache, item);
    view_insert(view, item, 0);
    //view_set_hidden(item, 1, 1);
    view_set_frame(item, (r2_t){0, -item->frame.local.h, item->frame.local.w, item->frame.local.h});
  }
  // TODO check TS_BLANK or TS_READY state!!!
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
        int     full = (*vh->update_item)(view, vh->userdata, item, 0, &vh->item_count);

        if (!full)
        {
          VREM(vh->cache, item);
          VADD(vh->items, item);
          //view_set_hidden(item, 0, 1);

          vh->item_wth = item->frame.global.w; // store maximum width

          view_set_frame(item, (r2_t){0, vh->header_size, item->frame.local.w, item->frame.local.h});
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
          int     full = (*vh->update_item)(view, vh->userdata, item, vh->head_index - 1, &vh->item_count);

          if (!full)
          {
            VREM(vh->cache, item);
            vec_ins(vh->items, item, 0);
            //view_set_hidden(item, 0, 1);

            vh->full     = 0;                    // there is probably more to come
            vh->item_wth = item->frame.global.w; // store maximum width
            vh->head_index -= 1;                 // decrease head index

            view_set_frame(item, (r2_t){0, head->frame.local.y - item->frame.local.h, item->frame.local.w, item->frame.local.h});
          }
          else
          {
            vh->full = 1;
          }
        }
        else
        {
          vh->full = 1;
        }

        if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h + PRELOAD_DISTANCE)
        {
          view_t* item = vh_list_get_item(view);
          int     full = (*vh->update_item)(view, vh->userdata, item, vh->tail_index + 1, &vh->item_count);

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
          {
            vh->full &= 1; // don't set to full if previously item is added
          }
        }
        else
        {
          vh->full &= 1; // don't set to full if previously item is added
        }

        // remove items if needed

        if (tail->frame.local.y - (head->frame.local.y + head->frame.local.h) > view->frame.local.h)
        {
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
    }
    // scroll bounce if needed
    if (vh->items->length > 0 && !vh->lock_scroll)
    {
      view_t* head = vec_head(vh->items);
      view_t* tail = vec_tail(vh->items);

      // horizontal bounce

      if (vh->item_pos > 0.0001)
      {
        vh->item_pos += -vh->item_pos / 5.0;
      }
      else if (vh->item_pos + vh->item_wth < view->frame.local.w - vh->vscr->frame.local.w)
      {
        if (vh->item_wth > view->frame.local.w - vh->vscr->frame.local.w)
        {
          vh->item_pos += (view->frame.local.w - vh->vscr->frame.local.w - vh->item_wth - vh->item_pos) / 5.0;
        }
        else if (vh->item_pos < -0.0001)
        {
          vh->item_pos += -vh->item_pos / 5.0;
        }
      }

      // vertical bounce

      if (head->frame.local.y > vh->header_size + 0.001)
      {
        vh_list_move(view, (vh->header_size - head->frame.local.y) / 5.0);
      }
      else if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h - 0.001 - vh->hscr->frame.local.h)
      {
        if (tail->frame.local.y + tail->frame.local.h - head->frame.local.y > view->frame.local.h - vh->hscr->frame.local.h)
        {
          vh_list_move(view, (view->frame.local.h - vh->hscr->frame.local.h - (tail->frame.local.y + tail->frame.local.h)) / 5.0);
        }
        else if (head->frame.local.y < -0.001)
        {
          vh_list_move(view, -head->frame.local.y / 5.0);
        }
      }
      else if (vh->item_pos > 0.001 || vh->item_pos < -0.0001)
      {
        vh_list_move(view, 0);
      }
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
    if (vh->items->length > 0 && !vh->lock_scroll)
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

void vh_list_lock_scroll(view_t* view, char state)
{
  vh_list_t* vh   = view->handler_data;
  vh->lock_scroll = state;
}

// cleanup all items, prepare for new content

void vh_list_reset(view_t* view)
{
  vh_list_t* vh = view->handler_data;

  vec_addinvector(vh->cache, vh->items);

  // remove all items from view

  for (int index = 0; index < vh->cache->length; index++)
  {
    view_t* sview = vh->cache->data[index];
    view_remove(view, sview);
  }

  vec_reset(vh->items);
  vec_reset(vh->cache);

  vh->head_index = 0;
  vh->tail_index = 0;
  vh->item_count = 0;
  vh->full       = 0;
}

void vh_list_scroll_v(view_t* view, void* userdata, float ratio)
{
  view_t*    listview  = userdata;
  vh_list_t* vh        = listview->handler_data;
  int        new_index = (int)((float)vh->item_count * ratio);

  if (new_index > 0 && new_index < vh->item_count)
  {
    vh->head_index = new_index;
    vh_list_refresh(listview);
    vh_list_update_scrollbars(listview);
  }
}

void vh_list_scroll_h(view_t* view, void* userdata, float ratio)
{
  view_t*    listview = userdata;
  vh_list_t* vh       = listview->handler_data;
  vh->item_pos        = -vh->item_wth * ratio;

  vh_list_move(listview, 0);
}

// reload visible items

void vh_list_refresh(view_t* view)
{
  vh_list_t* vh = view->handler_data;

  int full  = 0;
  int index = 0;

  for (index = 0; index < vh->items->length; index++)
  {
    view_t* item = vh->items->data[index];
    full         = (*vh->update_item)(view, vh->userdata, item, vh->head_index + index, &vh->item_count);
    if (full) break;
  }

  vh->top_index = vh->head_index;
  vh->bot_index = vh->head_index + index;

  // cache remaining items

  if (index < vh->items->length - 1)
  {

    for (; index < vh->items->length; index++)
    {
      view_t* item = vh->items->data[index];
      VADD(vh->cache, item);
      view_set_frame(item, (r2_t){0, -item->frame.local.h, item->frame.local.w, item->frame.local.h});
    }

    vec_reminvector(vh->items, vh->cache);
  }
}

void vh_list_add(view_t* view,
                 view_t* (*create_item)(view_t* listview, void* userdata),
                 int (*update_item)(view_t* listview, void* userdata, view_t* item, int index, int* item_count),
                 void* userdata)
{
  vh_list_t* vh   = mem_calloc(sizeof(vh_list_t), "vh_list", vh_list_del, NULL);
  vh->userdata    = userdata;
  vh->items       = VNEW();
  vh->cache       = VNEW();
  vh->create_item = create_item;
  vh->update_item = update_item;

  char* vid = cstr_fromformat("%s%s", view->id, "vscr", NULL);
  char* hid = cstr_fromformat("%s%s", view->id, "hscr", NULL);

  view_t* vscr = view_new(vid, (r2_t){view->frame.local.w - 18, 0, 15, view->frame.local.h});
  view_t* hscr = view_new(hid, (r2_t){0, view->frame.local.h - 18, view->frame.local.w, 15});

  REL(vid);
  REL(hid);

  vscr->layout.h_per  = 1.0;
  vscr->layout.right  = 3;
  hscr->layout.w_per  = 1.0;
  hscr->layout.bottom = 3;

  vscr->layout.background_color = 0x00000001;
  hscr->layout.background_color = 0x00000001;

  tg_css_add(vscr);
  tg_css_add(hscr);

  vh_sbar_add(vscr, SBAR_V, 30, vh_list_scroll_v, view);
  vh_sbar_add(hscr, SBAR_H, 30, vh_list_scroll_h, view);

  view_add(view, vscr);
  view_add(view, hscr);

  vh->vscr = vscr;
  vh->hscr = hscr;

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

void vh_list_set_header(view_t* view, view_t* headerview)
{
  vh_list_t* vh = view->handler_data;

  if (vh->header != NULL)
  {
    view_remove(view, vh->header);
    REL(vh->header);
  }
  RET(headerview);
  vh->header = headerview;

  // add as subview
  view_add(view, headerview);
  vh->header_size = headerview->frame.local.h;

  if (vh->vscr)
  {
    r2_t frame = vh->vscr->frame.local;
    frame.y    = vh->header_size;
    frame.h    = frame.h - vh->header_size;
    view_set_frame(vh->vscr, frame);
  }
}

#endif
