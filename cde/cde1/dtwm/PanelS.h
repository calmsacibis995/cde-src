/* $XConsortium: PanelS.h /main/cde1_maint/1 1995/07/18 01:52:44 drk $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)PanelS.h	1.14 96/05/06";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1992 HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*******************************************************************************
******************************************************************************/
#ifndef _DtPanelShell_h
#define _DtPanelShell_h

#include <Xm/Xm.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifndef DtIsPanelShell
#define DtIsPanelShell(w) XtIsSubclass(w, dtPanelShellWidgetClass)
#endif /* DtIsPanelShell */

externalref WidgetClass dtPanelShellWidgetClass;

typedef struct _DtPanelShellClassRec * DtPanelShellWidgetClass;
typedef struct _DtPanelShellRec      * DtPanelShellWidget;


#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtPanelShell_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
