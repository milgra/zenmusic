#ifndef itemlist_h
#define itemlist_h

#include "view.c"

void itemlist_new(view_t* view, vec_t* items, char* fontpath, void (*on_select)(int));

#endif

#if __INCLUDE_LEVEL__ == 0

/* #include "cr_text.c" */
/* #include "vh_list.c" */
/* #include "vh_list_item.c" */
/* #include "view_util.c" */

/* typedef struct _itemlist_t */
/* { */
/*   vec_t*      items; */
/*   view_t*     view; */
/*   textstyle_t textstyle; */

/*   void (*on_select)(char* id); */
/* } itemlist_t; */

/* view_t* itemlist_create_item(view_t* listview); */
/* int     itemlist_update_item(view_t* listview, view_t* item, int index, int* item_count); */

/* void itemlist_del(void* p) */
/* { */
/*   itemlist_t* il = p; */
/* } */

/* void itemlist_new(view_t* view, vec_t* items, char* fontpath, void (*on_select)(int)) */
/* { */
/*   itemlist_t* il = mem_calloc(sizeof(itemlist_t), "itemlist", itemlist_del, NULL); */

/*   il->items = items; */

/*   vh_list_add(view, itemlist_create_item, itemlist_update_item); */

/*   il->view                   = view; */
/*   il->textstyle.font         = fontpath; */
/*   il->textstyle.align        = TA_RIGHT; */
/*   il->textstyle.margin_right = 20; */
/*   il->textstyle.size         = 25.0; */
/*   il->textstyle.textcolor    = 0x000000FF; */
/*   il->textstyle.backcolor    = 0xFFFFFFFF; */

/*   return il; */
/* } */

/* void itemlist_update(itemlist_t* il) */
/* { */
/*   vh_list_reset(il->view); */
/* } */

/* void on_itemitem_select(view_t* view, void* data, uint32_t index, ev_t ev) */
/* { */
/*   itemlist_t* il = data; */
/*   printf("on_itemitem_select\n"); */
/*   (*il->on_select)(il->items->data[index]); */
/* } */

/* view_t* itemlist_create_item(view_t* listview, void* data) */
/* { */
/*   itemlist_t* il = data; */

/*   static int item_cnt      = 0; */
/*   char       idbuffer[100] = {0}; */
/*   snprintf(idbuffer, 100, "itemlist_item%i", item_cnt++); */

/*   view_t* rowview = view_new(idbuffer, (r2_t){0, 0, 0, 35}); */
/*   rowview->hidden = 1; */

/*   vh_litem_add(rowview, 35, on_itemitem_select, il); */
/*   vh_litem_add_cell(rowview, "item", 230, cr_text_add, cr_text_upd); */

/*   return rowview; */
/* } */

/* int itemlist_update_item(view_t* listview, void* data, view_t* item, int index, int* item_count) */
/* { */
/*   itemlist_t* il = data; */
/*   if (index < 0) */
/*     return 1; // no items before 0 */
/*   if (index >= il->items->length) */
/*     return 1; // no more items */

/*   *item_count = il->items->length; */

/*   vh_litem_upd_cell(item, "item", &((cr_text_data_t){.style = il->textstyle, .text = il->items->data[index]})); */

/*   return 0; */
/* } */

#endif
