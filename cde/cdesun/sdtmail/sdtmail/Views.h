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

#ifndef VIEWS_H
#define VIEWS_H

#include "FindDialog.h"
#include "Dialog.h"

class MsgScrollingList;

typedef enum UpdateType { SingleMessage, Range, List };

//
// This class is used for the 'Mailer - Views' dialog box.
//
class Views : public Dialog {

public:
    Views(RoamMenuWindow *);
    ~Views();

    //
    // These come from 'Dialog'.
    //
    void popped_up();
    void popped_down();
    void popup();
    void popdown();
    void widgetDestroyed() {};

    Widget createWorkArea(Widget);

    void popupFindDialog();
    void addView(const char *, const SdmString&);

    void updateViewsWindow(SdmError &error,
			   SdmBoolean regenerate,
			   SdmBoolean unread_only,
			   UpdateType type,
			   SdmMessageNumber start,
			   SdmMessageNumber end,
			   SdmMessageNumberL &msgs,
			   SdmBoolean add_to);
    // The updateNewViewStats method just updates the default view for
    // "new" messages
    void updateNewViewStats();
    void updateView(int index);
    SdmBoolean isMapped() { return _mapped; };

private:
    struct ViewInfo {
	char *_name;
	SdmString *_search_criteria;
	int _unread;
	int _total;
    };
    
    // callbacks for the various widgets in the UI
    //
    static void moveUpCallback(Widget, XtPointer, XtPointer);
    static void moveDownCallback(Widget, XtPointer, XtPointer);
    static void newCallback(Widget, XtPointer, XtPointer);
    static void editCallback(Widget, XtPointer, XtPointer);
    static void deleteCallback(Widget, XtPointer, XtPointer);
    static void updateCallback(Widget, XtPointer, XtPointer);
    static void saveCallback(Widget, XtPointer, XtPointer);
    static void closeCallback(Widget, XtPointer, XtPointer);
    static void selectionCallback(Widget, XtPointer, XtPointer);
    static void loadCallback(Widget, XtPointer, XtPointer);
    static void doubleClickCallback(Widget, XtPointer, XtPointer);

    void addEscapeChar(char *&, char);
    char *stripEscapeChar(char *);
    void searchCriteriaOfSelectedView(char *&, SdmString *&);
    int loadUserDefinedViews();
    char *formatViewInfo(ViewInfo *);
    int getUnreadCount(const SdmMessageNumberL &);
    int getMessageCount(const SdmMessageNumberL &);
    void updateMailrc();
    void setStatus(const char *);
    
    FindDialog *_find_dialog;
    RoamMenuWindow *_parent;
    MsgScrollingList *_header_list;

    // The first two elements of _views are "read-only" in that you
    // can't modify the search criteria or delete it from the list.
    // The user defined views are stored starting at position 2.
    SdmVector<ViewInfo *> *_views;
    SdmSearch *_search_obj;
    SdmMessageStore *_mbox;

    SdmBoolean _mapped;
    int _list_selected;
    int _current_view_index;

    // The first widget in _list_w is for the default views and the other
    // widget is for user defined views
    Widget _list_w[2];
    Widget _delete_w;
    Widget _move_up_w;
    Widget _move_down_w;
    Widget _status_text;
};

#endif
