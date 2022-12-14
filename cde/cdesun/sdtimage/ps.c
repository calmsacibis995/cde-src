#ifndef lint
static char sccsid[] = "@(#)ps.c 1.58 97/06/11";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * ps.c - All functions dealing with postscript docs.
 */

#define CR 10
#define NL 13

#include <signal.h>
#include <math.h>
#include <wait.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <DPS/dpsfriends.h>
#include <DPS/dpsXclient.h>
#include <X11/Xatom.h>
#include <Xm/Text.h>
#include <Dt/SpinBox.h>

#include "display.h"
#include "help.h"
#include "image.h"
#include "imagetool.h"
#include "imagetool_dps.h"
#include "misc.h"
#include "props.h"
#include "state.h"
#include "ui_imagetool.h"
#include "tags.h"

#define  PAGECOUNTER	"sdtpagecounter"
#define  PS_ADOBE	"%!PS-Adobe-"
#define  MAX_MAGIC      200     /* Arbitary Max size for MAGIC id of file */

#define  CLEAR_MESSAGE       0
#define  COUNTING_MESSAGE    1
#define  MONOCHROME_MESSAGE  2

extern	int		 choices;
extern  Pixmap		*pixmaps;

Pixmap		 	 small_pixmap = NULL;

float			 ctm_a,
			 ctm_b,
			 ctm_c,
			 ctm_d,
			 ctm_tx,
			 ctm_ty,
			 small_ctm_a;

int			 make_pixmaps;
int		 	 reversed_pages = FALSE;
int			 timeout_value = 60;
FILE			*current_file;
char			*current_ps_filename;
short			*page_errors;
DPSContext		 dps_context;
DPSContext  		 small_context = NULL;
XStandardColormap	 *gray_cmap = NULL;
XStandardColormap	 *rgb_cmap = NULL;
XStandardColormap	 *small_gray_cmap = NULL;
XStandardColormap	 *small_rgb_cmap = NULL;
GC			 small_gc = NULL;
int			 page_requested = 0;
int			 rendering = FALSE;
int			 waiting_to_exit = FALSE;
int			 page_one_error = -1;
int			 started_newfile = FALSE;
int			 reversed_total_pages = 0;
int			 pipe1 [2],
			 pipe2 [2];
char			*forked_app = NULL;
FILE			*pipe_fd;
int			 npages = 0;
pid_t			 child_pid = 0;
int		 	 rendering_pages = FALSE;
int		 	 pages_rendered = FALSE;
int			 started_thumbnails = FALSE;
int			 pixmaps_switched = FALSE;
int			 timeout_hit = FALSE;
struct itimerval	*ps_timeout = NULL;
XtInputId                input_id;
void		       (*old_sigpipe_handler)(int);

/* 
 * Additional vars needed for DSC method (uses document structuring
 * comments to determine where each page begins and ends.
 */

int			 using_dsc = FALSE;
int			 dsc_page_error = FALSE;
long 			*pagestart = NULL;    
				/* array of file pointers at %%Page comments */
long  			 end_prolog;           
				/* file pointer at the %%EndProlog comment */
long  			 begin_setup;          
				/* file pointer at the %%BeginSetup comment */
long  			 end_setup;            
				/* file pointer at the %%EndSetup comment */
long  			 trailer;             
				/* file pointer at the %%Trailer comment */
int   			 number_of_pages = 0;   /* number of pages */
int   		 	 in_imported_document;  
				/* bool if in imported document */

typedef enum {
	GET_TOTAL_PAGES,
	GET_NEXT_PAGE,
	NO_OP
} dps_ops;
	
/*
 * do_dps_op - set up an Xevent to send to ourselves to process the
 * 	       event later... just in case we're still processing
 *	       another X event when this dps event comes in.
 */
void
do_dps_op (dps_op)
int	dps_op;
{
    XClientMessageEvent		client_event;

    client_event.type = ClientMessage;
    client_event.message_type = XA_INTEGER;
    client_event.format = 32;
    client_event.data.l[0] = dps_op;
    client_event.window = ps_display->win;
    XSendEvent (ps_display->xdisplay, ps_display->win, False, NoEventMask,
		(XEvent *) &client_event);

    XPending (ps_display->xdisplay);
}
	
void 
dps_status_handler (ctxt, code)
DPSContext ctxt;
int	   code;
{

/*
 * We only look for ZOMBIE and FROZEN events...
 */

    if (prog->verbose) 
       fprintf (stderr, catgets (prog->catd, 1, 232, 
				 "%s: got ps status: %d\n"), prog->name, code);

    if (code == PSZOMBIE) {
       if (dps_context != (DPSContext) NULL) {
          DPSDestroySpace (DPSSpaceFromContext (dps_context));
          dps_context = (DPSContext) NULL;
          }
       }
    else if (code == PSFROZEN) { 
       if (prog->verbose)
          fprintf (stderr, "just got page: %d\n", page_requested);
       rendering = FALSE;
       if (page_requested != 0) 
          current_state->next_page = page_requested;
       page_requested = 0;
       }
}

void
dps_text_handler (ctxt, buf, count)
DPSContext  ctxt;
char	   *buf;
int	    count;
{
    if (strncmp (buf, ERROR_STRING, strlen (ERROR_STRING)) ==  0) {
       char *tmp = buf + strlen (ERROR_STRING);
       int page_number = atoi (tmp);
   
       if (started_newfile == TRUE)
	  page_one_error = page_number;
       else if (current_state->using_dsc == TRUE)
	  dsc_page_error = TRUE;
       else {
	  if (current_state->reversed == TRUE)
	     page_number = current_image->pages - page_number - 1;
          page_errors [page_number] = TRUE;
	  if ((current_state->current_page - 1) == page_number) 
	    SetFooter (base->left_footer, catgets (prog->catd, 1, 233,
		       "A PostScript error was found on this page."));
	  if (prog->verbose == TRUE)
	     fprintf (stderr, "got an error on page: %d\n", page_number);
	  return;
	  } 
       }
    else if (strncmp (buf, REVERSE_PAGES_STRING, 
		      strlen (REVERSE_PAGES_STRING)) ==  0) {
       char *tmp = buf + strlen (REVERSE_PAGES_STRING);

       reversed_total_pages = atoi (tmp);
       return;
       }
    else if (strncmp (buf, HAS_COLOR, strlen (HAS_COLOR)) == 0) {
       if (prog->verbose)
         printf ("imagetool found a color operator\n");
       SetFooter (base->left_footer, catgets (prog->catd, 1, 448,
		  "This document may use color.  Use Options to change setting."));
       XBell (ps_display->xdisplay, 0);
       prog->footer_priority = MONOCHROME_MESSAGE;
       return;
    }


    if (prog->verbose) {
       char *tmp = malloc (count + 1);

       strncpy (tmp, buf, count);
       tmp[count] = '\0';
       fprintf (stderr, "%s", tmp);
       free (tmp);
       }
}

