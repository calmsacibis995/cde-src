/**********************************<+>*************************************
***************************************************************************
**
**  File:        fileDlg.c
**
**  Project:     DT dtpad, a memo maker type editor based on the Dt Editor
**               widget.
**
**  Description:  Routines which manipulate the dialogs associated with
**                operations in the File menu.
**  -----------
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
*******************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "dtpad.h"
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/FileSB.h>
#include <Xm/Separator.h>
#include <Xm/MwmUtil.h>
#include <Dt/HourGlass.h>

#define MAX_DIR_PATH_LEN    1024

extern XmStringCharSet charset;

#define SAVE_CH ((char *)GETMESSAGE(5, 1, "Save changes to %s?"))
#define SAVE_CH_NONAME ((char *)GETMESSAGE(5, 2, "The current file has no name.\nDo you want to save it?"))
#define SAVE_CH_BUF ((char *)GETMESSAGE(5, 40, "Save changes?"))
#define SAVE_CH_DOC ((char *)GETMESSAGE(5, 41, "Save changes to \"%s?\""))
#define SAVE_DROP  ((char *)GETMESSAGE(5, 3, "Do you want to save the current copy of %s\nbefore you Edit %s?"))
#define MSG1 ((char *)GETMESSAGE(5, 5, "The file %s already exists.\nDo you want to over write that file\nwith the contents within this edit session? "))


/************************************************************************
 *			Forward Declarations
 ************************************************************************/
static void CreateFileSelectionDialog(
	Editor *pPad );
static void CreateSaveWarning(
	Editor *pPad );


/************************************************************************
 *  PostAlreadyExistsDlg - Setup and post dialog to determine whether
 *	the user wishes to clobber existing file or try saving it under a
 *	new name.
 *
 *	Inputs:	pPad->fileStuff.savingName - name of file to save
 ************************************************************************/
void 
PostAlreadyExistsDlg(
	Editor *pPad)
{
    Arg args[10];
    int n;
    char *tmpMess;
    XmString tmpMessStr;
    SaveAs *pSaveAs = &pPad->fileStuff.fileWidgets.saveAs;

    /* create the dialog if it is the first time */
    if (!pSaveAs->alrdy_exist) 
    {
	Pixel foreground;	/* dialog foreground */
	Pixel background;	/* dialog background */
	Pixmap pixmap;		/* dialog pixmap */
	XmString tempStr, tempStr2;
	char buf[256];

	n = 0;
	strcpy(buf, pPad->dialogTitle);
	strcat(buf, (char *)GETMESSAGE(5, 6, "Warning"));
	tempStr = XmStringCreateLtoR(buf, XmFONTLIST_DEFAULT_TAG);
	XtSetArg(args[n], XmNdialogTitle, tempStr); n++;
	XtSetArg (args[n], XmNokLabelString, XmStringCreateLtoR(_DtOkString,
								charset));  n++;
	tempStr2 = XmStringCreateLtoR((char *)GETMESSAGE(2, 1, "Help"),
					       XmFONTLIST_DEFAULT_TAG);
	XtSetArg (args[n], XmNhelpLabelString, tempStr2); n++;
	XtSetArg (args[n], XmNcancelLabelString, XmStringCreateLtoR(
						_DtCancelString,charset));  n++;
	pSaveAs->alrdy_exist = XmCreateMessageDialog (pPad->app_shell, "Warn", 
						      args, n);
	XmStringFree(tempStr);
	XmStringFree(tempStr2);
	n=0;
	XtSetArg(args[n], XmNmwmInputMode, 
		 MWM_INPUT_PRIMARY_APPLICATION_MODAL);n++;
	XtSetValues(XtParent(pSaveAs->alrdy_exist), args, n);


	n = 0;
	XtSetArg(args[n], XmNforeground, &foreground); n++;
	XtSetArg(args[n], XmNbackground, &background); n++;
	XtGetValues(pSaveAs->alrdy_exist, args, n);

	n = 0;
	pixmap = XmGetPixmap(XtScreen(pSaveAs->alrdy_exist), "warn_image",
					foreground, background);
	XtSetArg(args[n], XmNsymbolPixmap, pixmap); n++;
	XtSetValues(pSaveAs->alrdy_exist, args, n);

	XtAddCallback (pSaveAs->alrdy_exist, XmNokCallback, 
			(XtCallbackProc) AlrdyExistsOkCB,
			(XtPointer) pPad);
	XtAddCallback (pSaveAs->alrdy_exist, XmNcancelCallback, 
			(XtCallbackProc) AlrdyExistsCancelCB,
			(XtPointer) pPad);
	XtAddCallback (pSaveAs->alrdy_exist, XmNhelpCallback,
			(XtCallbackProc) HelpFileAlreadyExistsCB,
			(XtPointer) pPad);
		       
	XtRealizeWidget (pSaveAs->alrdy_exist);
    }

    tmpMess =
	(char *)XtMalloc(strlen(MSG1) + strlen(pPad->fileStuff.savingName) + 1);
    sprintf(tmpMess, MSG1, pPad->fileStuff.savingName);
    tmpMessStr = XmStringCreateLtoR(tmpMess, charset);
    n = 0;
    XtSetArg(args[n], XmNmessageString, tmpMessStr); n++;
    XtSetValues(pSaveAs->alrdy_exist, args, n);
    XtFree(tmpMess);
    XmStringFree(tmpMessStr);

    XtManageChild (pSaveAs->alrdy_exist);

}


