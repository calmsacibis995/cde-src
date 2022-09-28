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

#ifndef SEARCHCMD_H
#define SEARCHCMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RoamCmds.h	1.104 04/25/96"
#else
static char *sccs__SEARCHCMD_H = "@(#)RoamCmds.h	1.104 04/25/96";
#endif

#include <Dt/Action.h>

#include <DtMail/DtMail.hh>
#include <DtMail/DtVirtArray.hh>

#include "Cmd.h"
#include "NoUndoCmd.h"
#include "InterruptibleCmd.h"
#include "RoamInterruptibleCmd.hh"
#include "SelectFileCmd.h"
#include "DtMailWDM.hh"
#include "Editor.hh"
#include "DtMailGenDialog.hh"
#include "MotifCmds.h"
#include "TemplateDialog.h"
#include "DraftDialog.h"

#include <assert.h>

/* ROCMD_*      msgid 300 - 399
 */
#define ROCMD_EMPTY   300
#define ROCMD_MV      301

class SendMsgDialog;
class PopupWindow;
class RoamMenuWindow;
class ViewMsgDialog;
class UndelFromListDialog;

//class VacationPopup;

class RoamCmd : public NoUndoCmd {
  protected:
    
    RoamMenuWindow *_menuwindow;
    
  public:
    //  virtual void doit();   
    RoamCmd( char *, char *, int, RoamMenuWindow * );
    //  virtual const char *const className () { return "SaveCmd"; }
};

class OpenMsgCmd : public RoamCmd {
public:
  virtual void doit();
  OpenMsgCmd(char *, char *, int, RoamMenuWindow *);
  virtual const char *const className() { return "OpenMsgCmd"; }
};

#ifdef DEAD_WOOD
class SaveCmd : public RoamCmd {
  public:
    virtual void doit();   
    SaveCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "SaveCmd"; }
};
#endif /* DEAD_WOOD */


class MoveCopyCmd : public SelectFileCmd {
  private:

    static void fileSelectedCallback2 ( Widget, XtPointer, XtPointer );
    Boolean _copy_is_default;

  protected:

    FileCallback _copy_callback;
    RoamMenuWindow * _menuwindow;

  public:

    virtual void doit();
    MoveCopyCmd( char *, char *, Boolean, int, FileCallback, FileCallback, RoamMenuWindow *, Widget );
    ~MoveCopyCmd ();
    virtual const char *const className () { return "MoveCopyCmd"; }
    virtual void fileSelected2 ( char * );
};
 

class NextCmd : public RoamCmd {
  public:
    virtual void doit();   
    NextCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "NextCmd"; }
};


class PrevCmd : public RoamCmd {
  public:
    virtual void doit();   
    PrevCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "PrevCmd"; }
};

class ChooseCmd : public NoUndoCmd {
  protected:
    
    class RoamMenuWindow *_menuwindow;
    DtMailMessageHandle  _msgno;
    
    
  public:
    virtual void doit();   
    ChooseCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "ChooseCmd"; }
    DtMailMessageHandle    msgno() { return _msgno; }
    void    msgno( DtMailMessageHandle msgno) { _msgno=msgno; }
};

class CheckForNewMailCmd : public NoUndoCmd {
  protected:
    
    class RoamMenuWindow *_menuwindow;

  public:
    virtual void doit();   
    CheckForNewMailCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "CheckForNewMailCmd"; }
};


// Used for opening a container.

class OpenContainerCmd : public RoamInterruptibleCmd {

  protected:
    RoamMenuWindow	*_menuWindow;
    
    virtual	void	doit();
    virtual	void	undoit();
    virtual	void	post_dialog();
    virtual	void	unpost_dialog();
    virtual void check_if_done();
    
  public:

    // Need this to be public coz RMW's callback function needs it.
    // Its a boolean_t because that's what the back-end uses and
    // passes.  The back-end doesn't want to know anything about 
    // Boolean which is a Motif/Xt concept.  Instead of casting to
    // to and from Boolean<->boolean_t, might as well set it to be 
    // boolean_t and avoid the casting mess.  No?

    DtMailBoolean		_open_create_flag;
    DtMailBoolean		_open_lock_flag;

