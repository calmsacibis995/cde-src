/*
 *+SNOTICE
 *
 *	$Revision: 1.167 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993,1994,1995,1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamApp.C	1.167 05/08/96"
#endif

#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#ifdef DTMAIL_TOOLTALK
#include <Tt/tt_c.h>
#include <Tt/tttk.h>
#include "SendMsgDialog.h"
#endif

#if defined(POSIX_THREADS)
#include <thread.h>
#endif

#include "RoamApp.h"
#include "RoamMenuWindow.h"
#include "RoamCmds.h"
#include "WorkingDialogManager.h"
#include <DtMail/OptCmd.h>
#include "DtMailGenDialog.hh"
#include <DtMail/DtMailError.hh>
#include <DtMail/IO.hh>
#include <DtMail/Common.h>

#include "EUSDebug.hh"
#include "MemUtils.hh"
#include "MailMsg.h"
#include "DtMailHelp.hh"
#include "ComposeCmds.hh"
#include "Fonts.h"

#include <Xm/MessageB.h>
#include <DtMail/DtMailSigChld.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <Dt/Dt.h>
#include <Dt/Dts.h>
#include <Dt/Action.h>
#include <Dt/EnvControl.h>

int use_XmTextEditor = 0;

void
force( Widget w)
{
  Widget	shell;
  Display	*dpy;
  XWindowAttributes	xwa;
  Window	window;
  XtAppContext cxt=XtWidgetToApplicationContext( w );
  XEvent		event;

  shell=w;
  dpy=XtDisplay(shell);
  window=XtWindow( shell );

  while ( XGetWindowAttributes(dpy,window,&xwa)) {
    if ( XGetWindowAttributes( dpy, window, &xwa ) &&
	 xwa.map_state != IsViewable )
      break;

    XtAppNextEvent( cxt, &event );
    XtDispatchEvent( &event );
  }
  XmUpdateDisplay(shell);
}

void
forceUpdate( Widget w )
{
  Widget diashell, topshell;
  Window diawindow, topwindow;
  XtAppContext cxt = XtWidgetToApplicationContext( w );
  Display		*dpy;
  XWindowAttributes	xwa;
  XEvent		event;

  for (diashell=w;!XtIsShell(diashell);diashell=XtParent(diashell));
  for ( topshell=diashell;XtIsTopLevelShell( topshell );
	topshell = XtParent( topshell ) );

  dpy=XtDisplay(diashell);
  diawindow=XtWindow( diashell );
  topwindow=XtWindow(topshell);
  while ( XGetWindowAttributes(dpy,diawindow,&xwa)  
          && XEventsQueued( dpy, QueuedAlready) ) {
      
      XtAppNextEvent( cxt, &event );
      XtDispatchEvent( &event );
    }
  XmUpdateDisplay(topshell);
}

XtResource
RoamApp::_resources[] = {
  {
    "printscript",
    "PrintScript",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _print_script ),
    XtRString,
    ( XtPointer ) "lp",
  },
  {
    "mailfiles",
    "MailFiles",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _mailfiles_folder ),
    XtRString,
    ( XtPointer ) ".",
  },
  {
    "defaultmailbox",
    "DefaultMailBox",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _default_mailbox ),
    XtRString,
    ( XtPointer ) ".",
  },

  // Fixed width font
  {
    "userFont",
    "UserFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _user_font ),
    XtRString,
    ( XtPointer )"-b&h-lucidatypewriter-medium-r-*-sans-*-120-*-*-*-*-*-*",
  },

  // Fixed width fontlist
  {
    "userFont",
    "UserFont",
    XmRFontList,
    sizeof( XmFontList ),
    XtOffset ( RoamApp *, _user_fontlist ),
    XtRString,
    ( XtPointer )"-b&h-lucidatypewriter-medium-r-*-sans-*-120-*-*-*-*-*-*",
  },

  // Variable width font
  {
    "systemFont",
    "SystemFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _system_font ),
    XtRString,
    ( XtPointer )"-b&h-lucida-medium-r-*-sans-*-120-*-*-*-*-*-*"
  },

  // Variable width fontlist
  {
    "systemFont",
    "SystemFont",
    XmRFontList,
    sizeof( XmFontList ),
    XtOffset ( RoamApp *, _system_fontlist ),
    XtRString,
    ( XtPointer )"-b&h-lucida-medium-r-*-sans-*-120-*-*-*-*-*-*"
  },

  // Font used for attachment glyph
  {
    "glyphFont",
    "GlyphFont",
    XtRString,
    sizeof( XtRString ),
    XtOffset ( RoamApp *, _glyph_font ),
    XtRString,
    ( XtPointer )
    NULL,
  },

};

RoamApp theRoamApp("Dtmail");
int just_compose = 0;

void
SigUsr1(int)
{
    theRoamApp.setKilledBySignal();
    theRoamApp.closeAllWindows();
}

void

RoamApp::closeAllWindows(void)
{
    // Remove callbacks to prevent XtPhase2Destroy crashes.
    //
    XtRemoveAllCallbacks(baseWidget(), XmNdestroyCallback);
    for (int win = 0; win < _numWindows; win++) {
	_windows[win]->quit();
    }
    
    shutdown();
}

void
RoamApp::statusCallback(DtMailOperationId, DtMailEnv &error, void *)
{
   if (error.isSet()) {
	 // fprintf(stderr, "DEBUG: statusCallback(): Submission failed error = %d\n", error._major);
   } else {
	 // fprintf(stderr, "DEBUG: statusCallback(): Submission done.%s\n");
   }
}

#ifdef DTMAIL_TOOLTALK

int started_by_tt = 0;
// Move this to constructor of RoamMenuWindow???
int dtmail_mapped = 0;    // For explanation, look in RoamApp.h.
static int roam_tt_fd = 0;
char *roam_tt_procid = NULL;

//  Report ToolTalk error
int dieFromTtError(char *, Tt_status errid)
{
    /* Do not die on warnings or TT_OK */

    if ( tt_is_err(errid) ) {
	/*
	fprintf(stderr, catgets(DT_catd, 2, 3, "%s returned ToolTalk error: %s\n"), 
			procname, tt_status_message(errid));
	*/
	ttdt_close(0, 0, 1);
	return 1;   // Returns 1 indicating that error occurred.
    }
	return 0;
}

