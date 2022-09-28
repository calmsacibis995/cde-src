/*
 *+SNOTICE
 *
 *      $Revision: 1.1 $
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
 *+ENOTICE
 */

#ifndef _UNDELETE_HH
#define _UNDELETE_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Undelete.hh	1.31 05/16/97"
#endif

#include <SDtMail/Sdtmail.hh>
#include "DialogShell.h"
#include "MsgScrollingList.hh"
#include "CmdList.h"

/* UNDEL_*      msgid 600 - 699
 */
#define UNDEL_SENDER  600
#define UNDEL_SUB     601
#define UNDEL_DATE    602
#define UNDEL_SIZE    603
#define UNDEL_UNDEL   604
#define UNDEL_CLOSE   605
#define UNDEL_EXIT    606
#define UNDEL_OKBUT   607
#define UNDEL_DLGTTL  608

class RoamMenuWindow;

class UndelMsgScrollingList : public MsgScrollingList {
  public:
    UndelMsgScrollingList ( RoamMenuWindow *, Widget, char * );
    void extended_selection(SdmError &, int);
    void insertMsg(SdmMessageNumber);
    void insertMsg(SdmError &, MsgStruct *);
    void loadMsgs(SdmError &, MsgHndArray *, int);

    void deleteSelected(SdmBoolean);

    virtual ~UndelMsgScrollingList ();
  protected:
};


class UndelFromListDialog : public DialogShell {
  public:
    UndelFromListDialog (char *, RoamMenuWindow *);
    ~UndelFromListDialog();
    Widget  createWorkArea(Widget);
    virtual void initialize();
    virtual void quit();


	// Accessors
	// Mutators
    void popped_down();
    void popped_up();
    void insertMsg(SdmError &, MsgStruct *);
    void loadMsgs(SdmError &, MsgHndArray *, int);
    void updateListItems (SdmError &, int current);
    void undelSelected();
    void undelLast();
    void addToRowOfButtons();
    void checkDisplayProp(void);

    void expunge(void);

    void replaceItems(XmString * items, int num_items) {
      if (_list) {
          XmListReplaceItemsPos(_list->get_scrolling_list(), items, num_items, 1);
      }
    }

  protected:
    Cmd *_undelete_button;
    Cmd *_close_button;
  private:
    UndelMsgScrollingList *_list;
    Widget *_undel_list_button;
    Widget rowOfLabels, rowOfButtons;
    Widget rowOfMessageStatus;
    RoamMenuWindow *my_owner;

};

#endif