    OpenContainerCmd(char *, char *, int, RoamMenuWindow *);
    virtual void execute();
    virtual void execute(RoamTaskDoneCallback, void *);
    void   set_create_lock_flags(DtMailBoolean, DtMailBoolean);
    virtual void updateMessage ( char * );
    virtual const char *const className () { return "ShowStatusCmd"; }
};
class OpenMailboxCmd : public RoamCmd {
  private:

    char *_container_name;
	
  protected:
    virtual	void	doit();

  public:
    OpenMailboxCmd(char*, char *, int, RoamMenuWindow *, char *);
    virtual const char *const className() { return "OpenMailboxCmd"; }
    
};

// Used for converting containers from rfc<->???.

typedef int (*ConversionStatusCB) (int, int, void *);

#ifdef DEAD_WOOD

class ConvertContainerCmd : public RoamInterruptibleCmd {
    
  protected:
    
    RoamMenuWindow      *_menuWindow;
    DtMailWDM		*_dialog;
    char 		*_criteria;
    ConversionStatusCB  _conv_cb;
    char		*_src;
    char		*_dest;
    int			_num_converted;
    int			_num_to_be_converted;

    virtual void doit();   
    virtual void undoit(); 
    virtual	void	post_dialog();
    virtual	void	unpost_dialog();
    virtual void check_if_done();

  public:
    ConvertContainerCmd ( char *, char *, int, RoamMenuWindow *);
    virtual void execute();    
    virtual void execute(RoamTaskDoneCallback, void *);
    
    virtual void updateDialog ( char * );
    virtual void updateAnimation();
    virtual const char *const className () { return "ConvertContainerCmd"; }

    void	set_data(char *, char *, ConversionStatusCB);
    void	set_convert_data(int, int);
    int		get_num_converted();
    char *	get_destination_name();
};

#endif // DEAD_WOOD


// Other commands

class SelectAllCmd : public Cmd {
  protected:
 
    class RoamMenuWindow *_menuwindow;
 
  public:
    virtual void doit();
    virtual void undoit();
 
    SelectAllCmd(char *, char *, int, RoamMenuWindow * );
    virtual const char *const className() { return "SelectAllCmd"; }
};  

class DeleteCmd : public Cmd {
  protected:
    
    class RoamMenuWindow *_menuwindow;
    DtMailMessageHandle  _msgno;
    
    
  public:
    virtual void doit();   
    virtual void undoit();   
    DeleteCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "DeleteCmd"; }
    DtMailMessageHandle    msgno() { return _msgno; }
    void    msgno( DtMailMessageHandle msgno) { _msgno=msgno; }
};

class DestroyCmd : public Cmd {
    RoamMenuWindow	*_menuwindow;

  public:
    virtual void doit();
    virtual void undoit();

    DestroyCmd(char *, char *, int active, RoamMenuWindow *);
    virtual const char *const className() { return "DestroyCmd"; }
};

typedef enum { DTM_MOVE, DTM_COPY, DTM_OPEN } MoveCopyOpenOp;

class MoveCopyMenuCmd : public RoamCmd {
  protected:
    char *_container_name;
    MoveCopyOpenOp	_operation;

  public:
    virtual void doit();
    MoveCopyMenuCmd( char *, char *, int, RoamMenuWindow *, MoveCopyOpenOp);
    virtual ~MoveCopyMenuCmd();
    char *containerName(void) { return(_container_name); }
    void changeContainer( char * container_name ) 
				{ _container_name = container_name; }
    virtual const char *const className() { return "MoveCopyMenuCmd"; }
};

class MoveToInboxCmd : public RoamCmd {
  public:
    virtual void doit();
    MoveToInboxCmd( char *, char *, int, RoamMenuWindow * );
    virtual ~MoveToInboxCmd();
    virtual const char *const className () { return "MoveToInboxCmd"; }
};

class CopyToInboxCmd : public RoamCmd {
  public:
    virtual void doit();
    CopyToInboxCmd( char *, char *, int, RoamMenuWindow * );
    virtual ~CopyToInboxCmd();
    virtual const char *const className () { return "CopyToInboxCmd"; }
};


