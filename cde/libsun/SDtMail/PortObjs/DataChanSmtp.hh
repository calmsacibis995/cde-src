/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the "Smtp" data channel subclass
// --> It implements an interface to smtp compatible transports

#ifndef _SDM_DATACHANSMTP_H
#define _SDM_DATACHANSMTP_H

#pragma ident "@(#)DataChanSmtp.hh	1.10 97/03/27 SMI"

#include <PortObjs/DataChan.hh>
#include <stdio.h>
#include <mail.h>
#include <smtp.h>

// ---> smtp data channel class
//

class SdmDpDataChanSmtp : public SdmDpDataChan {
  
public:
  SdmDpDataChanSmtp(SdmDataPort* parentDataPort);
  virtual ~SdmDpDataChanSmtp();
  SdmErrorCode StartUp(SdmError& err);
  SdmErrorCode ShutDown(SdmError& err);
  
public:

// DATA CHANNEL API
// This is the API that is exported to the users of the data channel

// Attach to an object (as opposed to / as a precursor to / an "open")
//
SdmErrorCode Attach
  (
  SdmError& err,
  const SdmToken& tk		// token describing object to attach to
  );

// Cancel any operations pending on the data port
//
SdmErrorCode CancelPendingOperations
  (
  SdmError& err
  );

// Close the current connection down
//
SdmErrorCode Close
  (
  SdmError& err
  );


// open a connection to an object given a token stream
//
SdmErrorCode Open
  (
  SdmError& err,
  SdmMessageNumber& r_nmsgs,
  SdmBoolean& r_readOnly,
  const SdmToken& tk		// token describing object to open
  );

// Determine if a message has any addressess that a transport can handle
// and if so resolve (dealias) and produce a list of them - preparing
// the message for transmission
//
SdmErrorCode ResolveAndExtractAddresses
  (
  SdmError& err,
  SdmStringL& r_addressList,		// list of addressees resolved for transport returned in this container
  SdmStringL& r_filenameList,		// list of file names to append to resolved for transport returned in this container
  SdmDataPort& dataPort,		// data port on which message to operate on exists
  const SdmMessageNumber msgnum		// message number in data port of message to operate on
  );

// Submit a fully constructed message for transmission
//
SdmErrorCode Submit
  (
  SdmError& err,
  SdmDeliveryResponse& r_deliveryResponse,
  const SdmMsgFormat msgFormat,
  SdmDataPort& dataPort,
  const SdmMessageNumber msgnum,
  const SdmStringL &customHeaders
  );

protected:
  static SdmToken	*_dcRefToken;		// reference tokenused to filter out incompatible requests
  static int		_dcOneTimeFlag;		// global one time initialization flag

private:

  SENDSTREAM            *_dcStream;
};

#endif	// _SDM_DATACHANSMTP_H
