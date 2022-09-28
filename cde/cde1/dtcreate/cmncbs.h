/* $XConsortium: cmncbs.h /main/cde1_maint/1 1995/07/17 20:29:04 drk $ */
/***************************************************************************/
/*                                                                         */
/*  cmncbs.h                                                               */
/*                                                                         */
/***************************************************************************/

#ifndef _CMNCBS_H_INCLUDED
#define _CMNCBS_H_INCLUDED

#ifndef STORAGECLASS
#ifdef  NOEXTERN
#define STORAGECLASS
#else
#define STORAGECLASS extern
#endif
#endif

/***************************************************************************/
/*                                                                         */
/*  Prototypes for functions                                               */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

void activateCB_open_FindSet();
void activateCB_edit_icon ();
void DisplayHelpDialog ();
void helpCB_general ();

#else /* _NO_PROTO */

void activateCB_open_FindSet (Widget,XtPointer,XmFileSelectionBoxCallbackStruct *);
void activateCB_edit_icon (Widget,XtPointer,XmPushButtonCallbackStruct *);
void DisplayHelpDialog (Widget, XtPointer, XtPointer);
void helpCB_general (Widget, XtPointer, XtPointer);

#endif /* _NO_PROTO */

#endif /* _CMNCBS_H_INCLUDED */