Tt_message
attachmt_msg_handler(
    Tt_message msg,
    void *client_data,
    Tttk_op op,
    Tt_status diag,
    unsigned char *contents,
    int len,
    char *file,
    char *docname 
)
{
   static const char *thisFcn = "attachmt_msg_handler()";
   Tt_status status = TT_OK;
   SendMsgDialog *compose;

   if ( diag != TT_OK ) {
      // Toolkit detected an error
      // Let toolkit handle error
      return msg;
   }

   ttdt_message_accept(msg, NULL, (Widget)0, client_data, 1, 1);

   if ( op == TTME_MAIL_EDIT ) { 
//       status = tt_message_reply(msg);
//       dieFromTtError("tt_message_reply", status);
       // Put the data that is coming in (via buffer or file) as
       // the attachment of a message.
       if ( len > 0 ) {    // Pass by content
	   compose = theCompose.getWin(msg);
	   compose->inclAsAttmt(contents, len, docname);
       } else if ( file != NULL ) {    // Pass by filename
	   compose = theCompose.getWin(msg);
	   compose->inclAsAttmt(file, docname);
       } else {   // DO NOTHING.
	   // This is not an entry point to bring up a blank compose window
	   // because ttMediaLoadPatCB() returns with diagnosis =
	   // TT_DESKTOP_ENODATA. For INOUT mode, it expects file or buffer.
       }
   } else {
       tttk_message_fail( msg, TT_DESKTOP_ENODATA, 0, 1 );
   }
//
// Not sure if these should be freed.
//
//    tt_free( (caddr_t)contents );
//    tt_free( file );
//    tt_free( docname );
   return 0;
}

//[We expect this to be called only in case of memory allocation errors
void dtmail_xterr_handler(String msg)
{
	DtMailEnv mail_err;

	fprintf(stderr, msg);
	fprintf(stderr, "\n");
	fprintf(stderr, "Mailer: Xt ran out of memory on the system.\n");
	mail_err.logError(DTM_TRUE,
				      "Mailer: Xt ran out of memory on the system.\n");
	fprintf(stderr, "Mailer: DtMail Safe Exiting.\n");
	mail_err.logError(DTM_TRUE,
				      "Mailer: DtMail Safe Exiting.\n");
	fflush(stderr);
	if (theRoamApp.baseWidget() != NULL) {
		XtRemoveAllCallbacks(theRoamApp.baseWidget(), XmNdestroyCallback);
		theRoamApp.shutdown();
	}
	exit(3);
}

void dtmail_xtwrn_handler(String msg)
{
	fprintf(stderr, msg);
	return;
}

Tt_message
tooltalk_msg_handler( 
    Tt_message msg,
    void *client_data,
    Tttk_op op,
    Tt_status diag,
    unsigned char *contents,
    int len,
    char *file,
    char *docname 
)
{
    static const char *thisFcn = "tooltalk_msg_handler()";
    Tt_status status = TT_OK;
    SendMsgDialog *compose;

    if ( diag != TT_OK ) {
	// toolkit detected an error
	// Let toolkit handle error
	return msg;
    }

	XtAppSetErrorHandler(theApplication->appContext(), dtmail_xterr_handler);
	XtAppSetWarningHandler(theApplication->appContext(), dtmail_xtwrn_handler);

    // Need to check the return value of this call.
    //
    ttdt_message_accept(msg, NULL, (Widget)0, client_data, 1, 1);
    if ( op == TTME_MAIL ) {
	// Send without GUI.

	// Construct message handle
	DtMailBuffer mbuf;
	if ( len > 0 ) {
	    mbuf.buffer = (void *)contents;
	    mbuf.size = (unsigned long)len;
	} else if ( file != NULL ) {
	    // 1. Get file content into buffer.
	    int fd = SafeOpen(file, O_RDONLY);
	    if (fd < 0) {
		return msg;
	    }
	    struct stat buf;
	    if (SafeFStat(fd, &buf) < 0) {
		close(fd);
		return msg;
	    }
 
	    mbuf.buffer = new char[buf.st_size];
	    if (!mbuf.buffer) {
		close(fd);
		return msg;
	    }
 
	    if (SafeRead(fd, mbuf.buffer, 
				(unsigned int) buf.st_size) != buf.st_size) {
		delete mbuf.buffer;
		close(fd);
		return msg;
	    }
	    mbuf.size = (unsigned long)buf.st_size;
	}
	DtMailEnv error;
	DtMail::Session *d_session = theRoamApp.session()->session();
	DtMail::Message *msgHandle = d_session->messageConstruct(error,
								 DtMailBufferObject,
								 &mbuf,
								 NULL,
								 NULL,
								 NULL);
	if ( error.isSet() || !msgHandle ) {
	    return msg;
	}

	// Send the message
	theRoamApp.default_transport()->submit(error, msgHandle,
					       (DtMailBoolean)FALSE);

	if (error.isSet()) {
	    tttk_message_fail(msg, TT_DESKTOP_ENOTSUP, 0, 1);
	} else {
	    status = tt_message_reply(msg);
	    dieFromTtError("tt_message_reply", status);
	}
    } else if ( op == TTME_MAIL_COMPOSE ) {
	// Bring up blank compose window.
	status = tt_message_reply(msg);
	dieFromTtError("tt_message_reply", status);
	compose = theCompose.getWin(msg);
	if ( docname ) {
	    compose->title(docname);
	}
    } else if ( op == TTME_MAIL_EDIT ) { 
	// Bring up compose window with given data filled in.
// 	status = tt_message_reply(msg);
// 	dieFromTtError("tt_message_reply", status);
	// Parse data (coming in as buffer or file)
	if ( len > 0 ) {    // Pass by content
	    compose = theCompose.getWin(msg);
	    compose->parseNplace((char *)contents, len);
	    if ( docname ) {
		compose->title(docname);
	    }
	} else if ( file != NULL ) {    // Pass by filename
	    compose = theCompose.getWin(msg);
	    compose->parseNplace(file);
	    if ( docname ) {
		compose->title(docname);
	    }
	} else {
         // DO NOTHING
	 // This is not an entry point to bring up a blank compose window
         // because ttMediaLoadPatCB() returns with diagnosis =
         // TT_DESKTOP_ENODATA. For INOUT mode, it expects file or buffer.
	}
    } else if ( op == TTME_DISPLAY ) {
	// It is the Display message.  
	// Since the compose window can be started independent of RoamMenuWindow,
	// a DtMail process may be around (because of a compose window).  Then if
	// a DtMail process exists (ToolTalk will not start another DtMail process),
	// that process needs to respond to this DISPLAY message. 
	// If for some reason, the view of the specified mail folder is unmapped,
	// then need to map it.  Otherwise, create the view for the specified mail folder.
	// Need to remove self destruct when using an existing DtMail process if that
	// process was started by compose.

// This is the wrong place to reply to this message.  Replying here causes
// the action layer to think that it is ok to remove the file it passed in, but we
// are not done with it yet.  Calling tt_message_reply at the end of this function
// doesn't work either.  We are just not going to reply to this tooltalk message
// (tooltalk folks say this is ok) for the time being.
// 	status = tt_message_reply(msg);
// 	dieFromTtError("tt_message_reply", status);

	if ( theApplication == NULL ) {
#ifdef WM_TT
	    fprintf(stdout, "%s: theApplication is NULL\n", thisFcn);
#endif
	    return 0;
	}

#ifdef WM_TT
	fprintf(stdout, "%s: TTME_DISPLAY\n", thisFcn);
#endif

	if ( theCompose.getTimeOutId() != 0 ) { 
	    XtRemoveTimeOut(theCompose.getTimeOutId());
	}
  	assert(file != NULL);
	RoamMenuWindow *roamwin = new RoamMenuWindow(file, msg);
	roamwin->initialize();
	roamwin->mailboxName(strdup(file));
//	roamwin->manage();
	// Set this to True so Self_destruct() would not be started by Compose.
	dtmail_mapped = 1;
    } else {
	tttk_message_fail( msg, TT_DESKTOP_ENODATA, 0, 1 );
    }
//     tt_free( (caddr_t)contents );
//     tt_free( file );
//     tt_free( docname );
    return 0;
}

