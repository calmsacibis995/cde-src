/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: Rule_edit_stubs.C
 * Contains: Module callbacks and connection functions
 *
 * This file was generated by dtcodegen, from module Rule_edit
 *
 * Any text may be added between the DTB_USER_CODE_START and
 * DTB_USER_CODE_END comments (even non-C code). Descriptive comments
 * are provided only as an aid.
 *
 *  ** EDIT ONLY WITHIN SECTIONS MARKED WITH DTB_USER_CODE COMMENTS.  **
 *  ** ALL OTHER MODIFICATIONS WILL BE OVERWRITTEN. DO NOT MODIFY OR  **
 *  ** DELETE THE GENERATED COMMENTS!                                 **
 */

#include <stdio.h>
#include <Xm/Xm.h>
#include "dtb_utils.h"
#include "dtmailopts.h"
#include "Rule_edit_ui.h"


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All necessary header files have been included.
 ***
 *** Add include files, types, macros, externs, and user functions here.
 ***/

#include "RoamApp.h"
#include <DtMail/FilterListUiItem.hh>
#include <DtMail/RuleCmd.h>
#include <DtMail/OptCmd.h>

void
rule_browse_cb(void *client_data, char *selection)
{
       if (!selection) return;
       XtVaSetValues((_WidgetRec *)client_data, XmNvalue, selection, NULL);
}


/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/



void 
Rule_edit_cancel_button_CB1(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    widget=widget;
    callData = callData;
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    DtbRuleEditDialogInfo	dtbTarget = (DtbRuleEditDialogInfo)clientData;
    DtbRuleEditDialogInfo	instance = dtbTarget;	/* obsolete */
    
    if (!(dtbTarget->initialized))
    {
        dtb_rule_edit_dialog_initialize(dtbTarget, dtb_get_toplevel_widget());
    }
    XtUnmanageChild(instance->dialog_shellform);
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
editor_add_cb(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    widget = widget;
    callData = callData;
    DtbRuleEditDialogInfo	dtbTarget = (DtbRuleEditDialogInfo)clientData;
    DtbRuleEditDialogInfo	instance = dtbTarget;	/* obsolete */

    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    // Get the rules from UI
    RuleCmd *rule_ptr=NULL;
    XtVaGetValues(widget,
	XmNuserData, &rule_ptr,
	NULL);

    filter *f = NULL;
    if (rule_ptr) {
	if ((f = rule_ptr->get_filter()) == NULL)
	    return;
    }
    else
	return;

    FilterListUiItem *cmd_ptr = NULL;
    OptCmd *oc= (OptCmd*)theRoamApp.mailOptions();
    DtbOptionsDialogInfo od = oc->optionsDialog();

    // Add the rule to the scrolling list
    XtVaGetValues(od->auto_list,
	    XmNuserData, &cmd_ptr,
	    NULL);
    if (cmd_ptr) 
	cmd_ptr->handleAddButtonPress(f);
    XtUnmanageChild(instance->dialog_shellform);
    
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
editor_change_cb(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    widget = widget;
    callData = callData;
    DtbRuleEditDialogInfo	dtbTarget = (DtbRuleEditDialogInfo)clientData;
    DtbRuleEditDialogInfo	instance = dtbTarget;	/* obsolete */

    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    RuleCmd *rule_ptr=NULL;
    XtVaGetValues(widget,
	XmNuserData, &rule_ptr,
	NULL);

    filter *f = NULL;
    if (rule_ptr) {
	if ((f = rule_ptr->get_filter()) == NULL)
	    return;
    }
    else
	return;


    OptCmd *oc= (OptCmd*)theRoamApp.mailOptions();
    DtbOptionsDialogInfo od = oc->optionsDialog();

    FilterListUiItem  *cmd_ptr = NULL;

    // Change the rule in the scrolling list
    XtVaGetValues(od->auto_list,
	XmNuserData, &cmd_ptr,
	NULL);
    if (cmd_ptr) 
	cmd_ptr->handleChangeButtonPress(f);

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
editor_reset_cb(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    callData = callData;
    DtbRuleEditDialogInfo	dtbTarget = (DtbRuleEditDialogInfo)clientData;
    DtbRuleEditDialogInfo	instance = dtbTarget;	/* obsolete */
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    RuleCmd *rule_ptr=NULL;
    XtVaGetValues(widget,
	XmNuserData, &rule_ptr,
	NULL);

    if (rule_ptr)
	rule_ptr->update_fields();

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
editor_help_cb(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    widget = widget;
    callData = callData;
    clientData = clientData;
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    dtb_show_help_volume_info("Mailer.sdl", DTMAILHELPRULES);

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
create_match_tf(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    DtbRuleEditDialogInfo	dtbSource = (DtbRuleEditDialogInfo)callData;
    
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    clientData = clientData;
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc)XmProcessTraversal, (void*)XmTRAVERSE_NEXT_TAB_GROUP);

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}



/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All automatically-generated data and functions have been defined.
 ***
 *** Add new functions here, or at the top of the file.
 ***/

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/