/************************************************************************
 * SetSaveAsDirAndFile - Seeds the directory and file name fields of
 *	the SaveAs File Selection Box.
 *
 *    The possibilities for an initial directory are:
 *
 *	1.  The user's previously entered path.
 *	2.  There was a -directory specified.
 *	3.  There was a -saveAsDir specified.
 *	4.  We're editing a file, so use its path.
 *	5.  Use the CWD of our process.
 *
 ************************************************************************/
void
SetSaveAsDirAndFile(Editor *pPad)
{
    char dirbuf[MAX_DIR_PATH_LEN], *currentVal, *directoryVal,
	 *lastSlash, *pColon;
    int firstSelect = -1, lastSelect = -1;
    Widget textField;
    Arg args[5];
    register int n;
    XmString dirString;
    dirbuf[0] = (char) '\0';

    /* -----> Get the file name field from the previous SaveAs FSB.
     *        XXX - Should use GetValues to get XmNdirSpec. */
    textField = XmFileSelectionBoxGetChild(
			pPad->fileStuff.fileWidgets.saveAs.saveAs_form,
			XmDIALOG_TEXT);
    currentVal = XmTextFieldGetString(textField);

    /* -----> If the user previously entered a file name via the SaveAs FSB,
     *        use its values. */
    if (currentVal != (char *)NULL && *currentVal != (char)'\0') {
	/* -----> If the file name begins with '/', it has a pathname and we
	 *	  can use it for the initial directory */
	if ( currentVal[0] == '/' ) {
	    strcpy(dirbuf, currentVal);
	    /* Trim off everything behind the last slash */
	    if ((lastSlash = MbStrrchr(dirbuf, '/')) != (char *)NULL) {
		*(lastSlash + 1) = (char)'\0';
	    }
	} else {
	    /* -----> The filename does not have a directory, so use the value
	     *        from the directory field. */
	    n=0;
	    XtSetArg( args[n], XmNdirectory, &dirString ); n++;
	    XtGetValues(pPad->fileStuff.fileWidgets.saveAs.saveAs_form, args, n);
	    directoryVal = (char *) XtMalloc( sizeof(char) * 
					 (MAX_DIR_PATH_LEN + 1) );
	    XmStringGetLtoR( dirString, XmFONTLIST_DEFAULT_TAG, &directoryVal );
	    strcpy(dirbuf, directoryVal);
	    XmStringFree(dirString);
	    XtFree( directoryVal );
	}
	XtFree (currentVal);
    /* -----> No file name was not previously entered via a SaveAs FSB so use
     *        other defaults. */
    } else {
	if (pPad->xrdb.saveAsDir != (char *)NULL) {
	    strcpy(dirbuf, pPad->xrdb.saveAsDir);
	} else if (pPad->xrdb.directory != (char *)NULL) {
	    strcpy(dirbuf, pPad->xrdb.directory);
	} else if (pPad->fileStuff.pathDir[0] != (char)'\0') {
	    strcpy(dirbuf, pPad->fileStuff.pathDir);
	}
	/* -----> make sure dir ends in '/' */
	if (dirbuf[0] != (char )'\0') {
	    if (dirbuf[strlen(dirbuf) - 1] != (char)'/')
		strcat(dirbuf, "/");
	} else {
	    getcwd(dirbuf, MAX_DIR_PATH_LEN - 16);
	    strcat(dirbuf, "/");
	}
    }

    /* -----> set the SaveAs FSB directory mask */
    firstSelect = strlen(dirbuf);
    strcat(dirbuf, "[^.]*");
    dirString = XmStringCreateLtoR(dirbuf, XmFONTLIST_DEFAULT_TAG);
    n = 0;
    XtSetArg(args[n], XmNdirMask, dirString); n++;
    XtSetValues(pPad->fileStuff.fileWidgets.saveAs.saveAs_form, args, n);
    XmStringFree(dirString);
    dirbuf[firstSelect] = (char)'\0';

    /* -----> set the default file name and select it */
    strcpy(dirbuf, GETMESSAGE(5, 21, "UNTITLED"));
    lastSelect = strlen(dirbuf);
    XmTextFieldSetString(textField, dirbuf);
    XmTextFieldSetSelection(textField, 0, lastSelect, CurrentTime);
}


/************************************************************************
 * CreateNewLineToggles - used by CreateSaveAsDialog and CreateSaveWarning
 *	to create a radio box within 'parent' with two toggle buttons
 *	for adding or not adding new lines if word wrap	mode is on.
 ************************************************************************/
