/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#include "Views.h"
#include "RoamMenuWindow.h"
#include "RoamApp.h"
#include <X11/Intrinsic.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ArrowBG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Connection.hh>
#include "MailMsg.h"
#include "Help.hh"
#include <nl_types.h>

#define NEW_VIEW 1	// index of the new message view

/////////////////////////////////////////////////////////////////////////
Views::Views(RoamMenuWindow *parent) : Dialog("Views", parent)
{
    _find_dialog = NULL;
    _parent = parent;
    _mbox = parent->mailbox();
    _header_list = parent->list();
    _search_obj = new SdmSearch;
    _views = new SdmVector<ViewInfo *>(5);
    _list_selected = 0;
    _current_view_index = 0;	// zero is the index for the "All" view
    ViewInfo *view;

    // 2 default views, All and unread
    view = new ViewInfo;
    view->_name = strdup(catgets(DT_catd, 17, 1, "All"));
    view->_search_criteria = new SdmString(SdmSearch::Sdm_STK_All);
    view->_unread = 0;
    view->_total = 0;
    _views->AddElementToList(view);

    view = new ViewInfo;
    view->_name = strdup(catgets(DT_catd, 17, 2, "New"));
    view->_search_criteria = new SdmString(SdmSearch::Sdm_STK_Unread);
    view->_unread = 0;
    view->_total = 0;
    _views->AddElementToList(view);

    SdmError error;
    int num_views = loadUserDefinedViews();
}

/////////////////////////////////////////////////////////////////////////
Views::~Views()
{
    _views->ClearAllElements();
    delete _search_obj;
}

//
// Views::loadUserDefinedViews is a private member function that
// extracts the user's custom views from their .mailrc file and loads
// them into the array of views maintained by this class.
/////////////////////////////////////////////////////////////////////////
int Views::loadUserDefinedViews()
{
    SdmError error;
    SdmConnection *con = theRoamApp.connection()->connection();
    SdmMailRc *mailrc;
    char *views_str;
    SdmVector<char *> views;
    struct ViewInfo *vi;
    int len=0, j=0, i;
    char *tmp;

    error.Reset();
    con->SdmMailRcFactory(error, mailrc);
    assert(!error);

    // The format of the views mailrc variable is as follows:
    // set views='name1:search criteria;name2:search criteria'
    //
    mailrc->GetValue(error, "views", (char **) &views_str);
    if (error)
	return(0);

    // If the views_str contains the escape character '\', we
    // need to do special processing.
    if (strstr(views_str, "\\")) {
        len = strlen(views_str);
	tmp = (char *)malloc(sizeof(char)*len);
	for (i=0; i<len; i++) {
	    if ((views_str[i] == ';') && (i != 0)) {
		// Check if this semi-colon has been escaped
		if (views_str[i-1] != '\\') {
		    tmp[j] = '\0';
		    views.AddElementToList(strdup(tmp));
		    j=0;
		} else {
		    tmp[j++] = views_str[i];
		}
	    } else {
		tmp[j++] = views_str[i];
	    }
	}
	// add the last view string to the list of views
	tmp[j] = '\0';
	views.AddElementToList(strdup(tmp));
    } else {
	// Split up the big string returned from the mailrc into a list
	// of individual view strings.  They will be of the format:
	//		"view_name:search_criteria_string"
	// search_criteria_string may have embedded spaces.
	//
	char *token;

	token = strtok(views_str, ";");
	while (token) {
	    views.AddElementToList(strdup(token));
	    token = strtok((char *)0, ";");
	}
    }
    free(views_str);

    //
    // Extract the ViewInfo stuff out of the view string.
    //
    for (i=0; i<views.ElementCount(); i++) {
	vi = new ViewInfo;

	// If the view contains an escape character
	// then it will need special handling.
	if (strstr(views[i], "\\")) {
	    char *str = views[i];
	    j=0;
	    len = strlen(str);
	    tmp = (char *)malloc(sizeof(char)*len);
	    for (int k=0; k<len; k++) {
		if ((str[k] == ':') && (k != 0)) {
		    // Check if this colon has been escaped
		    if (str[k-1] != '\\') {
			tmp[j] = '\0';
			vi->_name = stripEscapeChar(tmp);
			k++;
			// Since there can only be one unescaped colon in
			// the views string, we can go ahead and just
			// copy the remaining portion of the views string
			// to the tmp buffer.  This is the search criteria.
			len = len - k;
			for (j=0; j<=len; j++) {
			    tmp[j] = str[k++];
			}
			tmp[j] = '\0';
			char *tmp2 = stripEscapeChar(tmp);
			vi->_search_criteria = new SdmString(tmp2);
			free(tmp2);
			vi->_unread = 0;
			vi->_total = 0;
			_views->AddElementToList(vi);
		    } else {
			tmp[j++] = str[k];
		    }
		} else {
		    tmp[j++] = str[k];
		}
	    }
	    free(tmp);
	} else {
	    char *token;
	    token = strtok(views[i], ":");
	    if (!token) {
		delete vi;
		continue;
	    }
	    vi->_name = strdup(token);
	    token = strtok((char *)0, ":");
	    if (!token) {
		free (vi->_name);
		delete vi;
		continue;
	    } else {
		vi->_search_criteria = new SdmString(token);
	    }
	    vi->_unread = 0;
	    vi->_total = 0;
	    _views->AddElementToList(vi);
	}
    }
    return(views.ElementCount());
}

#define L_MARGIN 3
#define R_MARGIN 97

