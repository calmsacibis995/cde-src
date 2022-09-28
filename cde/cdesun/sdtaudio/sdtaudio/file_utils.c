/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)file_utils.c	1.9 97/05/16 SMI"


#include <errno.h>
#include <nl_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <Xm/Xm.h>

#include "extern.h"
#include "utils.h"
#include "sdtaudio.h"


extern int errno;

extern char *temp_rec_file;  /* Temporary file used in record. */

/* Function	:	SaveNeeded()
 |
 | Objective	:	Inquire if the user wants to save the file that
 |			was just modified.
 |
 | Arguments	:	WindowData	- sdtaudio data structure.
 |
 | Return value	:	Boolean True if user requested a save, false if
 |			otherwise.
 */
Boolean
SaveNeeded(WindowData *wd)
{
	char           buffer[1024];
	int            answer;
	XtAppContext   cntxt;

 	/* Get the application context. */
	cntxt =	XtDisplayToApplicationContext(XtDisplay(wd->mainWindow));

	/*
	 * Was the current audio file modified?  If so, prompt
	 * the user they want to save their work.
	 */
	if (!wd->audio_modified) 
		return(False);
	else {
		sprintf(buffer, "%s%s%s", appnameString, 
			catgets(msgCatalog, 1, 5, " - "),
			catgets(msgCatalog, 4, 5, "Unsaved Changes"));
		answer = AlertPrompt(wd->toplevel,
				     DIALOG_TYPE, XmDIALOG_QUESTION,
				     DIALOG_TITLE, buffer,
				     DIALOG_STYLE, 
				     XmDIALOG_FULL_APPLICATION_MODAL,
				     DIALOG_TEXT, 
				     catgets(msgCatalog, 4, 6,
					     "The recorded audio file has not been saved.\nDo you want to save your recording?"),
				     BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 3, 15,"Yes"),
				     BUTTON_IDENT, ANSWER_ACTION_2, 
					catgets(msgCatalog, 3, 16,"No"),
				     BUTTON_IDENT, ANSWER_CANCEL, 
					catgets(msgCatalog, 2, 40, "Cancel"),
				     NULL);

		/* 
		 * If anser is:
		 *	Yes	- save changes
		 *	No	- don't save
		 *	Cancel	- don't save, and don't continue with
		 *		  whatever operation comes next.
		 */
		if (answer == ANSWER_ACTION_1) {

			/* Create Save As dialog and save the file. */
			if (wd->saveas_dlog == NULL)
				if (!SDtAuCreateSaveAsDlog(wd)) {

					AlertPrompt(wd->toplevel,
					    DIALOG_TYPE, XmDIALOG_ERROR,
					    DIALOG_TITLE, catgets(msgCatalog, 3, 59, "Audio - Error"),
					    DIALOG_TEXT, catgets(msgCatalog, 3, 8, "Cannot bring up \"Save As\" dialog."),
					    BUTTON_IDENT, ANSWER_ACTION_1, 
						catgets(msgCatalog, 2, 62, 
							"Continue"),
					    NULL);
				return(False);
			}

			wd->wait_for_dialog = True;
			XtManageChild(wd->saveas_dlog);

			while (wd->wait_for_dialog)
				XtAppProcessEvent(cntxt, XtIMAll);

			if (wd->audio_modified)
				/* File still needs to be saved. */
				return(True);

		} else if (answer == ANSWER_CANCEL) {
			return(True);
		}
	}

	/* User doesn't want to save the file.  Unlink the temporary
	 * file and then return False.  Reset the audio_modified flag
	 * to false.
	 */
	wd->audio_modified = False;
	unlink(temp_rec_file);
	free(temp_rec_file);
	temp_rec_file = (char *) NULL;

	/* Prabhat ::	Free temp file name; else SDtDraw
	 * 		tries to draw data of invalid SAfile
	 */
	if (wd->name)
		XtFree(wd->name);
	wd->name = NULL;
	return(False);
}

/* Function	:	CanRead()
 |
 | Objective	:	Check to see if the specified file is available
 |			for reading.  If it is not, bring up an error 
 |			dialog.
 |
 | Arguments	:	Widget		- the sdtaudio application, the 
 |					  parent of the error dialog.
 |			char *		- name of file for reading
 |
 | Return value	:	Boolean True if file is ok for reading, i.e., the
 |			file exists and the user has permission to read
 |			the file.
 */
