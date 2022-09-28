/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This contains the functions used to link the session to the XtEventLoop.

#ifndef _SDM_SESSIONLINK_H
#define _SDM_SESSIONLINK_H
 
#pragma ident "@(#)SessionLink.hh	1.13 96/05/13 SMI"

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <SDtMail/Session.hh>

extern SdmErrorCode SdmXtAttachSession(SdmError &err, XtAppContext &appcontext, SdmSession &ses);
extern void         SdmXtHandleSessionEvent(XtPointer calldata, int *pipe, XtInputId *id);

#endif