/////////////////////////////////////////////////////////////////////////
Widget Views::createWorkArea(Widget dialog)
{
    XmString lbl1, lbl2, lbl3, str;
    char dialog_title[128], *dialog_name, *mbox_name;
//     XmFontListEntry font_list_entry;
//     XmFontList font_list, new_font_list;

    mbox_name = _parent->mailboxName();
    if (strlen(mbox_name) == 0) {
	mbox_name = catgets(DT_catd, 17, 3, "Inbox");
    }
    dialog_name = catgets(DT_catd, 17, 4, "Views");
    sprintf(dialog_title, "%s [%s]", dialog_name, mbox_name);
    _name = strdup(dialog_title);

    title(_name);

    Widget vd_pane = XtVaCreateWidget("vd_pane",
				      xmPanedWindowWidgetClass,
				      dialog,
				      XmNsashHeight, 1,
				      XmNsashWidth, 1,
				      XmNspacing, 7,
				      XmNmarginHeight, 3,
				      NULL);

    Widget vd_form1 = XtVaCreateWidget("vd_form1",
				       xmFormWidgetClass,
				       vd_pane,
				       XmNfractionBase, 100,
				       NULL);

    lbl1 = XmStringCreateLocalized(catgets(DT_catd, 17, 19, "Default Views"));
    lbl2 = XmStringCreateLocalized(catgets(DT_catd, 17, 20, "New"));
    lbl3 = XmStringCreateLocalized(catgets(DT_catd, 17, 21, "Total"));
    Widget vd_label1 = XtVaCreateManagedWidget("vd_label1",
			       xmLabelGadgetClass,
			       vd_form1,
			       XmNlabelString, lbl1,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, L_MARGIN,
			       XmNtopAttachment, XmATTACH_FORM,
			       NULL);
    Widget vd_label2 = XtVaCreateManagedWidget("vd_label2",
			       xmLabelGadgetClass,
			       vd_form1,
			       XmNlabelString, lbl2,
			       XmNtopAttachment, XmATTACH_FORM,
			       NULL);
    Widget vd_label3 = XtVaCreateManagedWidget("vd_label3",
			       xmLabelGadgetClass,
			       vd_form1,
			       XmNlabelString, lbl3,
			       XmNtopAttachment, XmATTACH_FORM,
			       NULL);

    _list_w[0] = XtVaCreateManagedWidget("vd_default_list",
				   xmListWidgetClass,
				   vd_form1,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, vd_label1,
				   XmNleftAttachment, XmATTACH_POSITION,
				   XmNleftPosition, L_MARGIN,
				   XmNrightAttachment, XmATTACH_POSITION,
				   XmNrightPosition, R_MARGIN,
				   XmNselectionPolicy, XmBROWSE_SELECT,
				   XmNvisibleItemCount, 2,
				   NULL);

    XtAddCallback(_list_w[0], XmNbrowseSelectionCallback,
		  (XtCallbackProc)&Views::selectionCallback, (XtPointer)this);
    XtAddCallback(_list_w[0], XmNdefaultActionCallback,
		  (XtCallbackProc)&Views::doubleClickCallback,(XtPointer)this);

    XmStringFree(lbl1);
    lbl1 = XmStringCreateLocalized(catgets(DT_catd, 17, 22, "Custom Views"));
    Widget vd_label4 = XtVaCreateManagedWidget("vd_label4",
				xmLabelGadgetClass,
				vd_form1,
				XmNlabelString, lbl1,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNleftPosition, L_MARGIN,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, _list_w[0],
				NULL);
    Widget vd_label5 = XtVaCreateManagedWidget("vd_label5",
			       xmLabelGadgetClass,
			       vd_form1,
			       XmNlabelString, lbl2,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, _list_w[0],
			       NULL);
    Widget vd_label6 = XtVaCreateManagedWidget("vd_label6",
			       xmLabelGadgetClass,
			       vd_form1,
			       XmNlabelString, lbl3,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, _list_w[0],
			       NULL);
    XmStringFree(lbl1);
    XmStringFree(lbl2);
    XmStringFree(lbl3);

    Arg args[32];
    int n=0;
    XtSetArg(args[n], XmNvisualPolicy, XmCONSTANT); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
    _list_w[1] = XmCreateScrolledList(vd_form1,
				      "vd_custom_list",
				      args,
				      n);
    Widget scrolled_window = XtParent(_list_w[1]);
    XtVaSetValues(scrolled_window,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, vd_label4,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, L_MARGIN,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, R_MARGIN,
		  NULL);
    XtVaSetValues(_list_w[1],
		  XmNvisibleItemCount, 12,
		  XmNselectionPolicy, XmBROWSE_SELECT,
		  NULL);

    XtAddCallback(_list_w[1], XmNbrowseSelectionCallback,
		  (XtCallbackProc)&Views::selectionCallback, (XtPointer)this);
    XtAddCallback(_list_w[1], XmNdefaultActionCallback,
		  (XtCallbackProc)&Views::doubleClickCallback,(XtPointer)this);

    _move_up_w = XtVaCreateManagedWidget("move_up",
					 xmArrowButtonGadgetClass,
					 vd_form1,
					 XmNsensitive, False,
					 XmNbottomAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_POSITION,
					 XmNrightPosition, 49,
					 NULL);

    XtVaSetValues(scrolled_window,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, _move_up_w,
		  XmNbottomOffset, 3,
		  NULL);
    _move_down_w = XtVaCreateManagedWidget("move_down",
					   xmArrowButtonGadgetClass,
					   vd_form1,
					   XmNsensitive, False,
					   XmNarrowDirection, XmARROW_DOWN,
					   XmNbottomAttachment, XmATTACH_FORM,
					   XmNleftAttachment, XmATTACH_POSITION,
					   XmNleftPosition, 51,
					   NULL);

    XtAddCallback(_move_up_w, XmNactivateCallback, &Views::moveUpCallback,
		  (XtPointer) this);
    XtAddCallback(_move_down_w, XmNactivateCallback, &Views::moveDownCallback,
		  (XtPointer) this);

    Widget vd_form2 = XtVaCreateWidget("vd_form2",
				       xmFormWidgetClass,
				       vd_pane,
				       XmNfractionBase, 100,
				       XmNallowResize, False,
				       NULL);
    str = XmStringCreateLocalized(catgets(DT_catd, 17, 8, "New..."));
    Widget new_w = XtVaCreateManagedWidget("new_w",
				     xmPushButtonGadgetClass,
				     vd_form2,
				     XmNlabelString, str,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_POSITION,
				     XmNleftPosition, 15,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 36,
				     NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd, 17, 9, "Edit..."));
    Widget edit_w = XtVaCreateManagedWidget("edit_w",
				      xmPushButtonGadgetClass,
				      vd_form2,
				      XmNlabelString, str,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_POSITION,
				      XmNleftPosition, 40,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 61,
				      NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd, 17, 10, "Delete"));
    _delete_w = XtVaCreateManagedWidget("_delete_w",
					xmPushButtonGadgetClass,
					vd_form2,
					XmNlabelString, str,
					XmNtopAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_POSITION,
					XmNleftPosition, 65,
					XmNrightAttachment, XmATTACH_POSITION,
					XmNrightPosition, 86,
					XmNsensitive, False,
					NULL);
    XmStringFree(str);

    XtAddCallback(new_w, XmNactivateCallback, &Views::newCallback,
		  (XtPointer) this);
    XtAddCallback(_delete_w, XmNactivateCallback, &Views::deleteCallback,
		  (XtPointer) this);
    XtAddCallback(edit_w, XmNactivateCallback, &Views::editCallback,
		  (XtPointer) this);


    str = XmStringCreateLocalized(catgets(DT_catd, 17, 11, "Load"));
    Widget load_w = XtVaCreateManagedWidget("load_w",
				      xmPushButtonGadgetClass,
				      vd_form2,
				      XmNlabelString, str,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, new_w,
				      XmNtopOffset, 7,
				      XmNleftAttachment, XmATTACH_POSITION,
				      XmNleftPosition, L_MARGIN,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, 23,
				      XmNbottomAttachment, XmATTACH_FORM,
				      NULL);
    XmStringFree(str);

    str = XmStringCreateLocalized(catgets(DT_catd, 17, 12, "Update"));
    Widget update_w = XtVaCreateManagedWidget("update_w",
					xmPushButtonGadgetClass,
					vd_form2,
					XmNlabelString, str,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, new_w,
				      XmNtopOffset, 7,
					XmNleftAttachment, XmATTACH_POSITION,
					XmNleftPosition, 27,
					XmNrightAttachment, XmATTACH_POSITION,
					XmNrightPosition, 48,
					XmNbottomAttachment, XmATTACH_FORM,
					NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd, 17, 13, "Close"));
    Widget close_w = XtVaCreateManagedWidget("close_w",
				       xmPushButtonGadgetClass,
				       vd_form2,
				       XmNlabelString, str,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, new_w,
				      XmNtopOffset, 7,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNleftPosition, 52,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNrightPosition, 73,
				       XmNbottomAttachment, XmATTACH_FORM,
				       NULL);
    XmStringFree(str);
    str = XmStringCreateLocalized(catgets(DT_catd, 17, 14, "Help"));
    Widget help_w = XtVaCreateManagedWidget("help_w",
				      xmPushButtonGadgetClass,
				      vd_form2,
				      XmNlabelString, str,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, new_w,
				      XmNtopOffset, 7,
				      XmNrightAttachment, XmATTACH_POSITION,
				      XmNrightPosition, R_MARGIN,
				      XmNleftAttachment, XmATTACH_POSITION,
				      XmNleftPosition, 77,
				      XmNbottomAttachment, XmATTACH_FORM,
				      NULL);
    XmStringFree(str);

    XtAddCallback(load_w, XmNactivateCallback, &Views::loadCallback,
		  (XtPointer) this);
    XtAddCallback(update_w, XmNactivateCallback, &Views::updateCallback,
		  (XtPointer) this);
    XtAddCallback(close_w, XmNactivateCallback, &Views::closeCallback,
		  (XtPointer) this);
    XtAddCallback(help_w, XmNactivateCallback, HelpCB,
		  (XtPointer) DTMAILHELPVIEWS);

    Widget status_form = XtVaCreateWidget("status_form",
					  xmFormWidgetClass,
					  vd_pane,
					  NULL);
    _status_text = XtVaCreateManagedWidget("status_text",
					   xmLabelGadgetClass,
					   status_form,
					   XmNbottomAttachment, XmATTACH_FORM,
					   NULL);
    setStatus(" ");

    // This prevents the View dialog from being mapped when the user
    // adds a view from the FindDialog object and the Views dialog did not
    // exist.  We do not want to "popup" in this scenario.
    XtVaSetValues(_w,
		  XmNmappedWhenManaged, False,
		  NULL);

    Position margin;
    int pos=0, char_width, col1, col2;
    XmFontList font_list;
    XmString xmstr;

    xmstr = XmStringCreateLocalized(" ");
    XtVaGetValues(_list_w[1], XmNfontList, &font_list, NULL);
    char_width = XmStringWidth(font_list, xmstr);
    XtVaGetValues(vd_label1, XmNx, &margin, NULL);

    col1 = 26;
    col2 = 6;

    // Layout the header labels for the default views list.
    pos = (col1 * char_width) + margin;
    XtVaSetValues(vd_label2, XmNx, pos, NULL);
    pos += (col2 * char_width);
    XtVaSetValues(vd_label3, XmNx, pos, NULL);

    // Layout the header labels for the custom views list.
    // The custom list may have a vertical scrollbar that we
    // need to adjust the headers for.
    pos=0;
    unsigned char sbp;
    XtVaGetValues(scrolled_window, XmNscrollBarPlacement, &sbp, NULL);
    if (sbp == XmTOP_LEFT || sbp == XmBOTTOM_LEFT) {
	Widget sb = NULL;
	Dimension width = 0;
	Dimension spacing = 0;

	XtVaGetValues(scrolled_window, XmNverticalScrollBar, &sb, NULL);
	if (sb != NULL) {
	    if (XtIsManaged(sb)) {
		XtVaGetValues(sb, XmNwidth, &width, NULL);
		XtVaGetValues(sb, XmNspacing, &spacing, NULL);
	    }
	}
	pos = width + spacing;
    }
    pos += (col1 * char_width) + margin;
    XtVaSetValues(vd_label5, XmNx, pos, NULL);
    pos += (col2 * char_width);
    XtVaSetValues(vd_label6, XmNx, pos, NULL);
    pos += (8 * char_width) + 10;
    XtVaSetValues(vd_form1, XmNwidth, pos, NULL);

    XtManageChild(scrolled_window);
    XtManageChild(_list_w[1]);
    XtManageChild(vd_form1);
    XtManageChild(vd_form2);
    XtManageChild(status_form);
    XtManageChild(vd_pane);

    Dimension height;

    XtVaGetValues(vd_form2, XmNheight, &height, NULL);
    XtVaSetValues(vd_form2,
		  XmNpaneMaximum,	height,
		  XmNpaneMinimum,	height,
		  NULL);
    XtVaGetValues(status_form, XmNheight, &height, NULL);
    XtVaSetValues(status_form,
		  XmNpaneMaximum,	height,
		  XmNpaneMinimum,	height,
		  NULL);

    // Need to setup bold font that is used to represent the current view.
    // Since we can't modify the fontlist returned from the getvalues call,
    // we must make a copy of it and then add our new fontListEntry.  We must
    // free both the fontListEntry and the fontList items (but not the
    // fontList returned from the getvalues call).
