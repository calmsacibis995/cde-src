/**********************************<+>*************************************
***************************************************************************
**
**  File:        dtpad.c
**
**  Project:     DT dtpad, a memo maker type editor based on the Dt Editor
**               widget.
**
**  Description:
**  -----------
**
**	This file contains routines that create the dtpad menu bar widget
**	and the Dt Editor widget.
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
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
#include "signal.h"
#include "X11/Xutil.h"
#include <X11/StringDefs.h>

#ifdef MESSAGE_CAT
#define TRUE 1
#define FALSE 0
#include <nl_types.h>
#endif

extern Editor *pPadList; /* declared in main.c */
extern int numActivePads; /* declared in main.c */

/* used to set up XmStrings */
XmStringCharSet charset = (XmStringCharSet) XmFONTLIST_DEFAULT_TAG;


/************************************************************************
 * CreateDefaultImage - create a default image for the warning symbol.
 *		
 ************************************************************************/
void 
CreateDefaultImage(
        unsigned char *bits,
        int width,
        int height,
        char *name )
{
    XImage *image;	/* image for warning pixmap */

    image = (XImage *) XtMalloc (sizeof (XImage));
    image->width = width;
    image->height = height;
    image->data = (char*) bits;
    image->depth = 1;
    image->xoffset = 0;
    image->format = XYBitmap;
    image->byte_order = LSBFirst;
    image->bitmap_unit = 8;
    image->bitmap_bit_order = LSBFirst;
    image->bitmap_pad = 8;
    image->bytes_per_line = (width+7)/8;
    XmInstallImage (image, name);
}


/*
 * The following functions are multibyte-aware string handling routines,
 * including MbStrchr, MbStrrchr, and MbBasename.  These routines are
 * used as substitutes for the single-byte-only strchr and strrchr routines.
 *
 *
 * [SuG 5/12/95] Added MbStrchri which does exactly MbStrchr, but also returns
 * index.
 */

/************************************************************************
 * MbStrchr - 
 ************************************************************************/
char *
MbStrchr(
	char *str,
	int ch)
{
    size_t mbCurMax = MB_CUR_MAX;
    wchar_t targetChar, curChar;
    char tmpChar;
    int i, numBytes, byteLen;

    if(mbCurMax <= 1) return strchr(str, ch);

    tmpChar = (char)ch;
    mbtowc(&targetChar, &tmpChar, mbCurMax);
    for(i = 0, numBytes = 0, byteLen = strlen(str); i < byteLen; i += numBytes)
    {
	numBytes = mbtowc(&curChar, &str[i], mbCurMax);
	if(curChar == targetChar) return &str[i];
    }
    return (char *)NULL;
}

/************************************************************************ 
 * MbStrchri - 
 ************************************************************************/ 
char * 
MbStrchri( 
    char *str, 
    int ch, int* in)
{
    size_t mbCurMax = MB_CUR_MAX;
    wchar_t targetChar, curChar;
    char tmpChar;
    int i, j, numBytes, byteLen;
 
    /*if(mbCurMax <= 1) return strchr(str, ch);*/
 
    tmpChar = (char)ch;
    mbtowc(&targetChar, &tmpChar, mbCurMax);
    for(i = 0, j = 0, numBytes = 0, byteLen = strlen(str); i < byteLen;
        i += numBytes, j++)
    {
        numBytes = mbtowc(&curChar, &str[i], mbCurMax);
        if(curChar == targetChar) {
            *in = j;
            return &str[i];
        }
    }
	*in = -1;
    return (char *)NULL;
}    

/************************************************************************
 * MbStrrchr - 
 ************************************************************************/
