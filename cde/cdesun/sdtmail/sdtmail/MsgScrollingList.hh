/*
 *+SNOTICE
 *
 *	$Revision: 1.4 $
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

#ifndef MSGSCROLLINGLIST_H
#define MSGSCROLLINGLIST_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MsgScrollingList.hh	1.65 15 Apr 1995"
#endif


#include "UIComponent.h"
#include "ScrollingList.h"
#include "RoamCmds.h"
#include "MsgHndArray.hh"
#include "XtArgCollector.h"
#include "XmStrCollector.h"


//#include "UndeleteMessageFromListDialog.h"

extern "C" {
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
}

#include <Xm/Xm.h>

/* MSGLIST_*       msgid 200 -299
 */
#define MSGLIST_SELMV    200
#define MSGLIST_SELCP    201
#define MSGLIST_NORC     202
#define MSGLIST_NOBOX    203
#define MSGLIST_NOOPEN   204
#define MSGLIST_MSGMV    205
#define MSGLIST_MSGSMV   206
#define MSGLIST_MSGCP    207
#define MSGLIST_MSGSCP   208
#define MSGLIST_NOSEL    209
#define MSGLIST_RAISE    210
#define MSGLIST_EXIT	 211
#define MSGLIST_OKBUT	 212
#define MSGLIST_DLGTTL	 213

typedef enum MSL_Status { Complete, Pending, Destroyed };

class MsgScrollingList : public ScrollingList {
    
  public:
    
    MsgScrollingList ( RoamMenuWindow *, Widget, char * );    
    
    // Accesors
    
    int  selected_item_position() { return _selected_item_position; }
    RoamMenuWindow *parent() { return _parent; }


    void select_next_item();
    void select_prev_item();

    SdmMessageNumber msgno(int index);
    const MsgStruct* get_message_struct(int index);

    SdmMessageNumber current_msg_handle();
    int       position(SdmMessageNumber msgno);

    virtual const char *const className() {return ("MsgScrollingList");}
    virtual void insertMsg(SdmMessageNumber);
    virtual void insertDeletedMsg(SdmMessageNumber);
    virtual void deleteSelected(SdmBoolean silent = Sdm_True);

    int copySelected(SdmError &, char *, char *, int delete_after_copy, int silent);
    int copySelectedMsgs(SdmError &, char *);
    int copyPrintSelected(SdmError &, char *);

    void clearMsgs();

    void updateListItems(SdmError &, int);
    void checkDisplayProp(void);

    XmString formatHeader(SdmIntStrL *retrieved, 
			  int sess_num,
			  int view_msg_num,
			  SdmBoolean multi_part,
			  SdmBoolean new_msg);
			  
    SdmBoolean IsUserMyself(const char* user);

    SdmBoolean IsMultipartMessage(SdmIntStrL *intStrListPtr);

    void visibleItems(int n_vis) {
	XtVaSetValues(_w,
		      XmNvisibleItemCount, n_vis,
		      NULL);
    }

    int visibleItems(void) {
		int n_vis;
		XtVaGetValues(_w,
		      XmNvisibleItemCount, &n_vis,
		      NULL);
		return(n_vis);
	}

    Widget get_scrolling_list();

    void selected_msg_hndls( SdmMessageNumberL &);
    MsgHndArray* selected();
    void	display_message( SdmError &, SdmMessageNumber, int item_pos = 0 );
    void	display_and_select_message(SdmError &,
					   SdmMessageNumber);
    // Select all of the messages in the list and display the last one.
    void	select_all_and_display_last(SdmError &);

    //
    // Select all of the messages in the array[] of SdmMessageNumber.
    // 'array' is NULL terminated list.
    //
    void	select_all_and_display_last(SdmError &,
					    SdmMessageNumberL *array,
					    unsigned int	elements);

    void	display_no_message();
    void 	scroll_to_bottom();
    void	scroll_to_position( int );
    virtual void	extended_selection( SdmError &, int );
    int		get_selected_item();
    int		get_displayed_item();
    SdmMessageNumber get_last_msg_num() { return _last_msg_num; }
    int		get_num_new_messages() { return _num_new_messages; }
    // get_num_deleted_messages returns the delete count for the current view
    int		get_num_deleted_messages() { return _num_deleted_messages; }
    // get_num_total_deleted_messages returns the delete count for the mailbox
    int		get_num_total_deleted_messages() { return _total_deleted; }
    void	expunge(int, SdmMessageNumberL& msgList);
    SdmBoolean	warnAboutDestroy();
    int	 	resetSessionNums(void);
    void 	layoutLabels(int, Widget, Widget, Widget, Widget);
    int		get_num_messages() { return _msgs->length(); }
    SdmMessageNumberL* get_deleted_message_numbers();
    SdmMessageNumberL* get_message_numbers();
    MsgHndArray* get_deleted_messages();  // returns a copy of the deleted msgs list
    MsgHndArray* get_messages();

    void	viewInSeparateWindow(SdmError &);

    void	undelete_messages(SdmError &error, SdmMessageNumberL &);
    void	undelete_last_deleted(SdmError &error);
    void	display_message_summary();

    int		load_headers(SdmError &, SdmMessageNumber nmsgs);
    static Boolean load_headersWP(XtPointer client_data);
    int		load_all_headers(SdmError &, SdmMessageNumber nmsgs);
    void	load_new_headers(SdmError &, SdmMessageNumber nNewMsgs);
    void	load_view(SdmError &, const char *, SdmString *, int c=0, int i=0);
    void	do_list_vis_adjustment();
    void	deleteMe();
    
  protected:
    virtual void defaultAction( Widget, XtPointer, XmListCallbackStruct * );
    static void extendedSelectionCallback( Widget, XtPointer, XmListCallbackStruct * );
    MsgHndArray *_msgs;
    SdmMessageHeaderAbstractFlags	_header_info;
    // The destructor is virtual to force usage of the deleteMe function
    virtual ~MsgScrollingList ();

  private:
    MSL_Status _state;

    void GetUsernames();

    SdmString *_current_ss;		// current view search criteria
    SdmMessageNumber _last_msg_num; // handle of the last message in the mbox
    RoamMenuWindow *_parent;
    int	_selected_item_position;
    int _displayed_item_position;
    int _session_message_number;
    int _date_width;              // width of the date field
    SdmMessageNumberL *_msgNumL;	// list of non-deleted message numbers in the mailbox
    SdmMessageNumber _displayed_msgno;

// TOGO    CMContainer displayed_container;

    SdmBoolean _in_C_locale;

    int  _last_msg_index_loaded;
    int  _first_msg_index_loaded;
    int  _num_new_messages;
    int	 _num_deleted_messages;
    int	 _total_deleted;
    MsgHndArray *_deleted_messages;

    SdmBoolean	_numbered;
    SdmBoolean  _showTo;
    SdmBoolean	_selection_on;
    SdmBoolean  _useAlternates;
    char*       _alternates;
    SdmStringL  _usernames;

    // Collect the arguments to SetValues in load_headers
    // This prevents multiple redisplays of the XmList widget
    XtArgCollector	*_xtarg_collector;

    // Collect the XmString items in load_headers
    XmStrCollector	*_xmstr_collector;

    // Save the XmString that was allocated so that we can
    // free it later.  It would probably be better to add this
    // to the XtArgCollector class, but since it's only one
    // item it's easier to do it this way.
    XmString		_selected_items;
};

#endif
