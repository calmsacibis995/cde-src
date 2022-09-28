/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Base Server Class.

#pragma ident "@(#)Server.cc	1.28 96/06/19 SMI"

// Include Files.
#include <iostream.h>
#include <assert.h>
#include <SDtMail/Server.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/Error.hh>

SdmServer::SdmServer(const SdmClassId classId, SdmError& err)
  : SdmPrim(classId),
  SdmLinkedListElement(this),
  _ServerQueue(NULL)
{
  err = Sdm_EC_Success;
}

SdmServer::SdmServer(const SdmServer& copy) 
  : SdmPrim(copy),
  SdmLinkedListElement(this)
{
  cout << "*** Error: SdmServer copy constructor called\n";
  assert(Sdm_False);
}

SdmServer& 
SdmServer::operator=(const SdmServer &rhs)
{
  cout << "*** Error: SdmServer assignment operator called\n";
  assert(Sdm_False);
  return *this;
}

SdmServer::~SdmServer()
{
}

SdmBoolean
SdmServer::operator== (const SdmServer& server)
{
  if (this == &server)
    return Sdm_True;
  else
    return Sdm_False;
}

SdmRequestQueue* 
SdmServer::GetRequestQueue()
{
   // we cache the pointer to the queue so that we only do the look
   // up the first time.
   if (!_ServerQueue) {
	  if (_CollectorSession) {
      SdmError err;
		  _ServerQueue = _CollectorSession->MapToServerQueue(err, this);
	  }
   }
   return _ServerQueue;
}


void
SdmServer::ResetServerQueue()
{
    _ServerQueue = NULL;
}