void
CreateNewLineToggles(
	Editor *pPad,
	Widget parent,
	ToggleWidgets *pToggleWidgets)
{
    Arg args[20];
    register int n;
    XmString label_string;

    /* -----> Create Radio Box */
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0);			n++;
    XtSetArg(args[n], XmNtraversalOn, True);			n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL );		n++;
    pToggleWidgets->newl_radio = XmCreateRadioBox(parent, "radioBox",
						args, n);

    /* -----> Create the toggle button for adding newlines. */
    label_string = XmStringCreateLtoR(((char *)GETMESSAGE(5, 44,
		"Add newline characters to the end of wrap-to-fit lines.")),
		charset);
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0);			n++;
    XtSetArg(args[n], XmNlabelString, label_string);		n++;
    pToggleWidgets->with_newl = XmCreateToggleButtonGadget(
					pToggleWidgets->newl_radio,
					"with_nl", args, n);
    XmStringFree(label_string);
    XtAddCallback(pToggleWidgets->with_newl, XmNvalueChangedCallback,
			SaveNewLinesCB, (XtPointer) pPad);
    XtManageChild(pToggleWidgets->with_newl);

    /* -----> Create the toggle button for not adding newlines. */
    label_string = XmStringCreateLtoR(((char *)GETMESSAGE(5, 45, 
"Do not add newlines.  Only line breaks created by [Return]\nwill be preserved.")), 
					charset);
    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0);			n++;
    XtSetArg(args[n], XmNlabelString, label_string); n++;
    pToggleWidgets->without_newl = XmCreateToggleButtonGadget(
					pToggleWidgets->newl_radio, 
					"without_nl", args, n);
    XmStringFree(label_string);
    XtAddCallback(pToggleWidgets->without_newl, XmNvalueChangedCallback, 
			SaveNewLinesCB, (XtPointer) pPad);
    XtManageChild(pToggleWidgets->without_newl);

    /* -----> Set the default to add newlines (if wordwrap is on) */
    pPad->fileStuff.saveWithNewlines = True;
    XmToggleButtonGadgetSetState(pToggleWidgets->with_newl, True, False);

    /* -----> Display the radio box containing the two toggle buttons only
     *        if wordwrap is on */
    if (pPad->xrdb.wordWrap == True)
	XtManageChild(pToggleWidgets->newl_radio);

}


/************************************************************************
 * CreateSaveAsDialog
 ************************************************************************/
void
CreateSaveAsDialog(Editor *pPad)
{
    Arg args[20];
    register int n;
    XmString label_string;
    char buf[256];
    SaveAs *pSaveAs = &pPad->fileStuff.fileWidgets.saveAs;

    _DtTurnOnHourGlass(pPad->app_shell);

    /* -----> Create the FileSelectionDialog */
    strcpy(buf, pPad->dialogTitle);
    if (pPad->xrdb.nameChange) {
	strcat(buf, (char *)GETMESSAGE(5, 7, "Save As"));
    } else {
	strcat(buf, (char *)GETMESSAGE(5, 43, "Copy To File"));
    }
    label_string = XmStringCreateLtoR(buf, XmFONTLIST_DEFAULT_TAG);
    n = 0;
    XtSetArg(args[n], XmNdialogTitle, label_string); n++;
    pSaveAs->saveAs_form = XmCreateFileSelectionDialog(pPad->app_shell,
							"save_dialog", args, n);
    XmStringFree(label_string);

    /* -----> Add the FSB cancel, ok, and help callbacks */
    XtAddCallback(pSaveAs->saveAs_form, 
			XmNcancelCallback, 
			(XtCallbackProc) SaveAsCancelCB, 
			(XtPointer) pPad);
    XtAddCallback(pSaveAs->saveAs_form, 
			XmNokCallback, 
			(XtCallbackProc) SaveAsOkCB, 
			(XtPointer) pPad);
    XtAddCallback(pSaveAs->saveAs_form, 
			XmNhelpCallback, 
			(XtCallbackProc)HelpSaveAsDialogCB, 
		 	(XtPointer) pPad);

    /* ----->  Don't show the filter stuff when saving 
     *XtUnmanageChild(XmFileSelectionBoxGetChild(pSaveAs->saveAs_form, 
     *						XmDIALOG_FILTER_TEXT));
     *XtUnmanageChild(XmFileSelectionBoxGetChild(pSaveAs->saveAs_form,
     *						XmDIALOG_FILTER_LABEL));
     */

    /* -----> Set the useAsyncGeo on the shell */
    n = 0;
    XtSetArg(args[n], XmNuseAsyncGeometry, True); n++;
    XtSetValues (pPad->app_shell, args, n);

    /* -----> Set the ok button to the default for the bulletin board.
     *        This causes the return key from the text widget to be properly
     *        processed. */
    XtSetArg(args[0], XmNdefaultButton, 
		XmFileSelectionBoxGetChild(pSaveAs->saveAs_form,
		XmDIALOG_OK_BUTTON)); 
    XtSetValues (pSaveAs->saveAs_form, args, 1);

    /* -----> Create a radio box with two toggle buttons for either adding
     *        or not adding new lines if word wrap mode is on. */
    CreateNewLineToggles(pPad,
			pSaveAs->saveAs_form,		/* parent widget */
			&pSaveAs->toggleWidgets);	/* new widgets */

    _DtTurnOffHourGlass(pPad->app_shell);
}


