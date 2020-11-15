#ifndef view_gen_h
#define view_gen_h

#include "mtvector.c"
#include "view.c"

mtvec_t* view_gen_load(char* htmlpath, char* csspath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "html.c"

void view_gen_apply_style(view_t* view, mtmap_t* style)
{
  mtvec_t* keys = mtmap_keys(style);
  for (int index = 0; index < keys->length; index++)
  {
    char* key = keys->data[index];
    char* val = MGET(style, key);
    printf("key %s val %s\n", key, val);

    if (strcmp(key, "background-color") == 0)
    {
      int color                     = (int)strtol(val + 1, NULL, 16);
      view->layout.background_color = color;
      tg_color_add(view, color);
    }
    else if (strcmp(key, "width") == 0)
    {
      if (strstr(val, "%") != NULL)
      {
        char* end          = strstr(val, "%");
        int   len          = end - val;
        end[len - 1]       = '\0';
        int per            = atoi(val);
        view->layout.w_per = (float)per / 100.0;
      }
      else if (strstr(val, "px") != NULL)
      {
        char* end          = strstr(val, "px");
        int   len          = end - val;
        end[len - 1]       = '\0';
        int pix            = atoi(val);
        view->layout.width = pix;
      }
    }
    else if (strcmp(key, "height") == 0)
    {
      if (strstr(val, "%") != NULL)
      {
        char* end          = strstr(val, "%");
        int   len          = end - val;
        end[len - 1]       = '\0';
        int per            = atoi(val);
        view->layout.h_per = (float)per / 100.0;
      }
      else if (strstr(val, "px") != NULL)
      {
        char* end           = strstr(val, "px");
        int   len           = end - val;
        end[len - 1]        = '\0';
        int pix             = atoi(val);
        view->layout.height = pix;
      }
    }
  }
  printf("layout for %s: ", view->id);
  view_desc_layout(view->layout);
}

mtvec_t* view_gen_load(char* htmlpath, char* csspath)
{
  char* html = html_read(htmlpath);
  char* css  = html_read(csspath);

  tag_t*  view_structure = html_parse_html(html);
  prop_t* view_styles    = html_parse_css(css);

  // create style map
  mtmap_t* styles = MNEW();
  prop_t*  props  = view_styles;
  while ((*props).class.len > 0)
  {
    prop_t t   = *props;
    char*  cls = mtmem_calloc(sizeof(char) * t.class.len + 1, "char*", NULL, mtcstr_describe);
    char*  key = mtmem_calloc(sizeof(char) * t.key.len + 1, "char*", NULL, mtcstr_describe);
    char*  val = mtmem_calloc(sizeof(char) * t.value.len + 1, "char*", NULL, mtcstr_describe);
    memcpy(cls, css + t.class.pos, t.class.len);
    memcpy(key, css + t.key.pos, t.key.len);
    memcpy(val, css + t.value.pos, t.value.len);
    mtmap_t* style = MGET(styles, cls);
    if (style == NULL)
    {
      style = MNEW();
      MPUT(styles, cls, style);
    }
    MPUT(style, key, val);
    props += 1;
  }

  // create view structure
  mtvec_t* views = VNEW();
  tag_t*   tags  = view_structure;
  while ((*tags).len > 0)
  {
    tag_t t = *tags;
    if (t.id.len > 0)
    {
      char* id = mtmem_calloc(sizeof(char) * t.id.len + 1, "char*", NULL, NULL);
      memcpy(id, html + t.id.pos + 1, t.id.len);
      view_t* view = view_new(id, (vframe_t){0});
      VADD(views, view);
      if (t.level > 0) // add view to paernt
      {
        view_t* parent = views->data[t.parent];
        //printf("parent %i %i\n", t.parent, views->length);
        view_add(parent, view);
      }

      char cssid[100] = {0};
      snprintf(cssid, 100, "#%s", id);

      mtmap_t* style = MGET(styles, cssid);
      if (style)
      {

        view_gen_apply_style(view, style);
        // apply style to view
      }

      if (t.class.len > 0)
      {
        char csscls[100] = {0};
        snprintf(csscls, 100, ".%s", id);

        style = MGET(styles, csscls);
        if (style)
        {
          printf("style for %s : %i\n", cssid, style->count);
          mtmem_describe(style, 0);
          // apply style to view
        }
      }

      REL(id);
    }
    else
    {
      // idless view, probably </div>
      view_t* view = view_new("", (vframe_t){0});
      VADD(views, view);
    }
    tags += 1;
  }

  return views;
}

#endif
