/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.1 $
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
#pragma ident "@(#)DtMailTypes.hh	1.1 04/09/96"
#endif

#ifndef _DTMAILTYPES_H
#define _DTMAILTYPES_H

#include <DtMail/DtMailValues.hh>

typedef unsigned long DtMailObjectKey;

typedef enum {
    DtMailBusyState_NotBusy	=	0,
    DtMailBusyState_AutoSave	=	1,
    DtMailBusyState_NewMail	=	2
} DtMailBusyState;

typedef void * DtMailOperationId;

typedef struct {
    void *		buffer;
    unsigned long	size;
} DtMailBuffer;

/*
 * The callback ops are passed back to the client when the DtMail library
 * needs information from the user. Additional operations can be defined by
 * specific implementations. These should begin after the DTMC_RESERVED constant.
 */
typedef enum {
    DTMC_NOOP		=	0,
    DTMC_SAVE		=	1,
    DTMC_REVERT		=	2,
    DTMC_RAISE		=	3,
    DTMC_PASSWD		=	4,
    DTMC_NEWMAIL	=	5,
    DTMC_DELETEMSG	=	6,
    DTMC_BADSTATE	=	7,
    DTMC_UNLOCK		=	8,
    DTMC_LOSTLOCK	=	9,
    DTMC_QUERYLOCK	=	10,
    DTMC_READONLY	=	11,
    DTMC_READWRITEOVERRIDE =	12,
    DTMC_RESERVED 	=	8192
} DtMailCallbackOp;

typedef enum {
    DtMailFileObject		=	1,
    DtMailBufferObject		=	2,
    DtMailReservedObject	=	8192
} DtMailObjectSpace;

#ifdef DOLATER
// I want to take all this out, but I'm leaving it here for
// now just in case we need it.
typedef DtMailBoolean (*DtMailCallback)(DtMailCallbackOp op, 
					const char * path,
					const char * prompt_hint,
					void * client_data,
					...);

// This forward declaration is needed to resolve a circular dependency
// between DtMailTypes.h and DtMailError.hh
//
class DtMailEnv;

typedef void (*DtMailStatusCallback)(DtMailOperationId,
				     DtMailEnv & error,
				     void * client_data);

typedef DtMailBoolean (*DtMailEventFunc)(void * client_data);

typedef struct {
    char **		header_name;
    int			number_of_names;
} DtMailHeaderRequest;

typedef struct {
    DtMailValueSeq	*header_values;
    int			number_of_names;
} DtMailHeaderLine;

typedef enum {
    DtMailMessageNew		=	1,
    DtMailMessageDeletePending	=	2,
    DtMailMessageAnswered	=	3,
    DtMailMessageMultipart	=	4,
    DtMailMessagePartial	=	8,
    DtMailMessageReservedLimit	=	8192
} DtMailMessageState;

typedef enum {
    DtMailBodyPartDeletePending	=	1,
    DtMailBodyPartReservedLimit	=	8192
} DtMailBodyPartState;

typedef enum {
    DtMailCheckUnknown		=	0,
    DtMailCheckGood		=	1,
    DtMailCheckBad		=	2
} DtMailChecksumState;

/* The event packet is written to the event file descriptor.
 * This structure is only useful to programs dealing directly with
 * the DtMail library events.
 */

typedef enum {
    DTM_TARGET_MAILBOX,
    DTM_TARGET_TRANSPORT
} DtMailCallbackTarget;

typedef struct {
    DtMailObjectKey		key;
    DtMailCallbackTarget	target;
    void *			target_object;
    void *			operation;
    void *			argument;
    time_t			event_time;
} DtMailEventPacket;

typedef enum {
    DtMailLockRead	=	1,
    DtMailLockWrite	=	2
} DtMailLock;

#endif // DOLATER
#endif
