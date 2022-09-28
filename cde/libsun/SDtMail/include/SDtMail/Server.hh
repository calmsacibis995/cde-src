/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Server Class.

#ifndef _SDM_SERVER_H
#define _SDM_SERVER_H

#pragma ident "@(#)Server.hh	1.30 96/06/19 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/Vector.hh>
#include <SDtMail/LinkedList.hh>

class SdmRequestQueue;
class SdmError;

class SdmServer : public SdmPrim, public SdmLinkedListElement {

public:
  SdmServer(const SdmClassId classId, SdmError& err);
  virtual ~SdmServer();

  SdmBoolean operator== (const SdmServer& server);

  SdmRequestQueue* GetRequestQueue();
  
protected:
  void ResetServerQueue();

private:

  // Disallow creating copies of SdmServer.
  SdmServer(const SdmServer& copy);
  SdmServer& operator=(const SdmServer &rhs);
  SdmRequestQueue* _ServerQueue;

};

#endif

