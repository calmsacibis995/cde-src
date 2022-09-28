/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           dtaclafs.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    dtaclafs ... Display a dialog for editing Andrew 
 *                   File System Access Control Lists
 *
 *                   This program is generally invoked by the CDE File Manager,
 *                   dtfile, but can also be run from the command line.
 *
 *                   It accepts one argument, the complete path to the file.
 *                   Since AFS ACL's apply to a directory, any changes made
 *                   to the ACL will affect all files in the directory which
 *                   includes the input file.
 *
 *   FUNCTIONS: dirName
 *		displayError
 *		displayFatalError
 *		displayInfo
 *		displayWarning
 *		main
 *		yesOrNo
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Dt/DtNlUtils.h>
#include <Dt/EnvControl.h>

#include "dtaclafs.h"

#define ERRORSCRIPT CDE_INSTALLATION_TOP "/bin/dterror"  /* default script for posting error dialogs */

/* global variables */

#define EMSGMAX 256                      /* error message buffer */
char errorMessage[EMSGMAX];

typedef struct                           /* application resource data */
{
   Boolean  advancedMode;
   String   errorScript;
   Boolean  showNeg;
   Boolean  hideNeg;
} AppArgs;

AppArgs app_args;

Widget dtAclAfsShell;                    /* top-level shell widget */

XtAppContext appContext;                 /* used in yesOrNo */

Answer answer;                           /* answer given in question dialog */


/* local functions */
#ifdef _NO_PROTO
static void    displayFatalError();
static String  dirName();
#else
static void    displayFatalError(const String s);
static String  dirName(const String path);
#endif

 
/*---------
 *  main
 *-------*/
  
int 
#ifdef _NO_PROTO
main(argc, argv)
   int     argc;
   String  argv[];
#else
main (int argc, String argv[])
#endif  /* _NO_PROTO */

{ 
   char             path[FILENAME_MAX];
   String           directory;     /* directory is used except as as a label in the */
                                   /* dialog's heading         */
   DialogContext  * context = XtNew(DialogContext);
   String           s;

   /* application resource definitions */
   static XtResource resources[] =
   {
      {
        "advancedMode",
        "AdvancedMode",
        XmRBoolean,
        sizeof (Boolean),
        XtOffsetOf (AppArgs, advancedMode),
        XmRImmediate,
        (XtPointer) FALSE
       },
       {
        "errorScript",
        "ErrorScript",
        XmRString,
        sizeof (String),
        XtOffsetOf (AppArgs, errorScript),
        XmRString,
        (XtPointer) ERRORSCRIPT
       },
       {
        "showNeg",
        "ShowNeg",
        XmRString,
        sizeof (Boolean),
        XtOffsetOf (AppArgs, showNeg),
        XmRImmediate,
        (XtPointer) FALSE
       },
       {
        "hideNeg",
        "HideNeg",
        XmRString,
        sizeof (Boolean),
        XtOffsetOf (AppArgs, hideNeg),
        XmRImmediate,
        (XtPointer) FALSE
       }
    };

    /* option descriptions */
    static XrmOptionDescRec options[] =
    {
       { "-advanced", "advancedMode", XrmoptionNoArg, "True" },
       { "-showneg",  "showNeg",      XrmoptionNoArg, "True" },
       { "-hideneg",  "hideNeg",      XrmoptionNoArg, "True" }
    };


   /* I18N stuff */
   setlocale(LC_ALL, "");

   _DtEnvControl(DT_ENV_SET);

   XtSetLanguageProc(NULL, NULL, setlocale(LC_CTYPE, NULL));


   /* create top-level shell */
   s = GETMESSAGE(1, 2, "File Manager - AFS Folder Properties");

   dtAclAfsShell = XtVaAppInitialize (&appContext, "Dtfile",
                                      options, XtNumber(options),
                                      &argc, argv,
                                      NULL,
                                      XmNtitle, s,
                                      NULL);

   XtGetApplicationResources(dtAclAfsShell, &app_args, resources, XtNumber(resources), NULL, 0);

   
   if (argc != 2)
   {
      fprintf(stderr,
              GETMESSAGE(1, 1, "Bad argument list\nUsage: %s path <-advanced>\n"),
              argv[0]);
      exit (EXIT_FAILURE);
   }
   else
   {
      strncpy(path,argv[1],FILENAME_MAX);
      directory = dirName(path);
   }


   /* generate path-name independent parts of the dialog */
   createDialog(context);

   /* get the acl */
   if (getFileAcl(directory, &(context->initialAcl)) != 0)
   {
      strncpy(errorMessage,
              GETMESSAGE(1, 15, "Cannot get AFS Permissions for "),
              EMSGMAX);
      strncat(errorMessage,directory,EMSGMAX);
      strncat(errorMessage,":\n",EMSGMAX);
      strncat(errorMessage,strerror(errno),EMSGMAX);
      strncat(errorMessage,"\n",EMSGMAX);
      strncat(errorMessage,
              GETMESSAGE(1, 16, "It is possible this directory is not in AFS or that you do not have a valid token"),
              EMSGMAX);
      displayFatalError(errorMessage);  /* this does not return */
   }
   copyAcl(&(context->displayedAcl), &(context->initialAcl));
   
   /* set initial mode */
   if (app_args.advancedMode)
   {
      context->aclDisplay.currentMode = ADVANCED;
      XtManageChild(aclMgr(context->advNorm));
      XtManageChild(aclMgr(context->advNeg));
      XmToggleButtonSetState(context->advancedToggle, TRUE, FALSE);
   }
   else
   {
      context->aclDisplay.currentMode = BASIC;
      XtManageChild(aclMgr(context->basicNorm));
      XtManageChild(aclMgr(context->basicNeg));
      XmToggleButtonSetState(context->basicToggle,    TRUE, FALSE);
   }
   
   /* initially, are negative permissions shown or hidden? */
   /* command line arguments or resources can override the default, which */
   /*   is to show negative permissions window iff there are any to see   */
   if ( app_args.hideNeg || 
        ( (context->displayedAcl.nNeg == 0) && !app_args.showNeg) )
   {
      /* setting when widget is created is false, so callback is done explicitly */
      XmToggleButtonSetState(context->hideNegToggle, TRUE,  FALSE);                 
      XmToggleButtonSetState(context->showNegToggle, FALSE, TRUE);                 
      XtCallCallbacks(context->showNegToggle, XmNvalueChangedCallback, context);
   }
   else 
      XmToggleButtonSetState(context->showNegToggle, TRUE,  TRUE);

   /* add the path-dependent parts of the dialog */
   setValues(path, directory, context, context->initialAcl);

   /* say the magic words, and the dialog will appear! */
   XtRealizeWidget(dtAclAfsShell);
   
   /* set focus on default button */
   XmProcessTraversal(context->cancelButton, XmTRAVERSE_CURRENT);


   XtAppMainLoop(appContext);
        
}   /* end main */ 