void
dps_error_handler (ctxt, error_code, arg1, arg2)
DPSContext		ctxt;
DPSErrorCode		error_code;
long unsigned int	arg1;
long unsigned int	arg2;
{
    char	*prefix = catgets (prog->catd, 1, 234, "%% [Error: ");
    char	*suffix = catgets (prog->catd, 1, 235, " ] %%\n");
    char	*infix = catgets (prog->catd, 1, 236, " Offending Command: ");
    char	*nameinfix = catgets (prog->catd, 1, 237, "User name too long; Name: ");
    char	*contextinfix = catgets (prog->catd, 1, 238, "Invalid context: ");
    char	*taginfix = catgets (prog->catd, 1, 239, "Unexpected wrap result tag: ");
    char	*typeinfix = catgets (prog->catd, 1, 240, "Unexpected wrap result type; tag: ");
    char 	*buf1;
    char	 buf2 [256];
    DPSBinObj	 ary;
    DPSBinObj	 elements;
    char 	*error_str;
    char	*error_name;
    int 	 error_count;
    int		 error_name_count;
    int  	 resync_flag;
    unsigned char tag;
    unsigned long enable_mask, 
		  disable_mask,
	   	  next_mask;

    if (prog->verbose) 
       fprintf (stderr, catgets (prog->catd, 1, 241,
		"%s: got ps error: %d\n"), prog->name, error_code);

    switch (error_code) {
       case dps_err_ps: 
    	    buf1 = (char *) arg1;
 	    ary = (DPSBinObj) (buf1 + DPS_HEADER_SIZE);

  	    elements = (DPSBinObj) (((char *) ary) + ary->val.arrayVal);
	    error_name = (char *) (((char *) ary) + elements [1].val.nameVal);
	    error_name_count = elements [1].length; 

	    error_str = (char *) (((char *) ary) + elements [2].val.nameVal);
	    error_count = elements [2].length;

	    resync_flag = elements [3].val.booleanVal;

	    dps_text_handler (ctxt, prefix, strlen (prefix));
	    dps_text_handler (ctxt, error_name, error_name_count);
	    dps_text_handler (ctxt, infix, strlen (infix));
	    dps_text_handler (ctxt, error_str, error_count);
	    dps_text_handler (ctxt, suffix, strlen (suffix));
	
	    break;

       case dps_err_nameTooLong:
	    buf1 = (char *) arg1;
	    dps_text_handler (ctxt, prefix, strlen (prefix));
	    dps_text_handler (ctxt, nameinfix, strlen (nameinfix));
	    dps_text_handler (ctxt, buf1, arg2);
	    dps_text_handler (ctxt, suffix, strlen (suffix));
	    break;
	     
       case dps_err_invalidContext:
	    sprintf (buf2, "%s %s %d %s", prefix, contextinfix, arg1,
		     suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;

       case dps_err_resultTagCheck:
	    tag = *((unsigned char *) arg1 + 1);
	    sprintf (buf2, "%s %s %d %s", prefix, taginfix, tag, 
		     suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;

       case dps_err_resultTypeCheck:
	    tag = *((unsigned char *) arg1 + 1);
	    sprintf (buf2, "%s %s %d %s", prefix, typeinfix, tag, 
		     suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;

       case dps_err_invalidAccess:
	    sprintf (buf2, catgets (prog->catd, 1, 242, 
			   "%s Invalid context access. %s"), prefix, suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;

       case dps_err_encodingCheck:
	    sprintf (buf2, catgets (prog->catd, 1, 243, 
		     "%s Invalid name/program encoding: %d/%d. %s"),
		     prefix, (int) arg1, (int) arg2, suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;
	   
       case dps_err_closedDisplay:
	    sprintf (buf2, catgets (prog->catd, 1, 244,
		     "%s Broken display connection %d. %s"),
		     prefix, (int) arg1, suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;
       
       case dps_err_deadContext:
	    sprintf (buf2, catgets (prog->catd, 1, 245,
		     "%s Dead context 0x0%x. %s"),
		     prefix, (int) arg1, suffix);
	    dps_text_handler (ctxt, buf2, strlen (buf2));
	    break;
       
       default:
	    buf1 = catgets (prog->catd, 1, 246, "Unknown error from DPS\n");
	    dps_text_handler (ctxt, buf1, strlen (buf1));
       
       }

    if (dps_context != (DPSContext) NULL) {
       DPSDestroySpace (DPSSpaceFromContext (dps_context));
       dps_context = (DPSContext) NULL;
       }
}

#ifdef NEVER
void
extension_event_proc (dpy, xevent, win)
Display		*dpy;
XEvent		*xevent;
Xv_object	 win;
{
    if (XDPSIsDPSEvent (xevent)) 
       XDPSDispatchEvent (xevent);
}
#endif

/*
 * got_small_page - just got a small page, copy it onto new pixmap;
 */

void
got_small_page ()
{
    Pixmap	new_pixmap;
    Boolean     sensitive;

    npages ++;

    page_errors = (short *) realloc (page_errors, sizeof (short) * npages);
    page_errors [npages - 1] = FALSE;

/* 
 * If we just got the 2nd page, then unfreeze the context so the
 * user can go to the next page if he wants to.
 */
    if ((npages == 2) && (current_state->reversed == FALSE) &&
	(current_state->using_dsc == FALSE)) {
       page_requested = 2;
       do_dps_op (GET_NEXT_PAGE);
       if (pg_controls) {
	 XtVaSetValues (pg_controls->order_label, XmNsensitive, True, NULL);
	 XtVaSetValues (pg_controls->fpf, XmNsensitive, True, NULL);
	 XtVaSetValues (pg_controls->lpf, XmNsensitive, True, NULL);
       }
     }

    if (npages  > choices) {
        pageview_pixmaps_create (npages);
/* 	current_image->pages = npages;*/
    }

    if (prog->verbose)
       fprintf (stderr, "got page from sdtpagecounter: %d\n", npages);

    if (ps_display->depth == 1) 
      XCopyPlane (ps_display->xdisplay, small_pixmap, pixmaps [npages - 1], 
		  small_gc, 0, 0, (int) (ps_display->pagewidth * 6.0),
		  (int) (ps_display->pageheight * 6.0), 0, 0, 1L);
    else 
      XCopyArea (ps_display->xdisplay, small_pixmap, pixmaps [npages - 1],
		 small_gc, 0, 0, (int) (ps_display->pagewidth * 6.0),
		 (int) (ps_display->pageheight * 6.0), 0, 0);

    XSync (ps_display->xdisplay, 0);

    if ((current_state->reversed == FALSE) && 
	(current_state->using_dsc == FALSE)) {
       current_image->pages = npages * -1;
       update_page_number (current_state->current_page, current_image->pages, 
			   current_state->current_page);
       }
  
    XtVaGetValues (base->pgfwd_button, XmNsensitive, &sensitive, NULL);
    if (sensitive == False && npages > 2)
      XtVaSetValues (base->pgfwd_button, XmNsensitive, True, NULL);
}


/*
 * sigpipe_handler - function that is called if we get a SIGPIPE, meaning
 *  		     that something happened with our pipes between imagetool
 * 		     and pagecounter.
 */

void
sigpipe_handler (sig)
int		sig;
{
    extern void render_small_pages ();

    if (child_pid != 0) {
       int child_killed;
       kill (child_pid, SIGKILL);
       child_killed = wait ((int *) NULL);
       if (input_id != 0) {
         XtRemoveInput (input_id); 
         input_id = 0;
       }
       signal (SIGPIPE, old_sigpipe_handler);
       close (pipe1 [1]);
       close (pipe2 [0]);
       child_pid = 0;
       }

    render_small_pages (current_image, TRUE, TRUE);
}


void
ReadFromPagecounter (client_data, fd, id)
     XtPointer   client_data;
     int        *fd;
     XtInputId  *id;
{
    char	input_buf [80];
    int		nbytes;

    nbytes = read (*fd, input_buf, 80);

/*
 * First check if pagecounter is exiting...
 */
    if (!nbytes || input_buf[0] == '\n') {

	
/*
       notify_set_input_func (ps_display->canvas, NOTIFY_FUNC_NULL, pipe2 [0]);
       notify_set_signal_func (client, NOTIFY_FUNC_NULL, SIGPIPE, NOTIFY_SYNC);
       close (pipe1 [1]);
       close (pipe2 [0]);
*/
       return;
    }
/*
 * then check if pagecounter is exiting...
 */
    else if (strncmp (input_buf, PAGECOUNTER_END_STRING,
	     strlen (PAGECOUNTER_END_STRING)) == 0) {
/*
 * Sometimes this is called twice so
 * check to make sure we're not doing this twice.
 */
       if (input_id != 0) {
	 input_id = 0;
         XtRemoveInput (*id);
       }
       signal (SIGPIPE, old_sigpipe_handler);
       close (pipe1 [1]);
       close (pipe2 [0]);
       return;

    }
/*
 * Check if the pagecounter found a color operator
 */
     else if (strncmp (input_buf, PAGECOUNTER_COLOR_STRING,
		      strlen (PAGECOUNTER_COLOR_STRING)) == 0) {
       if (prog->verbose)
         printf ("sdtpagecounter found a color operator\n");
       SetFooter (base->left_footer, catgets (prog->catd, 1, 448,
		  "This document may use color.  Use Options to change setting."));
       XBell (ps_display->xdisplay, 0);
       nbytes = fwrite (CONTINUE_STRING, 1, strlen (CONTINUE_STRING), pipe_fd);
       fflush (pipe_fd);
       prog->footer_priority = MONOCHROME_MESSAGE;
       return;
       }

/*
 * Now check if we just rendered a page.
 * If we didn't get back what we expected, kill the child process
 * and default to the old slow way. 
 */

     else if (strncmp (input_buf, RENDERED_PAGE_STRING, 
		      strlen (RENDERED_PAGE_STRING)) != 0) {
       int child_killed;
       kill (child_pid, SIGKILL);
       child_killed = wait ((int *) NULL);
       return;
       }

    got_small_page ();

    nbytes = fwrite (CONTINUE_STRING, 1, strlen (CONTINUE_STRING), pipe_fd);
    fflush (pipe_fd);
    return;
}

void
set_pageview_pages (pageno)
int pageno;
{
    int   i;

    if ((pg_overview != NULL) && (pg_overview->showing == True)
	 && (rendering_pages == FALSE)) {
      /*
       * Unset all buttons
       */
      for (i = 0; i < current_image->pages; i++)
	XmToggleButtonGadgetSetState (pg_overview->buttons[i], False, False);
      /*
       * Set current page button.
       */
      XmProcessTraversal (pg_overview->buttons[pageno-1], XmTRAVERSE_CURRENT);
      XmToggleButtonGadgetSetState (pg_overview->buttons[pageno-1], True, False);
      /*
       * Set page value on spin box.
       */
      XtVaSetValues (pg_overview->goto_value, 
		     DtNposition, pageno, 
		     NULL);
    }

}

/*
 * got_total_pages - received total pages either from pagecounter app
 *		     or by using render_small_pages.
 */

void
got_total_pages ()
{
    short old_error;
    int   i;

    if (current_image->pages < 0) {

/*
 * If npages is 0, then we didn't get any pages? What this means is
 * that there wasn't a showpage/copypage in the file, so assume one page.
 */

       if (npages == 0)
	  npages = 1;
       current_image->pages = npages;
       update_page_number (current_state->current_page, current_image->pages,
			   current_state->current_page);
       page_errors = (short *) realloc (page_errors, sizeof (short) * npages);
       page_errors [npages - 1]  = FALSE;

       if (page_errors [current_state->current_page - 1] == TRUE)
	 SetFooter (base->left_footer, catgets (prog->catd, 1, 247,
		    "A PostScript error was found on this page."));
       else
 	 if (prog->footer_priority <= COUNTING_MESSAGE) {
	   SetFooter (base->left_footer, "");
	   prog->footer_priority = CLEAR_MESSAGE;
	 }
       }
	
    if (current_image->pages > 1) {
       Boolean  sensitive;

       XtVaSetValues (base->overview_item, XmNsensitive, True, NULL);
       XtVaGetValues (base->pgfwd_button, XmNsensitive, &sensitive, NULL);
       
       if ((current_image->pages > current_state->current_page) &&
	   (sensitive == False))
	 XtVaSetValues (base->pgfwd_button, XmNsensitive, True, NULL);
       }
	
    rendering_pages = FALSE;
    pages_rendered = TRUE;
    if (pg_overview != NULL && pg_overview->showing == True) {
       if (current_image->pages > 1) {
	 pg_overview->buttons = (Widget *) PgOverviewPagesCreate (pg_overview,
						       current_image->pages);
	 if (current_state->reversed == TRUE)
	   ReversePgOverviewPages (pg_overview, current_image->pages, TRUE);

	 set_pageview_pages (current_state->current_page);
	 /*
	  * Set scrollbar values.
	  */
	 if (pg_overview->nrows > 4) {
	   Dimension form_height, button_height;
	   XtVaGetValues (pg_overview->buttons[0], XmNheight, &button_height, NULL);
	   XtVaGetValues (pg_overview->pix_form, XmNheight, &form_height, NULL);
	   XtVaSetValues (pg_overview->vscroll, XmNmaximum, (form_height + 1),
			  XmNsliderSize, (int) (button_height * 4),
			  XmNincrement, (int) button_height,
			  XmNpageIncrement, (int) (button_height * 3), 
			  NULL);
	 }
	 /*
	  * Unset all buttons.
	  */
	 for (i = 0; i < current_image->pages; i++)
	   XmToggleButtonGadgetSetState (pg_overview->buttons[i], False, False);
	 /*
	  * Set current page button.
	  */
         XmProcessTraversal (pg_overview->buttons[current_state->current_page - 1], 
			     XmTRAVERSE_CURRENT);
	 XmToggleButtonGadgetSetState (pg_overview->buttons[current_state->current_page - 1], 
					True, False);
	 pg_overview->thumbnails_created = True;
       }
       else {
	 DismissDialog (pg_overview->dialog);
       }
    }
    if (pg_overview)
	_DtTurnOffHourGlass (pg_overview->shell);

}

/*
 * dps_small_status_handler - Handles status events from small_context.
 */

void
dps_small_status_handler (ctxt, code)
DPSContext ctxt;
int	   code;
{

/*
 * We only look for ZOMBIE and FROZEN events...
 */

    if (prog->verbose) 
       fprintf (stderr, catgets (prog->catd, 1, 248,
				 "%s: got ps status: %d\n"), prog->name, code);

/*
 * If zombie, then just assume we're dead and use the total # of pages
 * that we currently have.
 */

    if (code == PSZOMBIE) {
       if (small_context != (DPSContext) NULL) {
	  got_total_pages ();
          DPSDestroySpace (DPSSpaceFromContext (small_context));
          small_context = (DPSContext) NULL;
          }
       }
    else if (code == PSFROZEN) {
       got_small_page ();
       XDPSUnfreezeContext (small_context);
       }
}

/*
 * dps_small_error_handler - Error handler for small context. If we get an
 *			     error, just forget it and use the total # of
 * 			     pages that we have so far as the total.
 */

void
dps_small_error_handler (ctxt, error_code, arg1, arg2)
DPSContext		ctxt;
DPSErrorCode		error_code;
long unsigned int	arg1;
long unsigned int	arg2;
{
    got_total_pages ();
    DPSDestroySpace (DPSSpaceFromContext (small_context));
    small_context = (DPSContext) NULL;
}

/*
 * dps_small_text_handler - handles text from render_small_pixmap
 */

void
dps_small_text_handler (ctxt, buf, count)
DPSContext  ctxt;
char	   *buf;
int	    count;
{

/*
 * Only string we care about is the PAGES_STRING, which means we're
 * done.
 */

    if (strncmp (buf, PAGES_STRING, strlen (PAGES_STRING)) ==  0) {
       got_total_pages ();
       DPSDestroySpace (DPSSpaceFromContext (small_context));
       small_context = (DPSContext) NULL;
       }
}

/*
 * pagecounter_done - got some exit status from pagecounter.
 */

void
pagecounter_done (pid, status, rusage)
int		 pid;
int		*status;
void		*rusage;
{
    extern void render_small_pages ();

    if (prog->verbose)
       fprintf (stderr, "got some status from sdtpagecounter\n");

    if (WIFEXITED (*status)) {
/*
 * Kludge - because WEXITSTATUS only looks at
 * two lower bytes.
 */
       if (npages <= 255)
         npages = WEXITSTATUS (*status); 
       if (npages == 255 || npages == 0)
          render_small_pages (current_image, TRUE, TRUE);
       else 
	  got_total_pages ();
       }

/*
 * If we got WIFSTOPPED, then kill the process.
 */

    else if (WIFSTOPPED (*status)) {
       int child_killed;
       kill (child_pid, SIGKILL);
       child_killed = wait ((int *) NULL);
       return;
       }

/*
 * Else we got WIFSIGNALED, which means that the process exited.
 */

    else 
       render_small_pages (current_image, TRUE, TRUE);

    child_pid = 0;
    if (input_id != 0) {
      XtRemoveInput (input_id); 
      input_id = 0;
    }
    signal (SIGPIPE, old_sigpipe_handler);
    close (pipe1 [1]);
    close (pipe2 [0]);

    return;
}


void
sigchild_handler (signal)
     int  signal;
{
     int    status;
     pid_t  pid;
/* 
 * Called child is done.
 * Do a waitpid() then call wait3 callback.
 */
     if (signal == SIGCHLD) {
       pid = waitpid (child_pid, &status, WNOHANG | WUNTRACED);
       (void)pagecounter_done (pid, &status, NULL);
     }

}

void
pump_bytes (start, file_bytes, do_seek)
int		 start;
int		 file_bytes;
int		 do_seek;
{
    static int	num;
    int		nitems;

    if (do_seek == TRUE) {
       fseek (current_file, start, 0);
       num = 0;
       }

    if (prog->dps == TRUE) {
	if (num == 0) {
	    char *dpscommand = "/setpagedevice {pop} def";

	    DPSWritePostScript(dps_context, dpscommand, strlen(dpscommand));
	}

       if (using_dsc == TRUE) {
          char     buf [2048];
          char    *result;
          nitems = file_bytes - start;
          while (num < nitems) {
             result = fgets (buf, 2048, current_file);
             if ((dps_context == (DPSContext) NULL) || 
	         (result == (char *) NULL))
                return;
	
	     if ((int)(num + strlen (buf)) > nitems)
	        buf [nitems - num] = '\0';
             DPSWritePostScript (dps_context, buf, strlen (buf));
             num += strlen (buf);
	     }
	  }
       else {
          char     buf [4096];
	  int 	   tmpnum;
          nitems = file_bytes - start;

          while (num < nitems) {
	     tmpnum = fread (buf, 1, 4096, current_file);
 
/*
 * DPSWritePostScript might flush the buffer, which means that we
 * might get an error, so we should check to see if the context is
 * NULL before we do the write.
 */ 
 
	     if ((dps_context == (DPSContext) NULL) ||
    		 (tmpnum == 0))
		return;

	     if ((num + tmpnum) > nitems)
		buf [nitems - num] = '\0';
	     DPSWritePostScript (dps_context, buf, tmpnum);
	     num += tmpnum;
/* NOTE  replace with notify_dispatch */
	     if (XDPSGetContextStatus (dps_context) == PSFROZEN) 
 	        break;
	     }
          }
       }
}

void
create_small_gc ()
{
    if (small_pixmap == (Pixmap) NULL) { 
      int depth;
      
      XtVaGetValues (base->file_button, XmNdepth, &depth, NULL);
      
      small_pixmap = create_pixmap (ps_display, 
				    (int) (ps_display->pagewidth * 6.0),
				    (int) (ps_display->pageheight * 6.0),
				    depth, FALSE);
    }
    
    if (small_gc == (GC) NULL) {
      XGCValues	gc_vals;
      unsigned long gc_mask;
      Pixel         fg, bg;
      XtVaGetValues (base->file_button, XmNforeground, &fg,
		                        XmNbackground, &bg, NULL);
      gc_mask = GCFunction | GCForeground | GCBackground;
      gc_vals.function = GXcopy;
      gc_vals.foreground = fg;
      gc_vals.background = bg;
      
      small_gc = XCreateGC (ps_display->xdisplay, small_pixmap, gc_mask, 
			    &gc_vals);
    }
}    

void
dps_event_func (event)
XEvent		*event;
{
    XClientMessageEvent	*client_event = (XClientMessageEvent *) event;
    extern void render_small_pages ();

    if (client_event->data.l[0] == GET_TOTAL_PAGES) {

/*
 * If we got a timeout, then don't bother forking the process.
 * Just set the appropriate variables and return.
 * Also, if the # of pages is 1 (say for instance if using_Dsc is true
 * and there was only one %%Page comment), then don't bother forking
 * the process either.
 * Also, if it's a EPSF then don't fork either.
 *
 * (DKenny - 27 May, 1997)
 *
 * Actually, if it's an EPSF file, the there is the possiblity that it's
 * after being generated by some non-standard MS-Windows app, as these seem to
 * write most PS files as EPSF files - I've no idea why, but they do...
 * As a result if we don't count the pages for all EPSF files, then we could
 * end up ignoring any pages after page one!!
 *
 */

       if ((current_state->timeout_hit == TRUE) ||
	   (current_image->pages == 1)) {
          got_total_pages ();
	  return;
	  }

       if (forked_app == (char *) NULL) {
          char tmp_app [MAXPATHLEN];
	  char *ow;

          ow = getenv ("OPENWINHOME");
          if (ow == (char *) NULL)
	     ow = "/usr/openwin";

/*
 * malloc is +6 since we add "/bin/" and a NULL when sprintf-ing
 */
/*
	  forked_app = malloc (strlen ("/home/nakata/CDE-IMAGETOOL/cdesrc/sun1/imagetool/imagetool/pagecounter-g") + strlen (PAGECOUNTER) + 2);
          sprintf (forked_app, "/home/nakata/CDE-IMAGETOOL/cdesrc/sun1/imagetool/imagetool/pagecounter-g");
*/
	  forked_app = malloc (strlen ("/usr/dt/bin/") + strlen (PAGECOUNTER) + 2);
          sprintf (forked_app, "/usr/dt/bin/%s", PAGECOUNTER);
	  }

          create_small_gc ();

/*
 * Create two pipes for communication.
 */
       if (pipe (pipe1) == 0) {
	  if (pipe (pipe2) == 0) { 


	     child_pid = vfork ();
	     if (child_pid == 0) {
		struct rlimit rlim;
		char tmp_pixmap [40];
		char tmp_height [80];
		char tmp_width [80];
		char tmp_graybuf1 [20];
		char tmp_graybuf2 [20];
		char tmp_graybuf3 [20];
		char tmp_graybuf4 [20];
		char tmp_graybuf5 [20];
		char tmp_graybuf6 [20];
		char tmp_graybuf7 [20];
		char tmp_graybuf8 [20];
		char tmp_graybuf9 [20];
		char tmp_graybuf10 [20];
		char tmp_rgbbuf1 [20];
		char tmp_rgbbuf2 [20];
		char tmp_rgbbuf3 [20];
		char tmp_rgbbuf4 [20];
		char tmp_rgbbuf5 [20];
		char tmp_rgbbuf6 [20];
		char tmp_rgbbuf7 [20];
		char tmp_rgbbuf8 [20];
		char tmp_rgbbuf9 [20];
		char tmp_rgbbuf10 [20];
		char tmp_fg [20];
		char tmp_bg [20];
		char tmp_check_color [20];
		int i, status;
		Pixel  fg, bg;
		XStandardColormap *rcmap, *gcmap;

		dup2 (pipe1 [0], 0);
		dup2 (pipe2 [1], 1);
		getrlimit (RLIMIT_NOFILE, &rlim);
		for (i = rlim.rlim_cur; i > 2; i--)
		    close (i);

		sprintf (tmp_pixmap, "%u", small_pixmap);
		sprintf (tmp_width, "%f", ps_display->pagewidth);
		sprintf (tmp_height, "%f", ps_display->pageheight);

	        if (small_gray_cmap != NULL)
		  gcmap = small_gray_cmap;
   		else 
	 	  gcmap = gray_cmap;

	        if (small_rgb_cmap != NULL) 
		  rcmap = small_rgb_cmap;
		else 
		  rcmap = rgb_cmap;

		sprintf (tmp_graybuf1, "%u", gcmap->colormap);
		sprintf (tmp_graybuf2, "%u", gcmap->red_max);
		sprintf (tmp_graybuf3, "%u", gcmap->red_mult);
		sprintf (tmp_graybuf4, "%u", gcmap->green_max);
		sprintf (tmp_graybuf5, "%u", gcmap->green_mult);
		sprintf (tmp_graybuf6, "%u", gcmap->blue_max);
		sprintf (tmp_graybuf7, "%u", gcmap->blue_mult);
		sprintf (tmp_graybuf8, "%u", gcmap->base_pixel);
		sprintf (tmp_graybuf9, "%u", gcmap->visualid);
		sprintf (tmp_graybuf10, "%u", gcmap->killid);

		sprintf (tmp_rgbbuf1, "%u", rcmap->colormap);
		sprintf (tmp_rgbbuf2, "%u", rcmap->red_max);
		sprintf (tmp_rgbbuf3, "%u", rcmap->red_mult);
		sprintf (tmp_rgbbuf4, "%u", rcmap->green_max);
		sprintf (tmp_rgbbuf5, "%u", rcmap->green_mult);
		sprintf (tmp_rgbbuf6, "%u", rcmap->blue_max);
		sprintf (tmp_rgbbuf7, "%u", rcmap->blue_mult);
		sprintf (tmp_rgbbuf8, "%u", rcmap->base_pixel);
		sprintf (tmp_rgbbuf9, "%u", rcmap->visualid);
		sprintf (tmp_rgbbuf10, "%u", rcmap->killid);

		XtVaGetValues (base->file_button, XmNforeground, &fg,
			                          XmNbackground, &bg, NULL);
		sprintf (tmp_fg, "%u", fg);
		sprintf (tmp_bg, "%u", bg);
		sprintf (tmp_check_color, "%d", prog->check_for_color);

		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe2[1]);

		status = execl (forked_app, forked_app, current_ps_filename, 
				tmp_pixmap,
				DisplayString (ps_display->xdisplay),
				tmp_width, tmp_height,
				tmp_graybuf1, tmp_graybuf2, tmp_graybuf3,
				tmp_graybuf4, tmp_graybuf5, tmp_graybuf6,
				tmp_graybuf7, tmp_graybuf8, tmp_graybuf9, 
				tmp_graybuf10,
				tmp_rgbbuf1, tmp_rgbbuf2, tmp_rgbbuf3, 
				tmp_rgbbuf4, tmp_rgbbuf5, tmp_rgbbuf6, 
				tmp_rgbbuf7, tmp_rgbbuf8, tmp_rgbbuf9, 
				tmp_rgbbuf10,
				tmp_fg, tmp_bg, 
				tmp_check_color,
				NULL);

		_exit (255);
		}

/*
 * Parent process..
 */

	     else {
		close (pipe1 [0]);
		close (pipe2 [1]);

		npages = 0;
		pipe_fd = fdopen (pipe1 [1], "w");
		input_id = XtAppAddInput (base->app_context, pipe2[0], 
					  (XtPointer)XtInputReadMask,
					  ReadFromPagecounter, NULL);
		signal (SIGCHLD, sigchild_handler);
		old_sigpipe_handler = signal (SIGPIPE, sigpipe_handler);
		return;
		}
	     }
          }
       render_small_pages (current_image, TRUE, TRUE);
       }
    else if (client_event->data.l[0] == GET_NEXT_PAGE) {
       rendering = TRUE;
       XDPSUnfreezeContext (dps_context);
       pump_bytes (0, current_image->file_size, FALSE);
       }

/*
 * Note that the NO_OP event is just so if we get the timeout,
 * we can send ourselves an event and get it so that we
 * exit the wait_for_dps_event function. That is handled here in
 * the default case since we don't really want to do anything
 * anyway.
 */

    else {
       if (prog->verbose == TRUE)
	  fprintf (stderr, "got an unknown client event: %d\n", 
			    client_event->data.l[0]);
    }

}

int
open_psfile (image)
ImageInfo	*image;
{
    char	 buf [MAXPATHLEN];
    char	 psid[MAX_MAGIC+1];
    char        *result;

    if (image->data == (char *) NULL) {
       if (image->compression == NO_COMPRESS) 
	  current_ps_filename = image->realfile;
       else 
          current_ps_filename = (char *) file_to_open (image->file, 
					  image->realfile, image->compression, 
					  image->data, image->file_size);
       }

/*
 * So we have data ... just write it out (file_to_open will uncompress
 * the file if need be). 
 */

    else 
       current_ps_filename = (char *) file_to_open (image->file, 
					  image->realfile, image->compression, 
					  image->data, image->file_size);

/*
 * Get new file size.
 */
    if ( image->compression != NO_COMPRESS )
	image->file_size = check_open_file (current_ps_filename);

    current_file = fopen (current_ps_filename, "r");
    if (current_file == (FILE *) NULL)
       return (-1);
 
    result = fgets( psid, MAX_MAGIC, current_file );
    if ( result == NULL )
        return(-1);

    psid[MAX_MAGIC]='\0'; /* Just in case we overrun */

    /* Check the first two char to determin whether this is a PS file */ 
    if (strncmp(psid, "%!", 2)) { 
	return(-1); 
    }  

    /* Determine if the file is actually an EPS file, or just a plain ol'
     * PS file.
     */

    /* If the first line contains the string EPSF, the it's and EPSF file */
    if ( strstr( psid, "EPSF-" ) != NULL ) 
        image->type_info = &all_types [EPSF];

    fseek (current_file, (off_t)0, (off_t)0); /* return to start of file */

    return (0);
}

void
init_ps_vars ()
{
    dps_context = (DPSContext) NULL;
}

void
wait_for_dps_event (which_event)
int	*which_event;
{
    int 	 nevents = 0;
    int		 i;
    XEvent	 xevent;
    XEvent     **old_events;

    while (*which_event == TRUE) {
	XNextEvent (ps_display->xdisplay, &xevent);
	if (XDPSIsDPSEvent (&xevent)) {
	    XDPSDispatchEvent (&xevent);
	} else {
          nevents++;
          if (nevents == 1)
	     old_events = (XEvent **) malloc (sizeof (XEvent *));
          else
	     old_events = (XEvent **) realloc (old_events, sizeof (XEvent *) *
							   nevents);
          old_events [nevents - 1] = (XEvent *) malloc (sizeof (XEvent));
          memcpy (old_events [nevents - 1], &xevent, sizeof (XEvent));
          }
       if (dps_context == (DPSContext) NULL)
	  *which_event = FALSE;
       }

    if (nevents != 0) {
       for (i = (nevents - 1); i > -1 ; i--) {
	   XPutBackEvent (ps_display->xdisplay, old_events [i]);
	   free (old_events [i]);
	   }
       free (old_events);
       }
}

void
check_pagecounter ()
{

/*
 * If rendering_pages == TRUE, then we're rendering the thumbnails and
 * determining total number of pages. If child_pid is not zero, then
 * the process didn't finish, so let's kill it now. If not, then
 * if small_context is not null, destroy the context.
 */

    if (rendering_pages == TRUE) {
       if (child_pid != 0) {
          int child_killed;
          kill (child_pid, SIGKILL);
          child_killed = wait ((int *) NULL);
	  if (input_id != 0) {
 	    XtRemoveInput (input_id); 
	    input_id = 0;
	  }
	  signal (SIGPIPE, old_sigpipe_handler);
	  close (pipe1 [1]);
	  close (pipe2 [0]);
          child_pid = 0;
          }
       else if (small_context != (DPSContext) NULL) {
          DPSDestroySpace (DPSSpaceFromContext (small_context));
          small_context = (DPSContext) NULL;
          }
       }
}

void
close_ps (wait_for_reply, exiting)
int	wait_for_reply;
int	exiting;
{

    if (current_file != (FILE *) NULL) {
       fclose (current_file);
       current_file = (FILE *) NULL;
       }

    if (dps_context != (DPSContext) NULL) {
       if (exiting == TRUE)
	  check_pagecounter ();

       waiting_to_exit = TRUE;
       if ((rendering == TRUE) ||
           (XDPSGetContextStatus (dps_context) == PSFROZEN)) {
	  DPSInterruptContext (dps_context);
          DPSWritePostScript (dps_context, END_OF_FILE, strlen (END_OF_FILE));
	  }
       waiting_to_exit = FALSE;
       DPSDestroySpace (DPSSpaceFromContext (dps_context));
       dps_context = (DPSContext) NULL;
       }
}

int
reopen_ps ()
{
    if (prog->dps == TRUE) {
       if (dps_context != (DPSContext) NULL) {
	  waiting_to_exit = TRUE;
	  if ((rendering == TRUE) || 
	      (XDPSGetContextStatus (dps_context) == PSFROZEN)) {
	     DPSInterruptContext (dps_context);
             DPSWritePostScript (dps_context, END_OF_FILE, 
				 strlen (END_OF_FILE));
	     }
	  waiting_to_exit = FALSE;
          DPSDestroySpace (DPSSpaceFromContext (dps_context));
	  dps_context = (DPSContext) NULL;
	  }
       }
    else {
      if (prog->verbose)
	fprintf (stderr, catgets (prog->catd, 1, 250,
		    "%s couldn't connect to DPS server.\n"), 
		    prog->name);
      AlertPrompt (base->top_level,
		   DIALOG_TYPE,  XmDIALOG_ERROR,
                   DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
		   DIALOG_TITLE, catgets (prog->catd, 1, 398, "Image Viewer - Open"),
		   DIALOG_TEXT, catgets (prog->catd, 1, 251,
					 "Display PostScript (DPS) interpreter not found.\nCannot open PostScript file."),
		   BUTTON_IDENT, 0, catgets (prog->catd, 1, 252, "Continue"),
		   HELP_IDENT, HELP_OPEN_ITEM,
		   NULL);
      return (-2);
    }
    return (0);
}

void
make_new_pixmaps (zoom)
    float 	zoom;
{
    
/*
 * Since we're creating new pixmaps, set pixmaps_switched to FALSE.
 */

    pixmaps_switched = FALSE;
    if (ps_display->pixmap1 != (Pixmap) NULL)
       XFreePixmap (ps_display->xdisplay, ps_display->pixmap1);
    if (ps_display->pixmap2 != (Pixmap) NULL)
       XFreePixmap (ps_display->xdisplay, ps_display->pixmap2);

    if (prog->ps_mono == True)
      ps_display->pixmap1 = create_pixmap (ps_display, ps_display->pix_width,
                                         ps_display->pix_height,
                                         1, TRUE);
    else
      ps_display->pixmap1 = create_pixmap (ps_display, ps_display->pix_width,
                                         ps_display->pix_height,
                                         ps_display->depth, TRUE);

/*
 * Create a mono gc only once and after after the 1-bit pixmap was created 
 * and only if monochrome resource is set.
 */
    if (prog->ps_mono && ps_display->mono_gc == NULL) {

        XGCValues     gc_vals;
        unsigned long gc_mask;

	gc_mask = GCFunction;
	gc_vals.function = GXcopyInverted;
	ps_display->mono_gc = XCreateGC (ps_display->xdisplay, 
					 ps_display->pixmap1, gc_mask, &gc_vals);
    }

/*
 * If using dsc, then only create one pixmap, since we never use
 * the second one.
 */
    if (using_dsc == FALSE) {
      if (prog->ps_mono == True) {
        ps_display->pixmap2 = create_pixmap (ps_display, ps_display->pix_width,
                                           ps_display->pix_height, 
                                           1, FALSE);
        ps_display->pixmap3 = create_pixmap (ps_display, 1, 1, 1, FALSE);
      }
      else {
        ps_display->pixmap2 = create_pixmap (ps_display, ps_display->pix_width,
                                           ps_display->pix_height, 
                                           ps_display->depth, FALSE);
        ps_display->pixmap3 = ps_display->win;
      }
    }
    else {
       ps_display->pixmap2 = (Pixmap) NULL;
       ps_display->pixmap3 = (Pixmap) NULL;
    }

    XSync (ps_display->xdisplay, 0);
}

int
MakePaper ()
{
    float	  zoom;
    int		  tmp;
    int		  dpi;
    unsigned long enable_mask, 
		  disable_mask,
	   	  next_mask;
    GC            gc;
    
    zoom = ps_display->res_x / 72.0;  /* res_x or y */
    ps_display->pix_width = ps_display->pagewidth * (int) ps_display->res_x;
    ps_display->pix_height = ps_display->pageheight * (int) ps_display->res_y;

    ctm_a = zoom;
    ctm_b = ctm_c = 0.0;
    ctm_d = -zoom;
    ctm_tx = 0;
    ctm_ty = ps_display->pix_height;

    make_new_pixmaps (1.0);

/*
 * If dps, if dps_context is NULL (and it should be), create the
 * context. Use the canvas xid since this doesn't change, whereas
 * the pixmaps do change.
 */

    if (prog->dps == TRUE) {
       if (dps_context == (DPSContext) NULL) {

          if (prog->ps_mono == True)
            gc = ps_display->mono_gc;
          else
            gc = ps_display->win_gc;
 
	  if (using_dsc == TRUE) 
             dps_context = XDPSCreateContext (ps_display->xdisplay,
				ps_display->pixmap1, gc, 0, 0, 
				0, gray_cmap, rgb_cmap, 0,
				(DPSTextProc) dps_text_handler, dps_error_handler, NULL);
	  else
             dps_context = XDPSCreateContext (ps_display->xdisplay,
				ps_display->pixmap3, gc, 0, 0,
				0, gray_cmap, rgb_cmap, 0,
			        (DPSTextProc) dps_text_handler, dps_error_handler, NULL);
	  }
       if (dps_context == (DPSContext) NULL) {
	  if (prog->verbose)
             fprintf (stderr, catgets (prog->catd, 1, 253,
		      "%s couldn't connect to DPS server.\n"), prog->name); 
	  return (-2);
	  }

       enable_mask = PSZOMBIEMASK | PSFROZENMASK ;
       disable_mask = PSRUNNINGMASK | PSNEEDSINPUTMASK;
       next_mask = 0;
       DPSSetContext (dps_context);
       XDPSSetStatusMask (dps_context, enable_mask, disable_mask, next_mask);
       XDPSRegisterStatusProc (dps_context, dps_status_handler);
       }
    return (0);
}

void
copy_pixmaps (pix1, pix2)
Pixmap	pix1;
Pixmap	pix2;
{
   if (ps_display->depth == 1) 
      XCopyPlane (ps_display->xdisplay, pix2, pix1, ps_display->win_gc, 0, 0, 
		  ps_display->pix_width, ps_display->pix_height, 0, 0, 1L);
    else if (prog->ps_mono == True)
      XCopyPlane (ps_display->xdisplay, pix2, pix1, ps_display->mono_gc, 0, 0,
                  ps_display->pix_width, ps_display->pix_height, 0, 0, 1L);
   else
      XCopyArea (ps_display->xdisplay, pix2, pix1, ps_display->win_gc, 0, 0,
		 ps_display->pix_width, ps_display->pix_height, 0, 0);
   XSync (ps_display->xdisplay, 0);
}

/*
 * dps timeout function - just set rendering to FALSE,
 */
  
void
dps_timeout_func (sig)
int sig;
{
    rendering = FALSE;
    timeout_hit = TRUE;
    page_one_error = 0;
    do_dps_op (NO_OP);
}

void
fast_dsc_page (page_to_get)
int	page_to_get;
{
    if (dps_context == NULL) {
      int   tmp_width, tmp_height;
      float zoom = (float) prog->def_ps_zoom / 100.0;
      extern void make_new_dsc_context();

      make_new_dsc_context ();

      tmp_width = ps_display->pagewidth * (int) ps_display->res_x;
      tmp_height = ps_display->pageheight * (int) ps_display->res_y;

      dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
	         tmp_width, tmp_height, (int) ps_display->res_x, FALSE, FALSE);
      if (zoom != 1.0) 
        dps_zoom (zoom, ps_display->pix_height);
      dps_dsc_paging ();
      if (end_prolog > 0) {
	pump_bytes (0, end_prolog, TRUE);
      }
      if ((begin_setup > 0) && (end_setup > 0))
	pump_bytes (begin_setup, end_setup, TRUE);
    }
    else
      XDPSUnfreezeContext (dps_context);
    dsc_page_error = FALSE;
    dps_clearcanvas ();
    if (current_state->reversed == TRUE)
       pump_bytes (pagestart [current_image->pages - page_to_get],
		   pagestart [current_image->pages - page_to_get + 1], TRUE);
    else
       pump_bytes (pagestart [page_to_get - 1], pagestart [page_to_get], TRUE);
    if (dps_context == (DPSContext) NULL)
       return;
    DPSclientsync (dps_context);
    rendering = TRUE;
    setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
    signal (SIGALRM, dps_timeout_func);
    timeout_hit = FALSE;
    wait_for_dps_event (&rendering);
    signal (SIGALRM, SIG_IGN);
    update_page ();
    current_state->current_page = page_to_get;
    if ((dsc_page_error == TRUE) || (timeout_hit == TRUE))
       page_errors [page_to_get - 1] = TRUE;
}

void
fast_reversed_page (page_to_get, force, interrupt_done)
int	page_to_get;
int	force;
int 	interrupt_done;
{
    int		skip_pages;
    Pixmap 	tmp_pixmap;

    if (rendering == TRUE) 
       wait_for_dps_event (&rendering);

    if ((current_state->next_page == page_to_get) && (force == FALSE)) {
       int	 tmp_page;
       tmp_pixmap = ps_display->pixmap1;
       ps_display->pixmap1 = ps_display->pixmap2;
       ps_display->pixmap2 = tmp_pixmap;
       update_page ();
       tmp_page = current_state->next_page;
       pixmaps_switched = !pixmaps_switched;
       current_state->next_page = current_state->current_page;
       current_state->current_page = tmp_page;
       return;
       }

/*
 * If page_to_get is less than the current page, then we can continue
 * through the file...
 */

    if (page_to_get < current_state->current_page) {
       if (current_state->next_page != 0) {
	  if (current_state->next_page < current_state->current_page)
             skip_pages = current_state->next_page - page_to_get;
	  else
             skip_pages = current_state->current_page - page_to_get;
	  }
       else
          skip_pages = current_state->current_page - page_to_get;

/*
 * If skip_pages is 1, then we already have the `next' page. So,
 * make sure that this next page is copied onto pixmap1 before continuing
 * since the new page will be rendered onto pixmap2. The `next' page is
 * on pixmap2 if current_state->next_page == page_to_get + 1;
 * If current_state->current_page = page_to_get + 1, then it's already
 * on pixmap1.
 */

       if (skip_pages == 1) {
	  if (current_state->next_page == (page_to_get + 1)) { 
	     if (pixmaps_switched == FALSE) 
                copy_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
	     current_state->next_page = page_to_get + 1;
	     }
	  else if (current_state->current_page == (page_to_get + 1)) {
	     if (pixmaps_switched == TRUE) 
		copy_pixmaps (ps_display->pixmap2, ps_display->pixmap1);
	     current_state->next_page = page_to_get + 1; 
	     }
	  } 
       XDPSUnfreezeContext (dps_context);
       pump_bytes (0, current_image->file_size, FALSE);
       }

    else {  /* Page is before current one or is current one */

/*
 * If only one page, then we're doing some sort of zoom/flip/rotate.
 * Unfreeze the context so the procedure will end (since there are no
 * more pages to render) and then continue... Note that if the context
 * isn't frozen, this has absolutely no affect.
 */

/*
 * If interrupt_done == TRUE, then we have already interrupted the
 * context so we don't need to do it again.
 */


       if (interrupt_done == FALSE) {
          DPSInterruptContext (dps_context);
          DPSWritePostScript (dps_context, END_OF_FILE, strlen (END_OF_FILE));
	  }
       dps_check_stacks ();
       dps_do_restore ();
       dps_fast_paging ();

/*
 * If current_image->pages < 0, then we haven't even determined the total
 * number of pages in this document. So, set user object 9 to 1 (to denote
 * that we're going to look for all pages, and don't set the page
 * requested, and continue.
 */

       skip_pages = 1;
       if (current_image->pages < 0) { 
	  reversed_total_pages = 1;
	  dps_set_all_pages (1);
	  started_newfile = TRUE;
	  }
       else if (page_to_get == current_image->pages) {
          dps_page_requested (1);
	  current_state->next_page = 0;
	  }
       else {
          dps_page_requested (current_image->pages - page_to_get);
          skip_pages = 2;
	  }
       pump_bytes (0, current_image->file_size, TRUE);
       }

/*
 *
 * Before we do any copying.. check if pixmaps_switched is true,
 * and if so, switch the pixmaps back since we aren't switching
 * them in dps anymore.
 */

    if (pixmaps_switched == TRUE) {
       tmp_pixmap = ps_display->pixmap1;
       ps_display->pixmap1 = ps_display->pixmap2;
       ps_display->pixmap2 = tmp_pixmap;
       }

    if (current_image->pages < 0) {
       dps_stop_rendering ();
       DPSstop (dps_context);
       }

/*
 * Wait for page to be rendered.
 */

    while (skip_pages > 0) {
       rendering = TRUE;
       signal (SIGALRM, dps_timeout_func);
       setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
       timeout_hit = FALSE;
       wait_for_dps_event (&rendering);
       signal (SIGALRM, SIG_IGN);
       if (timeout_hit == TRUE) 
	  skip_pages = 1;
       skip_pages--;

/* 
 * If skip_pages == 1, then we just got the page before the one
 * we wanted... which is really page n + 1, since we're in reverse
 * page order. So, copy that one to pixmap 1, and set next_page
 * appropriately.
 */

       if (skip_pages == 1) {
	  copy_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
	  current_state->next_page = page_to_get + 1;
	  }

       if (skip_pages > 0) {
          XDPSUnfreezeContext (dps_context);
          pump_bytes (0, current_image->file_size, FALSE);
   	  }
       }

/*
 * We now have the `right' page on pixmap2, so switch pixmaps 1 & 2
 */

    if (current_image->pages > 0) {
       tmp_pixmap = ps_display->pixmap1;
       ps_display->pixmap1 = ps_display->pixmap2;
       ps_display->pixmap2 = tmp_pixmap;
       pixmaps_switched = TRUE;

       update_page ();
       current_state->current_page = page_to_get;
       }

/*
 * If # of pages < 0, then we didn't have total pages. Now we do.
 * Do what we need to do (set # pages), turn on/off various
 * stuff, etc.
 */

    else {
       if (timeout_hit == FALSE) {
          XDPSUnfreezeContext (dps_context);
          dps_check_stacks ();
          dps_do_restore ();
          dps_fast_paging ();
          DPSshowpage (dps_context);
          current_image->pages = reversed_total_pages;
          started_newfile = FALSE;
          free (page_errors);
          page_errors = (short *) calloc (1, sizeof (short) * 
					  current_image->pages);
          if ((page_one_error != -1) || (timeout_hit == TRUE))
             page_errors [current_image->pages - page_one_error - 1] = TRUE;
	  current_state->current_page = 1;
	  current_state->next_page = 0;
 	  if (page_to_get != 1)
	     fast_reversed_page (page_to_get, FALSE, TRUE);
          }
       update_page ();
       }
      
}

void
fast_forward_page (page_to_get, force, interrupt_done)
int	page_to_get;
int	force;
int 	interrupt_done;
{
    int		skip_pages;

    timeout_hit = FALSE;

    if (rendering == TRUE) {
       setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
       signal (SIGALRM, dps_timeout_func);
       timeout_hit = FALSE;
       wait_for_dps_event (&rendering);
       signal (SIGALRM, SIG_IGN);

/*
 * If we hit the timeout, then we'll never get past this page.
 */

       if (timeout_hit == TRUE) {
          if (page_requested != 0) 
             current_state->next_page = page_requested;
          page_requested = 0;
	  page_errors [page_requested - 1] = TRUE;
	  }
       }
 
    if ((current_state->next_page == page_to_get) && (force == FALSE)) {
       if ((page_to_get == current_image->pages) ||
	   (current_state->current_page == current_image->pages)) {
	  Pixmap tmp_pixmap;
  	  int	 tmp_page;
          tmp_pixmap = ps_display->pixmap1;
          ps_display->pixmap1 = ps_display->pixmap2;
          ps_display->pixmap2 = tmp_pixmap;
	  update_page ();
          pixmaps_switched = !pixmaps_switched;
          tmp_page = current_state->next_page;
          current_state->next_page = current_state->current_page;
          current_state->current_page = tmp_page;
	  }
       else {
	  copy_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
          update_page ();
          current_state->current_page = current_state->next_page;
	  current_state->next_page = 0;
	  page_requested = current_state->current_page + 1;
	  do_dps_op (GET_NEXT_PAGE);
          }
       return;
       }

/*
 * Also check if pixmaps switched is true (could be.. if doc was
 * in last page first order and then switched to first page first).
 * If so, switch them back now.
 */

    if (pixmaps_switched == TRUE) {
       Pixmap tmp_pixmap;
       pixmaps_switched = FALSE;
       tmp_pixmap = ps_display->pixmap1;
       ps_display->pixmap1 = ps_display->pixmap2;
       ps_display->pixmap2 = tmp_pixmap;
       }

    if (page_to_get > current_state->current_page) {
	XDPSUnfreezeContext (dps_context);
	pump_bytes (0, current_image->file_size, FALSE);

	if (current_state->next_page != 0)
	    skip_pages = page_to_get - current_state->next_page;
	else
	    skip_pages = page_to_get - current_state->current_page;

    } else {  /* Page is before current one or is current one */
	/*
	 * If interrupt_done == TRUE, then we have already interrupted the
	 * context so we don't need to do it again.
	 */
	if (interrupt_done == FALSE) {
	    DPSInterruptContext (dps_context);
	    DPSWritePostScript (dps_context, END_OF_FILE, strlen (END_OF_FILE));
	}
	dps_check_stacks ();
	dps_do_restore ();
	dps_fast_paging ();
	dps_page_requested (page_to_get);

	/* Added this line to fix the reverse page problem
	 * Without this line imagetool may hang till timeout
	 * when going reverse
	 */
	XDPSUnfreezeContext (dps_context);

	pump_bytes (0, current_image->file_size, TRUE);
	skip_pages = 1;
    }

/*
 * Wait for page to be rendered.
 */

    while (skip_pages > 0) {
       rendering = TRUE;
       signal (SIGALRM, dps_timeout_func);
       setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
       timeout_hit = FALSE;
       wait_for_dps_event (&rendering);
       if (timeout_hit == TRUE)
	  page_errors [page_to_get - 1] = TRUE;
       signal (SIGALRM, SIG_IGN);
       skip_pages--;
       if (skip_pages > 0) {
          XDPSUnfreezeContext (dps_context);
          pump_bytes (0, current_image->file_size, FALSE);
	  }
       }

    copy_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
    update_page ();
    current_state->current_page = page_to_get;
    current_state->next_page = 0;

/*
 * If we got a timeout error, then don't even bother to render the
 * next page since it won't work.
 */

    if ((page_to_get != current_image->pages) && (timeout_hit == FALSE) &&
	(current_image->pages > 0)) {
       page_requested = current_state->current_page + 1;
       do_dps_op (GET_NEXT_PAGE);
       }
}

void
goto_page (page_to_get, force, interrupt_done)
int         page_to_get;
int	    force;
int	    interrupt_done;
{
    int		done;
    int		tag;
    int		output [MAXOUTPUT + 1];
    int		pageno;
    Pixmap	tmp_pixmap;
    int		tmp_page;
    int		no_input = 0;
    int		page_number;
    int		old_page = current_state->current_page;
    int		real_page_to_get = page_to_get;

    if (prog->dps == TRUE) {
	if (current_state->using_dsc == TRUE) {
	    fast_dsc_page (page_to_get);
	} else {
	    if (current_state->reversed == TRUE)
		fast_reversed_page (page_to_get, force, interrupt_done);
	    else
		fast_forward_page (page_to_get, force, interrupt_done);
	}
    } 
 
    update_page_number (page_to_get, current_image->pages, old_page);
    if (page_errors [page_to_get - 1] == TRUE)
      SetFooter (base->left_footer, catgets (prog->catd, 1, 259,
	         "A PostScript error was found on this page."));
    else if (current_image->pages > 0) {
      SetFooter (base->left_footer, "");
      prog->footer_priority = CLEAR_MESSAGE;
    }
    else {
      if (prog->footer_priority < MONOCHROME_MESSAGE) {
        SetFooter (base->left_footer, catgets (prog->catd, 1, 260,
	           "Checking document's page count..."));
	prog->footer_priority = COUNTING_MESSAGE;
	}
      }
 

    XmProcessTraversal (ps_display->new_canvas, XmTRAVERSE_CURRENT);
}

int
check_for_reverse (image, data, file_size)
ImageInfo *image;
char	*data;
int	 file_size;
{
    mmap_handle_t       *file_fp = NULL;
    encoded_bm          *encoded_search;
    char                *page_comments;
    char                *page_comments_start;
    int			 val;
    int		   	 pages [2] = {-1, -1};
    int			 page_index = 0;
    char		*start_addr = data;
    char		*tmp_addr;
    int			 amt = file_size;
    int			 size = file_size;
    char		*data_ptr = data;
    int			 count;
    char		*tmpnum, *tmpstr;
    int			 page_length, pageorder_length;
    int			 done = FALSE;
    Boolean              page_order = False;
    Boolean              descend = True;
    char                *search_strings [10] = {"%%Page:", 
					       "%%BeginDocument",
                                               "%%EndDocument",
                                               "%%EndProlog",
                                               "%%EndSetup",
                                               "%%BeginSetup",
                                               "%%Trailer",
					       "%%PageOrder",
					       "%%Orientation",
                                               NULL};
    char		*usedsc_strings [2] = {"Due to bugs in Transcript, the 'PS-Adobe-' is omitted from line 1", NULL};

/*
 * If we have a file, then do a fast_read. Otherwise,
 * we already have the data.
 */

    if (current_ps_filename != (char *) NULL) {
       file_fp = fast_read (current_ps_filename);
       if (file_fp == (mmap_handle_t *) NULL)
	  return (-1);
       start_addr = file_fp->addr;
       amt = file_fp->len;
       data_ptr = file_fp->addr;
       size = file_fp->len;
       }

    if (current_props->use_dsc == TRUE) {
       using_dsc = TRUE;
    } else if (strncmp (start_addr, PS_ADOBE, strlen (PS_ADOBE)) == 0) {
	using_dsc = TRUE;
    } else {
	encoded_search = strbmencode(usedsc_strings);
	using_dsc = FALSE;
	tmp_addr = start_addr;
	while(page_comments = strbmexec((unsigned char *) tmp_addr, amt,
		encoded_search)) {
	    if(!strncmp(page_comments, usedsc_strings[0],
		strlen(usedsc_strings[0]))) {
		using_dsc = TRUE;
		break;
	    }
	    tmp_addr = page_comments+1;
	}
	strbmdestroy(encoded_search);
     }
   
    if (using_dsc == FALSE)
       search_strings [1] = NULL;
    else {
       in_imported_document = 0;
       end_prolog = 0;
       begin_setup = 0;
       end_setup = 0;
       trailer = 0;
       if ((number_of_pages > 0) && (pagestart != NULL)) {
          free (pagestart);
          pagestart = NULL;
          }
       number_of_pages = 0;
       }  

    encoded_search = strbmencode(search_strings);
    page_length = strlen (search_strings [0]);
    pageorder_length = strlen (search_strings [7]);

    while (done == FALSE) {
       page_comments = strbmexec ((unsigned char *) start_addr, amt, 
				  encoded_search);
	if (page_comments == (char *) NULL) {
	    if (!number_of_pages) {
		/* There is no DSC comment in the entire document */
		using_dsc = FALSE;
	    }
	    break;
	}

       if (using_dsc == TRUE) {
	  if (in_imported_document != 0) {
	     if (strncmp (page_comments, "%%EndDocument", 13) == 0)
		in_imported_document--;
	     }

   	  if (strncmp (page_comments, "%%BeginDocument", 15) == 0)
	     in_imported_document++;

 	  if (in_imported_document != 0) {
             start_addr = page_comments + 1;
             amt = size - (start_addr - data_ptr);
             continue;
   	     }
 
          if (strncmp (page_comments, "%%EndProlog", 11) == 0) {
             if (end_prolog == 0)
                end_prolog = (long) (page_comments - file_fp->addr);
             }
          else if (strncmp (page_comments, "%%BeginSetup", 12) == 0) {
             if (begin_setup == 0)
                begin_setup = (long) (page_comments - file_fp->addr);
             }
          else if (strncmp (page_comments, "%%EndSetup", 10) == 0) {
             if (end_setup == 0)
                end_setup = (long) (page_comments - file_fp->addr);
             }
          else if (strncmp (page_comments, "%%Trailer", 9) == 0) {
             trailer = (long) (page_comments - file_fp->addr);
             }
	}  

/*
 * Do more if comment is "%%PageOrder"... 
 */
       if (strncmp (page_comments, "%%PageOrder", 11) == 0) {
/*
 * Check if %%PageOrder is first char on line by checking 
 * if previous char is NL or CR.
 */
	  val = (int) *(page_comments - 1);
	  if ((val != CR) && (val != NL)) {
	     start_addr = page_comments + 1;
	     amt = size - (start_addr - data_ptr);
	     continue;
	     }	
/*
 * Go past the entire %%PageOrder string.
 */
	  page_comments_start = page_comments;
          page_comments += pageorder_length;
/*
 * Search for new line or carriage return. Search at most 20 characters.
 */
          start_addr = page_comments;
          count = 0;
          while (((val = (int) start_addr [0]) != 0) && (val != CR) &&
	          (val != NL) && (count < 20)) {
	     start_addr++;
	     count++;
	     }
/*
 * If val is zero, we found the end of the file... and no string!
 */
          if (val == 0)
	     break;
/*
 * Didn't find the nl or cr.
 */

          if (count == 20) {
    	     start_addr = page_comments + 1;
             amt = size - (start_addr - data_ptr);
	     continue;
             }
/*
 * Point tmp_addr to CR/NL.
 */
          tmp_addr = start_addr;
/*
 * Since we're pointing at the CR or NL, go back one.
 */
          start_addr--;
/*
 * Copy the string into a tmp.
 */
          amt = start_addr - page_comments + 1;
          tmpnum = (char *) malloc (amt + 1);
          strncpy (tmpnum, page_comments, amt);  
/*
 * Check for Descend or Ascend value on this comment line.
 */
          start_addr = tmp_addr + 1;
          amt = size - (start_addr - data_ptr);

	  tmpstr = strstr (tmpnum, "Descend");
	  if (tmpstr != NULL) {
	    page_order = True;
	    descend = True;
	    continue;
	  }
	  tmpstr = strstr (tmpnum, "Ascend");
	  if (tmpstr != NULL) {
	    page_order = True;
	    descend = False;
	    continue;
	  }
	  free (tmpnum);

       }

/*
 * Do more if comment is "%%Page"... 
 */
	  
       else if (strncmp (page_comments, "%%Page", 6) == 0) {
	  
/*
 * Check if %%Page is first char on line by checking 
 * if previous char is NL or CR.
 */

	  val = (int) *(page_comments - 1);
	  if ((val != CR) && (val != NL)) {
	     start_addr = page_comments + 1;
	     amt = size - (start_addr - data_ptr);
	     continue;
	     }	

    
/*
 * Go past the entire %%Page string.
 */

	  page_comments_start = page_comments;
          page_comments += page_length;

/*
 * Search for new line or carriage return. Search at most 20 characters.
 */

          start_addr = page_comments;
          count = 0;
          while (((val = (int) start_addr [0]) != 0) && (val != CR) &&
	          (val != NL) && (count < 20)) {
	     start_addr++;
	     count++;
	     }
    
/*
 * If val is zero, we found the end of the file... and no number!
 */

          if (val == 0)
	     break;

/*
 * Didn't find the nl or cr.
 */

          if (count == 20) {
    	     start_addr = page_comments + 1;
             amt = size - (start_addr - data_ptr);
	     continue;
             }
       

/*
 * Since we're pointing at the CR or NL, go back one.
 */

          start_addr--;

/*
 * Working backwards, find the last integer on the line.
 * Check to make sure we're not going too far back (ie. less than
 * page_comments pointer).
 */

          while ((isdigit ((int) start_addr [0]) == 0) &&
	         (start_addr >= page_comments))
	     start_addr--;
	


/*
 * If start_addr is less than page_comments then we went back too far
 * and didn't find a digit, so forget it.
 */

          if (start_addr < page_comments) {
    	     start_addr = page_comments + 1;
             amt = size - (start_addr - data_ptr);
	     continue;
	     }

/*
 * Point tmp_addr to character following last digit.
 */

          tmp_addr = start_addr + 1;

/*
 * Set pointer to previous character
 */

          start_addr--;

/*
 * Ok, now, go back till no more digits.
 */

          while (isdigit ((int) start_addr [0]) != 0)
     	     start_addr--;

/*
 * At end, start_addr points to `previous' character (non-digit).
 * Up it by one to point to the digit.
 */

          start_addr++;

/*
 * Copy the string into a tmp.
 */

          amt = tmp_addr - start_addr + 1;
          tmpnum = (char *) malloc (amt);
          strncpy (tmpnum, start_addr, amt);  

	  if (page_index < 2)
             pages [page_index] = atoi (tmpnum);
	  free (tmpnum);
       
	  if (using_dsc == TRUE) {
	     if (number_of_pages == 0)
		pagestart = (long *) malloc (2 * sizeof (long));
	     else
		pagestart = (long *) realloc (pagestart, 
					(number_of_pages + 2) * sizeof (long));
	     
    	     pagestart [number_of_pages] = (long) (page_comments_start - 
						   file_fp->addr);

	     number_of_pages++;
	     }

          page_index++;
          start_addr = tmp_addr + 1;
          amt = size - (start_addr - data_ptr);
	  }

       /*
	* bhc's changes for Orientation
	*/
       else if(strncmp(page_comments, "%%Orientation:", 14) == 0) {
	 char *pos = page_comments+14;
	 while (isspace(*pos))
	   pos++;
	 
	 /*default is portrait */

	 if(strncmp(pos, "Landscape", 9) == 0)
	   {
	     image->orient = LANDSCAPE;
	   }

	 start_addr = page_comments+1;
	 amt = size - (start_addr - data_ptr);
       }

/*
 * Update start_addr and amt for next parse.
 */

       else {
	  start_addr = page_comments + 1;
	  amt = size - (start_addr - data_ptr);
	  }

/*
 * If not using_dsc, then quit when page_index is more than 2
 * otherwise continue parsing til end of file.
 */

       if ((using_dsc == FALSE) && (page_index > 1))
	  done = TRUE;

       }
       
/*
 * Set pagestart to beginning in case no %%Page comments were found.
 */

    if (using_dsc == TRUE) {
       if (number_of_pages == 0) {
	  number_of_pages = 1;
	  pagestart = (long *) malloc (2 * sizeof (long));
	  pagestart [0] = 0;
	  pagestart [1] = size;
	  }
       else if (trailer == 0)
	  pagestart [number_of_pages] = (long) size;
       else
	  pagestart [number_of_pages] = trailer;

/*
 * Also check values returned.
 */

/*
 * If prolog, or setup ends after first page, assume the included
 * doc had one, and ignore it.
 */

       if (end_prolog > pagestart [0])
	  end_prolog = pagestart [0];

       if ((begin_setup > pagestart [0]) || (end_setup > pagestart [0])) {
	  begin_setup = 0;
	  end_setup = 0;
	  }

       if (begin_setup > end_prolog)
	  end_prolog = begin_setup;
  
       if (end_setup > end_prolog)
	  end_setup = pagestart [0];
       else
	  end_prolog = pagestart [0];

/*
 * Also need to check if we got the BeginSetup but not the EndSetup
 */

       if ((begin_setup > 0) && (begin_setup > end_setup))
	  end_setup = pagestart [0];

       }

    strbmdestroy (encoded_search);
    if (file_fp != (mmap_handle_t *) NULL)
       fast_close (file_fp);
/*
 * If valid page_order comment found, then
 * return it's value.
 */
    if (page_order == True)
      return (descend);

/*
 * If either pages[0] or pages[1] are -1, then we didn't
 * find enough info to determine if the pages are in
 * reverse order, or there is just one page.
 */

    if ((pages [0] == -1) || (pages [1] == -1))
       return (FALSE);
    
/*
 * If both numbers were the same, assume a screw up and assume
 * first page first order.
 */

    if (pages [1] >= pages [0])
       return (FALSE);

    return (TRUE);
}

int
initial_reverse (print_msg)
int	print_msg;
{

/*
 * This is a bad hack, but, if we found an error on page 1, we
 * should display an error message in the footer. However, in
 * set_tool_options (after we've done the ps_load), we clear
 * out the left footer. So, since we're going to call this function
 * to determine if the pages are in reverse order, we'll also 
 * check to see if we should display a message in the left footer.
 * Now also check to see if we're using the `clientsync' method t
 * do paging, and if so, put up a message in the left footer telling
 * the user that we're still looking for pages.
 */

    if (print_msg == TRUE) {
       if (current_image->pages < 0) {
          if (prog->footer_priority < MONOCHROME_MESSAGE) {
	    SetFooter (base->left_footer, catgets (prog->catd, 1, 261,
	               "Checking document's page count..."));
   	    prog->footer_priority = COUNTING_MESSAGE;
	  }
       }
       else if (page_errors [0] == TRUE) 
	  SetFooter (base->left_footer, catgets (prog->catd, 1, 262,
	             "A PostScript error was found on this page."));
       }

    return (reversed_pages);
}

int
initial_dsc ()
{
    return (using_dsc);
}

int
initial_timeout ()
{
    return (timeout_hit);
}

void
render_small_pages (image)
ImageInfo	*image;
{
    int 		 i;
    int 		 done;
    int 		 tag;
    int			 pageno;
    int         	 output [MAXOUTPUT + 1];
    char		 buf [2048];
    char		*result;
    int			 start = 0;
    int			 current_file_posn;
    XStandardColormap	 gray_map;
    XStandardColormap	 rgb_map;
    unsigned long 	 enable_mask, 
		  	 disable_mask,
	   	  	 next_mask;
    int                  depth = 0;
    
/*
 * Reset npages back to zero.
 */

    npages = 0;
    small_ctm_a = ((ps_display->pagewidth * 6.0) / 8.5 ) / ps_display->res_x;
    XtVaGetValues (base->file_button, XmNdepth, &depth, NULL);

    if (prog->dps == TRUE) {
       npages = 0;

       create_small_gc ();

       gray_map.colormap = None;
       gray_map.red_max = 1;
       gray_map.red_mult = -1;
       gray_map.base_pixel = 1;
       rgb_map.colormap = None;
       rgb_map.red_max = 0;
       rgb_map.green_max = 0;
       rgb_map.blue_max = 0;
       rgb_map.red_mult = 0;
       rgb_map.green_mult = 0;
       rgb_map.blue_mult = 0;
       rgb_map.base_pixel = 0;
       
       if (depth == ps_display->depth) {
         small_context = XDPSCreateContext (ps_display->xdisplay, 
					  small_pixmap, small_gc, 0, 0, 0, 
					  gray_cmap, rgb_cmap, 0, 
					  (DPSTextProc) dps_small_text_handler, 
					  dps_small_error_handler, NULL);
       }
       else {
         small_context = XDPSCreateContext (ps_display->xdisplay, 
					  small_pixmap, small_gc, 0, 0, 0, 
					  small_gray_cmap, small_rgb_cmap, 0, 
					  (DPSTextProc) dps_small_text_handler, 
					  dps_small_error_handler, NULL);
       }

       enable_mask = PSZOMBIEMASK | PSFROZENMASK;
       disable_mask = PSRUNNINGMASK | PSNEEDSINPUTMASK;
       next_mask = 0;

       XDPSSetStatusMask (small_context, enable_mask, disable_mask, next_mask);
       XDPSRegisterStatusProc (small_context, dps_small_status_handler);

       DPSSetContext (small_context);
       dps_setup (small_ctm_a, 0.0, 0.0, -small_ctm_a, 0.0, 
		  (ps_display->pageheight * 6.0), ps_display->pix_width, 
		  ps_display->pix_height, (int) ps_display->res_x, FALSE, FALSE);

       dps_count_pages ();
       
       if (current_file)
	 current_file_posn = ftell (current_file);
       else {
#ifdef DEBUG
	 fprintf (stderr, "current_file = NULL in render_small_pages()\n");
#endif
	 return;
       }

       fseek (current_file, 0, 0);
	
       while (start < image->file_size) {
          result = fgets (buf, 2048, current_file);
 
/*
 * DPSWritePostScript might flush the buffer, which means that we
 * might get an error, so we should check to see if the context is
 * NULL before we do the write.
 */ 
 
          if ((small_context == (DPSContext) NULL) || 
	      (result == (char *) NULL))
             break;
          DPSWritePostScript (small_context, buf, strlen (buf));
          start += strlen (buf);
	  }

/*
 * If the context is NULL, then we're screwed. Set total # of pages to
 * 1, and return.
 */

       if (small_context == (DPSContext) NULL) {
	  npages = 1;
     	  got_total_pages ();
	  return;
      	  }

       dps_stop_rendering ();
       DPSstop (small_context);

/*
 * Reset file position.
 */

       fseek (current_file, current_file_posn, 0);
       DPSFlushContext (small_context);
       DPSSetContext (dps_context);
       }

}

void
make_new_dsc_context ()
{
    unsigned long	enable_mask,
			disable_mask,
			next_mask; 
    GC			gc;

    if (dps_context != NULL)
      DPSDestroySpace (DPSSpaceFromContext (dps_context));

    if (prog->ps_mono == True)
      gc = ps_display->mono_gc;
    else
      gc = ps_display->win_gc;

    dps_context = XDPSCreateContext (ps_display->xdisplay,
				ps_display->pixmap1, gc, 0, 0, 
				0, gray_cmap, rgb_cmap, 0,
				(DPSTextProc) dps_text_handler, dps_error_handler, NULL);
    enable_mask = PSZOMBIEMASK | PSFROZENMASK ;
    disable_mask = PSRUNNINGMASK | PSNEEDSINPUTMASK;
    next_mask = 0;
    DPSSetContext (dps_context);
    XDPSSetStatusMask (dps_context, enable_mask, disable_mask, next_mask);
    XDPSRegisterStatusProc (dps_context, dps_status_handler);
}

int
init_ps (image)
ImageInfo	*image;
{
    int		pageno = -1;
    float       zoom;
    int		status;

/*
 * Before we do anything, let's check and see if the pages
 * are in reverse order, that way when we figure out the
 * total number of pages, we can make sure that we really display
 * page one when we're done.
 */

    using_dsc = FALSE;
    timeout_hit = FALSE;
    /* check_for_reverse also set the using_dsc flag */
    reversed_pages = check_for_reverse (image, image->data, image->file_size);
    if (reversed_pages == -1)
       return (-1);

    status = MakePaper ();
    if (status != 0)
       return (status);

    zoom = (float) prog->def_ps_zoom / 100.0;
    ps_display->pix_width = (int) (ps_display->pagewidth * ps_display->res_x *
                            zoom);
    ps_display->pix_height = (int) (ps_display->pageheight * ps_display->res_y *
                             zoom);

    XDPSSetEventDelivery (ps_display->xdisplay, dps_event_pass_through);
#ifdef NEVER
    xv_set (XV_SERVER_FROM_WINDOW (base_window->base_window),
			SERVER_EXTENSION_PROC, extension_event_proc,
			NULL);
#endif
    if (using_dsc == TRUE) {
       int	tmp_width,
		tmp_height;

       if (zoom != 1.0) {
          make_new_pixmaps (zoom);
	  make_new_dsc_context ();
	  }

       tmp_width = ps_display->pagewidth * (int) ps_display->res_x;
       tmp_height = ps_display->pageheight * (int) ps_display->res_y;
/*
 * If dsc and only 1 page, then
 * check for color in imagetool because
 * we won't fork the pagecounter.
 */
       if (number_of_pages == 1)
         dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
	            tmp_width, tmp_height, (int) ps_display->res_x, FALSE, prog->check_for_color);
       else
         dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
	            tmp_width, tmp_height, (int) ps_display->res_x, FALSE, FALSE);

       if (zoom != 1.0) 
          dps_zoom (zoom, ps_display->pix_height);
       dps_dsc_paging ();
       if (end_prolog > 0) {
	  pump_bytes (0, end_prolog, TRUE);
	}
       if ((begin_setup > 0) && (end_setup > 0))
	  pump_bytes (begin_setup, end_setup, TRUE);
       if (reversed_pages == TRUE)
          pump_bytes (pagestart [number_of_pages - 1], 
		      pagestart [number_of_pages], TRUE);
       else
          pump_bytes (pagestart [0], pagestart [1], TRUE);

       if (dps_context == (DPSContext) NULL)
	  return (-1);
       DPSclientsync (dps_context);
       signal (SIGALRM, dps_timeout_func);
       setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
       page_one_error = -1;
       rendering = TRUE;
       timeout_hit = FALSE;
       started_newfile = TRUE;
       wait_for_dps_event (&rendering);
       signal (SIGALRM, SIG_IGN);
       image->pages = number_of_pages;
       }
    else {
       dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
	          ps_display->pix_width, ps_display->pix_height, 
	          (int) ps_display->res_x, reversed_pages, FALSE);

       dps_set_all_pages (reversed_pages);

       if (zoom != 1.0) {
          dps_zoom (zoom, ps_display->pix_height);
          make_new_pixmaps (zoom);
	  }

       dps_new_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
       dps_fast_paging ();
       dps_page_requested (1);
       rendering = TRUE;
       page_one_error = -1;
       started_newfile = TRUE;

       pump_bytes (0, image->file_size, TRUE);

	/* Added the showpage so wait_for_dps_event will receive an
	 * event when loading an EPSF file
	 */
	if (image->type_info->type == EPSF) {
	    DPSshowpage (dps_context);
	}

       if (reversed_pages == TRUE) {
	  reversed_total_pages = 0;
          dps_stop_rendering ();
          DPSstop (dps_context);
          }

       if (dps_context == (DPSContext) NULL) {
          rendering = FALSE;
          close_ps (FALSE, FALSE);
          return (-1);      
          }

       signal (SIGALRM, dps_timeout_func);
       setitimer (ITIMER_REAL, ps_timeout, (struct itimerval *) NULL);
       timeout_hit = FALSE;
       if (rendering == TRUE)
          wait_for_dps_event (&rendering);
       signal (SIGALRM, SIG_IGN);
   
/*
 * If timeout_hit is TRUE, then we hit the timeout trying to render
 * page 1, so we can set # of pages to 1, and we don't need to fork
 * the process to count pages and render the thumbnails.
 */

       if (reversed_pages == FALSE) {
	  copy_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
          if (timeout_hit == TRUE)
	     image->pages = 1;
	  else
             image->pages = -1;
          }
       else {
          image->pages = reversed_total_pages;

/*
 * Unfreeze the context so that the dps_fast_paging function ends.
 * Then start it up again, and do a showpage so that we're sitting at
 * a clientsync when the user decides to do something (go to next/random
 * page).
 */

          XDPSUnfreezeContext (dps_context);
          dps_check_stacks ();
          dps_do_restore ();
          dps_fast_paging ();
          DPSshowpage (dps_context);
          }
       }

    update_page_number (1, image->pages, 0);

    if (page_errors != (short *) NULL)
       free (page_errors);

    if ((image->pages > 0) || (using_dsc == TRUE))
       page_errors = (short *) calloc (1, sizeof (short) * image->pages);
    else
       page_errors = (short *) calloc (1, sizeof (short));
    if ((page_one_error != -1) || (timeout_hit == TRUE))
       if (reversed_pages == TRUE)
          page_errors [image->pages - page_one_error - 1] = TRUE;
       else
          page_errors [page_one_error] = TRUE;

    return (0);
}

int
ps_load (image)
ImageInfo	*image;
{
    int		 done;
    int 	 status;
    int 	 tag;
    int 	 output [MAXOUTPUT + 1];
    char	 buf [MAXPATHLEN];
    float        zoom;

/*
 * If timeout is NULL, then we've never created it.
 */

    if (ps_timeout == (struct itimerval *) NULL) {
       ps_timeout = (struct itimerval *) malloc (sizeof (struct itimerval));
       (ps_timeout->it_value).tv_sec = prog->timeout;
       (ps_timeout->it_value).tv_usec = 0;
       (ps_timeout->it_interval).tv_sec = 0;
       (ps_timeout->it_interval).tv_usec = 0;
       }
  
    if (prog->news_opened == FALSE) {
       BasePSCanvasObjectsCreate (base); 

/*
 * Note, do the make_newsserver even if we're running on a dps
 * server since this is where we figure out if we're running remotely.
 */

       make_newsserver ();
       prog->news_opened = TRUE;
       }

    if ((status = open_psfile (image)) != 0)
       return (status);

    status = reopen_ps ();
    if (status != 0)
       return (status);

/*
 * Get number of pages..
 */
       status = init_ps (image);

/*
 * number of pages is known at this point,
 * go ahead and create enough pixmaps to hold thumbnails
 */
    pageview_pixmaps_create(number_of_pages);

    if (status == 0) {
       check_pagecounter ();

       zoom = (float) prog->def_ps_zoom / 100.0;
       image->height = (int) (ps_display->pageheight * ps_display->res_y * 
			      zoom);
       image->width = (int) (ps_display->pagewidth * ps_display->res_x * zoom);
       if (prog->dps == TRUE) {
	  started_newfile = FALSE;
	  rendering_pages = TRUE;
	  if (pg_overview && pg_overview->showing == TRUE)
	    _DtTurnOnHourGlass (pg_overview->shell);
          pages_rendered = FALSE;
	  started_thumbnails = FALSE;
	  pixmaps_switched = FALSE;
	  }
       if (pg_overview != NULL) 
          pg_overview->thumbnails_created = False;
       }



/*
 * Note, if load was not successful for some reason, we will go back
 * to the old file in open.c (open_newfile). 
 */

    return (status);
}


void
first_page ()
{
    if (current_state->current_page != 1) {
        setbusy ();
        goto_page (1, FALSE, FALSE);
	set_pageview_pages (1);
        setactive ();
    }
}
 
void
last_page()
{
    if ((current_image->pages > 0) &&
        (current_state->current_page != current_image->pages)) {
        setbusy ();
        goto_page (current_image->pages, FALSE, FALSE);
	set_pageview_pages (current_image->pages);
        setactive ();
    }
}

void
next_page()          
{
    if (current_state->current_page < abs (current_image->pages)) {
        setbusy ();
        goto_page (current_state->current_page + 1, FALSE, FALSE);
	set_pageview_pages (current_state->current_page);
        setactive ();
    }
}

void
PgfwdButtonCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    if (current_state->current_page < abs (current_image->pages)) {
        setbusy ();
        goto_page (current_state->current_page + 1, FALSE, FALSE);
	set_pageview_pages (current_state->current_page);
        setactive ();
    }
}
 