class DoUndeleteCmd : public Cmd {
  protected:
    class UndelFromListDialog *_undelDialog;
  public:
    virtual void doit();
    virtual void undoit();
    DoUndeleteCmd(char *, char *, int, UndelFromListDialog * );
    virtual ~DoUndeleteCmd();
    virtual const char *const className () { return "DoUndeleteCmd";}
};

class CloseUndelCmd : public Cmd {
  protected:
    class UndelFromListDialog *_undelDialog;
  public:
    virtual void doit();
    virtual void undoit();
    CloseUndelCmd(char *, char *, int, UndelFromListDialog *);
    virtual ~CloseUndelCmd();
    virtual const char *const className () { return "CloseUndelCmd";}
};


class UndeleteCmd : public ChooseCmd {
    
  public:
    virtual void doit();   
    UndeleteCmd( char *, char *, int, RoamMenuWindow *, Boolean );
    ~UndeleteCmd();
    UndelFromListDialog *dialog() { return _undelFromList; }
    virtual const char *const className () { return "UndeleteCmd"; }
  protected:
    void	*_clientData; // Data provided by caller
    UndelFromListDialog *_undelFromList;
  private:
    int _num_deleted;	// number of deleted messages
    Boolean _fromList;
};


class PrintCmd : public ChooseCmd {
  public:
    virtual void doit();   
    PrintCmd( char *, char *, int, int silent, RoamMenuWindow * );
    virtual const char *const className () { return "PrintCmd"; }
    void printit(int);
  protected:
    RoamMenuWindow *_parent;
  private:
    struct tmp_file {
	char			*file;
	DtActionInvocationID	id;
    };
    DtVirtArray<struct tmp_file *>	_tmp_files;
    int					_silent;

    int  _register_tmp_file(const char *, DtActionInvocationID);
    void _unregister_tmp_file(DtActionInvocationID);
    static void actioncb(DtActionInvocationID,XtPointer,DtActionArg *,int,int);
}; 

#ifdef DEAD_WOOD
class PopupCmd : public NoUndoCmd {
  private:
    RoamMenuWindow *parent;
    PopupWindow *(RoamMenuWindow::* pmpopup) ( void );
  public:
    virtual void doit();   
    PopupCmd( char *, char *, int, PopupWindow * (RoamMenuWindow::*) (void), RoamMenuWindow * );
    virtual const char *const className () { return "PopupCmd"; }
};
#endif /* DEAD_WOOD */

class OnItemCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    OnItemCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "OnItemCmd"; }
};

class OnAppCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    OnAppCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "OnAppCmd"; }
};

class TasksCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    TasksCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "TasksCmd"; }
};

class ReferenceCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    ReferenceCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "ReferenceCmd"; }
};

class MessagesCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    MessagesCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "MessagesCmd"; }
};

class UsingHelpCmd : public NoUndoCmd {
  private:
    UIComponent *_parent;
  public:
    virtual void doit();
    UsingHelpCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "UsingHelpCmd"; }
};

class RelNoteCmd : public NoUndoCmd {
  private:
    DtMailGenDialog	*_genDialog;
    UIComponent		*_parent;

  public:
    virtual void doit();
    RelNoteCmd( char *, char *, int, UIComponent * );
    virtual const char *const className () { return "RelNoteCmd"; }
    ~RelNoteCmd();
};

class FindCmd : public RoamCmd {
  public:
    virtual void doit();   
    FindCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "FindCmd"; }
};

typedef enum { OPT_BASIC, OPT_TB, OPT_MV, OPT_MF, OPT_NO, OPT_CW, OPT_SIG, OPT_VAC, OPT_TD, 
	OPT_AL, OPT_AD } OptionsOp;

class OptionsCmd : public NoUndoCmd {
  private:
    OptionsOp _op;
  public:
    virtual void doit();   
    OptionsCmd( char *, char *, int, OptionsOp );
    virtual const char *const className () { return "OptionsCmd"; }
};

class SendCmd : public NoUndoCmd {
  private:
    SendMsgDialog	*_parent;
    int _default_trans;
  public:
    virtual void doit();
    SendCmd( char *, char *, int, SendMsgDialog *, int );
    virtual const char *const className () { return "SendCmd"; }
};

class SaveAsTextCmd : public SelectFileCmd {
  private:
    Editor *	_text_editor;
    SendMsgDialog *	_smd;

