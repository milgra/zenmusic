#ifndef view_gen_h
#define view_gen_h

#include "mtvector.c"
#include "view.c"

vec_t* view_gen_load(char* htmlpath, char* csspath, char* respath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "html.c"
#include "tg_css.c"
#include <limits.h>

void view_gen_apply_style(view_t* view, map_t* style, char* respath)
{
  vec_t* keys = VNEW();
  map_keys(style, keys);
  for (int index = 0; index < keys->length; index++)
  {
    char* key = keys->data[index];
    char* val = MGET(style, key);

    if (strcmp(key, "background-color") == 0)
    {
      int color                     = (int)strtol(val + 1, NULL, 16);
      view->layout.background_color = color;
      tg_css_add(view);
    }
    else if (strcmp(key, "background-image") == 0)
    {
      if (strstr(val, "url") != NULL)
      {
        char* url = mem_calloc(sizeof(char) * strlen(val), "char*", NULL, NULL);
        memcpy(url, val + 5, strlen(val) - 7);
        char* imagepath               = cstr_fromformat("%s/../res/%s", respath, url, NULL);
        view->layout.background_image = imagepath;
        REL(url);
        tg_css_add(view);
      }
    }
    else if (strcmp(key, "width") == 0)
    {
      if (strstr(val, "%") != NULL)
      {
        char* end = strstr(val, "%");
        int   len = end - val;
        //end[len - 1]       = '\0';
        int per            = atoi(val);
        view->layout.w_per = (float)per / 100.0;
      }
      else if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        //end[len - 1]        = '\0';
        int pix             = atoi(val);
        view->layout.width  = pix;
        view->frame.local.w = pix;
      }
    }
    else if (strcmp(key, "height") == 0)
    {
      if (strstr(val, "%") != NULL)
      {
        char* end = strstr(val, "%");
        int   len = end - val;
        //end[len - 1]       = '\0';
        int per            = atoi(val);
        view->layout.h_per = (float)per / 100.0;
      }
      else if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        //end[len - 1]        = '\0';
        int pix             = atoi(val);
        view->layout.height = pix;
        view->frame.local.h = pix;
      }
    }
    else if (strcmp(key, "display") == 0)
    {
      if (strcmp(val, "flex") == 0)
        view->layout.display = LD_FLEX;
      if (strcmp(val, "none") == 0)
        view->display = 0;
    }
    else if (strcmp(key, "overflow") == 0)
    {
      if (strcmp(val, "hidden") == 0)
        view->masked = 1;
    }
    else if (strcmp(key, "flex-direction") == 0)
    {
      if (strcmp(val, "column") == 0)
        view->layout.flexdir = FD_COL;
      else
        view->layout.flexdir = FD_ROW;
    }
    else if (strcmp(key, "margin") == 0)
    {
      if (strcmp(val, "auto") == 0)
      {
        view->layout.margin = INT_MAX;
      }
      else if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]               = '\0';
        int pix                    = atoi(val);
        view->layout.margin        = pix;
        view->layout.margin_top    = pix;
        view->layout.margin_left   = pix;
        view->layout.margin_right  = pix;
        view->layout.margin_bottom = pix;
      }
    }
    else if (strcmp(key, "top") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]     = '\0';
        int pix          = atoi(val);
        view->layout.top = pix;
      }
    }
    else if (strcmp(key, "left") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]      = '\0';
        int pix           = atoi(val);
        view->layout.left = pix;
      }
    }
    else if (strcmp(key, "right") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]       = '\0';
        int pix            = atoi(val);
        view->layout.right = pix;
      }
    }
    else if (strcmp(key, "bottom") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]        = '\0';
        int pix             = atoi(val);
        view->layout.bottom = pix;
      }
    }
    else if (strcmp(key, "border-radius") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]               = '\0';
        int pix                    = atoi(val);
        view->layout.border_radius = pix;
      }
    }
    else if (strcmp(key, "box-shadow") == 0)
    {
      if (strstr(val, "px") != NULL)
      {
        char* end = strstr(val, "px");
        int   len = end - val;
        // end[len - 1]             = '\0';
        int pix                  = atoi(val);
        view->layout.shadow_blur = pix;
      }
    }
    else if (strcmp(key, "align-items") == 0)
    {
      if (strcmp(val, "center") == 0)
      {
        view->layout.itemalign = IA_CENTER;
      }
    }
    else if (strcmp(key, "justify-content") == 0)
    {
      if (strcmp(val, "center") == 0)
      {
        view->layout.cjustify = JC_CENTER;
      }
    }
  }
  /* printf("layout for %s: ", view->id); */
  /* view_desc_layout(view->layout); */
  /* printf("\n"); */
}

vec_t* view_gen_load(char* htmlpath, char* csspath, char* respath)
{
  char* html = html_read(htmlpath);
  char* css  = html_read(csspath);

  tag_t*  view_structure = html_parse_html(html);
  prop_t* view_styles    = html_parse_css(css);

  // create style map
  map_t*  styles = MNEW();
  prop_t* props  = view_styles;
  while ((*props).class.len > 0)
  {
    prop_t t   = *props;
    char*  cls = mem_calloc(sizeof(char) * t.class.len + 1, "char*", NULL, cstr_describe);
    char*  key = mem_calloc(sizeof(char) * t.key.len + 1, "char*", NULL, cstr_describe);
    char*  val = mem_calloc(sizeof(char) * t.value.len + 1, "char*", NULL, cstr_describe);
    memcpy(cls, css + t.class.pos, t.class.len);
    memcpy(key, css + t.key.pos, t.key.len);
    memcpy(val, css + t.value.pos, t.value.len);
    map_t* style = MGET(styles, cls);
    if (style == NULL)
    {
      style = MNEW();
      MPUT(styles, cls, style);
    }
    MPUT(style, key, val);
    props += 1;
  }

  // create view structure
  vec_t* views = VNEW();
  tag_t* tags  = view_structure;
  while ((*tags).len > 0)
  {
    tag_t t = *tags;
    if (t.id.len > 0)
    {
      char* id = mem_calloc(sizeof(char) * t.id.len + 1, "char*", NULL, NULL);
      memcpy(id, html + t.id.pos + 1, t.id.len);
      view_t* view = view_new(id, (r2_t){0});
      VADD(views, view);
      if (t.level > 0) // add view to paernt
      {
        view_t* parent = views->data[t.parent];
        //printf("parent %i %i\n", t.parent, views->length);
        view_add(parent, view);
      }

      char cssid[100] = {0};
      snprintf(cssid, 100, "#%s", id);

      map_t* style = MGET(styles, cssid);
      if (style)
      {
        view_gen_apply_style(view, style, respath);
      }

      if (t.class.len > 0)
      {
        char* class = mem_calloc(sizeof(char) * t.class.len + 1, "char*", NULL, NULL);
        memcpy(class, html + t.class.pos + 1, t.class.len);

        char csscls[100] = {0};
        snprintf(csscls, 100, ".%s", class);

        style = MGET(styles, csscls);
        if (style)
        {
          view_gen_apply_style(view, style, respath);
        }
      }

      REL(id);
    }
    else
    {
      // idless view, probably </div>
      view_t* view = view_new("", (r2_t){0});
      VADD(views, view);
    }
    tags += 1;
  }

  return views;
}

#endif