/************************************************************************
 * ExtractAndStoreDir - extracts the directory from the specified
 *	/dir/filename and stores it in pPad->fileStuff.pathDir.
 ************************************************************************/
void
ExtractAndStoreDir(
	Editor *pPad,
	char *fileName)
{
    char *pFirstSlash, *pLastSlash;

    pFirstSlash = MbStrchr(fileName, '/');
    if (pFirstSlash != (char *)NULL) {
	pLastSlash = MbStrrchr(fileName, '/');
	strncpy(pPad->fileStuff.pathDir, pFirstSlash,
		pLastSlash - pFirstSlash + 1);
	pPad->fileStuff.pathDir[pLastSlash - pFirstSlash + 1] = '\0';
    }

}


/************************************************************************
 *  GetFileName - Ask for a file name via an Xm File Selection Box
 ************************************************************************/
void 
GetFileName(
	Editor *pPad,
	XmString title)
{
    Arg al[10];
    int ac;
    Select *pSelect = &pPad->fileStuff.fileWidgets.select;
    FileStuff *pStuff = &pPad->fileStuff;

    _DtTurnOnHourGlass(pPad->app_shell);

    /* -----> create the FSB if it doesn't exits */
    if (pSelect->file_dialog == (Widget) NULL) {
	CreateFileSelectionDialog(pPad);
    }

    /* -----> set the FSB "ok" and "help" button callbacks */
    XtRemoveAllCallbacks(pSelect->file_dialog, XmNokCallback);
    XtAddCallback(pSelect->file_dialog, XmNokCallback,
			(XtCallbackProc) pStuff->pendingFileFunc, 
			(XtPointer)pPad);
    pStuff->pendingFileFunc = (void(*)())NULL;
    XtRemoveAllCallbacks(pSelect->file_dialog, XmNhelpCallback);
    XtAddCallback(pSelect->file_dialog, XmNhelpCallback,
			(XtCallbackProc) pStuff->pendingFileHelpFunc, 
			(XtPointer)pPad);
    pStuff->pendingFileHelpFunc = (void (*)())NULL;

    /* -----> seed FSB directory from either fileName or -directory resource */
    if (pStuff->pathDir[0] == (char)'\0') {
	if (pStuff->fileName != (char *)NULL) {
	    ExtractAndStoreDir(pPad, pStuff->fileName); /*set pStuff->pathDir*/
	} else if (pPad->xrdb.directory != (char *)NULL) {
	    strcpy(pStuff->pathDir, pPad->xrdb.directory);
	}
    }
    if (pStuff->pathDir[0] != (char)'\0') {
	XmString tmpStr = XmStringCreateLtoR(pStuff->pathDir, charset);
	ac = 0;
	XtSetArg(al[ac], XmNdirectory, tmpStr); ac++;
	XtSetValues(pSelect->file_dialog, al, ac);
	XmStringFree(tmpStr);
    }

    /* -----> find the seeded directory */
    /* For some reason the following call was commented out - there was no explanation
       why it was commented out. To fix bug 1222644, it was uncommented.
    */
    XmFileSelectionDoSearch(pSelect->file_dialog, NULL);

    /* -----> set the FSB title (as passed in) */
    ac = 0;
    XtSetArg(al[ac], XmNdialogTitle, title); ac++;
    XtSetValues(pSelect->file_dialog, al, ac);

    /* -----> delete the first entry from the list of dirs
     * if(!pPad->nodo) {
     *     XmListDeletePos(pSelect->dir_list, 1);
     * }
     */
    pPad->nodo = False;

    /* -----> manage the FSB */
    XtManageChild (pSelect->file_dialog);
    XmProcessTraversal(XmFileSelectionBoxGetChild(pSelect->file_dialog,
					XmDIALOG_TEXT), XmTRAVERSE_CURRENT);
    XSync(pPad->display, 0);

    _DtTurnOffHourGlass(pPad->app_shell);
}


/************************************************************************
 * CreateFileSelectionDialog - 
 ************************************************************************/
static void 
CreateFileSelectionDialog(
        Editor *pPad )
{
    Arg al[10];             /* arg list */
    register int ac;        /* arg count */
    XmString tmpStr;
    Select *pSelect = &pPad->fileStuff.fileWidgets.select;

    ac = 0;
    tmpStr = XmStringCreateLtoR((char *)GETMESSAGE(2, 1, "Help"),
					XmFONTLIST_DEFAULT_TAG);
    XtSetArg (al[ac], XmNhelpLabelString, tmpStr);	ac++;

    pSelect->file_dialog = XmCreateFileSelectionDialog(pPad->app_shell,
					"file_sel_dlg", al, ac);
    XmStringFree(tmpStr);

    XtAddCallback(pSelect->file_dialog, XmNcancelCallback,
			(XtCallbackProc) CancelFileSelectCB, (XtPointer)pPad );

    XtRealizeWidget (pSelect->file_dialog);
    XSync(pPad->display, 0);

/* XXX
 *  pSelect->dir_list = FS_DirList(pSelect->file_dialog);
 */

    pSelect->filter_text = XmFileSelectionBoxGetChild(pSelect->file_dialog,
					XmDIALOG_FILTER_TEXT);
    pSelect->select_text = XmFileSelectionBoxGetChild(pSelect->file_dialog, 
					XmDIALOG_TEXT);
    pSelect->file_list = XmFileSelectionBoxGetChild(pSelect->file_dialog,
					XmDIALOG_LIST);
}


