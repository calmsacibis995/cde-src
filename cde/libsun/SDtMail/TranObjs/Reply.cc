/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Reply Event Class.

#pragma ident "@(#)Reply.cc	1.30 97/03/21 SMI"

// Include Files.
#include <TranObjs/Reply.hh>
#include <TranObjs/Request.hh>

SdmReply::SdmReply()
  : SdmLinkedListElement(this),
  _ClassOpId(-1), _Event(NULL), _Client(NULL), 
  _ServiceFuncType(Sdm_SVF_Unknown), _ServiceFunction(NULL)
{ }

SdmReply::SdmReply(const SdmReply& reply)
  : SdmLinkedListElement(this)
{
  *this = reply;
}

SdmBoolean
SdmReply::operator== (const SdmReply& reply)
{
  if (this == &reply)
    return Sdm_True;
  else
    return Sdm_False;
}

SdmReply&
SdmReply::operator= (const SdmReply& reply)
{
  if (this == &reply)
    return *this;

  _ClassOpId = reply._ClassOpId;
  _ServiceFuncType = reply._ServiceFuncType;
  
  // WARNING: can't copy _Client and _Event ptr if requests are 
  // passed across processes.
  _Client = reply._Client;
  _Event = reply._Event;
  _ServiceFunction = reply._ServiceFunction;
  
  return *this;

}

SdmReply&
SdmReply::operator= (const SdmRequest& reqst)
{
  _ServiceFunction = reqst._ServiceFunction;
  _ClassOpId = reqst._ClassOpId;
  
  // WARNING: can't copy _Client ptr if requests are passed across processes.
  _Client = reqst._Client;
  
  return *this;
}
