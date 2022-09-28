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
#pragma ident "@(#)FindDialog.h	1.30 11/19/96"
#endif

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Search.hh>
#include <SDtMail/String.hh>
#include "Dialog.h"

class Views;

//
// This class is used for the 'Mailer - Find' dialog box.
//
class FindDialog : public Dialog {

public:
  FindDialog(RoamMenuWindow	* parent);
  ~FindDialog();

  //
  // These come from 'Dialog'.
  //
  void		popped_up() { };
  void		popped_down() { };
  void		popup() { XtPopup(_w, XtGrabNone); };
  void		popdown() { XtPopdown(_w); };
  void		widgetDestroyed();

  // The set/clear status methods will set and clear the status line.
  //
  void setStatus(const char * str);
  void clearStatus(void);

  Widget	createWorkArea(Widget);

  SdmBoolean scriptIsHidden() { return _script_is_hidden;}
  void loadSearchCriteria(char *name, SdmString *);
  void clearFields();

private:

  SdmBoolean predicateIsSimple(SdmSearchLeaf *);
  void loadFields();
  void clearValues();

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
//   Boolean	compareMessage(SdmMessageNumber);
//   Boolean	compareHeader(SdmError &, Sdm &, const char	*);

  static void	findCallback ( Widget, XtPointer, XtPointer );
  static void	findPrevCallback ( Widget, XtPointer, XtPointer );
  static void	findSelectAllCallback ( Widget, XtPointer, XtPointer );
  static void	clearCallback ( Widget, XtPointer, XtPointer );
  static void	closeCallback ( Widget, XtPointer, XtPointer );
  static void	showScriptCallback ( Widget, XtPointer, XtPointer );
  static void	showViewsCallback ( Widget, XtPointer, XtPointer );
  static void	saveAsViewCallback ( Widget, XtPointer, XtPointer );
  static void	storeViewNameCallback ( Widget, XtPointer, XtPointer );
  static void	helpCallback ( Widget, XtPointer, XtPointer );
  static void	textFieldCallback ( Widget, XtPointer, XtPointer );
  static void	optionMenuCallback ( Widget, XtPointer, XtPointer );
  static void	dateOptMenuCallback ( Widget, XtPointer, XtPointer );
  static void	headerTokenCallback ( Widget, XtPointer, XtPointer );
  static void	dateTokenCallback ( Widget, XtPointer, XtPointer );
  static void	stateTokenCallback ( Widget, XtPointer, XtPointer );
  static void	operatorTokenCallback ( Widget, XtPointer, XtPointer );

  SdmBoolean getAllFields();
  void addView(char *);

  const int	   _num_text_fields;    	// Array size.

  SdmBoolean	_script_is_hidden;

  SdmStringL	*_headers;
  SdmStringL	*_dates;
  SdmStringL	*_states;
  SdmStringL	*_operators;

  XmString	_basic_str;
  XmString	_advanced_str;
  Widget	_search_type;
  Widget	_save_as_view_dialog;
  Widget	_simple_form;
  Widget	_script_form;
  Widget	_script_area;
  Widget	_status_text;
  Widget	* _text_fields;
  char		** _text_names;
//   char		** _text_abstract_name;
  char		** _text_labels;
  char		** _text_values;
  char		*_view_name;

  unsigned int	   _num_buttons; 	   	// Array size.
  ActionAreaItem * _buttonData;

  SdmSearch     *_srch_crit;
  Boolean	_searchForward;
  RoamMenuWindow * _roamWindow;
  Views		*_viewsDialog;
};

#endif