/*--------------------------------------------------------------------
 *
 *  dirName
 *
 *  if a path points to a file, return the name of the directory
 *  if a path points to a directory, return it
 *
 *  assume that the file can be stat'd
 *  it is the caller's responsiblity to use XtFree to de-allocate
 *     memory allocated for the directory name
 *
 *-------------------------------------------------------------------*/


static String
#ifdef _NO_PROTO
dirName(path)
   const String path;
#else
dirName(const String path)
#endif

{
   struct stat  buf;
          char  dirname[FILENAME_MAX];
   int          i, len;

   strncpy(dirname,path,FILENAME_MAX);
   stat(path, &buf);
   if (! S_ISDIR(buf.st_mode) && (len=strlen(dirname)) > 1)
   {
      for (i = len - 1;
           i > 0  &&  (dirname[i] != '/');
           i--)
         ;
      dirname[i] = '\0';
   }

   return XtNewString(dirname);

}  /* end dirName */



/*--------------------------------
 * 
 *  displayFatalError
 *
 *  this function DOES NOT RETURN
 *
 *-------------------------------*/

static void
#ifdef _NO_PROTO
displayFatalError(s)
   const String s;
#else
displayFatalError(const String s)
#endif

{  

   execl(app_args.errorScript,
         app_args.errorScript,    /* argv[0] */
         s,                       /* argv[1] */
         NULL);

   /* in case there's a problem running the error script */
   fprintf(stderr, "%s", s);
   exit(EXIT_FAILURE);


}  /* end displayFatalError */


/*-------------------------------------------------------------------
 *
 *  displayError
 *
 *  display an error dialog
 *  a callback is provided to destroy the widget when OK is pressed
 *  the parent widget is taken from global variable "dtAclAfsShell"
 *
 *-----------------------------------------------------------------*/

void
#ifdef _NO_PROTO
displayError(s)
   const String s;
#else
displayError(const String s)
#endif

{
   XmString  xs, dialogTitle;
   int       n;
   Arg       args[20];
   Widget    dialog;

   xs          = XmStringCreateLtoR (s, XmFONTLIST_DEFAULT_TAG);
   dialogTitle = XmStringCreateLtoR (GETMESSAGE(1, 22, "AFS Folder Properties Error"), XmFONTLIST_DEFAULT_TAG);

   n = 0;
   XtSetArg(args[n], XmNmessageString, xs);					n++;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);		n++;
   XtSetArg(args[n], XmNdialogTitle, dialogTitle);				n++;
   dialog = XmCreateErrorDialog(dtAclAfsShell, "errorDialog", args, n);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), XmNactivateCallback, destroyCB, dialog);
   XmStringFree(xs);
   XtManageChild(dialog);


}  /* end displayError */



