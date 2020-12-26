#ifndef vh_list_item_h
#define vh_list_item_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "view.c"

typedef struct _lheadcell_t
{
  char* id;
  int   size;
  void (*upd)(view_t* view, void* data);
  view_t* view;
} lheadcell_t;

typedef struct _vh_lhead_t
{
  vec_t*       cells;
  view_t*      view;
  lheadcell_t* dragged;
  int          height;
  int          index;
  void (*on_select)(view_t* view, uint32_t index);
} vh_lhead_t;

void vh_lhead_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index));
void vh_lhead_upd(view_t* view, int index);
void vh_lhead_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data));
void vh_lhead_upd_cell(view_t* view, char* id, int size, void* data);
void vh_lhead_rem_cell(char* id);
void vh_lhead_swp_cell(char* ida, char* idb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

void vh_lhead_del(void* p)
{
  vh_lhead_t* list = p;
  REL(list->cells);
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
      if (f.x < ev.x && f.x + f.w > ev.x)
      {
        printf("ON CELL %s\n", cell->id);
        vh->dragged = cell;
      }
    }
  }
  else if (ev.type == EV_MMOVE)
  {
    // resize or move cell
    if (ev.drag)
    {
      vh_lhead_t* vh = view->handler_data;
      r2_t        f  = vh->dragged->view->frame.local;
      f.x            = ev.x;
      view_set_frame(vh->dragged->view, f);
    }
  }
  else if (ev.type == EV_MUP)
  {
    if (!ev.drag)
    {
      vh_lhead_t* vh = view->handler_data;
      (*vh->on_select)(view, vh->index);
    }
  }
}

void vh_lhead_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index))
{
  vh_lhead_t* vh = mem_calloc(sizeof(vh_lhead_t), "vh_lhead_t", vh_lhead_del, NULL);
  vh->cells      = VNEW();
  vh->height     = h;
  vh->on_select  = on_select;

  view->handler_data = vh;
  view->handler      = vh_lhead_evt;
}

void vh_lhead_upd(view_t* view, int index)
{
  vh_lhead_t* vh = view->handler_data;
  vh->index      = index;
}

void vh_lhead_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data))
{
  vh_lhead_t* vh = view->handler_data;

  lheadcell_t* cell = mem_alloc(sizeof(lheadcell_t), "lheadcell_t", NULL, NULL);
  cell->id          = cstr_fromcstring(id);
  cell->size        = size;
  cell->upd         = upd;

  lheadcell_t* last = vec_tail(vh->cells);
  float        x    = last == NULL ? 0 : (last->view->frame.local.x + last->view->frame.local.w + 1);

  view_t* cellview = view_new(cstr_fromformat("%s%s", view->id, id, NULL), (r2_t){x, 0, size, vh->height});

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

void vh_lhead_upd_cell(view_t* view, char* id, int size, void* data)
{
  vh_lhead_t* vh = view->handler_data;

  for (int i = 0; i < vh->cells->length; i++)
  {
    lheadcell_t* cell = vh->cells->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      (*cell->upd)(cell->view, data);
      break;
    }
  }
}

#endif
