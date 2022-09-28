/* $XConsortium: VolSelectI.h /main/cde1_maint/1 1995/07/17 17:54:39 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        VolSelectI.h
 **
 **   Project:     DtHelp Project
 **
 **   Description: File locating and handling utilities
 ** 
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelp_VolSelectI_h
#define _DtHelp_VolSelectI_h

#include "Canvas.h"
#include "DisplayAreaP.h"
#include <Xm/Xm.h>

/* collects all the dlg children together */
/* sort of a pseudo widget rec */
typedef struct _DtHelpFileDlgChildren {
          Widget        shell;
          Widget        form;
          Widget        label;
          Widget        list;
          Widget        prompt;
          Widget        separator;
          Widget        closeBtn;
          Widget        helpBtn;
} _DtHelpFileDlgChildren;

#ifdef _NO_PROTO
Widget _DtHelpFileListCreateSelectionDialog ();
#else
Widget _DtHelpFileListCreateSelectionDialog (
        DtHelpDialogWidget hw,
        Widget         parent,
        Boolean        modalDialog,
        char *         dlgTitle,
        XmFontList *   io_titlesFontList,
        _DtHelpFileList   in_out_list,
        _DtHelpFileDlgChildren * out_struct);
#endif 

#endif /* _DtHelp_VolSelectI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

