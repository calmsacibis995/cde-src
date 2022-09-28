/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the base Event Class.

#pragma ident "@(#)Transaction.cc	1.32 97/03/21 SMI"

// Include Files.
#include <TranObjs/Transaction.hh>
#include <SDtMail/ServiceFunction.hh>

SdmTransaction::SdmTransaction(const SdmTransaction& event)
  :SdmLinkedListElement(this), _ServiceFunction(event._ServiceFunction)
{
}

SdmTransaction&
SdmTransaction::operator= (const SdmTransaction& event)
{
  if (this == &event)
    return *this;
    
  _ServiceFunction = event._ServiceFunction;
  return *this;
}

void
SdmTransaction::SetServiceFunction(SdmServiceFunction* svf)
{
  _ServiceFunction = svf;
}

SdmServiceFunction*
SdmTransaction::GetServiceFunction()
{
  return _ServiceFunction;
}