    static void fileCB(void * client_data, char * selection);
    void saveText(const char * filename);

  public:
    virtual void doit();
    SaveAsTextCmd(char *, char *, char * title, int, Editor *, SendMsgDialog *, Widget);
    virtual const char *const className() { return "SaveAsTextCmd"; }
};

class SaveAsDraftCmd : public NoUndoCmd {
  private:
    SendMsgDialog *_sendMsgDialog;
    Widget _parent;
    void saveDraft();
    DraftModuleDialogInfo _draftDialog;

  public:
    virtual void doit();
    SaveAsDraftCmd(char *, char *, int, SendMsgDialog *, Widget );
    virtual const char *const className() { return "SaveAsDraftCmd"; }
    void getNameAndSave();
    void quit();
};
class SaveAsTemplateCmd : public NoUndoCmd {
  private:
    Editor *	_text_editor;
    DtbModuleDialogInfo _templateDialog;
    SendMsgDialog *_sendMsgDialog;
    Widget _parent;
    void saveTemplate();

  public:
    virtual void doit();
    SaveAsTemplateCmd(char *, char *, int, Editor *, SendMsgDialog *, Widget);
    virtual const char *const className() { return "SaveAsTemplateCmd"; }
    void getNameAndSave();
    void quit();
};
class SaveAttachCmd : public SelectFileCmd {
  private:
    AbstractEditorParent *_parent;
    XmString _name;
    static void updateCallback( Widget, XtPointer, XtPointer);

  public:
    virtual void doit();   
    SaveAttachCmd(char *, char *, char *, int, FileCallback, RoamMenuWindow *, Widget);
    SaveAttachCmd(char *, char *, char *, int, FileCallback, ViewMsgDialog *, Widget);
    SaveAttachCmd(char *, char *, char *, int, FileCallback, SendMsgDialog *, Widget);
    ~SaveAttachCmd();
    virtual const char *const className () { return "SaveAttachCmd"; }
};

class DeleteAttachCmd : public Cmd {
  private:
    SendMsgDialog *_parent;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    DeleteAttachCmd ( char *, char *, int, SendMsgDialog *);

    virtual const char *const className () { return "AttachDeleteCmd"; }
};

class UndeleteAttachCmd : public Cmd {
  private:
    SendMsgDialog *_parent;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    UndeleteAttachCmd ( char *, char *, int, SendMsgDialog *);

    virtual const char *const className () { return "AttachUndeleteCmd"; }
};

class RenameAttachCmd : public Cmd {
    
private:

    SendMsgDialog *_parent;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    static void cancelCallback( Widget, XtPointer, XtPointer );
    void ok( XtPointer );	// Called when the OK button is pressed
    static void okCallback( Widget, XtPointer, XtPointer );

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    RenameAttachCmd ( char *, char *, int, SendMsgDialog * );

    virtual const char *const className () { return "RenameAttachCmd"; }
};

class AttachmentActionCmd : public Cmd {

  private:
	AbstractEditorParent  *_parent;
	int		_index;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachmentActionCmd( char *, char *, RoamMenuWindow *, int);
    AttachmentActionCmd( char *, char *, ViewMsgDialog *, int);
    AttachmentActionCmd( char *, char *, SendMsgDialog *, int);

    virtual const char *const className () { return "AttachmentActionCmd"; }
};

	
class SelectAllAttachsCmd : public Cmd {
    
private:
    AbstractEditorParent  *_parent;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    SelectAllAttachsCmd ( char *, char *, RoamMenuWindow *);
    SelectAllAttachsCmd	( char *, char *, ViewMsgDialog *);
    SelectAllAttachsCmd ( char *, char *, SendMsgDialog *);

    virtual const char *const className () { return "SelectAllAttachsCmd"; }
};


// Its critical that children of ToggleButtonCmd not have a 
// const className() method.
//

class ShowAttachPaneCmd : public ToggleButtonCmd {
    
private:
    AbstractEditorParent  *_parent;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    ShowAttachPaneCmd (char *, char *, AbstractEditorParent *);

};

class AbbrevHeadersCmd : public ToggleButtonCmd {
    
private:
    RoamMenuWindow *_parent;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AbbrevHeadersCmd (char *, char *, RoamMenuWindow *);
};

// Close button applies only to SMD