//     font_list_entry = XmFontListEntryLoad(XtDisplay(dialog),
// 		  "-dt-interface user-bold-*-*-*-*-*-*-*-*-*-*-*-*",
// 		  XmFONT_IS_FONT, "bold");
//     XtVaGetValues(_list_w[0], XmNfontList, &font_list, NULL);
//     new_font_list = XmFontListCopy(font_list);
//     new_font_list = XmFontListAppendEntry(new_font_list, font_list_entry);
//     XmFontListEntryFree(&font_list_entry);

//     XtVaSetValues(_list_w[0], XmNfontList, new_font_list, NULL);
//     XtVaSetValues(_list_w[1], XmNfontList, new_font_list, NULL);
//     XmFontListFree(new_font_list);

    XmListSelectPos(_list_w[1], 1, True);

    return(vd_pane);
}

/////////////////////////////////////////////////////////////////////////
void Views::popdown()
{
    XtPopdown(_w);
}

/////////////////////////////////////////////////////////////////////////
void Views::setStatus( const char *str )
{
    char *tmpstr = strdup(str);
    XmString label = XmStringCreateLocalized(tmpstr);
 
    XtVaSetValues(_status_text,
                  XmNlabelString, label,
                  NULL);
 
    XmUpdateDisplay(baseWidget());
    XmStringFree(label);
    free(tmpstr);
}

/////////////////////////////////////////////////////////////////////////
void Views::popup()
{
    // We need to set the mappedWhenManaged resource so that we can avoid
    // doing unnecessary work if the dialog is not mapped.  Specifically,
    // this resource will be checked in Views::addView().
    //
    XtVaSetValues(_w, XmNmappedWhenManaged, True, NULL);
    XtPopup(_w, XtGrabNone);
}

/////////////////////////////////////////////////////////////////////////
void Views::popped_up()
{
    SdmError error;
    SdmMessageNumber last = _header_list->get_last_msg_num();
    SdmMessageNumberL emptyList;

    theRoamApp.busyAllWindows(catgets(DT_catd, 17, 27, "Updating Views..."));
    busyCursor();
    updateViewsWindow(error,
		      Sdm_True,
		      Sdm_False,
		      Range,
		      1, last,
		      emptyList,
		      Sdm_True);
    
    theRoamApp.unbusyAllWindows();
    normalCursor();

    if (error) {
    	setStatus(catgets(DT_catd, 17, 25, "Error updating statistics."));
    }

    _mapped = Sdm_True;
}

/////////////////////////////////////////////////////////////////////////
void Views::popped_down()
{
  _mapped = Sdm_False;
}

