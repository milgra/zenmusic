#ifndef vh_list_item2_h
#define vh_list_item2_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "view.c"

typedef struct _vh_litem_cell_t
{
  char*   id;
  int     size;
  view_t* view;
} vh_litem_cell_t;

typedef struct _vh_litem2_t
{
  int              index;
  vh_litem_cell_t* sel_cell;
  ev_t             sel_ev;
  vec_t*           cells;
  view_t*          view;
  void*            userdata;
  void (*on_select)(view_t* view);
} vh_litem2_t;

void vh_litem2_add(view_t* view, void* userdata, void (*on_select)(view_t* view));
void vh_litem2_upd_index(view_t* view, int index);

void    vh_litem2_add_cell(view_t* view, char* id, int size, view_t* cellview);
view_t* vh_litem2_get_cell(view_t* view, char* id);
void    vh_litem2_rem_cell(char* id);
void    vh_litem2_swp_cell(view_t* view, int src, int tgt);
void    vh_litem2_rpl_cell(view_t* view, char* id, view_t* newcell);
void    vh_litem2_upd_cell_size(view_t* view, char* id, int size);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

void vh_litem2_del(void* p)
{
  vh_litem2_t* list = p;
  REL(list->cells);
}

void vh_litem2_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_litem2_t* vh = view->handler_data;
    vh->sel_cell    = NULL;
    vh->sel_ev      = ev;

    // get selected cell
    for (int index = 0; index < vh->cells->length; index++)
    {
      vh_litem_cell_t* cell = vh->cells->data[index];
      if (ev.x > cell->view->frame.global.x && ev.x < cell->view->frame.global.x + cell->view->frame.global.w)
      {
        vh->sel_cell = cell;
        break;
      }
    }

    (*vh->on_select)(view);
  }
}

void vh_litem2_add(view_t* view, void* userdata, void (*on_select)(view_t* view))
{
  vh_litem2_t* vh = mem_calloc(sizeof(vh_litem2_t), "vh_litem2_t", vh_litem2_del, NULL);
  vh->cells       = VNEW();
  vh->userdata    = userdata;
  vh->on_select   = on_select;

  view->handler_data = vh;
  view->handler      = vh_litem2_evt;
}

void vh_litem2_upd_index(view_t* view, int index)
{
  vh_litem2_t* vh = view->handler_data;
  vh->index       = index;
}

void vh_litem2_add_cell(view_t* view, char* id, int size, view_t* cellview)
{
  vh_litem2_t* vh = view->handler_data;

  vh_litem_cell_t* cell = mem_alloc(sizeof(vh_litem_cell_t), "vh_litem_cell_t", NULL, NULL);
  cell->id              = cstr_fromcstring(id);
  cell->size            = size;
  cell->view            = cellview;

  view_add(view, cellview);

  // store cell
  VADD(vh->cells, cell);

  // set cell position
  r2_t frame = cellview->frame.local;
  frame.x    = view->frame.local.w;
  view_set_frame(cellview, frame);

  // increase item size
  r2_t local = view->frame.local;
  local.w += size;
  view_set_frame(view, local);
}

view_t* vh_litem2_get_cell(view_t* view, char* id)
{
  vh_litem2_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    vh_litem_cell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      return cell->view;
    }
  }
  return NULL;
}

void vh_litem2_rearrange(view_t* view)
{
  vh_litem2_t* vh = view->handler_data;
  float        x  = 0;

  for (int index = 0;
       index < vh->cells->length;
       index++)
  {
    vh_litem_cell_t* cell = vh->cells->data[index];

    r2_t f = cell->view->frame.local;
    f.x    = x;
    x      = x + f.w;
    view_set_frame(cell->view, f);

    printf("rearrange %s %f\n", cell->id, x);
  }
}

void vh_litem2_swp_cell(view_t* view, int src, int tgt)
{
  vh_litem2_t* vh = view->handler_data;

  vh_litem_cell_t* cell = vh->cells->data[src];
  VREM(vh->cells, cell);
  vec_addatindex(vh->cells, cell, tgt);
  vh_litem2_rearrange(view);
}

void vh_litem2_rpl_cell(view_t* view, char* id, view_t* newcell)
{
  vh_litem2_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    vh_litem_cell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      printf("replacing cell view for %s\n", cell->id);
      view_add(view, newcell);
      view_remove(view, cell->view);

      cell->view = newcell;
      break;
    }
  }
  vh_litem2_rearrange(view);
}

void vh_litem2_upd_cell_size(view_t* view, char* id, int size)
{
  vh_litem2_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    vh_litem_cell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      r2_t f = cell->view->frame.local;
      f.w    = size;
      view_set_frame(cell->view, f);
      break;
    }
  }
  vh_litem2_rearrange(view);
}

#endif
