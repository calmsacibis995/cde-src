/*
 * File: DraftDialog.h
 * Contains: Declarations of module objects, user create procedures,
 *           and callbacks.
 *
 * 
 *
 *    
 */

#ifndef _DRAFT_UI_H_
#define _DRAFT_UI_H_

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
    
    Widget	draftDialog;	/* object "dialog" */
    Widget	dialog_shellform;
    Widget	dialog_panedwin;
    Widget	dialog_form;
    
    Widget	dialog_button_panel;
    Widget	controlpane;
    
    Widget	draft_name_rowcolumn;	/* object "draft_name" */
    Widget	draft_name_label;
    Widget	draft_name;
    
    Widget	separator;
    Widget	save_button;
    Widget	cancel_button;
    Widget	help_button;

} DraftModuleDialogInfoRec, *DraftModuleDialogInfo;


extern DraftModuleDialogInfoRec module_dialog;

/*
 * Structure Clear Procedures: These set the fields to NULL
 */
int DraftDialogInfo_clear(DraftModuleDialogInfo instance);

/*
 * Structure Initialization Procedures: These create the widgets
 */
int draft_dialog_initialize(
    DraftModuleDialogInfo	instance,
    Widget	parent
);

/*
 * User Callbacks
 */
void draft_save_butCB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData
);
void draft_cancel_butCB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData
);

/*
 * Connections
 */
#endif /* _DRAFT_UI_H_ */
