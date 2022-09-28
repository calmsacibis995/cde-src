#ifndef lint
static char sccsid[] = "@(#)tt.c 1.14 97/05/29";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * tt.c - Functions used for tooltalk interface.
 */

#include <limits.h>
#include <sys/param.h>
#include <Tt/tttk.h>

#include "display.h"
#include "image.h"
#include "imagetool.h"
#include "props.h"
#include "ui_imagetool.h"

#define TIMEOUT_FACTOR        1000

/*
 * foward declarations
 */
static void        TTDeclarePtypes();
static Tt_message  TTContractCallback();
static Tt_message  TTMessageCallback();

static Tt_message  _contract = 0;
static Boolean     new_msg = False;

/*
 * (DKenny - 29 May, 1997)
 *
 * Fix for bug 4039282 sdtimage with "Print Preview" window can not be brought
 *                     up again from dtmail
 *
 * When we receive a new TT connection, we actually want the call to 
 * tttk_block_while, to return. Without this what was happening was that
 * the call to tttk_block_while never returned, except on timeout, and
 * kept handling the X events internally! - not the main Xevent loop. This
 * caused some inconsistencies in the state of things. So now we use the
 * variable tt_waiting, to allow us to return on reciept of a new
 * tt_connection. How it works:
 *
 * tt_waiting is: Set when 'exiting' and we are blocking for new connection.
 *                Unset when we get a new connection.
 */

static int         tt_waiting = 0;

Boolean
TTInitialize (Display *dpy)
{
     Tt_status	        status;
     Tt_pattern        *pattern;
     char	       *procid, *session, *ttenv = NULL;
     int		ttfd;

     ttenv = (char *) getenv ("TT_SESSION");
     if (!ttenv || strlen (ttenv) == 0) {
       session = tt_X_session (DisplayString (dpy));
       tt_default_session_set (session);
       tt_free (session);
     }
     procid = ttdt_open (&ttfd, "Sdtimage", "SunSoft", "%I", 1);

     if (tt_is_err (tt_ptr_error (procid))) 
       return False;
/*
 * Declare ptypes
 */
#ifdef DEBUG
printf ("declaring ptypes...\n");
#endif
     TTDeclarePtypes ();
#ifdef DEBUG
printf (" done\n ");
#endif
/*
 * Join session
 */     
     pattern = ttdt_session_join (0, TTContractCallback, base->top_level, 
				  NULL, 1);
     status = tt_ptr_error (pattern);
     if (tt_is_err (tt_ptr_error (pattern)))
       return False;
/*
 * Catch any input on tt file descriptor.
 */     
     XtAppAddInput (base->app_context, ttfd, (XtPointer) XtInputReadMask, 
		    tttk_Xt_input_handler, procid);
/* 
 * Process the message that started us, if any
 * This needs to go AFTER ptype declaration and XtAppAddInput()!
 */
     tttk_Xt_input_handler (0, 0, 0);

     return True;
}


Boolean
TTFailMessage ()
{
   Tt_status   status = TT_OK;
   Display    *dpy;

   if (prog->tt_started == True) {
     if (_contract != 0) {
       /*
        * Reply to the message in process first.
        */
       new_msg = False;
       tt_message_reply (_contract);
       tttk_message_destroy (_contract);
       _contract = 0;
     }
     /*
      * Unmap frame and set timeout to not go away immediately.
      */
     dpy = XtDisplay (base->top_level);
     XWithdrawWindow (dpy, XtWindow (base->top_level), DefaultScreen (dpy));

     /*
      * (DKenny - 29 May, 1997)
      *
      * Fix for bug 4039282 sdtimage with "Print Preview" window can not be 
      *                     brought up again from dtmail
      *
      * Set to 1 - tttk_block_while returns when < 1 (Done in TTMessageCallback)
      */
     tt_waiting = 1;
     prog->tt_timer_set = True;
     status = tttk_block_while (base->app_context, &tt_waiting, current_props->tt_timeout * 1000);
#ifdef DEBUG
     if (status == TT_DESKTOP_ETIMEDOUT)
       fprintf (stderr, "tttk_block_while() timed out.\n");
     else if (status != TT_OK)
       fprintf (stderr, "tttk_block_while() failed %d\n", status);
#endif
     prog->tt_timer_set = False;
     /*	
      * Check to see if we got any new display messages
      * since we went to sleep.
      */
     if (_contract != 0 || new_msg == True)
       return False;	/* Don't quit process. */
     else
       return True;     /* Quit process */
   }

   return True;

}

