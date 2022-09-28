/*
 *+SNOTICE
 *
 *	$Revision: 1.1 $
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

#ifndef _MAILSESSION_HH
#define _MAILSESSION_HH

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MailSession.hh	1.14 05/31/95"
#endif

#include <DtMail/DtMail.hh>
#include "RoamMenuWindow.h"

class MailSession {
  public:
    MailSession(DtMailEnv &, XtAppContext);
    ~MailSession(void);

    DtMail::MailBox * open(DtMailEnv &, 
			   const char * path, 
			   DtMailCallback lockCB,
			   void * client_data,
			   DtMailBoolean auto_create = DTM_TRUE,
			   DtMailBoolean request_lock = DTM_TRUE,
			   DtMailBoolean auto_parse = DTM_TRUE);

    // raises ME_NotBento, ME_AlreadyLocked,
    // ME_OtherAssumedRole, ME_NoSuchFile

    void close(DtMailEnv &, const DtMail::MailBox * folder);

    void convert(DtMailEnv &, 
		 const char * old_path, 
		 const char * new_path,
		 DtMailStatusCallback cb_func = NULL,
		 void * client_data = NULL);
    // raises ME_UnknownFormat

    void copy(DtMailEnv &, DtMail::Message & msg, const char * path);
    // raises ME_NotBento, ME_RequestDenied


    DtMail::Session * session(void) { return _app_session; }

    Boolean isMboxOpen(const char *path);
    RoamMenuWindow * getRMW(DtMail::MailBox *mbox);

  private:
    struct MailBoxItem {
	DtMail::MailBox		*handle;
	char			*path;
	int			open_ref_count;
	RoamMenuWindow		*rmw;
    };

    int locate(const char * path);
    int locate(const DtMail::MailBox *);

    void addToList(DtMail::MailBox *, const char * path, RoamMenuWindow * rmw);
    static Tt_status lockCB(Tt_message, Tttk_op, const char * path,
			    uid_t, int same_proc, void * instance);

    DtVirtArray<MailBoxItem *>	_open_mailboxes;
    static DtMail::Session	*_app_session;
};

#endif
