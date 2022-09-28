#ifndef lint
static char sccsid[] = "@(#)print.c 1.28 97/05/12";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <sys/param.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Dt/SpinBox.h>
#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "misc.h"
#include "state.h"
#include "ui_imagetool.h"

#define RASH_OPT	"rash "
#define WIDTH_OPT	"-W "
#define HEIGHT_OPT	"-H "
#define SCALE_OPT	"-s "
#define POSITION_OPT	"-l "
#define LANDSCAPE_OPT	"-r "
#define PRINT_OPT	"/usr/dt/bin/dtlp -s"
#define PRINTER_OPT	"-d "
#define COPIES_OPT	"-n "
#define HEADER_OPT	"-o \"-o nobanner\" "
#define TITLE_OPT	"-b "

#define SVR4_LIMIT	2048

extern float page_widths [];
extern float page_heights [];

typedef struct tnode{
      char *printer;
      struct tnode *left, *right;
} tnode;

static int      nprinters = 0;
static tnode   *root = NULL;

static void
set_lc_all (char *str)
{
   static char locale_str[128]; 

   if ((char *)getenv("LANG") == NULL)  /* ie. Eng. */
      return;
      
   if (str) {
      sprintf(locale_str, "%s=%s", "LC_ALL", str);
      if (putenv (locale_str)) 
         return;
   }
   else {  /* unset LC_ALL */
      sprintf(locale_str, "%s=%s", "LC_ALL", "");
      if (putenv(locale_str)) 
         return;
   }

}

tnode *
tree (p, printer, compare_func)
tnode	*p;
char	*printer;
int	(*compare_func)();
{
   char *cp, *strcpy();
   int cond;
 
   if (p == NULL) {
      p = (tnode*) malloc (sizeof (struct tnode));
      if ((cp = malloc (strlen (printer)+1)) != NULL)
         strcpy (cp, printer);
      p->printer = cp;
      p->left = p->right = NULL;
      }
   else 
      if ((cond = (*compare_func)(printer, p->printer)) == 0)
      ;
   else 
      if (cond < 0)
         p->left = tree (p->left, printer, compare_func);
   else
      p->right = tree (p->right, printer, compare_func);
       
   return(p);
}

#ifdef LATER
void
SetClientData (Widget list, int entry, caddr_t cookie)
{
     caddr_t   *client_data, *new_data;
     int        i, j = 0;

/*
 * If UserData already exists,
 * realloc UserData and insert blank data at entry.
 * Preserve other client_data pointers.
 */
     XtVaGetValues (list, XmNuserData, &client_data, NULL);
     new_data = (caddr_t *) malloc (sizeof (caddr_t) * nprinters);
     for (i = 0; i < nprinters; i++) {
       if (i == entry)
	 new_data[entry] = cookie;
       else if (client_data)
	 new_data[i] = client_data[i];
     }
     if (client_data) {
       free (client_data);
       client_data = NULL;
     }
     XtVaSetValues (list, XmNuserData, new_data, NULL);

}

caddr_t
GetClientData (Widget list, int entry) 
{
     caddr_t  *client_data;

     XtVaGetValues (list, XmNuserData, &client_data, NULL);
     if (client_data)
       return (client_data[entry]);
     else {
       return NULL;
     }
}
#endif

/*
 * Retrieves output from lpstat -d
 * This call must be done after setting locale to C.
 */
static char *
get_default_printer()
{
    char   *ptr;
    FILE   *fp;
    char   *printer;
    char   *default_printer = NULL;

    /* get the default printer from lpstat -d command */
    /* the output looks like "system default destination: spitfire\n" */
    fp = popen("lpstat -d", "r");
    if (fp) {
      char message[80];
      fgets(message, sizeof(message), fp);
      if ((ptr = (char*)strchr(message, ':')) != NULL) {
	strtok(ptr, " ");
	printer = (char *)strtok((char *)NULL, "\n");
	if (printer)
	  default_printer = strdup (printer);
      }
      pclose(fp);
    } /* end if fp */

    return default_printer;

}

