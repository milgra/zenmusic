#ifndef vh_item_h
#define vh_item_h

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

typedef struct _vh_item_t
{
  vec_t*   cells;
  view_t*  view;
  uint32_t index;
  void (*on_select)(view_t* view, uint32_t index);
} vh_item_t;

void vh_item_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index));
void vh_item_set_index(view_t* view, uint32_t index);
void vh_item_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data));
void vh_item_upd_cell(view_t* view, char* id, int size, void* data);
void vh_item_rem_cell(char* id);
void vh_item_swp_cell(char* ida, char* idb);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"

void vh_item_del(void* p)
{
  vh_item_t* list = p;
  REL(list->cells);
}

void vh_item_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MDOWN)
  {
    vh_item_t* vh = view->handler_data;
    (*vh->on_select)(view, vh->index);
  }
}

void vh_item_add(view_t* view, int h, void (*on_select)(view_t* view, uint32_t index))
{
  vh_item_t* vh = mem_calloc(sizeof(vh_item_t), "vh_item_t", vh_item_del, NULL);
  vh->cells     = VNEW();
  vh->on_select = on_select;

  view->handler_data = vh;
  view->handler      = vh_item_evt;
}

void vh_item_add_cell(view_t* view, char* id, int size, void (*upd)(view_t* view, void* data))
{
  vh_item_t* vh = view->handler_data;

  cell_t* cell = mem_alloc(sizeof(cell_t), "cell_t", NULL, NULL);
  cell->id     = cstr_fromcstring(id);
  cell->size   = size;
  cell->upd    = upd;

  view_t* cellview = view_new(cstr_fromformat("%s%s", view->id, id, NULL), (r2_t){view->frame.local.w, 0, size, view->frame.local.h});

  cell->view = cellview;

  view_add(view, cellview);

  // store cell
  VADD(vh->cells, cell);

  // increase item size
  r2_t local = view->frame.local;
  local.w += size;
  view_set_frame(view, local);
}

void vh_item_upd_cell(view_t* view, char* id, int size, void* data)
{
  vh_item_t* vh = view->handler_data;

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