char *
MbStrrchr(
	char *str,
	int ch)
{
    size_t mbCurMax = MB_CUR_MAX;
    wchar_t targetChar, curChar;
    char tmpChar, *lastChar;
    int i, numBytes, byteLen;

    if(mbCurMax <= 1) return strrchr(str, ch);

    tmpChar = (char)ch;
    mbtowc(&targetChar, &tmpChar, mbCurMax);
    for(i = 0, numBytes = 0, lastChar = (char *)NULL, byteLen = strlen(str);
          i < byteLen; i += numBytes)
    {
	numBytes = mbtowc(&curChar, &str[i], mbCurMax);
	if(curChar == targetChar) lastChar = &str[i];
    }

    return lastChar;
}


/************************************************************************
 * MbBasename - 
 ************************************************************************/
char *
MbBasename(
	char *str)
{
    char *lastSlash;

    lastSlash = MbStrrchr(str, '/');

    if(lastSlash != (char *)NULL)
	return lastSlash + 1;
    else
	return str;
}


/************************************************************************
 *  ChangeMainWindowTitle - If necessary, put file name in window title
 ************************************************************************/
void 
ChangeMainWindowTitle( 
	Editor *pPad)
{
    Arg al[10];
    int ac;
    char *name;
    char *title;
    Boolean freeName = False, freeTitle = False;
    XTextProperty textProp;

    /* Should happen only when creating a new view */
    if (!XtIsRealized(pPad->app_shell))
       return;

    if(pPad->xrdb.nameInTitle == True) {
	if (pPad->ttEditReq.contract && 
		(pPad->ttEditReq.docName && *pPad->ttEditReq.docName)) {
	    name = pPad->ttEditReq.docName;
        } else if (pPad->fileStuff.fileName && *pPad->fileStuff.fileName) {
	    name = MbBasename(pPad->fileStuff.fileName);
        } else {
	    name = strdup(UNNAMED_TITLE);
	    freeName = True;
        }
        title = (char *)XtMalloc(strlen(pPad->dialogTitle) + strlen(name) + 1);
	freeTitle = True;
        sprintf(title, "%s%s", pPad->dialogTitle, name);
    } else {
	title = pPad->dialogTitle;
	name = pPad->dialogTitle;
    }

    XmbTextListToTextProperty(pPad->display, &name, 1, XStdICCTextStyle,
			      &textProp);
    XSetWMIconName(pPad->display, XtWindow(pPad->app_shell), &textProp);
    XFree(textProp.value);

    ac = 0;
    XtSetArg(al[ac], XmNtitle, title); ac++;
    XtSetArg(al[ac], XmNtitleEncoding, None); ac++;
    XtSetValues(pPad->app_shell, al, ac);

    if(freeName == True)
	XtFree(name);
    if(freeTitle == True)
	XtFree(title);
}


/************************************************************************
 * SetWindowTitles - Used to set/reset the titles of all the existing
 *		dialogs within a pad.  This is called from 
 *		SetStateFromResources().
 *
 *	The SaveAs dialog title is set in SetSaveAsLabelAndDialog() since
 *	pads can be reused and this title can vary based on xrdb.noNameChange.
 *		
 ************************************************************************/