void
prev_page()
{
    if (current_state->current_page > 1) {
        setbusy ();
        goto_page (current_state->current_page - 1, FALSE, FALSE);
	set_pageview_pages (current_state->current_page);
        setactive ();
    }
}

void
PgbackButtonCallback (w, client_data, call_data)
    Widget     w;
    XtPointer  client_data;
    XtPointer  call_data;
{
    if (current_state->current_page > 1) {
        setbusy ();
        goto_page (current_state->current_page - 1, FALSE, FALSE);
	set_pageview_pages (current_state->current_page);
        setactive ();
    }
}

void
GoToValueCallback (w, client_data, cbs)
     Widget                      w;
     XtPointer                   client_data;
     XmTextVerifyCallbackStruct *cbs;
{
     int  goto_value, i;

     if (cbs == NULL || pg_overview->thumbnails_created == False)
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
/*
 * User hit return in text field of spin box.
 * Default to Display Page.
 */
     else if (cbs->reason == XmCR_ACTIVATE) {

       char  *str;
       int    min, max, value;

       str = XmTextGetString (w);
       value = atoi (str);
       XtVaGetValues (pg_overview->goto_value, DtNminimumValue, &min,
		                               DtNmaximumValue, &max, NULL);
       if (value < min)
	 value = min;
       else if (value > max)
	 value = max;

       XtVaSetValues (pg_overview->goto_value, DtNposition, value, NULL);

     }
}