static void
initialize_printers()
{
    static int   lpstated = FALSE;
    int          def_printer_exists = False;
    FILE 	*fp;
    char	 buf [256];
    char	*first_printer = NULL;
    char	*printer;
    char	*ptr;
    char	*buf_ptr;

    if (lpstated == TRUE)
      return;

    set_lc_all ("C");
/*
 * First try to get default printer from
 * $PRINTER or $LPDEST.
 */
    if (!prog->def_printer) 
      prog->def_printer = (char *)getenv ("PRINTER");
   
    if (!prog->def_printer) 
      prog->def_printer = (char *)getenv ("LPDEST");
/*
 * Then try to get default printer from lpstat -d
 */
    if (!prog->def_printer) 
      prog->def_printer = get_default_printer();

/*
 * Read in all printers from 'lpstat -v' command.
 */
    fp = popen ("lpstat -v", "r");
    if (fp) {
      while (fgets (buf, sizeof (buf), fp) != NULL) {

	if ((buf_ptr = strstr(buf, "for ")) != NULL) {

	  strtok (buf_ptr, " ");
	  printer = (char *) strtok (NULL, " ");
	  if ((int) strlen (printer) >= 1 && printer[strlen (printer) - 1] == ':')
	    printer[strlen(printer) - 1] = NULL;
	  
	  if (printer) {
	    nprinters++;
	    
	    if (first_printer == NULL)
	      first_printer = strdup (printer);
	    
	    if (!def_printer_exists && (prog->def_printer) &&
		(strcoll (prog->def_printer, printer) == 0)) {
	      def_printer_exists = True;
	      /* continue; */
	    }
	    root = tree (root, printer, strcmp);
	  }
	}
      }  
      pclose (fp);
    } 
    
/*
 * Check if prog->def_printer actually is set up on the system or
 * if we have a def_printer configured.
 * If not, fall back on lpstat -d output or first printer that comes up
 * when doing an 'lpstat -v'.
 */
    if (def_printer_exists == False || prog->def_printer == NULL) {
      prog->def_printer = get_default_printer();
      if (prog->def_printer == NULL && first_printer)
        prog->def_printer = strdup (first_printer);
    }

    if (first_printer) 
      free (first_printer);

    set_lc_all (NULL);

    lpstated = TRUE;

}

int
printer_exists ()
{
    if (prog->def_printer != (char *) NULL)
       return (TRUE);
/*
 * There can be no default printer, but still have printers
 * configured on your system.
 */
    initialize_printers();

    if (nprinters > 0)
      return TRUE;
    else {
      AlertPrompt (base->top_level,
		   DIALOG_TYPE, XmDIALOG_WARNING,
		   DIALOG_TITLE, catgets (prog->catd, 1, 408, "Image Viewer - Print"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 159,
			"Print service has not been configured for this system.\nPlease contact your system administrator for information\non enabling print service."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 409, "Continue"),
		   HELP_IDENT, HELP_PRINT_ITEM,
		   NULL);
      return (FALSE);
    }
}

void
PrintUnitsCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    int  value = (int) client_data;

    if (value == INCHES) {
      XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNuserData, "in", NULL);
      XtUnmanageChild (print->left_cm_label); 
      XtManageChild (print->top_inches_label);
      XtManageChild (print->left_inches_label);
   }
    else if (value == CM) {
      XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNuserData, "cm", NULL);
      XtUnmanageChild (print->top_inches_label);
      XtUnmanageChild (print->left_inches_label);
      XtManageChild (print->top_cm_label);
      XtManageChild (print->left_cm_label);
    }
    set_margin_value (value);
#ifdef LATER
/* 
 * Update print preview if showing.
 */
    if (print_preview && 
        (xv_get (print_preview->print_prev, XV_SHOW) == TRUE))
          position_image();
#endif
}


void
CopiesTextCallback (w, client_data, call_data)
     Widget                      w;
     XtPointer                   client_data;
     XtPointer			 call_data;
{
     XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *)call_data;
     PrintObjects  *p = (PrintObjects *)client_data;

     if (cbs == NULL)
       return;
     else if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE) {
/*
 * Return if backspace hit.
 */
       if (cbs->text->ptr == NULL)
	 return;
/*
 * Do not allow non-digits.
 */
       if (!isdigit (cbs->text->ptr[0])) {
	 cbs->doit = False;
	 return;
       }

     }
}

void
ImageSizeTextCallback (w, client_data, call_data)
     Widget                      w;
     XtPointer                   client_data;
     XtPointer			 call_data;
{
     XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *)call_data;
     PrintObjects  *p = (PrintObjects *)client_data;
     int               size_value;
     extern void       ImageSizeSliderCallback();

     if (cbs == NULL)
       return;
     else if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE) {
/*
 * Return if backspace hit.
 */
       if (cbs->text->ptr == NULL)
	 return;
/*
 * Do not allow non-digits.
 */
       if (!isdigit (cbs->text->ptr[0])) {
	 cbs->doit = False;
	 return;
       }

     }
     else if (cbs->reason == XmCR_VALUE_CHANGED) {
       XtRemoveCallback (p->image_size_slider, XmNdragCallback, 
			 ImageSizeSliderCallback, p);
       XtRemoveCallback (p->image_size_slider, XmNvalueChangedCallback, 
			 ImageSizeSliderCallback, p);

       XtVaGetValues (p->image_size_value, DtNposition, &size_value, NULL);
       XtVaSetValues (p->image_size_slider, XmNvalue, size_value, NULL);
       /*
        * Update Print Preview if showing.
	*/
       if (print_prev && print_prev->showing)
	 position_image();

       XtAddCallback (p->image_size_slider, XmNdragCallback, 
		      ImageSizeSliderCallback, p);
       XtAddCallback (p->image_size_slider, XmNvalueChangedCallback, 
		      ImageSizeSliderCallback, p);
     }