void 
SetWindowTitles( 
	Editor *pPad)
{
    char buf[256];
    Arg al[1];
    XmString tempStr;

    /* dialogs created in fileDlg.c */
    if(pPad->fileStuff.fileWidgets.saveAs.alrdy_exist != (Widget) NULL)
    {
        strcpy(buf, pPad->dialogTitle);
        strcat(buf, (char *)GETMESSAGE(5, 6, "Warning"));
        tempStr = XmStringCreateLtoR(buf, XmFONTLIST_DEFAULT_TAG);
        XtSetArg(al[0], XmNdialogTitle, tempStr);
	XtSetValues(pPad->fileStuff.fileWidgets.saveAs.alrdy_exist, al, 1);
	XmStringFree(tempStr);
    }
    if(pPad->fileStuff.openTitle != (XmString)NULL)
    {
	/* Used for the file selection dialog title during Open */
	XmStringFree(pPad->fileStuff.openTitle);
	pPad->fileStuff.openTitle = (XmString)NULL;
    }
    if(pPad->fileStuff.includeTitle != (XmString)NULL)
    {
	/* Used for the file selection dialog title during Include */
	XmStringFree(pPad->fileStuff.includeTitle);
	pPad->fileStuff.includeTitle = (XmString)NULL;
    }
    if(pPad->fileStuff.fileWidgets.select.save_warning != (Widget)NULL)
    {
        strcpy(buf, pPad->dialogTitle);
	strcat(buf, (char *)GETMESSAGE(5, 16, "Save?"));
        tempStr = XmStringCreateLtoR(buf, charset);
	XtSetArg(al[0], XmNdialogTitle, tempStr);
	XtSetValues(pPad->fileStuff.fileWidgets.select.save_warning, al, 1);
	XmStringFree(tempStr);
    }
    if(pPad->fileStuff.fileWidgets.gen_warning != (Widget)NULL)
    {
	/* This dialog resets its title on each posting */
    }
    if(pPad->confirmStuff.dialog != (Widget)NULL)
    {
        if(pPad->xrdb.mainTitle == (char *)NULL)
        {
            strcpy(buf, pPad->dialogTitle);
            strcat(buf, (char *) GETMESSAGE(5, 18, "Exit Confirmation"));
            tempStr = XmStringCreateLtoR(buf, charset);
        }
        else
        {
            tempStr = XmStringCreateLtoR(pPad->xrdb.mainTitle, charset);
        }
	XtSetArg(al[0], XmNdialogTitle, tempStr);
	XtSetValues(pPad->confirmStuff.dialog, al, 1);
	XmStringFree(tempStr);
    }

}


/************************************************************************
 * SetSaveAsLabelAndDialog - set the File menu [Save As] button label and
 *	mnemonic, and the SaveAs dialog title based on whether or not the
 *	file name can change.
 ************************************************************************/
void
SetSaveAsLabelAndDialog(
        Editor *pPad)
{
    char buf[256];
    Arg al[10];
    int ac, count = 0;
    char      *mnemonic;
    XmString tmpStr;

    if (pPad->fileStuff.fileWidgets.saveAsBtn) {
	if (pPad->xrdb.nameChange) {
	    mnemonic = strdup(((char *)GETMESSAGE(11, 25, "A")));
	    tmpStr = XmStringCreateLtoR((
			(char *)GETMESSAGE(11, 26, "Save As...")), charset);
	} else {
	    mnemonic = strdup(((char *)GETMESSAGE(11, 109, "F")));
	    tmpStr = XmStringCreateLtoR((
			(char *)GETMESSAGE(11, 110, "Copy To File...")), charset);
	}
	ac = 0;
	XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
	XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
	XtSetValues(pPad->fileStuff.fileWidgets.saveAsBtn, al, ac);
	XtFree(mnemonic);
	XmStringFree(tmpStr);
    }

    if (pPad->fileStuff.fileWidgets.saveAs.saveAs_form) {
        strcpy(buf, pPad->dialogTitle);
	if (pPad->xrdb.nameChange) {
	    strcat(buf, (char *)GETMESSAGE(5, 7, "Save As"));
	} else {
	    strcat(buf, (char *)GETMESSAGE(5, 43, "Copy To File"));
	}
        tmpStr = XmStringCreateLtoR(buf, XmFONTLIST_DEFAULT_TAG);
        XtSetArg(al[0], XmNdialogTitle, tmpStr);
	XtSetValues(pPad->fileStuff.fileWidgets.saveAs.saveAs_form, al, 1);
	XmStringFree(tmpStr);
    }
   
}



/************************************************************************
 * CreateHelpMenu - Create the Help pulldown menu
 *
 *	Overview
 *	----------
 *	Tasks
 *	Table of Contents
 *	Reference
 *	On Item
 *	----------
 *	Using Help
 *	----------
 *	About Text Editor
 *
 ************************************************************************/
