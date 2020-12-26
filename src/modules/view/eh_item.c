#ifndef evthan_item_h
#define evthan_item_h

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

typedef struct _eh_item_t
{
  vec_t*   cells;
  view_t*  view;
  uint32_t index;
  void (*on_select)(view_t* view, uint32_t index);
} eh_item_t;

void eh_item_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index));
void eh_item_set_index(view_t* view, uint32_t index);
void eh_item_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data));
void eh_item_upd_cell(view_t* view, char* id, int size, void* data);
void eh_item_rem_cell(char* id);
void eh_item_swp_cell(char* ida, char* idb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

void eh_item_del(void* p)
{
  eh_item_t* list = p;
  REL(list->cells);
}

void eh_item_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    eh_item_t* eh = view->evt_han_data;
    (*eh->on_select)(view, eh->index);
  }
}

void eh_item_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index))
{
  eh_item_t* eh = mem_calloc(sizeof(eh_item_t), "eh_item_t", eh_item_del, NULL);
  eh->cells     = VNEW();
  eh->on_select = on_select;

  view->evt_han_data = eh;
  view->evt_han      = eh_item_evt;
}

void eh_item_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data))
{
  eh_item_t* eh = view->evt_han_data;

  cell_t* cell = mem_alloc(sizeof(cell_t), "cell_t", NULL, NULL);
  cell->id     = cstr_fromcstring(id);
  cell->size   = size;
  cell->upd    = upd;

  view_t* cellview = view_new(cstr_fromformat("%s%s", view->id, id, NULL), (r2_t){view->frame.local.w, 0, size, view->frame.local.h});

  cell->view = cellview;

  view_add(view, cellview);

  // store cell
  VADD(eh->cells, cell);

  // increase item size
  r2_t local = view->frame.local;
  local.w += size;
  view_set_frame(view, local);
}

void eh_item_upd_cell(view_t* view, char* id, int size, void* data)
{
  eh_item_t* eh = view->evt_han_data;

  for (int i = 0; i < eh->cells->length; i++)
  {
    cell_t* cell = eh->cells->data[i];
    if (strcmp(cell->id, id) == 0)
    {
      (*cell->upd)(cell->view, data);
      break;
    }
  }
}

#endif
