
/*
 *+SNOTICE
 *
 *	$Revision: 1.41 $
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
#pragma ident "@(#)MailSession.hh	1.41 05/05/97"
#endif

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Message.hh>
#include <SDtMail/Connection.hh>
#include "RoamMenuWindow.h"
#include "DtVirtArray.hh"
#include <Tt/tttk.h>

class MailSession {
  public:
    MailSession(SdmError &, XtAppContext, SdmSessionType);
    ~MailSession(void);

    SdmMessageStore * open(SdmError &,
                           SdmMessageNumber &,
			   SdmBoolean &,
			   const SdmToken &token,
			   void * client_data);

    SdmMessageStore * openAsync(SdmError &,
                                const SdmToken &token,
                                void * client_data);



    void close(SdmError &, const SdmMessageStore * folder);

#ifdef DOLATER
    void convert(SdmError &,
		 const char * old_path,
		 const char * new_path,
		 DtMailStatusCallback cb_func = NULL,
		 void * client_data = NULL);
    // raises ME_UnknownFormat
#endif // DOLATER

    void copy(SdmError &, SdmMessage & msg, const char * path);

    SdmSession * isession(void) { return _app_isession; }
    SdmSession * osession(void) { return _app_osession; }

    SdmBoolean isMboxOpen(const char *path, const char *hostname);
    RoamMenuWindow * getRMW(SdmMessageStore *mbox);
    RoamMenuWindow * getRMW(const char *path, const char *host);
    SdmBoolean getMailboxName( RoamMenuWindow *rmw, 
                        SdmString& path, SdmString& server);
    int getOpenCount();

  private:
    struct MailBoxItem {
	SdmMessageStore	*handle;
	char		*path;
	char		*hostname;
	int		open_ref_count;
	RoamMenuWindow	*rmw;
    };

    struct OpenContext {
	MailSession *session;
	SdmMessageStore *mbox;
	RoamMenuWindow *rmw;
	char *path;
	char *hostname;
    };
    void registerServiceFunctions(SdmError error, SdmMessageStore *mailbox,
                                  void *client_data);

    void checkForTooltalkLockingError(SdmError &error,
                  SdmMessageNumber &nmsgs,
                  SdmBoolean & readOnly,
                  const SdmToken &token,
                  SdmMessageStore*& mailbox);
                  
    int locate(const char * path, const char *server);
    int locate(const SdmMessageStore *);
    int locate(const RoamMenuWindow *);

    void addToList(SdmMessageStore *, const char * path,
		   const char *hostname, RoamMenuWindow * rmw);
    static Tt_status lockCB(Tt_message, Tttk_op, const char * path,
			    uid_t, int same_proc, void * instance);

    DtVirtArray<MailBoxItem *>	_open_mailboxes;

    static SdmSession	*_app_isession;
    static SdmSession	*_app_osession;
    static void updateMailboxCB(SdmError & error, void *clientdata,
                            SdmServiceFunctionType type, SdmEvent* event);

#ifdef OPEN_ASYNC                            
    void finishOpen(OpenContext *);
    static void openAsyncCB(SdmError & error, void *clientdata,
			    SdmServiceFunctionType type, SdmEvent* event);
#endif

    static void lockActivityCB(SdmError & error, void *clientdata,
			    SdmServiceFunctionType type, SdmEvent* event);
    static void logMessageServiceCB(SdmError & error, void *clientdata,
                            SdmServiceFunctionType type, SdmEvent* event);

    static void CloseMailboxCallback ( void* clientData );
    static void CloseAndReopenMailboxCallback ( void* clientData );
    static void SetReadOnlyMailboxCallback ( void* clientData );

};

#endif
