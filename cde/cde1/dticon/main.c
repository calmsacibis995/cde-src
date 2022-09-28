/* $XConsortium: main.c /main/cde1_maint/2 1995/08/29 19:49:49 gtsang $ */
/*********************************************************************
*  (c) Copyright 1993, 1994 Hewlett-Packard Company
*  (c) Copyright 1993, 1994 International Business Machines Corp.
*  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
*  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
*      Novell, Inc.
**********************************************************************/
/*-----------------------------------------------------------*/
/* This is the project main program file.                    */
/* You may add application dependent source code             */
/* at the appropriate places.                                */
/*-----------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/param.h>

#include <X11/Intrinsic.h>
#include <Xm/Protocols.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Dt/EnvControl.h>
#include "externals.h"
#include "main.h"

#ifdef __TOOLTALK
#include <Tt/tttk.h>
int ttMark;
int tt_tmpfile_fd = -1;
static int undeclared = 0;
static Tt_message local_msg = (Tt_message) 0;
void DieFromToolTalkError( );
Tt_message ProcessToolTalkMessage( );
Tt_message ProcessToolTalkMediaMessage( );
extern void ProcessAppArgs();
void send_tt_saved();
#define dticon_ptype "DT_Icon_Editor"
#endif

#define ERROR           -1
#define NO_ERROR        0

static XtResource resources[] = {
  {"useMessaging", "UseMessaging", XmRBoolean, sizeof (Boolean),
    XtOffset (ApplicationDataPtr, useBMS), XmRImmediate, (XtPointer)False,
  },
  {"session", "Session", XmRString, sizeof (char *),
    XtOffset (ApplicationDataPtr, session), XmRImmediate, (XtPointer)NULL,
  },
  {"bmSuffix", "BmSuffix", XmRString, sizeof (char *),
    XtOffset (ApplicationDataPtr, bmSuffix), XmRImmediate, (XtPointer)".bm",
  },
  {"pmSuffix", "PmSuffix", XmRString, sizeof (char *),
    XtOffset (ApplicationDataPtr, pmSuffix), XmRImmediate, (XtPointer)".pm",
  },
  {"maxIconWidth", "MaxIconWidth", XmRInt, sizeof (int),
    XtOffset (ApplicationDataPtr, maxIconWidth), XmRImmediate,
    (caddr_t)MAX_ICON_WIDTH,
  },
  {"maxIconHeight", "MaxIconHeight", XmRInt, sizeof (int),
    XtOffset (ApplicationDataPtr, maxIconHeight), XmRImmediate,
    (caddr_t)MAX_ICON_HEIGHT,
  },
  {"useFileFilter", "UseFileFilter", XmRBoolean, sizeof (Boolean),
    XtOffset (ApplicationDataPtr, useFileFilter), XmRImmediate, (XtPointer)True,
  },
  {"useFileLists", "UseFileLists", XmRBoolean, sizeof (Boolean),
    XtOffset (ApplicationDataPtr, useFileLists), XmRImmediate, (XtPointer)True,
  },
};

static XrmOptionDescRec option_list[] =
{
   {"-noMessaging", "useMessaging", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-session", "session", XrmoptionSepArg, NULL},
   {"-bmSuffix", "bmSuffix", XrmoptionSepArg, NULL},
   {"-pmSuffix", "pmSuffix", XrmoptionSepArg, NULL},
   {"-maxIconWidth", "maxIconWidth", XrmoptionSepArg, (caddr_t)MAX_ICON_WIDTH},
   {"-maxIconHeight", "maxIconHeight",XrmoptionSepArg,(caddr_t)MAX_ICON_HEIGHT},
   {"-noFileFilter", "useFileFilter", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-noFileLists", "useFileLists", XrmoptionNoArg, (caddr_t)"FALSE"},
   {"-useFileFilter", "useFileFilter", XrmoptionNoArg, (caddr_t)"TRUE"},
   {"-useFileLists", "useFileLists", XrmoptionNoArg, (caddr_t)"TRUE"},
};


extern Widget mainForm, iconForm, fileMenu_quit_pb;
extern void GetSessionInfo();
extern char start_file[];
extern char dummy[];


/*-----------------------------------------------------------*/
/* Insert application global declarations here               */
/*-----------------------------------------------------------*/

