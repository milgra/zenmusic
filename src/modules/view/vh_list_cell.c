#ifndef vh_list_cell_h
#define vh_list_cell_h

#include "view.c"

typedef struct _vh_lcell_t
{
  char*   id;
  int     size;
  view_t* view;
  int     index;
} vh_lcell_t;

void    vh_lcell_arrange(vec_t* cells);
view_t* vh_lcell_get(vec_t* cells, char* id);
void    vh_lcell_ins(vec_t* cells, int si, int ti);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtvector.c"

void vh_lcell_arrange(vec_t* cells)
{
  float pos = 0;
  for (int i = 0; i < cells->length; i++)
  {
    vh_lcell_t* cell = cells->data[i];
    r2_t        f    = cell->view->frame.local;
    f.x              = pos;
    pos += f.w + 1;
    view_set_frame(cell->view, f);
  }
}

view_t* vh_lcell_get(vec_t* cells, char* id)
{
  for (int index = 0; index < cells->length; index++)
  {
    vh_lcell_t* cell = cells->data[index];
    if (strcmp(cell->id, id) == 0)
    {
      return cell->view;
    }
  }
  return NULL;
}

void vh_lcell_ins(vec_t* cells, int si, int ti)
{
  vh_lcell_t* cell = cells->data[si];
  RET(cell); // increase retain count for the swap
  VREM(cells, cell);
  vec_addatindex(cells, cell, ti);
  REL(cell);
  vh_lcell_arrange(cells);
}

#endif