static void
TTCheckPtypeStatus (status)
     Tt_status status;
{
     if (tt_is_err (status)) {
       fprintf (stderr, catgets (prog->catd, 1, 351,
			  "Sdtimage: Could not declare ptypes - %s\n"), 
		tt_status_message (status));
     }
}

static void
TTDeclarePtypes ()
{
     Tt_status          status;

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 0, 
				     TTMessageCallback, "Sun-raster", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 1000, 
				     TTMessageCallback, "GIF", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 2000, 
				     TTMessageCallback, "TIFF", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 3000, 
				     TTMessageCallback, "POSTSCRIPT", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 4000, 
				     TTMessageCallback, "JPEG", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 5000, 
				     TTMessageCallback, "photocd-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 6000, 
				     TTMessageCallback, "pict-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 7000, 
				     TTMessageCallback, "ppm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 8000, 
				     TTMessageCallback, "pj-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 9000, 
				     TTMessageCallback, "pgm-file", 0);
     TTCheckPtypeStatus (status);
     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 10000, 
				     TTMessageCallback, "pbm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 11000, 
				     TTMessageCallback, "ilbm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 12000, 
				     TTMessageCallback, "pi1-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 13000, 
				     TTMessageCallback, "pcx-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 14000, 
				     TTMessageCallback, "tga-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 15000, 
				     TTMessageCallback, "xim-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 16000, 
				     TTMessageCallback, "mtv-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 17000, 
				     TTMessageCallback, "qrt-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 18000, 
				     TTMessageCallback, "imgw-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 19000, 
				     TTMessageCallback, "sld-file", 0);
     TTCheckPtypeStatus (status);
     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 20000, 
				     TTMessageCallback, "spc-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 21000, 
				     TTMessageCallback, "spu-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 22000, 
				     TTMessageCallback, "gould-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 23000, 
				     TTMessageCallback, "fs-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 24000, 
				     TTMessageCallback, "fits-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 25000, 
				     TTMessageCallback, "lispm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 26000, 
				     TTMessageCallback, "hips-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 27000, 
				     TTMessageCallback, "mac-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 28000, 
				     TTMessageCallback, "atk-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 29000, 
				     TTMessageCallback, "brush-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 30000, 
				     TTMessageCallback, "cmu-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 31000, 
				     TTMessageCallback, "mgr-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 32000, 
				     TTMessageCallback, "g3-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 33000, 
				     TTMessageCallback, "img-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 34000, 
				     TTMessageCallback, "pi3-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 35000, 
				     TTMessageCallback, "xbm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 36000, 
				     TTMessageCallback, "ybm-file", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 37000, 
				     TTMessageCallback, "XWD", 0);
     TTCheckPtypeStatus (status);

     status = ttmedia_ptype_declare ("Sun_CDEImagetool", 38000, 
				     TTMessageCallback, "PM", 1);
     TTCheckPtypeStatus (status);
}

void
TTBreak ()
{
    if (prog->tt_sender != (char *) NULL)
       free (prog->tt_sender);

    prog->tt_sender = (char *) NULL;
    prog->tt_started = FALSE;
    new_msg = True;
    if (_contract != NULL) {
      tt_message_reply (_contract);
      tttk_message_destroy (_contract);
      _contract = 0;
    }
}