void
DisplayPageCallback (w, client_data, call_data)
     Widget     w;
     XtPointer  client_data;
     XtPointer  call_data;
{
     int	pageno;

     if (pg_overview->thumbnails_created == False)
       return;

     XtVaGetValues (pg_overview->goto_value, DtNposition, &pageno, NULL);

     if (pageno > current_image->pages)
       pageno = current_image->pages;

     if (pageno != current_state->current_page) {
       setbusy ();
       goto_page (pageno, FALSE, FALSE);
       set_pageview_pages (pageno);
       setactive ();
     }
}

void
PageSliderCallback (w, client_data, cbs)
     Widget                 w;
     XtPointer              client_data;
     XmScaleCallbackStruct *cbs;
{
     int	pageno, max, val;
     XmString   xmstr;
     char       value[80];
     
     switch (cbs->reason) {
     case XmCR_DRAG:
       sprintf (value, "%-d", (cbs->value));
       xmstr = XmStringCreateLocalized (value);
       XtVaSetValues (base->goto_pgno, XmNlabelString, xmstr, NULL);
       XmStringFree (xmstr);
       break;
     case XmCR_VALUE_CHANGED:
       if (!current_image)
	 return;
       else if (current_image && current_display == image_display)
	 return;
       
       XtVaGetValues (w, XmNvalue, &pageno, NULL);
       
       if (pageno > current_image->pages)
	 pageno = current_image->pages;
       
       if (pageno != current_state->current_page) {
	 setbusy ();
	 goto_page (pageno, FALSE, FALSE);
	 set_pageview_pages (pageno);
	 setactive ();
       }
       break;
     default:
       break;
     }
}

