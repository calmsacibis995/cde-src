/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the basic Reply Class.

#ifndef _SDM_REPLY_H
#define _SDM_REPLY_H

#pragma ident "@(#)Reply.hh	1.34 97/03/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Event.hh>
#include <SDtMail/ServiceFunction.hh>



class SdmRequest;

class SdmReply : public SdmLinkedListElement {

  friend SdmRequest;

public:
  SdmReply();
  virtual ~SdmReply() {};
  SdmReply(const SdmReply& reply);

  SdmBoolean operator== (const SdmReply& reply);
  SdmReply& operator= (const SdmReply& reply);
  SdmReply& operator= (const SdmRequest& reqst);

public:
  void SetEvent(SdmEvent *event) { _Event = event; }
  void SetOperator(const int opId) { _ClassOpId = opId; }
  void SetClientObject(SdmPrim* client) { _Client = client; }
  void SetServiceFunctionType(SdmServiceFunctionType type)
          { _ServiceFuncType = type; }
  void SetServiceFunction(SdmServiceFunction* func)
          { _ServiceFunction = func; }
 
  SdmEvent* GetEvent() const { return _Event; }
  int GetOperator() const { return _ClassOpId; }
  SdmPrim* GetClientObject() const { return _Client; }
  SdmServiceFunctionType GetServiceFunctionType() const 
            { return _ServiceFuncType; }
  SdmServiceFunction* GetServiceFunction() 
            { return _ServiceFunction; }

 
private:
  SdmEvent* _Event;
  int _ClassOpId;
  SdmPrim* _Client;
  SdmServiceFunctionType _ServiceFuncType;
  SdmServiceFunction*    _ServiceFunction;
};

#endif