//
// This routine is a public method used to add a new view to the Views
// dialog.  It is called from the FindDialog in response to the user
// saving the search criteria.
/////////////////////////////////////////////////////////////////////////
void Views::addView(const char *name,
		    const SdmString &search_string)
{
    ViewInfo *vi = new ViewInfo;
    SdmMessageNumberL msg_list;
    SdmError err;
    SdmMessageNumber last;
    int i;

    _search_obj->SetSearchString(err, search_string);
    if (err) {
      _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
      return;
    }
      
    last = _header_list->get_last_msg_num();

    _mbox->PerformSearch(err, msg_list, *_search_obj, 1, last);
    if (err) {
      _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
      return;
    }

    vi->_name = strdup(name);
    vi->_search_criteria = new SdmString(search_string);
    vi->_unread = getUnreadCount(msg_list);
    vi->_total = getMessageCount(msg_list);

    SdmBoolean found = Sdm_False;
    for (i=2; i<_views->ElementCount(); i++) {
	if (strcmp((*_views)[i]->_name, name) == 0) {
	    found = Sdm_True;
	    break;
	}
    }

    char *str;
    XmString item;
    int pos;

    if (found) {
	_views->RemoveElement(i);
	_views->InsertElementBefore(vi, i);
	str = formatViewInfo(vi);
	item = XmStringCreateLocalized(str);
	XmListReplaceItemsPos(_list_w[1], &item, 1, i-1);
    } else {
	pos = _views->AddElementToList(vi);
	str = formatViewInfo(vi);
	item = XmStringCreateLocalized(str);
	XmListAddItem(_list_w[1], item, pos - 1);
    }
    updateMailrc();
    free(str);
    XmStringFree(item);
}

//
/////////////////////////////////////////////////////////////////////////
int Views::getUnreadCount(const SdmMessageNumberL &msgs)
{
    SdmError error;
    int count=0, i;
    SdmMessageFlagAbstractFlagsL flagsList = 0;
    SdmMessageStore *mbox = _parent->mailbox();

    mbox->GetFlags(error, flagsList, msgs);
    if (error) {
      _parent->genDialog()->post_error(error);
      return (-1);
    }

    for (i=0; i<msgs.ElementCount(); i++) {
	if ( !(flagsList[i] & Sdm_MFA_Seen) && !(flagsList[i] & Sdm_MFA_Deleted)) {
	    count++;
	}
    }
    return (count);
}

//
/////////////////////////////////////////////////////////////////////////
int Views::getMessageCount(const SdmMessageNumberL &msgs)
{
    SdmError error;
    int count=0, i;
    SdmMessageFlagAbstractFlagsL flagsList = 0;
    SdmMessageStore *mbox = _parent->mailbox();

    mbox->GetFlags(error, flagsList, msgs);
    if (error) {
      _parent->genDialog()->post_error(error);
      return (-1);
    }

    for (i=0; i<msgs.ElementCount(); i++) {
	if ( !(flagsList[i] & Sdm_MFA_Deleted)) {
	    count++;
	}
    }
    return (count);
}

