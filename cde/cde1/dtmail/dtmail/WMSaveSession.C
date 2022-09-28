/*
 *      $Revision: 1.1 $
 *
 *      @(#)WMSaveSession.C	1.16 05/29/96
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 */

#pragma ident "@(#)WMSaveSession.C	1.16 96/05/29 SMI"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include <Dt/Session.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/MainW.h>

#include <DtMail/IO.hh>                    // SafeAccess...
#include <RoamApp.h>
#include <RoamMenuWindow.h>
#include <SendMsgDialog.h>

#include <Dt/Session.h>
#include <Dt/DtNlUtils.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>


// Uncomment this out only for debugging
//#define LOG_SESSION 


#ifdef LOG_SESSION
#define MAXLOGSZ	10000
static char *logfile = "/tmp/dtmail.log";
#endif

// Data struct to hold the iconic state of a window.
typedef struct _WmState {
	int state;
   	Window icon;
} WmState;

/*
 * This method implements the save yourself operation for
 * RoamApp object. The steps involved are:
 *	1. Get the session file name from Dt services
 *	   and open the session file. 
 *	2. For each toplevel window if the window is a
 *	   RoamApp or a SendMsgDialog, invoke the saveYourSelf
 *	   method for the object in question.
 *	3. Update the WM_COMMAND property. 
 */
 
void 
RoamApp::saveYourSelf(void)
{
    char *pathname = NULL, *filename = NULL;
    char **save_argv = NULL;
    int save_argc = 0;
    char **argv = NULL;
    int argc = 0;
    
#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif
    
    
    DtSessionSavePath(_w, &pathname, &filename);
    
    if(!(session_fp = fopen(pathname, "w+"))) {
	perror(pathname);
	XtFree((char *)pathname);
	XtFree((char *)filename);
	session_fp = NULL;
	return;
    }
    
    
    // Set permissions for read write for user and group.
    chmod(pathname, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);
    
    for(int i = 0; i < _numWindows; i++) {
	MainWindow *obj = _windows[i];
	const char *const name = obj->className();
	
	switch(name[0]) {
	  case 'R':
	    if(!strncmp(name,"RoamMenu", 8)) 
		((RoamMenuWindow *)obj)->saveYourSelf();
	    break;
	  case 'S':
	    if(!strncmp(name,"SendMsg", 7))
		((SendMsgDialog *)obj)->saveYourSelf();
	    break;
	  default:
	    break;      
	}
    }
    
    fclose(session_fp);
    session_fp = NULL;
    
    XtVaGetValues(_w, XtNargc, &argc, XtNargv, &argv, NULL);
    
    save_argv = (char **)XtMalloc(3*sizeof(char *));
    
    save_argv[save_argc++] = argv[0];
    save_argv[save_argc++] = "-session";
    save_argv[save_argc++] = filename;
    
#ifdef LOG_SESSION
    if(log) {
	for(int j=0; j < save_argc; j++)
	    fprintf(log, "WM_COMMAND[%d]: %s\n",j,save_argv[j]);
	fclose(log);
    }
#endif
    
    XSetCommand(_display, XtWindow(_w), save_argv, save_argc);
    
    /* There should be no code after this */
    
}

/*
 *	This method implements the restore operation for RoamApp.
 *	For each RoamMenuWindow it creates a new RoamMenuWindow,
 *	initializes it and manages it. For each SendMsgDialog, it
 *	creates a SendMsgDialog and loads the dead letter in it.
 */