/*
 * User hit return in text field of spin box.
 * - Get the value of the text
 * - Set this value on the spin box.
 * - Set this value on the slider.
 * - Update print preview.
 */
     else if (cbs->reason == XmCR_ACTIVATE) {

       int  min, max, text_value = 100;
       char *text;

       XtRemoveCallback (p->image_size_slider, XmNdragCallback, 
			 ImageSizeSliderCallback, p);
       XtRemoveCallback (p->image_size_slider, XmNvalueChangedCallback, 
			 ImageSizeSliderCallback, p);

       XtVaGetValues (p->image_size_value, DtNminimumValue, &min,
		                           DtNmaximumValue, &max, NULL);
       text = XmTextGetString (p->image_size_text);
       if (text)
	 text_value = atoi (text);
       
       if (text_value < min) 
	 size_value = min;
       else if (text_value > max) 
	 size_value = max;
       else
	 size_value = text_value;
       
       XtVaSetValues (p->image_size_value, DtNposition, size_value, NULL);
       XtVaSetValues (p->image_size_slider, XmNvalue, size_value, NULL);
       /*
        * Update Print Preview if showing.
	*/
       if (print_prev && print_prev->showing)
	 position_image();

       XtAddCallback (p->image_size_slider, XmNdragCallback, 
		      ImageSizeSliderCallback, p);
       XtAddCallback (p->image_size_slider, XmNvalueChangedCallback, 
		      ImageSizeSliderCallback, p);
     }

}

void
MarginTextCallback (w, client_data, call_data)
     Widget                      w;
     XtPointer                   client_data;
     XtPointer			 call_data;
{
     XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *)call_data;
     PrintObjects  *p = (PrintObjects *)client_data;
     int            units_value;

     if (cbs == NULL)
       return;
     else if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE) {
/*
 * Return if backspace hit.
 */
       if (cbs->text->ptr == NULL)
	 return;
/*
 * Do not allow non-digits.
 */
       if (!isdigit (cbs->text->ptr[0]) && cbs->text->ptr[0] != '.') {
	 cbs->doit = False;
	 return;
       }

     }

/*
 * User hit return in text field of spin box.
 * - Get the value of the text
 * - Set this value on the spin box.
 * - Set this value on the slider.
 * - Update print preview.
 */
     else if (cbs->reason == XmCR_ACTIVATE) {

#ifdef LATER
       char *text;
 
/* Set min, max values on text for in, cm        */
      text = XmTextGetString (w);
       text_value = atoi (text);
       if (text_value < 0) 
	 units_value = 0;
       else if (text_value > ) 
	 units_value = max;
       else
	 units_value = text_value;
#endif       
       /*
        * Update Print Preview if showing.
	*/
       if (print_prev && print_prev->showing)
	 position_image();
     }

}

void
ImageSizeSliderCallback (w, client_data, call_data)
     Widget                 w;
     XtPointer              client_data;
     XtPointer		    call_data;
{
     XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
     PrintObjects *p = (PrintObjects *) client_data;
     extern void      ImageSizeTextCallback();

     if (cbs) {
       XtRemoveCallback (p->image_size_text, XmNmodifyVerifyCallback,
			 ImageSizeTextCallback, (XtPointer)p);
       XtRemoveCallback (p->image_size_text, XmNvalueChangedCallback,
			 ImageSizeTextCallback, (XtPointer)p);

       XtVaSetValues (p->image_size_value, DtNposition, cbs->value, NULL);
       /*
	* Update print preview if showing.
	*/
       if (print_prev && print_prev->showing)
	 position_image();

       XtAddCallback (p->image_size_text, XmNmodifyVerifyCallback,
		      ImageSizeTextCallback, (XtPointer)p);
       XtAddCallback (p->image_size_text, XmNvalueChangedCallback,
		      ImageSizeTextCallback, (XtPointer)p);
     }
}


void
ImagePositionCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PrintObjects *p = (PrintObjects *) client_data;
     Boolean       cset, mset;

     XtVaGetValues (p->centered_toggle, XmNset, &cset, NULL);
     XtVaGetValues (p->margin_toggle, XmNset, &mset, NULL);

/*
 * Centered: Gray out text fields, labels, units menu.
 */     
     if (cset == True && mset == False) {
       XtVaSetValues (p->top_text, XmNsensitive, False, NULL);
       XtVaSetValues (p->left_text, XmNsensitive, False, NULL);
       XtVaSetValues (p->top_inches_label, XmNsensitive, False, NULL);
       XtVaSetValues (p->left_inches_label, XmNsensitive, False, NULL);
       XtVaSetValues (p->top_cm_label, XmNsensitive, False, NULL);
       XtVaSetValues (p->left_cm_label, XmNsensitive, False, NULL);
       XtVaSetValues (XmOptionButtonGadget(p->units_menu), XmNsensitive, False, NULL);
     }
     else if (cset == False && mset == True) {
       XtVaSetValues (p->top_text, XmNsensitive, True, NULL);
       XtVaSetValues (p->left_text, XmNsensitive, True, NULL);
       XtVaSetValues (p->top_inches_label, XmNsensitive, True, NULL);
       XtVaSetValues (p->left_inches_label, XmNsensitive, True, NULL);
       XtVaSetValues (p->top_cm_label, XmNsensitive, True, NULL);
       XtVaSetValues (p->left_cm_label, XmNsensitive, True, NULL);
       XtVaSetValues (XmOptionButtonGadget(p->units_menu), XmNsensitive, True, NULL);
     }