void
output_prolog (file, type)
FILE 		*file;
FileTypes	 type;
{
    fprintf (file, "\n/display_image {\n");
    fprintf (file, "    /colorimage where {\n");
    fprintf (file, "        pop colorimage\n");
    fprintf (file, "    } {\n");
    fprintf (file, "        pop pop pop\n");
    fprintf (file, "        /imagebuf 3 string def\n");
    fprintf (file, "        /str1 1 string def\n");
    fprintf (file, "        {\n");
    fprintf (file, "            str1 dup 0\n");
    fprintf (file, "            currentfile imagebuf readhexstring pop\n");
    fprintf (file, "            {} forall\n");
    fprintf (file, "            .11 mul exch .59 mul add exch .3 mul add\n");
    fprintf (file, "            round cvi put\n");
    fprintf (file, "         } image\n");
    fprintf (file, "    } ifelse\n");
    fprintf (file, "} def\n");

    if (type == EPSF)
       return ;

    fprintf (file, "\n/beginpage {\n");
    fprintf (file, "    /image_save save def\n");
    fprintf (file, "    gsave\n");
    fprintf (file, "} def\n");
    fprintf (file, "\n/endpage {\n");
    fprintf (file, "    grestore\n");
    fprintf (file, "    image_save restore\n");
    fprintf (file, "    showpage\n");
    fprintf (file, "} def\n");
    fprintf (file, "\n/center {\n");
    fprintf (file, "    11 72 mul exch sub 2 div dup 0 lt { pop 0 } if\n");
    fprintf (file, 
		"    exch 8.5 72 mul exch sub 2 div dup 0 lt { pop 0 } if\n");
    fprintf (file, "    exch\n");
    fprintf (file, "} def\n");


}

