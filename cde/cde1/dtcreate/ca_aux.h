/* $XConsortium: ca_aux.h /main/cde1_maint/4 1995/10/08 00:11:19 pascale $ */
/***************************************************************************/
/*                                                                         */
/*  Header file for ca_aux.c                                               */
/*                                                                         */
/***************************************************************************/
#ifndef _CA_AUX_H_INCLUDED
#define _CA_AUX_H_INCLUDED

#include "UxXt.h"

#if defined(USL) || defined(__uxp__)
#if !defined(_DIRENT_H)
#include <dirent.h>
#endif
#if !defined(_DtIcon_h_)
#include <Dt/Icon.h> 
#endif 
#endif

/***************************************************************************/
/*                                                                         */
/*  Constants                                                              */
/*                                                                         */
/***************************************************************************/
#define EQUAL      0
#define NOT_EQUAL  1

/***************************************************************************/
/*                                                                         */
/*  Macros                                                                 */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*  Function Declarations                                                  */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

void readCAFromGUI ();
void getCAactionName ();
void getCAdblClkCmd ();
void getCAactionHelpText ();
void getCAactionOpensText ();
void getCAwindowType ();
void writeCAToGUI ();

void putCAactionName ();
void putCAdblClkCmd ();
void putCAfileTypes ();
void putCAactionOpensText ();
void putCAwindowType ();
void putCAactionHelpText ();
void putCAactionIcons ();
void putCAdropFiletypes ();

void set_ca_dialog_height ();
Widget get_selected_action_icon ();
Boolean CreateActionAppShellCheckFields();
void FreeAndClearAD();
void clear_CreateActionAppShell_fields();
Boolean compareAD();
ActionData *copyAD();
void FreeResources();

#else /* _NO_PROTO */

void readCAFromGUI (struct _ActionData *ca_struct);
void getCAactionName (struct _ActionData *ca_struct);
void getCAdblClkCmd (struct _ActionData *ca_struct);
void getCAactionHelpText (struct _ActionData *ca_struct);
void getCAactionOpensText (struct _ActionData *ca_struct);
void getCAwindowType (struct _ActionData *ca_struct);
void writeCAToGUI (struct _ActionData *ca_struct);

void putCAactionName (struct _ActionData *ca_struct);
void putCAdblClkCmd (struct _ActionData *ca_struct);
void putCAfileTypes (struct _ActionData *ca_struct);
void putCAactionOpensText (struct _ActionData *ca_struct);
void putCAwindowType (struct _ActionData *ca_struct);
void putCAactionHelpText (struct _ActionData *ca_struct);
void putCAactionIcons (struct _ActionData *ca_struct);
void putCAdropFiletypes (ActionData *pAD);

void set_ca_dialog_height (void);
Widget get_selected_action_icon (void);
Boolean CreateActionAppShellCheckFields(void);
void FreeAndClearAD(ActionData *pAD);
void clear_CreateActionAppShell_fields(void);
Boolean compareAD(ActionData *pAD1, ActionData *pAD2);
ActionData *copyAD(ActionData *pAD);
void FreeResources (void);

#endif /* _NO_PROTO */

/***************************************************************************/
/*                                                                         */
/*      Callbacks for Create Action 'File' menu                            */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

void activateCB_FileNew ();
void activateCB_FileOpen ();
void activateCB_FileQuit ();

#else /* _NO_PROTO */

void activateCB_FileNew (Widget wid, XtPointer cdata,
                         XtPointer cbstruct);
void activateCB_FileOpen (Widget wid, XtPointer cdata,
                         XtPointer cbstruct);
void activateCB_FileQuit (Widget wid, XtPointer cdata,
                         XtPointer cbstruct);

#endif /* _NO_PROTO */

/***************************************************************************/
/*                                                                         */
/*      Callbacks for Create Action 'Options' menu                         */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

void activateCB_ExpertOption ();
void valueChangedCB_ColorMonoOption ();
void createCB_ColorMonoOption ();

#else /* _NO_PROTO_ */

void activateCB_ExpertOption (Widget wid, XtPointer client_data, XtPointer *cbs);
void valueChangedCB_ColorMonoOption (Widget, XtPointer,
                              XmToggleButtonCallbackStruct *);
void createCB_ColorMonoOption (Widget);

#endif /* _NO_PROTO */

/***************************************************************************/
/*                                                                         */
/*      Callbacks for Create Action Panel Buttons                          */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

void activateCB_action_icon ();
void activateCB_add_filetype ();
void activateCB_edit_filetype ();

#else /* _NO_PROTO */

void activateCB_action_icon (Widget wid, XtPointer client_data, 
			     DtIconCallbackStruct *cbs);
void activateCB_add_filetype (Widget wid, XtPointer client_data,
                              XmPushButtonCallbackStruct *cbs);
void activateCB_edit_filetype (Widget wid, XtPointer client_data,
			       XmPushButtonCallbackStruct *cbs);

#endif /* _NO_PROTO */

#endif /* _CA_AUX_H_INCLUDED */