/*
 * Update print preview if showing.
 */
     if (print_prev && print_prev->showing)
       position_image();

}

void
ImageOrientationCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
/* 
 * Update print preview if showing.
 */
     if (print_prev && print_prev->showing)
       position_image();

}


void
PageRangeCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PrintObjects *p = (PrintObjects *) client_data;
     Boolean          oset, pset, mset;

/*
 * If new choice is "This page (as image)" turn on all of the image
 * ops, turn off page order.
 */
     XtVaGetValues (p->this_page_toggle, XmNset, &pset, NULL);
     XtVaGetValues (p->orig_toggle, XmNset, &oset, NULL);

     if (pset == True && oset == False) {
       XtVaSetValues (print->image_size_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_value, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_text, XmNsensitive, True, NULL);
       XtVaSetValues (print->percent_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->image_size_slider, XmNsensitive, True, NULL);

       XtVaSetValues (print->orientation_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->portrait_toggle, XmNsensitive, True, NULL);
       XtVaSetValues (print->landscape_toggle, XmNsensitive, True, NULL);

       XtVaSetValues (print->position_label, XmNsensitive, True, NULL);
       XtVaSetValues (print->centered_toggle, XmNsensitive, True, NULL);
       XtVaSetValues (print->margin_toggle, XmNsensitive, True, NULL);

       XtVaGetValues (print->margin_toggle, XmNset, &mset, NULL);
       if (mset == True) {
	 XtVaSetValues (print->top_text, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_text, XmNsensitive, True, NULL);
	 XtVaSetValues (print->top_inches_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_inches_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->top_cm_label, XmNsensitive, True, NULL);
	 XtVaSetValues (print->left_cm_label, XmNsensitive, True, NULL);
	 XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNsensitive, True, NULL);
       }
     }

/*
 * if new choice is all, turn off image ops, turn on page range
 */
     
    else if (pset == False && oset == True) {
      XtVaSetValues (print->orientation_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->portrait_toggle, XmNsensitive, False, NULL);
      XtVaSetValues (print->landscape_toggle, XmNsensitive, False, NULL);
      
      XtVaSetValues (print->image_size_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->image_size_value, XmNsensitive, False, NULL);
      XtVaSetValues (print->image_size_text, XmNsensitive, False, NULL);
      XtVaSetValues (print->percent_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->image_size_slider, XmNsensitive, False, NULL);
      
      XtVaSetValues (print->position_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->centered_toggle, XmNsensitive, False, NULL);
      XtVaSetValues (print->margin_toggle, XmNsensitive, False, NULL);
      
      XtVaSetValues (print->top_text, XmNsensitive, False, NULL);
      XtVaSetValues (print->left_text, XmNsensitive, False, NULL);
      XtVaSetValues (print->top_inches_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->left_inches_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->top_cm_label, XmNsensitive, False, NULL);
      XtVaSetValues (print->left_cm_label, XmNsensitive, False, NULL);
      XtVaSetValues (XmOptionButtonGadget(print->units_menu), XmNsensitive, False, NULL);
    }

}

#ifdef LATER
void
margin_notify_proc (item, event)
Panel_item	 item;
Event		*event;
{
    double	 x;
    char 	*value = (char *) xv_get (item, PANEL_VALUE);

    if (check_float_value (value, &x) != 0)
       xv_set (item, PANEL_VALUE, DEFAULT_MARGIN, NULL);
    panel_advance_caret (print->controls);
/* 
 * Update print preview if showing.
 */
    if (print_preview && 
        (xv_get (print_preview->print_prev, XV_SHOW) == TRUE))
          position_image();
}
#endif

void
get_print_options (left_margin, top_margin, centered, orientation, scale, 
		   page_range)
