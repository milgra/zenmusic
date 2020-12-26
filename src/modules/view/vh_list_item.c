#ifndef vh_list_item_h
#define vh_list_item_h

#include "mtcstring.c"
#include "mtmap.c"
#include "mtvector.c"
#include "view.c"

typedef struct _cell_t
{
  char* id;
  int   size;
  void (*upd)(view_t* view, void* data);
  view_t* view;
} cell_t;

typedef struct _vh_litem_t
{
  vec_t*  cells;
  view_t* view;
  int     index;
  void (*on_select)(view_t* view, uint32_t index);
} vh_litem_t;

void vh_litem_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index));
void vh_litem_upd(view_t* view, int index);
void vh_litem_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data));
void vh_litem_upd_cell(view_t* view, char* id, int size, void* data);
void vh_litem_rem_cell(char* id);
void vh_litem_swp_cell(char* ida, char* idb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

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
    (*vh->on_select)(view, vh->index);
  }
}

void vh_litem_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index))
{
  vh_litem_t* vh = mem_calloc(sizeof(vh_litem_t), "vh_litem_t", vh_litem_del, NULL);
  vh->cells      = VNEW();
  vh->on_select  = on_select;

  view->handler_data = vh;
  view->handler      = vh_litem_evt;
}

void vh_litem_upd(view_t* view, int index)
{
  vh_litem_t* vh = view->handler_data;
  vh->index      = index;
}

void vh_litem_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data))
{
  vh_litem_t* vh = view->handler_data;

  cell_t* cell = mem_alloc(sizeof(cell_t), "cell_t", NULL, NULL);
  cell->id     = cstr_fromcstring(id);
  cell->size   = size;
  cell->upd    = upd;

  view_t* cellview = view_new(cstr_fromformat("%s%s", view->id, id, NULL), (r2_t){view->frame.local.w, 0, size, view->frame.local.h});

  cell->view = cellview;

  cellview->needs_touch = 0;

  view_add(view, cellview);

  // store cell
  VADD(vh->cells, cell);

  // increase item size
  r2_t local = view->frame.local;
  local.w += size;
  view_set_frame(view, local);
}

void vh_litem_upd_cell(view_t* view, char* id, int size, void* data)
{
  vh_litem_t* vh = view->handler_data;

  for (int i = 0; i < vh->cells->length; i++)
  {
    cell_t* cell = vh->cells->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      (*cell->upd)(cell->view, data);
      break;
    }
  }
}

#endif