Boolean
CanRead(Widget parent, char *file)
{
	char msgbuff[MAXPATHLEN];


	if (file == (char *) NULL) {

		/* Calling function provided NULL - no file name
		 | was provided?
		 */
		AlertPrompt(parent,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_STYLE, 
			    XmDIALOG_FULL_APPLICATION_MODAL,
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 4, 2, "No file was specified!"),
			    BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 2, 37, "OK"),
			    NULL);
		return False;
	}


	if (access(file, F_OK | R_OK) == 0) {

		/* File exists, and user can read it. */
		return True;
	} else {

		/* Bring up a dialog that tells the user why they cannot
		 | read in the file.
		 */
		switch (errno) {

		case ENOENT:
			sprintf(msgbuff, catgets(msgCatalog, 4, 1, "%s\n\nThe specified file does not exist!"), file);
			AlertPrompt(parent,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, 
				    catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_STYLE, 
				    XmDIALOG_FULL_APPLICATION_MODAL,
				    DIALOG_TEXT, msgbuff,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 37, "OK"),
				    NULL);
			break;


		case EACCES:
			sprintf(msgbuff, catgets(msgCatalog, 4, 3, "%s\n\nCannot open the specified file.\nAccess denied."), file);
			AlertPrompt(parent,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, 
				    catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_STYLE, 
				    XmDIALOG_FULL_APPLICATION_MODAL,
				    DIALOG_TEXT, msgbuff,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 37, "OK"),
				    NULL);
			break;

		default:
			sprintf(msgbuff, catgets(msgCatalog, 4, 4, "%s\n\nCannot open the specified file.\nSystem failure."), file);
			AlertPrompt(parent,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, 
				    catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_STYLE, 
				    XmDIALOG_FULL_APPLICATION_MODAL,
				    DIALOG_TEXT, msgbuff,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 37, "OK"),
				    NULL);
			break;
		}

		return False;
	}
}

static Boolean
AllBlanks(char *str)
{
	int             i, len = 0;
	Boolean         ans = True;

	if (str)
		len = strlen(str);
	else
		ans = False;

	for (i = 0; i < len; i++) {
		if (str[i] != ' ') {
			ans = False;
			break;
		}
	}

	return ans;

}

static char    *
basename(char *path)
{
	register char  *p = strrchr(path, '/');

	if (p == (char *) NULL)
		p = path;
	else
		p++;

	return (p);
}


Boolean
CanWrite(Widget p, char *fname)
{
	struct stat     file_info;
	FILE           *fp = NULL;
	int             status;
	char            error[MAXPATHLEN];
	int		answer;

	if (strcmp(fname, "") == 0 || AllBlanks(basename(fname))) {

		AlertPrompt(p,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 1, 3,
						  "Audio - Save As"),
			    DIALOG_TEXT, catgets(msgCatalog, 4, 16,
					       "Please enter a file name."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return (False);

	} else if (stat(fname, &file_info) == 0) {

		if (S_ISDIR(file_info.st_mode)) {

			sprintf(error, catgets(msgCatalog, 4, 17, "%s is a folder.\nPlease enter a file name."), fname);

			AlertPrompt(p,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 1, 3,
							  "Audio - Save As"),
				    DIALOG_TEXT, error,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return (False);
		}
	}

	/*
	 * First try opening it for reading to check if file exists.
	 */
	if ((fp = fopen(fname, "r")) == NULL) {

		/*
		 * Couldn't open if for reading, but file can be 044, 004,
		 * 000 permissions so do a stat.
		 */
		if (access(fname, F_OK) == 0)
			return (0);

		/*
		 * File doesn't exist so open it for writing to check if
		 * user can write - user may be in a non-writeable
		 * directory, or what have you.
		 */
		if ((fp = fopen(fname, "w")) == NULL) {
			sprintf(error, catgets(msgCatalog, 4, 18, 
					       "Cannot save to %s."), fname);
			strcat(error, "\n");
			strcat(error, strerror(errno));
			AlertPrompt(p,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 1, 3, 
							  "Audio - Save As"),
				    DIALOG_TEXT, error,
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return (False);
		} else
			unlink(fname);
	} else {

		/* File exists.  Does the user want to over write the file?
		 * First, clear the file pointer.
		 */
		fclose(fp);

		/* Now, inquire from the user if they want to over write
		 * the file.
		 */
		answer = AlertPrompt(p,
			    DIALOG_TYPE, XmDIALOG_QUESTION,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 3, 
						  "Audio - Save As"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 3, 
						 "The file you specified exists.\nDo you want to overwrite the file?"),
			    BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 3, 15,"Yes"),
			    BUTTON_IDENT, ANSWER_ACTION_2, 
				catgets(msgCatalog, 3, 16,"No"),
			    NULL);
		
		/* 
		 * If anser is:
		 *	Yes	- overwrite file.
		 *	No	- don't save
		 */
		if (answer == ANSWER_ACTION_1)
			return(True);
		else
			return(False);
	}
	return (True);
}

/*
 * Converts XmString to char.  It's up to calling function to allocate enough
 * space in buf.
 */
void
ConvertCompoundToChar(XmString str, char *buf)
{
	XmStringContext context;
	char           *text, *tag, *p;
	XmStringDirection direction;
	Boolean         separator;

	buf[0] = '\0';

	if (!XmStringInitContext(&context, str)) {
		return;
	}

	/*
	 * p keeps a running pointer thru bug as text is read
	 */
	p = buf;
	while (XmStringGetNextSegment(context, &text, &tag, &direction,
				      &separator)) {

		/* Copy text into p and advance to end of string */
		p += (strlen(strcpy(p, text)));
		if (separator == True) {	/* if there's a separator ... */
			*p++ = '\n';
			*p = 0;	/* Add newline and null-terminate */
		}
		XtFree(text);	/* We're done with text, free it */
	}

	XmStringFreeContext(context);
}

