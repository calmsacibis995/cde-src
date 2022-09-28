/**********************************<+>*************************************
***************************************************************************
**
**  File:        editCB.c
**
**  Project:     DT dtpad, a memo maker type editor based on the Dt Editor
**               widget.
**
**  Description:
**  -----------
**
**	This file contains the callbacks for the [Edit] menu items.
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1991, 1992.  All rights are
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
#include <Dt/HourGlass.h>

#ifdef USE_SDTSPELL

#include <dlfcn.h>

/* Function pointer to the Solaris CDE speling checker. */
void *SDtSpellLibHandle; 

#endif

/************************************************************************
 * EditUndoCB - [Edit] menu, [Undo] button.
 *	Undoes the last edit.
 ************************************************************************/
/* ARGSUSED */
void
EditUndoCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorUndoEdit(pPad->editor);
}


/************************************************************************
 * EditCutCB - [Edit] menu, [Cut] button.
 *	Cuts the current selection to the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
void
EditCutCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorCutToClipboard(pPad->editor);
}


/************************************************************************
 * EditCopyCB - [Edit] menu, [Copy] button.
 *	Copies the current selection to the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
void
EditCopyCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorCopyToClipboard(pPad->editor);
}


/************************************************************************
 * EditPasteCB - [Edit] menu, [Paste] button.
 *	Pastes from the Motif clipboard.
 ************************************************************************/
/* ARGSUSED */
void
EditPasteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorPasteFromClipboard(pPad->editor);

    /* XXX - Do DtEditorGetInsertPosition & DtEditorSetInsertionPosition
     *       need to be executed here??
     * XmTextPosition cursorPos;
     * cursorPos = XmTextGetInsertionPosition(pPad->text);
     * XmTextShowPosition(pPad->text, cursorPos);
     */
}


/************************************************************************
 * EditClearCB - [Edit] menu, [Replace] button
 *	Replaces the current selection with blanks.
 ************************************************************************/
/* ARGSUSED */
void
EditClearCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorClearSelection(pPad->editor);
}


/************************************************************************
 * EditDeleteCB - [Edit] menu, [Delete] button.
 *	Deletes the current selection.
 ************************************************************************/
/* ARGSUSED */
void
EditDeleteCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorDeleteSelection(pPad->editor);
}


/************************************************************************
 * SelectAllCB - [Edit] menu, [Select All] button.
 *	Selects all text.
 ************************************************************************/
/* ARGSUSED */
void
SelectAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorSelectAll(pPad->editor);
}


/************************************************************************
 * FindChangeCB - [Edit] menu, [Find/Change...] button.
 *	Invokes the Dt Editor widget search dialog.
 ************************************************************************/
/* ARGSUSED */
void
FindChangeCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;
    DtEditorInvokeFindChangeDialog(pPad->editor);
}


/************************************************************************
 * CheckSpellingCB - [Edit] menu, [Check Spelling...] button.
 *	Invokes the Dt Editor widget spell dialog.
 ************************************************************************/
/* ARGSUSED */
void
CheckSpellingCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Editor *pPad = (Editor *)client_data;

#ifdef USE_SDTSPELL
    /* Function pointer to SDtInvokeSpell(), as returned by dlsym. */
    Boolean (*SDtInvokeSpell_FP)(Widget);

    /* Can we access the Solaris CDE spelling checker? */
    SDtSpellLibHandle = dlopen("libSDtSpell.so", RTLD_LAZY);
    if (SDtSpellLibHandle != (void *) NULL) {
	    /* We have access to the Sun Spell Checking library.
	     * Get the handle to the spell checking function.
             */
            SDtInvokeSpell_FP = 
		    (Boolean (*)(Widget)) dlsym(SDtSpellLibHandle, 
						"SDtInvokeSpell");

	    if (SDtInvokeSpell_FP != NULL)
		    /* Run the Sun Spell checker. */
		    if ((*SDtInvokeSpell_FP)(pPad->editor))
			    return;
    }
#endif

    /* If not Sun, or if Sun but could not open the Solaris CDE
     * spelling checker, then default to the standard CDE spelling
     * checker.
     */
    DtEditorInvokeSpellDialog(pPad->editor);
}


/************************************************************************
 * SetSelectionMenuItems - Sets the sensitivity of [Edit] menu items
 *	that deal with the current selection in the edit window - allowing
 *	for viewOnly mode.
 ************************************************************************/
/* ARGSUSED */
void
SetSelectionMenuItems(
        Editor *pPad,
	Boolean sensitivity)
{
    XtSetSensitive(pPad->editStuff.widgets.cutBtn,
      sensitivity && ! pPad->xrdb.viewOnly);
    XtSetSensitive(pPad->editStuff.widgets.copyBtn,
      sensitivity);			/* Copy can be done in viewOnly mode */
    XtSetSensitive(pPad->editStuff.widgets.clearBtn,
      sensitivity && ! pPad->xrdb.viewOnly);
    XtSetSensitive(pPad->editStuff.widgets.deleteBtn,
      sensitivity && ! pPad->xrdb.viewOnly);
}


/************************************************************************
 * TextSelectedCB - DtEditor widget DtNtextSelectCallback called when
 *	text in the editor window is selected.
 *	Makes [Edit] menu items related to a selection sensitive.
 ************************************************************************/
/* ARGSUSED */
void
TextSelectedCB(
	Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Editor *pPad = (Editor *)client_data;
    SetSelectionMenuItems(pPad, True);
}


/************************************************************************
 * TextDeselectedCB - DtEditor widget DtNtextSelectCallback called when
 *	text in the editor window is deselected.
 *	Makes [Edit] menu items related to a selection insensitive.
 ************************************************************************/
/* ARGSUSED */
void
TextDeselectedCB(
	Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Editor *pPad = (Editor *)client_data;
    SetSelectionMenuItems(pPad, False);
}
