/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the I Session Class.

#ifndef _SDM_ISESS_H
#define _SDM_ISESS_H

#pragma ident "@(#)ISess.hh	1.39 96/05/23 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/LinkedList.hh>
#include <SDtMail/Session.hh>

class SdmMessageStore;
class SdmToken;

class SdmIncomingSession : public SdmSession {
public:
  SdmIncomingSession(SdmError& err, SdmConnection& connection);
  virtual ~SdmIncomingSession();

  // Message Store creation
  virtual SdmErrorCode SdmMessageStoreFactory(SdmError &err, SdmMessageStore *&r_mstore);
  

private:
};

#endif