/************************************************************************
 * CreateSaveWarning - Builds a PromptDialog to ask if the current contents
 *	are to be saved prior to some operation.  Also used to ask if new
 *	lines characters are to be inserted when word wrap is on.
 *
 *	PromptDialog
 *	    (message)				(unmanaged)
 *	    (text input region)			(unmanaged)
 *	    Form				work area
 *		RowColumn			horz orientation
 *		    LabelGadget			exclamation symbol
 *		    LabelGadget			prompt text
 *		Separator
 *		RadioBox			contains two new line toggles
 *	    (buttons)				OK, Apply, Cancel, Help
 *
 *	The RadioBox is managed only if wordwrap is on.
 *	
 ************************************************************************/
static void 
CreateSaveWarning(
        Editor *pPad )
{
    Arg al[10];			/* arg list */
    register int ac;		/* arg count */
    char buf[256];
    Widget w, kid[5];
    Pixel foreground, background;
    Pixmap pixmap;		/* dialog pixmap */
    XmString tmpStr1, tmpStr2, tmpStr3, tmpStr4, tmpStr5;
    Select *pSelect = &pPad->fileStuff.fileWidgets.select;

    /* -----> Create a SelectionBox PromptDialog with appropriate title and
     *        button labels */
    tmpStr1 = XmStringCreateLtoR(((char *)GETMESSAGE(5, 14, "No")), charset);
    tmpStr2 = XmStringCreateLtoR(((char *)GETMESSAGE(5, 15, "Yes")), charset);
    strcpy(buf, pPad->dialogTitle);
    strcat(buf, (char *)GETMESSAGE(5, 16, "Save?"));
    tmpStr3 = XmStringCreateLtoR(buf, charset);
    tmpStr4 = XmStringCreateLtoR(((char *)GETMESSAGE(2, 1, "Help")), charset);
    tmpStr5 = XmStringCreateLtoR(_DtCancelString,charset);
    ac = 0;
    XtSetArg(al[ac], XmNapplyLabelString, tmpStr1); ac++;
    XtSetArg(al[ac], XmNokLabelString, tmpStr2); ac++;
    XtSetArg(al[ac], XmNdialogTitle, tmpStr3); ac++;
    XtSetArg(al[ac], XmNhelpLabelString, tmpStr4); ac++;
    XtSetArg(al[ac], XmNcancelLabelString, tmpStr5); ac++;
    pSelect->save_warning = XmCreatePromptDialog(pPad->app_shell, "save_warn",
							al, ac);
    XmStringFree(tmpStr1);
    XmStringFree(tmpStr2);
    XmStringFree(tmpStr3);
    XmStringFree(tmpStr4);
    XmStringFree(tmpStr5);

    /* -----> Set the dialog input mode */
    ac=0;
    XtSetArg(al[ac], XmNmwmInputMode, MWM_INPUT_PRIMARY_APPLICATION_MODAL);ac++;
    XtSetValues(XtParent(pSelect->save_warning), al, ac);
 
    /* -----> Create Form to contain the prompt symbol and text (in a RowColumn)
    *         and the "insert new lines?" toggles (in a RadioBox) */
    ac = 0;
    XtSetArg(al[ac], XmNshadowThickness, 0); ac++;
    pSelect->work_area = XmCreateForm(pSelect->save_warning, "wkarea",  al, ac);
    XtManageChild(pSelect->work_area);

    /* -----> Create a horizontal RowColumn container for the warning symbol
     *        and text prompt */
    ac = 0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
    pSelect->row_col = XmCreateRowColumn(pSelect->work_area, "rowCol", al, ac);
    XtManageChild(pSelect->row_col);

    /* -----> Place the warning symbol in RowColumn container */
    ac = 0;
    XtSetArg(al[ac], XmNforeground, &foreground); ac++;
    XtSetArg(al[ac], XmNbackground, &background); ac++;
    XtGetValues(pSelect->save_warning, al, ac);
    ac = 0;
    XtSetArg(al[ac], XmNlabelType, XmPIXMAP); ac++;
    pixmap = XmGetPixmap(XtScreen(pSelect->save_warning), "warn_image",
				foreground, background);
    XtSetArg(al[ac], XmNlabelPixmap, pixmap); ac++;
    w = XmCreateLabelGadget(pSelect->row_col, "pixmap_label", al, ac);
    XtManageChild(w);

    /* -----> Place the prompt string in the RowColumn container
     *        (the prompt string is set in AskIfSave() based on whether
     *        or not a file name has been specified among other things) */
    ac = 0;
    pSelect->wkArea_textLabel = XmCreateLabelGadget(pSelect->row_col, 
							"txt_lab", al, ac);
    XtManageChild(pSelect->wkArea_textLabel);


    /* -----> Create a separator between message and new line toggles */
    ac = 0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pSelect->row_col); ac++;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pSelect->separator = XmCreateSeparator(pSelect->work_area, "sep", al, ac);
    if (pPad->xrdb.wordWrap == True)
	XtManageChild(pSelect->separator);
 
    /* -----> Create a radio box with two toggle buttons for either adding
     *        or not adding new lines if word wrap mode is on. */
    CreateNewLineToggles(pPad,
			pSelect->work_area,		/* parent widget */
			&pSelect->toggleWidgets);	/* new widgets */

    /* -----> Place toggle buttons below separator */
    ac = 0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pSelect->separator); ac++;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetValues(pSelect->toggleWidgets.newl_radio, al, ac);

    /* -----> manage the apply button of the SelectionBox PromptDialog */
    w = XmSelectionBoxGetChild (pSelect->save_warning, XmDIALOG_APPLY_BUTTON);
    XtManageChild(w);

    /* -----> don't need the SelectionBox PromptDialog text input field and
     *        label */
    ac = 0;
    kid[ac++] = XmSelectionBoxGetChild (pSelect->save_warning, XmDIALOG_TEXT);
    kid[ac++] = XmSelectionBoxGetChild (pSelect->save_warning,
                                        XmDIALOG_SELECTION_LABEL);
    XtUnmanageChildren (kid, ac);

    /* -----> set the SelectionBox PromptDialog [No] [Cancel] [Help] callbacks
     *        (the [Yes] XmNokCallback is set in AskIfSave() based on whether
     *        or not a file name has been specified among other things) */
    XtAddCallback(pSelect->save_warning, XmNapplyCallback, 
		(XtCallbackProc) NoSaveCB, (XtPointer) pPad);
    XtAddCallback(pSelect->save_warning, XmNcancelCallback, 
		(XtCallbackProc) AskIfSaveCancelCB, (XtPointer) pPad);
    XtAddCallback(pSelect->save_warning, XmNhelpCallback, 
		(XtCallbackProc) HelpAskIfSaveDialogCB, (XtPointer) pPad);

    /* -----> save the widget id of the Cancel button so that it can be
     *        unmanaged (in AskIfSave) if a "forced" TTDT_QUIT is being
     *        processed */
    pSelect->cancelBtn = XmSelectionBoxGetChild(
				pSelect->save_warning, XmDIALOG_APPLY_BUTTON);

    XtRealizeWidget(pSelect->save_warning);

}


