/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.3 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtMailXtProc.C	1.8 05/23/95"
#endif

#include <stdio.h>
#include <unistd.h>

#include <DtMail/DtMail.hh>
#include <DtMail/DtMailXtProc.h>

extern "C" void
DtMailXtInputProc(XtPointer client_data,
		  int * file_desc,
		  XtInputId *)
{
    DtMail::Session *session = (DtMail::Session *)client_data;

    // We read one event packet. The call back caller is then
    // invoked to fire the appropriate callback.
    //
    DtMailEventPacket	packet;

    read(*file_desc, &packet, sizeof(DtMailEventPacket));

    if (session->validObjectKey(packet.key) == DTM_FALSE) {
	return;
    }

    switch(packet.target) {
      case DTM_TARGET_MAILBOX:
      {
	DtMail::MailBox * self = (DtMail::MailBox *)packet.target_object;
	
	self->callCallback((DtMailCallbackOp)packet.operation, packet.argument);
	break;
      }

      case DTM_TARGET_TRANSPORT:
      {
	DtMail::Transport * self = (DtMail::Transport *)packet.target_object;
	self->callCallback((DtMailOperationId)packet.operation,
			   packet.argument);
	break;
      }

      default:
	break;
    }

    return;
}

void
DtMailProcessClientEvents(void)
{
#if 0
//    if (DtMailDamageContext) {
//	XtInputMask events = XtAppPending(DtMailDamageContext);
//	if (events & (XtIMXEvent | XtIMAlternateInput)) {
//	    XtAppProcessEvent(DtMailDamageContext, XtIMXEvent | XtIMAlternateInput);
//	}
//    }
#endif
}