static void
CreateHelpMenu(
	Editor *pPad)
{
    Widget help_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char      *mnemonic;
    XmString tmpStr;

    /* -----> create "Help" PulldownMenu */
    ac = 0;
    help_pane = XmCreatePulldownMenu (pPad->menu_bar, "helpMenu", al, ac);
    XtAddCallback(help_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpHelpCB, pPad);
    mnemonic = strdup(((char *)GETMESSAGE(11, 81, "H")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 82, "Help")), charset);
    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, help_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade = XmCreateCascadeButtonGadget(pPad->menu_bar, "Help", al, ac);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    ac = 0;
    XtSetArg(al[ac], XmNmenuHelpWidget, cascade); ac++;
    XtSetValues (pPad->menu_bar, al, ac);

    /* -----> create "Overview" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 83, "v")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 84, "Overview")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "overview", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpOverviewCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> craete Separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (help_pane, "sep1", al, ac);

    /* -----> create "Tasks" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 85, "T")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 86, "Tasks")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "tasks", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpTasksCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Table of Contents" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 87, "C")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 88, "Table of Contents")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "TOC", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpTOCCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Reference" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 89, "R")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 90, "Reference")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "reference", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpReferenceCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "On Item" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 91, "O")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 92, "On Item")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "onItem", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpOnItemCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create Separator*/
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (help_pane, "sep2", al, ac);

    /* -----> create "Using Help" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 93, "U")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 94, "Using Help")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "usingHelp", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpUsingHelpCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create Separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (help_pane, "sep3", al, ac);

    /* -----> create "About Text Editor" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 95, "A")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 96, "About Text Editor")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++]= XmCreatePushButtonGadget(help_pane, "about", al, ac );
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)HelpAboutCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    XtManageChildren(WidgList, count);
}


/************************************************************************
 * CreateFileMenu - Create the "File" pulldown menu
 *
 *	New
 *	Open...
 *	Include...
 *	----------
 *	Save		or [Save (needed)]
 *	Save As...
 *	Print...
 *	----------
 *	Close
 *		
 ************************************************************************/
