/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Service Function Class.

#ifndef _SDM_SERVICEFUNCTION_H
#define _SDM_SERVICEFUNCTION_H

#pragma ident "@(#)ServiceFunction.hh	1.32 97/03/14 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/Event.hh>


// SdmCallback is a pointer to a function that takes an error object,
// a pointer to client data, and a pointer to a response.
typedef void (*SdmCallback)(SdmError& err, void* userdata, SdmServiceFunctionType type, SdmEvent* response);


class SdmServiceFunction : public SdmLinkedListElement {

public:
  SdmServiceFunction();
  SdmServiceFunction(const SdmCallback& cbk, void* clientData = NULL, 
                      SdmServiceFunctionType type = Sdm_SVF_Unknown);
  SdmServiceFunction(const SdmServiceFunction& ctx);
  SdmServiceFunction& operator= (const SdmServiceFunction& ctx);
  virtual ~SdmServiceFunction();
  
  // execute the callback function.
  void Execute(SdmError& err, SdmServiceFunctionType type, SdmEvent* response);
  
  SdmServiceFunctionType GetType() const;
  
  // creates a new service function.
  SdmServiceFunction* SdmServiceFunctionFactory(void* clientData, 
                        SdmServiceFunctionType type) const;
  SdmServiceFunction* SdmServiceFunctionFactory(void* clientData) const;

private:
  SdmCallback _Callback;
  void* _ClientData;
  SdmServiceFunctionType  _ServiceType;
};

#endif