static Tt_message
TTContractCallback (Tt_message  msg,
		    void       *client_data,
		    Tt_message *contract)
{  
     int             iconify;
     char *opString = tt_message_op (msg);
     Tttk_op   op = tttk_string_op (opString);
     tt_free (opString);
#ifdef DEBUG
     printf("Sdtimage: Contract Callback\n");
#endif

     switch (op) {
     case TTME_DISPLAY:
       tt_message_reply (msg);
       tttk_message_destroy (msg);
       break;
#ifdef LATER
     case TTDT_SET_ICONIFIED:
       tt_message_arg_ival (msg, 0, &iconify);
       if (iconify == True)
	 XIconifyWindow (viewer->display, XtWindow (viewer->uimgr->FrameHandle()), 
			 XScreenNumberOfScreen (XtScreen (viewer->uimgr->FrameHandle())));
       else {
	 XtPopup (viewer->uimgr->FrameHandle(), XtGrabNone);
	 XMapWindow (viewer->display, 
		     XtWindow (viewer->uimgr->FrameHandle()));
       }
       tt_message_reply (msg);
       tttk_message_destroy (msg);
       break;
#endif
     case TTDT_QUIT:
       /*
	*  Received a Quit message from some where.
	*  Fix to check to see if it was from the orig_sender, then quit.
	*/
#ifdef DEBUG
	 fprintf(stderr, "Sdtimage: TTDT_QUIT\n");
#endif
       tt_message_reply (msg);
       tttk_message_destroy (msg);
       _contract = 0;
       ExitCallback();
       return 0;  /* this causes tool to not quit and laucher to time out! */
	 /*
	   case TTDT_GET_STATUS:
	   fprintf(stderr, "Sdtimage: TTDT_GET_STATUS\n");
	   break;
	   case TTDT_PAUSE:
	   fprintf(stderr, "Sdtimage: TTDT_PAUSE\n");
	   break;
	   case TTDT_RESUME:
	   fprintf(stderr, "Sdtimage: TTDT_RESUME\n");
	   break;
	   case TTDT_DO_COMMAND:
	   fprintf(stderr, "Sdtimage: TTDT_DO_COMMANT\n");
	   break;
	   */
       default:
#ifdef DEBUG
	 fprintf(stderr, "Sdtimage: op = %d\n", op);
#endif
       break;
     }
     
     return msg;

}


static Tt_message 
TTMessageCallback (Tt_message     msg,
		   void          *client_data,
		   Tttk_op        op,
		   Tt_status      diagnosis,
		   unsigned char *contents,
		   int            len, 
		   char  	 *file,
		   char 	 *docname)
{
     Tt_status    status;
     Tt_pattern  *contract_pat;
     int          w = INT_MAX, h = INT_MAX, x = INT_MAX, y = INT_MAX;
     static int	  attempt = 0;
     char	  tmpfilename [MAXPATHLEN];
     char        *type = (char *) client_data;
     int	  write_status, open_status;
     TypeInfo    *file_type;

     if (diagnosis != TT_OK) {
       /*
	* Toolkit detected an error.
	*/
       if (tt_message_status (msg) == TT_WRN_START_MESSAGE) {
	 /*
	  * Error is in start message!  
	  * We really should exit() here, but for now let toolkit
	  * handle the message.
	  */
	 fprintf(stderr, "Sdtimage: Error in start message!\n");
	 return msg;
       }
       /*
	* Let the toolkit handle the error.
	*/
       return msg;
     }


     switch (op) {

     case TTME_DISPLAY:
#ifdef DEBUG
       fprintf(stderr, "Sdtimage: we received a Display message!\n");
#endif
       /*
	* If currently displaying an image, reject this message
	* to start another one.
	*/
       if (_contract != 0 || new_msg == True) {
	 tt_message_reject (msg);
	 return (msg);
       }

       new_msg = True;

       /*
        * (DKenny - 29 May, 1997)
        *
        * Fix for bug 4039282 sdtimage with "Print Preview" window can not be 
        *                     brought up again from dtmail
        *
        * Unset when we get a new connection.
        */
       tt_waiting = 0;

       /*
	* Get the type of file.
	*/
       file_type = str_to_ftype (type);

       /*
	* Passed data in buffer
	*/
       if (contents != NULL) {
	 if (docname == (char *) NULL) {
	   attempt++;
	   sprintf (tmpfilename, "%s%d", catgets (prog->catd, 1, 352, 
						  "Untitled"), attempt);
	   write_status = write_tmpfile (&(prog->datafile), contents, 
					 tmpfilename, len);
	 }
	 else
	   write_status = write_tmpfile (&(prog->datafile), contents, docname,
					 len);
	 if (write_status == -1) {
	   fprintf (stderr, catgets (prog->catd, 1, 353, 
				     "Couldn't create tmp file\n"));
	   status = tttk_message_fail (_contract, TT_DESKTOP_ECANCELED, 0, 1);
	 }
	 
	 open_status = open_newfile (docname, prog->datafile, 
				   (file_type != NULL) ? file_type->type : -1,
				   (char *) NULL, 0);
       }

       /*
        * Passed by filename
	*/
       else if (file != NULL) {
	 open_status = OpenAtStart (file, (file_type != NULL) ? file_type->type : 1);
	 if (open_status < 0 && ps_display->new_canvas == NULL)
	   current_display = image_display;
       }
       
       /*
	* Adopt behavior and certain characteristics from requestor.
	*/ 
       if (open_status >= 0) {
         status = ttdt_sender_imprint_on (0, msg, 0, &w, &h, &x, &y, 
			       XtWidgetToApplicationContext (base->top_level), 
			       10 * TIMEOUT_FACTOR);
/*
         if (status == TT_OK) 
	   PositionFrame (x, y, w, h, base->top_level);
*/

         XtMapWidget (base->top_level);
         if (prog->tt_timer_set == True && current_props->show_palette) {
	   PaletteCallback (NULL, NULL, NULL);
           set_zoom_and_rotate ();
         }
       }
       else
	 new_msg = False;

       XtMapWidget (base->top_level);
       if (open_status < 0)
	  TTBreak ();
       break;
	
     default:
#ifdef DEBUG
       fprintf(stderr, "Sdtimage: we got another type of message, fail it!\n"); 
#endif
       status = tttk_message_fail (msg, TT_DESKTOP_ENODATA, 0, 1);  /* change err later */
       if (status != TT_OK)
	    fprintf (stderr, "Sdtimage.MessageCallback - Unable to fail message %d\n", status);
       msg = 0;
       _contract = 0;
       return 0;
     }

     /*
      * Accept a contract to handle this ToolTalk request.
      * Couldn't get callback to work.
      */
     contract_pat = ttdt_message_accept (msg, NULL,
					 NULL, base->top_level, 1, 1);
     status = tt_ptr_error (contract_pat);
     if (status != TT_OK) {
       fprintf(stderr, "Sdtimage: Bad return value from ttdt_message_accept.\n");
       return msg;
     }

/*
 * Reply to the previous message if there was one.
 */
     if (_contract != NULL) {
       tt_message_reply (_contract);
       tttk_message_destroy (msg);
     }
/*
 * Set new contract.
 */
     _contract = msg;
     if (open_status < 0) {
	tt_message_reply (_contract);
	tttk_message_destroy (msg);
	_contract = 0;
     }
     
     return 0;

}