double	*left_margin;
double	*top_margin;
int	*centered;
int	*orientation;
float	*scale;
int	*page_range;
{
    char       *value;
    double      x;
    int	        size;
    Boolean     set, sensitive;

    *centered = CENTERED;
    *left_margin = 1.0;
    *top_margin = 1.0;
    *scale = 1.0;
    *page_range = ALL_PAGES;
    *orientation = PORTRAIT;

    if (print != (PrintObjects *) NULL) {

      XtVaGetValues (print->range_label, XmNsensitive, &sensitive, NULL);
      if (sensitive == True) {
	XtVaGetValues (print->this_page_toggle, XmNset, &set, NULL);
	if (set == True)
	  *page_range = PAGE_AS_IMAGE;
	else
	  *page_range = ALL_PAGES;
      }

	  
      XtVaGetValues (print->orientation_label, XmNsensitive, &sensitive, 
		     NULL);
      if (sensitive == True) {
	XtVaGetValues (print->portrait_toggle, XmNset, &set, NULL);
	if (set == True)
	  *orientation = PORTRAIT;
	else
	  *orientation = LANDSCAPE;
      }

      XtVaGetValues (print->image_size_label, XmNsensitive, &sensitive, 
		     NULL);
      if (sensitive == True) {
	XtVaGetValues (print->image_size_slider, XmNvalue, &size, NULL);
	*scale = ((float) size) / 100.0;
      }

      XtVaGetValues (print->position_label, XmNsensitive, &sensitive, NULL);
      if (sensitive == True) {
	XtVaGetValues (print->centered_toggle, XmNset, &set, NULL);
	if (set == True)
	  *centered = CENTERED;
	else
	  *centered = MARGINS;

	if (*centered == MARGINS) {
	  value = XmTextGetString (print->top_text);

	  if (check_float_value (value, &x) != 0) 
	    XmTextSetString (print->top_text, catgets (prog->catd, 1, 160, "1.0"));
	  else
	    *top_margin = x;
	  
	  value = XmTextGetString (print->left_text);
	  if (check_float_value (value, &x) != 0) 
	    XmTextSetString (print->left_text, catgets (prog->catd, 1, 161, "1.0"));
	  else
	    *left_margin = x;

	  XtVaGetValues (XmOptionButtonGadget(print->units_menu), XmNuserData, &value, NULL);
	  if(value != NULL)
	    if (strcmp (value, "cm") == 0) {
	      *top_margin /= 2.54;
	      *left_margin /= 2.54;
	    }
	}
      }
    }
}
   
char *
get_printer_options (pagewidth, pageheight, copies, header, free_ptr) 
float	*pagewidth;
float	*pageheight;
int	*copies;
int	*header;
Boolean *free_ptr;
{
    int	 	   row, nrows;
    char	  *printer = NULL;
    Boolean        set;
    XmStringTable  strs;

    initialize_printers();
    printer = prog->def_printer;
    *pagewidth = page_widths [DEFAULT_PAGE_SIZE];
    *pageheight = page_heights [DEFAULT_PAGE_SIZE];
    *copies = 1;
    *header = HEADER_PAGE;
    *free_ptr = False;
    
    if (print != (PrintObjects *) NULL) {
       row = GetListSelection (print->printer_list);
       if (row == -1)
	 return NULL;
       XtVaGetValues (print->printer_list, XmNitems, &strs, 
					   XmNitemCount, &nrows, NULL);
       if (row < nrows) {
	 char  buf[MAXNAMELEN];
	 ConvertCompoundToChar (strs[row], &buf);
	 printer = strdup (buf);
	 *free_ptr = True;
       }
       else if (root != NULL)
	 printer = root->printer;

       row = GetListSelection (print->size_list);
       *pagewidth = page_widths [row];
       *pageheight = page_heights [row];

       XtVaGetValues (print->copies_value, DtNposition, copies, NULL);
       XtVaGetValues (print->header_checkbox, XmNset, &set, NULL);
       if (set == True)
	 *header = 1;
       else
	 *header = 0;
       }
    else if (root != NULL && printer == NULL) {
/*
 * Pick first printer from list here.
 */
      printer = root->printer;
    }

    return (printer);
}

void
print_error (image)
ImageInfo	*image;
{

     AlertPrompt (base->top_level,
		  DIALOG_TYPE,  XmDIALOG_ERROR,
                  DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		  DIALOG_TITLE, catgets (prog->catd, 1, 410, "Image Viewer - Print"),
		  DIALOG_TEXT, catgets (prog->catd, 1, 162,
					"Print request failed: Could not create temporary file for printout."),
		  BUTTON_IDENT, 0, catgets (prog->catd, 1, 163, "Continue"),
		  HELP_IDENT, HELP_PRINT_ITEM,
		  NULL);
	  
     if (image != (ImageInfo *) NULL) {
        image->cmap = NULL;
        destroy_image (image);
     }

     setactive ();
}
  
void
PrintButtonCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
    double 	 	 left_margin;
    double 	 	 top_margin;
    double		 bottom_margin;
    double	  	 scale_height;
    double	 	 scale_width;
    float		 scale_factor;
    float		 tmpf;
    float		 page_height;
    float		 page_width;
    int			 orientation;
    int			 position;
    int			 range;
    int		 	 copies;
    int			 header;
    char		*printer = NULL;
    char		*file_to_print;
    char		 tmp_buf [80];
    char                 error [1024];

    FILE		*popen_file;
    FILE		*out_file;
    int			 bytes_to_write;
    int		 	 bytes_written;
    char		 write_buf [SVR4_LIMIT];

    char		 print_cmd [2048];
    char		 print_cmd2 [2048];
    char		*ow;
    ImageInfo		*tmp_image = NULL;  
    unsigned int	 width, height, nbands;
    XilDataType	 	 datatype;
    float		 mult [1], offset [1];
    int			 status;
    int			 print_as_image = TRUE;
    int			 i, j = 0;
    Boolean              free_printer, 
                         rm_rashfile = False, rm_printfile = False;

    setbusy ();
    if (print)
      _DtTurnOnHourGlass (print->shell);