//
//	regenerate	- causes the appropriate stats to be recalculated
//	unread_only	- Will only update the unread stat for the appropriate
//			  views. This is used to update stats when an unread
//			  message becomes "read".  Since the total # messages
//			  doesn't change no need to waste our time recalculating
//			  anything.
//	type		- SingleMessage, Range, or List
//	start		- start message number for Range type
//	end		- last message number in Range
// 	msgs		- list of message numbers
//	add_to		- determines whether we increment or decrement stats
/////////////////////////////////////////////////////////////////////////
void Views::updateViewsWindow(SdmError &err,
			      SdmBoolean regenerate,
			      SdmBoolean unread_only,
			      UpdateType type,
			      SdmMessageNumber start,
			      SdmMessageNumber end,
			      SdmMessageNumberL &msgs,
			      SdmBoolean add_to)
{
    int view_count, msg_count, i, *p_list, p_count, pos=0, selected_pos=0;
    char *str;
    SdmBoolean in_view, changed = Sdm_False, update_all = Sdm_False;
    SdmMessageNumberL msg_list;
    SdmMessageFlagAbstractFlagsL flagsList = 0;
    SdmMessageFlagAbstractFlags flags = 0;
    char *error_buf = catgets(DT_catd, 17, 26, "The view \"%s\" is invalid.\n");
    SdmErrorCode error_code = 6000;

    // Remember which view is selected
    if (XmListGetSelectedPos(_list_w[_list_selected], &p_list, &p_count)) {
	selected_pos = p_list[0];
	free(p_list);
    }

    view_count = _views->ElementCount();
    int *pos_list = (int *)malloc(sizeof(int) * view_count);

    if (regenerate) {
	switch (type) {
	case SingleMessage:
	    // SingleMessage has two cases:
	    //	- reading an unread message
	    //	- undeleting the last deleted message
	    pos=0;
	    for (i=0; i<view_count; i++) {
		SdmString *ss = (*_views)[i]->_search_criteria;
		_search_obj->SetSearchString(err, (const SdmString)*ss);
		if (err) {
		    err.AddMajorErrorCodeAndMessageWithFormat(error_code,
 		       error_buf, (*_views)[i]->_name);
		    _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
		    err.Reset();
		    // Remove the bogus view and adjust the counts.
		    _views->RemoveElement(i); i--; view_count--;
		    continue;
		}
		_mbox->PerformSearch(err, in_view, *_search_obj, start);
		if (err) {
		    err.AddMajorErrorCodeAndMessageWithFormat(error_code,
 		       error_buf, (*_views)[i]->_name);
		    _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
		    err.Reset();
		    // Remove the bogus view and adjust the counts.
		    _views->RemoveElement(i); i--; view_count--;
		    continue;
		}
		// The new message view is kind of a strange beast.  Once you
		// have read a message in this view, that message no longer belongs
		// there because it is no longer a new message.  So lets just pretend
		// that the message in question (start) actually still meets the
		// criteria of the view.
		if ((!in_view) && (i != NEW_VIEW))
		    continue;

		pos_list[pos++] = i;

		if (unread_only) {	// reading an unread message
		    (*_views)[i]->_unread -= 1;
		    if ((_current_view_index != NEW_VIEW) && (i == NEW_VIEW)) {
			(*_views)[NEW_VIEW]->_total -= 1;
		    }
		} else if (add_to) {	// undeleting last deleted message
		    SdmMessageStore *mbox = _parent->mailbox();
		    mbox->GetFlags(err, flags, start);
		    if (err) return;
		    if ( !(flags & Sdm_MFA_Seen) )
			(*_views)[i]->_unread += 1;
		    // GL - If we are updating the stats for the new view and the current
		    // view is the new view, then increment the new view total.  I know
		    // this seems odd, but its the right thing to do because you should
		    // be able to undelete messages that you deleted in the new view.
		    // Trust me.
		    if (i == NEW_VIEW) {
			if (_current_view_index == NEW_VIEW) {
			    (*_views)[i]->_total += 1;
			}
		    } else {
			(*_views)[i]->_total += 1;
		    }
		}
	    }
	    break;
	case Range:
	    // No messages in the mailbox.
	    if (end > 0) {
		pos=0;
		for (i=0; i<view_count; i++) {
		    SdmString *ss = (*_views)[i]->_search_criteria;
		    _search_obj->SetSearchString(err, (const SdmString)*ss);
		    // What to do about an error??
		    if (err) {
		        err.AddMajorErrorCodeAndMessageWithFormat(error_code,
			    error_buf, (*_views)[i]->_name);
			_parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
			err.Reset();
			// Remove the bogus view and adjust the counts.
			_views->RemoveElement(i); i--; view_count--;
			continue;
		    }
		    _mbox->PerformSearch(err, msg_list, *_search_obj, start, end);
		    if (err) {
		        err.AddMajorErrorCodeAndMessageWithFormat(error_code,
			    error_buf, (*_views)[i]->_name);
			_parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
			err.Reset();
			// Remove the bogus view and adjust the counts.
			_views->RemoveElement(i); i--; view_count--;
			continue;
		    }
		    msg_count = msg_list.ElementCount();
		    if (msg_count == 0 && start != 1)
			continue;
		    pos_list[pos++] = i;

		    if (start == 1) {	// manual update
			(*_views)[i]->_total = getMessageCount(msg_list);
			(*_views)[i]->_unread = getUnreadCount(msg_list);
		    } else if (add_to) {	// new mail and undelete range
			(*_views)[i]->_total += msg_count;
			(*_views)[i]->_unread += getUnreadCount(msg_list);
		    } else {		// delete range
			(*_views)[i]->_total -= msg_count;
			(*_views)[i]->_unread -= getUnreadCount(msg_list);
		    }
		    msg_list.ClearAllElements();
		}
	    }
	    if (start == 1)
		update_all = Sdm_True;
	    break;
	case List:
	    pos=0;
	    for (i=0; i<view_count; i++) {
		SdmString *ss = (*_views)[i]->_search_criteria;
		_search_obj->SetSearchString(err, (const SdmString)*ss);
		if (err) {
		    err.AddMajorErrorCodeAndMessageWithFormat(error_code,
			error_buf, (*_views)[i]->_name);
		    _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
		    err.Reset();
		    // Remove the bogus view and adjust the counts.
		    _views->RemoveElement(i); i--; view_count--;
		    continue;
		}
		_mbox->PerformSearch(err, msg_list, *_search_obj, msgs);
		if (err) {
		    err.AddMajorErrorCodeAndMessageWithFormat(error_code,
			error_buf, (*_views)[i]->_name);
		    _parent->genDialog()->post_error(err, DTMAILHELPINVALIDVIEW);
		    err.Reset();
		    // Remove the bogus view and adjust the counts.
		    _views->RemoveElement(i); i--; view_count--;
		    continue;
		}
		if ((i == NEW_VIEW) && (_current_view_index == NEW_VIEW)) {
		    msg_count = msgs.ElementCount();
		} else {
		    msg_count = msg_list.ElementCount();
		}
		if (msg_count == 0)
		    continue;
		pos_list[pos++] = i;

		if (add_to) {	// undelete
		    (*_views)[i]->_total += msg_count;
		    (*_views)[i]->_unread += getUnreadCount(msg_list);
		} else {		// delete
		    (*_views)[i]->_total -= msg_count;
		    (*_views)[i]->_unread -= getUnreadCount(msg_list);
		}
		msg_list.ClearAllElements();
	    }
	    break;
	}
    }

    int default_count = 0, custom_count = 0;
    XmString default_items[2], *custom_items;

    // allocate enough space to hold all of the custom view entries
    custom_items = new XmString[view_count];

    // Update both the default and custom scrolling list with the new stats
    if (update_all) {
	XmListDeleteAllItems(_list_w[0]);
	XmListDeleteAllItems(_list_w[1]);
	for (i=0; i<view_count; i++) {
	    str = formatViewInfo((*_views)[i]);
	    if (i < 2)
		default_items[default_count++] = XmStringCreateLocalized(str);
	    else
		custom_items[custom_count++] = XmStringCreateLocalized(str);
	    free(str);
	}
	XmListAddItems(_list_w[0], default_items, default_count, 1);
	XmListAddItems(_list_w[1], custom_items, custom_count, 1);
    } else {
	// If any of the view stats changed, update only those stats
	if (pos > 0) {
	    for (i=0; i<pos; i++) {
		str = formatViewInfo((*_views)[pos_list[i]]);
		if (pos_list[i] < 2) {
		    default_items[default_count++] = XmStringCreateLocalized(str);
		    pos_list[i] += 1;
		} else {
		    custom_items[custom_count++] = XmStringCreateLocalized(str);
		    // subtract 1 from the value at pos_list[i] so that it refers
		    // to the list position of views in the custom list.  We are
		    // actually subtracting 2 for the number of default views and
		    // then adding 1 because list items go from 1 to n not 0 to n-1.
		    pos_list[i] -= 1;
		}
		free(str);
	    }
	    // We will always have to update the default view "All" so just do it.
	    //
	    XmListReplacePositions(_list_w[0], pos_list, default_items, default_count);

	    // Update only those views whose stats have changed
	    if (custom_count > 0) {
		XmListReplacePositions(_list_w[1], &pos_list[default_count],
				       custom_items, custom_count);
	    }
	}
    }

    delete [] custom_items;

    // If there was a view selected before we rebuilt the list then
    // select it.  Don't actually load the view though.
    if (selected_pos != 0)
	XmListSelectPos(_list_w[_list_selected], selected_pos, False);
    else
        // Select the "All" view in the default list.
	XmListSelectPos(_list_w[0], 1, True);

    free (pos_list);
}

/////////////////////////////////////////////////////////////////////////
char *Views::formatViewInfo(ViewInfo *vi)
{
    char str[128];

    sprintf(str, "%-21.21s     %-4.1d  %-5.1d",
	    vi->_name,
	    vi->_unread,
	    vi->_total);
    return (strdup(str));
}

/////////////////////////////////////////////////////////////////////////
void Views::popupFindDialog()
{
    if (_find_dialog == NULL) {
	_find_dialog = parent()->get_find_dialog();
	_find_dialog->clearFields();
    } else {
	// If we don't call manage() before popup() the find dialog will
	// come up empty if it was dismissed via the window manager close
	_find_dialog->clearFields();
	_find_dialog->manage();
	_find_dialog->popup();
    }
}

/////////////////////////////////////////////////////////////////////////
void Views::moveUpCallback(Widget,
			   XtPointer client_data,
			   XtPointer)
{
    Views *vp = (Views *)client_data;
    int *position, count;
    XmStringTable selected_items;
    XmString selected_item;
    Boolean status;

    status = XmListGetSelectedPos(vp->_list_w[1], &position, &count);
    if (count == 0 || !status || *position == 1) {
        return;
    }

    XtVaGetValues(vp->_list_w[1],
                  XmNselectedItems, &selected_items,
                  NULL);

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(vp->_list_w[1], *position);
    XmListAddItem(vp->_list_w[1], selected_item, (*position)-1);
    XmListSelectPos(vp->_list_w[1], (*position)-1, True);
    if (!XmListPosToBounds(vp->_list_w[1], (*position)-1, NULL, NULL, NULL, NULL))
	XmListSetBottomPos(vp->_list_w[1], (*position)-1);
    XmStringFree(selected_item);

    // Remember, views 0 and 1 are the default views "all" and "new".
    // We have to skip over them to get at the custom views.
    // Also remember that the list widget deals with items 1 through n,
    // while the _views list is indexed 0 thru n-1.
    ViewInfo *vi = (*vp->_views)[*position + 1];
    vp->_views->RemoveElement(*position + 1);
    vp->_views->InsertElementBefore(vi, *position);
    vp->updateMailrc();

    XtFree((char *)position);
}