#ifdef LATER
extern Selection_requestor	 current_sel;
extern Dnd                       current_dnd_object;

struct itimerval		 timeout;
Tt_message			 tt_msg = NULL;  /* Message we receive 	  */

Notify_value
quit_after_sleep ()
{
    prog->tt_started = FALSE;
    xv_destroy_safe (base_window->base_window);
}

/*
 * Reply to last message so don't fail.
 */

void
last_tt_reply ()
{
    if (tt_msg != NULL) {
       dstt_set_status (tt_msg, dstt_status_user_request_cancel);
       tt_message_reply (tt_msg);
       tt_message_destroy (tt_msg);
       tt_msg = NULL;
       }
}

void
set_tt_timer ()
{
    timeout.it_value.tv_sec = 180;
    timeout.it_value.tv_usec = 0;
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_usec = 0;

    notify_set_itimer_func (base_window->base_window, quit_after_sleep,
			    ITIMER_REAL, &timeout, NULL);

    if (prog->tt_sender != (char *) NULL) {
       last_tt_reply ();
       free (prog->tt_sender);
       prog->tt_sender = (char *) NULL;
       } 

   prog->tt_timer_set = TRUE;
}

void
turnoff_tt_timer ()
{
    notify_set_itimer_func (base_window->base_window, NOTIFY_FUNC_NULL,
                            ITIMER_REAL, NULL, NULL);
}