static void
CreateFileMenu(
	Editor *pPad)
{
    Widget file_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char      *mnemonic;
    XmString tmpStr;
    FileWidgets *pWidg = &pPad->fileStuff.fileWidgets;

    /* -----> create "File" PulldownMenu */
    ac = 0;
    file_pane = XmCreatePulldownMenu (pPad->menu_bar, "fileMenu", al, ac);
    XtAddCallback(file_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpFileCB, pPad);
    mnemonic = strdup(((char *)GETMESSAGE(11, 15, "F")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 16, "File")), charset);
    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, file_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade = XmCreateCascadeButtonGadget (pPad->menu_bar, "File", al, ac);
    XtAddCallback (cascade, XmNcascadingCallback, 
		   (XtCallbackProc)FileCascadingCB, (XtPointer) pPad);
    XtManageChild (cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "New" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 17, "N")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 18, "New")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "new", al, ac);
    pWidg->newBtn = WidgList[count-1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc)FileNewCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Open..." push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 19, "O")));
    tmpStr=XmStringCreateLtoR(((char *)GETMESSAGE(11, 20, "Open...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "open", al, ac);
    pWidg->openBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileOpenCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Include..." push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 21, "I")));
    tmpStr = XmStringCreateLtoR((
	     (char *)GETMESSAGE(11, 22, "Include...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "include", al, ac);
    pWidg->includeBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileIncludeCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create save/print separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (file_pane, "SvSp", al, ac);

    /* -----> create "Save" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 23, "S")));
    /* Note: The "Save" label is set in FileCascadingCB() when the File menu
     *       is displayed based on whether or not there are unsaved changes */
    pPad->fileStuff.saveBtnLabel = 
      XmStringCreateLtoR(((char *)GETMESSAGE(11, 24, "Save")), charset);
    pPad->fileStuff.saveNeededBtnLabel = 
      XmStringCreateLtoR(((char *)GETMESSAGE(11, 73, "Save (needed)")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "save", al, ac);
    pWidg->saveBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileSaveCB, (XtPointer) pPad);
    XtFree(mnemonic);

    /* -----> create "Save As..." push button
     *        (its label/mnemonic are set later in SetSaveAsLabelAndDialog()
     *        based on xrdb.nameChange) */
    ac = 0;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "saveAs", al, ac);
    pWidg->saveAsBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileSaveAsCB, (XtPointer) pPad);

    /* -----> create "Print..." push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 27, "P")));
    tmpStr = XmStringCreateLtoR((
		(char *)GETMESSAGE(11, 28, "Print...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "print", al, ac);
    pWidg->printBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FilePrintCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create close/exit separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (file_pane, "ExSp", al, ac);

    /* -----> create "Close" (Exit) push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 29, "C")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 30, "Close")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (file_pane, "close", al, ac);
    pPad->ExitWid = WidgList[count-1];
    pWidg->exitBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FileExitCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> manage the all "File" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * CreateEditMenu - Create the "Edit" pulldown menu
 *		
 *	Undo
 *	----------
 *	Cut
 *	Copy
 *	Paste
 *	Clear
 *	Delete
 *	Select All
 *	----------
 *	Find/Change...
 *	Check Spellilng...
 *		
 ************************************************************************/
static void
CreateEditMenu(
	Editor *pPad)
{
    Widget edit_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;
    EditWidgets *pWidg = &pPad->editStuff.widgets;

    /* -----> create "Edit" PulldownMenu */
    ac = 0;
    edit_pane = XmCreatePulldownMenu (pPad->menu_bar, "editMenu", al, ac);
    XtAddCallback(edit_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpEditCB, pPad);
    mnemonic = strdup(((char *)GETMESSAGE(11, 31, "E")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 32, "Edit")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, edit_pane); ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade= XmCreateCascadeButtonGadget(pPad->menu_bar, "Edit", al, ac);
    XtManageChild(cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Undo" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 33, "U")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 34, "Undo")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "undo", al, ac);
    pWidg->undoBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditUndoCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create undo separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget(edit_pane, "CtSp", al, ac);

    /* -----> create "Cut" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 35, "t")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 36, "Cut")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "cut", al, ac);
    pWidg->cutBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditCutCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Copy" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 37, "C")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 38, "Copy")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "copy", al, ac);
    pWidg->copyBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditCopyCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Paste" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 39, "P")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 40, "Paste")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "paste", al, ac);
    pWidg->pasteBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditPasteCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Clear" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 41, "e")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 42, "Clear")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "clear", al, ac);
    pWidg->clearBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditClearCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Delete" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 43, "D")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 44,"Delete")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (edit_pane, "delete", al, ac);
    pWidg->deleteBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) EditDeleteCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> XXX - create "Select All" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 101, "S")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 102, "Select All")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "selectAll", al, ac);
    pWidg->selectAllBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) SelectAllCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create find/change separator */
    ac = 0;
    WidgList[count++] = XmCreateSeparatorGadget (edit_pane, "FSSp", al, ac);

    /* -----> create "Find/Change" push button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 103, "F")));
    tmpStr =XmStringCreateLtoR((
		(char *)GETMESSAGE(11, 104,"Find/Change...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = 
		XmCreatePushButtonGadget(edit_pane, "findChange", al, ac);
    pWidg->findChangeBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		  (XtCallbackProc) FindChangeCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Check Spelling" push button  */
    mnemonic = strdup(((char *)GETMESSAGE(11, 105, "k")));
    tmpStr = XmStringCreateLtoR((
		     (char *)GETMESSAGE(11, 106, "Check Spelling...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget(edit_pane, "checkSpelling", 
						     al, ac);
    pWidg->checkSpellingBtn = WidgList[count - 1];
    XtAddCallback(WidgList[count-1], XmNactivateCallback, 
		      (XtCallbackProc) CheckSpellingCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> manage all the "Edit" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * CreateFormatMenu - Create the "Format" pulldown menu
 *
 *	Settings...
 *	Paragraph
 *	All
 *		
 ************************************************************************/
static void
CreateFormatMenu(
	Editor *pPad)
{
    Widget format_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;

    /* -----> create "Format" PulldownMenu */
    ac = 0;
    format_pane = XmCreatePulldownMenu (pPad->menu_bar, "formatMenu", al, ac);
    XtAddCallback(format_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpFormatCB, pPad);
    mnemonic = strdup(((char *)GETMESSAGE(11, 51, "r")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 52, "Format")), charset);
    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, format_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade= XmCreateCascadeButtonGadget (pPad->menu_bar, "format", al, ac);
    pPad->formatStuff.widgets.formatBtn = cascade;
    XtManageChild (cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Settings..." button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 53, "S")));
    tmpStr = XmStringCreateLtoR(
	       ((char *)GETMESSAGE(11, 54, "Settings...")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] =
                   XmCreatePushButtonGadget (format_pane, "settings", al, ac);
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FormatCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Paragraph" button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 55, "P")));
    tmpStr = XmStringCreateLtoR(
	       ((char *)GETMESSAGE(11, 56, "Paragraph")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] =
                  XmCreatePushButtonGadget (format_pane, "paragraph", al, ac);
    XtAddCallback (WidgList[count-1], XmNactivateCallback,
		   (XtCallbackProc) FormatParaCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "All" button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 57, "l")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 58, "All")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    WidgList[count++] = XmCreatePushButtonGadget (format_pane, "all", al, ac);
    XtAddCallback (WidgList[count-1], XmNactivateCallback, 
		   (XtCallbackProc) FormatAllCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> manage the all "Format" menu buttons */
    XtManageChildren(WidgList, count);
}


/************************************************************************
 * CreateOptionsMenu - Create the "Options" pulldown menu
 *
 *	Overstrike
 *	Wrap To Fit
 *	Status Line
 *		
 ************************************************************************/
static void
CreateOptionsMenu(
	Editor *pPad)
{
    Widget options_pane, cascade, WidgList[12];
    Arg al[10];
    int ac, count = 0;
    char *mnemonic;
    XmString tmpStr;
    OptionsMenuWidgets *pWidg = &pPad->optionsStuff.widgets;

    /* -----> create "Options" PulldownMenu */
    ac = 0;
    options_pane = XmCreatePulldownMenu (pPad->menu_bar, "optionsMenu", al, ac);
    XtAddCallback(options_pane, XmNhelpCallback, 
		  (XtCallbackProc)HelpOptionsCB, pPad);
    mnemonic = strdup(((char *)GETMESSAGE(11, 59, "O")));
    tmpStr = XmStringCreateLtoR(((char *)GETMESSAGE(11, 60, "Options")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNsubMenuId, options_pane);  ac++;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    cascade= XmCreateCascadeButtonGadget (pPad->menu_bar, "options", al, ac);
    pPad->optionsStuff.widgets.optionsBtn = cascade;
    XtManageChild (cascade);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Overstrike" toggle button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 61, "O")));
    tmpStr = XmStringCreateLtoR((
	       (char *)GETMESSAGE(11, 62, "Overstrike")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, pPad->xrdb.overstrike); ac++;
    WidgList[count++] = XmCreateToggleButton (options_pane, "overstrike", al, ac);
    pWidg->overstrikeBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) OverstrikeCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Wrap To Fit" toggle button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 111, "W")));
    tmpStr = XmStringCreateLtoR((
	       (char *)GETMESSAGE(11, 112, "Wrap To Fit")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, pPad->xrdb.wordWrap); ac++;
    WidgList[count++] = XmCreateToggleButton (options_pane, "wordWrap", al, ac);
    pWidg->wordWrapBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) WordWrapCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    /* -----> create "Status Line" toggle button */
    mnemonic = strdup(((char *)GETMESSAGE(11, 65, "S")));
    tmpStr = XmStringCreateLtoR((
	       (char *)GETMESSAGE(11, 66, "Status Line")), charset);
    ac = 0;
    XtSetArg(al[ac], XmNlabelString, tmpStr); ac++;
    XtSetArg(al[ac], XmNmnemonic, mnemonic[0]); ac++;
    XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
    XtSetArg(al[ac], XmNset, pPad->xrdb.statusLine); ac++;
    WidgList[count++] = XmCreateToggleButton (options_pane, "statusLine", al, ac);
    pWidg->statusLineBtn = WidgList[count - 1];
    XtAddCallback (WidgList[count-1], XmNvalueChangedCallback, 
		   (XtCallbackProc) StatusLineCB, (XtPointer) pPad);
    XtFree(mnemonic);
    XmStringFree(tmpStr);

    XtManageChildren(WidgList, count);

}


/************************************************************************
 * CreateMenuBar - Create MenuBar in MainWindow
 *		
 ************************************************************************/
void
CreateMenuBar(
	Editor *pPad)
{ 
    Arg al[10];

    pPad->menu_bar = XmCreateMenuBar (pPad->mainWindow, "bar", al, 0);
    XtAddCallback(pPad->menu_bar, XmNhelpCallback, 
		  (XtCallbackProc)HelpMenubarCB, pPad);

    CreateFileMenu(pPad);
    CreateEditMenu(pPad);
    CreateFormatMenu(pPad);
    CreateOptionsMenu(pPad);
    CreateHelpMenu(pPad);
}


/************************************************************************
 * CreateEditorWidget - creates the Dt Editor widget
 ************************************************************************/
void 
CreateEditorWidget(
        Editor *pPad)
{
    Widget parent = pPad->mainWindow;
    Arg al[10];		/* arg list */
    register int ac;	/* arg count */
    Pixel background, foreground, top_shadow, bottom_shadow, selectColor;
    Colormap colormap;
    XmString dialogTitleStr = 
	XmStringCreateLtoR(pPad->dialogTitle, XmFONTLIST_DEFAULT_TAG);

    /* create the DtEditor widget */
    ac = 0;
    XtSetArg(al[ac], DtNrows, DEFAULT_ROWS);  ac++;
    XtSetArg(al[ac], DtNcolumns, DEFAULT_COLS);  ac++;
    XtSetArg(al[ac], DtNdialogTitle, dialogTitleStr);  ac++;
    pPad->editor = DtCreateEditor(parent, "editor", al, ac);
    XmStringFree(dialogTitleStr);

    /* add the DtEditor callbacks */
    XtAddCallback(pPad->editor, XmNhelpCallback,
	           (XtCallbackProc) HelpEditorWidgetCB,
		   (XtPointer) pPad);
    XtAddCallback(pPad->editor, DtNtextSelectCallback,
	           (XtCallbackProc) TextSelectedCB,
		   (XtPointer) pPad);
    XtAddCallback(pPad->editor, DtNtextDeselectCallback,
	           (XtCallbackProc) TextDeselectedCB,
		   (XtPointer) pPad);

    /* get id of status line text field widget */
    pPad->statusLineMsg = DtEditorGetMessageTextFieldID(pPad->editor);

}


/************************************************************************
 * FindOrCreatePad - finds an unused entry in the list of Editors or,
 *	if none can be found, creates a new entry with some defaults
 *	obtain from the last entry that was created.
 * 
 *	Returns True if an existing, unused entry is "reused" or False
 *	if a new entry is created.  Also updates the current entry
 *	(whose address is passed as a parameter) to point to the
 *	found/created entry.
 *
 *	 Note:	The first entry in the list of Editors is created by
 *		CreatedFirstPad().  This routine is used to create
 *		all subsequent entries.
 *		
 ************************************************************************/
Boolean 
FindOrCreatePad(
    Editor **ppPad)
{
    Boolean foundPad = False;
    Editor *pPad;
    Arg al[10];
    register int ac;

    /* -----> Reuse unused entries (corresponding to closed editor windows)
     *        before creating new ones. */
    for (pPad = pPadList; pPad != (Editor *)NULL; pPad = pPad->pNextPad){
	if (pPad->inUse == False) {
	    pPad->inUse = True;
	    foundPad = True;
	    numActivePads++;
	    break;
	}
    }

    /* -----> Create a new entry if an unused one can not be found. */
    if (foundPad == False) {
	char *pProgName;
	Display *display;
	unsigned short group_id,
                       user_id;

	/* -----> Save some defaults from the last entry that was created. */
	Boolean exitOnLastClose = pPadList->xrdb.exitOnLastClose;
	pProgName = pPadList->progname;
	group_id = pPadList->group_id;
	user_id = pPadList->user_id;
	display = pPadList->display;

	/* -----> Create a new entry and point pPad and the global, pPadList 
	 *        to it. */
	CreatePad(&pPad);
	pPad->inUse = True;

	/* -----> Set some defaults from the last entry that was created. */
	pPad->display = display;
	pPad->progname = pProgName;
	pPad->group_id = group_id;
	pPad->user_id = user_id;
	pPad->xrdb.exitOnLastClose = exitOnLastClose;

	/* -----> Create the application shell for the new Editor entry. */
	ac = 0;
	pPad->app_shell = XtAppCreateShell(pPad->progname, DTPAD_CLASS_NAME,
					   applicationShellWidgetClass,
					   pPad->display, al, ac);
	pPad->app_context = XtWidgetToApplicationContext(pPad->app_shell);

	/* -----> Set all resources to initial server values */
	RestoreInitialServerResources(pPad);

    }

    /* -----> update the pointer to the current entry and return whether or
     *        not an existing entry was reused */
    *ppPad = pPad;
    return foundPad;
}


/************************************************************************
 * SetWorkSpaceHints - 
 *		
 ************************************************************************/
void
SetWorkSpaceHints(
        Widget shell,
        char *workspaces )
{
  char * ptr;
  Atom * workspace_atoms = NULL;
  int num_workspaces=0;

  if (workspaces)
  {
    do
    {
      ptr = MbStrchr (workspaces, ' ');

      if (ptr != NULL) *ptr = NULL;

      workspace_atoms = (Atom *) XtRealloc ((char *)workspace_atoms,
                        sizeof (Atom) * (num_workspaces + 1));
      workspace_atoms[num_workspaces] =
                           XmInternAtom (XtDisplay(shell), workspaces, True);

      num_workspaces++;

      if (ptr != NULL)
      {
        *ptr = ' ';
        workspaces = ptr + 1;
      }
    } while (ptr != NULL);

    DtWsmSetWorkspacesOccupied(XtDisplay(shell), XtWindow (shell), 
			workspace_atoms, num_workspaces);

    XtFree ((char *) workspace_atoms);
    workspace_atoms = NULL;
  }
  else
  {
     Window   rootWindow;
     Atom     pCurrent;
     Screen   *currentScreen;
     int      screen;
     char     *workspace_name;

     screen = XDefaultScreen(XtDisplay(shell));
     currentScreen = XScreenOfDisplay(XtDisplay(shell), screen);
     rootWindow = RootWindowOfScreen(currentScreen);

     if(DtWsmGetCurrentWorkspace(XtDisplay(shell), rootWindow,
                                                      &pCurrent) == Success)
     {
        DtWsmSetWorkspacesOccupied(XtDisplay(shell), XtWindow (shell), 
				&pCurrent, 1);
     }
  }
}