/////////////////////////////////////////////////////////////////////////
void Views::moveDownCallback(Widget,
			     XtPointer client_data,
			     XtPointer)
{
    Views *vp = (Views *)client_data;
    int *position, item_count;
    XmStringTable selected_items;
    XmString selected_item;
    Boolean status;

    status = XmListGetSelectedPos(vp->_list_w[1], &position, &item_count);
    XtVaGetValues(vp->_list_w[1],
                  XmNselectedItems, &selected_items,
                  NULL);
    if (!status || *position == vp->_views->ElementCount() - 2) {
        return;
    }

    selected_item = XmStringCopy(selected_items[0]);
    XmListDeletePos(vp->_list_w[1], *position);
    XmListAddItem(vp->_list_w[1], selected_item, (*position)+1);
    XmListSelectPos(vp->_list_w[1], (*position)+1, True);
    if (!XmListPosToBounds(vp->_list_w[1], (*position)+1, NULL, NULL, NULL, NULL))
	XmListSetBottomPos(vp->_list_w[1], (*position)+1);
    XmStringFree(selected_item);

    // Remember, views 0 and 1 are the default views "all" and "new".
    // We have to skip over them to get at the custom views.
    // Also remember that the list widget deals with items 1 through n,
    // while the _views list is indexed 0 - n-1.
    ViewInfo *vi = (*vp->_views)[*position + 1];
    vp->_views->RemoveElement(*position + 1);
    vp->_views->InsertElementAfter(vi, *position + 1);
    vp->updateMailrc();

    XtFree((char *)position);
}

/////////////////////////////////////////////////////////////////////////
void Views::newCallback(Widget, XtPointer client_data, XtPointer)
{
    Views *obj = (Views *)client_data;

    obj->popupFindDialog();
}

/////////////////////////////////////////////////////////////////////////
void Views::loadCallback(Widget, XtPointer client_data, XtPointer)
{
    Views *vp = (Views *)client_data;
    int index=0, *p_list, p_count;
    SdmError err;
    char *name;

    if (XmListGetSelectedPos(vp->_list_w[vp->_list_selected], &p_list,
			     &p_count)) {

	if (vp->_list_selected == 0)
	    index = p_list[0] - 1;
	else
	    index = p_list[0] + 2 - 1;
	free(p_list);

	if (vp->_views->operator[](index)->_total < 1) {
	    vp->setStatus(catgets(DT_catd, 17, 15, "Selected view is empty"));
	    return;
	}
	// If the view we are replacing is the "New" view, then we need
	// to manually update the "New" view stats.  This is because as the
	// user reads messages in the "New" view, those read messages no longer meet
	// the "New" view criteria.  So if we were to reload the "New"
	// view, we would not get the same number of messages.  While we're in the
	// "New" view
	// we don't decrement it's total count (because we are not
	// deleting them as they are read).  I know this is confusing, are you
	// still with me?  So before we replace the "New" view, we should correct
	// it's total count.
	if (vp->_current_view_index == 1) {
	  vp->updateNewViewStats();
	}
	vp->_current_view_index = index;

	theRoamApp.busyAllWindows(catgets(DT_catd, 17, 16, "Loading view..."));
	vp->busyCursor();
	vp->setStatus(catgets(DT_catd, 17, 16, "Loading view..."));
	name = vp->_views->operator[](index)->_name;
	vp->_header_list->load_view(err, name,
		    vp->_views->operator[](index)->_search_criteria,
		    vp->_views->operator[](index)->_total, index);
	if (err) {
	  vp->setStatus(catgets(DT_catd, 17, 18, "Error loading view"));
	} else {
	  vp->setStatus(" ");
	  XmString str = XmStringCreateLocalized(name);
	  XmStringFree(str);
	}
	theRoamApp.unbusyAllWindows();
	vp->normalCursor();
    } else {
	vp->setStatus(catgets(DT_catd, 17, 17, "No view selected"));
    }
}

//
/////////////////////////////////////////////////////////////////////////
void Views::doubleClickCallback(Widget w,
				XtPointer client_data,
				XtPointer call_data)
{
    Views *vp = (Views *)client_data;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *)call_data;
    SdmError err;
    int new_view_index=0;
    XmString str;
    char *name;
//     XmString bold_item, tmp;

    if (w == vp->_list_w[0]) {
	vp->_list_selected = 0;
	new_view_index = cbs->item_position - 1;
    } else {
	vp->_list_selected = 1;
	new_view_index = cbs->item_position + 2 - 1;
    }

    // Make sure that the view is not empty.
    if (vp->_views->operator[](new_view_index)->_total < 1) {
	vp->setStatus(catgets(DT_catd, 17, 15, "Selected view is empty"));
	return;
    }

    // If the view we are replacing is the "New" view, then we need
    // to manually update the "New" view stats.  This is because as the
    // user reads messages in the "New" view, those read messages no longer meet
    // the "New" view criteria.  So if we were to reload the "New"
    // view, we would not get the same number of messages. While we're in the
    // "New" view
    // we don't decrement it's total count (because we are not
    // deleting them as they are read).  I know this is confusing, are you still
    // with me? So before we replace the "New" view, we should correct it's total
    // count.
    if (vp->_current_view_index == 1) {
        vp->updateNewViewStats();
    }
    vp->_current_view_index = new_view_index;

    theRoamApp.busyAllWindows(catgets(DT_catd, 17, 16, "Loading view..."));
    vp->busyCursor();

    // Make the newly selected item bold.
//     XmStringGetLtoR(cbs->item, XmSTRING_DEFAULT_CHARSET, &str);
//     bold_item = XmStringCreate(str, "bold");
//     XtFree(str);
//     XmListReplacePositions(w, &cbs->item_position, &bold_item, 1);

//     if (vp->_current_view_item != NULL) {
// 	// Extract the string from the previously loaded view item
// 	XmStringGetLtoR(vp->_current_view_item,XmSTRING_DEFAULT_CHARSET,&str);

// 	// Replace the previously bold item with normal weight item.
// 	tmp = XmStringCreateLocalized(str);
// 	XtFree(str);
// 	XmListReplaceItems(vp->_list_w[vp->_current_view_list],
// 			   &vp->_current_view_item, 1, &tmp);
// 	XmStringFree(tmp);
//     }

//     // set the newly selected view to be the "current" one
//     XmStringFree(vp->_current_view_item);
//     vp->_current_view_item = XmStringCopy(bold_item);
//     XmStringFree(bold_item);

    // Remember, we have 2 list widgets, gotta remember which one we are in!
//     vp->_current_view_list = vp->_list_selected;


