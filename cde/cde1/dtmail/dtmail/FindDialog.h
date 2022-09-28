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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)FindDialog.h	1.15 04/15/95"
#endif

#include <DtMail/DtMail.hh>
#include "Dialog.h"

//
// This class is used for the 'Mailer - Find' dialog box.
//
class FindDialog : public Dialog {

public:
  FindDialog(RoamMenuWindow	* parent);
  ~FindDialog() { clear(); };

  //
  // These come from 'Dialog'.
  //
  void		popped_up() { };
  void		popped_down() { };
  void		popup() { XtPopup(_w, XtGrabNone); };
  void		popdown() { XtPopdown(_w); };
  void		widgetDestroyed() {};

  // The set/clear status methods will set and clear the status line.
  //
  void setStatus(const char * str);
  void clearStatus(void);

  Widget	createWorkArea(Widget);

  //
  // Like initialize() except returns success status.
  //
  Boolean		startup();

  //
  // Set the search direction for find.
  //
  void setSearchForward(Boolean forward) {	// True == Forward, False = Backward.
    _searchForward = forward;
  };

private:

  struct ActionAreaItem {
    char		* label;	// Button label.
    XtCallbackProc	  callback;
    caddr_t		  data;
  };
    
  //
  // Clear out the data. After this function is complete the
  // data should look as if the constructor was just called and
  // before initialize().
  //
  void		clear();

  Boolean	findMatching(Boolean findAll = False);

  Boolean	compareMessage(DtMailMessageHandle	  handle);

  Boolean	compareHeader(DtMailEnv		& error,
		      DtMailValueSeq	& seq,
		      const char	* cmpToString);

  static void	directionCallback(Widget	button,
				  XtPointer	closure,
				  XtPointer	call_data);

  static void	findCallback(Widget	button,
			     XtPointer	closure,
			     XtPointer	call_data);

  static void	findSelectAllCallback(Widget	button,
				      XtPointer	closure,
				      XtPointer	call_data);

  static void	clearCallback(Widget	button,
			      XtPointer	closure,
			      XtPointer	call_data);

  static void	closeCallback(Widget	button,
			      XtPointer	closure,
			      XtPointer	call_data);

  static void	helpCallback(Widget	button,
			     XtPointer	closure,
			     XtPointer	call_data);

  static void	textFieldCallback(Widget, XtPointer, XtPointer);

  //
  // Pull all fields out of the dialog and store in _text_values;
  //
  void		getAllFields();

  unsigned int	   _num_text_fields;    	// Array size.

  Widget	   _status_text;
  Widget	 * _text_fields;
  char		** _text_names;
  char		** _text_abstract_name;
  char		** _text_labels;
  char		** _text_values;

  unsigned int	   _num_buttons; 	   	// Array size.
  ActionAreaItem * _buttonData;

  Boolean	   _searchForward;
  RoamMenuWindow * _roamWindow;
};

#endif
