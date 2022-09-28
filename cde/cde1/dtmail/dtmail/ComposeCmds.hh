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

#ifndef COMPOSECMDS_HH
#define COMPOSECMDS_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ComposeCmds.hh	1.13 02/05/96"
#endif

class ComposeFamily : public RoamCmd {
  private:
    RoamMenuWindow *	_parent;
    
  public:
    ComposeFamily(char * name, char *label, int active, RoamMenuWindow *);
#ifndef CAN_INLINE_VIRTUALS
    ~ComposeFamily( void );
#endif /* ! CAN_INLINE_VIRTUALS */
    void Display_entire_msg(DtMailMessageHandle, SendMsgDialog *, char *);
    void appendSignature(SendMsgDialog *, Boolean );
    char * valueToAddrString(DtMailValueSeq & value);
};

class ComposeCmd : public ComposeFamily {
  private:
    RoamMenuWindow *	_parent;

  public:
    virtual void doit();   
    ComposeCmd( char *, char *, int, RoamMenuWindow * );
    virtual const char *const className () { return "ComposeCmd"; }
};
class ComposeSigCmd : public ComposeFamily {
  private:
    SendMsgDialog *	_smd;

  public:
    virtual void doit();   
    ComposeSigCmd( char *, char *, int, RoamMenuWindow *, SendMsgDialog * );
    virtual const char *const className () { return "ComposeSigCmd"; }
};

class ForwardCmd : public ComposeFamily {
  private:
	RoamMenuWindow *_parent;
	int _forward;
  public:
    virtual void doit();   
    ForwardCmd( char *, char *, int, RoamMenuWindow *, int );
    virtual const char *const className () { return "ForwardCmd"; }
};

class ReplyAllCmd : public ComposeFamily {
  private:
	RoamMenuWindow *_parent;
	int _include;
  public:
    virtual void doit();   
    ReplyAllCmd( char *, char *, int, RoamMenuWindow *, int );
    virtual const char *const className () { return "ReplyAllCmd"; }
};

class ReplyCmd : public ComposeFamily {
  private:
	RoamMenuWindow *_parent;
	int _include;
  public:
    virtual void doit();
    ReplyCmd( char *, char *, int, RoamMenuWindow *, int );
    virtual const char *const className () { return "ReplyCmd"; }
};
class AliasCmd : public NoUndoCmd {
  private:
    SendMsgDialog	*_compose;
    char		*_alias;

  public:
    virtual void doit();
    AliasCmd(char * name, 
		char *label, 
		int active, 
		SendMsgDialog *, 
		const char * alias);
    virtual ~AliasCmd();
    virtual const char *const className() { return "AliasCmd"; }
};

typedef enum { TYPE_TEMPLATE, TYPE_DRAFT } FileType;

class LoadFileCmd : public NoUndoCmd {
  private:
    SendMsgDialog	*_compose;
    char		*_file;
    FileType		_type;

  public:
    virtual void doit();
    LoadFileCmd(char * name, 
		char *label, 
		int active, 
		SendMsgDialog *, 
		const char * file,
		FileType type);
    virtual ~LoadFileCmd();
    virtual const char *const className() { return "LoadFileCmd"; }
};

class HideShowCmd : public NoUndoCmd {
  private:
    SendMsgDialog	*_compose;
    char		*_header;

  public:
    virtual void doit(void);
    HideShowCmd(char * name,
		char *widgetlabel,
		int active,
		SendMsgDialog *,
		const char * label);
    virtual ~HideShowCmd(void);
    virtual const char *const className() { return "HideShowCmd"; }
};

#endif
