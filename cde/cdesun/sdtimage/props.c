#ifndef lint
static char sccsid[] = "@(#)props.c 1.15 97/04/24";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <sys/stat.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>

#include "help.h"
#include "misc.h"
#include "props.h"
#include "display.h"
#include "imagetool.h"
#include "ui_imagetool.h"

/*
 * To add a property, add the name here.
 */
char *resource_names[] = {
	"ViewImageIn",
	"Colors",
	"DisplayPalette",
	"UseDSC",
	"ViewPostScriptIn",
	"TTTimeoutSecs"
};

char *resource_defaults[] = {
         DEFAULT_VIEW_IN,
	 DEFAULT_COLORS,
	 DEFAULT_SHOW_PALETTE,
	 DEFAULT_DSC,
	 DEFAULT_PS_VIEW,
	 DEFAULT_TT_TIMEOUT
};

#define NRESOURCES   sizeof resource_names / sizeof resource_names[0]

ResourceInfo  resources[NRESOURCES];

int  gray_vis1 = -1;
int  color_vis1 = -1;
int  gray_vis4 = -1;
int  color_vis4 = -1;
int  gray_vis8 = -1;
int  color_vis8 = -1;
int  gray_vis16 = -1;
int  color_vis16 = -1;
int  gray_vis24 = -1;
int  color_vis24 = -1;

int   depth1 = 0, depth4 = 0, depth8 = 0, depth16 = 0, depth24 = 0;
int   gray_vis = 0;
int   color_vis = 0;

static XrmDatabase dsdb, rdb;

/*
 * Pointer to property sheet values.
 * Not to be confused with props in propsui.c which 
 * contains the ui objects.
 */
PropInfo     *current_props;


/*
 * Some utilities needed for conversion to and
 * from resource types.
 */
static int
boolean_to_int (value)
    char *value;
{
    if ((strcmp (value, "True") == 0) ||
        (strcmp (value, "TRUE") == 0) ||
	(strcmp (value, "true") == 0))
      return 1;
    else if ((strcmp (value, "False") == 0) ||
	     (strcmp (value, "FALSE") == 0) ||
	     (strcmp (value, "false") == 0))
      return 0;
    else
      return 1;

}

static char *
int_to_boolean (value)
    int value;
{
    if (value == 0)
      return UNDISPLAY_PALETTE;
    else
      return DISPLAY_PALETTE;
}

char *
int_to_view (value)
  int  value;
{
  if (value == 0 && gray_vis == TRUE)
    return VIEW_GRAY_SCALE;
  else
    return VIEW_COLOR;

}

char *
int_to_color (value)
  int  value;
{
  char buf[80];
  int  choice = -1;

  if (depth1)
    choice++;
  if (value == choice)
    return COLORS_BW;

  if (depth4)
    choice++;
  if (value == choice)
    return COLORS_16;

  if (depth8)
    choice++;
  if (value == choice)
    return COLORS_256;

  if (depth16)
    choice++;
  if (value == choice)
    return COLORS_THOUSANDS;

  if (depth24)
    choice++;
  if (value == choice)
    return COLORS_MILLIONS;

  return COLORS_256;

}

static int
view_to_int (value)
   char *value;
{
  if (gray_vis && (strcmp (value, VIEW_COLOR) == 0))
    return 1;
  else if (strcmp (value, VIEW_GRAY_SCALE) == 0)
    return 0;
  else
    return 0;

}

