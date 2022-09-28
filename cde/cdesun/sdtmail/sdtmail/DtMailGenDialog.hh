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

//////////////////////////////////////////////////////////
// DtMailGenDialog.hh: A generic dialog based on MessageBox
//////////////////////////////////////////////////////////
#ifndef DTMAILGENDIALOG_HH
#define DTMAILGENDIALOG_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "15 Mar 1995 @(#)DtMailGenDialog.hh	1.11"
#endif

#include "UIComponent.h"
#include "DtMailDialogCallbackData.hh"
#include "DtMailHelp.hh"

class SdmError;

class DtMailGenDialog : public UIComponent {
    
  private:
    int		_info_dialog;
    Widget	_otherWidget;

    void cleanup ( Widget, DtMailDialogCallbackData* );

  protected:

    void forceUpdate( Widget );
    
    static void okCallback ( Widget, 
			     XtPointer, 
			    XtPointer );
    
    static void cancelCallback ( Widget, 
				XtPointer, 
				XtPointer );

    
    static void helpCallback ( Widget, 
			      XtPointer, 
			      XtPointer );

    static void otherCallback ( Widget, 
			      XtPointer, 
			      XtPointer );



    void setDialog(char * title, char * text, unsigned char type);

  public:
    
    DtMailGenDialog ( char *, Widget, 
      int dialogStyle = XmDIALOG_FULL_APPLICATION_MODAL);

    
    void	setToQuestionDialog(char *, 
				    char *);
#ifdef DEAD_WOOD
    void	setToInfoDialog(char *,
				char *);
#endif /* DEAD_WOOD */
    void	setToErrorDialog(char *, 
				 char *);
    void	setToWarningDialog(char *, 
				   char *);

    void	setToAboutDialog(void);


    virtual Widget post (
			 void *clientData      = NULL,
			 DialogCallback ok     = NULL,
			 DialogCallback cancel = NULL,
			 DialogCallback other  = NULL,
			 DialogCallback help   = NULL,
			 char *helpId          = NULL);

    virtual Widget post(void *clientData,
                  char *okLabelString,
                  char *cancelLabelString,
                  char *helpId,
                  DialogCallback okCallBack,
                  DialogCallback cancelCallBack);

    virtual int	   post_and_return(char *);

    virtual int	   post_and_return(
				char *,
				char *);

    virtual int	   post_and_return(
				char *, 
				char *,
				char *);
				
    virtual int	   post_and_return(
				char *, 
				char *,
				char *,
				char *);
				
    virtual void post_error(SdmError &error,
			    const char* helpId = DTMAILHELPERROR,
			    const char* errorMessageText = NULL);

};
#endif
