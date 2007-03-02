#ifndef DVIEW_H
#define DVIEW_H

#include <gtk/gtk.h>

#include "driver.h"
#include "video.h"
#include "debug/debugvw.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

GType dview_get_type(void);

#define DVIEW_TYPE           (dview_get_type())
#define DVIEW(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), DVIEW_TYPE, DView))
#define DVIEW_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST((obj), DVIEW,  DViewClass))
#define IS_DVIEW(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), DVIEW_TYPE))
#define IS_DVIEW_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((obj), DVIEW_TYPE))
#define DVIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), DVIEW_TYPE, DViewClass))


typedef struct _DViewClass  DViewClass;
typedef struct _DView       DView;

struct _DViewClass
{
  GtkContainerClass parent_class;
  PangoFontDescription *fixedfont;
  int fixedfont_width, fixedfont_height;
};

struct _DView
{
  GtkContainer parent;
  GtkAdjustment *hadj, *vadj;
  GtkWidget *hscrollbar, *vscrollbar;
  int hsz, vsz;
  int hs, vs;
  int tr, tc;
  int this_one_is_stupidly_autoscrolling;
  debug_view *dw;
  gchar *name;
  PangoLayout *playout;
  GdkGC *gc;
};


GtkWidget *dview_new(gchar *widget_name, gchar *string1, gchar *string2, gint int1, gint int2);
void dview_set_debug_view(DView *dv, debug_view *dw);
void dview_this_one_is_stupidly_autoscrolling(DView *dv);

#endif
