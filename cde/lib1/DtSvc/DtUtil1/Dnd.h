/* $XConsortium: Dnd.h /main/cde1_maint/1 1995/07/17 18:02:18 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef _Dt_Dnd_h
#define _Dt_Dnd_h

#include <Xm/DragIcon.h>
#include <Xm/DragC.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constants
 */

/* Dnd Callback Reasons */

enum {
	DtCR_DND_CONVERT_DATA,
	DtCR_DND_CONVERT_DELETE,
	DtCR_DND_DRAG_FINISH,
	DtCR_DND_TRANSFER_DATA,
	DtCR_DND_DROP_ANIMATE,
	DtCR_DND_ROOT_TRANSFER
};
#define DtCR_DND_TRANSFER DtCR_DND_TRANSFER_DATA

/* 
 * Dnd Drag Start Resources 
 *	dropOnRootCallback is private and should not be used
 */

#define	DtNdropOnRootCallback		"dropOnRootCallback"
#define	DtCDropOnRootCallback		"DropOnRootCallback"
#define DtNsourceIcon			"sourceIcon"
#define DtCSourceIcon			"SourceIcon"
#define	DtNbufferIsText			"bufferIsText"
#define	DtCBufferIsText			"BufferIsText"

/* 
 *Dnd Drop Register Resources
 */

#define	DtNdropAnimateCallback		"dropAnimateCallback"
#define	DtCDropAnimateCallback		"DropAnimateCallback"
#define	DtNpreserveRegistration		"preserveRegistration"
#define	DtCPreserveRegistration		"PreserveRegistration"
#define	DtNregisterChildren		"registerChildren"
#define	DtCRegisterChildren		"RegisterChildren"
#define	DtNtextIsBuffer			"textIsBuffer"
#define	DtCTextIsBuffer			"TextIsBuffer"

/*
 * Types
 */

typedef enum {
	DtDND_SUCCESS,
	DtDND_FAILURE
} DtDndStatus;

typedef unsigned long DtDndProtocol;

enum {
	DtDND_NOOP_TRANSFER		= 0L,
	DtDND_TEXT_TRANSFER	 	= (1L << 0),
	DtDND_FILENAME_TRANSFER	 	= (1L << 1),
	DtDND_BUFFER_TRANSFER	 	= (1L << 2)
};

typedef struct _DtDndBuffer {
        void *		bp;
        int     	size;
        String  	name;
} DtDndBuffer;

typedef struct _DtDndContext {
        DtDndProtocol	protocol;
	Cardinal	numItems;
	union {
		XmString *	strings;
		String *	files;
		DtDndBuffer *	buffers;
	} data;
} DtDndContext;

 
/*
 * Dnd Callback Structures
 */

typedef struct _DtDndConvertCallbackStruct {
        int     	reason;
        XEvent *	event;
        DtDndContext *	dragData;
	DtDndStatus	status;
} DtDndConvertCallbackStruct, *DtDndConvertCallback;

typedef struct _DtDndDragFinishCallbackStruct {
	int		reason;
	XEvent *	event;
	DtDndContext *	dragData;
	Widget		sourceIcon;
} DtDndDragFinishCallbackStruct, *DtDndDragFinishCallback;

typedef struct _DtDndTransferCallbackStruct {
	int		reason;
	XEvent *	event;
	Position	x, y;
	unsigned char	operation;
	DtDndContext *	dropData;
	Widget		dragContext;
	Boolean		completeMove;
	DtDndStatus	status;
} DtDndTransferCallbackStruct, *DtDndTransferCallback;

typedef DtDndTransferCallbackStruct DtDndDropCallbackStruct, *DtDndDropCallback;

typedef struct _DtDndDropAnimateCallbackStruct {
	int		reason;
	XEvent *	event;
	Position	x, y;
	unsigned char	operation;
	DtDndContext *	dropData;
} DtDndDropAnimateCallbackStruct, *DtDndDropAnimateCallback;


/*
 * Functions
 */

extern Widget DtDndCreateSourceIcon(
		Widget		widget,
		Pixmap 		source,
		Pixmap		mask);

extern Widget DtDndDragStart(
		Widget		dragInitiator, 
		XEvent*		event, 
		DtDndProtocol	protocol, 
		Cardinal	numItems, 
		unsigned char	operations,
                XtCallbackList	convertCallback, 
		XtCallbackList	dragFinishCallback,
		ArgList		argList,
		Cardinal	argCount);

extern Widget DtDndVaDragStart(
		Widget		dragInitiator, 
		XEvent*		event, 
		DtDndProtocol	protocol, 
		Cardinal	numItems, 
		unsigned char	operations,
                XtCallbackList	convertCallback, 
		XtCallbackList	dragFinishCallback,
		...);

extern void DtDndDropRegister(
		Widget		dropSite, 
		DtDndProtocol	protocols, 
		unsigned char	operations, 
                XtCallbackList	transferCallback,
		ArgList		argList,
		Cardinal	argCount);

extern void DtDndVaDropRegister(
		Widget		dropSite, 
		DtDndProtocol	protocols, 
		unsigned char	operations, 
                XtCallbackList	transferCallback,
		...);

extern void DtDndDropUnregister(
		Widget		widget);
 
#ifdef __cplusplus
}
#endif

#endif /* _Dt_Dnd_h */