Tt_message
quit_message_cb(Tt_message m,
	void *client_data,
	Tt_message)
{
    static const char *thisFcn = "quit_message_cb()";
    int silent, force;
    Tt_status status;

    // int mark = tt_mark();

    char *op = tt_message_op(m);
    if ( tttk_string_op(op) == TTDT_QUIT ) {

	/* silent? */
	status = tt_message_arg_ival(m, 0, &silent);
	if ( status != TT_OK ) {
	    fprintf(stderr, "DEBUG: %s: tt_message_arg_ival(0): %s\n",
		    thisFcn, tt_status_message(status));
	    return m;
	}
	if ( silent ) {
	}

	/* force? */
	status = tt_message_arg_ival(m, 1, &force);
	if ( status != TT_OK ) {
	    fprintf(stderr, "DEBUG: %s: tt_message_arg_ival(1): %s\n",
		    thisFcn, tt_status_message(status));
	    return m;
	}
	if ( force ) {
	    tt_message_reply(m);
	    free(op);
	    // tt_release(mark);
	    RoamApp *app = (RoamApp *)client_data;
	    // Need to make the following call; otherwise, will 
	    // run into Phase2Destroy
	    XtRemoveAllCallbacks(app->baseWidget(), 
				 XmNdestroyCallback);
	    app->shutdown();
	}
	// tt_release(mark);
	return m; 
    } else {
	// tt_release(mark);
	free(op);
	return m;
    }
}
#endif   /* DTMAIL_TOOLTALK */

#ifdef DEAD_WOOD
static void
SigChldHandler(int)
{
    int status;
    int pid = (int) wait(&status);

    ChildExitNotify(pid, status);
}
#endif /* DEAD_WOOD */

void
Usage(char *progname)
{
   printf("Usage:  %s\n", progname);
   printf("[-h]  Help\n");
   printf("[-c]  A blank compose window comes up.\n");
   printf("[-a file1 ... fileN]  Compose window comes up with file1 through fileN as attachments.\n"); 
   printf("[-f mailfile]  The specified mail folder is displayed instead of INBOX.\n");
   printf("[-l]  Start the compose window on a dead letter file.\n");

   // There is also a "-e" option which would run dtmail with the XmTextEditor
   // instead of DtEditor.  Right now it is an undocumented feature.
}

nl_catd DT_catd = (nl_catd) -1;    // catgets file descriptor

