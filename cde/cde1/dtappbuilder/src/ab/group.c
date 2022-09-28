/*
 *      $XConsortium: group.c /main/cde1_maint/2 1995/10/16 13:44:35 rcs $
 *
 * @(#)group.c	1.6 23 May 1994      cde_app_builder/src/ab
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
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Mrm/MrmAppl.h>
#include <ab_private/ab.h>

/*************************************************************************
**
**       Global  Data
**
**************************************************************************/
static const char	*groupwin_dialog = { "ggpwin112" };

/*************************************************************************
**
**       Function Definitions
**
**************************************************************************/

/*
 * Popup the group window by managing the BulletinBoard child of the
 * Dialog shell.
 */
void
group_show_dialog(
    Widget              widget,
    MrmHierarchy	s_mrm_hierarchy
)
{
    Widget	shell;
    Widget      *children;
    MrmType     class   = NULL;
 
    /* Get the dialog shell for the group popup window.  */

    if (AB_group_dialog == NULL)
    {
	if (MrmFetchWidget(s_mrm_hierarchy, (String)groupwin_dialog,
                 widget, &shell, &class) != MrmSUCCESS)
	{
	    fprintf(stderr,"Can't fetch interface\n");
	    exit(1);
	}

        XtAddCallback(shell, XtNpopupCallback,
                ab_dialog_popupCB, (XtPointer)GroupEditorUp);
        XtAddCallback(shell, XtNpopdownCallback,
                ab_dialog_popdownCB, (XtPointer)GroupEditorUp);

	/* Now get the handle to the bulletin board widget, so that
	* we can popup the group window by managing the BBoard.
	*/
	XtVaGetValues(shell, XmNchildren, &children, NULL);

	/* There can be only one child of a shell widget, and
	* we know that the child is the BBoard widget.
	*/
	if (*children != NULL)
	{
	    /* Store the group dialog bulletin board handle in
	     * a global variable so that it can be easily accessed.
	     */
	    AB_group_dialog = *children;
	}
    }
    XtManageChild(AB_group_dialog);
}