static int
color_to_int (value)
   char *value;
{
  if (strcmp (value, COLORS_BW) == 0)
/* If value is "BW" and 1-bit not supported,
 * find "best value".
 */
    if (depth1 == 0) {
      if (depth24)
	return color_to_int (COLORS_MILLIONS);
      else if (depth16)
	return color_to_int (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_int (COLORS_256);
      else if (depth4)
	return color_to_int (COLORS_16);
    }
    else
      return 0;

  if (strcmp (value, COLORS_16) == 0) {
/* If value is "16" and 4-bit not supported,
 * find "best value".
 */
    if (depth4 == 0) {
      if (depth24)
	return color_to_int (COLORS_MILLIONS);
      else if (depth16)
	return color_to_int (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_int (COLORS_256);
      else if (depth1)
	return color_to_int (COLORS_BW);
    }
    else if (depth1)
      return 1;
    else
      return 0;
  }

  if (strcmp (value, COLORS_256) == 0) {
/* If value is "256" and 8-bit not supported,
 * find "best value".
 */
    if (depth8 == 0) {
      if (depth24)
	return color_to_int (COLORS_MILLIONS);
      else if (depth16)
	return color_to_int (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_int (COLORS_256);
      else if (depth1)
	return color_to_int (COLORS_BW);
    }
    else if (depth1 && depth4)
      return 2;
    else if ((depth1 + depth4) == 1)
      return 1;
    else 
      return 0;
  }

  if (strcmp (value, COLORS_THOUSANDS) == 0) {
/* If value is Thousands and 16-bit not supported,
 * find "best value".
 */
    if (depth16 == 0) {
      if (depth24)
	return color_to_int (COLORS_MILLIONS);
      else if (depth8)
	return color_to_int (COLORS_256);
      else if (depth4)
	return color_to_int (COLORS_16);
      else if (depth1)
	return color_to_int (COLORS_BW);
    }
    else if (depth1 && depth4 && depth8)
      return 3;
    else if ((depth1 + depth4 + depth8) == 2)
      return 2;
    else if ((depth1 + depth4 + depth8) == 1)
      return 1;
    else
      return 0;
  }

  if (strcmp (value, COLORS_MILLIONS) == 0) {
/* If value is Millions and 24-bit not supported,
 * find "next best value".
 */
    if (depth24 == 0) {
      if (depth16)
        return color_to_int (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_int (COLORS_256);
      else if (depth4)
	return color_to_int (COLORS_16);
      else if (depth1)
	return color_to_int (COLORS_BW);
    }
    else if (depth1 && depth4 && depth8 && depth16)
      return 4;
    else if ((depth1 + depth4 + depth8 + depth16) == 3)
      return 3;
    else if ((depth1 + depth4 + depth8 + depth16) == 2)
      return 2;
    else if ((depth1 + depth4 + depth8 + depth16) == 1)
      return 1;
    else
      return 0;
  }
 
  return 0;

}

static char *
view_to_str (value)
   char *value;
{
  if (color_vis && (strcmp (value, VIEW_COLOR) == 0))
    return VIEW_COLOR;
  else if (gray_vis && (strcmp (value, VIEW_GRAY_SCALE) == 0))
    return VIEW_GRAY_SCALE;
  else if (color_vis)
    return VIEW_COLOR;
  else if (gray_vis)
    return VIEW_GRAY_SCALE;
  else
    return VIEW_COLOR;

}

static char *
psview_to_str (value)
   char *value;
{
  if (color_vis && (strcmp (value, PS_COLOR) == 0))
    return PS_COLOR;
  else if (strcmp (value, PS_MONO))
    return PS_MONO;
  else
    return PS_MONO;

}

static char *
color_to_str (value)
   char *value;
{
  if (strcmp (value, COLORS_BW) == 0)
/* If value is "BW" and 1-bit not supported,
 * find "best value".
 */
    if (depth1 == 0) {
      if (depth24)
	return color_to_str (COLORS_MILLIONS);
      else if (depth16)
	return color_to_str (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_str (COLORS_256);
      else if (depth4)
	return color_to_str (COLORS_16);
    }
    else
      return COLORS_BW;

  if (strcmp (value, COLORS_16) == 0) {
/* If value is "16" and 4-bit not supported,
 * find "best value".
 */
    if (depth4 == 0) {
      if (depth24)
	return color_to_str (COLORS_MILLIONS);
      else if (depth16)
	return color_to_str (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_str (COLORS_256);
      else if (depth1)
	return color_to_str (COLORS_BW);
    }
    else
      return COLORS_16;
  }

  if (strcmp (value, COLORS_256) == 0) {
/* If value is "256" and 8-bit not supported,
 * find "best value".
 */
    if (depth8 == 0) {
      if (depth24)
	return color_to_str (COLORS_MILLIONS);
      else if (depth16)
	return color_to_str (COLORS_THOUSANDS);
      else if (depth4)
	return color_to_str (COLORS_16);
      else if (depth1)
	return color_to_str (COLORS_BW);
    }
    else 
      return COLORS_256;
  }

  if (strcmp (value, COLORS_THOUSANDS) == 0) {
/* If value is Thousands and 16-bit not supported,
 * find "best value".
 */
    if (depth16 == 0) {
      if (depth24)
	return color_to_str (COLORS_MILLIONS);
      else if (depth8)
	return color_to_str (COLORS_256);
      else if (depth4)
	return color_to_str (COLORS_16);
      else if (depth1)
	return color_to_str (COLORS_BW);
    }
    else 
      return COLORS_THOUSANDS;
  }

  if (strcmp (value, COLORS_MILLIONS) == 0) {
/* If value is Millions and 24-bit not supported,
 * find "next best value".
 */
    if (depth24 == 0) {
      if (depth16)
        return color_to_str (COLORS_THOUSANDS);
      else if (depth8)
	return color_to_str (COLORS_256);
      else if (depth4)
	return color_to_str (COLORS_16);
      else if (depth1)
	return color_to_str (COLORS_BW);
    }
    else 
      return COLORS_MILLIONS;
  }
 
  if (depth24)
    return COLORS_MILLIONS;
  else if (depth16)
    return COLORS_THOUSANDS;
  else if (depth8)
    return COLORS_256;
  else if (depth4)
    return COLORS_16;
  else if (depth1)
    return COLORS_BW;
  else
    return COLORS_256;

}

/*
 * Set the resource into the current_props struct
 * by converting the resource to ASCII and interpreting it.
 */
static void
set_property (resource)
    ResourceInfo  *resource;
{

    if (strcmp (resource->name, resource_names[RES_VIEW_IN]) == 0) {
      if (current_props->view_in)  free (current_props->view_in);
      current_props->view_in = strdup (view_to_str (resource->value));
    }
    else if (strcmp (resource->name, resource_names[RES_COLORS]) == 0) { 
      if (current_props->color)  free (current_props->color);
      current_props->color = strdup (color_to_str (resource->value));
    }
    else if (strcmp (resource->name, resource_names[RES_SHOW_PALETTE]) == 0) 
      current_props->show_palette = boolean_to_int (resource->value);
    else if (strcmp (resource->name, resource_names[RES_DSC]) == 0) 
      current_props->use_dsc = boolean_to_int (resource->value);
    else if (strcmp (resource->name, resource_names[RES_PS_VIEW]) == 0) {
      if (current_props->ps_view)  free (current_props->ps_view);
      current_props->ps_view = strdup (psview_to_str (resource->value));
    }
    else if (strcmp (resource->name, resource_names[RES_TT_TIMEOUT]) == 0) 
      current_props->tt_timeout = atoi (resource->value);

}

/*
 * read from the resource database and
 * stuff values into current_props and resources.
 */
static int
read_resources (file_name)
    char *file_name;
{

    int          i;
    char        *resource;
    extern char *ds_get_resource();

    for (i = 0; i < NRESOURCES; i++) {
      resource = ds_get_resource (rdb, "imagetool", resource_names[i]);
      if (resource != NULL) {
        if (resources[i].value != NULL)
          free (resources[i].value);
        resources[i].value = strdup (resource);
	set_property (&resources[i]);
      }
    }

   return 1;

}

/*
 * set props with defaults in case nothing in .desksetdefaults
 * about imagetool.
 */
static PropInfo *
init_props()
{
    PropInfo    *tmp_props = (PropInfo *) calloc (1, sizeof (PropInfo));
    int          i;
    XVisualInfo  vtemplate;
    int 	 num_vis;
    int          class;
    int          screen;
    Display     *dpy;
    XVisualInfo  vinfo;

    for (i = 0; i < NRESOURCES; i++) {
       resources[i].name = strdup (resource_names[i]);
       resources[i].value = strdup (resource_defaults[i]);
    }

    dpy = image_display->xdisplay;
    screen = DefaultScreen (dpy);

    for (class = StaticGray; class <= DirectColor; class++) {

      if (XMatchVisualInfo (dpy, screen, 1, class, &vinfo)) {
	depth1 = TRUE; 
	if ((class < StaticColor) && (class > gray_vis1))
	  gray_vis1 = class;
	else if (class > color_vis1)
          color_vis1 = class;  /* shouldn't happen, 1-bit color */
      }

      if (XMatchVisualInfo (dpy, screen, 4, class, &vinfo)) {
	depth4 = TRUE;
	if (class < StaticColor && class > gray_vis4)
	  gray_vis4 = class;
	else if (class > color_vis4 && color_vis4 != PseudoColor)
          color_vis4 = class;
      }

      if (XMatchVisualInfo (dpy, screen, 8, class, &vinfo)) {
	depth8 = TRUE;
	if (class < StaticColor && class > gray_vis8)
	  gray_vis8 = class;
	else if (class > color_vis8 && color_vis8 != PseudoColor)
          color_vis8 = class;
      }

      if (XMatchVisualInfo (dpy, screen, 16, class, &vinfo)) {
	depth16 = TRUE;
	if (class < StaticColor && class > gray_vis16)
	  gray_vis16 = class;
	else if (class > color_vis16 && color_vis16 != PseudoColor)
          color_vis16 = class;
      }

      if (XMatchVisualInfo (dpy, screen, 24, class, &vinfo)) {
	depth24 = TRUE; 
	if (class < StaticColor && class > gray_vis24)
	  gray_vis24 = class;
	else if (class > color_vis24 && color_vis24 != TrueColor)
          color_vis24 = class;
      }

    }

    if (gray_vis1 != -1 || gray_vis4 != -1 || gray_vis8 != -1 ||
	gray_vis16 != -1 || gray_vis24 != -1)
      gray_vis = TRUE;
    
    if (color_vis1 != -1 || color_vis4 != -1 || color_vis8 != -1 ||
	color_vis16 != -1 || color_vis24 != -1)
      color_vis = TRUE;

    return (tmp_props);

}

/*
 * Called then any of the objects on
 * the props sheet has been modified.
 * Set flag to changed so we know whether to put
 * a notice up when dismissing without applying.
 */
static void
set_dirty_flag(flag)
     Boolean  flag;
{
    current_props->props_changed = flag;

    if (properties == NULL)
      return;
}

static void
set_props_defaults()
{
    current_props = init_props();
    current_props->view_in = strdup (DEFAULT_VIEW_IN);
 
/*
 * Set default props to be the "best" (e.e. egret  24-bit)
 * This will be overridden if defined in .desksetdefaults
 * or on command line.
 */
    if (color_vis24 != -1)
      current_props->color = strdup (COLORS_MILLIONS);
    else if (color_vis16 != -1)
      current_props->color = strdup (COLORS_THOUSANDS);
    else if (color_vis8 != -1) 
      current_props->color = strdup (COLORS_256);
    else if (color_vis4 != -1) 
      current_props->color = strdup (COLORS_16);
    else if (color_vis1 != -1 || gray_vis1 != -1)
      current_props->color = strdup (COLORS_BW);
    else
      current_props->color = strdup (DEFAULT_COLORS);

    current_props->show_palette = boolean_to_int (DEFAULT_SHOW_PALETTE);
    current_props->use_dsc = boolean_to_int (DEFAULT_DSC);
    current_props->ps_view = strdup (DEFAULT_PS_VIEW);
    current_props->tt_timeout = atoi (DEFAULT_TT_TIMEOUT);
    set_dirty_flag (False);
}

/*
 * Free up resources before re-reading in again.
 * Called from Reset button.
 */
static void
free_resources()
{
    int   i;

    for (i = 0; i < NRESOURCES; i++) {
      if (resources[i].name) {
	free (resources[i].name);
        resources[i].name = NULL;
      }
      if (resources[i].value) {
	free (resources[i].value);
        resources[i].value = NULL;
      }
    }

}

/*
 * Main function to call when first
 * starting up imagetool and read in the imagetool
 * resources, if any.
 */
int
read_props (int initialize)
{
    char  	 *ds, file_name [MAXPATHLEN];
    struct stat   statbuf;
    int           status, ret_value = FALSE;
    char         *view_value, *color_value, *ps_value;
    extern XrmDatabase  ds_load_deskset_defs();
    extern XrmDatabase  ds_load_resources();
/*
 * Set defaults in case no imagetool properties
 * are in the resource database.
 */
    set_props_defaults();

/*
 * Determine which resource file to read in: 
 * Either DESKSETDEFAULTS or $HOME/.desksetdefaults.
 */
    if ((ds = getenv ("DESKSETDEFAULTS")) == NULL) {
      sprintf (file_name, "%s/%s", getenv ("HOME"), DS_FILENAME);
      status = stat (file_name, &statbuf);
    }
    else
      status = stat (ds, &statbuf);

/*
 * Read in the resource database and overwrite
 * any imagetool properties with the defaults.
 */
    if (status != -1) {
      dsdb = ds_load_deskset_defs();
      rdb = ds_load_resources (image_display->xdisplay);
      read_resources (file_name);
      ret_value = TRUE;
    }

/*
 * Check if this is a valid setting.
 * For example, if no gray visual in 24-bit is available,
 * then can't have view in = Gray && Color == 24-bit.
 */
    view_value = strdup (view_to_str (current_props->view_in));
    color_value = strdup (color_to_str (current_props->color));

    if (strcmp (color_value, COLORS_BW) == 0) {
      if (strcmp (view_value, VIEW_COLOR) == 0 && color_vis1 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_GRAY_SCALE);
      }
      else if (strcmp (view_value, VIEW_GRAY_SCALE) == 0 && gray_vis1 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_COLOR);
      }
    }
    else if (strcmp (color_value, COLORS_16) == 0) {
      if (strcmp (view_value, VIEW_COLOR) == 0 && color_vis4 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_GRAY_SCALE);
      }
      else if (strcmp (view_value, VIEW_GRAY_SCALE) == 0 && gray_vis4 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_COLOR);
      }
    }
    else if (strcmp (color_value, COLORS_256) == 0) {
      if (strcmp (view_value, VIEW_COLOR) == 0 && color_vis8 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_GRAY_SCALE);
      }
      else if (strcmp (view_value, VIEW_GRAY_SCALE) == 0 && gray_vis8 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_COLOR);
      }
    }
    else if (strcmp (color_value, COLORS_THOUSANDS) == 0) {
      if (strcmp (view_value, VIEW_COLOR) == 0 && color_vis16 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_GRAY_SCALE);
      }
      else if (strcmp (view_value, VIEW_GRAY_SCALE) == 0 && gray_vis16 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_COLOR);
      }
    }
    else if (strcmp (color_value, COLORS_MILLIONS) == 0) {
      if (strcmp (view_value, VIEW_COLOR) == 0 && color_vis24 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_GRAY_SCALE);
      }
      else if (strcmp (view_value, VIEW_GRAY_SCALE) == 0 && gray_vis24 == -1) {
	if (current_props->view_in) free (current_props->view_in);
        current_props->view_in = strdup (VIEW_COLOR);
      }
    }
	
    free (view_value);
    free (color_value);     