void RoamApp::initialize ( int *argcp, char **argv )
{

    // Check to see if this is the vacation start/stop invocation
    if (argcp && *argcp >= 3) {
	if (argv && strcmp(argv[1], "-v") == 0) {

    		_vacation = new VacationCmd("Vacation", catgets(DT_catd, 1, 3, "Vacation"));
		if (strcmp (argv[2], "start") == 0) {
			if (argcp && *argcp == 4)
				_vacation->addVacationToForwardFile(argv[3]);
			else
				_vacation->addVacationToForwardFile(NULL);
		}
		else if (strcmp(argv[2], "stop") == 0)
			_vacation->removeVacationFromForwardFile();
		exit (1);
	  }
    }

    Tt_status status;
    Tt_pattern *tt_pat;
    char **av = argv;
    int	opt;

    _busy_count = 0;
    _killedBySignal = FALSE;
    _options = NULL;
    _optionsHandle = NULL;
    _tempDialog = NULL;
    _vacation = NULL;

    int n = 1;
    char * mail_file = NULL;
    char * dead_letter = NULL;
    char *session_file = NULL;
    char *helpId;

#ifdef hpV4
    signal(SIGUSR1, (void(*)(int ...))SigUsr1);
#else
    signal(SIGUSR1, SigUsr1);
#endif

    _mailview = new DtVirtArray<RoamMenuWindow *>(2);

    // Must be called before XtAppInitialize.
    XtSetLanguageProc(NULL, NULL, NULL);
	XtAppSetErrorHandler(theApplication->appContext(), dtmail_xterr_handler);
	XtAppSetWarningHandler(theApplication->appContext(), dtmail_xtwrn_handler);

    // Set up environment variable (including NLSPATH) before calling 
    // XtAppInitialize() (cmvc 6576). 
    _DtEnvControl (DT_ENV_SET);

    // This will take care of standard arguments such as -display.
    Application::initialize(argcp,argv);

    // If -session arg is present remove it and return session file name. 
    session_file = parseSessionArg(argcp, argv);

    // export display and locale settings

    static char langenv[64];
    static char displayenv[64];

    sprintf(langenv, "LANG=%s", setlocale(LC_ALL, NULL));
    putenv(langenv);

    sprintf(displayenv, "DISPLAY=%s", XDisplayString(_display));
    putenv(displayenv);

    // Process dtmail opt args.

    int num_legit_args = 0;

    while((opt = getopt(*argcp, argv, "a:cf:l:hte")) != EOF) {
	switch (opt) {
	  case 'a':
	    	just_compose = 1;
		num_legit_args++;
		break;
	  case 'c':
		just_compose = 1;
		num_legit_args++;
		break;

	  case 't':
		// Started by ToolTalk
		started_by_tt++;
		num_legit_args++;
		n = 2;
		break;

	  case 'e':
		use_XmTextEditor = 1;
		num_legit_args++;
		break;

	  case 'f':
		mail_file = optarg;
		num_legit_args++;
		break;

	  case 'l':
		just_compose = 1;
		num_legit_args++;
		break;
	  case 'h':
	  default:
		Usage(argv[0]);
		exit (1);
	}
    }

    initDebug();

    if(!just_compose && !started_by_tt && !mail_file && session_file)
    	openSessionFile(session_file); // Open the session file

    MdbgChainCheck();
    getResources( _resources, XtNumber ( _resources ) );

    DtInitialize(XtDisplay(baseWidget()), baseWidget(), argv[0], "Dtmail");

    // Must be called after XtSetLanguageProc and DtInitialize because DtInitialize 
    // sets the environment variable NLSPATH which is used in catopen(). That's why
    // we have to take out catopen from Application::initialize and put a new mathod
    // open->catalog for both Application and RoamApp
    this->open_catalog();

    // Initialize the mail_error. This also has to be done after DtInitialize 
    // because the DtMailEnv consturctor calls catopen as well.
    DtMailEnv mail_error;
    mail_error.clear();

    DtDbLoad();


    //
    // Some of the scrolling lists contain formatted text and thus
    // require a fixed width font to display the text correctly.
    // We want to use the fixed width font defined by dt.userFont as
    // it will be internationalized correctly.  If it is not there
    // we fall back to a fixed width lucida font.  If that fails
    // we let the widget fallback to what ever it thinks is best.
    //

    // Default font in case we can't find anything else.
    XrmDatabase db = XtScreenDatabase(XtScreen(baseWidget()));

    if (db) {
	FontType	userfont;
	char		*glyphname = NULL;
        char		buf[1024];


	// Need to check fixed width font mailrc variable.  Assume
	// True for now
#if 0
	if (True) {
#endif

		XrmPutStringResource(&db, "*Work_Area*Text*fontList",
							 _user_font);
		XrmPutStringResource(&db, "*Work_Area*iconGadget*fontList",
							 _user_font);
		XrmPutStringResource(&db, "*Work_Area*Text*textFontList",
							 _user_font);
		
		XrmPutStringResource(&db, "*XmDialogShell*XmList*FontList",
							 _user_font);

		// Convert the user fontlist to a font.
                if (!fontlist_to_font(_user_fontlist, &userfont)) {
                    /* Couldn't convert the user fontlist to a font */
                    if (!(userfont.f.cf_font =
                             XLoadQueryFont(XtDisplay(baseWidget()), "fixed")))
			fprintf (stderr, "RoamApp::initialize : error loading fixed font\n");
                    else
                        userfont.cf_type = XmFONT_IS_FONT;
                }

#if 0
	} else {
		FontType	systemfont;

		XrmPutStringResource(&db, "*Work_Area*Text*fontList",
							 _system_font);
		XrmPutStringResource(&db, "*Work_Area*iconGadget*fontList",
							 _system_font);
		XrmPutStringResource(&db, "*Work_Area*Text*textFontList",
							 _system_font);
		XrmPutStringResource(&db, "*XmDialogShell*XmList*FontList",
							 _system_font);

                if (!fontlist_to_font(_system_fontlist, &systemfont)) {
                    /* Couldn't convert the system fontlist to a font */
                    if (!(systemfont.f.cf_font =
                             XLoadQueryFont(XtDisplay(baseWidget()),
				"variable")))
			fprintf (stderr, "RoamApp::initialize : error loading fixed font\n");
                    else
                        systemfont.cf_type = XmFONT_IS_FONT;
                }
                
	}
#endif

	// If the glyph font is specified in a resource file, use it.
        if (_glyph_font) {
	    glyphname = XtNewString(_glyph_font);
	}
	
	// If the glyph font hasn't been specified, try to match it with
	// the user font.
        if (!glyphname) {
	    // Get the font name that matches the user font pixel size.
            load_app_font(baseWidget(), &userfont, &glyphname);
        }

	// Create a fontlist that contains the glyph and user fonts
	strcpy(buf, _user_font);
	if (strchr(_user_font, '=') == NULL) {
	    // No tag.  Add one
	    strcat(buf, "=plain, ");
	}

	// If the symbol font can't be found, use user font above
if (glyphname) {
	    strcat(buf, glyphname);
	    strcat(buf, "=attach");
	}

	// Loosely bind font to message list.  This lets users override
 	// with a more strongly bound name
	// (ie "Dtmail*Message_List*FontList");
	XrmPutStringResource(&db, "*Message_List*FontList", buf);

	XtFree (glyphname);

    }

    // If we don't have a mail file yet, then we need to retrieve the
    // Initialize Tooltalk
    // NOTE:  For now, must make the FIRST ttdt_open call to be the proc_id 
    //        that will respond to the start message.  Therefore, call gui's 
    //        ttdt_open before libDtMail calls its.
    char *sess = NULL;

    sess = (char *)getenv("TT_SESSION");
    if (!sess || (*sess == '\0')) {
      sess = getenv("_SUN_TT_SESSION");
    }
    if (!sess || (*sess == '\0')) {
      tt_default_session_set(tt_X_session(XDisplayString(_display)));
    }

    roam_tt_procid = ttdt_open( &roam_tt_fd, "DTMAIL", "SunSoft", "%I", 1 );
    if (dieFromTtError("ttdt_open", tt_ptr_error(roam_tt_procid))) {
	// Serious error here -- Tooltalk did not initialize -- 
	// we're exiting for now.
	DtMailGenDialog *exit_dialog = new DtMailGenDialog(
					   "ExitDialog", 
					   theApplication->baseWidget());
	exit_dialog->setToErrorDialog( catgets(DT_catd, 2, 1, "Mailer"),
				       catgets(DT_catd, 2, 2, "ToolTalk is not initialized.  Mailer cannot run without ToolTalk.\nTry starting /usr/dt/bin/dtsession, or contact your System Administrator.") );
	helpId = DTMAILHELPCANTINITTOOLTALK;
	exit_dialog->post_and_return(catgets(DT_catd, 1, 1, "OK"), helpId);
	XtRemoveAllCallbacks(theApplication->baseWidget(), XmNdestroyCallback);
	exit(1);
    }

    // This is for supporting old ptype where RFC_822_Message is
    // in lower case.
    status = ttmedia_ptype_declare( "RFC_822_Message",
				    0,
				    tooltalk_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    status = ttmedia_ptype_declare( "RFC_822_MESSAGE",
				    0,
				    tooltalk_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    status = ttmedia_ptype_declare( "MAIL_TYPE",
				    0,
				    attachmt_msg_handler,
				    this,
				    1);
    dieFromTtError("ttmedia_ptype_declare", tt_ptr_error(status));

    /* Join the default session -- This should have been done by default */
    tt_pat = ttdt_session_join((const char *)0,
			       (Ttdt_contract_cb)quit_message_cb,
			       baseWidget(),
			       this,
			       1);
    dieFromTtError("ttdt_session_join", tt_ptr_error(*tt_pat));
    
    _mail_session = new MailSession(mail_error, Application::appContext());

    if (mail_error.isSet()) {
	// what do we do here?  there are no windows for dialogs
	// should we just register a syslog() and exit? ugggh
	// SR
	fprintf(stderr, "RoamApp::initialize : error creating MailSession\n");
	exit (1);
    }


    //[.mailrc parsing error checking
    DtMail::Session * d_session = _mail_session->session();
    DtMail::MailRc* mailRc = d_session->mailRc(mail_error);
    DTMailError_t pErr = mailRc->getParseError();
    if (pErr != DTME_NoError) {
	int answer = 0;
    	DtMailGenDialog *mailrc_dialog = new DtMailGenDialog("MailRcDialog",
                                         theApplication->baseWidget());
        mailrc_dialog->setToQuestionDialog(catgets(DT_catd, 2, 1, "Mailer"),
        catgets(DT_catd, 2, 22, "There were unrecoverable syntax errors found in the ~/.mailrc file.\nCheck for more messages on terminal. Fix the errors and restart dtmail.\nIf you choose to continue you will not be able to save any changes made\nin the options dialog to file.") );
    	helpId = DTMAILHELPERROR;
    	Widget hpBut = XmMessageBoxGetChild(mailrc_dialog->baseWidget(),
                                        XmDIALOG_HELP_BUTTON);
    	XtSetSensitive(hpBut, False);
    	answer = mailrc_dialog->post_and_return(catgets(DT_catd, 2, 23, "Continue"),
								            catgets(DT_catd, 2, 24, "Exit"),
											helpId);
	if (answer == 2) {
		XtRemoveAllCallbacks(theApplication->baseWidget(), XmNdestroyCallback);
		exit(1);
	}
    }
    mail_error.clear();
    // Store the template list so we know if the list has changed via props.
    if (!just_compose) 
        d_session->mailRc(mail_error)->getValue(mail_error, "templates", (const char **)&_templateList);
    else
	_templateList = NULL;

    _options = new OptCmd("Mail Options...",
                          catgets(DT_catd, 1, 2,"Mail Options..."),
                          TRUE,
                          _w);

    if ( just_compose ) {
	// Need to install self destruct
	SendMsgDialog *compose = theCompose.getWin();

	if (dead_letter) {
	    compose->loadDeadLetter(dead_letter);
	}
	else {
	    if ( *argcp > num_legit_args + 1) {    // Have attachment(s)
		for ( int k = num_legit_args + 1;  k < *argcp;  k++ ) {
		    compose->inclAsAttmt(argv[k], NULL);
		}
	    }
	}
    }
    
    // inbox. We do this by querying the current implementation for
    // the path to its inbox.
    //
    if (!mail_file) {
	DtMailObjectSpace space;

	d_session->queryImpl(mail_error,
			     d_session->getDefaultImpl(mail_error),
			     DtMailCapabilityInboxName,
			     &space,
			     &mail_file);
    }

    // We need to determine which transport to use. The order of preference
    // is:
    //
    // 1) The value of DEFAULT_TRANSPORT in the .mailrc file.
    //
    // 2) The value of DEFAULT_TRANSPORT in the environment.
    //
    // 3) The transport, if available for the default implementation.
    //
    // 4) The first implementation that provides a transport.
    //
    const char * trans_impl = NULL;

    d_session->mailRc(mail_error)->getValue(mail_error, 
					    "DEFAULT_TRANSPORT", 
					    &trans_impl);
    if (mail_error.isSet()) {
	trans_impl = NULL;
    }
    
    
    if (!trans_impl) {
	trans_impl = getenv("DEFAULT_TRANSPORT");
    }

    if (!trans_impl) {
	trans_impl = d_session->getDefaultImpl(mail_error);

	DtMailBoolean trans;

	d_session->queryImpl(mail_error, trans_impl, 
			     DtMailCapabilityTransport, &trans);
	if (mail_error.isSet() || trans == DTM_FALSE) {
	    trans_impl = NULL;
	}
    }

    if (!trans_impl) {
	const char ** impls = d_session->enumerateImpls(mail_error);

	for (int impl = 0; impls[impl]; impl++) {
	    DtMailBoolean trans;
	    d_session->queryImpl(mail_error, impls[impl],
				 DtMailCapabilityTransport, &trans);
	    if (!mail_error.isSet() || trans == DTM_TRUE) {
		trans_impl = impls[impl];
		break;
	    }
	}
    }

    if (trans_impl) {
	_mail_transport = d_session->transportConstruct(mail_error,
							trans_impl,
							statusCallback,
							this);
    }
    else {
	_mail_transport = NULL;
    }

    // Register all callbacks the backend may have to deal with
    //
    d_session->registerLastInteractiveEventTimeCallback(raLastInteractiveEventTime, this);
    d_session->registerDisableGroupPrivilegesCallback(raDisableGroupPrivileges, this);
    d_session->registerEnableGroupPrivilegesCallback(raEnableGroupPrivileges, this);
    
    if (d_session->pollRequired(mail_error) == DTM_TRUE) {
	d_session->registerBusyCallback(mail_error, setBusyState, this);
	XtAppAddTimeOut( XtWidgetToApplicationContext( _w ),
			 15000,
			 &RoamApp::applicationTimeout,
			 (XtPointer) this );
    }

    // Process the message that started us, if any.  Should I pass in
    // roam_tt_procid or NULL.
    tttk_Xt_input_handler( 0, 0, 0 );
    XtAppAddInput( Application::appContext(), roam_tt_fd,
		   (XtPointer)(XtInputReadMask | XtInputExceptMask),
 		   tttk_Xt_input_handler, roam_tt_procid );


    // DtMail only supports the "Mail" message.  
    // If DtMail is started by ToolTalk, then we assume that the 
    // client wants a Compose window.  Therefore, we do not
    // bring up a RoamMenuWindow.
    //
    if ( !session_fp && !started_by_tt && !just_compose) {
	RoamMenuWindow *rmw = new RoamMenuWindow(mail_file);
	dtmail_mapped = 1;
    	rmw->initialize();
	rmw->mailboxName(mail_file);
//    	rmw->manage();
    } else {
	if(session_fp ) 
		restoreSession();
    }

    // We update the at job id resources used to determine if vacation is on or off.
    _vacation = new VacationCmd("Vacation", catgets(DT_catd, 1, 3, "Vacation"));
    _vacation->resetVacationStatus();

    // This is for setting the Vacation title stripe on the window and must be
    // done after resetVacationStatus().
    if (_vacation->vacationIsOn())
        setTitles();

    initSession();

    // Make sure the Mail folder exists. Its safer to do 
    // this here as there are so many places to check that we might as well 
    // get it out of the way.
    char *dir = getFolderDir(TRUE);
    if (SafeAccess(dir, F_OK) != 0)
	mkdir(dir, 0700);
    free(dir);
 
    _dialog = new DtMailGenDialog("Dialog", _w);
} 

RoamApp::RoamApp(char *name) : Application (name)
{
    DebugPrintf(2, "RoamApp::RoamApp(0x%x \"%s\")\n", name);
}

// Let the destructor of parent Application class handle the
// exit().

RoamApp::~RoamApp()
{
#ifdef DTMAIL_TOOLTALK
    if ( roam_tt_procid != NULL ) {
	ttdt_close(0, 0, 1);
    }
#endif

    catclose(DT_catd);
    
}

void
RoamApp::shutdown()
{
    delete _mail_session;

    // Delete this and let the parent class's destructor call exit(0)

    delete this;
}

void
RoamApp::applicationTimeout( XtPointer client_data,
				   XtIntervalId *id )
{
  RoamApp *app = ( RoamApp * ) client_data;
  app->timeout( id );
}

void
RoamApp::timeout( XtIntervalId * )
{
    DtMail::Session * d_session = _mail_session->session();
    DtMailEnv error;

    d_session->poll(error);

    XtAppAddTimeOut( XtWidgetToApplicationContext( _w ),
		     15000,
		     &RoamApp::applicationTimeout,
		     (XtPointer) this );
}

void
RoamApp::showBusyState(DtMailEnv &, DtMailBusyState busy_state, void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;

    switch (busy_state)
    {
    case DtMailBusyState_AutoSave:
      self->busyAllWindows(catgets(DT_catd, 3, 1, "Auto-saving..."));
      break;

    case DtMailBusyState_NewMail:
      self->busyAllWindows(catgets(DT_catd, 3, 86, "Checking for new mail..."));
      break;

    case DtMailBusyState_NotBusy:
    default:
      self->unbusyAllWindows();
      break;
    }
}

long
RoamApp::raLastInteractiveEventTime(void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;
    return (theApplication->lastInteractiveEventTime());
}

void
RoamApp::raDisableGroupPrivileges(void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;
    theApplication->disableGroupPrivileges();
    return;
}

void
RoamApp::raEnableGroupPrivileges(void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;
    theApplication->enableGroupPrivileges();
    return;
}

void
RoamApp::setBusyState(DtMailEnv & error, DtMailBusyState busy_state, void * client_data)
{
    RoamApp * self = (RoamApp *)client_data;

    static const MAXBUSYSTATES=20;
    static DtMailBusyState busyStateStack[MAXBUSYSTATES] = {DtMailBusyState_NotBusy};
    static busyStateStackTop = 0;

    DtMailEnv localError;	// used to hold local copy of passed in error state

    localError.setError((const DTMailError_t)error, error.isFatal());

    switch (busy_state)
    {
    case DtMailBusyState_AutoSave:
    case DtMailBusyState_NewMail:
    default:
      showBusyState(localError, busy_state, client_data);
      if (busyStateStackTop)  		// Already busy from previous message ?
	self->_busy_count--;		// Yes, backoff busy count added for this message
      busyStateStack[++busyStateStackTop] = busy_state;
      assert(busyStateStackTop < MAXBUSYSTATES);
      break;

    case DtMailBusyState_NotBusy:
      if (busyStateStackTop) {
	if (localError.isSet()) {
	    // There should always be a valid RoamMenuWindow pointer in
	    // the zero'th element of the _mailview array
	    self->mailView(0)->postErrorDialog(localError);
	}
	busy_state = busyStateStack[--busyStateStackTop];
        showBusyState(localError, busy_state, client_data);
	if (busyStateStackTop)
	  self->_busy_count--;		// backoff busy count for redisplaying this message
      }
      else {
	showBusyState(localError, busy_state, client_data);
      }
      break;
    }
    assert(busyStateStack[0] == DtMailBusyState_NotBusy);
}

void
RoamApp::busyAllWindows(const char * msg)
{
  for (int win = 0; win < _numWindows; win++) {
    if (_busy_count == 0) {
      _windows[win]->busyCursor();
    }
    if (msg) {
      _windows[win]->setStatus(msg);
    }
  }
  _busy_count++;
}

void
RoamApp::unbusyAllWindows(void)
{
    _busy_count--;
    if (_busy_count == 0) {
      for (int win = 0; win < _numWindows; win++) {
	_windows[win]->normalCursor();
	_windows[win]->clearStatus();
      }
    }
}

void
RoamApp::globalPropChange(void)
{
    busyAllWindows(catgets(DT_catd, 1, 4, "Updating properties..."));

    for (int win = 0; win < _numWindows; win++) {
	_windows[win]->propsChanged();
    }
    updateTemplates();

    unbusyAllWindows();
}

DtMailGenDialog *
RoamApp::genDialog()
{
    return(_dialog);
}

int
RoamApp::numOpenRMW(void)
{
        const char *cname;
        int win;
        int numOpen = 0;

        for (win = 0; win < _numWindows; win++) {
                cname = _windows[win]->className();
                if (strcmp(cname, "RoamMenuWindow") == 0) {
                        if (!_windows[win]->isIconified())
                                numOpen++;
                }
        }
        return numOpen;
}
void
RoamApp::updateTemplates(void)
{
    DtMailEnv error;
    DtMail::Session *d_session = theRoamApp.session()->session();
    char *value=NULL; 
    int win = -1;

    // Only update if templates have changed.
    d_session->mailRc(error)->getValue(error, "templates", (const char **)&value);
    if  ( (value == NULL && _templateList != NULL) || 
        (value != NULL &&
	 (_templateList == NULL || strcmp(value, _templateList)) != 0) ) {

    	const char *cname;

     	for (win = 0; win < _numWindows; win++) {
		cname = _windows[win]->className();
		if (strcmp(cname, "RoamMenuWindow") == 0) {
    			RoamMenuWindow *rmw;
			rmw = (RoamMenuWindow *)_windows[win];
			rmw->construct_compose_menu();
		}
		else if (strcmp(cname, "SendMsgDialog") == 0) {
    			SendMsgDialog *smd;
			smd = (SendMsgDialog *)_windows[win];
			smd->createFormatMenu();
		}
        }

    	if (_templateList != NULL) 
                free(_templateList); 
    	if (value && *value) 
                _templateList = value; 
    	else 
                _templateList = NULL; 
    }
    // This means the templates havent changed, so free value 
    if (win == -1 && value)
	free(value);
}

void 
RoamApp::resetTitles()
{
    const char *cname;
    RoamMenuWindow *rmw;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className(); 
	if (strcmp(cname, "RoamMenuWindow") == 0) { 
    		rmw = (RoamMenuWindow *)_windows[win];
	    	rmw->removeVacationTitle();
	}
    }
    
}
void 
RoamApp::setTitles()
{
    RoamMenuWindow *rmw;
    const char *cname;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className();
	if (strcmp(cname, "RoamMenuWindow") == 0) {
		rmw = (RoamMenuWindow *)_windows[win];
		rmw->setVacationTitle();
	}
    }
}

