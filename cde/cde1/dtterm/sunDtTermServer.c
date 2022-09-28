#pragma ident	"@(#)sunDtTermServer.c	1.2	95/05/22 SMI"

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include "TermPrimDebug.h"
#include "TermView.h"
#include "TermPrimSetPty.h"
#ifdef  LOG_USAGE
#include "DtTermLogit.h"
#endif  /* LOG_USAGE */
#include <signal.h>
#include <errno.h>
#include <Dt/Service.h>
#include <Tt/tttk.h>
#include <Dt/DtP.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#define VENDOR                "CDE"
#define VERSION               "1.2"

static char * procId;
static Tt_pattern * DtTermToolTalkPattern = NULL;
static Boolean sendStopped = False;
XtInputId ProcessToolTalkInputId = 0;
static int ttFd;
char DTTERM_CLASS_NAME[] = "Dtterm";
char *displayString = NULL;
Widget refWidget;

#ifdef  TIMEOUT
#define STICKAROUND     15              /* 15 minutes... */
static XtIntervalId waitId = (XtIntervalId) 0;
#endif  /* TIMEOUT */

extern void
sunSetupIA(Widget w);

extern void AdjustForMenuBarHeight(Widget, Boolean);

Tt_message
SessionCallback( Tt_message msg, void * client_data, Tt_message contract);

Tt_callback_action
HandleTtRequest(Tt_message msg, Tt_pattern pat);

static void
dttermNewHandler(Tt_message msg);

#ifdef  TIMEOUT
static void TimeOut(XtPointer clientData, XtIntervalId *id);
#endif  /* TIMEOUT */

Boolean
FinalizeToolTalkSession( )
{
    Tt_status 	ttRc;
    int 	i;

    if (DtTermToolTalkPattern && tt_ptr_error(DtTermToolTalkPattern) == TT_OK) {
        ttRc = ttdt_session_quit(NULL, DtTermToolTalkPattern, 1);
        if (ProcessToolTalkInputId)
            XtRemoveInput(ProcessToolTalkInputId);
        return(True);
    }
    ttRc = ttdt_close(NULL, NULL, sendStopped);
    return(True);
}

Tt_message
SessionCallback( Tt_message msg, void * client_data, Tt_message contract)
{
    char 	*opString = tt_message_op(msg);
    Tttk_op 	op = tttk_string_op(opString);

    tt_free(opString);

    switch (op) {
        int i;

        default:
            break;
        case TTDT_QUIT:
            if (contract == 0) {
                tt_message_reply(msg);
                FinalizeToolTalkSession( );
                exit(0);
            }
          /* Should we send some type of tt failure message? */
            return 0;
    }
    return msg;
}

int
InitializeToolTalkSession(Boolean sendStarted)
{
    Tt_status 	ttstat;

    sendStopped = sendStarted;
    procId = ttdt_open(&ttFd,
                       DTTERM_CLASS_NAME,
                       VENDOR,
                       VERSION,
                       sendStarted);

    if (tt_ptr_error(procId) != TT_OK) {
        ttdt_close(NULL, NULL, sendStopped);
        return(0);
    }

    tt_ptype_declare("SDT_Terminal");
    tt_ptype_opnum_callback_add("SDT_Terminal", 0, HandleTtRequest);
    /*
     * If we were started by a message, the following call to
     * tttk_Xt_input_handler will process it.  Otherwise,
     * tttk_Xt_input_handler will just return.
     */
    tttk_Xt_input_handler(NULL, 0, 0);

    return(TT_OK);
}

int
FinishToolTalkInit(Widget topLevel)
{
    ProcessToolTalkInputId =
            XtAppAddInput(XtWidgetToApplicationContext(topLevel),
                          ttFd, (XtPointer)XtInputReadMask,
                          tttk_Xt_input_handler, procId);
    DtTermToolTalkPattern = ttdt_session_join(tt_default_session( ),
                                              SessionCallback,
                                              topLevel,
                                              NULL,
                                              1);
    if (tt_is_err(tt_ptr_error(DtTermToolTalkPattern))) {
        ttdt_close(NULL, NULL, sendStopped);
        return(0);
    }
    refWidget = topLevel;
    if (!displayString) {
	/* This dtterm -sdtserver must have been started up from either a
	 * session startup or the command line.  */
	displayString = DisplayString(XtDisplay(refWidget));
    }


}

