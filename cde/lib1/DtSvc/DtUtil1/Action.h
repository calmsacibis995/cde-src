/* $XConsortium: Action.h /main/cde1_maint/1 1995/07/17 17:57:40 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef _Dt_Action_h
#define _Dt_Action_h

#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Argument types
 */

#define DtACTION_FILE	 1	/* file argument */
#define DtACTION_BUFFER	 2	/* buffer argument */
#define DtACTION_STRING	 3	/* string argument */
#define DtACTION_NULLARG 4	/* untyped return-only argument */

typedef struct {
	void *bp;		/* location of buffer */
	int size;		/* size of buffer in bytes */
	char *type;		/* (opt.) type of buffer */
        char *name;		/* (opt.) name of buffer object */
        Boolean writable;	/* allow changes to buffer object? */
} DtActionBuffer;

typedef struct {
	char *name;
} DtActionFile;

typedef struct {
	/*
	 * Structure containing argument information
	 */
	int argClass;		/* see argument types */
	union {
		DtActionFile	file;
		DtActionBuffer	buffer;
        } u;
} DtActionArg;

/*
 * DtActionStatus codes
 */
typedef enum {
	DtACTION_OK,			/* If not any of the below */
	DtACTION_INVALID_ID,		/* ID is not valid */
	DtACTION_INVOKED,		/* the action invocation step is done */
	DtACTION_STATUS_UPDATE,		/* status update */
	DtACTION_DONE,			/* normal action termination code */
	DtACTION_FAILED,		/* error running action */
	DtACTION_CANCELED		/* normal action termination by cancel*/
} DtActionStatus;

typedef void (*DtDbReloadCallbackProc)(
		XtPointer	clientData);

/*
 * DtActionInvocationID is the fundamental user-space handle to invoked
 * actions.
 */
typedef unsigned long DtActionInvocationID;

typedef void (*DtActionCallbackProc) (
		DtActionInvocationID	id,
		XtPointer	client_data,
		DtActionArg	*actionArgPtr,
		int		actionArgCount,
		int		status);

/*
 * Functions
 */

extern Boolean DtActionExists(
		char		*actionName);

extern char * DtActionLabel(
		char		*actionName);

extern char * DtActionDescription(
		char		*actionName);

extern void DtDbReloadNotify ( 
		DtDbReloadCallbackProc proc, 
		XtPointer	clientData);

extern void DtDbLoad(void);

extern DtActionInvocationID DtActionInvoke (
		Widget		w,
		char		*action,
		DtActionArg 	*args,
		int		argCount,
		char		*termOpts,
		char		*execHost,
		char		*contextDir,
		int		useIndicator,
		DtActionCallbackProc statusUpdateCb,
		XtPointer	client_data);

extern char * DtActionIcon(
		char		*actionName);

#ifdef __cplusplus
}
#endif

#endif /* _Dt_Action_h */
