/*
 *  MsguHandle.c -- Routines to convert between handles and names
 *
 *  $XConsortium: MsguHandle.c /main/cde1_maint/1 1995/07/14 20:37:23 drk $
 *  $XConsortium: MsguHandle.c /main/cde1_maint/1 1995/07/14 20:37:23 drk $
 *
 * (C) Copyright 1993, Hewlett-Packard, all rights reserved.
 */

/*
 * Include Files
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Dt/MsgP.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>


/* 
 * _DtMsgHandleAllocate
 *
 * Convert a string name to a message handle
 */

DtMsgHandle
#ifdef _NO_PROTO
_DtMsgHandleAllocate (pchName, widget)
	char *			pchName;
	Widget			widget;
#else
_DtMsgHandleAllocate (
	char *			pchName, 
	Widget 			widget
)
#endif
{
    struct _DtMsgHandle  *pHandle;

    /* allocate data for this guy */
    pHandle = XtNew (struct _DtMsgHandle);

    /* fill in the blanks */
    DtMsgH_Name(pHandle) = XtNewString (pchName);
    DtMsgH_Widget(pHandle) = widget;
    DtMsgH_Atom(pHandle) = XInternAtom (XtDisplay(widget), pchName, False);
    DtMsgH_SvcData(pHandle) = NULL;
    DtMsgH_BcData(pHandle) = XtNew(DtMsgBroadcastData);
    DtMsgH_PropertyName(pHandle) = XtNewString (pchName);
    DtMsgH_PropertyAtom(pHandle) = pHandle->atom;
    DtMsgH_RegistryAtom(pHandle) = XInternAtom (XtDisplay(widget), 
					DT_MSG_XA_BROADCAST_REGISTRY, False);
    DtMsgH_SharedWindow(pHandle) = None;
    DtMsgH_SharedWidget(pHandle) = None;
    DtMsgH_Listener(pHandle) = None;
    DtMsgH_BreceiveProc(pHandle) = NULL;
    DtMsgH_BclientData(pHandle) =  NULL;
    DtMsgH_BSenderInit(pHandle) = False;

    /* return the ptr to the data struct as the "handle" */
    return (pHandle);
}

/* 
 * _DtMsgHandleSetProperty
 *
 * Set the property to be used for this handle
 */

void
#ifdef _NO_PROTO
_DtMsgHandleSetProperty (handle, pchPropName)
	DtMsgHandle	handle;
	char *		pchPropName;
#else
_DtMsgHandleSetProperty (
	DtMsgHandle	handle,
	char *		pchPropName
)
#endif
{
    if (DtMsgH_PropertyName(handle)) 
    {
	XtFree ((char *)DtMsgH_PropertyName(handle));
    }
    DtMsgH_PropertyName(handle) = XtNewString (pchPropName);
    DtMsgH_PropertyAtom(handle) = 
	XInternAtom (XtDisplay(DtMsgH_Widget(handle)), pchPropName, False);
}

/* 
 * _DtMsgHandleGetPropertyAtom
 *
 * Return the property atom for this handle
 */

Atom
#ifdef _NO_PROTO
_DtMsgHandleGetPropertyAtom (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleGetPropertyAtom (
	DtMsgHandle	handle
)
#endif
{
    return (DtMsgH_PropertyAtom(handle));
}

/* 
 * _DtMsgHandleGetPropertyName
 *
 * Return the property name for this handle
 */

char *
#ifdef _NO_PROTO
_DtMsgHandleGetPropertyName (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleGetPropertyName (
	DtMsgHandle	handle
)
#endif
{
    return (DtMsgH_PropertyName(handle));
}

/* 
 * _DtMsgHandleDestroy
 *
 * Destroy a handle, free the data
 */

void
#ifdef _NO_PROTO
_DtMsgHandleDestroy (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleDestroy (
	DtMsgHandle	handle
)
#endif
{
    if (handle)
    {
	if (DtMsgH_PropertyName(handle)) 
	    XtFree ((char *) DtMsgH_PropertyName(handle));

	if (DtMsgH_SharedWidget(handle))
	{
	    Widget widget;

	    /* The widget was a convenience for callbacks and
	     * event processing. Destroy it, but not the shared
	     * window. There may be others listening on that
	     * window.
	     */
	    widget = DtMsgH_SharedWidget(handle);
	    widget->core.window = None;
	    XtDestroyWidget (widget);
	    DtMsgH_SharedWidget(handle) = None;
	}
	if (DtMsgH_Listener(handle))
	{
	    XtDestroyWidget (DtMsgH_Listener(handle));
	    DtMsgH_Listener(handle) = None;
	}
	_DtMsgBroadcastSenderUninitialize (handle);

	XtFree ((char *) DtMsgH_Name(handle));
	XtFree ((char *) handle);
    }
}

char *
#ifdef _NO_PROTO
_DtMsgHandleGetName (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleGetName (
	DtMsgHandle	handle
)
#endif
{
    return (DtMsgH_Name(handle));
}

Widget
#ifdef _NO_PROTO
_DtMsgHandleGetWidget (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleGetWidget (
	DtMsgHandle	handle
)
#endif
{
    return (DtMsgH_Widget(handle));
}

Atom
#ifdef _NO_PROTO
_DtMsgHandleGetAtom (handle)
	DtMsgHandle	handle;
#else
_DtMsgHandleGetAtom (
	DtMsgHandle	handle
)
#endif
{
    return (DtMsgH_Atom(handle));
}