/* 
 * Determine if we supposed to print a ps file or print as an image.
 */

    get_print_options (&left_margin, &top_margin, &position, &orientation,
                       &scale_factor, &range);

    printer = get_printer_options (&page_width, &page_height, &copies, &header, &free_printer);
    if (printer == NULL && nprinters == 0) {
        AlertPrompt (base->top_level,
		     DIALOG_TYPE, XmDIALOG_WARNING,
		     DIALOG_TITLE, catgets (prog->catd, 1, 411, "Image Viewer - Print"),
		     DIALOG_TEXT, catgets (prog->catd, 1, 164,
                       "Print service has not been configured for this system.\nPlease contact your system administrator for information\non enabling print service."),
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 165, "Continue"),
		     HELP_IDENT, HELP_PRINT_ITEM,
		     NULL);
	if (print)
	  _DtTurnOffHourGlass (print->shell);
	setactive();
        return;
     }

/*
 * If a postscript doc, and printing all pages, just print the file.
 * If only a selected page, then prepend some ps to the front of the
 * the file which will cause only the selected page to get printed out.
 * Note that if it's only one page, then no other setting must have been
 * done (ie. no scale/no margins/etc.).
 */

    if (current_display == ps_display) {
       if (range != ALL_PAGES) {
          if (((position == CENTERED) ||
	       ((left_margin == 0.0) && (top_margin == 0.0))) &&
	      (scale_factor == 1.0)) {

/*
 * Ok, we can just add out "extra" stuff to the file and print it out
 */

	     mmap_handle_t	*file_ptr;

	     if (prog->printfile == (char *) NULL) 
	        make_tmpfile (&(prog->printfile), "print");
	     rm_printfile = True;
	     if (current_image->compression == UNIX)
	        file_ptr = fast_read (prog->uncompfile);
	     else	
	        file_ptr = fast_read (current_image->realfile);
	     status = fast_write (prog->printfile, file_ptr->addr, 
		file_ptr->len, current_state->reversed ?
		current_image->pages - current_state->current_page + 1:
		current_state->current_page);
	     fast_close (file_ptr);
	     if (status != 0) {
	        print_error (NULL);
		if (print)
		  _DtTurnOffHourGlass (print->shell);
		setactive();
	        return;
	        }
   
	     file_to_print = prog->printfile;
	     print_as_image = FALSE;
	     }
	  }
       else
	  print_as_image = FALSE;
       }
	  
