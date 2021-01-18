#ifndef vh_list_item2_h
#define vh_list_item2_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "vh_list_cell.c"
#include "view.c"

typedef struct _vh_litem_t
{
  int         index;
  vh_lcell_t* sel_cell;
  ev_t        sel_ev;
  vec_t*      cells;
  view_t*     view;
  void*       userdata;
  void (*on_select)(view_t* view);
} vh_litem_t;

void vh_litem_add(view_t* view, void* userdata, void (*on_select)(view_t* view));
void vh_litem_upd_index(view_t* view, int index);

void    vh_litem_add_cell(view_t* view, char* id, int size, view_t* cellview);
view_t* vh_litem_get_cell(view_t* view, char* id);
void    vh_litem_rem_cell(char* id);
void    vh_litem_swp_cell(view_t* view, int src, int tgt);
void    vh_litem_rpl_cell(view_t* view, char* id, view_t* newcell);
void    vh_litem_upd_cell_size(view_t* view, char* id, int size);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

// cell related functions

view_t* vh_litem_get_cell(view_t* view, char* id)
{
  vh_litem_t* vh = view->handler_data;
  return vh_lcell_get(vh->cells, id);
}

void vh_litem_del(void* p)
{
  vh_litem_t* list = p;
  REL(list->cells);
}

void vh_litem_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_litem_t* vh = view->handler_data;
    vh->sel_cell   = NULL;
    vh->sel_ev     = ev;

    // get selected cell
    for (int index = 0; index < vh->cells->length; index++)
    {
      vh_lcell_t* cell = vh->cells->data[index];
      if (ev.x > cell->view->frame.global.x && ev.x < cell->view->frame.global.x + cell->view->frame.global.w)
      {
        vh->sel_cell = cell;
        break;
      }
    }

    (*vh->on_select)(view);
  }
}

void vh_litem_add(view_t* view, void* userdata, void (*on_select)(view_t* view))
{
  vh_litem_t* vh = mem_calloc(sizeof(vh_litem_t), "vh_litem_t", vh_litem_del, NULL);
  vh->cells      = VNEW();
  vh->userdata   = userdata;
  vh->on_select  = on_select;

  view->handler_data = vh;
  view->handler      = vh_litem_evt;
}

void vh_litem_upd_index(view_t* view, int index)
{
  vh_litem_t* vh = view->handler_data;
  vh->index      = index;
}

void vh_litem_add_cell(view_t* view, char* id, int size, view_t* cellview)
{
  vh_litem_t* vh = view->handler_data;

  vh_lcell_t* cell = mem_alloc(sizeof(vh_lcell_t), "vh_lcell_t", NULL, NULL);
  cell->id         = cstr_fromcstring(id);
  cell->size       = size;
  cell->view       = cellview;
  cell->index      = vh->cells->length;

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

void vh_litem_swp_cell(view_t* view, int src, int tgt)
{
  vh_litem_t* vh = view->handler_data;
  vh_lcell_ins(vh->cells, src, tgt);
}

void vh_litem_rpl_cell(view_t* view, char* id, view_t* newcell)
{
  vh_litem_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    vh_lcell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      printf("replacing cell view for %s\n", cell->id);
      view_add(view, newcell);
      view_remove(view, cell->view);

      cell->view = newcell;
      break;
    }
  }
  vh_lcell_arrange(vh->cells);
}

void vh_litem_upd_cell_size(view_t* view, char* id, int size)
{
  vh_litem_t* vh = view->handler_data;

  for (int index = 0; index < vh->cells->length; index++)
  {
    vh_lcell_t* cell = vh->cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      r2_t f = cell->view->frame.local;
      f.w    = size;
      view_set_frame(cell->view, f);
      break;
    }
  }
  vh_lcell_arrange(vh->cells);
}

#endif
