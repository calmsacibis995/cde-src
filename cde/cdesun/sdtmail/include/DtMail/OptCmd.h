/*
 *+SNOTICE
 *
 *	$Revision: 1.0 $
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// OptCmd.h: Display the Mail Options dialog
////////////////////////////////////////////////////////////
#ifndef OPTCMD_H
#define OPTCMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "%W %G"
#endif

#include <DtMail/PropUi.hh>
#include <DtVirtArray.hh>
#include "DtMailGenDialog.hh"
#include "WarnNoUndoCmd.h"
#include "MainWindow.h"
#include "options_ui.h"
#ifdef AUTOFILING
#include <DtMail/RuleCmd.h>
#endif // AUTOFILING

void vaca_msg_changedCB(Widget w, 
			XtPointer ClientData, 
			XtPointer calldata);
int options_set_category_pane(Widget pane);
void opt_help_button_CB(Widget wid, XtPointer clientData, XtPointer calldata);


class OptCmd: public WarnNoUndoCmd {
    
  protected:

    virtual void doit();      // Call exit
    DtbOptionsDialogInfo _opt_dlog;
#ifdef AUTOFILING
    RuleCmd *_rule;
#endif // AUTOFILING

  public:
#ifdef AUTOFILING
    friend class RuleCmd;
#endif // AUTOFILING
    OptCmd ( char *, char *, int, Widget parent);
    virtual ~OptCmd();
    void create_dlog();
    void unmanage();
    virtual const char *const className () { return "OptCmd"; }
    virtual void execute(); // Overrides the AskFirstCmd member function

    void update_panes(); // sync up ui with hash tables
    void update_pane(Widget current_pane); 
    void update_vacation(); 
    void updateUiFromBackEnd(DtVirtArray<PropUiItem *> *list);

    int update_source();
    void updateBackEndFromUi(DtVirtArray<PropUiItem *> *list);

    void doVacationCheck();
    void setDateChanged();
    void set_vaca_msg_Changed();
    void onOffButtonChanged();
    void startVacation();
    void stopVacation();
    void help_button_CB(Widget pane); 
    void setNewCategoryStr();
    void setOldCategoryStr();
    void setWithCategoryStr(char *str);
    DtMailGenDialog *genDialog() { return generalDialog; }
    void setCurrentPane(Widget pane) { CurrentPane = pane; }
    Widget currentPane() { return CurrentPane; }
#ifdef AUTOFILING
    void handleAutoEdit();
    void manage_rule_dialog();
    void clear_rule_dialog();
    void update_rule_dialog();
#endif
    SdmBoolean _startDateFormatError;
    SdmBoolean _endDateFormatError;
    SdmBoolean windowIsAlive() { return _window_alive; }
    void setWindowAlive(SdmBoolean wa) { _window_alive = wa; }
    DtbOptionsDialogInfo optionsDialog() { return _opt_dlog; }
    void whenICloseMailboxChanged();
    void signatureChanged();
    SdmBoolean signatureFileChanged() { return _signature_changed; };
    void resetSignature() { _signature_changed = Sdm_False; };

  private:
    virtual void init_panes(); // hook up front and back ends

    virtual void init_basic_pane();
    virtual void init_msg_view_pane();
    virtual void init_toolbar_pane();
    virtual void init_compose_pane();
    virtual void init_signature_pane();
    virtual void init_msg_filing_pane();
    virtual void init_vacation_pane();
    virtual void init_template_pane();
    virtual void init_aliases_pane();
    virtual void init_advanced_pane();
    virtual void init_notify_pane();

    DtVirtArray<PropUiItem *> *basic_pane_list;
    DtVirtArray<PropUiItem *> *msg_view_pane_list;
    DtVirtArray<PropUiItem *> *toolbar_pane_list;
    DtVirtArray<PropUiItem *> *compose_pane_list;
    DtVirtArray<PropUiItem *> *signature_pane_list;
    DtVirtArray<PropUiItem *> *msg_filing_pane_list;
    DtVirtArray<PropUiItem *> *vacation_pane_list;
    DtVirtArray<PropUiItem *> *template_pane_list;
    DtVirtArray<PropUiItem *> *aliases_pane_list;
    DtVirtArray<PropUiItem *> *advanced_pane_list;
    DtVirtArray<PropUiItem *> *notify_pane_list;	

    SdmBoolean vacation_msg_changed;
    SdmBoolean vacation_date_changed;
    SdmBoolean vacation_onoff_changed;
    SdmBoolean _window_alive;

    XmString vaca_sub;
    XmString vaca_msg;
    XmString catstr;
    DtMailGenDialog *generalDialog;
    Widget CurrentPane;

    static void quitCB( Widget, XtPointer, XmAnyCallbackStruct * );

    int imapinboxserver_changed();
    void log_text_changed();
    void restartInbox(int restart);
    void popupWarning();

    SdmBoolean _signature_changed;
};
#endif