void
RoamApp::startVacation()
{

    if (_vacation->startVacation() == 0)
	setTitles();
    else
	resetTitles();
}

void
RoamApp::stopVacation()
{
    _vacation->stopVacation();
    resetTitles();
}

char*
getPropStringValue(DtVirtArray<PropStringPair *> & results, const char *label)
{
        for (int mrc = 0; mrc < results.length(); mrc++) {
            PropStringPair * psp = results[mrc];
            if (strcmp(label, psp->label) == 0) {
		if (psp->value != NULL)
                	return strdup(psp->value);
        	return (char*)NULL;
	    } 
        }
        return (char*)NULL;
}

void
parsePropString(const char * input, DtVirtArray<PropStringPair *> & results)
{
  if (input == NULL)
	return;

    // If it's not multibyte, use the regular string function
  if (MB_CUR_MAX <= 1) {
     const char * start = input;
     const char * end;
     char *ptr;

    do {
	while (isspace(*start)) {
	    start += 1;
	}
	
	for (end = start; end && *end; end++) 
	    if (isspace(*end) && *(end-1) != '\\')
		break;

	PropStringPair * new_pair = new PropStringPair;

	int len = end - start;
	char * label = new char[len + 5];
	memset(label, 0, len+5);
	strncpy(label, start, len);
	label[len] = 0;
	for (ptr = label; (ptr = strstr(ptr, "\\ ")); ptr++) 
		strcpy(ptr, ptr+1); 
	
	char * file = strchr(label, ':');
	if (!file) {
	    new_pair->label = strdup(label);
	    new_pair->value = NULL;
	}
	else {
	    *file = 0;
	    new_pair->label = strdup(label);

	    file += 1;
	    if (strlen(file) == 0) {
		new_pair->value = NULL;
	    }
	    else {
		new_pair->value = strdup(file);
	    }
	}
 	int i, list_len = results.length();
	for (i=0; i < list_len; i++) {
	    PropStringPair *psp = results[i];
	    if (strcmp(psp->label, new_pair->label) >= 0)
		 break;
        }
	results.insert(new_pair, i);

	delete label;

	start = end;
	
	while (*start && isspace(*start)) {
	    start += 1;
	}
	
    } while(*start && *end);
  }

  else {
    // The string can contain multibyte characters and it must be converted to 
    // wide character string before parsing
    int len = strlen(input);
    wchar_t *wc_input= new wchar_t[len+1];
    mbstowcs(wc_input, input, len+1);
    const wchar_t *start = wc_input;
    const wchar_t *end;
    wchar_t *ptr;

    do {
        while (iswspace(*start)) {             
            start += 1;             
        }

        for (end = start; end && *end; end++)
            if (iswspace(*end) && *(end-1) != (wint_t)'\\')
                break;
        PropStringPair * new_pair = new PropStringPair;

        int wclen = end - start;
        wchar_t *wc_label = new wchar_t[wclen+1];
        wsncpy(wc_label, start, wclen);
        wc_label[wclen] = (wint_t)'\0';

	// Search the string "\ " and take out the back slash
        wchar_t esc_space[3];
        mbstowcs(esc_space, "\\ ", 3);
        for (ptr = wc_label; (ptr = wcswcs(ptr, esc_space)); ptr++)
                wcscpy(ptr, ptr+1);

        wchar_t *file = wcschr(wc_label, (wint_t)':');
        if (!file) {
            new_pair->label = new char[(wclen+1)*MB_CUR_MAX];
            wcstombs(new_pair->label, wc_label, wclen+1);
            new_pair->value = NULL;

        }

        else {
            *file = (wint_t)'\0';
            new_pair->label = new char[(wclen+1)*MB_CUR_MAX];
            wcstombs(new_pair->label, wc_label, (wclen+1)*MB_CUR_MAX);

            file += 1;
            int filelen = wcslen(file);
            if (filelen == 0) {
                new_pair->value = NULL;
            }
            else {
                new_pair->value = new char[(filelen+1)*MB_CUR_MAX];
                wcstombs(new_pair->value, file, (filelen+1)*MB_CUR_MAX);

            }
        }
 	int i, list_len = results.length();
	for (i=0; i < list_len; i++) {
	    PropStringPair *psp = results[i];
	    if (strcmp(psp->label, new_pair->label) >= 0)
		 break;
        }
	results.insert(new_pair, i);

        delete []wc_label;
        start = end;

        while (*start && iswspace(*start)) {
            start += 1;
        }

    } while(*start && *end);

    delete []wc_input;
}
}