void
output_image (file, image)
FILE *file;
ImageInfo  *image;
{
    unsigned char *cb, *image_data;
    int count = 1;
    int i, j;
    int mx;
    unsigned int  pixel_stride, scanline_stride;
    
    fprintf (file, "%d %d scale\n", image->width,
			image->height);
    fprintf (file, "\n/picstr %d string def\n", 
		image->depth >= 8 ? image->width : image->bytes_per_line);
    fprintf (file, "%d %d %d [%d 0 0 -%d 0 %d]\n", image->width,
		image->height, image->depth >= 8 ? 8 : 1,
		image->width, image->height, image->height);
    fprintf (file, "{currentfile picstr readhexstring pop}\n");
    fprintf (file, "false %d display_image\n", 
			image->depth >= 8 ? 3 : 1);

    image_data = retrieve_data (image->orig_image, &pixel_stride,
				&scanline_stride);

    switch (image->depth) {
       case 1:  cb = (unsigned char *) image_data;
		for (i = 0 ; i < image->height; i++) {
		    for (j = 0; j < image->bytes_per_line; j++) {
			if (count++ & 31)
			   fprintf (file, "%02x", cb[j] ^ 255);
			else
			   fprintf (file, "%02x\n", cb[j] ^ 255);
		        }
		    cb += image->bytes_per_line;
		    }
		break; 
       case 8:  for (i = 0; i < image->height ; i++ ) { 
		     cb = (unsigned char *) (image_data +  
					     (image->bytes_per_line * i));
		     for (j = 0 ; j < image->width ; j++) {
			 fprintf (file, "%02x%02x%02x", 
				  image->red [*(cb+j)],
			          image->green [*(cb+j)], 
				  image->blue [*(cb+j)]);
			 if ( (count++ & 7) == 0)
			    putc ('\n', file);
			 }
		     }
		 break;
       case 24:  if (image->rgborder)
		   swap_red_blue (image, image_data, pixel_stride, 
				  image->bytes_per_line);

	         for (i = 0 ; i < image->height; i++ ) {
		     cb = (unsigned char *) (image_data + 
					     (image->bytes_per_line * i));
		     for (j = 0 ; j < image->width ; ++j, cb += pixel_stride) {
			 fprintf (file, "%02x%02x%02x", cb[2], cb[1], cb[0]);
			 if ( (count++ & 7) == 0)
			    putc ('\n', file);
			 }
		     }
       }

    reimport_image(image->orig_image, FALSE);

    putc ('\n', file);
}

