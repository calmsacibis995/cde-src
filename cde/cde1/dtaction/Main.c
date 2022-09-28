/* $XConsortium: Main.c /main/cde1_maint/6 1995/10/07 19:15:15 pascale $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         Main.c
 **
 **   Project:	    DT
 **
 **   Description:  This file contains the main program for dtaction.
 **		  
 **
 **(c) Copyright 1993, 1994 Hewlett-Packard Company
 **(c) Copyright 1993, 1994 International Business Machines Corp.
 **(c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **(c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <limits.h>
#include <stddef.h>
#include <unistd.h>
#include <nl_types.h>
#include <signal.h>
#include <sys/param.h>		/* for MAXPATHLEN and MAXHOSTNAMELEN */

#ifdef sun
#include <crypt.h>
#include <shadow.h>
#endif

#include <errno.h>
#include <grp.h>
#include <pwd.h>

#include <X11/Intrinsic.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>

#include <Dt/Dt.h>
#include <Dt/DbUtil.h>
#include <Dt/CmdInv.h>
#include <Dt/Action.h>
#include <Dt/Dts.h>
#include <Dt/EnvControl.h>


/******************************************************************************/

#include <time.h>


/* Command line options */
XrmOptionDescRec option_list[] =
{
   {  "-user",     "user",          XrmoptionSepArg,   NULL},
   {  "-contextDir", "contextDir",  XrmoptionSepArg,   NULL},
   {  "-execHost", "execHost",      XrmoptionSepArg,   NULL},
   {  "-termOpts", "termOpts",      XrmoptionSepArg,   NULL},
};

/* Fallback Resources */
static char *fallback_resources[] = {
    "*timeout: 5",
    (char *) NULL
};

typedef struct {
   char * user;
   char * contextDir;
   char * execHost;
   char * termOpts;
} ApplArgs, *ApplArgsPtr;

#define LOGIN_STR_LEN 15
#define STAR ' '

Widget dlog;
char *password;
char *stars;
int  passwordLength = 0;

Boolean finished = False;
Boolean rootRequest = False;
char * toPword = NULL;
char * rootPword = NULL;
char * origName = "Unknown";
uid_t basegid = -1;
uid_t newuid = -1;

/* Dtaction resources */
XtResource resources[] =
{
   {
    "user", "User", XmRString, sizeof(char *),
    XtOffsetOf(ApplArgs, user), XmRImmediate, (XtPointer) NULL,
   },
   {
    "contextDir", "ContextDir", XmRString, sizeof(char *),
    XtOffsetOf(ApplArgs, contextDir), XmRImmediate, (XtPointer) NULL,
   },
   {
    "execHost", "ExecHost", XmRString, sizeof(char *),
    XtOffsetOf(ApplArgs, execHost), XmRImmediate, (XtPointer) NULL,
   },
   {
    "termOpts", "TermParms", XmRString, sizeof(char *),
    XtOffsetOf(ApplArgs, termOpts), XmRImmediate, (XtPointer) NULL,
   },
};

 /*
 * macro to get message catalog strings
 */

#ifdef MESSAGE_CAT
#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtact.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtact"
#endif /* __ultrix */
#ifdef _NO_PROTO
extern char *_DtGetMessage();
#else  /* _NO_PROTO */
extern char *_DtGetMessage(
                        char *filename,
                        int set,
                        int n,
                        char *s );