/************************************************************************
 *  Warning - posts a information/warning/error message dialog
 ************************************************************************/
void 
Warning(
	Editor *pPad,
	char *mess ,
	unsigned char dialogType)
{
    Arg al[10];
    int ac;
    char *tmpMess, buf[256];
    FileWidgets *pFileWidgets = &pPad->fileStuff.fileWidgets;
    XmString tempStr;
 
    tmpMess = strdup(mess);

    /* create the dialog if it is the first time */
    if (!pFileWidgets->gen_warning) 
    {
	ac = 0;
	XtSetArg(al[ac], XmNokLabelString, XmStringCreateLtoR(_DtOkString,
							charset));  ac++;
	pFileWidgets->gen_warning = XmCreateMessageDialog (pPad->app_shell, 
							"Warn", al, ac);
	/* Unmanage unneeded children. */
	XtUnmanageChild(XmMessageBoxGetChild(pFileWidgets->gen_warning, 
						XmDIALOG_CANCEL_BUTTON) );
	XtUnmanageChild(XmMessageBoxGetChild(pFileWidgets->gen_warning,
						XmDIALOG_HELP_BUTTON) );

	XtRealizeWidget(pFileWidgets->gen_warning);
	ac=0;
        XtSetArg(al[ac], XmNmwmInputMode, 
			MWM_INPUT_PRIMARY_APPLICATION_MODAL);ac++;
	XtSetValues(XtParent(pFileWidgets->gen_warning), al, ac);
    }

    strcpy(buf, pPad->dialogTitle);
    switch(dialogType)
    {
	case XmDIALOG_ERROR:
	    strcat(buf, (char *)GETMESSAGE(5, 17, "Error"));
	    break;
	case XmDIALOG_INFORMATION:
	    strcat(buf, (char *)GETMESSAGE(5, 20, "Information"));
	    break;
	case XmDIALOG_WARNING:
	default:
	    strcat(buf, (char *)GETMESSAGE(5, 6, "Warning"));
	    break;
    }

    ac = 0;
    tempStr = XmStringCreateLtoR(buf, charset);
    XtSetArg(al[ac], XmNdialogTitle, tempStr); ac++; 
    XtSetArg(al[ac], XmNdialogType, dialogType); ac++; 
    XtSetArg(al[ac], XmNmessageString,
		XmStringCreateLtoR(tmpMess, charset)); ac++;
    XtSetValues(pFileWidgets->gen_warning, al, ac);
    XmStringFree(tempStr);

    XtManageChild (pFileWidgets->gen_warning);
}


