/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Event base Class.

#ifndef _SDM_TRANSACTION_H
#define _SDM_TRANSACTION_H

#pragma ident "@(#)Transaction.hh	1.34 97/03/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/ServiceFunction.hh>


class SdmTransaction : public SdmLinkedListElement {

public:
  SdmTransaction(const SdmClassId classId) : 
     SdmLinkedListElement(this), _ServiceFunction(NULL) { }

  virtual ~SdmTransaction() {};
  SdmTransaction(const SdmTransaction& event);
  SdmTransaction& operator= (const SdmTransaction& event);

public:
  void SetServiceFunction(SdmServiceFunction* svf);
  SdmServiceFunction* GetServiceFunction();

protected:
  SdmServiceFunction* _ServiceFunction;
};

#endif