XtAppContext AppContext;
Widget       TopLevel;
char        *execName, *progName;
char msgID[120];




/*-----------------------------------------------------------*/
/* User has selected window manager close button... verify   */
/* close desired if data has not been saved.                 */
/*-----------------------------------------------------------*/
static void
#ifdef _NO_PROTO
CloseCB ( w, clientData, callbackArg )
        Widget w ;
        XtPointer clientData ;
        XtPointer callbackArg ;
#else
CloseCB (
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
#endif /* _NO_PROTO */
{
    DialogFlag = QUIT;
    XtCallCallbacks (fileMenu_quit_pb, XmNactivateCallback, NULL);
}


/*-----------------------------------------------------------*/
/* Session is ending, save session information               */
/*-----------------------------------------------------------*/
static void
#ifdef _NO_PROTO
SaveSessionCB ( w, clientData, callbackArg )
        Widget w ;
        XtPointer clientData ;
        XtPointer callbackArg ;
#else
SaveSessionCB (
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
#endif /* _NO_PROTO */
{
#ifdef DEBUG
  if (debug)
    stat_out("SaveSessionCB\n");
#endif

    SaveSession();
}



/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
/* main                                                      */
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
int
#ifdef _NO_PROTO
main( argc, argv )
        int argc ;
        char *argv[] ;
#else
main(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
        /*-----------------------------------------------------------*/
        /* Declarations.                                             */
        /*-----------------------------------------------------------*/
        Widget  w, d;
        int n;
        Arg args[20];
        Atom xa_WM_DELETE_WINDOW, xa_WM_SAVE_YOURSELF;
        char *tmpPtr=NULL;
        static char *untitledStr = NULL;
        char *p=NULL;
        char *tmpp=NULL;

#ifdef __TOOLTALK
    int ttFd;
    char * procId;
    Tt_status ttRc;
    char * sessionString;
#endif

        /*-----------------------------------------------------------*/
        /* Interface function declaration                            */
        /*-----------------------------------------------------------*/
        Widget  create_dtIconShell();
        Widget  create_newIconDialog();
        Widget  create_queryDialog();
        Widget  create_stdErrDialog();
        Widget  create_fileIODialog();
        void    GetMarginData();


        /* ------------------------*/
        /*      Initialize program      */
        /*------------------------------*/
        _DtEnvControl(DT_ENV_SET); /* set up environment variables */

        setlocale(LC_ALL, "");

        execName = argv[0];
        if (progName=strrchr(argv[0], '/'))
            progName++;
        else
            progName = argv[0];


        TopLevel = XtAppInitialize(&AppContext, CLASS_NAME,
                                option_list, XtNumber(option_list),
                                &argc, argv, NULL, NULL, NULL);

        XtGetApplicationResources(TopLevel, &xrdb, resources,
                                      XtNumber(resources), NULL, 0);

        GetSessionInfo();



        /*------------------------------------------------------------*/
        /*    Insert initialization code for your application here    */
        /*------------------------------------------------------------*/


        /*------------------------------------------------------------------*/
        /*  Create and popup the first window of the interface.  The        */
        /*  return value can be used in the popdown or destroy functions.   */
        /*------------------------------------------------------------------*/
        w = create_dtIconShell();
        d = create_fileIODialog();
        d = create_newIconDialog();
        d = create_queryDialog();
        d = create_stdErrDialog();
        Create_Gfx_Labels(Foreground, Background);
        AssignHelpCallbacks();

        XtRealizeWidget (TopLevel);
        XtMapWidget (TopLevel);

        GetMarginData();

        /* */
        /*  Since I can't change the .msg files I'll have to force
            it here.
        */
    last_fname[0] = '\0';
    if (!untitledStr)
      untitledStr = GETSTR(2,20, "UNTITLED");
    strcpy(last_fname, untitledStr);
    strcat(last_fname, ".m.pm");
    ChangeTitle();
        /* */

        ParseAppArgs(argc, argv);
        if (start_file[0] != '\0')
        {
           /* strip off path portion of name if there is one */
           tmpPtr = strrchr(start_file,'/');
           if (tmpPtr != NULL)
               XSetIconName(dpy, XtWindow(dtIconShell), tmpPtr+1);
           else
               XSetIconName(dpy, XtWindow(dtIconShell), start_file);
        }
        n = 0;
        XtSetArg(args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
        XtSetValues(mainForm, args, n);
        XtSetValues(iconForm, args, n);

        /*------------------------------------------------------------------*/
        /*   Catch window close event and call "File...Exit" callback.      */
        /*------------------------------------------------------------------*/
        xa_WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XmAddWMProtocolCallback(dtIconShell, xa_WM_DELETE_WINDOW, CloseCB,
                                NULL);

        /*------------------------------------------------------------------*/
        /*   Register for save and restore.                                 */
        /*------------------------------------------------------------------*/
        xa_WM_SAVE_YOURSELF = XInternAtom(dpy, "WM_SAVE_YOURSELF", False);
        XmAddWMProtocolCallback(dtIconShell, xa_WM_SAVE_YOURSELF,
                                SaveSessionCB, NULL);

#ifdef __TOOLTALK
    ttMark = tt_mark( );
    p = (char *)getenv("TT_SESSION");
    if (!p || strlen(p)==0) {
    tt_default_session_set( tt_X_session( XDisplayString( dpy )));}
    procId = ttdt_open( &ttFd, "dticon", "CDE", "1.0", 1 );
    ttRc = ttmedia_ptype_declare( dticon_ptype, 0,
                                  ProcessToolTalkMediaMessage, NULL, 0);
    DieFromToolTalkError( "ttmedia_ptype_decalare", tt_int_error( ttFd ));
    ttRc = ttmedia_ptype_declare( dticon_ptype, 1000,
                                  ProcessToolTalkMediaMessage, NULL, 0);
    DieFromToolTalkError( "ttmedia_ptype_decalare", tt_int_error( ttFd ));
    ttRc = ttmedia_ptype_declare( dticon_ptype, 2000,
                                  ProcessToolTalkMediaMessage, NULL, 1);
    DieFromToolTalkError( "ttmedia_ptype_decalare", tt_int_error( ttFd ));
    tttk_Xt_input_handler(NULL, 0, 0);
    ttRc = tt_ptr_error(ttdt_session_join( tt_default_session( ),
                                          ProcessToolTalkMessage,
                                          (Widget) 0, NULL, 1));
    DieFromToolTalkError( "ttdt_session_join", ttRc );
    XtAppAddInput( AppContext, ttFd, (XtPointer)XtInputReadMask,
                        tttk_Xt_input_handler, 0 );
    if (!undeclared) tt_ptype_undeclare(dticon_ptype);
#endif


        /*-------------------------*/
        /*   Enter the event loop  */
        /*-------------------------*/
        {
            XEvent event;

            for (;;) {
                XtAppNextEvent(AppContext, &event);
                if ((GraphicsOp == S_GRAB) &&
                    ((event.type == ButtonPress)  ||
                     (event.type == MotionNotify) ||
                     (event.type == ButtonRelease)))
                  Do_ButtonOp(&event);
                else
                {
                    if ((GraphicsOp == POLYGON || GraphicsOp == POLYLINE) &&
                          (event.type == ButtonPress))
                    {
                        if (event.xbutton.window != tablet_win)
                            EndPolyOp();
                    }
                    XtDispatchEvent(&event);
                }
            }
        }
}

#ifdef __TOOLTALK

void
DieFromToolTalkError( char * procName, Tt_status errId )
{
  if( tt_is_err( errId ) )
  {
    fprintf( stderr, "%s returned ToolTalk error: %s\n", procName,
                        tt_status_message( errId ) );
    exit( -1 );
  }
}


#ifdef _NO_PROTO
Tt_message ProcessToolTalkMediaMessage( msg, cdata, op, diag,
                                        contents, len, file, doc)
        Tt_message      msg;
        void*           cdata;
        Tttk_op         op;
        Tt_status       diag;
        unsigned char*  contents;
        int             len;
        char*           file;
        char*           doc;
#else
Tt_message ProcessToolTalkMediaMessage( Tt_message      msg,
                                        void*           cdata,
                                        Tttk_op         op,
                                        Tt_status       diag,
                                        unsigned char*  contents,
                                        int             len,
                                        char*           file,
                                        char*           doc)
#endif
{
int mark = tt_mark();
int tmplen=0;
int tt_tmpMaskfile_fd = -1;
char mask_file[MAX_FNAME];
char *tmp = NULL;
char *tmp1 = NULL;

if (tt_message_status(msg) == TT_WRN_START_MESSAGE) tt_message_reply(msg);


  switch(op) {
    case TTME_COMPOSE:
        /*
         * At this time, we don't handle composing new icons.
         * Fail any such requests.
         */
        tt_message_fail(msg);
        break;
    case TTME_DISPLAY:
        /*
         * At this time, we don't handle displaying icons
         * without editing them.
         * Fail any such requests.
         */
        tt_message_fail(msg);
        break;
    case TTME_EDIT:

        /*
         * Undeclare the ptype so that this process will
         * not service any more TT requests.
         */

        tt_ptype_undeclare(dticon_ptype);
        undeclared = 1;

        if (tt_message_status(msg) != TT_WRN_START_MESSAGE)
        {
          /*
           * A message came in while we were already up
           * and running -- reject it so that another dticon
           * process will be started up to service this
           * request.
           */

          tt_message_reject(msg);
        }
        else if (edit_notifier(NULL, msg, 0))
        {
           strcpy( msgID, tt_message_arg_val(msg, 1) );

           if (file)
           {
                   /*
                    * Icon data is in a file
                   */
                   strcpy(start_file, file);
           }
           else
           {
             /*
              * Icon data is in a buffer -- write it
              * to a /tmp file.
             */
             unsigned char* buf;
             int blen;
             int wlen;

             if (tt_message_arg_bval(msg, 0,
                                     &buf, &blen) != TT_OK)
             {
                tt_message_fail(msg);
                edit_notifier(NULL, 0, 1);
             }
             else
             {
                (void) tmpnam(start_file);
                if( (buf) && (!strncmp((char *)buf, "/* XPM */", 9)) )
                {
                   /* Format XPM */
                   tt_tmpfile_fd =
                           open(start_file,
                                O_CREAT | O_WRONLY | O_NDELAY,
                                0666);
                   if (tt_tmpfile_fd && (wlen =
                                 write(tt_tmpfile_fd,
                                       buf, blen)) == blen)
                   {
                      (void) close(tt_tmpfile_fd);
                      param_flag[AUTO_FILE] = True;
                      argsNeedProcessed = True;
                   }
                   else {
                           tt_message_fail(msg);
                           edit_notifier(NULL, 0, 1);
                        }
                }
                else
                {
                   /* Format XBM */
                   /* assume two buffers.... */
                   tmp = strchr((char *)buf, ';');
                   tmp ++;
                   tmplen=blen - strlen(tmp);

                     /* read XBM file first*/
                     /* Grab the first buffer. */
                     tt_tmpfile_fd =
                             open(start_file,
                                  O_CREAT | O_WRONLY | O_NDELAY,
                                  0666);

                     if (tt_tmpfile_fd && (wlen =
                                   write(tt_tmpfile_fd,
                                         buf, tmplen)) == tmplen)
                     {
                        (void) close(tt_tmpfile_fd);
                        param_flag[AUTO_FILE] = True;
                        argsNeedProcessed = True;
                     }
                     else
                     {
                             tt_message_fail(msg);
                            edit_notifier(NULL, 0, 1);
                     }

                   /* Try to find the second buffer. */
                   tmp ++;
                   if(tmp) tmp1=strchr(tmp, ';');

                   if (tmp1)
                   {
                      /* we have a mask */
                      /* construct the name of the mask file */
                      strcpy(mask_file, start_file);
                      strcat(mask_file, "_m\0");
                      tt_tmpMaskfile_fd =
                            open(mask_file,
                            O_CREAT | O_WRONLY | O_NDELAY,
                            0666);
                      /* Read the mask fro the seconf buffer. */
                      if (tt_tmpMaskfile_fd && (wlen =
                      write(tt_tmpMaskfile_fd,
                            tmp, strlen(tmp))) == strlen(tmp))
                      {
                         (void) close(tt_tmpMaskfile_fd);
                         param_flag[AUTO_FILE] = True;
                         argsNeedProcessed = True;
                      }
                      else {
                              tt_message_fail(msg);
                              edit_notifier(NULL, 0, 1);
                           }
                   }
                } /* else XBM Format */
             }
           }
        }
    break;
  }

  tt_release(mark);
  return (Tt_message) 0;
}

int
#ifdef _NO_PROTO
edit_notifier(fname, msg, clear)
        char* fname;
        Tt_message msg;
        int clear;
#else
edit_notifier(char* fname, Tt_message msg, int clear)
#endif
{
        /*
         * Triple-mode function that either (1) saves the message
         * so it can be replied to later, (2) replies to a
         * previously-saved message, or (3) clears its state.
         */

        if (clear) {
                if (local_msg != (Tt_message) 0) {
                        tt_message_reply(local_msg);
                        tt_message_destroy(local_msg);
                }
                local_msg = (Tt_message) 0;

                return 1;
        }

        if (fname && !msg) {

                /*
                 * Not currently used, DELETE THIS COMMENT if it
                 * is ever needed.
                 */

                if (local_msg != (Tt_message) 0) {
                        /*
                         * Set the filename arg to the new, passed-in
                         * file name, and reply to the message.
                         */
                        tt_message_arg_val_set(local_msg, 0, fname);
                        tt_message_reply(local_msg);
                        local_msg = (Tt_message) 0;
                }
                return 1;
        }
        else if (!fname && msg) {
                /*
                 * Just store the message for a later reply.  If
                 * we already have a stored message, return a failure
                 * status (0) and reject the incoming message.
                 */
                if (local_msg != (Tt_message) 0) {
                        tt_message_reject(msg);
                        return 0;
                }
                else {
                        local_msg = msg;
                        return 1;
                }
        }
        else {
                /*
                 * The caller is confused.
                 */
                tt_message_reject(msg);
                return 0;
        }
}


void
send_tt_saved()
{
        int mark;
        int fd;
        int fdm;
        int len=0;
        int Mlen=0;
        int rlen=0;
        char* buffer=NULL;
        char* Fbuffer=NULL;
        char* Mbuffer=NULL;
        struct stat statbuf;        /* Information on a file. */
        Tt_status ttstat;
        Tt_message msg;

        mark = tt_mark();
        msg = tt_message_create();
        tt_message_address_set(msg, TT_HANDLER);
        tt_message_handler_set(msg, tt_message_sender(local_msg));
        tt_message_class_set(msg, TT_NOTICE);
        tt_message_scope_set(msg, TT_SESSION);
        tt_message_session_set(msg, tt_default_session());
        tt_message_op_set(msg, "Saved");

        /* Read the base file regardless of format */
        fd = open(last_fname, O_RDONLY | O_NDELAY);
        len = lseek(fd, 0, SEEK_END);
        (void) lseek(fd, 0, SEEK_SET);
        Fbuffer = XtMalloc(len + 1);
        if ((rlen = read(fd, Fbuffer, len)) != len)
        {
        /* didn't read whole file! */
        printf("dtcreate: Only read %d of %d bytes of icon file!\n", rlen, len);
        }
        (void) close(fd);
        Fbuffer[len]='\0';

        rlen = 0;
        /* Handle the XBM format */
        if( (fileFormat != FORMAT_XPM) && (stat(dummy, &statbuf) == 0) )
        {
           /* Read the mask file if one exists */
           fdm = open(dummy, O_RDONLY | O_NDELAY);
           Mlen = lseek(fdm, 0, SEEK_END);
           (void) lseek(fdm, 0, SEEK_SET);
           Mbuffer = XtMalloc(Mlen + 1);
           if ((rlen = read(fdm, Mbuffer, Mlen)) != Mlen)
           {
            /* didn't read whole file! */
            printf("dtcreate: Only read %d of %d bytes of icon file!\n", rlen, Mlen);
           }
           (void) close(fdm);
           Mbuffer[Mlen] ='\0';

           /* Double buffer the base and mask */
           buffer = XtMalloc(strlen(Fbuffer)+strlen(Mbuffer)+1);
           strcpy(buffer, Fbuffer);
           buffer[strlen(Fbuffer)] ='\0';
           strcat(buffer, Mbuffer);
           buffer[strlen(Fbuffer)+strlen(Mbuffer)+1] ='\0';
        }
        else
        {
           /* No mask/bm just pass the pm file */
             buffer = XtMalloc(len + 1);
             strcpy(buffer, Fbuffer);
             buffer[len+1] ='\0';
        }

        XtFree(Fbuffer);
        XtFree(Mbuffer);
        tt_message_barg_add(msg, TT_IN, tt_message_arg_type(local_msg, 0),
                            (void *) buffer, Mlen+len+1);
        XtFree(buffer);

        tt_message_arg_add(msg, TT_IN, "string", msgID);

        ttstat = tt_message_send(msg);

        tt_message_destroy(msg);
}


#ifdef _NO_PROTO
Tt_message ProcessToolTalkMessage(msg, cdata, contract)
        Tt_message msg;
        void* cdata;
        Tt_message contract;
#else
Tt_message ProcessToolTalkMessage(Tt_message msg,
                                  void* cdata,
                                  Tt_message contract)
#endif
{
        int mark = tt_mark();
        char* op = tt_message_op(msg);

        if (!strcmp(op, "Quit")) {
                /*
                 * Do any necessary cleanup here, call
                 * tt_message_reply(), and quit.
                 * Since this is not currently implemented here,
                 * the message will just be failed for now.
                 */
                tt_message_fail(msg);
        }
        /*
         * The messages that must interact with the X environment
         * are not supported currently.  If a realized,
         * mappedWhenManaged top-level widget were passed to
         * ttdt_session_join(), they would be handled automatically.
         * Since that is not the case, they are passed to this
         * callback, and could be implemented here if desired
         * (if so, be sure to change tt_message_reject to
         * tt_message_reply!).
         */
        else {
                tt_message_reject(msg);
        }

        tt_release(mark);
        return (Tt_message) 0;
}

#endif


/******************************************************************************
NAME:           handle_dialog_child( wgt, manage_func )

INPUT:          Widget  wgt                     - the dialogShellWidget
                void    (*manage_func)()        - either XtManageChild
                                                  or XtUnmanageChild

RETURN:         int                             - NO_ERROR if successfull
                                                  ERROR otherwise

DESCRIPTION:    Handles the popping up or popping down of dialog shells
                by managing or unmanaging their children.

CREATION:       Visual Edge Software            Sept 19/91
-----------------------------------------------------------------------------*/
#ifdef _NO_PROTO
static  int     handle_dialog_child( wgt, manage_func )
        Widget  wgt;
        void    (*manage_func)();
#else
static  int     handle_dialog_child( Widget wgt, void (*manage_func)() )
#endif
{
        int     i, num_children;
        Widget  *children;

        XtVaGetValues( wgt,
                       XmNnumChildren, &num_children,
                       XmNchildren, &children,
                       NULL );

        /* We manage/unmanage the first rectObj child in the list.
         * Note that the check for rectObjClass is necessary since some
         * implementations of Motif add protocol children to the dialogShell.
         */

        for (i = 0; i < num_children; i++)
        {
                if ( XtIsSubclass( children[i], rectObjClass ) )
                {
                        (*manage_func)(children[i]);
                        return ( NO_ERROR );
                }
        }

        return ( ERROR );
}

/******************************************************************************
NAME:           PopupInterface( wgt, grab_flag )

INPUT:          Widget          wgt             - Widget to popup
                XtGrabKind      grab_flag       - grab flag

RETURN:         int                     ERROR or NO_ERROR

DESCRIPTION:    Popups up an interface. The widget should be a toplevel widget.
                Note that special handling is required for dialogShells since
                those are popped up by managing their children if they have
                some.
                The grab_flag could be any of:
                                no_grab (XtGrabNone)
                                nonexclusive_grab (XtGrabNonexclusive)
                                exclusive_grab (XtGrabExclusive)

CREATION:       Visual Edge Software            April 6 1991
-----------------------------------------------------------------------------*/
#ifdef _NO_PROTO
int     PopupInterface( wgt, grab_flag )
        Widget          wgt;
        XtGrabKind      grab_flag;
#else
int     PopupInterface( Widget wgt, XtGrabKind grab_flag )
#endif
{

        if ( XtIsSubclass( wgt, xmDialogShellWidgetClass ) )
        {
                if ( handle_dialog_child( wgt, XtManageChild ) == ERROR )
                        XtPopup( wgt, grab_flag );
        }
        else
        {
                XtPopup( wgt, grab_flag );
        }

        return ( NO_ERROR );
}