void 
RoamApp::restoreSession(void)
{
    char buf[256];
    char *workspaces = new char[256];
    int iconic = 0; 
    int x = 0, y = 0; 
    int width = 0, height = 0;
    char * ptr;
    Atom * workspace_atoms = NULL;
    int num_workspaces=0;
    
    // If there is no session file or it has no contents, there
    // is no way to recover the session.  Exit!
    
    if (!session_fp) {
	_exit(0);
    }
    
#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif
    
#ifdef LOG_SESSION
    if(log) 
	fprintf(log, "In restore Session");
#endif
    
    if(session_fp == NULL) {
#ifdef LOG_SESSION
	if(log) 
	    fprintf(log, "session_fp is NULL");
#endif
	return;
    }
    
    for(;;) {
	if(fscanf(session_fp,"%s",buf) == EOF)
	    break;
	
	switch(buf[0]) {
	  case 'R':
#ifdef LOG_SESSION
	    if(log) 
		fprintf(log, "Character R found");
#endif
	    
	    if(!strncmp(buf,"RoamMenu", 8)) { 
#ifdef LOG_SESSION
		if(log) 
		    fprintf(log, "Is a RMW");
#endif
		
		RoamMenuWindow *rmw = NULL;
		
		if(fscanf(session_fp, "%s%s%d%d%d%d%d",
		    buf,workspaces,&iconic,&x,&y,&width,&height)!= 7) {
		    fscanf(session_fp,"[\n]",buf);
		    break;
		}
#ifdef LOG_SESSION
		if(log) 
		    fprintf(log, "Restore: RoamMenu %s %s %d %d %d %d %d\n",
			buf,workspaces,iconic,x,y,width,height);
#endif
		
		rmw =  new RoamMenuWindow(buf);
		rmw->initialize();

		// Use the appropriate DtWsm* method to restore the window
		// to the workspace it belonged to.
		// A problem though: the DtWsm* methods require the widget
		// to be realized.  We normally realize the window only when
		// we call manage.  If we call manage now, the window will
		// pop up in the default workspace and then *move* to the
		// appropriate workspace later.  To avoid this, we realize,
		// but not manage, the widget, call the DtWsm* method,
		// and then manage the widget.  This will cause the window
		// to appear only in the appropriate workspace.

		Widget bw = rmw->baseWidget();

		XtRealizeWidget(bw);
		XmUpdateDisplay(bw);

		// If the arguments are all valid, use them.
		// Else let the values in app-defaults take over.

		if (((iconic == 0) || 
		    (iconic == 1)) &&
		    x && y && width && height) {
		    XtVaSetValues(bw,
			XtNinitialState, 
			(iconic ? IconicState: NormalState), 
			XtNx, (Position)x, XtNy, (Position)y, 
			XtNwidth, (Dimension)width, XtNheight, 
			(Dimension)height, NULL);

  
		    if (workspaces)
		      {
			  do
			    {
				ptr = strchr (workspaces, '*');

				if (ptr != NULL) *ptr = NULL;
				
				workspace_atoms = (Atom *) XtRealloc (
					(char *)workspace_atoms, 
					sizeof (Atom) * (num_workspaces + 1));

				workspace_atoms[num_workspaces] = 
				    XmInternAtom (
					XtDisplay(bw),
					workspaces, True);
					
				num_workspaces++;

				if (ptr != NULL)
				  {
				      *ptr = '*';
				      workspaces = ptr + 1;
				  }
				    } while (ptr != NULL);

			  DtWsmSetWorkspacesOccupied (
					XtDisplay(bw),
					XtWindow (bw),
					workspace_atoms,
					num_workspaces);

			  XtFree ((char *) workspace_atoms);
			  workspace_atoms = NULL;
			  
		      }
		
		    if((*_mailview)[0] == NULL) {
			dtmail_mapped = 1;
			_mailview->append(rmw);
		    }  
		    
		}
		// Manage the RMW now.  This will cause the window
		// to appear in the correct workspace.

		rmw->manage();
	    }
	    break;
	  case 'S':
#ifdef LOG_SESSION
	    if(log) 
		fprintf(log, "Is an S");
#endif
	    
	    if(!strncmp(buf,"SendMsg", 7)) {
#ifdef LOG_SESSION
		if(log) 
		    fprintf(log, "Is a SMD");
#endif
		
		SendMsgDialog *smd = NULL;
		
		if(fscanf(session_fp, "%s%s%d%d%d%d%d",
		    buf,workspaces,&iconic,&x,&y,&width,&height)!= 7) {
		    fscanf(session_fp,"[\n]",buf);
		    break;
		}
#ifdef LOG_SESSION
		if(log) 
		    fprintf(log, "Restore: SendMsg %s %s %d %d %d %d %d\n",
			buf,workspaces,iconic,x,y,width,height);
#endif
		
		smd =  theCompose.SessiongetWin();
		smd->loadDeadLetter(buf);

		// Use the appropriate DtWsm* method to restore the window
		// to the workspace it belonged to.
		// A problem though: the DtWsm* methods require the widget
		// to be realized.  We normally realize the window only when
		// we call manage.  If we call manage now, the window will
		// pop up in the default workspace and then *move* to the
		// appropriate workspace later.  To avoid this, we realize,
		// but not manage, the widget, call the DtWsm* method,
		// and then manage the widget.  This will cause the window
		// to appear only in the appropriate workspace.

		Widget bw = smd->baseWidget();

		XtRealizeWidget(bw);
		XmUpdateDisplay(bw);

		
		// If the values are valid, use them.
		// Else let the ones in the app-defaults take over.
		
		if (((iconic == 0) || 
		    (iconic == 1)) &&
		    x && y && width && height) {
		    XtVaSetValues(bw,
			XtNinitialState, 
			(iconic ? IconicState: NormalState), 
			XtNx, (Position)x, XtNy, (Position)y, 
			XtNwidth, (Dimension)width, XtNheight, 
			(Dimension)height, NULL);
		    if (workspaces)
		      {
			  do
			    {
				ptr = strchr (workspaces, '*');

				if (ptr != NULL) *ptr = NULL;
				
				workspace_atoms = (Atom *) XtRealloc (
					(char *)workspace_atoms, 
					sizeof (Atom) * (num_workspaces + 1));

				workspace_atoms[num_workspaces] = 
				    XmInternAtom (XtDisplay(bw),
					workspaces, True);

				num_workspaces++;

				if (ptr != NULL)
				  {
				      *ptr = '*';
				      workspaces = ptr + 1;
				  }
			    } while (ptr != NULL);

			  DtWsmSetWorkspacesOccupied (
					XtDisplay(bw),
					XtWindow (bw),
					workspace_atoms,
					num_workspaces);

			  XtFree ((char *) workspace_atoms);
			  workspace_atoms = NULL;
			  
		      }
		}
		// Manage the SMD now.  This will cause the window
		// to appear in the correct workspace.

		smd->manage();
		break;
	    }
	  default:
#ifdef LOG_SESSION
	    if(log) 
		fprintf(log, "%s contains neither an R or an S", buf);
#endif
	    
	    break;      
	}
    }
#ifdef LOG_SESSION
    if(log)
	fclose(log);
#endif
} 