/*
 * Check if ps_view is a valid setting.
 * i.e., if set to color and on a monochrome, go back to mono.
 */
    ps_value = strdup (psview_to_str (current_props->ps_view));
    if (current_props->ps_view) free (current_props->ps_view);
    current_props->ps_view = strdup (ps_value);
 
    free (ps_value);
/* 
 * Now that we set ps_view, set flag in prog to see
 * if we shold be checking if there is color operator
 * in postscript files.
 * Logic is: if PS view set to MONO and color is available on system,
 * then check for color == true;
 *
 * if Initialize == True, then this is first time read.
 * we only want to initialize these at start up because
 * they can change on the fly.
 */
    if (initialize == True) {
      if ((strcmp (current_props->ps_view, PS_MONO) == 0) && 
	  (color_vis == True))
	prog->check_for_color = True;
      else
	prog->check_for_color = False;
/*
 * Save this value since the properties can
 * change on the fly.
 */
      if (strcmp (current_props->ps_view, PS_MONO) == 0)
	prog->ps_mono = True;
      else
	prog->ps_mono = False;
    }
/*
 * Check if tt_timeout is a positive number
 */
    if (current_props->tt_timeout < 0)
      current_props->tt_timeout = 0;
    
}

static void
RemoveViewCallbacks (p)
     PropertiesObjects *p;
{
     extern void PropsViewChangedCallback();

     if (p->grayscale_toggle)
       XtRemoveCallback (p->grayscale_toggle, XmNvalueChangedCallback, 
		         PropsViewChangedCallback, p);
     if (p->color_toggle)
       XtRemoveCallback (p->color_toggle, XmNvalueChangedCallback, 
		         PropsViewChangedCallback, p);
}

