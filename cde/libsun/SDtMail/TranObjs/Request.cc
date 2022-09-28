/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Basic Reqst Event Class.

#pragma ident "@(#)Request.cc	1.31 97/03/21 SMI"

// Include Files.
#include <TranObjs/Request.hh>
#include <TranObjs/Reply.hh>
#include <TranObjs/ArgumentList.hh>

SdmRequest::SdmRequest()
  : SdmLinkedListElement(this), 
   _ClassOpId(-1), _Client(NULL), _Transaction(NULL), _ServiceFunction(NULL)
{
}

SdmRequest::SdmRequest(SdmArgumentList *arg, int op, SdmPrim *client)
  : SdmLinkedListElement(this), _Transaction(arg), _ClassOpId(op), _Client(client),
    _ServiceFunction(NULL)
{
}

SdmRequest::SdmRequest(const SdmRequest& reqst)
  :SdmLinkedListElement(this)
{
  *this = reqst;
}

SdmBoolean
SdmRequest::operator== (const SdmRequest& reqst)
{
  if (this == &reqst)
    return Sdm_True;
  else
    return Sdm_False;
}

SdmRequest&
SdmRequest::operator= (const SdmRequest& reqst)
{
  if (this == &reqst)
    return *this;
  
  _ClassOpId = reqst._ClassOpId;
  
  // WARNING: can't copy _Client and _Transaction ptr if requests 
  // are passed across processes.
  _Client = reqst._Client;
  _Transaction = reqst._Transaction;
  _ServiceFunction = reqst._ServiceFunction;
  
  return *this;
}

SdmRequest&
SdmRequest::operator= (const SdmReply& reply)
{
  _ServiceFunction = reply._ServiceFunction;
  _ClassOpId = reply._ClassOpId;  
  
  // WARNING: can't copy _Client ptr if requests are passed across processes.
  _Client = reply._Client;
  
  return *this;
}