/*
 * This methods parses the -session argument and
 * returns the name of the session file if present.
 */

char *RoamApp::parseSessionArg(int *argc, char **argv)
{
    if(*argc < 3)
	return NULL;
    
    char *filename = NULL;
    
#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif
    
    for(int i=0; i < *argc; i++)  {
#ifdef LOG_SESSION
	if(log) 
	    fprintf(log,"restart argv[%d]: %s\n",i, argv[i]);
#endif
	if(!strcmp(argv[i], "-session")) {
	    if(i < *argc - 1) {
		filename = argv[i+1];
		for(int j=i+2; j < *argc; )
		    argv[i++] = argv[j++]; 
		*argc -= 2;
	    }
	    break;
	}
    }
    
#ifdef LOG_SESSION
    if(log)
	fclose(log);
#endif
    
    return filename;
}

/*
 * This method takes a file name and computes the full pathname 
 * using Dt services and then opens the session file pointed
 * to by pathname.
 */
 
void
RoamApp::openSessionFile(char *filename)
{
    struct stat s;

    if(filename == NULL)
	return;
    char *pathname = NULL;
    
    session_fp = NULL;

#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif

    // If the session file is an absolute path, just use it.
    if (filename[0] == '/') {
	pathname = strdup(filename);
    } else if (DtSessionRestorePath(_w, &pathname, filename) == FALSE) {
#ifdef LOG_SESSION
	if(log){
	    fprintf(log,"DtSessionRestorePath Failed on: %s\n",
		filename);
	    fclose(log);
	}
	return;
#endif
    }

    // Sometimes the session file can be empty.
    // This can cause dtmail to exist as a zombie process on login.
    // To prevent that, we stat the session file and if necessary,
    // set the session_fp to NULL.

    SafeStat(pathname, &s);

    if (s.st_size == 0) {
	session_fp = NULL;
    }
    else {
	if(!(session_fp = fopen(pathname, "r"))) {
	    perror(pathname);
	    session_fp = NULL;
	}
    }
#ifdef LOG_SESSION
    if(log) {
	fprintf(log,"Opened session file: %s\n",pathname);
	fclose(log);
    }
#endif
    if(pathname != NULL)
	XtFree((char *)pathname);

}