PropStringPair::PropStringPair(void)
{
    label = NULL;
    value = NULL;
}

PropStringPair::PropStringPair(const PropStringPair & other)
{
    label = NULL;
    value = NULL;

    if (other.label) {
	label = strdup(other.label);
    }

    if (other.value) {
	value = strdup(other.value);
    }
}

PropStringPair::~PropStringPair(void)
{
    if (label) {
	free(label);
    }

    if (value) {
	free(value);
    }
}

void
RoamApp::setSession( MailSession *session )
{
   _mail_session = session; 
}


VacationCmd*
RoamApp::vacation()
{
    return (_vacation);
}

char *
PropStringPair::formatPropPair()
{

    // figure out whitespace for formatting
    // assume 13 for normal sized alias name

    if (value != NULL) {
    	char *formatted_str = NULL;
    	int i, num_spaces = 0;
	int key_len = strlen(label);

    	if (key_len < 13)
		num_spaces = 13 - key_len;
	formatted_str = (char*)malloc(key_len + strlen((char *)value) + num_spaces + 5);
	strcpy(formatted_str, label);
	for (i=key_len; i < (key_len+num_spaces); i++)
		formatted_str[i] = ' ';	
	formatted_str[i] = '\0';	
	strcat(formatted_str, " = ");
	strcat(formatted_str, (char*)value);
        return formatted_str;
    }
    return strdup(label);

}