//     if (vp->_views->operator[](new_view_index)->_total < 1) {
// 	vp->setStatus(catgets(DT_catd, 17, 15, "Selected view is empty"));
//     } else {
    vp->setStatus(catgets(DT_catd, 17, 16, "Loading view..."));
    name = vp->_views->operator[](new_view_index)->_name;
    str = XmStringCreateLocalized(name);
    XmStringFree(str);
    vp->_header_list->load_view(err, name,
	vp->_views->operator[](new_view_index)->_search_criteria,
	vp->_views->operator[](new_view_index)->_total, new_view_index);
    if (err) {
      vp->setStatus(catgets(DT_catd, 17, 18, "Error loading view"));
    } else {
      vp->setStatus(" ");
    }
    theRoamApp.unbusyAllWindows();
    vp->normalCursor();
}

// The updateNewViewStats routine is used to update the "New" message
// view.  I made this a seperate method because the "New" view is
// kind of a special case and we end up needing to update it from
// several different places in the Views object.
// IMPORTANT: We don't want to count deleted msgs in the stats!
/////////////////////////////////////////////////////////////////////////
void
Views::updateNewViewStats()
{
    int pos = 2;	// 2 is the position of the "New" view
    XmString item;
    SdmError err;
    SdmMessageNumber last = _header_list->get_last_msg_num();
    SdmMessageNumberL msg_list;
    char *str;

    // malloc space for the unread and undeleted string and also
    // for " and " and the null terminator.
    str = (char *)malloc((strlen(SdmSearch::Sdm_STK_Unread) +
			 strlen(SdmSearch::Sdm_STK_Undeleted) +
			 6) * sizeof(char));
			 
    sprintf(str, "%s and %s", SdmSearch::Sdm_STK_Unread,
	    SdmSearch::Sdm_STK_Undeleted);

    SdmString ss(str);
    free(str);
    str = NULL;

    _search_obj->SetSearchString(err, (const SdmString)ss);
    if (err) return;

    _mbox->PerformSearch(err, msg_list, *_search_obj, 1, last);
    if (err) return;

    (*_views)[NEW_VIEW]->_total = msg_list.ElementCount();
    (*_views)[NEW_VIEW]->_unread = msg_list.ElementCount();

    str = formatViewInfo((*_views)[NEW_VIEW]);
    item = XmStringCreateLocalized(str);
    XmListReplacePositions(_list_w[0], &pos, &item, 1);
    free(str);
}

/////////////////////////////////////////////////////////////////////////
void
Views::updateView(int index)
{
    int pos, listi;
    char *str;
    XmString item;
    SdmString *ss;
    SdmError err;
    SdmMessageNumber last = _header_list->get_last_msg_num();
    SdmMessageNumberL msg_list;
    
    ss = (*_views)[index]->_search_criteria;
    _search_obj->SetSearchString(err, (const SdmString)*ss);
    if (err) return;

    _mbox->PerformSearch(err, msg_list, *_search_obj, 1, last);
    if (err) return;

    (*_views)[index]->_total = getMessageCount(msg_list);
    (*_views)[index]->_unread = getUnreadCount(msg_list);

    str = formatViewInfo((*_views)[index]);
    item = XmStringCreateLocalized(str);
    if (index < 2) {
      listi = 0;
      pos = index + 1;	// add one because index starts at 0 and pos starts at 1
    } else {
      listi = 1;
      pos = index - 1;	// this is really (index - 2 + 1). sub 2 to skip over
                        // default views.
    }
    XmListReplacePositions(_list_w[listi], &pos, &item, 1);
    free(str);
}

/////////////////////////////////////////////////////////////////////////
void Views::selectionCallback(Widget w,
			      XtPointer client_data,
			      XtPointer call_data)
{
    Views *vp = (Views *)client_data;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *)call_data;

    // We only want one item to be selected between the two lists.
    // This means selecting an item in either list should deselect
    // any selected item in the other list.
    // list 0 is for the default views and list 1 is for user defined
    // views.
    if (w == vp->_list_w[0]) {
	if (vp->_list_selected == 1) {
	    //
	    // An item has been selected in the default view list.
	    // Deselect the selected item in the custom list.
	    //
	    XmListDeselectAllItems(vp->_list_w[1]);
	    vp->_list_selected = 0;
	    XtVaSetValues(vp->_delete_w,
			  XmNsensitive, False,
			  NULL);
	    XtVaSetValues(vp->_move_up_w,
			  XmNsensitive, False,
			  NULL);
	    XtVaSetValues(vp->_move_down_w,
			  XmNsensitive, False,
			  NULL);
	}
	vp->setStatus(" ");
    } else {
	if (vp->_list_selected == 0) {
	    //
	    // An item has been selected in the custom view list.
	    // Deselect the selected item in the default list.
	    //
	    XmListDeselectAllItems(vp->_list_w[0]);
	    vp->_list_selected = 1;
	    XtVaSetValues(vp->_delete_w,
			  XmNsensitive, True,
			  NULL);
	    XtVaSetValues(vp->_move_up_w,
			  XmNsensitive, True,
			  NULL);
	    XtVaSetValues(vp->_move_down_w,
			  XmNsensitive, True,
			  NULL);
	}
	vp->setStatus(" ");
    }
}

//
// This routine should only be called for items selected in the user
// defined view list (_list_w[1]).
/////////////////////////////////////////////////////////////////////////
// void Views::deleteView()
// {
//     int *p_list, p_count, pos;

//     if (_list_selected == 1) {
// 	if (XmListGetSelectedPos(_list_w[_list_selected], &p_list, &p_count)) {

// 	    XmListDeletePositions(_list_w[_list_selected], p_list, p_count);

// 	    //
// 	    // Subtract one because the _views list starts at 0 and add 2
// 	    // to skip over the first two default items in the list.  I
// 	    // realize I could have simply added 1 to p_list[0] but I wanted
// 	    // it to be clear to the casual reader.
// 	    //
// 	    _views->RemoveElement(p_list[0]-1+2);

// 	    //
// 	    // Update the mailrc with the new list of views.  
// 	    //
// 	    updateMailrc();

// 	    int custom_view_count = _views->ElementCount() - 2;
// 	    if (custom_view_count < 1) {
// 		XtVaSetValues(_delete_w,
// 			      XmNsensitive, False,
// 			      NULL);
// 		free(p_list);
// 		return;
// 	    }
		
// 	    if (p_list[0] > custom_view_count)
// 		pos = custom_view_count;
// 	    else
// 		pos = p_list[0];
// 	    XmListSelectPos(_list_w[_list_selected], pos, True);
// 	    free(p_list);
// 	}
//     }
// }

/////////////////////////////////////////////////////////////////////////
void Views::updateMailrc()
{
    SdmError error;
    SdmMailRc *mailrc;
    SdmConnection *con = theRoamApp.connection()->connection();
    SdmString view_str("");
    char *tmp_str;
    int i, count;
    char colon = ':';
    char semi_colon=';';

    con->SdmMailRcFactory(error, mailrc);
    if (error) {
      _parent->genDialog()->post_error(error);
      return;
    }

    count = _views->ElementCount();

    // start with the 3rd element in the _views list because we don't need
    // write out the default views to the mailrc.
    //
    for (i=2; i<count; i++) {
	tmp_str = strdup((*_views)[i]->_name);
	if (strstr(tmp_str, ";")) {
	    // addEscapeChar frees tmp_str and returns a new address
	    addEscapeChar(tmp_str, semi_colon);
	}
	if (strstr(tmp_str, ":")) {
	    addEscapeChar(tmp_str, colon);
	}
	view_str += tmp_str;
	view_str += ":";
	// escape any colons or semi-colons in the search criteria
	tmp_str = strdup((const char *)*(*_views)[i]->_search_criteria);
	if (strstr(tmp_str, ":")) {
	    addEscapeChar(tmp_str, colon);
	}
	if (strstr(tmp_str, ";")) {
	    addEscapeChar(tmp_str, semi_colon);
	}
	view_str += (tmp_str);
	if (i < count - 1) {
	    view_str += ";";
	}
	free(tmp_str);
    }

    mailrc->SetValue(error, "views", (const char *)view_str);
    if (error) {
      _parent->genDialog()->post_error(error);
    } else {
      mailrc->Update(error);
      if (error)
        _parent->genDialog()->post_error(error);
    }
}