/*
 * This method implements the save yourself operation for
 * SendMsgDialog object. The steps involved are:
 *	1. Get the session file pointer from theRoamApp.
 *	2. Get the iconic state, mail folder, x, y, width, height and 
 *	   save it. 
 */
 
void
SendMsgDialog::saveYourSelf(void)
{
    WmState *iconic_state = NULL;
    FILE *fp = theRoamApp.sessionFile();
    Display *display = theRoamApp.display();
    int initialstate = 0;
    Atom wm_state, actual_type;
    unsigned long nitems, leftover;
    int actual_format;
    Position x=0, y=0;
    Dimension width = 0, height =0;
    Atom * ws_presence = NULL;
    char * workspace_name=NULL;
    unsigned long num_workspaces = 0;
    char *all_workspace_names;
    int j;

    
    if(fp == NULL)
	return;
    
    // If tthe SMD is not being used, return.
    if (!_msgHandle) {
	return;
    }
    
#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif
    
    // Create a dead letter if this one is currently in use
    
    doAutoSave();
    
    wm_state = XmInternAtom(display, "WM_STATE", False);
    XGetWindowProperty(display,XtWindow(_w), wm_state, 0L,
	(long)BUFSIZ, False, wm_state, &actual_type,
	&actual_format, &nitems, &leftover,
	(unsigned char **) &iconic_state);
    
    initialstate = (iconic_state->state == IconicState ? 1: 0);
    // Get the workspaces this window is present.

    if (DtWsmGetWorkspacesOccupied (display, XtWindow (_w),
                                  &ws_presence, &num_workspaces) == Success)
      {
         for (j = 0; j < num_workspaces; j++)
         {
            workspace_name = XGetAtomName (display, ws_presence[j]);
	    if (j == 0) {
		all_workspace_names = strdup((char *)workspace_name);
	    }
	    else {
		// We need to get some more memory before adding the extra strings.
		realloc(all_workspace_names, strlen(all_workspace_names) 
						    + strlen(workspace_name) + 2);
		(void) strcat(all_workspace_names, "*");
		(void) strcat(all_workspace_names, workspace_name);
	    }
	    
            XtFree ((char *) workspace_name);
         }

         XFree((char *)ws_presence);
     }

    XtVaGetValues(_w, XtNx, &x, XtNy, &y, XtNwidth, &width,
	XtNheight, &height, NULL);

    fprintf(fp,"SendMsg %s %s %d %d %d %d %d\n",
	_auto_save_path, all_workspace_names, initialstate, (int)x, (int)y, 
	(int)width, (int)height);
#ifdef LOG_SESSION
    if(log) {
	fprintf(log,"Save: SendMsg %s %s %s %d %d %d %d %d\n",
	    XtName(_w), _auto_save_path, , all_workspace_names, initialstate, 
	    (int)x, (int)y, (int)width, (int)height);
	fclose(log);
    }
#endif
    
}

