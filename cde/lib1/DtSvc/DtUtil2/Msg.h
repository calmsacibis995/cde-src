#ifndef _DT_MSG_H 
#define _DT_MSG_H
/*
 *  Msg.h -- Header file for DT messaging library 
 *
 *  $XConsortium: Msg.h /main/cde1_maint/1 1995/07/14 20:35:47 drk $
 *  $XConsortium: Msg.h /main/cde1_maint/1 1995/07/14 20:35:47 drk $
 *
 * (C) Copyright 1993, Hewlett-Packard, all rights reserved.
 */
#include <X11/Intrinsic.h>
#include <Dt/DataTypes.h>


/*
 * _DtMessage handle definitions
 */
typedef struct _DtMsgHandle *DtMsgHandle;


/*
 * Return values
 */
#define dtmsg_NO_LISTENERS		(102)
#define dtmsg_SUCCESS			(1)
#define dtmsg_FAIL			(-1)
#define dtmsg_NO_SERVICE		(-102)
#define dtmsg_WRONG_FORMAT		(-103)
#define dtmsg_ANOTHER_PROVIDER		(-104)
#define dtmsg_LOST_SERVICE		(-105)

/*
 * Callback procedure prototypes
 */
typedef void (*DtMsgReceiveProc) ();

typedef void (*DtMsgStatusProc) ();


/*
 * Point-to-point Function Prototypes
 */
extern DtMsgHandle _DtMsgHandleAllocate (
#ifndef _NO_PROTO
	char *,			/* string name */
	Widget			/* widget (for service window) */
#endif
);

extern void _DtMsgHandleDestroy (
#ifndef _NO_PROTO
	DtMsgHandle		/* handle */
#endif
);

extern int _DtMsgServiceOffer (
#ifndef _NO_PROTO
	DtMsgHandle, 		/* service handle */
	Boolean,		/* seize service from other server */
	DtMsgReceiveProc, 	/* proc called when request received */
	Pointer,		/* client data passed to receive proc */
	DtMsgStatusProc, 	/* proc called when service lost */
	Pointer		/* client data passed to lose proc */
#endif
);

extern void _DtMsgServiceWithdraw (
#ifndef _NO_PROTO
	DtMsgHandle		/* service handle */
#endif
);

extern Boolean _DtMsgServiceIsOffered (
#ifndef _NO_PROTO
	DtMsgHandle		/* service handle */
#endif
);

extern int _DtMsgServiceRequest (
#ifndef _NO_PROTO
	DtMsgHandle, 		/* service handle */
	char **, 		/* ptr to message (array of strings) */
	int, 			/* number of strings in message */
	DtMsgReceiveProc, 	/* proc called when reply msg arrives */
	Pointer		/* client data passed to reply proc */
#endif
);


#define DtMsgContext   Pointer


#endif /* not defined _DT_MSG_H */
/***** END OF FILE ****/