/************************************************************************
 * PostSaveError - posts a dialog displaying the status returned by
 *	DtEditorSaveContentsToFile() (when called by the "File" menu
 *	callbacks).
 ************************************************************************/
void 
PostSaveError(
	Editor *pPad,
	char *saveName,
	DtEditorErrorCode errorCode)
{
    char errorMsg[256], *errorReason;

    /* -----> determine the reason why save failed */
    switch(errorCode) {
	case DtEDITOR_NO_ERRORS:
	    return;
	case DtEDITOR_INVALID_FILENAME:
	    Warning(pPad, ((char *)
        	    GETMESSAGE(5, 31, "Unable to save file.\nFile name was not specified.")),
		    XmDIALOG_ERROR);
	    return;
	case DtEDITOR_UNWRITABLE_FILE:
	    errorReason = strdup( (char *)
		    GETMESSAGE(5, 34, "File does not allow writing by anyone."));
	    break;
	case DtEDITOR_DIRECTORY:
	    errorReason = strdup((char *)
		    GETMESSAGE(5, 35, "File is a directory."));
	    break;
	case DtEDITOR_CHAR_SPECIAL_FILE:
	    errorReason = strdup((char *)
		    GETMESSAGE(5, 36, "File is a character special device."));
	    break;
	case DtEDITOR_BLOCK_MODE_FILE:
	    errorReason = strdup((char *)
		    GETMESSAGE(5, 37, "File is a block mode device."));
	    break;
	case DtEDITOR_INVALID_TYPE:	/* this should never occur */
	    errorReason = strdup((char *)
		    GETMESSAGE(5, 38, "Invalid file format."));
	    break;
	case DtEDITOR_SAVE_FAILED:	/* this should catch everything else */
	default:			/* - but just in case */
	    errorReason = strdup((char *)
		    GETMESSAGE(5, 39, "Unknown reason - possibly not enough disk space."));
	    break;
    }

    /* -----> format and display error messages containing the
     *        file name and reason why the save failed */
    sprintf(errorMsg,
	    (char *) GETMESSAGE(5, 30, "Unable to save %1$s.\n%2$s"),
	    saveName,
	    errorReason);
    Warning(pPad, errorMsg, XmDIALOG_ERROR);
    free(errorReason);

    if (pPad->fileStuff.pendingFileFunc == FileExitCB) {
	TTfailPendingQuit(pPad);
    }

}


/************************************************************************
 * AskIfSave - creates a dialog and corresponding "o.k." button callback
 *	to save the current text to a file.  The dialog message and
 *	o.k. callback depend on whether or not a name is associated
 *	with the text.
 ************************************************************************/