/*-------------------------------------------------------------------
 *
 *  displayInfo
 *
 *  display an information dialog
 *  a callback is provided to destroy the widget when OK is pressed
 *  the parent widget is taken from global variable "dtAclAfsShell"
 *
 *-----------------------------------------------------------------*/

void
#ifdef _NO_PROTO
displayInfo(s)
   const String s;
#else
displayInfo(const String s)
#endif

{
   XmString  xs, dialogTitle;
   int       n;
   Arg       args[20];
   Widget    dialog;

   xs          = XmStringCreateLtoR (s, XmFONTLIST_DEFAULT_TAG);
   dialogTitle = XmStringCreateLtoR (GETMESSAGE(1, 27, "AFS Folder Properties Information"), XmFONTLIST_DEFAULT_TAG);

   n = 0;
   XtSetArg(args[n], XmNmessageString, xs);					n++;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);		n++;
   XtSetArg(args[n], XmNdialogTitle, dialogTitle);				n++;
   dialog = XmCreateInformationDialog(dtAclAfsShell, "infoDialog", args, n);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), XmNactivateCallback, destroyCB, dialog);
   XmStringFree(xs);
   XtManageChild(dialog);


}  /* end displayInfo */



/*------------------------------------------------------------------
 *
 *  displayWarning
 *
 *  display a warning dialog
 *  a callback is provided to destroy the widget when OK is pressed
 *  the parent widget is taken from global variable "dtAclAfsShell"
 *
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
displayWarning(s)
   const String s;
#else
displayWarning(const String s)
#endif

{
   XmString  xs, dialogTitle;
   int       n;
   Arg       args[20];
   Widget    dialog;

   xs          = XmStringCreateLtoR (s, XmFONTLIST_DEFAULT_TAG);
   dialogTitle = XmStringCreateLtoR (GETMESSAGE(1, 28, "AFS Folder Properties Warning"), XmFONTLIST_DEFAULT_TAG);

   n = 0;
   XtSetArg(args[n], XmNmessageString, xs);					n++;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);		n++;
   XtSetArg(args[n], XmNdialogTitle, dialogTitle);				n++;
   dialog = XmCreateWarningDialog(dtAclAfsShell, "warnDialog", args, n);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), XmNactivateCallback, destroyCB, dialog);
   XmStringFree(xs);
   XtManageChild(dialog);


}  /* end displayWarning */



/*------------------------------------------------------------------
 *
 *  yesOrNo
 *
 *  display a question dialog
 *  event processing is done here until the question is answered
 *     see the example in X Vol 6A, Dan Heller, 1st Edition, p 151
 *
 *------------------------------------------------------------------*/

Answer
#ifdef _NO_PROTO
yesOrNo(s)
   const String s;
#else
yesOrNo(const String s)
#endif

{
   XmString  xs, dialogTitle;
   int       n;
   Arg       args[20];
   Widget    dialog;

   xs          = XmStringCreateLtoR (s, XmFONTLIST_DEFAULT_TAG);
   dialogTitle = XmStringCreateLtoR (GETMESSAGE(1, 43, "AFS Folder Properties Question"), XmFONTLIST_DEFAULT_TAG);

   answer = NONE;
   n = 0;
   XtSetArg(args[n], XmNmessageString, xs);					n++;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);		n++;
   XtSetArg(args[n], XmNdialogTitle, dialogTitle);				n++;
   dialog = XmCreateQuestionDialog(dtAclAfsShell, "questionDialog", args, n);
   XtVaSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
                 XmNlabelString, XmStringCreateLtoR(GETMESSAGE(1, 44, "Yes"), XmFONTLIST_DEFAULT_TAG),
                 NULL);
   XtVaSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
                 XmNlabelString, XmStringCreateLtoR(GETMESSAGE(1, 45, "No"), XmFONTLIST_DEFAULT_TAG),
                 NULL);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), XmNactivateCallback, questionDialogCB, YES);
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON), XmNactivateCallback, destroyCB, dialog);
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, questionDialogCB, NO);
   XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, destroyCB, dialog);
   XmStringFree(xs);
   XtManageChild(dialog);

   while (answer == NONE)
      XtAppProcessEvent(appContext, XtIMAll);

   return answer;


}  /* end displayWarning */

