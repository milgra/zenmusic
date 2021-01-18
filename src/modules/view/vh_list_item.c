#ifndef vh_list_item2_h
#define vh_list_item2_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "vh_list_cell.c"
#include "view.c"

typedef struct _vh_litem_t
{
  view_t* view;
  int     index;
  vec_t*  cells;
  void*   userdata;

  vh_lcell_t* sel_cell;
  ev_t        sel_ev;
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

void vh_litem_evt(view_t* view, ev_t ev);
void vh_litem_del(void* p);

void vh_litem_add(view_t* view, void* userdata, void (*on_select)(view_t* view))
{
  vh_litem_t* vh = mem_calloc(sizeof(vh_litem_t), "vh_litem_t", vh_litem_del, NULL);
  vh->cells      = VNEW();
  vh->userdata   = userdata;
  vh->on_select  = on_select;

  view->handler_data = vh;
  view->handler      = vh_litem_evt;
}

void vh_litem_del(void* p)
{
  vh_litem_t* list = p;
  REL(list->cells);
}

void vh_litem_resize(view_t* view)
{
  vh_litem_t* vh = view->handler_data;

  vh_lcell_t* last   = vec_tail(vh->cells);
  r2_t        lframe = last->view->frame.local;
  r2_t        vframe = view->frame.local;
  vframe.w           = lframe.x + lframe.w;

  view_set_frame(view, vframe);
}

void vh_litem_upd_index(view_t* view, int index)
{
  vh_litem_t* vh = view->handler_data;
  vh->index      = index;
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

// cell handling

void vh_litem_add_cell(view_t* view, char* id, int size, view_t* cellview)
{
  vh_litem_t* vh   = view->handler_data;
  vh_lcell_t* cell = vh_lcell_new(id, size, cellview, vh->cells->length);

  // add subview
  view_add(view, cellview);

  // store cell
  VADD(vh->cells, cell);

  // arrange and resize
  vh_lcell_arrange(vh->cells);
  vh_litem_resize(view);
}

view_t* vh_litem_get_cell(view_t* view, char* id)
{
  vh_litem_t* vh = view->handler_data;
  return vh_lcell_get(vh->cells, id);
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
  vh_litem_resize(view);
}

void vh_litem_upd_cell_size(view_t* view, char* id, int size)
{
  vh_litem_t* vh = view->handler_data;
  vh_lcell_set_size(vh->cells, id, size);
  vh_litem_resize(view);
}

#endif
