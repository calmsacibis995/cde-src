/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the main init file.

#ifndef _SDM_MAIN_H
#define _SDM_MAIN_H

#pragma ident "@(#)Main.hh	1.38 96/06/14 SMI"

#include <iostream.h>
#include <stdlib.h>
#include <SDtMail/Sdtmail.hh>

extern SdmErrorCode RunAsyncTest();
extern SdmErrorCode RunExpungeMsgTest();
extern SdmErrorCode RunCancelOpTest();
extern SdmErrorCode RunConnectionTest();
extern SdmErrorCode RunDataTypeTest();
extern SdmErrorCode RunDisconnectTest();
extern SdmErrorCode RunSessionTest();
extern SdmErrorCode RunMessageStoreTest();
extern SdmErrorCode RunMessageTest();
extern SdmErrorCode RunMessageEnvelopeTest();
extern SdmErrorCode RunMessageBodyTest();
extern SdmErrorCode RunBasicTest();
extern SdmErrorCode RunUtilityTest();
extern SdmErrorCode RunSearchTest();
extern SdmErrorCode RunComposeTest();
extern SdmErrorCode RunOutgoingMessageTest();
extern SdmErrorCode RunUnsolicitEventTest();

extern void PrintSizeOfSelectedObjects();

#endif
