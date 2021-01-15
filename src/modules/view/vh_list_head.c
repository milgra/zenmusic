// TODO it should re-use list_item parts

#ifndef vh_list_head_h
#define vh_list_head_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "view.c"

typedef struct _lheadcell_t
{
  char*   id;
  int     size;
  view_t* view;
} lheadcell_t;

typedef struct _vh_lhead_t
{
  vec_t*       cells;
  view_t*      view;
  lheadcell_t* dragged_cell;
  int          dragged_pos;
  int          dragged_flag;
  int          dragged_ind;
  lheadcell_t* resized_cell;
  lheadcell_t* resized_flag;
  int          height;
  int          index;
  void (*on_select)(view_t* view, char* id, ev_t ev);
  void (*on_insert)(view_t* view, int src, int tgt);
  void (*on_resize)(view_t* view, char* id, int width);
} vh_lhead_t;

void    vh_lhead_add(view_t* view, int h, void (*on_select)(view_t* view, char* id, ev_t ev), void (*on_insert)(view_t* view, int src, int tgt), void (*on_resize)(view_t* view, char* id, int width));
view_t* vh_lhead_get_cell(view_t* view, char* id);
void    vh_lhead_add_cell(view_t* view, char* id, int size, view_t* cellview);
void    vh_lhead_rem_cell(char* id);
void    vh_lhead_swp_cell(char* ida, char* idb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

void vh_lhead_del(void* p)
{
  vh_lhead_t* list = p;
  REL(list->cells);
}

void vh_lhead_rearrange(view_t* view)
{
  vh_lhead_t* vh  = view->handler_data;
  float       pos = 0;
  for (int i = 0; i < vh->cells->length; i++)
  {
    lheadcell_t* cell = vh->cells->data[i];
    r2_t         f    = cell->view->frame.local;
    f.x               = pos;
    pos += f.w + 1;
    view_set_frame(cell->view, f);
  }
}

void vh_lhead_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    // check cell or cell border intersection
    vh_lhead_t* vh = view->handler_data;
    for (int i = 0; i < vh->cells->length; i++)
    {
      lheadcell_t* cell = vh->cells->data[i];
      r2_t         f    = cell->view->frame.local;
      if (f.x < ev.x && f.x + f.w + 1 > ev.x)
      {
        if (ev.x > f.x + f.w - 10)
        {
          vh->resized_cell = cell;
        }
        else
        {
          vh->dragged_cell = cell;
          vh->dragged_pos  = ev.x - f.x;
          vh->dragged_ind  = i;
        }
        view_remove(view, cell->view);
        view_add(view, cell->view);
        break;
      }
    }
  }
  else if (ev.type == EV_MMOVE)
  {
    // resize or move cell
    if (ev.drag)
    {
      vh_lhead_t* vh = view->handler_data;

      if (vh->resized_cell)
      {
        r2_t f = vh->resized_cell->view->frame.local;
        f.w    = ev.x - f.x;
        view_set_frame(vh->resized_cell->view, f);
        vh_lhead_rearrange(view);
      }
      else if (vh->dragged_cell)
      {
        vh->dragged_flag = 1;
        r2_t f           = vh->dragged_cell->view->frame.local;
        f.x              = ev.x - vh->dragged_pos;
        view_set_frame(vh->dragged_cell->view, f);
      }
    }
  }
  else if (ev.type == EV_MUP)
  {
    vh_lhead_t* vh = view->handler_data;
    if (!vh->dragged_flag)
    {
      if (vh->resized_cell)
      {
        (*vh->on_resize)(view, vh->resized_cell->id, vh->resized_cell->view->frame.local.w);
      }
      else if (vh->dragged_cell)
      {
        (*vh->on_select)(view, vh->dragged_cell->id, ev);
      }
    }
    else
    {
      for (int i = 0; i < vh->cells->length; i++)
      {
        lheadcell_t* cell = vh->cells->data[i];
        r2_t         f    = cell->view->frame.local;
        if (f.x < ev.x && f.x + f.w > ev.x && cell != vh->dragged_cell)
        {
          RET(vh->dragged_cell);
          vec_rem(vh->cells, vh->dragged_cell);
          vec_addatindex(vh->cells, vh->dragged_cell, i);
          // cleanup
          REL(vh->dragged_cell);

          (*vh->on_insert)(view, vh->dragged_ind, i);

          break;
        }
      }
    }
    vh->dragged_cell = NULL;
    vh->resized_cell = NULL;
    vh->dragged_flag = 0;
    vh_lhead_rearrange(view);
  }
}

void vh_lhead_add(view_t* view,
                  int     h,
                  void (*on_select)(view_t* view, char* id, ev_t ev),
                  void (*on_insert)(view_t* view, int src, int tgt),
                  void (*on_resize)(view_t* view, char* id, int width))
{
  vh_lhead_t* vh = mem_calloc(sizeof(vh_lhead_t), "vh_lhead_t", vh_lhead_del, NULL);
  vh->cells      = VNEW();
  vh->height     = h;
  vh->on_select  = on_select;
  vh->on_insert  = on_insert;
  vh->on_resize  = on_resize;

  view->handler_data = vh;
  view->handler      = vh_lhead_evt;
}

view_t* vh_lhead_get_cell(view_t* view, char* id)
{
  vh_lhead_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    lheadcell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      return cell->view;
    }
  }
  return NULL;
}

void vh_lhead_add_cell(view_t* view, char* id, int size, view_t* cellview)
{
  vh_lhead_t* vh = view->handler_data;

  lheadcell_t* cell = mem_alloc(sizeof(lheadcell_t), "lheadcell_t", NULL, NULL);
  cell->id          = cstr_fromcstring(id);
  cell->size        = size;

  lheadcell_t* last = vec_tail(vh->cells);
  float        x    = last == NULL ? 0 : (last->view->frame.local.x + last->view->frame.local.w + 1);

  // set cell position
  r2_t frame = cellview->frame.local;
  frame.x    = x;
  view_set_frame(cellview, frame);

  cell->view = cellview;

  cellview->needs_touch = 0;

  view_add(view, cellview);

  // store cell
  VADD(vh->cells, cell);

  // increase item size
  r2_t local = view->frame.local;
  local.w    = cellview->frame.local.x + cellview->frame.local.w;
  view_set_frame(view, local);
}

#endif
