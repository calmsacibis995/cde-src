/* 
 *+SNOTICE
 *
 *      $Revision: 1.2 $
 * 
 *      RESTRICTED CONFIDENTIAL INFORMATION: 
 * 
 *      The information in this document is subject to special 
 *      restrictions in a confidential disclosure agreement between 
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this 
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without 
 *      Sun's specific written approval.  This document and all copies 
 *      and derivative works thereof must be returned or destroyed at 
 *      Sun's request. 
 * 
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved. 
 * 
 *+ENOTICE 
 */
#ifndef _TEMPLATE_UI_H_
#define _TEMPLATE_UI_H_

#include <stdlib.h>
#include <X11/Intrinsic.h>
#include "dtb_utils.h"

/*
 * User Interface Objects
 */
/*
 * Shared data structures
 */

typedef struct
{
    Boolean	initialized;
    
    Widget	templateDialog;	/* object "dialog" */
    Widget	dialog_shellform;
    Widget	dialog_panedwin;
    Widget	dialog_form;
    
    Widget	dialog_button_panel;
    Widget	controlpane;
    
    Widget	templ_name_rowcolumn;	/* object "templ_name" */
    Widget	templ_name_label;
    Widget	templ_name;
    
    Widget	separator;
    Widget	save_button;
    Widget	cancel_button;
    Widget	help_button;
} DtbModuleDialogInfoRec, *DtbModuleDialogInfo;


extern DtbModuleDialogInfoRec dtb_module_dialog;

/*
 * Structure Clear Procedures: These set the fields to NULL
 */
int dtbTemplTemplateDialogInfo_clear(DtbModuleDialogInfo instance);

/*
 * Structure Initialization Procedures: These create the widgets
 */
int dtb_template_dialog_initialize(
    DtbModuleDialogInfo	instance,
    Widget	parent
);

/*
 * User Callbacks
 */
void save_butCB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData
);
void cancel_butCB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData
);

/*
 * Connections
 */
#endif /* _TEMPLATE_UI_H_ */