static void
AddViewCallbacks (p)
     PropertiesObjects *p;
{
     extern void PropsViewChangedCallback();

     if (p->grayscale_toggle)
       XtAddCallback (p->grayscale_toggle, XmNvalueChangedCallback, 
		      PropsViewChangedCallback, p);
     if (p->color_toggle)
       XtAddCallback (p->color_toggle, XmNvalueChangedCallback,
		      PropsViewChangedCallback, p);
}

static void
RemoveColorCallbacks (p)
     PropertiesObjects *p;
{
     extern void PropsColorChangedCallback();

     if (p->depth24_toggle)
       XtRemoveCallback (p->depth24_toggle, XmNvalueChangedCallback, 
		         PropsColorChangedCallback, p);
     if (p->depth16_toggle)
       XtRemoveCallback (p->depth16_toggle, XmNvalueChangedCallback, 
		         PropsColorChangedCallback, p);
     if (p->depth8_toggle)
       XtRemoveCallback (p->depth8_toggle, XmNvalueChangedCallback, 
		         PropsColorChangedCallback, p);
     if (p->depth4_toggle)
       XtRemoveCallback (p->depth4_toggle, XmNvalueChangedCallback, 
		         PropsColorChangedCallback, p);
     if (p->depth1_toggle)
       XtRemoveCallback (p->depth1_toggle, XmNvalueChangedCallback,
		         PropsColorChangedCallback, p);
}

static void
AddColorCallbacks (p)
     PropertiesObjects *p;
{
     extern void PropsColorChangedCallback();

     if (p->depth24_toggle)
       XtAddCallback (p->depth24_toggle, XmNvalueChangedCallback, 
		      PropsColorChangedCallback, p);
     if (p->depth16_toggle)
       XtAddCallback (p->depth16_toggle, XmNvalueChangedCallback, 
		      PropsColorChangedCallback, p);
     if (p->depth8_toggle)
       XtAddCallback (p->depth8_toggle, XmNvalueChangedCallback, 
		      PropsColorChangedCallback, p);
     if (p->depth4_toggle)
       XtAddCallback (p->depth4_toggle, XmNvalueChangedCallback, 
		      PropsColorChangedCallback, p);
     if (p->depth1_toggle)
       XtAddCallback (p->depth1_toggle, XmNvalueChangedCallback, 
		     PropsColorChangedCallback, p);
}

