#ifndef view_gen_h
#define view_gen_h

#include "mtvector.c"
#include "view.c"

mtvec_t* view_gen_load(char* htmlpath, char* csspath);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mtparser.c"

mtvec_t* view_gen_load(char* htmlpath, char* csspath)
{
  char* html = parse_read(htmlpath);
  char* css  = parse_read(csspath);

  tag_t*  view_structure = parse_html(html);
  prop_t* view_styles    = parse_css(css);

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
        printf("parent %i %i\n", t.parent, views->length);
        view_add(parent, view);
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