// addEscapeChar() escapes all occurances of char_to_escape in string s.
// It frees the string passed in and allocates space for the newly escaped
// string and assigns the new address to s.
/////////////////////////////////////////////////////////////////////////
void Views::addEscapeChar(char *&s, char char_to_escape)
{
    int i, j=0, len, count=0;
    char *tmp_str;

    len = strlen(s);
    // Count the number of characters we need to escape.
    for (i=0; i<len; i++) {
	if (s[i] == char_to_escape) {
	    count++;
	}
    }
    tmp_str = (char *)malloc(sizeof(char)*(len+count+1));
    for (i=0; i<len; i++) {
	if (s[i] == char_to_escape) {
	    tmp_str[j++] = '\\';
	    tmp_str[j++] = s[i];
	} else {
	    tmp_str[j++] = s[i];
	}
    }
    tmp_str[j] = '\0';
    free(s);
    s = tmp_str;
}    

// stripEscapeChar allocates space for a new string with the escape
// characters striped out of it.  It does nothing to the incoming string.
/////////////////////////////////////////////////////////////////////////
char *Views::stripEscapeChar(char *str)
{
    int i, j=0, len;
    char *tmp;

    len = strlen(str);
    // We malloc len+1 bytes because it is possible that the string
    // does not have any escape characters in it.  note we only
    // escape the colon or semicolon.
    tmp = (char *)malloc((sizeof(char)*len)+1);
    for (i=0; i<len; i++) {
	if (!(str[i] == '\\' && i+1<len && (str[i+1] == ':' || str[i+1] == ';'))) {
	    tmp[j++] = str[i];
	}
    }
    tmp[j] = '\0';
    return(tmp);
}

/////////////////////////////////////////////////////////////////////////
void Views::deleteCallback(Widget, XtPointer client_data, XtPointer)
{
    Views *vp = (Views *)client_data;
    int *p_list, p_count, pos;

//     view->deleteView();
    if (vp->_list_selected == 1) {
	if (XmListGetSelectedPos(vp->_list_w[vp->_list_selected], &p_list, &p_count)) {
	    XmListDeletePositions(vp->_list_w[vp->_list_selected], p_list, p_count);
	    //
	    // Subtract one because the _views list starts at 0 and add 2
	    // to skip over the first two default items in the list.  I
	    // realize I could have simply added 1 to p_list[0] but I wanted
	    // it to be clear to the casual reader.
	    //
	    vp->_views->RemoveElement(p_list[0]-1+2);
	    //
	    // Update the mailrc with the new list of views.  
	    //
	    vp->updateMailrc();

	    int custom_view_count = vp->_views->ElementCount() - 2;
	    if (custom_view_count < 1) {
		XtVaSetValues(vp->_delete_w,
			      XmNsensitive, False,
			      NULL);
		free(p_list);
		return;
	    }
		
	    if (p_list[0] > custom_view_count)
		pos = custom_view_count;
	    else
		pos = p_list[0];
	    XmListSelectPos(vp->_list_w[vp->_list_selected], pos, True);
	    free(p_list);
	}
    }
}

//
// This routine returns the search string associated with the item
// in the views scrolling list that is currently selected.  It also
// returns the name of the view.  Callers of this function should *NOT*
// free the memory associated with the returned view name.
/////////////////////////////////////////////////////////////////////////
void Views::searchCriteriaOfSelectedView(char *&r_name,
					 SdmString *&r_ss)
{
    int *p_list, p_count, offset = 0;

    if (!XmListGetSelectedPos(_list_w[_list_selected], &p_list, &p_count))
	return;
    else {
	if (_list_selected == 1) {	// user defined views
	    offset = 2;
	}
	r_ss = (*_views)[p_list[0] - 1 + offset]->_search_criteria;
	r_name = (*_views)[p_list[0] - 1 + offset]->_name;
	free(p_list);
    }
}

/////////////////////////////////////////////////////////////////////////
void Views::editCallback(Widget, XtPointer client_data, XtPointer)
{
    Views *vp = (Views *)client_data;
    SdmString *ss;
    char *name;
    int *p_list, p_count;

    if (!XmListGetSelectedPos(vp->_list_w[vp->_list_selected], &p_list,
			      &p_count)) {
	vp->setStatus(catgets(DT_catd, 17, 17, "No view selected"));
	return;
    } else {
	free(p_list);
    }

    if (vp->_find_dialog == NULL) {
	vp->_find_dialog = vp->parent()->get_find_dialog();
	vp->_find_dialog->clearFields();
	vp->searchCriteriaOfSelectedView(name, ss);
	if (vp->_list_selected == 0) {
	    name = "";
	}
	if (ss) {
	    vp->_find_dialog->loadSearchCriteria(name, ss);
	    vp->setStatus(" ");
	} else {
	    vp->setStatus(catgets(DT_catd, 17, 23, "Error editing view"));
	}
    } else {
	// If we don't call manage() before popup() the find dialog will
	// come up empty if it was dismissed via the window manager close
	vp->_find_dialog->clearFields();
	vp->_find_dialog->manage();
	vp->_find_dialog->popup();
	vp->searchCriteriaOfSelectedView(name, ss);
	if (vp->_list_selected == 0) {
	    name = "";
	}
	if (ss) {
	    vp->_find_dialog->loadSearchCriteria(name, ss);
	    vp->setStatus(" ");
	} else {
	    vp->setStatus(catgets(DT_catd, 17, 23, "Error editing view"));
	}
    }
}


/////////////////////////////////////////////////////////////////////////
void Views::updateCallback(Widget, XtPointer client_data, XtPointer)
{
    SdmError error;
    Views *vd = (Views *)client_data;
    SdmMessageNumber last = vd->_header_list->get_last_msg_num();
    SdmMessageNumberL emptyList;

    theRoamApp.busyAllWindows();
    vd->busyCursor();
    vd->updateViewsWindow(error,
			  Sdm_True,
			  Sdm_False,
			  Range,
			  1, last,
			  emptyList,
			  Sdm_True);
    
    theRoamApp.unbusyAllWindows();
    vd->normalCursor();

    if (error) {
    	vd->setStatus(catgets(DT_catd, 17, 25, "Error updating statistics."));
    }
}

/////////////////////////////////////////////////////////////////////////
void Views::closeCallback(Widget, XtPointer client_data, XtPointer)
{
    Views *vd = (Views *)client_data;

    vd->popdown();
}