#endif /* _NO_PROTO */
#define GETMESSAGE(set, number, string)\
    (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

Boolean _DtWmStringsAreEqual( 
                        register char *in_str,
                        register char *test_str) ;
void CheckForDone( 
                        XtPointer clientData,
                        XtIntervalId id) ;
void CheckUserRequest( void ) ;
void CheckPasswd( void ) ;
void AddSuLog( 
                        char *FromName,
                        char *ToName,
                        char *ChangeType) ;
void CleanPath( 
                        char *Path) ;
void OkCallback( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
void ErrOkCallback( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
void CancelCallback( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
void MapCallback( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;

void actionStatusCallback (
                        DtActionInvocationID id,
                        XtPointer   client_data,
                        DtActionArg *aap,
                        int         aac,
                        int         status );

void GetUserPrompt( void ) ;
void LogSuccess( void ) ;
void LogFailure( void ) ;
void MyInsert( 
                        Widget w,
                        XEvent *event,
                        char **params,
                        Cardinal *num_params) ;
void MyCancel( 
                        Widget w,
                        XEvent *event,
                        char **params,
                        Cardinal *num_params) ;
void EditPasswdCB( 
                        Widget w,
                        XtPointer client,
                        XtPointer call) ;
void UnknownUser( void ) ;
void UnknownUserCallback( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;


/********    End Forward Function Declarations    ********/


#ifdef MESSAGE_CAT
#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtact.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtact"
#endif /* __ultrix */
#define GETMESSAGE(set, number, string)\
    (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */


/***************************************************************************
 *
 *  Text widget actions and translations
 *
 ***************************************************************************/

XtActionsRec textActions[] = {
        {"my-insert", (XtActionProc)MyInsert},
        {"my-cancel", (XtActionProc)MyCancel},
};

char textEventBindings[] = {
"Shift <Key>Tab:                        prev-tab-group() \n\
  Ctrl <Key>Tab:                        next-tab-group() \n\
 <Key>Tab:                              next-tab-group() \n\
 <Key>osfEndLine:                       end-of-line() \n\
 <Key>osfBeginLine:                     beginning-of-line() \n\
 ~Shift <Key>osfRight:                  forward-character()\n\
 ~Shift <Key>osfLeft:                   backward-character()\n\
  Ctrl <Key>osfDelete:                  delete-to-end-of-line()\n\
 <Key>osfDelete:                        delete-next-character()\n\
  <Key>osfBackSpace:                    delete-previous-character()\n\
 <Key>osfActivate:                      activate()\n\
  Ctrl <Key>Return:                     activate()\n\
 <Key>Return:                           activate()\n\
 <Key>osfCancel:                        my-cancel()\n\
 <Key>:                                 my-insert()\n\
 ~Ctrl ~Shift ~Meta ~Alt<Btn1Down>:     grab-focus() \n\
 <EnterWindow>:                         enter()\n\
 <LeaveWindow>:                         leave()\n\
 <FocusIn>:                             focusIn()\n\
 <FocusOut>:                            focusOut()\n\
 <Unmap>:                               unmap()"
};


/****************************************************************************/
/****************************************************************************/

static Widget toplevel;
static ApplArgs appArgs;
static XtAppContext appContext;
static DtActionInvocationID actionId;
static Boolean exitAfterInvoked = False; 


/* ARGSUSED */
void
#ifdef _NO_PROTO
CheckForDone( clientData, id )
   XtPointer clientData;
   XtIntervalId id;
#else
CheckForDone(
   XtPointer clientData,
   XtIntervalId id)
#endif /* _NO_PROTO */
{
    if ( toplevel->core.num_popups ==  0 ) 
	exit(0);

    XtAppAddTimeOut(appContext,
	10, (XtTimerCallbackProc)CheckForDone,
      NULL);
}


void
main( 
     int argc,
     char **argv ) 
{
    Display *display;
    Arg args[20];
    int n=0;
    char *actionName;
    int numArgs = 0;
    char contextDir[MAXPATHLEN+1];
    DtActionArg *ap = NULL;
  
#ifdef __osf__
    svc_init();
#endif

    XtSetLanguageProc(NULL, NULL, NULL);
    _DtEnvControl(DT_ENV_SET);
   (void) signal(SIGCLD, (void (*)())SIG_IGN);

    /*  Initialize the toolkit and open the display  */
    XtToolkitInitialize() ;
    appContext = XtCreateApplicationContext() ;
    if ( !(display = XtOpenDisplay( appContext, NULL, argv[0], "Dtaction", 
                             option_list, 
			     sizeof(option_list)/sizeof(XrmOptionDescRec),
			     &argc, argv)) )
    {
        fprintf(stderr,GETMESSAGE(1,11,"Can't open display.\n"));
	exit(1);
    }
  
    XtSetArg(args[n], XmNallowShellResize, True); n++;
    XtSetArg(args[n], XmNmappedWhenManaged, False); n++;
    XtSetArg(args[n], XmNheight, 1); n++;
    XtSetArg(args[n], XmNwidth, 1); n++;
    toplevel = XtAppCreateShell( argv[0], "Dtaction", 
            topLevelShellWidgetClass, display, args, n) ;

    XtRealizeWidget(toplevel);

    display = XtDisplay (toplevel);
    XtGetApplicationResources(toplevel, &appArgs, 
	resources, XtNumber(resources), NULL, 0);

    password = XtMalloc(1);
    password[0] = '\0';
    stars = XtMalloc(1);
    stars[0] = '\0';

   /*  Get Dt initialized  */
   if (DtInitialize (display, toplevel, argv[0], "Dtaction") == False)
   {
      /* Fatal Error: could not connect to the messaging system. */
      /* DtInitialize() has already logged an appropriate error msg */
      exit(2);
   }

   /*
    * If the request specified that it wanted to run as a different
    * user, then take care of prompting for a password, and doing any
    * necessary verification and logging.
    */
   CheckUserRequest();
   
   /* Load the filetype/action dbs; DtInvokeAction() requires this */
   DtDbLoad();

   /*
    * Get the requested action name
    */
    actionName = argv[1];
    if ((actionName == 0) || (*actionName == '\0'))
    {
	if (argc > 2) {
	    char *type;
	    char *actions;
	    type = DtDtsFileToDataType( argv[2] );
	    actions = DtDtsDataTypeToAttributeValue( type,
					DtDTS_DA_ACTION_LIST, 0 );
	    if (actions != 0) {
	        char *comma;
		comma = strchr( actions, ',' );
		if (comma != 0) *comma = '\0';
		actionName = actions;
	    }
	}
    }
    if ((actionName == 0) || (*actionName == '\0'))
    {
	fprintf(stderr,GETMESSAGE(1,10,"No action name specified.\n"));
	exit(3);
    }

    if ( argc > 2 ) 
    {
	/*
	 * create an action arg array for the file objects for
	 * this action.  This number of objects should be one
	 * less than the argument count. The returned vector will
	 * be terminated by a null pointer.
	 */
	numArgs= argc - 2;
	ap = (DtActionArg *) XtCalloc(numArgs,sizeof(DtActionArg)); 
    }

	/*
	 * This client is restricted to FILE arguments.
	 * for the time being.
	 */
    for ( n = 0; n < numArgs; n++) {
        ap[n].argClass    = DtACTION_FILE;
	ap[n].u.file.name = argv[n+2];
    }

    actionId = DtActionInvoke(toplevel, actionName, ap, numArgs,
        appArgs.termOpts,
        appArgs.execHost,
        appArgs.contextDir,
        True,			/* use indicator */
        (DtActionCallbackProc) actionStatusCallback,
        NULL);

    /*
     * Set up a timer if we didn't get a valid procId -- since there will
     * be no invocation update in that case.
     * We must invoke XtMainLoop() at least once, to force any prompt or
     * error dialogs to get posted.
     */
    if ( !actionId)
	    XtAppAddTimeOut(appContext,
		10, (XtTimerCallbackProc)CheckForDone,
			   NULL);

    XtAppMainLoop(appContext);

}


static void
SetGidUid ( uid_t rgid, uid_t ruid )
{

	/* fix process gid */
#ifdef __hpux
      setresgid(rgid, rgid, rgid);
#else
#  if defined (SVR4) || defined (_AIX)
      setgid(rgid);
#  elif defined(__osf__)
      setregid(rgid, rgid);
#  else
      setregid(rgid, rgid, rgid);
#  endif
#endif

	/* fix process uid */
#ifdef __hpux
      setresuid(ruid, ruid, ruid);
#  elif defined(__osf__)
      setreuid(ruid, ruid);
#else
#  if defined (SVR4) || defined (_AIX)
      setuid(ruid);
#  else
      setreuid(ruid, ruid, ruid);
#  endif
#endif

}


/*
 * This function checks to see if the user has requested that the action
 * be invoked under a different user Id.  If a different user Id has been
 * requested, then the user will be prompted to enter either the password
 * for that user, or the root password.  Once a valid password has been
 * entered, this function will return.
 */

void 
#ifdef _NO_PROTO
CheckUserRequest()
#else
CheckUserRequest( void )
#endif /* _NO_PROTO */

{
   uid_t ruid;
   uid_t rgid;
   struct passwd * passwd;
#ifdef sun
   struct spwd * spwd;
#endif
   Boolean notExist = False;
   Boolean alreadySetToRoot = False;

   rootRequest = False;

	/* get current group id */
   rgid = getgid();
        /* get current user id */
   ruid = getuid();

      /* See if the user wants to run as himself */
   if (appArgs.user == NULL)
   {
      SetGidUid(rgid,ruid);
      return;
   }

   /* Get password for the requested user */
   passwd = getpwnam(appArgs.user);
   if (passwd) {
#ifdef sun
      spwd = getspnam(appArgs.user);
      if (spwd)
      {
#endif
         if (passwd->pw_uid == ruid)
         {
            /* 
             * We are already running as the
             *  requested user.  So return now.
             */
	     SetGidUid(rgid,ruid);
             return;
          }

#ifdef sun
          if (spwd->sp_pwdp)
             toPword = XtNewString(spwd->sp_pwdp);
       }
#else
       if (passwd->pw_passwd)
          toPword = XtNewString(passwd->pw_passwd);
#endif /* sun */
       basegid = passwd->pw_gid;
       newuid = passwd->pw_uid;
    }
   else
       notExist = True;

   /* Root requests require some extra work later */
   if (strcmp(appArgs.user, "root") == 0)
         rootRequest = True;

   /* Get name for the current user */
   passwd = getpwuid(ruid);
   if (passwd && passwd->pw_name)
       origName = XtNewString(passwd->pw_name);

   /* Get password for the root user */
   passwd = getpwnam("root");
   if (passwd && passwd->pw_passwd) 
   {
#ifdef sun
       spwd = getspnam("root");
       if (spwd && spwd->sp_pwdp) 
       { 
           rootPword = XtNewString(spwd->sp_pwdp);
       }
#else
      rootPword = XtNewString(passwd->pw_passwd);
#endif /* sun */

       if (passwd->pw_uid == ruid)
          alreadySetToRoot = True;
   }

   /*
    * If 'alreadySetToRoot' is set to True, then that means that the
    * user is  currently running as root.
    */

   if (notExist)
   {
      /* Requested user does not exist; this function will exit() */
       UnknownUser();
   }
   else if ((alreadySetToRoot) ||  /* requested users passwd is null */
      ((toPword && (toPword[0] == '\0')) || (toPword == NULL)))
   {
       /* Already there -- no need to check a passwd */
          LogSuccess();
   }
   else
         CheckPasswd();
}


/**********
 * void CheckPasswd ()
 *
 * get a password from the user and check it against an encrypted passwd
 *
 * Returns:
 *	0 if invalid
 *	True if valid
 *
 * Side Effects:
 *	none.
 */

void 
#ifdef _NO_PROTO
CheckPasswd()
#else
CheckPasswd( void )
#endif /* _NO_PROTO */

{
    /*
     * get this users password
     */
    GetUserPrompt();
}


/**********
 * void AddSuLog (FromName, ToName, ChangeType)
 *
 * add switch from user "FromName" to user "ToName" to sulog.
 * ChangeType is "+" for success, "-" for failure.
 *
 * Parameters:
 *	char *FromName -- from name (for logging).
 *	char *ToName -- to name (for logging).
 *	char *ChangeType -- +/- (for logging).
 */

void 
#ifdef _NO_PROTO
AddSuLog( FromName, ToName, ChangeType )
        char *FromName ;
        char *ToName ;
        char *ChangeType ;
#else
AddSuLog(
        char *FromName,
        char *ToName,
        char *ChangeType )
#endif /* _NO_PROTO */

{
    char        toString[BUFSIZ];

    register    FILE * f;
    struct stat st;
    time_t    timenow;
    struct tm  *now;

#ifdef hpV4  /* 10.* versions */
    char * SULog = "/var/adm/sulog";
#elif  defined( hpux )	/* 9.* versions */
    char * SULog = "/usr/adm/sulog";
#else
    char * SULog = "/var/adm/sulog";
#endif

    if ((f = fopen (SULog, "a")) == NULL)
	return;

    (void) time (&timenow);
    now = localtime (&timenow);

    /* build toString... */
    if (ToName && *ToName) 
	    (void) sprintf(toString, "%s", ToName);
    else 
	    (void) sprintf(toString, "%s", FromName);

    fprintf(f, (GETMESSAGE(1,1, 
           "dtaction %1$.2d/%2$.2d %3$.2d:%4$.2d %5$1.1s %6$s %7$s-%8$s\n")),
	   now -> tm_mon + 1, now -> tm_mday, now -> tm_hour,
	   now -> tm_min, ChangeType, "?", FromName, toString);

    (void) fclose (f);

    /*
     * take away write access from SULog
     */

    if (stat (SULog, &st) == 0)
       chmod (SULog, (int) (st.st_mode & 07777) & ~0222);

    return;
}


/**********
 * void CleanPath (Path);
 *
 * remove any directories from the path that are
 *    - null (leading/trailing colon or double colon)
 *    - not anchored to root (no leading /)
 *
 * the returned path is the original path with any such
 * directories stripped
 *
 * Parameters:
 *	char *Path -- $PATH to clean
 *
 * Returns:
 *	none.
 *
 * Side Effects:
 *	Unanchored paths will be stripped off of Path.
 */

void 
#ifdef _NO_PROTO
CleanPath( Path )
        char *Path ;
#else
CleanPath(
        char *Path )
#endif /* _NO_PROTO */

{
    register char  *StrippedPath;
    register char  *PathHead;

    StrippedPath = PathHead = Path;

    while (*Path) {

	/*
	 * remove multiple ':'s
	 */

	while (*Path && (*Path == ':')) {
	    (void) Path++;
	}

	/*
	 * is the first character of this
	 * directory a '/'????
	 */

	if (*Path == '/') {

	    /*
	     * copy directory intact;
	     */

	    while (*Path && (*Path != ':')) {
		*StrippedPath++ = *Path++;
	    }

	    if (*Path == ':') {
		*StrippedPath++ = *Path++;
	    }
	}
	else {

	    /*
	     * zip past directory
	     */

	    while (*Path && (*Path != ':')) {
		(void) Path++;
	    }

	    if (*Path == ':') {
		(void) Path++;
	    }
	}

    }

    /*
     * remove all trailing ':'s
     */

    while ((StrippedPath > PathHead) && (StrippedPath[-1] == ':')) {
	StrippedPath--;
    }

    /*
     * null terminate the path
     */

    *StrippedPath = '\0';
    return;
}


/*
 * This is the Ok callback for the password dialog.  It will attempt to
 * validate the password.  If invalid, then an error dialog is posted,
 * and the user is prompted to try again.  If valid, then the process
 * will change to run as the requested user, and dtaction will continue
 * on its way, attempting to invoke the requested action.
 */

/* ARGSUSED */
void 
#ifdef _NO_PROTO
OkCallback( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
OkCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   Boolean valid = False;

   /* Do any verification here */

   /* check for primary passwd... */
   if (!strcmp (crypt (password, toPword), toPword))
      valid = True;

   /* check for secondary passwd ... */
   if (rootPword && *rootPword &&
       !strcmp (crypt (password, rootPword), rootPword))
   {
      valid = True;
   }
   else if (((rootPword == NULL) || (*rootPword == '\0')) &&
            (*password == '\0'))
   {
      /* No root password, and the user entered no password */
      valid = True;
   }


   /* If valid password, then unpost */
   if (valid)
   {
      XtUnmanageChild(dlog);
      XFlush(XtDisplay(dlog));
      finished = True;
      LogSuccess();
   }
   else
   {
      /* Invalid password */
      Widget err;
      int n;
      Arg args[10];
      XmString okLabel;
      XmString message;
      char * template;
      char * title;
      char * master;

      okLabel = XmStringCreateLtoR((GETMESSAGE(1,2, "OK")), 
                                   XmFONTLIST_DEFAULT_TAG);
      template = (GETMESSAGE(1,3, "The password you entered does not match\nthe password for user %s.\n\nPlease reenter the password, or select the\nCancel button to terminate the operation."));
      master = XtMalloc(strlen(template) + strlen(appArgs.user) + 10);
      sprintf(master, template, appArgs.user);
      message = XmStringCreateLtoR(master, XmFONTLIST_DEFAULT_TAG);
      title = (GETMESSAGE(1,4, "Action Invoker - Password Error"));

      /* Post an error dialog */
      n = 0;
      XtSetArg(args[n], XmNtitle, title); n++;
      XtSetArg(args[n], XmNmessageString, message); n++;
      XtSetArg(args[n], XmNokLabelString, okLabel); n++;
      err = XmCreateErrorDialog(dlog, "err", args, n);
      XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_HELP_BUTTON));
      XtManageChild(err);
      XtAddCallback(err, XmNokCallback, ErrOkCallback, err);

      XFlush(XtDisplay(dlog));
      XmUpdateDisplay(dlog);
      LogFailure();
      XmStringFree(okLabel);
      XmStringFree(message);
      XtFree(master);
   }
}


/*
 * This is the Cancel callback for the password dialog.  It will unpost
 * the dialog, and exit.
 */

/* ARGSUSED */
void 
#ifdef _NO_PROTO
CancelCallback( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
CancelCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   XtUnmanageChild(dlog);
   XFlush(XtDisplay(dlog));
   exit(255);
}


/*
 * This is the 'Ok' callback for the invalid password error dialog.
 * It will simply unpost and destroy the error dialog.
 */

/* ARGSUSED */
void 
#ifdef _NO_PROTO
ErrOkCallback( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
ErrOkCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   Widget err = (Widget)clientData;

   XtUnmanageChild(err);
   XFlush(XtDisplay(err));
   XmUpdateDisplay(err);
   XtDestroyWidget(err);
}


/*
 * This callback is invoked when the password dialog is posted; it forces
 * the focus to the text input field.
 */

/* ARGSUSED */
void 
#ifdef _NO_PROTO
MapCallback( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
MapCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   Widget text;
   Widget dlog = (Widget)clientData;

   /* Force focus initially to the text field */
   text = XmSelectionBoxGetChild(dlog, XmDIALOG_TEXT);
   XmProcessTraversal(text, XmTRAVERSE_CURRENT);
}


/*
 * This function creates the prompt dialog for collecting the password
 * from the user.  It will not give up control until a valid password
 * has been entered.  If the user cancels the request, then the cancel
 * callback will exit.
 */

void 
#ifdef _NO_PROTO
GetUserPrompt()
#else
GetUserPrompt( void )
#endif /* _NO_PROTO */

{
   int n;
   XmString xmString;
   XmString xmString2;
   char prompt[256];
   Widget help;
   Widget text;
   XEvent event;
   Arg args[10];
   XtTranslations      textTable;
   XmString cancelLabel;
   XmString okLabel;

   sprintf(prompt, (GETMESSAGE(1,5, "Enter password for user %s:")), 
           appArgs.user);
   xmString = XmStringCreateLtoR(prompt, XmFONTLIST_DEFAULT_TAG);
   xmString2 =XmStringCreateLtoR((GETMESSAGE(1,6, "Action Invoker - Password")),
                                  XmFONTLIST_DEFAULT_TAG);
   cancelLabel = XmStringCreateLtoR((GETMESSAGE(1,7, "Cancel")), 
                                  XmFONTLIST_DEFAULT_TAG);
   okLabel = XmStringCreateLtoR((GETMESSAGE(1,2, "OK")), 
                                  XmFONTLIST_DEFAULT_TAG);

   XtAppAddActions(appContext,textActions, 2);
   textTable = XtParseTranslationTable(textEventBindings);

   /* Create the prompt dialog */
   n = 0;
   XtSetArg(args[n], XmNselectionLabelString, xmString);  n++;
   XtSetArg(args[n], XmNdialogTitle, xmString2);  n++;
   XtSetArg(args[n], XmNautoUnmanage, False);  n++;
   XtSetArg(args[n], XmNokLabelString, okLabel);  n++;
   XtSetArg(args[n], XmNcancelLabelString, cancelLabel);  n++;
   XtSetArg(args[n], XmNdefaultPosition, False);  n++;
   dlog = XmCreatePromptDialog(toplevel, "prompt", args, n);
   XmStringFree(xmString);
   XmStringFree(xmString2);
   XmStringFree(okLabel);
   XmStringFree(cancelLabel);
   XtAddCallback(dlog, XmNokCallback, OkCallback, NULL);
   XtAddCallback(dlog, XmNcancelCallback, CancelCallback, NULL);

   text = XmSelectionBoxGetChild(dlog, XmDIALOG_TEXT);
   n = 0;
   XtSetArg(args[n], XmNtranslations, textTable); n++;
   XtSetArg(args[n], XmNverifyBell, False);       n++;
   XtSetValues(text, args, n);
   XtAddCallback(text, XmNmodifyVerifyCallback, EditPasswdCB, NULL);

   /* Add callback for forcing traversal to the text field */
   XtAddCallback (XtParent(dlog), XmNpopupCallback, MapCallback, dlog);

   /* Unmanage the help button */
   help = XmSelectionBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
   XtUnmanageChild(help);

   /* Center the dialog */
   XtRealizeWidget(dlog);
   XtSetArg (args[0], XmNx,
            (WidthOfScreen(XtScreen(dlog)) - dlog->core.width) / 2);
   XtSetArg (args[1], XmNy,
            (HeightOfScreen(XtScreen(dlog)) - dlog->core.height) / 2);
   XtSetValues (dlog, args, 2);

   /* Set the transient property */
   XSetTransientForHint (XtDisplay (toplevel),
                         XtWindow (XtParent (dlog)),
                         XtWindow (toplevel));

   /*  Adjust the decorations for the dialog shell of the dialog  */
   n = 0;
   XtSetArg(args[n], XmNmwmFunctions, 0);          n++;
   XtSetArg(args[n], XmNmwmDecorations, 
            MWM_DECOR_BORDER | MWM_DECOR_TITLE);   n++;
   XtSetValues(XtParent(dlog), args, n);

   /* Post the dialog */
   XtManageChild(dlog);

   /* Wait for the user to finish with the dialog */
   while (!finished)
   {
      XtAppNextEvent(appContext,&event);
      XtDispatchEvent(&event);
   }

   /* Destroy the widget, and return any data back to the appl */
   XtDestroyWidget(dlog);
}


/*
 * When a user has successfully logged in as another user, we need to set
 * the uid and gid to the requested user.  In addition, if the user is
 * changing to 'root', then we need to log this in /usr/adm/sulog, and
 * we need to do some housekeeping of the $PATH environment variable.
 */

void 
#ifdef _NO_PROTO
LogSuccess()
#else
LogSuccess( void )
#endif /* _NO_PROTO */

{
   char * path;
   char * tmpPath;

   AddSuLog(origName, appArgs.user, "+");

   if (rootRequest)
   {
      /* Special stuff for the root user */
      /* Cleanse the $PATH setting */
      tmpPath = getenv("PATH");
      path = XtNewString(tmpPath);
      CleanPath (path);
      tmpPath = XtMalloc(strlen(path) + 10);
      strcpy(tmpPath, "PATH=");
      strcat(tmpPath, path);
      putenv(tmpPath);
   }

   /* Set up the user's new id's */
   SetGidUid(basegid,newuid);
#ifndef sco
   initgroups(appArgs.user, basegid);
#endif

}


/*
 * Each time the user enters an invalid password, we need to log this in
 * /usr/adm/sulog, if the user is attempting to switch to the 'root' user.
 */

void 
#ifdef _NO_PROTO
LogFailure()
#else
LogFailure( void )
#endif /* _NO_PROTO */

{
   /* Unable to change to specified user; post error, then exit */
   AddSuLog(origName, appArgs.user, "-");
}


/***************************************************************************
 *
 *  MyInsert
 *
 *  Local self-insert action for the text widget. The default action
 *  discards control characters, which are allowed in password.
 ***************************************************************************/

/* ARGSUSED */
void 
#ifdef _NO_PROTO
MyInsert( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        char **params ;
        Cardinal *num_params ;
#else
MyInsert(
        Widget w,
        XEvent *event,
        char **params,
        Cardinal *num_params )
#endif /* _NO_PROTO */
{
    char           str[32];
    XComposeStatus compstatus;
    int            n;

    n = XLookupString((XKeyEvent *)event, str, sizeof(str),
                      (KeySym *)NULL, &compstatus);

    if (n > 0) {
       str[n] = '\0';
       XmTextFieldInsert(w, XmTextFieldGetCursorPosition(w), str);
    }
}


/***************************************************************************
 *
 *  MyCancel
 *
 *  This action catches the 'Escape' key, and following Motif standards,
 *  unposts the dialog, as if the 'Cancel' button had been pressed.
 ***************************************************************************/

/* ARGSUSED */
void 
#ifdef _NO_PROTO
MyCancel( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        char **params ;
        Cardinal *num_params ;
#else
MyCancel(
        Widget w,
        XEvent *event,
        char **params,
        Cardinal *num_params )
#endif /* _NO_PROTO */
{
   CancelCallback(w, NULL, NULL);
}



/***************************************************************************
 *
 *  EditPasswdCB
 *
 *  implement no-echo of the password
 ***************************************************************************/


/* ARGSUSED */
void 
#ifdef _NO_PROTO
EditPasswdCB( w, client, call )
        Widget w ;
        XtPointer client ;
        XtPointer call ;
#else
EditPasswdCB(
        Widget w,
        XtPointer client,
        XtPointer call )
#endif /* _NO_PROTO */
{

    static Boolean      allow_flag = False;
    int                 replaced_length, i;
    char                *src, *dst;
    XmTextVerifyPtr     cbData = (XmTextVerifyPtr) call;
  
    if (!allow_flag)
    {
      /* 
       * we need to keep track of the password ourselves in order to 
       * disable echoing of the password...
       */
      
      replaced_length = cbData->endPos - cbData->startPos;
      if (replaced_length > cbData->text->length)
      {
        /* shift the text at and after endPos to the left...  */
        for (src = password + cbData->endPos,
             dst = src + (cbData->text->length - replaced_length),
             i = passwordLength - cbData->endPos;
             i > 0;
             ++src, ++dst, --i)
         {
            *dst = *src;
         }
      }
      else if (replaced_length < cbData->text->length)
      {
        /* Buffer must grow */
        password = XtRealloc(password,
              passwordLength + cbData->text->length - replaced_length + 5);

        /* shift the text at and after endPos to the right...  */
        for (src = password + passwordLength - 1,
             dst = src + (cbData->text->length - replaced_length),
             i = passwordLength - cbData->endPos;
             i > 0;
             --src, --dst, --i)
         {
            *dst = *src;
         }
      }
      
      /*
       * update the password...
       */

      for (src = cbData->text->ptr,
           dst = password + cbData->startPos,
           i = cbData->text->length;
           i > 0;
           ++src, ++dst, --i)
      {
        *dst = *src;
      }

      passwordLength += cbData->text->length - replaced_length;
      password[passwordLength] = '\0';
      stars = XtRealloc(stars, cbData->text->length + 10);
      for (i = 0; i < cbData->text->length; i++)
         stars[i] = ' ';
      stars[cbData->text->length] = '\0';
      
      /*
       * put the appropriate number of stars in the passwd Widget..
       */

      allow_flag = True;
      XmTextFieldReplace(w, cbData->startPos, cbData->endPos, stars);
      allow_flag = False;
    }
  
    cbData->doit = allow_flag;
}


/*
 * This function posts an error dialog informing the user that they have
 * specified an invalid user name.  No further processing will be done; we
 * will simply wait for the user to acknowledge the error, and then exit.
 */

void 
#ifdef _NO_PROTO
UnknownUser()
#else
UnknownUser( void )
#endif /* _NO_PROTO */

{
   Widget err;
   int n;
   Arg args[10];
   XmString okLabel;
   XmString message;
   char * template;
   char * title;
   char * master;

   okLabel = XmStringCreateLtoR((GETMESSAGE(1,2, "OK")), 
                                  XmFONTLIST_DEFAULT_TAG);
   template = (GETMESSAGE(1,8, "The user '%s' is an unknown user name.\n\nThe requested action will not be executed."));
   master = XtMalloc(strlen(template) + strlen(appArgs.user) + 10);
   sprintf(master, template, appArgs.user);
   message = XmStringCreateLtoR(master, XmFONTLIST_DEFAULT_TAG);
   title = (GETMESSAGE(1,9, "Action Invoker - Unknown User"));

   /* Post an error dialog */
   n = 0;
   XtSetArg(args[n], XmNtitle, title); n++;
   XtSetArg(args[n], XmNmessageString, message); n++;
   XtSetArg(args[n], XmNokLabelString, okLabel); n++;
   XtSetArg(args[n], XmNdefaultPosition, False);  n++;
   err = XmCreateErrorDialog(toplevel, "err", args, n);
   XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(err, XmDIALOG_HELP_BUTTON));

   /* Center the dialog */
   XtRealizeWidget(err);
   XtSetArg (args[0], XmNx,
            (WidthOfScreen(XtScreen(err)) - err->core.width) / 2);
   XtSetArg (args[1], XmNy,
            (HeightOfScreen(XtScreen(err)) - err->core.height) / 2);
   XtSetValues (err, args, 2);

   XtManageChild(err);
   XtAddCallback(err, XmNokCallback, UnknownUserCallback, err);
   XtAddCallback(err, XmNcancelCallback, UnknownUserCallback, err);

   XFlush(XtDisplay(toplevel));
   XmUpdateDisplay(toplevel);
   LogFailure();
   XmStringFree(okLabel);
   XmStringFree(message);
   XtFree(master);
   XtAppMainLoop(appContext);
}


/*
 * This is the 'Cancel' callback for the 'Invalid User' error dialog.
 * It removes the dialog, and then exits.
 */

/* ARGSUSED */
void 
#ifdef _NO_PROTO
UnknownUserCallback( w, clientData, callData )
        Widget w ;
        XtPointer clientData ;
        XtPointer callData ;
#else
UnknownUserCallback(
        Widget w,
        XtPointer clientData,
        XtPointer callData )
#endif /* _NO_PROTO */

{
   Widget err = (Widget)clientData;

   XtUnmanageChild(err);
   XFlush(XtDisplay(err));
   XmUpdateDisplay(err);
   exit(4);
}

void actionStatusCallback(
                        DtActionInvocationID id,
                        XtPointer   client_data,
                        DtActionArg *aap,
                        int         aac,
                        int         status )
{

    switch( status ) {
        case DtACTION_INVOKED:
	    /*
	     * There may still be error dialogs to post so we must return
	     * to mainLoop before exiting.
	     */
            if ( exitAfterInvoked )
		    XtAppAddTimeOut(appContext,
			10 , (XtTimerCallbackProc)CheckForDone,
		      NULL);
            break;
        case DtACTION_DONE:
        case DtACTION_FAILED:
        case DtACTION_CANCELED:
	    XtAppAddTimeOut(appContext,
		10 , (XtTimerCallbackProc)CheckForDone,
	      NULL);
            break;
        default:
            break;
    }

}