class CloseCmd : public NoUndoCmd {
  private:
	SendMsgDialog *_compose_dialog;
  public:
	Widget menubar_w;

	virtual void doit();
	CloseCmd( char *, char *, int, Widget, SendMsgDialog * );
	virtual const char *const className () { return "CloseCmd"; }
};

class EditUndoCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
	virtual void doit();
	EditUndoCmd( char *, char *, int, AbstractEditorParent * );
	virtual const char *const className () { return "EditUndoCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditCutCmd : public NoUndoCmd {
  private:
    Editor *editor;
    SendMsgDialog *_compose_dialog;
  public:
    virtual void doit();
    EditCutCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditCutCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditCopyCmd : public NoUndoCmd {
  private:
    Editor *editor;	
    SendMsgDialog *_compose_dialog;
  public:
    virtual void doit();
    EditCopyCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditCopyCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditPasteCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    EditPasteCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditPasteCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditPasteSpecialCmd : public NoUndoCmd {
  private:
    Editor *editor;
    Editor::InsertFormat insert_format;
  public:
    virtual void doit();
    EditPasteSpecialCmd( char *, char *, int, AbstractEditorParent *, Editor::InsertFormat );
    virtual const char *const className () { return "EditPasteSpecialCmd"; }
};


//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditClearCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    EditClearCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditClearCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class EditDeleteCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    EditDeleteCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditDeleteCmd"; }
};

//	XmTextEditor *editor;

class EditSelectAllCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    EditSelectAllCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "EditSelectAllCmd"; }
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class WordWrapCmd : public ToggleButtonCmd {
  private:
    Editor *editor;
    Boolean cur_setting;
  public:
    virtual void doit();
    virtual void undoit() {};
    WordWrapCmd( char *, char *, int, AbstractEditorParent * );
    Boolean wordWrap();
};

//	CDEM_DtWidgetEditor *editor;
//	XmTextEditor *editor;

class FindChangeCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    FindChangeCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "FindChangeCmd"; }
};

class SpellCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    SpellCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "SpellCmd"; }
};

class FormatCmd : public NoUndoCmd {
  private:
    Editor *editor;
  public:
    virtual void doit();
    FormatCmd( char *, char *, int, AbstractEditorParent * );
    virtual const char *const className () { return "FormatCmd"; }
};

// Applicable only to SMD

class LogMsgCmd : public ToggleButtonCmd {
  private:
    SendMsgDialog * _send;

  public:
	virtual void doit();
	virtual void undoit() {};
	LogMsgCmd( char *, char *, int, SendMsgDialog *);
    int LogMsg(void) { return ((ToggleButtonCmd *)this)->getButtonState(); }
};

class VacationCmd : public Cmd {
  private:
    char *_subject;
    char *_body;
    void	resetAtJobs();
    void removeForwardBackupFile();
    XtIntervalId _stop_timeout;
    XtIntervalId _start_timeout;
    void	removeBackupFile(char *);
    void	startVacationError(char *);
    int		backupFile(char *);
    void	createForwardVacationStr (char *buf, char *pw_name, char *ri);
    int		recoverForwardFile(char *);
    static void resetVacationCallback(XtPointer ptr, XtIntervalId * id);
    static void startVacationCallback(XtPointer ptr, XtIntervalId * id);
    char * getMonth(struct tm *tm);
    char * getHost();

  public:
    
    VacationCmd(char *, char *);
    virtual ~VacationCmd() {} ;

    virtual void doit();
    virtual void undoit() {};

    int		startVacation();
    void	stopVacation();
    void resetVacationStatus();

    int		handleMessageFile();
    Boolean	vacationIsOn();
    void	addVacationToForwardFile(char *);
    void 	removeVacationFromForwardFile();
    Boolean 	vacationAlreadyStarted();
    void	resetVacationItems();

    void	setStartVacationTimeout(time_t tick);
    void	setStopVacationTimeout(time_t tick);

    void	setSubject(char *subj) {if (_subject) free(_subject); _subject = subj;}
    void        setBody(char *body) {if (_body) free(_body); _body = body;}
    void        parseVacationMessage();
    void	resetVacationMessage();
    void	resetRadioButton(Boolean turnOn);
};

#endif