status_t
load_from_tt (ttmsg, media_type, type, tt_contents, size, messageid, title)
Tt_message 	 ttmsg;
char		*media_type;
Data_t		 type;
void 		*tt_contents;
int 		 size;
char		*messageid;
char		*title;
{
    int		 		 file_type = -1;
    int	 	 		 status;
    char			*sender;
    int		 		 i;
    int				 write_status;
    Selection_requestor		 tt_sel = NULL;
    static int			 attempt = 0;
    char			 tmpfilename [MAXPATHLEN];

/*
 * If we weren't started via tooltalk, reject the message since the
 * user may not want a new image blowing away the one he's currently
 * viewing.
 */

    if (prog->tt_started == FALSE)
       return (REJECT);

    if (prog->tt_sender == (char *) NULL) 
       prog->tt_sender = strdup (tt_message_sender (ttmsg));
    else if (strcmp (prog->tt_sender, tt_message_sender (ttmsg)) != 0)
       return (REJECT);

    for (i = 0; i < ntypes; i++)
	if ((all_types[i].media_type != (char *) NULL) &&
            (strcmp (media_type, all_types[i].media_type) == 0)) {
           file_type = i;
	   break;
	   }

    if (file_type == -1)
       return (FAIL);

    turnoff_tt_timer ();

    if (prog->verbose == TRUE)
       fprintf (stderr, "%s started with tooltalk, type is: %d\n",
			prog->name, type);

    if (type == path) 
       status = open_newfile (NULL, tt_contents, tt_contents, file_type, 
			      (char *) NULL, 0);
    else if (type == contents) {
       if (title == (char *) NULL) {
	  sprintf (tmpfilename, "%s%d", LGET ("Untitled"), attempt);
	  attempt++;
	  write_status = write_tmpfile (&(prog->datafile), tt_contents, 
					tmpfilename, size);
	  }
       else
	  write_status = write_tmpfile (&(prog->datafile), tt_contents, title,
					size);
       if (write_status == -1) {
          fprintf (stderr, MGET ("Couldn't create tmp file\n"));
          return (FAIL);
          }
	  
       status = open_newfile (NULL, title, prog->datafile, file_type, 
			      (char *) NULL, 0);
       }
    else if (type == x_selection) {
       prog->tt_load = TRUE;
       tt_sel = xv_get (base_window->base_window, XV_KEY_DATA, SELECTOR);
       xv_set (tt_sel, SEL_RANK_NAME, (char *) tt_contents, NULL);
       current_sel = tt_sel;
       current_dnd_object = NULL;
       load_from_dragdrop ();
       status = 0;
       }
    else  /* can't figure out the type */
       return (FAIL);

    if (status != 0 && prog->verbose) {
       fprintf (stderr, MGET ("Error loading image from tt message\n"));
       return (FAIL);
       }

/*
 * Everything is OK, so let's reply to the old message.
 */

    if (tt_msg != NULL) {
       tt_message_reply (tt_msg);
       tt_message_destroy (tt_msg);
       }

    tt_msg = ttmsg;
    xv_set (base_window->base_window, FRAME_CLOSED, FALSE, NULL);
    dstt_status (prog->tt_sender, 
		 (char *) dstt_set_status (NULL, dstt_status_req_rec),
		 messageid, setlocale (LC_CTYPE, NULL));
    return (HOLD);
}

/*
 * Other application is quitting. Set timer in case sdtimage 
 * might be started again soon.
 */

status_t
quit_tt (ttmsg, silent, force, msgID)
Tt_message 	 ttmsg; 
int		 silent;
int 		 force; 
char 		*msgID;
{
    Frame	frame;
    int		n = 1;

    set_tt_timer ();

    xv_set (base_window->base_window, XV_SHOW, FALSE, NULL);
    while (frame = xv_get (base_window->base_window, FRAME_NTH_SUBFRAME, n++))
       xv_set (frame, FRAME_CMD_PUSHPIN_IN, FALSE,
                      XV_SHOW,              FALSE,
                      NULL);

    return (OK);
}

void
break_tt ()
{
    if (prog->tt_sender != (char *) NULL)
       free (prog->tt_sender);

    prog->tt_sender = (char *) NULL;
    prog->tt_started = FALSE;
    if (tt_msg != NULL) {
       dstt_set_status (tt_msg, dstt_status_user_request_cancel);
       tt_message_reply (tt_msg);
       tt_message_destroy (tt_msg);
       tt_msg = NULL;
       }
}

void
respond_to_tt (status)
int	status;
{

/*
 * If status is true, then we got everything OK. If false,
 * then not ok, so respond to the message with a FAIL.
 */

    if ((status == FALSE) && (tt_msg != NULL)) {
       dstt_message_fail (tt_msg, dstt_status_data_not_avail);
       tt_msg = NULL;
       }
       
}

void
save_file_tt ()
{
    dstt_saved (DGET ("File"), current_image->file);
}

#endif
