/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Service Function Class.

#pragma ident "@(#)ServiceFunction.cc	1.31 97/03/14 SMI"

// Include Files
#include <assert.h>
#include <SDtMail/ServiceFunction.hh>

SdmServiceFunction::SdmServiceFunction()
  :  SdmLinkedListElement(this),
  _Callback(NULL), _ClientData(NULL),
  _ServiceType(Sdm_SVF_Unknown)
{
}

SdmServiceFunction::SdmServiceFunction(const SdmCallback& cbk,
	void* clientData, SdmServiceFunctionType type)
  : SdmLinkedListElement(this),
  _Callback(cbk), _ClientData(clientData),
  _ServiceType(type)
{
}

SdmServiceFunction::~SdmServiceFunction()
{
}

SdmServiceFunction::SdmServiceFunction(const SdmServiceFunction& svf)
  : SdmLinkedListElement(this)
{
   _Callback = svf._Callback;
   _ClientData = svf._ClientData;
   _ServiceType = svf._ServiceType;
}
  
SdmServiceFunction& 
SdmServiceFunction::operator=(const SdmServiceFunction& svf)
{
  if (&svf != this) {
    _Callback = svf._Callback;
    _ClientData = svf._ClientData;
    _ServiceType = svf._ServiceType;
  }
  return *this;
}
  
  // execute the callback function.
void 
SdmServiceFunction::Execute(SdmError& err, SdmServiceFunctionType type, 
    SdmEvent* response)
{
   // call callback function passing it the error, clientdata, and response.
   assert(_Callback != NULL);
   (*_Callback)(err, _ClientData, type, response);
}
  
SdmServiceFunctionType 
SdmServiceFunction::GetType() const
{
  return _ServiceType;
}

  
SdmServiceFunction* 
SdmServiceFunction::SdmServiceFunctionFactory(void* data, 
    SdmServiceFunctionType type) const
{
  void* userdata = (data == NULL ? _ClientData : data);
  return (new SdmServiceFunction(_Callback, userdata, type));
}
  
  
SdmServiceFunction* 
SdmServiceFunction::SdmServiceFunctionFactory(void* data) const
{
  void* userdata = (data == NULL ? _ClientData : data);
  return(new SdmServiceFunction(_Callback, userdata, _ServiceType));
}