int RoamApp::registerMailView(RoamMenuWindow *rmw)
{
    _mailview->append(rmw);
    return (_mailview->length() - 1);
}

void RoamApp::unregisterMailView(int index)
{
    int i;

    _mailview->remove(index);
    // reset the mailbox number for each of the RMW's created after the
    // one being removed.
    for (i=index; i<_mailview->length(); i++) {
	(*_mailview)[i]->setMailboxIndex(i);
    }
}

RoamMenuWindow*
RoamApp::inboxWindow()
{
    RoamMenuWindow *rmw;
    const char *cname;

    for (int win = 0; win < _numWindows; win++) {
	cname = _windows[win]->className();
	if (strcmp(cname, "RoamMenuWindow") == 0) {
	    rmw = (RoamMenuWindow *)_windows[win];
	    if (rmw->inbox()) {
		return(rmw);
	    }
	}
    }
    return(NULL);
}

void
RoamApp::open_catalog()
{
    Application::open_catalog();      // Open Motif Application message catalog file
    DT_catd = catopen(DTMAIL_CAT, NL_CAT_LOCALE); // Open DtMail message catalog file
}
char *
RoamApp::getFolderDir(Boolean expand)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    char *bufptr, *expanded_dir;
    char folderdir[1024];
    MailRcSource *mrs;
    DtMailEnv error;

    // Get the folder directory...
    m_session = theRoamApp.session()->session();
    mrs = (MailRcSource*)new MailRcSource (m_session->mailRc(error), "folder");
    bufptr = (char*)mrs->getValue();
    delete mrs;
    expanded_dir = m_session->expandPath(error, bufptr);
    // Is the folder a relative path? Prepend the home directory
    if (expanded_dir[0] != '/') {
	if (expand)
        	sprintf(folderdir, "%s/%s", getenv("HOME"), expanded_dir);
	else
        	sprintf(folderdir, "$HOME/%s", bufptr);
        free(expanded_dir);
        free(bufptr);
	return strdup(folderdir);
   }
   if (expand) {
	free(bufptr);
   	return expanded_dir;
   }
   free(expanded_dir);
   return bufptr;
}
char *
RoamApp::getResource(resourceType rtype, Boolean expand)
{
    FORCE_SEGV_DECL(DtMail::Session, m_session);
    DtMailEnv error;
    char *bufptr=NULL, *expanded_res;
    char resource[1024];

    m_session = theRoamApp.session()->session();
    MailRcSource *mrs;
    if (rtype == DraftDir) 
	mrs = (MailRcSource*)new MailRcSource 
			(m_session->mailRc(error), "draftdir");
    else if (rtype == TemplateDir)
	mrs = (MailRcSource*)new MailRcSource 
			(m_session->mailRc(error), "templatedir");
    else if (rtype == DeadletterDir)
	mrs = (MailRcSource*)new MailRcSource 
			(m_session->mailRc(error), "deaddir");
    else if (rtype == SentMessageLogFile)
	mrs = (MailRcSource*)new MailRcSource 
			(m_session->mailRc(error), "record");
    bufptr = (char*)mrs->getValue();

    expanded_res = m_session->expandPath(error, bufptr);

    // Is it a relative path? Prepend the folder resource
    if (expanded_res[0] != '/') {
	char *folder = NULL;

	// If message log, make it relative to folder directory
	if (rtype == SentMessageLogFile)
   		folder = getFolderDir(expand);
        else { // For all other directorys, relative to home directory
		if (expand) 
			folder = m_session->expandPath(error, "~");
		else
			folder = "$HOME";
        }

	// Prepend the appropriate directory.
        if (expand) 
		sprintf(resource, "%s/%s", folder, expanded_res);
	else
		sprintf(resource, "%s/%s", folder, bufptr);
	free(expanded_res);
 	if (rtype == SentMessageLogFile || expand)
		free(folder);
	free(bufptr);
	return strdup(resource);
    }
    if (expand) {
    	free(bufptr);
    	return expanded_res;
    }
    free(expanded_res);
    return bufptr;
}
void
RoamApp::configure_menu(Widget menu)
{
	Dimension h;
        int     screenheight = HeightOfScreen(XtScreen(menu));
 
        XtVaGetValues(menu, XmNheight, &h, NULL);
 
        if ((int)h <= (screenheight / 2))
                return;
 
        WidgetList buttonList;
        Dimension bh, w;
        short   maxcols, newcols, mcols;
        int total_num_butts=0, buttons_that_fit;
 
        XtVaGetValues(menu,
                XmNwidth, &w,
                XmNnumColumns, &mcols,
                XmNnumChildren, &total_num_butts,
                XmNchildren, &buttonList,
                NULL);

        XtVaGetValues(buttonList[0],
                XmNheight, &bh,
                NULL);

        // The number of buttons that will fit on half the height of screen
        buttons_that_fit = screenheight / (int)bh / 2;

        // The total number of buttons on the menu divided by the number of
        // buttons on half a screen gives us the number of columns for the menu
        newcols = (total_num_butts / buttons_that_fit) +
                ((total_num_butts % buttons_that_fit) > 0 ? 1 : 0);

        maxcols = WidthOfScreen(XtScreen(menu)) / ((int)w / (int)mcols);
        if (newcols > maxcols)
                newcols = maxcols;

        XtVaSetValues(menu, XmNnumColumns, newcols, NULL);
}
void
RoamApp::map_menu( Widget menu, XtPointer clientData, XtPointer)
{
	if (clientData) {
                CmdList *cl = (CmdList *) clientData;
                RAMenuClientData *mcd = (RAMenuClientData *) cl->getClientData();
                if (mcd) {
                        struct stat fstat;
                        stat(mcd->filename, &fstat);
                        if (fstat.st_mtime != mcd->timestamp) {
                                MenuBar *mb = new MenuBar("menuPR", menu);
                                // Menu was mapped with commands already so 
				// remove them and add new ones
                                if (mcd->timestamp != 0) {
                                        mb->removeCommands(menu, cl);
                                        while (cl->size()) cl->remove(0);
                                }
                                mcd->timestamp = fstat.st_mtime;
                                theRoamApp.addDrafts(cl, mcd->smd, mcd->filename);
                                mb->addCommands(cl, FALSE, XmMENU_PULLDOWN);
                        }
                }
        }
	theRoamApp.configure_menu(menu);
}

#define DOTS(a) (a[0] == '.' && \
        (a[1] == 0 || (a[1] == '.' && a[2] == 0)))
void
RoamApp::addDrafts(CmdList *subCmd, SendMsgDialog *smd, char *draftDir)
{
    Cmd *button = NULL;
    DIR *dirp; 
    struct dirent *direntp;
    struct stat fstat;
    char filename[1024];

    if (dirp = opendir(draftDir)) {
        while ((direntp = readdir(dirp)) != NULL) {
                if (direntp->d_ino == 0 || DOTS(direntp->d_name)) continue;
                sprintf(filename, "%s/%s", draftDir, direntp->d_name);
                stat(filename, &fstat);
                if ((fstat.st_mode & S_IFMT) == S_IFREG) {
                        button = new LoadFileCmd(strdup(direntp->d_name),
                                           direntp->d_name,
                                           1, smd,
                                           filename, TYPE_DRAFT);
                        subCmd->insert(button);
                }
        }
    }
    if (button == NULL) {
	button = new LoadFileCmd("No Drafts",
			 catgets(DT_catd, 3, 114, "No Drafts"),
			 1, smd,
			 NULL, TYPE_DRAFT);
	subCmd->add(button);
    }
}