static void
SetToggle (toggle, value)
     Widget  toggle;
     Boolean value;
{
     if (toggle)
       XtVaSetValues (toggle, XmNset, value, NULL);
}
 

static char *
PropsViewValue (p)
     PropertiesObjects  *p;
{
     Boolean    set = False;

     if (p->grayscale_toggle && set == False) {
       XtVaGetValues (p->grayscale_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return VIEW_GRAY_SCALE;
     }

     if (p->color_toggle && set == False) {
       XtVaGetValues (p->color_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return VIEW_COLOR;
     }

     return VIEW_COLOR;
}

static char *
PropsPSValue (p)
     PropertiesObjects  *p;
{
     Boolean    set = False;

     if (p->bwps_toggle && set == False) {
       XtVaGetValues (p->bwps_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return PS_MONO;
     }

     if (p->colorps_toggle && set == False) {
       XtVaGetValues (p->colorps_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return PS_COLOR;
     }

     return PS_MONO;
}

static char *
PropsColorValue (p)
     PropertiesObjects  *p;
{
     Boolean  set = False;

     if (p->depth1_toggle && set == False) {
       XtVaGetValues (p->depth1_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return COLORS_BW;
     }

     if (p->depth4_toggle && set == False) {
       XtVaGetValues (p->depth4_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return COLORS_16;
     }

     if (p->depth8_toggle && set == False) {
       XtVaGetValues (p->depth8_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return COLORS_256;
     }

     if (p->depth16_toggle && set == False) {
       XtVaGetValues (p->depth16_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return COLORS_THOUSANDS;
     }

     if (p->depth24_toggle && set == False) {
       XtVaGetValues (p->depth24_toggle, XmNset, &set, NULL);
       if (set == True) 
	 return COLORS_MILLIONS;
     }

     return COLORS_256;
}

void
PropsViewChangedCallback (w, client_data, cbs)
     Widget                         w;
     XtPointer                      client_data;
     XmToggleButtonCallbackStruct  *cbs;
{
     PropertiesObjects  *p = (PropertiesObjects *)client_data;
     char               *view_str = NULL, *color_str = NULL;
     char               *view_value = NULL;
     char               *color_value = NULL;
     Boolean            set = False;
     int                status;

     if (cbs && cbs->reason != XmCR_VALUE_CHANGED) 
       return;

/*
 * Get the current View value set.
 */   
     view_str = strdup (PropsViewValue (p));
/*
 * Get the current Colors value set.
 */   
     color_str = strdup (PropsColorValue (p));

     view_value = strdup (view_to_str (view_str));
     color_value = strdup (color_to_str (color_str));

     if (view_str)   free (view_str);
     if (color_str)  free (color_str);

/*
 * If attempting to select GrayScale...
 */
     if (strcmp (view_value, VIEW_GRAY_SCALE) == 0) {

      /* Check if gray visual for
         currently selected colors */

       if ((strcmp (color_value, COLORS_MILLIONS) == 0 && gray_vis24 == -1) ||
	   (strcmp (color_value, COLORS_THOUSANDS) == 0 && gray_vis16 == -1) ||
	   (strcmp (color_value, COLORS_256) == 0 && gray_vis8 == -1) ||
	   (strcmp (color_value, COLORS_16) == 0 && gray_vis4 == -1) ||
	   (strcmp (color_value, COLORS_BW) == 0 && gray_vis1 == -1)) {
	 /*
	  * Remove value changed callback for color toggles.
	  */
	 RemoveColorCallbacks (p);
	 /*
	  * Unset all color toggles first, then
	  * set the correct one.
	  */
	 SetToggle (p->depth24_toggle, False);
	 SetToggle (p->depth16_toggle, False);
	 SetToggle (p->depth8_toggle, False);
	 SetToggle (p->depth4_toggle, False);
	 SetToggle (p->depth1_toggle, False);
	 
	 /* Automatically select the "greatest" gray visual */
	 if (gray_vis24 != -1) 
	   SetToggle (p->depth24_toggle, True);
	 else if (gray_vis16 != -1) 
	   SetToggle (p->depth16_toggle, True);
	 else if (gray_vis8 != -1) 
	   SetToggle (p->depth8_toggle, True);
	 else if (gray_vis4 != -1) 
	   SetToggle (p->depth4_toggle, True);
	 else if (gray_vis1 != -1) 
	   SetToggle (p->depth1_toggle, True);
	 /*
	  * Put back value changed callback for color toggles.
	  */
	 AddColorCallbacks (p);
       }
       
     }
     /*
      * Attempting to select Color... 
      */
     else if (strcmp (view_value, VIEW_COLOR) == 0) {
       /*
	* Check if color visual exists
	* for currently selected colors.
	*/
       if ((strcmp (color_value, COLORS_MILLIONS) == 0 && color_vis24 == -1) ||
	   (strcmp (color_value, COLORS_THOUSANDS) == 0 && color_vis16 == -1) ||
	   (strcmp (color_value, COLORS_256) == 0 && color_vis8 == -1) ||
	   (strcmp (color_value, COLORS_16) == 0 && color_vis4 == -1) ||
	   (strcmp (color_value, COLORS_BW) == 0 && color_vis1 == -1)) {
	 /*
	  * Remove value changed callback for color toggles.
	  */
	 RemoveColorCallbacks (p);
	 
	 /*
	  * Unset all color toggles first, then
	  * set the correct one.
	  */
	 SetToggle (p->depth24_toggle, False);
	 SetToggle (p->depth16_toggle, False);
	 SetToggle (p->depth8_toggle, False);
	 SetToggle (p->depth4_toggle, False);
	 SetToggle (p->depth1_toggle, False);
	 
	 /* Automatically select the "greatest" color visual */
	 if (color_vis24 != -1)
	   SetToggle (p->depth24_toggle, True);
	 else if (color_vis16 != -1)
	   SetToggle (p->depth16_toggle, True);
	 else if (color_vis8 != -1)	
	   SetToggle (p->depth8_toggle, True);
	 else if (color_vis4 != -1)
	   SetToggle (p->depth4_toggle, True);
	 else if (color_vis1 != -1)
	   SetToggle (p->depth1_toggle, True);
	 /*
	  * Put back value changed callback for color toggles.
	  */
	 AddColorCallbacks (p);
       }
     }
     
     if (view_value)
       free (view_value);
     
     if (color_value)
       free (color_value);
     
     set_dirty_flag (True);
}

void
PropsPSChangedCallback (w, client_data, cbs)
     Widget                         w;
     XtPointer                      client_data;
     XmToggleButtonCallbackStruct  *cbs;
{
     PropertiesObjects  *p = (PropertiesObjects *)client_data;

     if (cbs && cbs->reason == XmCR_VALUE_CHANGED)
       set_dirty_flag (True);
}

void
PropsColorChangedCallback (w, client_data, cbs)
     Widget                         w;
     XtPointer                      client_data;
     XmToggleButtonCallbackStruct  *cbs;
{
     PropertiesObjects  *p = (PropertiesObjects *)client_data;
     char               *view_str = NULL, *color_str = NULL;
     char               *view_value = NULL;
     char               *color_value = NULL;

     if (cbs && cbs->reason != XmCR_VALUE_CHANGED) 
       return;
/*
 * Get the current View value set.
 */   
     view_str = strdup (PropsViewValue (p));
/*
 * Get the current Colors value set.
 */   
     color_str = strdup (PropsColorValue (p));

     view_value = strdup (view_to_str (view_str));
     color_value = strdup (color_to_str (color_str));

     if (view_str)  free (view_str);
     if (color_str) free (color_str);

     if (strcmp (view_value, VIEW_GRAY_SCALE) == 0) {

       if ((strcmp (color_value, COLORS_MILLIONS) == 0 && gray_vis24 == -1) ||
	   (strcmp (color_value, COLORS_THOUSANDS) == 0 && gray_vis16 == -1) ||
	   (strcmp (color_value, COLORS_256) == 0 && gray_vis8 == -1) ||
	   (strcmp (color_value, COLORS_16) == 0 && gray_vis4 == -1) ||
	   (strcmp (color_value, COLORS_BW) == 0 && gray_vis1 == -1)) {

	 AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_WARNING,
		      DIALOG_TITLE, catgets (prog->catd, 1, 413, "Image Viewer - Options"),
		      DIALOG_TEXT, catgets (prog->catd, 1, 206,
				 "Invalid Colors set for Gray Scale viewing."),
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, 207, "Continue"),
		      HELP_IDENT, HELP_PROPS_ITEM,
		      NULL);
	 /*
	  * Remove value changed callback for color toggles.
	  */
	 RemoveColorCallbacks (p);
	/*
	 * Unset all color toggles first, then
	 * set the correct one.
	 */
	 SetToggle (p->depth24_toggle, False);
	 SetToggle (p->depth16_toggle, False);
	 SetToggle (p->depth8_toggle, False);
	 SetToggle (p->depth4_toggle, False);
	 SetToggle (p->depth1_toggle, False);

	 /* Automatically select the "greatest" gray visual */
	 if (gray_vis24 != -1)
	   SetToggle (p->depth24_toggle, True);
	 else if (gray_vis16 != -1)
	   SetToggle (p->depth16_toggle, True);
	 else if (gray_vis8 != -1)
	   SetToggle (p->depth8_toggle, True);
	 else if (gray_vis4 != -1)
	   SetToggle (p->depth4_toggle, True);
	 else if (gray_vis1 != -1)
	   SetToggle (p->depth1_toggle, True);
	 /*
	  * Put back value changed callback for color toggles.
	  */
	 AddColorCallbacks (p);
       }
     }
     else {

       if ((strcmp (color_value, COLORS_MILLIONS) == 0 && color_vis24 == -1) ||
          (strcmp (color_value, COLORS_THOUSANDS) == 0 && color_vis16 == -1) ||
          (strcmp (color_value, COLORS_256) == 0 && color_vis8 == -1) ||
          (strcmp (color_value, COLORS_16) == 0 && color_vis4 == -1) ||
          (strcmp (color_value, COLORS_BW) == 0 && color_vis1 == -1)) {

	 AlertPrompt (base->top_level,
		      DIALOG_TYPE,  XmDIALOG_WARNING,
		      DIALOG_TITLE, catgets (prog->catd, 1, 414, "Image Viewer - Options"),
		      DIALOG_TEXT, catgets (prog->catd, 1, 208,
				 "Invalid Colors set for Color viewing."),
		      BUTTON_IDENT, 0, catgets (prog->catd, 1, 209, "Continue"),
		      HELP_IDENT, HELP_PROPS_ITEM,
		      NULL);

	 /*
	  * Remove value changed callback for color toggles.
	  */
	 RemoveColorCallbacks (p);
	/*
	 * Unset all color toggles first, then
	 * set the correct one.
	 */
	 SetToggle (p->depth24_toggle, False);
	 SetToggle (p->depth16_toggle, False);
	 SetToggle (p->depth8_toggle, False);
	 SetToggle (p->depth4_toggle, False);
	 SetToggle (p->depth1_toggle, False);

	/* Automatically select the "greatest" color visual */
	if (color_vis24 != -1)
	  SetToggle (p->depth24_toggle, True);
	else if (color_vis16 != -1)
	  SetToggle (p->depth16_toggle, True);
	else if (color_vis8 != -1)
	  SetToggle (p->depth8_toggle, True);
	else if (color_vis4 != -1)
	  SetToggle (p->depth4_toggle, True);
	else if (color_vis1 != -1)
	  SetToggle (p->depth1_toggle, True);
	 /*
	  * Put back value changed callback for color toggles.
	  */
	 AddColorCallbacks (p);
      }
    }

    if (color_value)
      free (color_value);

    if (view_value)
      free (view_value);

   set_dirty_flag (True);
}


void
PropsOpendocChangedCallback (w, client_data, cbs)
     Widget                         w;
     XtPointer                      client_data;
     XmToggleButtonCallbackStruct  *cbs;
{
     if (cbs && cbs->reason == XmCR_VALUE_CHANGED)
       set_dirty_flag (True);
}
/*
 * This call is needed because the read_props() function
 * reads the resource database, but does not set the
 * XView Objects on the props sheet because it might have
 * not been created yet.  This call sets the props
 * sheet according to what is stored in current_props.
 *
 * The xview objects in propsui need to be created before
 * calling this because after reading in the resources
 * it sets the properties popup to display what is
 * currently defined in .desksetdefaults.
 */
int
PropsSetValues (p)
     PropertiesObjects  *p;
{

     RemoveViewCallbacks (p);
/* 
 * Set the View In toggle
 */
     if (strcmp (current_props->view_in, VIEW_GRAY_SCALE) == 0) {
       SetToggle (p->grayscale_toggle, True);
       SetToggle (p->color_toggle, False);
     }
     else {
       SetToggle (p->color_toggle, True);
       SetToggle (p->grayscale_toggle, False);
     }
     AddViewCallbacks (p);

     RemoveColorCallbacks (p);
/*
 * Unset all colors toggle, then set correct one.
 * Set the Colors toggle.
 */
     SetToggle (p->depth24_toggle, False);
     SetToggle (p->depth16_toggle, False);
     SetToggle (p->depth8_toggle, False);
     SetToggle (p->depth4_toggle, False);
     SetToggle (p->depth1_toggle, False);

     if (strcmp (current_props->color, COLORS_MILLIONS) == 0) 
       SetToggle (p->depth24_toggle, True);
     else if (strcmp (current_props->color, COLORS_THOUSANDS) == 0) 
       SetToggle (p->depth16_toggle, True);
     else if (strcmp (current_props->color, COLORS_256) == 0) 
       SetToggle (p->depth8_toggle, True);
     else if (strcmp (current_props->color, COLORS_16) == 0) 
       SetToggle (p->depth4_toggle, True);
     else if (strcmp (current_props->color, COLORS_BW) == 0) 
       SetToggle (p->depth1_toggle, True);
     else
       SetToggle (p->depth8_toggle, True);

     AddColorCallbacks (p);

/*
 * Set the Display Palette check box.
 */
     SetToggle (p->opendoc_button, current_props->show_palette);

/*
 * Unset all PS view toggles, then set correct one.
 * Set the PS view toggle
 */
     SetToggle (p->colorps_toggle, False);
     SetToggle (p->bwps_toggle, False);

     if (strcmp (current_props->ps_view, PS_COLOR) == 0)
       SetToggle (p->colorps_toggle, True);
     else if (strcmp (current_props->ps_view, PS_MONO) == 0)
       SetToggle (p->bwps_toggle, True);
     else
       SetToggle (p->bwps_toggle, True);

/*
 * Set dirty flag to clean.
 */
     set_dirty_flag (False);

}

/*
 * Create the toggle buttons in the radio boxes
 * based on the visual.
 */
int
PropsViewButtonsCreate (p)
     PropertiesObjects  *p;
{
     Arg           al[64];           /* Arg List */
     register int  ac = 0;           /* Arg Count */
     XmString      xmstrings[16];    /* temporary storage for XmStrings */
     Widget        toggle;
     int           count = 0;

/*
 * Set Gray Scale and Color for View Image In.
 */
     if (gray_vis) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 210,
						"Gray Scale"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->grayscale_toggle = XmCreateToggleButtonGadget (p->radio1, 
						 "GrayScaleButton", al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     if (color_vis) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 211,
						"Color"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->color_toggle = XmCreateToggleButtonGadget (p->radio1, 
						      "ColorButton", al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

   return count;

}

/*
 * Create the toggle buttons in the radio boxes
 * based on the visual.
 * Returns the number of buttons created in
 * order to determine fraction base of radio box
 * when creating properties form.
 */
int
PropsColorButtonsCreate (p)
     PropertiesObjects  *p;
{
     Arg           al[64];           /* Arg List */
     register int  ac = 0;           /* Arg Count */
     XmString      xmstrings[16];    /* temporary storage for XmStrings */
     Widget        toggle;
     int           count = 0;

/*
 * Set number of colors for Colors choice.
 */
     if (depth1) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 212,
						"2 (B&W)"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->depth1_toggle = XmCreateToggleButtonGadget (p->radio2, "ButtonBW", 
						       al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     if (depth4) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 213,
						"16"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->depth4_toggle = XmCreateToggleButtonGadget (p->radio2, "Button16", 
						       al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     if (depth8) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 214,
						"256"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->depth8_toggle = XmCreateToggleButtonGadget (p->radio2, "Button256",
						       al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     if (depth16) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 215,
						"65536"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->depth16_toggle = XmCreateToggleButtonGadget (p->radio2, 
							"Button65536", al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     if (depth24) {
	ac = 0;
	xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 216,
						"Millions"));
	XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
	XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
	p->depth24_toggle = XmCreateToggleButtonGadget (p->radio2, 
						     "ButtonMillions", al, ac);
	XmStringFree (xmstrings[0]);
	count++;
     }

     return count;
}

/*
 * Create the toggle buttons in the radio boxes
 * based on the visual.
 * Returns the number of buttons created in
 * order to determine fraction base of radio box
 * when creating properties form.
 */
int
PropsPSButtonsCreate (p)
     PropertiesObjects  *p;
{
     Arg           al[64];           /* Arg List */
     register int  ac = 0;           /* Arg Count */
     XmString      xmstrings[16];    /* temporary storage for XmStrings */
     Widget        toggle;
     int           count = 0;

     ac = 0;     
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 440,
						      "Black & White"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->bwps_toggle = XmCreateToggleButtonGadget (p->radio3, 
						 "BWPSToggle", al, ac);
     count++;
     XmStringFree (xmstrings[0]);

     if (color_vis == True) {
       ac = 0;     
       xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 211,
							"Color"));
       XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
       XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
       p->colorps_toggle = XmCreateToggleButtonGadget (p->radio3, 
						     "ColorPSToggle", al, ac);
       count++;
       XmStringFree (xmstrings[0]);
     }

     return count;
}

/*
 * Callback for props OK.
 */
void
PropsOKCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PropertiesObjects *p = (PropertiesObjects *)client_data;
     char              *new_value;
     int                status;
     Boolean            set;
     extern int         ds_save_resources();

     setbusy();
/*
 * Get the current ViewImageIn value set.
 */
     if (current_props->view_in) free (current_props->view_in);
     current_props->view_in = strdup (PropsViewValue (p));
/*
 * Store in db.
 */
     new_value = strdup (current_props->view_in);
     free (resources [RES_VIEW_IN].value);
     resources [RES_VIEW_IN].value = new_value;
     ds_put_resource (&dsdb, "Imagetool", resources [RES_VIEW_IN].name,
		      resources [RES_VIEW_IN].value);
/*
 * Get the current Colors value set.
 */   
     if (current_props->color) free (current_props->color);
     current_props->color = strdup (PropsColorValue (p));
/*
 * Store in db.
 */
     new_value = strdup (current_props->color);
     free (resources [RES_COLORS].value);
     resources [RES_COLORS].value = new_value;
     ds_put_resource (&dsdb, "Imagetool", resources [RES_COLORS].name,
	             resources [RES_COLORS].value);
/*
 * Get the current OpeningDoc value set.
 */
     XtVaGetValues (p->opendoc_button, XmNset, &set, NULL);
     current_props->show_palette = (int) set;
/*
 * Store in db.
 */
     new_value = strdup (int_to_boolean ((int)set));
     free (resources [RES_SHOW_PALETTE].value);
     resources [RES_SHOW_PALETTE].value = new_value;
     ds_put_resource (&dsdb, "Imagetool", resources [RES_SHOW_PALETTE].name,
		      resources [RES_SHOW_PALETTE].value);
/*
 * Get the current PS value set.
 */
     if (current_props->ps_view) free (current_props->ps_view);
     current_props->ps_view = strdup (PropsPSValue (p));

/*
 * Store in db.
 */
     new_value = strdup (current_props->ps_view);
     free (resources [RES_PS_VIEW].value);
     resources [RES_PS_VIEW].value = new_value;
     ds_put_resource (&dsdb, "Imagetool", resources [RES_PS_VIEW].name,
		      resources [RES_PS_VIEW].value);
/*
 * Write out changes.
 */
     status = ds_save_resources (dsdb);
     if (status != 0) 
       AlertPrompt (base->top_level,
		    DIALOG_TYPE,  XmDIALOG_ERROR,
                    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		    DIALOG_TITLE, catgets (prog->catd, 1, 415, "Image Viewer - Options"),
		    DIALOG_TEXT, catgets (prog->catd, 1, 449, "Cannot save option changes."),
		    BUTTON_IDENT, 0, catgets (prog->catd, 1, 218, "Continue"),
		    HELP_IDENT, HELP_PROPS_ITEM,
		    NULL);
       
/*
 * Reset flag.
 */
     set_dirty_flag (False);
/*
 * Dismiss properties dialog if OK button only.
 */
     if (w == p->ok_button)
       DismissDialog (p->dialog);

     setactive();
}

/*
 * Callback for props Reset.
 */
void
PropsResetCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PropertiesObjects *p = (PropertiesObjects *)client_data;
/*
 * Free up current property entries.
 */
     free_resources ();

/*
 * Read in the properties from scratch.
 */
     free (current_props->view_in);
     free (current_props->color);
     free (current_props);
     read_props (False);
/*
 * Set items on props sheet accordingly.
 */
     PropsSetValues (p);
}

void
PropsCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PropertiesObjects *p = (PropertiesObjects *)client_data;

     XtPopdown (XtParent (p->dialog));
}

void
PropsQuitHandler (w, client_data, call_data)
     Widget      w;
     XtPointer   client_data;
     XtPointer   call_data;
{
     PropertiesObjects *p = (PropertiesObjects *)client_data;
     int  reply;

     if (!current_props->props_changed) {
       DismissDialog (properties->dialog);
       return;
     }

     reply = AlertPrompt (base->top_level,
	       DIALOG_TYPE,  XmDIALOG_QUESTION,
               DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
	       DIALOG_TITLE, catgets (prog->catd, 1, 416, "Image Viewer - Options"),
	       DIALOG_TEXT, catgets (prog->catd, 1, 450,
		 "You have made changes to the Options\nsettings that have not been applied.\nDo you still want to dismiss this window?"),
	       BUTTON_IDENT, 0, catgets (prog->catd, 1, 220, "Dismiss"),
	       BUTTON_IDENT, 1, catgets (prog->catd, 1, 221, "Cancel"),
               HELP_IDENT, HELP_PROPS_ITEM,
	       NULL);
     if (reply == 0) {
       DismissDialog (properties->dialog);
       set_dirty_flag (False);
     }
     else if (reply == 1)
       return;

}