/*
 * If we're not printing all pages (ie. not a ps file with the the
 * print all pages selection made) , then do all of this to determine
 * where to print the image on the page.
 */

    if (print_as_image == TRUE) {

/*
 * Create a tmp_image (ImageInfo *) and fill correctly. Then do the
 * rast_save to save out a temporary raster file in /tmp (check 
 * TMPDIR first).  Then do the rash and pipe to lp.
 */

/*
 * First, check if the current file has been altered... if not
 * then we can just use that file (if a sun raster file), and forget about 
 * the tmp file. Use prog->printfile.
 */

       if ((current_image->realfile != (char *) NULL) &&
	   ((current_state->undo).op == NO_UNDO) &&
	   (current_image->type_info->type == RASTER)) 

/*
 * Even though its a raster file, there is a chance that we could have
 * data, compressed data or a compressed file. Therefore, call file_to_open
 * to get the right file (it will write out the file and/or uncompress if
 * necessary).
 */
	  file_to_print = (char *) file_to_open (current_image->file,
						 current_image->realfile,
						 current_image->compression,
						 current_image->data,
						 current_image->file_size);
       else {
	  if (prog->printfile == (char *) NULL) 
	     make_tmpfile (&(prog->printfile), "print");
	  rm_printfile = True;
          tmp_image = init_image (prog->printfile, prog->printfile, 0, 
				  str_to_ftype ("Sun-raster"), 
				  (char *) NULL);

          tmp_image->depth = current_display->depth;

/*
 * If we're viewing a ps file, then have to save it out to an xil image.
 */

	  if (current_display == ps_display) {
	     if (prog->xil_opened == FALSE) {
	       BaseImageCanvasObjectsCreate (base);
	       if (prog->xil_opened == False) {
		 if (print)
		   _DtTurnOffHourGlass (print->shell);
	         setactive ();
	         return;
	       }
	     }
	     tmp_image->orig_image = create_image_from_ps (tmp_image);
	     if (current_display->depth == 8 || current_display->depth == 4)
       	       save_colormap (tmp_image);
       	       compress_colors (tmp_image);
	     }
	  else {
	     u_char       *tmp_data;
	     unsigned int  pixel_stride, scanline_stride;

	     current_image->saving = TRUE;
	     assign_display_func(current_image, tmp_image->depth);
	     current_image->saving = FALSE;
             copy_image_data (tmp_image, current_image);
             (*xil->get_info) (current_image->view_image, &width, &height,
			      &nbands, &datatype);
	     tmp_image->orig_image = (current_image->display_func) 
							(current_image->dest_image);
	     tmp_data = retrieve_data (tmp_image->orig_image, &pixel_stride,
				       &scanline_stride);
             reimport_image(tmp_image->orig_image, FALSE);

	     tmp_image->bytes_per_line = scanline_stride;
	     tmp_image->width = width;
	     tmp_image->height = height;
	     tmp_image->dithered24to8 = current_image->dithered24to8;
             if (tmp_image->dithered24to8 != 0) {
	        mult [0] = 1.0;
	        offset [0] = (float) tmp_image->offset * -1.0;
	        (*xil->rescale) (tmp_image->orig_image, tmp_image->orig_image, mult,
			      offset);
                }
	     /* 
	      * Restore current_image. Note that there is a side effect of 
	      * calling assign_display_func when colours have not been dithered 
	      * on an 8 bit display. This means that we do not call 
	      * assign_display_func in this case, but correct the image directly.
	      */
	     if(nbands == 1 && current_image->colors < 216 
						&& current_image->depth == 8) {

		 float c;
		 c = (float)current_image->offset;
		 if (c) {
		    (*xil->add_const) (current_image->dest_image, &c, 
							current_image->dest_image);
	            } 
	     }
	     else {
		assign_display_func(current_image, current_display->depth); 
		}
	     }
          
	  status = (*tmp_image->type_info->save_func) (tmp_image);
          if (status != 0) {
	     print_error (tmp_image);
	     if (print)
	       _DtTurnOffHourGlass (print->shell);
	     setactive();
	     return;
	     }

	  file_to_print = prog->printfile;
          }

/*
 * Start building command ...
 */

       ow = getenv ("OPENWINHOME");
       if (ow == (char *) NULL)
          ow = "/usr/openwin";

       sprintf (print_cmd, "%s/bin/%s", ow, RASH_OPT);

/*
 * Check orientation next, cause that affects top and left margins.
 */

       if (orientation == LANDSCAPE) {
          tmpf = page_height;
          page_height = page_width;
          page_width = tmpf;
          strcat (print_cmd, LANDSCAPE_OPT);
          }

/*
 * Check if centered, or positioned
 */

       if (position != CENTERED) {

/*
 * Figure out what bottom margin is.. We always do everything in inches
 * for now...
 */
          bottom_margin = page_height - top_margin - 
			  (current_image->height / current_display->res_y);
   
          sprintf (tmp_buf, "%s%5.2fin %5.2fin ", POSITION_OPT,
		  	     left_margin, bottom_margin);

          strcat (print_cmd, tmp_buf);
          }

/*
 * Get scale factors, or absolute height/width...
 * Note: Must have scale factor in here for this to work!
 */

       scale_height = current_image->height / current_display->res_y * scale_factor;
       scale_width = current_image->width / current_display->res_y * scale_factor;

       sprintf (tmp_buf, "%s%5.2fin %5.2fin ", SCALE_OPT, scale_width, 
				   scale_height);
       strcat (print_cmd, tmp_buf);

/*
 * Add file to be rash-ed
 */

       strcat (print_cmd, file_to_print);

/*
 * Ok, create temp file for rash output and append file names to command.
 */
       if (prog->rashfile == (char *) NULL) 
	 make_tmpfile (&(prog->rashfile), "rash");
       rm_rashfile = True;
/*
 * Check for any "&" in string in case filename has a '&' in it.
 * Place a '\'in front if the '&'.
 */
       j = 0;
       for (i = 0; i < (int) strlen (print_cmd); i++) {
         if (print_cmd[i] == '&')
	   print_cmd2[j++] = '\\'; 
         print_cmd2[j++] = print_cmd[i];
       }
       print_cmd2[j] = '\0';
       strcpy (print_cmd, print_cmd2);

/* 
 * Done with rash setup.. do it and write to rash file.
 * Note, we use popen/fread/fwrite so we can determine if the 
 * write is failing (probably a file system full message).
 */

       popen_file = popen (print_cmd, "r");
       if (popen_file == (FILE *) NULL) {
	  print_error (tmp_image);
	  if (print)
	    _DtTurnOffHourGlass (print->shell);
	  setactive();
	  return ;
  	  }
       out_file = fopen (prog->rashfile, "w");
       if (out_file == (FILE *) NULL) {
	  print_error (tmp_image);
	  pclose (popen_file);
	  if (print)
	    _DtTurnOffHourGlass (print->shell);
	  setactive();
	  return ;
  	  }
	  
       while ((bytes_to_write = fread (write_buf, 1, SVR4_LIMIT, popen_file))
				!= 0) {
	  bytes_written = fwrite (write_buf, 1, bytes_to_write, out_file);
  	  if (bytes_written != bytes_to_write) {
	     print_error (tmp_image);
	     pclose (popen_file);
	     fclose (out_file);
	     unlink (prog->rashfile);
	     if (print)
	       _DtTurnOffHourGlass (print->shell);
	     setactive();
	     return;
	     }
	  }
/*
 * Done rashing, rm file that was rash-ed.
 */
       if (rm_printfile == True)
	 unlink (prog->printfile);
       file_to_print = prog->rashfile;
       fclose (out_file);
       pclose (popen_file);

       }