void
AskIfSave(
        Editor *pPad)
{
    Arg al[10];             /* arg list */
    register int ac;        /* arg count */
    char *tmp = NULL;
    XmString tmpStr;
    Select *pSelect = &pPad->fileStuff.fileWidgets.select;

    if (pSelect->save_warning == (Widget) NULL) 
	CreateSaveWarning(pPad);

    if (pPad->ttQuitReq.contract && pPad->ttQuitReq.force) {
	XtUnmanageChild(pSelect->cancelBtn);
    } else {
	XtManageChild(pSelect->cancelBtn);
    }

    XtRemoveAllCallbacks(pSelect->save_warning, XmNokCallback);

    if (pPad->ttEditReq.contract &&
      pPad->ttEditReq.docName && *pPad->ttEditReq.docName) {
	/* -----> TT docName supplied in media request, so ask:
	 *	  "Save changes to "<docName>"?" */
	tmp = (char *)XtMalloc(strlen(SAVE_CH_DOC) + 
				strlen(pPad->ttEditReq.docName) + 4);
	sprintf(tmp,SAVE_CH_DOC, pPad->ttEditReq.docName);
	XtAddCallback(pSelect->save_warning, XmNokCallback,
			(XtCallbackProc) FileSaveCB, (XtPointer) pPad);
    } else if (pPad->fileStuff.fileName && *pPad->fileStuff.fileName) {
	/* -----> TT or non-TT filename given, so ask:
	 *	  "Save changes to <file>?" */
	tmp = (char *)XtMalloc(strlen(SAVE_CH) + 
				strlen(pPad->fileStuff.fileName) + 4);
	sprintf(tmp,SAVE_CH, pPad->fileStuff.fileName);
	XtAddCallback(pSelect->save_warning, XmNokCallback,
			(XtCallbackProc) FileSaveCB, (XtPointer) pPad);
    } else if (pPad->ttEditReq.contract && pPad->ttEditReq.contents) {
	/* -----> TT buffer being edited w/o a docName, so simply ask:
	 *	  "Save Changes?" */
	tmp = (char *)XtMalloc(strlen(SAVE_CH_BUF) + 4);
	sprintf(tmp,"%s",SAVE_CH_BUF);
	XtAddCallback(pSelect->save_warning, XmNokCallback,
			(XtCallbackProc) FileSaveCB, (XtPointer) pPad);
    } else {
	/* -----> no file/doc name (and not a TT buffer), so ask:
	 *	  "File has no name. Do you wash to save it?" */
	tmp = (char *)XtMalloc(strlen(SAVE_CH_NONAME) + 4);
	sprintf(tmp,"%s",SAVE_CH_NONAME);
	XtAddCallback(pSelect->save_warning, XmNokCallback,
			(XtCallbackProc) FileSaveAsCB, (XtPointer) pPad);
    }

    tmpStr = XmStringCreateLtoR(tmp, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetValues(pSelect->wkArea_textLabel, al, ac);
    XmStringFree(tmpStr);
    XtFree(tmp);

    /*
     * Make sure that the AskIfSave will be visible in case the
     * main window is iconified and the user is closing the window
     * from a window manager menu.
     */
    XtPopup(pPad->app_shell, XtGrabNone);
    XtMapWidget(pPad->app_shell);

    XtManageChild (pSelect->save_warning);
    XmProcessTraversal(XmSelectionBoxGetChild(pSelect->save_warning, 
			XmDIALOG_OK_BUTTON), XmTRAVERSE_CURRENT);
}


/************************************************************************
 * AskForConfirmation - called iff we had a non-null confirmation string
 *	defined in our resources.  The confirmation string is typically used
 *	by another program (e.g. a mailer) to determine what action to take
 *	when dtpad exits.  E.g. A mailer might give us a confirmation string
 *	like "Do you wish to send this message?", and will use our exit code
 *	to determine whether or not to pass the message on.
 ************************************************************************/
void
AskForConfirmation(
	Editor *pPad)
{
    XmString tmpStr1;
    Arg al[10];             /* arg list */
    register int ac;        /* arg count */
    ConfirmStuff *pStuff = &pPad->confirmStuff;

    if(pStuff->dialog == (Widget)NULL)
    {
	XmString tmpStr2, tmpStr3;
	Widget w, kid[5];
	/*
	 * Create the confirm dialog with the resource spec'd string
	 * as the displayed text.
	 */
	tmpStr1 = XmStringCreateLtoR(((char *)
			GETMESSAGE(5, 14, "No")), charset);
	tmpStr2 = XmStringCreateLtoR(((char *)
			GETMESSAGE(5, 15, "Yes")), charset);
	if(pPad->xrdb.mainTitle == (char *)NULL)
	{
	    char buf[256];
	    strcpy(buf, pPad->dialogTitle);
	    strcat(buf, (char *) GETMESSAGE(5, 18, "Exit Confirmation"));
	    tmpStr3 = XmStringCreateLtoR(buf, charset);
	}
	else
	{
	    tmpStr3 = XmStringCreateLtoR(pPad->xrdb.mainTitle, charset);
	}
	    
	ac = 0;
	XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL); ac++;
	XtSetArg(al[ac], XmNapplyLabelString, tmpStr1); ac++;
	XtSetArg(al[ac], XmNokLabelString, tmpStr2); ac++;
	XtSetArg(al[ac], XmNdialogTitle, tmpStr3); ac++;
	pStuff->dialog = XmCreatePromptDialog(pPad->app_shell, "confirm", 
					      al, ac);
	XmStringFree(tmpStr1);
	XmStringFree(tmpStr2);
	XmStringFree(tmpStr3);

	ac = 0;
	XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
	pStuff->wk_area = XmCreateRowColumn( pStuff->dialog, "wkarea", al, ac);
	XtManageChild(pStuff->wk_area);

	tmpStr1 = XmStringCreateLtoR(pPad->xrdb.confirmationString, charset);
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, tmpStr1);ac++;
	pStuff->wkArea_textLabel = XmCreateLabelGadget(pStuff->wk_area,
						"txt_lab", al, ac);
	XtManageChild(pStuff->wkArea_textLabel);

	w = XmSelectionBoxGetChild (pStuff->dialog, XmDIALOG_APPLY_BUTTON);
	XtManageChild(w);

	/* Unmanage unneeded children. */
	ac = 0;
	kid[ac++] = XmSelectionBoxGetChild (pStuff->dialog, XmDIALOG_TEXT);
	kid[ac++] = XmSelectionBoxGetChild (pStuff->dialog,
						XmDIALOG_SELECTION_LABEL);
	XtUnmanageChildren (kid, ac);
    
	XtAddCallback (pStuff->dialog, XmNokCallback,
			(XtCallbackProc) ConfirmYesCB, (XtPointer) pPad);
	XtAddCallback (pStuff->dialog, XmNapplyCallback,
			(XtCallbackProc) ConfirmNoCB, (XtPointer) pPad);
	XtAddCallback (pStuff->dialog, XmNhelpCallback,
			(XtCallbackProc) HelpConfirmDialogCB, (XtPointer) pPad);

	XtRealizeWidget (pStuff->dialog);
    }

    tmpStr1 = XmStringCreateLtoR(pPad->xrdb.confirmationString, charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr1);ac++;
    XtSetValues(pStuff->wkArea_textLabel, al, ac);

    XtManageChild (pStuff->dialog);

    XmStringFree(tmpStr1);
}