int
postscript_save (image, type)
ImageInfo	*image;
FileTypes 	 type;
{
    FILE 	*save_file;
    struct tm	*current_time;
    time_t	 current_seconds;

    if ((save_file = fopen (image->file, "w"))  == NULL)
       return (-1);

/* 
 *  get the time/date
 */

    current_seconds = time ((time_t *) NULL);
    current_time = localtime (&current_seconds);

/*
 * type == 0 ... generate eps file
 * type == 1 ... generate ps file
 */

    if (type == EPSF) {
       fprintf (save_file, "%%%!PS-Adobe-3.0 EPSF-3.0\n");
       fprintf (save_file, "%%%%BoundingBox: 0 0 %d %d\n",
			image->width, image->height);
       fprintf (save_file, "%%%%Title: (%s)\n", basename (image->file));
       fprintf (save_file, "%%%%CreationDate: (%2d/%2d/%4d) (%2d:%2d %s)\n",
		    current_time->tm_mon + 1, current_time->tm_mday,
		    current_time->tm_year + 1900, 
		    current_time->tm_hour > 12 ? current_time->tm_hour - 12 : 
						 current_time->tm_hour,
		    current_time->tm_min,
		    current_time->tm_hour > 11 ? "PM" : "AM");
       fprintf (save_file, "%%%%EndComments\n");

       output_prolog (save_file, type);
       output_image (save_file, image);
       
       fprintf (save_file, "\nshowpage\n");
       fprintf (save_file, "%%%%EOF\n");
       }
    else {
       fprintf (save_file, "%%%!PS-Adobe-3.0\n");
       fprintf (save_file, "%%%%Pages: 1\n");
       fprintf (save_file, "%%%%Title: (%s)\n", basename (image->file));
       fprintf (save_file, "%%%%Creator: %s\n", cuserid ((char *) NULL));
       fprintf (save_file, "%%%%CreationDate: (%2d/%2d/%4d) (%2d:%2d %s)\n",
		    current_time->tm_mon + 1, current_time->tm_mday,
		    current_time->tm_year + 1900, 
		    current_time->tm_hour > 12 ? current_time->tm_hour - 12 : 
						 current_time->tm_hour,
		    current_time->tm_min,
		    current_time->tm_hour > 11 ? "PM" : "AM");
       fprintf (save_file, "%%%%EndComments\n");
       fprintf (save_file, "%%%%BeginProlog\n");

       output_prolog (save_file, type);

       fprintf (save_file, "\n%%%%EndProlog\n");

       fprintf (save_file, "%%%%Page: 1 1\n");
       fprintf (save_file, "beginpage\n");
       fprintf (save_file, "%d %d center translate\n", image->width, 
					     image->height);
       
       output_image (save_file, image);

       fprintf (save_file, "\nendpage\n");
       fprintf (save_file, "%%%%Trailer\n");
       fprintf (save_file, "%%%%EOF\n");
       }

    fclose (save_file);
  
    return (0);
}

XilImage 
create_image_from_ps (image)
ImageInfo	*image;
{
    XImage           *ximage;
    XilImage          xilimage;
    XilDataType       datatype;
    Xil_boolean       success;
    XilMemoryStorage  storage;
    int x, y;
    Window root;
    int i, size;
    int x_index, xil_index, row, col;
    unsigned int      pixel_stride;
    unsigned char    *startlineptr, *xstartlineptr;
    unsigned char    *lineptr, *xlineptr;
    XWindowAttributes win_attrs;	
    int 	      xpixel_stride;
   
    if (prog->ps_mono == True) {
      Pixmap   pixmap;
/*
 * Copy pixmap1 bit plane to pixmap of ps_display->depth.
 */
      pixmap = create_pixmap (ps_display, 
				    (int) (ps_display->pix_width),
				    (int) (ps_display->pix_height),
				    ps_display->depth, FALSE);
      XCopyPlane (ps_display->xdisplay, ps_display->pixmap1, pixmap,
		 ps_display->win_gc, 0, 0, (int) (ps_display->pix_width),
		 (int) (ps_display->pix_height), 0, 0, 1L);

      ximage = XGetImage (ps_display->xdisplay, pixmap, 0, 0, 
			ps_display->pix_width, ps_display->pix_height, 
			AllPlanes, ps_display->depth == 1 ? XYPixmap : ZPixmap);
      XFreePixmap (ps_display->xdisplay, pixmap);
/*
 * Convert ximage into ps_display->depth ximage.
 */
      
    }
    else
      ximage = XGetImage (ps_display->xdisplay, ps_display->pixmap1, 0, 0, 
			ps_display->pix_width, ps_display->pix_height, 
			AllPlanes, ps_display->depth == 1 ? XYPixmap : ZPixmap);

    xpixel_stride = ximage->bytes_per_line / ximage->width;

    image->width = ximage->width;
    image->height = ximage->height;
    
    if (ps_display->depth == 8) {
       XColor xcolors [256];
       Colormap cmap;

       XGetWindowAttributes(ps_display->xdisplay, ps_display->win, &win_attrs); 
       cmap = win_attrs.colormap;

       image->colors = 256;
       image->red = (unsigned char *) malloc (sizeof (unsigned char) * 
						      256);
       image->green = (unsigned char *) malloc (sizeof (unsigned char) *
							256);
       image->blue = (unsigned char *) malloc (sizeof (unsigned char) *
						       256);

       for (i = 0; i < 256; i++)
	   xcolors[i].pixel = i;

       XQueryColors (ps_display->xdisplay, cmap, xcolors, 256);

       for (i = 0; i < 256; i++) {
	   image->red [i] = xcolors[i].red >> 8;
	   image->green [i] = xcolors[i].green >> 8;
	   image->blue [i] = xcolors[i].blue >> 8;
	}
    }

    datatype = ps_display->depth == 1 ? XIL_BIT : XIL_BYTE;
    xilimage = (*xil->create) (image_display->state, ps_display->pix_width,
	ps_display->pix_height, ps_display->depth == 24 ? 3 : 1, datatype);

    (*xil->export_ptr) (xilimage);
    success = (*xil->get_memory_storage) (xilimage, &storage);
    if (success == FALSE) {
       if (prog->verbose)
          fprintf (stderr, 
	      catgets (prog->catd, 1, 269, 
	      "%s.create_image_from_ps: xil_get_memory_storage_failed\n"),
	      prog->name);
       XDestroyImage (ximage);
       return (NULL);
    }

    if (datatype == XIL_BYTE) {
	image->bytes_per_line = storage.byte.scanline_stride;
	pixel_stride = storage.byte.pixel_stride;
    } else {
	image->bytes_per_line = storage.bit.scanline_stride;
    }

    /*
     * Copy data to new image. Now, image->bytes_per_line should be
     * correct for all depths, so we should be able to copy the correct
     * number of bytes from the ximage to the xil image in a standard way.
     */
    xil_index = 0;
    x_index = 0;
    if (ps_display->depth != 24) {
	for (row = 0; row < ximage->height; row++) {
	    memcpy (&storage.byte.data[xil_index], &ximage->data[x_index], 
		image->bytes_per_line);
	    xil_index += image->bytes_per_line;
	    x_index += ximage->bytes_per_line;
	}
    } else {
	startlineptr = storage.byte.data;
	xstartlineptr = (unsigned char *) ximage->data;

	lineptr = startlineptr;
	xlineptr = xstartlineptr + 1;

	for (row = 0; row < ximage->height; row++) {
	    for (col = 0; col < ximage->width; col++) {
		memcpy (lineptr, xlineptr, 3);
		lineptr += pixel_stride;
		xlineptr += xpixel_stride;
	    }
	    startlineptr += image->bytes_per_line;
	    lineptr = startlineptr;
	    xstartlineptr += ximage->bytes_per_line;
	    xlineptr = xstartlineptr + 1;
	}
	image->rgborder = 0;
/*
	swap_red_blue (image, storage.byte.data, pixel_stride, 
 	               image->bytes_per_line);
*/
    }

    XDestroyImage (ximage);
    return (xilimage);
}

int
ps_save (image)
ImageInfo	*image;
{
    return (postscript_save (image, POSTSCRIPT));
}

int
epsf_save (image)
ImageInfo	*image;
{
    return (postscript_save (image, EPSF));
}


void
ps_hflip_func (image)
    XilImage     image;
{
    if (current_state->frontside)
      current_state->frontside = FALSE;
    else
      current_state->frontside = TRUE;
 
    if (current_state->frontside == FALSE) {
       if (prog->dps == TRUE)
          dps_vflip (current_image->width);
       }
 
    current_state->angle = 180 - current_state->angle;
    if (current_state->angle < 0)
      current_state->angle += 360;
}

void
ps_zoom_func (image)
    XilImage     image;
{
    int 	new_width,
		new_height;


    make_pixmaps = FALSE;

    new_width = (int) (ps_display->pagewidth * ps_display->res_x * 
		       current_state->zoom);
    new_height = (int) (ps_display->pageheight * ps_display->res_y * 
			current_state->zoom);


    if ( new_width <= 0 || new_height <= 0 ) {
       if (prog->verbose)
          fprintf (stderr, catgets (prog->catd, 1, 270,
			   "%s: Unable to zoom image.\n"), prog->name);
       return;
       }  

   
    if (dps_context == NULL) {

      extern void make_new_dsc_context();

      make_new_dsc_context ();

      dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
	         new_width, new_height, (int) ps_display->res_x, FALSE, FALSE);

      dps_dsc_paging ();
    }
    else
      XDPSUnfreezeContext (dps_context);


    if (prog->dps == TRUE) {
       if ((image == (XilImage) NULL) && (current_state->using_dsc == FALSE)) {
          DPSInterruptContext (dps_context);
          DPSWritePostScript (dps_context, END_OF_FILE, strlen (END_OF_FILE));
	  }
       dps_reset_ctm (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty);
       }

/*
 * If zoom == 1.0, we don't need to do anything, since the default
 * transformation matrix is based on a 1.0 zoom.
 */

    if (current_state->zoom != 1.0) {
       if (prog->dps == TRUE)
          dps_zoom (current_state->zoom, new_height);
       }

    if (current_image->width != new_width) 
       make_pixmaps = TRUE;

    ps_display->pix_width = new_width;
    ps_display->pix_height = new_height;
    current_image->width  = new_width;
    current_image->height = new_height;
 
}

void
ps_turnover_func (image)
    XilImage     image;
{
    if (current_state->frontside)
      return;

    if (prog->dps == TRUE)
       dps_vflip (current_image->width);

}
 
void
ps_vflip_func (image)
    XilImage     image;
{
    if (current_state->frontside)
      current_state->frontside = FALSE;
    else
      current_state->frontside = TRUE;

    if (current_state->frontside == FALSE)
       if (prog->dps == TRUE)
          dps_vflip (current_image->width);
 
    current_state->angle = 360 - current_state->angle;
    current_state->angle %= 360;

}  

void
ps_rotate_func (image)
    XilImage     image;
{
    int		xoffset,
		yoffset;
    int		new_width,
		new_height;
    int		tmp_angle,
		tmp_angle2;

/*
 * Since this is the last function called, in here we recreate
 * the pixmaps, and render the page.
 */

/*
 * If angle == 0, do nothing, image is already
 * in the view image after.
 */

    if (current_state->angle != 0) { 

/*   
 * Calculate new width and height after rotation.
 */

       get_dimensions (current_image->width, current_image->height, 
		       &new_width, &new_height);

       ps_display->pix_height = new_height;
       ps_display->pix_width = new_width;

/*
 * Need to figure out new x and y values, based on rotation.
 * If angle between 1 and 90 degrees, already know yoffset (new_height), 
 *		figure out xoffset (pageheight * sin angle)
 * If angle between 91 and 180,  already know xoffset (new_width),
 * 		figure out yoffset (pageheight * cos angle)
 * If angle between 181 and 270, already know yoffset (new_height),
 *		figure out xoffset (pagewidth * sin angle)
 * If angle between 271 and 359, already know xoffset (new_width),
 * 		figure out yoffset (pageheight * cos angle)
 */

       if (current_state->frontside == TRUE) {

/*
 * If frontside, then we need to subtract angle from 360...
 */

          tmp_angle = 360 - current_state->angle;

          if (tmp_angle > 0 && tmp_angle <= 90) {
	     tmp_angle2 = 90 - tmp_angle;
	     xoffset = (int) (current_image->height * 
			      cos (RADIANS (tmp_angle2))); 
	     yoffset = new_height;
             }
          else if (tmp_angle > 91 && tmp_angle <= 180) {
	     tmp_angle2 = tmp_angle - 90;
   	     xoffset = new_width;
	     yoffset = (int) (current_image->width * 
			      cos (RADIANS (tmp_angle2))); 
	     }
          else if (tmp_angle > 181 && tmp_angle <= 270) {
	     tmp_angle2 = tmp_angle - 180;
	     xoffset = (int) (current_image->width * 
			      cos (RADIANS (tmp_angle2)));
	     yoffset = 0;
             }
          else {     /* angle is between 271 and 359 */
	     tmp_angle2 = 360 - tmp_angle ;
   	     xoffset = 0;
	     yoffset = (int) (current_image->height * 
		 	      cos (RADIANS (tmp_angle2)));
	     }
	  }

       else {    /*  image was flipped */
	  tmp_angle = current_state->angle;
          if (tmp_angle > 0 && tmp_angle <= 90) {
	     xoffset = (int) (current_image->width * 
			      cos (RADIANS (tmp_angle))); 
	     yoffset = new_height;
             }
          else if (tmp_angle > 91 && tmp_angle <= 180) {
	     tmp_angle2 = tmp_angle - 90;
   	     xoffset = 0;
	     yoffset = (int) (current_image->width * 
			      cos (RADIANS (tmp_angle2))); 
	     }
          else if (tmp_angle > 181 && tmp_angle <= 270) {
	     tmp_angle2 = 270 - tmp_angle;
	     xoffset = (int) (current_image->height * 
			      cos (RADIANS (tmp_angle2)));
	     yoffset = 0;
             }
          else {     /* angle is between 271 and 359 */
	     tmp_angle2 = 360 - tmp_angle ;
   	     xoffset = new_width;
	     yoffset = (int) (current_image->height * 
		 	      cos (RADIANS (tmp_angle2)));
	     }
	  }

       if (prog->dps == TRUE)
          dps_rotate (tmp_angle, xoffset, yoffset);
      
/*
 * Set the new view to the rotated image.
 */
 
       if ((current_image->width != new_width) ||
   	  (current_image->height != new_height))
	  make_pixmaps = TRUE;

       current_image->width = new_width;
       current_image->height = new_height;
       }

/*
 * Note that even if make_pixmaps is FALSE, if dsc is true, we still want
 * to make the new pixmaps and start over so that we initialize the ctm.
 */

    if ((make_pixmaps == TRUE) || (current_state->using_dsc == TRUE)) {
       make_new_pixmaps (current_state->zoom);
       if (prog->dps == TRUE) {
	  if (current_state->using_dsc == TRUE) {
	     int	orig_width,
			orig_height;

	     make_new_dsc_context ();
       	     orig_width = ps_display->pagewidth * (int) ps_display->res_x;
       	     orig_height = ps_display->pageheight * (int) ps_display->res_y;
             dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty, orig_width,
			orig_height, (int) ps_display->res_x, FALSE, FALSE);
    	     if (current_state->zoom != 1.0) 
                dps_zoom (current_state->zoom, current_image->height);
    	     if (current_state->frontside == FALSE)
       		dps_vflip (current_image->width);
    	     if (current_state->angle != 0) 
          	dps_rotate (tmp_angle, xoffset, yoffset);
       	     dps_dsc_paging ();
             if (end_prolog > 0)
	        pump_bytes (0, end_prolog, TRUE);
             if ((begin_setup > 0) && (end_setup > 0))
	        pump_bytes (begin_setup, end_setup, TRUE);
	     }
	  else 
             dps_new_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
	  }
       }

