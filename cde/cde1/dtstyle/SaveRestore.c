/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        SaveRestore.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Controls the Dtstyle Save/Restore functionality
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/*$XConsortium: SaveRestore.c /main/cde1_maint/2 1995/10/23 11:26:48 gtsang $ */
/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/

#include <stdio.h>

#include <X11/Xlib.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>

#include <Dt/DialogBox.h>

#include <Dt/HourGlass.h>
#include <Dt/UserMsg.h>

#include "Main.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* include extern functions              */
/*+++++++++++++++++++++++++++++++++++++++*/
#include "SaveRestore.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* Local #defines                        */
/*+++++++++++++++++++++++++++++++++++++++*/
#define MSG1  ((char *)GETMESSAGE(10, 1, "Check file permissions.")) 
#define MSG2  ((char *)GETMESSAGE(10, 3, "%s is the file that would have been used to save your session\n"))


/************************************************************************
 *
 *  saveSessionCB
 *      Creates a file as a resource data base, and writes out all
 *  info needed to save our current state.  This info will be used
 *  later by restoreSession to start up the help system in the exact
 *  state in which we saved the session.
 *
 ************************************************************************/
void 
#ifdef _NO_PROTO
saveSessionCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
saveSessionCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  char *longpath, *name;
  int fd, n; 
  char *xa_CommandStr[3];
  unsigned char *data = NULL;
  char *tmpStr;

#ifdef _SUN_OS
  char *noCommands = NULL;
#endif /*  _SUN_OS */

  /* get the root window property of SaveMode */
  GetSessionSaveMode(&data );
  
  /* if the property is "home" don't save dtstyle */
  if(strcmp((char *)data, "home") == 0)
  {
#ifdef _SUN_OS   /*  Sun doesn't like "NULL" when expecting "char **" */
     XSetCommand(style.display, XtWindow(w), &noCommands, 0);
#else
     XSetCommand(style.display, XtWindow(w), NULL, 0);
#endif /* _SUN_OS */
     return;
  }

  /* Xt may not pass a widget as advertised */
  if(!XtIsShell(w))
    DtSessionSavePath(XtParent(w), &longpath, &name);
  else
    DtSessionSavePath(w, &longpath, &name);

  /*  Create the session file  */
  if ((fd = creat (longpath, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP)) == -1)
    {
      tmpStr = (char *)XtMalloc(strlen(MSG2) + strlen(longpath)+ 1);
      sprintf(tmpStr, MSG2, longpath);
      _DtSimpleErrnoError(progName,DtError, MSG1, tmpStr, NULL);   
      XtFree(tmpStr);
      XtFree ((char *)longpath);
      XtFree ((char *)name);
      return;
    }

  /*write out each needed resource*/
  saveMain(fd);
  saveColor(fd);
  saveColorEdit(fd);
  saveFonts(fd);
  saveBackdrop(fd);
  saveKeybd(fd);
  saveMouse(fd);
  saveAudio(fd);
  saveScreen(fd);
  saveDtwm(fd);
  saveStartup(fd);
  
  close (fd);

/*If this is a session manager requested saveYrSlf, need to put dialogs in*/
/*recoverable state, and make them consistent with server (etc) state.*/
/*Have to do it before exit so SM gets corrected server state.*/
  callCancels();

  n = 0;
  xa_CommandStr[n] = style.execName; n++;
  xa_CommandStr[n] =  "-session"; n++;
  xa_CommandStr[n] = name; n++;

  XSetCommand(style.display, XtWindow(w), xa_CommandStr, n);
  XtFree ((char *)longpath);
  XtFree ((char *)name);

/*  Don't exit yet, SM needs time to get the new commandStr.*/
}


/************************************************************************
 *
 *  RestoreSession
 *      Open the file as a resource data base, and use the data to
 *      set the dialog position and (optionally) size.
 *
 ************************************************************************/

Boolean 
#ifdef _NO_PROTO
restoreSession( shell, name )
        Widget shell ;
        char *name ;
#else
restoreSession(
        Widget shell,
        char *name )
#endif /* _NO_PROTO */
{
  XrmDatabase db;
  char *tmpStr;
  char *longpath;
  Boolean status;

  status = DtSessionRestorePath(shell, &longpath, name);
  if (!status)
      return(1);
  
  /*  Open the file as a resource database */
  if ((db = XrmGetFileDatabase (longpath)) == NULL) {
    tmpStr = (char *)XtMalloc(strlen(MSG2) + strlen(longpath)+ 1);
    sprintf(tmpStr, MSG2, longpath);
    _DtSimpleErrnoError(progName, DtError, MSG1, tmpStr, NULL);   
    XtFree(tmpStr);
    XtFree(longpath);
    return(1);
  }

  restoreMain(shell, db);   /* Must happen first */

  /* Now, Restore the dialogs */
  restoreColor(shell, db);
  restoreColorEdit(shell, db);
  restoreFonts(shell, db);
  restoreBackdrop(shell, db);
  restoreKeybd(shell, db);
  restoreMouse(shell, db);
  restoreAudio(shell, db);
  restoreScreen(shell, db);
  restoreDtwm(shell, db);
  restoreStartup(shell, db);

  XrmDestroyDatabase(db);

  XtFree(longpath);
  return(0);
}