Tt_callback_action
HandleTtRequest(Tt_message msg, Tt_pattern pat)
{
  char 	    *op;
  Tt_status  status;
  static Boolean exit_flag = False;

  op = tt_message_op( msg );
  status = tt_ptr_error( op );

  /* Let tttk_Xt_input_handler() Do The Right Thing */
  if (tt_is_err(status) || (op == 0))
    return TT_CALLBACK_CONTINUE;

  if (strcmp(op, "SDtTerminal_New") == 0)
    {
      if ((geteuid() == tt_message_uid(msg)) &&
          (getegid() == tt_message_gid(msg)))
        dttermNewHandler(msg); /*An existing process handles the request*/

      else if (tt_message_status(msg) == TT_WRN_START_MESSAGE)
        {
          /* A new process fails the request and the ToolTalk
             will not offer this message to the other processes,
             then exit. A better way is to set a flag and exit later */
          tttk_message_fail(msg, TT_DESKTOP_EACCES, 0, 1);

          if (exit_flag == True)
            _exit(1);
          else
            {
              exit_flag = True;
              return TT_CALLBACK_PROCESSED;
            }
	}
      else
        {
          /* An existing process rejects the request and ToolTalk
             will deliver the massage to the other processes.  */
          tttk_message_reject(msg, TT_DESKTOP_EACCES, 0, 1);
          return TT_CALLBACK_PROCESSED;
	}
    }
  else
    {
      tt_free(op);
      return TT_CALLBACK_CONTINUE;
    }

  tt_free(op);
  return TT_CALLBACK_PROCESSED;
}

/*ARGSUSED*/
static void
dttermNewHandler(
    Tt_message msg)
{
    Widget                shellWidget;
    int                   pid = -1;
    Arg                   arglist[20];
    int                   argcnt = 0;
    char                  *msgFile;
    char                  numArgs;
    int                   i, j, k;
    char                  *displayEnv, *newDisplayString;

    msgFile = tt_message_file(msg);
    if (tt_is_err(tt_ptr_error(msgFile))) msgFile = 0;
    numArgs = tt_message_args_count(msg);
    if (tt_is_err( tt_int_error(numArgs))) numArgs = 0;
    for (i = 0; i < numArgs; i++) {
	char *vtype, *val;

	vtype = tt_message_arg_type(msg, i);
	if ((vtype == 0) || (tt_is_err(tt_ptr_error(vtype)))) {
	    continue;
	}
	val = tt_message_arg_val(msg, i);
	if(strcmp(vtype, "-display") == 0) {
	    newDisplayString = XtNewString(val);
	}
	tt_free( val );
	tt_free( vtype );
    }

    if (!displayString) {
	/* This tt message is part of an action dtterm -server startup.  */
	displayString = newDisplayString;
	displayEnv = (char *)malloc(strlen("DISPLAY=") +
				    strlen(displayString) + 2);
	displayEnv[0]=NULL;
	strcat(displayEnv, "DISPLAY=");
	strcat(displayEnv, displayString);
	putenv(displayEnv);
	tt_free(msgFile);
	tt_message_reply(msg);
	tttk_message_destroy(msg);
	return;
    } else {

	char *screen = strrchr( displayString, '.' );
	char *newScreen = strrchr( newDisplayString, '.' );

	if (strcmp(screen, newScreen)) {
            /* The existing process rejects the request since
               its screen does not match with the screen of
               the new process */
	    tt_free(msgFile);
	    tt_message_reject(msg);
	    XtFree(newDisplayString);
	    return;
	}
	XtFree(newDisplayString);
    }

    argcnt = 0;
    XtSetArg(arglist[argcnt], XmNallowShellResize, True);
	    argcnt++;
    shellWidget = XtAppCreateShell((char *) 0, "Dtterm",
	    applicationShellWidgetClass, XtDisplay((Widget) refWidget),
	    arglist, argcnt);

    /* parse off messageFields and build the dttermview arglist... */
    argcnt = 0;

    /* create the dtterm... */
    CreateInstance(shellWidget, "dtTermView", arglist, argcnt, True);

    {
      int num_children = 0;
      WidgetList children = NULL;

      XtVaGetValues(shellWidget,
                    XmNnumChildren, &num_children,
                    XmNchildren, &children,
                    NULL);

      if (children && children[0])
        AdjustForMenuBarHeight(children[0], False);
    }

    XtRealizeWidget(shellWidget);

#ifdef sun
    sunSetupIA(shellWidget);
#endif

#ifdef  TIMEOUT
	    /* since we now have active instances, we can remove our
	     * wait timeout...
	     */
	    if (waitId) {
		XtRemoveTimeOut(waitId);
		waitId = (XtIntervalId) 0;
	    }
#endif  /* TIMEOUT */

    tt_free(msgFile);
    tt_message_reply(msg);
    tttk_message_destroy(msg);
}



Boolean
ServerStartSession(int argc, char **argv, Boolean loginShell,
		   char **commandToExec)
{
    return(InitializeToolTalkSession(True));
}

#ifdef  TIMEOUT
static void
TimeOut(
    XtPointer             clientData,
    XtIntervalId         *id
)
{
    /* if we have no instances active, go away... */
    if (InstanceCount <= 0) {
	exit(0);
    }

    /* otherwise, clear the waitId... */
    if (*id == waitId) {
	waitId = (XtIntervalId) 0;
    }
}
#endif  /* TIMEOUT */