/*
 * Note if the third argument is always TRUE since we either did the
 * interrupt already (back in zoom) or don't want to do it at all.
 */

    goto_page (current_state->current_page, TRUE, TRUE);
}

/*
 * If we had a ps file loaded, and tried to load in a new file,
 * and it didn't work, we have to reopen the currently viewed ps
 * file.
 */

void
restart_ps ()
{
    open_psfile (current_image);
    using_dsc = current_state->using_dsc;

    reopen_ps ();
    MakePaper ();

    if (prog->dps == TRUE) {
       dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty, 
	         ps_display->pix_width, ps_display->pix_height, 
	         (int) ps_display->res_x, FALSE, FALSE);
   
       if (current_state->using_dsc == TRUE) {
	  dps_dsc_paging ();
          if (end_prolog > 0)
	     pump_bytes (0, end_prolog, TRUE);
          if ((begin_setup > 0) && (end_setup > 0))
	     pump_bytes (begin_setup, end_setup, TRUE);
	  }
       else {
          dps_new_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
          dps_set_all_pages (FALSE);
	  dps_fast_paging ();
	  DPSFlushContext (dps_context);
	  DPSclientsync (dps_context);
	  rendering = TRUE;
	  page_requested = 0;
    	  wait_for_dps_event (&rendering);

/*
 * Besides setting up for rendering pages, render page one, so we'll be
 * sitting at a clientsync (if # pages > 1)

	  dps_page_requested (1);
	  pump_bytes (0, current_image->file_size, TRUE);
 */
	  }
       }

    ps_zoom_func (NULL);
    ps_turnover_func (NULL);
    ps_rotate_func (NULL);
}

void
reset_ps (new_page_size, fit_frame)
int	new_page_size;
int	fit_frame;
{
   
/*
 * If we just switched to a new page size, then do a MakePaper
 * to that we recalculate the default transformation matrix for this page
 * size.
 */

    if (new_page_size == TRUE) 
       restart_ps ();
    else {
       if (prog->dps == TRUE) {
          dps_reset_ctm (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty);
          dps_set_height_width (ps_display->pix_height, ps_display->pix_width);
          }

       ps_zoom_func (NULL);
       ps_turnover_func (NULL);
       ps_rotate_func (NULL);
       }

    resize_canvas ();
    if (fit_frame == TRUE)
       fit_frame_to_image (current_image);
    display_new_image ();
}

void
ps_revert_func ()
{

  if(current_image->orient == False)
    {

      /* ps_display->pix_height = current_image->height;
	 ps_display->pix_width = current_image->width;
       */
    }
  else
    {
      current_image->width = ps_display->pagewidth * ps_display->res_x *
	current_state->zoom;
      current_image->height = ps_display->pageheight * ps_display->res_y *
	current_state->zoom;
    }


    if (prog->dps == TRUE) {
       if (current_state->using_dsc == FALSE) {
          DPSInterruptContext (dps_context);
          DPSWritePostScript (dps_context, END_OF_FILE, strlen (END_OF_FILE));
	  }
       dps_reset_ctm (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty);
       }


    if(current_image->orient == LANDSCAPE)
      {
	int new_height, new_width;
	get_dimensions (current_image->width, current_image->height, 
			&new_width, &new_height);

	
        ps_display->pix_height = new_height; 
        ps_display->pix_width = new_width; 

	current_image->width = new_width;
	current_image->height = new_height;

      }
    else 
      { 
	ps_display->pix_width = ps_display->pagewidth * ps_display->res_x *
	  current_state->zoom;
	ps_display->pix_height = ps_display->pageheight * ps_display->res_y *
	  current_state->zoom;
      } 
/*
 * If zoom == 1.0, we don't need to do anything, since the default
 * transformation matrix is based on a 1.0 zoom.
 */
                            
    if (current_state->zoom != 1.0) {
       if (prog->dps == TRUE)
          dps_zoom (current_state->zoom, ps_display->pix_height);
       } 



    make_new_pixmaps (current_state->zoom);
    if (prog->dps == TRUE) {
       if (current_state->using_dsc == TRUE) {
	  int	orig_width,
		orig_height;

	  make_new_dsc_context ();
	  
	  if(current_image->orient == LANDSCAPE)
	    {
/* 	      orig_width = ps_display->pageheight * (int) ps_display->res_x; */
/* 	      orig_height = ps_display->pagewidth * (int) ps_display->res_y;  */
	      dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
			 ps_display->pix_height, ps_display->pix_width, 
			 (int) ps_display->res_x, FALSE, FALSE);

	    }
	  else
	    {
/* 	      orig_width = ps_display->pagewidth * (int) ps_display->res_x; */
/* 	      orig_height = ps_display->pageheight * (int) ps_display->res_y; */
	      dps_setup (ctm_a, ctm_b, ctm_c, ctm_d, ctm_tx, ctm_ty,
			 ps_display->pix_width, ps_display->pix_height, 
			 (int) ps_display->res_x, FALSE, FALSE);

	    }

    	  if (current_state->zoom != 1.0) 
             dps_zoom (current_state->zoom, ps_display->pix_height);
    /*
     * See if this fixes the landscape problem
     */
	  if (current_image->orient == LANDSCAPE)
	    dps_rotate (270, 0, 0);
       	  dps_dsc_paging ();
          if (end_prolog > 0)
	     pump_bytes (0, end_prolog, TRUE);
          if ((begin_setup > 0) && (end_setup > 0))
	     pump_bytes (begin_setup, end_setup, TRUE);
	  }
       else
          dps_new_pixmaps (ps_display->pixmap1, ps_display->pixmap2);
       }
    goto_page (current_state->current_page, TRUE, TRUE);
}

void
MakeStdColormaps (obj, vis_class)
Widget   	obj;
int		vis_class;
{
    int		 status;
    Colormap	 canvas_cmap;
    int		 depth;
    int		 win;
    Display     *dpy;
    int		 screen;
    XVisualInfo	 vinfo;

    XtVaGetValues (obj, XmNdepth, &depth, NULL);
    win = XtWindow (obj);
    dpy = XtDisplay (obj);
    screen = DefaultScreen (dpy);
    XtVaGetValues (obj, XmNcolormap, &canvas_cmap, NULL);

    status = XMatchVisualInfo (dpy, screen, depth, vis_class, &vinfo);

/*
 * Only values of vis_class that are passed to this function are:
 * StaticGray (could be depth 1 or 8), StaticColor, TrueColor (24 bit only),
 * GrayScale or PseudoColor.
 */

    if (vis_class == StaticGray || prog->ps_mono == True) {

       rgb_cmap = (XStandardColormap *) 
			calloc (1, sizeof (XStandardColormap));
       gray_cmap = (XStandardColormap *) 
			calloc (1, sizeof (XStandardColormap));
       gray_cmap->colormap = canvas_cmap;
       if (prog->ps_mono == True) {
	 gray_cmap->colormap = None;
	 gray_cmap->red_max = 1;
	 gray_cmap->red_mult = -1;
	 gray_cmap->base_pixel = 1;
       }
       else {
         status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, 
					     (XStandardColormap *) NULL, 
					     gray_cmap, False);
       }
    }

/*
 * TrueColor, and StaticColor handled the same.
 */

    else if ((vis_class == TrueColor) || (vis_class == StaticColor)) {
       rgb_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
       gray_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
       rgb_cmap->colormap = canvas_cmap;
       gray_cmap->colormap = canvas_cmap;
       status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, rgb_cmap, 
					     gray_cmap, False);
       }

/*
 * PseudoColor and GrayScale might need new colormaps created. Also, copy
 * values from the default colormap so that flashing is minimized.
 * 11/16/93 - note may be able to add DirectColor here also if
 *	      Adobe fixed their bug.
 */

    else {		/* vis is PseudoColor or GrayScale */

/*
 * First try and allocate in default colormap 
 */
       gray_cmap = (XStandardColormap *) 
			calloc (1, sizeof (XStandardColormap));
       gray_cmap->colormap = canvas_cmap;

       if ((vis_class == PseudoColor) || (vis_class == DirectColor)) {
          rgb_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
          rgb_cmap->colormap = canvas_cmap;
   	  }

       status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, rgb_cmap, 
					     gray_cmap, False);

/*
 * If we couldn't allocate the colors, then we have to create our own
 * colormap.
 */

       if ((rgb_cmap != (XStandardColormap *) NULL) &&
	   (rgb_cmap->red_max == 0)) { 
          unsigned long plane_masks [1];
          unsigned long pixels [40];

/*
 * Check if the canvas_cmap is the Default... if so, then create a new
 * one. If not, then use it.
 */

          canvas_cmap = XCreateColormap (dpy, win, vinfo.visual, AllocNone);

          if (depth == 4) {
             status = XAllocColorCells (dpy, canvas_cmap, True, plane_masks,
                                        0, pixels, 6);
             copy_default_cmap (dpy, canvas_cmap, 6);
             }
          else if (DefaultDepth (dpy, screen) == 4) {
             status = XAllocColorCells (dpy, canvas_cmap, True, plane_masks,
                                        0, pixels, 16);
             copy_default_cmap (dpy, canvas_cmap, 16);
             }
          else if (depth == 8) {
             status = XAllocColorCells (dpy, canvas_cmap, True, plane_masks,
                                        0, pixels, 40);
             copy_default_cmap (dpy, canvas_cmap, 40);
             }

          gray_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
          gray_cmap->colormap = canvas_cmap;

          if ((vis_class == PseudoColor) || (vis_class == DirectColor)) {
             rgb_cmap = (XStandardColormap *) 
				   calloc (1, sizeof (XStandardColormap));
             rgb_cmap->colormap = canvas_cmap;
   	     }

          status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					        0, 0, 0, 0, rgb_cmap, 
						gray_cmap, False);

          XSetWindowColormap (dpy, win, canvas_cmap);

          }
       }
}

void
MakeSmallStdColormaps (obj, vis_class)
Widget   	obj;
int             vis_class;
{
    int		 status;
    Colormap	 cmap;
    int		 depth;
    int		 win;
    Display     *dpy;
    int		 screen;
    XVisualInfo	 vinfo;
    XColor       xcolor;

    XtVaGetValues (obj, XmNdepth, &depth, NULL);
    win = XtWindow (obj);
    dpy = XtDisplay (obj);
    screen = DefaultScreen (dpy);
    XtVaGetValues (obj, XmNcolormap, &cmap, NULL);

    status = XMatchVisualInfo (dpy, screen, depth, vis_class, &vinfo);

/*
 * Only values of vis_class that are passed to this function are:
 * StaticGray (could be depth 1 or 8), StaticColor, TrueColor (24 bit only),
 * GrayScale or PseudoColor.
 */

    if (vis_class == StaticGray || 
        ((prog->ps_mono == True)  && (vis_class != TrueColor) &&
	 (DefaultDepth (dpy, screen) != 24))) {
 
       small_rgb_cmap = (XStandardColormap *)
                        calloc (1, sizeof (XStandardColormap));
       small_gray_cmap = (XStandardColormap *) 
			calloc (1, sizeof (XStandardColormap));
       small_gray_cmap->colormap = cmap;

       if (prog->ps_mono == True) {
         small_gray_cmap->red_max = 1;
 
         xcolor.red = xcolor.blue = xcolor.green = 0;
         XAllocColor (dpy, cmap, &xcolor);
         small_gray_cmap->base_pixel = xcolor.pixel;
 
         xcolor.red = xcolor.blue = xcolor.green = 65535;
         XAllocColor (dpy, cmap, &xcolor);
         small_gray_cmap->red_mult = xcolor.pixel - small_gray_cmap->base_pixel;
       }
       else {
         status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, 
					     (XStandardColormap *) NULL, 
					     small_gray_cmap, False);
       }
    }
/*
 * TrueColor, and StaticColor handled the same.
 */

    else if ((vis_class == TrueColor) || (vis_class == StaticColor)) {
       small_rgb_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
       small_gray_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
       small_rgb_cmap->colormap = cmap;
       small_gray_cmap->colormap = cmap;
       status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, small_rgb_cmap, 
					     small_gray_cmap, False);
       }

/*
 * PseudoColor and GrayScale might need new colormaps created. Also, copy
 * values from the default colormap so that flashing is minimized.
 * 11/16/93 - note may be able to add DirectColor here also if
 *	      Adobe fixed their bug.
 */

    else {		/* vis is PseudoColor or GrayScale */

/*
 * First try and allocate in default colormap 
 */
       small_gray_cmap = (XStandardColormap *) 
			calloc (1, sizeof (XStandardColormap));
       small_gray_cmap->colormap = cmap;

       if ((vis_class == PseudoColor) || (vis_class == DirectColor)) {
          small_rgb_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
          small_rgb_cmap->colormap = cmap;
   	  }

       status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					     0, 0, 0, 0, small_rgb_cmap, 
					     small_gray_cmap, False);

/*
 * If we couldn't allocate the colors, then we have to create our own
 * colormap.
 */

       if ((small_rgb_cmap != (XStandardColormap *) NULL) &&
	   (small_rgb_cmap->red_max == 0)) { 
          unsigned long plane_masks [1];
          unsigned long pixels [40];

/*
 * Check if the canvas_cmap is the Default... if so, then create a new
 * one. If not, then use it.
 */

          cmap = XCreateColormap (dpy, win, vinfo.visual, AllocNone);

          if (depth == 4) {
             status = XAllocColorCells (dpy, cmap, True, plane_masks,
                                        0, pixels, 6);
             copy_default_cmap (dpy, cmap, 6);
             }
          else if (DefaultDepth (dpy, screen) == 4) {
             status = XAllocColorCells (dpy, cmap, True, plane_masks,
                                        0, pixels, 16);
             copy_default_cmap (dpy, cmap, 16);
             }
          else if (depth == 8) {
             status = XAllocColorCells (dpy, cmap, True, plane_masks,
                                        0, pixels, 40);
             copy_default_cmap (dpy, cmap, 40);
             }

          small_gray_cmap = (XStandardColormap *) 
				calloc (1, sizeof (XStandardColormap));
          small_gray_cmap->colormap = cmap;

          if ((vis_class == PseudoColor) || (vis_class == DirectColor)) {
             small_rgb_cmap = (XStandardColormap *) 
				   calloc (1, sizeof (XStandardColormap));
             small_rgb_cmap->colormap = cmap;
   	     }

          status = XDPSCreateStandardColormaps (dpy, win, vinfo.visual, 
					        0, 0, 0, 0, small_rgb_cmap, 
						small_gray_cmap, False);

          XSetWindowColormap (dpy, win, cmap);

          }
       }
}

int
display_postscript_present( Display* dpy )
{
    DPSContext ctxt;
    XStandardColormap gray_ramp = {0};
    XStandardColormap color_ramp = {0};
    int present = FALSE;
    
    /* make sure it doesn't choke on invalid grayramp */
    gray_ramp.red_max = 1;
    gray_ramp.red_mult = 1;
    color_ramp.red_max = 1;
    color_ramp.red_mult = 1;

    ctxt = XDPSCreateContext(dpy, None, None, 0, 0, 0,
			     &gray_ramp, &color_ramp, 0,
			     DPSDefaultTextBackstop,
			     DPSDefaultErrorProc,
			     NULL);

    if ( ctxt != (DPSContext) NULL ) {
	present = TRUE;
	DPSDestroySpace(DPSSpaceFromContext(ctxt));
    }

    return present;
}