/*
 * Now, start print cmd.
 */
    sprintf (print_cmd, "%s %s%s %s%s ", PRINT_OPT, PRINTER_OPT, 
	     printer, TITLE_OPT, basename (current_image->file));

/*
 * Get # of copies..
 */

    if (copies != 1) {
       sprintf (tmp_buf, "%s%d ", COPIES_OPT, copies);
       strcat (print_cmd, tmp_buf);
       }

/*
 * Print header page??
 */
    if (header == NO_HEADER_PAGE)
       strcat (print_cmd, HEADER_OPT);

/*
 * If we're printing an entire ps file, add filename.
 * Note, check here if we have a compressed file, or data, or compressed
 * data, and use the correct file name. Note that if we have data
 * we may have not written out the data, so do it now.
 */

    if (current_display == ps_display) {
       if (range == ALL_PAGES) {
          if (current_image->compression == UNIX)
	     file_to_print = prog->uncompfile;
          else if (current_image->data != (char *) NULL) {
	     if (write_tmpfile (&(prog->uncompfile), current_image->data, 
			        basename (current_image->file),
                                current_image->file_size) == -1) {
	        print_error (tmp_image);
		if (print)
		  _DtTurnOffHourGlass (print->shell);
		setactive();
	        return ;
	      }
             file_to_print = prog->uncompfile;
	   }
          else
	    file_to_print = current_image->realfile;
       }
       
       strcat (print_cmd, file_to_print);
       }
    else {
       file_to_print = prog->rashfile;
       strcat (print_cmd, file_to_print);
     }
/*
 * Check for any "&" in string in case filename has a '&' in it.
 * Place a '\'in front if the '&'.
 */
    j = 0;
    for (i = 0; i < (int) strlen (print_cmd); i++) {
      if (print_cmd[i] == '&')
	print_cmd2[j++] = '\\'; 
      print_cmd2[j++] = print_cmd[i];
    }
    print_cmd2[j] = '\0';
    strcpy (print_cmd, print_cmd2);

/*
 * Do this instead of surpressing output with -o -s.
 */
    strcat (print_cmd, " > /dev/null");

/*
 * Add rm command to remove rashfile after queued.
 * Command will look like "(lp [options] filename; rm filename)&"
 */
    if (rm_rashfile == True) {
      print_cmd2[0] = '\0';
      strcat (print_cmd2, print_cmd);
      sprintf (print_cmd, "%s; rm %s", print_cmd2, file_to_print);
    }

/*
 * Do it!
 */
    if (system (print_cmd) != 0) {
      sprintf (error, catgets (prog->catd, 1, 166, 
	    "Unable to print %s.\nCheck console window for error messages."), current_image->file);
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 412, "Image Viewer - Print"),
		   DIALOG_TEXT, error,
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 167, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
    }
    else
      SetFooter (base->left_footer, catgets (prog->catd, 1, 437,
					     "Print job queued.")); 		   
/*
 * Avoid cmap from being freed by setting to NULL.
 * The cmap was copied from current_image.
 */
    if (tmp_image != (ImageInfo *) NULL) {
       tmp_image->cmap = NULL;
       destroy_image (tmp_image);
    }
/*
 * Free printer pointer if flag set.
 */  
    if (free_printer == True && printer != NULL)
      free (printer);

    setactive ();

    if (print) {
/*
 * if print dialog showing and this wasn't called from PrintOne
 * then remove dialog.
 */
      if (print->showing == True && w != NULL)
        XtPopdown (XtParent (print->dialog)); 

      _DtTurnOffHourGlass (print->shell);
    }
    
}



void 
treelist (p, i, list)
tnode		*p;
int		*i;
Widget           list;
{
   if (p != NULL) {
      int       pos;
      XmString  str;

      treelist (p->left, i, list);
      pos = (*i) - 1;
      str = XmStringCreateSimple (p->printer);
      XmListAddItemUnselected (list, str, pos);
      XmStringFree (str);				  
      (*i)++;
      treelist (p->right, i, list);
   }
}
 
void
SetPrinterList (list)
    Widget  list;
{
    int       n = nprinters;
    XmString  str;

    initialize_printers();
    treelist (root, &n, list);
    if (prog->def_printer) {

      str = XmStringCreateSimple (prog->def_printer);
      XmListSelectItem (list, str, False);
      XmStringFree (str);
    }
}

void
PrintCancelCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     PrintObjects *p = (PrintObjects *)client_data;

     XtPopdown (XtParent (p->dialog));
}
