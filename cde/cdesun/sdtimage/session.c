#ifndef lint
static char sccsid[] = "@(#)session.c 1.4 95/02/03";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Dt/Session.h>
#include "imagetool.h"
#include "ui_imagetool.h"


/*  Structure used on a save session to see if a dt is iconic  */
typedef struct
{
   int state;
   Window icon;
} WM_STATE;


/***************************************************************************
 *                                                                         *
 * Routine:   SaveSession                                                  *
 *                                                                         *
 * Purpose:   save state information for session management                *
 **************************************************************************/
void
SaveSessionCallback (
        Widget w,
        XtPointer clientData,
        XtPointer callbackArg )
{
    char *path, *name;
    FILE *fp;
    int  n;
    Position x,y;
    Dimension width, height;
    WM_STATE *wmState;
    Atom wmStateAtom, actualType;
    int actualFormat;
    unsigned long nitems, leftover;
    Atom command_atom;
    int command_len;
    char *command;
    Boolean save_session = True;


    if (!DtSessionSavePath (base->top_level, &path, &name))
	save_session = False;

    /*  Create the session file  */

    if (save_session == True) {
      if (!(fp = fopen(path, "w+"))) {
	perror (path);
        XtFree ((char *)path);
        XtFree ((char *)name);
      }

      chmod(path, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);

    /* The initial set up is done on the file.  From here on 
       out the application should write out a set of resource 
       definitions that will later be read in.  These resource 
       definitions will be used to restore the syate of the tool 
       when the session is restarted. */


    /*  Getting the WM_STATE property to see if iconified or not */

      wmStateAtom = XInternAtom(XtDisplay(base->top_level), "WM_STATE", False);

      XGetWindowProperty (XtDisplay(base->top_level), XtWindow(base->top_level),
			  wmStateAtom, 0L,
                          (long)BUFSIZ, False, wmStateAtom, &actualType,
                          &actualFormat, &nitems, &leftover,
                          (unsigned char **) &wmState);

      if (wmState->state == IconicState)
          fprintf(fp, "*iconic: True\n");
      else
          fprintf(fp, "*iconic: False\n");

      /*** Get and write out the geometry info for our Window ***/

      x = XtX(base->top_level);
      y = XtY(base->top_level);
      width = XtWidth(base->top_level);
      height = XtHeight(base->top_level);

      fprintf(fp, "*x: %d\n", x);
      fprintf(fp, "*y: %d\n", y);
      fprintf(fp, "*width: %d\n", width);
      fprintf(fp, "*height: %d\n", height);

    /* This is the end of client resource writing.  At this point 
       the client will reset the WM_COMMAND property on it's top 
       level window to include the original starting parameters as 
       well as the -session flag and the file to restore with. */

    }

    command_atom = XInternAtom (XtDisplay(base->top_level), "WM_COMMAND", False);
    /*  Generate the reinvoking command and add it as the property value */

    if (save_session)
      command_len = prog->wm_cmdstrlen + strlen("-session") + strlen(name) + 2;
    else
      command_len = prog->wm_cmdstrlen + 2;

    command = XtMalloc (sizeof (char) * command_len);
    memcpy(command, prog->wm_cmdstr, prog->wm_cmdstrlen);
    command_len = prog->wm_cmdstrlen;
 
    if (save_session) {
      memcpy(command + command_len, "-session", 8);
      command_len += 8;
      command[command_len] = NULL;
      command_len++;
      memcpy(command + command_len, name, strlen(name));
      command_len += strlen(name);
    }
	
    XChangeProperty (XtDisplay(base->top_level), XtWindow (base->top_level), 
		command_atom, XA_STRING, 8, PropModeReplace, 
		(unsigned char *)command,
                command_len);

    free(command);

    /* Note the bogus use of XtFree here.  DtSessionSavePath requires that
       the returned strings be freed this way.  Yick. */

    if (save_session) {
      XtFree ((char *)path);
      XtFree ((char *)name);
      fclose(fp);
    }

}

/***************************************************************************
 *                                                                         *
 * Routine:   GetSessionInfo                                               *
 *                                                                         *
 * Purpose:   get session information                                      *
 **************************************************************************/

void
GetSessionInfo(sessionfile)
     char *sessionfile;
{
    char *path;

    if (sessionfile == NULL)
        return;

    /***  Open the resource database file ***/

    /* TopLevel is used because Shell isn't created yet...     */
    /* okay because it only uses it to get a display, not a window    */

    if (DtSessionRestorePath(base->top_level, &path, sessionfile) == False)
        return;

    if (path != sessionfile)
        XtFree(path);

}
