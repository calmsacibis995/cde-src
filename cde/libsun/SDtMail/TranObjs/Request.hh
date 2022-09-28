/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Reqeust Event.

#ifndef _SDM_REQUEST_H
#define _SDM_REQUEST_H

#pragma ident "@(#)Request.hh	1.40 97/03/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ServiceFunction.hh>


class SdmArgumentList;
class SdmReply;
class SdmPrim;

typedef SdmPtrVector<SdmRequest *> SdmRequestL;

class SdmRequest : public SdmLinkedListElement {

  friend SdmReply;

public:
  SdmRequest();
  SdmRequest(SdmArgumentList *args, int op, SdmPrim *client); 
  virtual ~SdmRequest() {};
  SdmRequest(const SdmRequest& reqst);

  SdmBoolean operator== (const SdmRequest& reqst);
  SdmRequest& operator= (const SdmRequest& reqst);
  SdmRequest& operator= (const SdmReply& reply);

public:
  void SetArguments(SdmArgumentList* ios) { _Transaction = ios; }
  void SetOperator(const int opId) { _ClassOpId = opId; } 
  void SetClientObject(SdmPrim* client) { _Client = client; }
  void SetServiceFunction(SdmServiceFunction* func) { _ServiceFunction = func; }

  SdmArgumentList* GetArguments() const { return _Transaction; }
  int GetOperator() const { return _ClassOpId; }
  SdmPrim* GetClientObject() const { return _Client; }
  SdmServiceFunction* GetServiceFunction() const { return _ServiceFunction; }

private:
  SdmArgumentList* _Transaction;
  int _ClassOpId;
  SdmPrim* _Client;	
  SdmServiceFunction* _ServiceFunction;
};

#endif