/*
 * This method implements the save yourself operation for
 * RoamMenuWindow object. The steps involved are:
 *	1. Get the session file pointer from theRoamApp.
 *	2. Get the iconic state, mail folder, x, y, width, height and 
 *	   save it. 
 */
 
void
RoamMenuWindow::saveYourSelf(void)
{
    WmState *iconic_state = NULL;
    FILE *fp = theRoamApp.sessionFile();
    Display *display = theRoamApp.display();
    int initialstate = 0;
    Atom wm_state, actual_type;
    unsigned long nitems, leftover;
    int actual_format;
    Atom * ws_presence = NULL;
    char * workspace_name=NULL;
    unsigned long num_workspaces = 0;
    char *all_workspace_names;
    
#ifdef LOG_SESSION
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
#endif
    
    if(fp == NULL)
	return;
    
    wm_state = XmInternAtom(display, "WM_STATE", False);
    XGetWindowProperty(display,XtWindow(_w), wm_state, 0L,
	(long)BUFSIZ, False, wm_state, &actual_type,
	&actual_format, &nitems, &leftover,
	(unsigned char **) &iconic_state);
    
    initialstate = (iconic_state->state == IconicState ? 1: 0);

    // Get the workspaces this window is present.

    if (DtWsmGetWorkspacesOccupied (display, XtWindow (_w),
                                  &ws_presence, &num_workspaces) == Success)
      {
         for (int j = 0; j < num_workspaces; j++)
         {
            workspace_name = XGetAtomName (display, ws_presence[j]);
	    if (j == 0) {
		all_workspace_names = strdup((char *)workspace_name);
	    }
	    else {
		(void) strcat(all_workspace_names, "*");
		(void) strcat(all_workspace_names, workspace_name);
	    }
	    
            XtFree ((char *) workspace_name);
         }

         XFree((char *)ws_presence);
      }


    fprintf(fp,"RoamMenu %s %s %d %d %d %d %d\n",
	_mailbox_fullpath, all_workspace_names, initialstate, (int)_x, (int)_y, 
	(int)_width, (int)_height);
#ifdef LOG_SESSION
    if(log) {
	fprintf(log,"Save: RoamMenu %s %s %s %d %d %d %d %d\n",
	    XtName(_w), _mailbox_fullpath, all_workspace_names, initialstate,
	    (int)_x, (int)_y, (int)_width, (int)_height);
	fclose(log);
    }
#endif
    
}

/*
 * This method initializes the session management for
 * RoamApp by adding a WM protocol callback for WM_SAVE_YOURSELF
 * protocol.
 */

void 
RoamApp::initSession(void)
{
    Atom wm_save_yourself;
    
#ifdef LOG_SESSION
    struct stat sbuf;
    FILE *log = NULL;
    if(!(log = fopen(logfile,"a+")))
	perror(logfile);
    
    // If the size of the logfile exceeds a max size, truncate it.
    if(stat(logfile, &sbuf) == 0) {
	if(sbuf.st_size > MAXLOGSZ)
	    truncate(logfile, 0);
    }
#endif
    
    session_fp = NULL;
    
    wm_save_yourself  = XmInternAtom(_display, "WM_SAVE_YOURSELF", False);
    
    XmAddWMProtocols(_w, &wm_save_yourself, 1);
    XmAddWMProtocolCallback(_w, wm_save_yourself, 
	saveSessionCB, (XtPointer)this);  	

#ifdef LOG_SESSION
    if(log) {
	time_t tloc;
	
	if(time(&tloc) == -1) {
	    perror("initSession:time:");
	} else
	    fprintf(log,"Session Initialized at: %s\n", 
		ctime(&tloc));
	fclose(log);
    }
#endif
    
}

/*
 * WM protocol callback for WM_SAVE_YOURSELF
 */

void 
RoamApp::saveSessionCB(Widget , XtPointer client, XtPointer )
{
    RoamApp *obj = (RoamApp *)client;

    obj->saveYourSelf();
}

