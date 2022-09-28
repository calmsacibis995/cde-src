/*
 * File: TemplateDialog.c
 * 
 *
 * 
 *
 * 
 */


#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include "dtb_utils.h"
#include "TemplateDialog.h"
#include "RoamCmds.h"
#include "Help.hh"
#include "DtMailHelp.hh"
#include "MailMsg.h"


DtbModuleDialogInfoRec	dtb_module_dialog = 
{
    False	 /* initialized */
};

/*
 * Widget create procedure decls
 */
static int dtb_module_dialog_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_dialog_button_panel_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_controlpane_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_templ_name_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_separator_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_save_button_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_cancel_button_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);
static int dtb_module_help_button_create(
    DtbModuleDialogInfo	instance,
    Widget	parent
);

int 
dtbTemplTemplateDialogInfo_clear(DtbModuleDialogInfo instance)
{
    memset((void *)(instance), 0, sizeof(*instance));
    return 0;
}

int 
dtb_template_dialog_initialize(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    WidgetList	children = NULL;
    int		numChildren = 0;
    if (instance->initialized)
    {
        return 0;
    }
    instance->initialized = True;

    dtb_module_dialog_create(instance,
        parent);
    dtb_module_dialog_button_panel_create(instance,
        instance->dialog_panedwin);
    dtb_module_controlpane_create(instance,
        instance->dialog_form);
    dtb_module_templ_name_create(instance,
        instance->controlpane);
    dtb_module_separator_create(instance,
        instance->controlpane);
    dtb_module_save_button_create(instance,
        instance->dialog_button_panel);
    dtb_module_cancel_button_create(instance,
        instance->dialog_button_panel);
    dtb_module_help_button_create(instance,
        instance->dialog_button_panel);
    
    /*
     * Add widget-reference resources.
     */
	XtVaSetValues(instance->separator,
        XmNtopWidget, instance->templ_name_rowcolumn,
        NULL);

    
    /*
     * Call utility functions to do group layout
     */
    
    /*
     * Make Dialog Button-Panel & Footer a fixed height
     */
    {
        Dimension	pane_height;

        XtVaGetValues(instance->dialog_button_panel,
            XmNheight, &pane_height,
            NULL);
        XtVaSetValues(instance->dialog_button_panel,
            XmNpaneMinimum,   pane_height,
            XmNpaneMaximum,   pane_height,
            NULL);

    }
    
    /*
     * Manage the tree, from the bottom up.
     */
    XtVaGetValues(instance->templ_name_rowcolumn,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->controlpane,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->dialog_form,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->dialog_button_panel,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtManageChild(instance->dialog_form);
    XtManageChild(instance->dialog_button_panel);
    XtVaGetValues(instance->dialog_shellform,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    /*
     * Turn off traversal for invisible sash in dialog's PanedWindow
     */
    dtb_remove_sash_focus(instance->dialog_panedwin);
    
    /*
     * Add User and Connection callbacks
     */
    	XtAddCallback(instance->save_button,
		XmNactivateCallback, save_butCB,
    		(XtPointer)&(*instance));
	XtAddCallback(instance->cancel_button,
		XmNactivateCallback, cancel_butCB,
		(XtPointer)&(*instance));
 	XtAddCallback(instance->help_button,
                XmNactivateCallback, &HelpErrorCB,
                DTMAILHELPTEMPLATEDIALOGHELPBUTTON);
	XtVaSetValues(instance->dialog_shellform,
                XmNdefaultButton, instance->save_button,
                NULL);
return 0;
}



static int 
dtb_module_dialog_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    Display	*display= (parent == NULL? NULL:XtDisplay(parent));
    Arg	args[8];	/* need 3 args (add 5 to be safe) */
    int	n = 0;
    
    if (instance->templateDialog == NULL) {
        n = 0;
        XtSetArg(args[n], XmNallowShellResize, True);  ++n;
        XtSetArg(args[n], XmNbackground, dtb_cvt_string_to_pixel(parent, "white"));  ++n;
        instance->templateDialog =
            XmCreateDialogShell(parent,
                "dtb_module_dialog", args, n);
    }
    if (instance->templateDialog == NULL)
        return -1;

    if (instance->dialog_shellform == NULL) {
        instance->dialog_shellform =
            XtVaCreateWidget("dtb_module_dialog_shellform",
                xmFormWidgetClass,
                instance->templateDialog,
                XmNdefaultPosition, False,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNbackground, dtb_cvt_string_to_pixel(instance->templateDialog, "white"),
                NULL);
    }
    if (instance->dialog_shellform == NULL)
        return -1;

    if (instance->dialog_panedwin == NULL) {
        instance->dialog_panedwin =
            XtVaCreateWidget("dialog_panedwin",
                xmPanedWindowWidgetClass,
                instance->dialog_shellform,
                XmNbottomOffset, 0,
                XmNbottomPosition, 100,
                XmNbottomAttachment, XmATTACH_POSITION,
                XmNrightOffset, 0,
                XmNrightPosition, 100,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNleftOffset, 0,
                XmNleftPosition, 0,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 0,
                XmNtopPosition, 0,
                XmNtopAttachment, XmATTACH_POSITION,
                XmNsashWidth, 1,
                XmNsashHeight, 1,
                XmNspacing, 0,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNbackground, dtb_cvt_string_to_pixel(instance->dialog_shellform, "white"),
                NULL);
    }
    if (instance->dialog_panedwin == NULL)
        return -1;

    if (instance->dialog_form == NULL) {
        instance->dialog_form =
            XtVaCreateWidget("dialog_form",
                xmFormWidgetClass,
                instance->dialog_panedwin,
                XmNallowResize, True,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNresizePolicy, XmRESIZE_ANY,
                XmNbackground, dtb_cvt_string_to_pixel(instance->dialog_panedwin, "white"),
                NULL);
    }
    if (instance->dialog_form == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_dialog_button_panel_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    
    if (instance->dialog_button_panel == NULL) {
        instance->dialog_button_panel =
            XtVaCreateWidget("dialog_button_panel",
                xmFormWidgetClass,
                parent,
                XmNpaneMaximum, 48,
                XmNpaneMinimum, 48,
                XmNresizePolicy, XmRESIZE_GROW,
                XmNmarginHeight, 7,
                XmNmarginWidth, 7,
                XmNpositionIndex, 1,
                XmNheight, 48,
                NULL);
    }
    if (instance->dialog_button_panel == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_controlpane_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    
    if (instance->controlpane == NULL) {
        instance->controlpane =
            XtVaCreateWidget("controlpane",
                xmFormWidgetClass,
                parent,
                XmNbottomOffset, 0,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrightOffset, 0,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 0,
                XmNtopAttachment, XmATTACH_FORM,
                XmNresizePolicy, XmRESIZE_ANY,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNy, 0,
                XmNx, 0,
                NULL);
    }
    if (instance->controlpane == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_templ_name_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    Arg	args[27];	/* need 21 args (add 5 to be safe) */
    int	n = 0;
    
    if (instance->templ_name_rowcolumn == NULL) {
        instance->templ_name_rowcolumn =
            XtVaCreateWidget("templ_name_rowcolumn",
                xmRowColumnWidgetClass,
		parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightAttachment, XmATTACH_NONE,
                XmNleftOffset, 61,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 51,
                XmNtopAttachment, XmATTACH_FORM,
                XmNadjustLast, True,
                XmNspacing, 0,
                XmNmarginWidth, 0,
                XmNmarginHeight, 0,
                XmNentryAlignment, XmALIGNMENT_END,
                XmNorientation, XmHORIZONTAL,
                XmNy, 51,
                XmNx, 61,
                NULL);
    }
    if (instance->templ_name_rowcolumn == NULL)
        return -1;

    label_xmstring = XmStringCreateLocalized(catgets(DT_catd, 1, 332, "Template Name:"));
    if (instance->templ_name_label == NULL) {
        instance->templ_name_label =
            XtVaCreateWidget("templ_name_label",
                xmLabelWidgetClass,
                instance->templ_name_rowcolumn,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->templ_name_label == NULL)
        return -1;

    if (instance->templ_name == NULL) {
        n = 0;
        XtSetArg(args[n], XmNmaxLength, 80);  ++n;
        XtSetArg(args[n], XmNeditable, True);  ++n;
        XtSetArg(args[n], XmNcursorPositionVisible, True);  ++n;
        XtSetArg(args[n], XmNcolumns, 30);  ++n;
        instance->templ_name =
            XmCreateTextField(instance->templ_name_rowcolumn,
                "templ_name", args, n);
    }
    if (instance->templ_name == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_separator_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    
    if (instance->separator == NULL) {
        instance->separator =
            XtVaCreateWidget("separator",
                xmSeparatorWidgetClass,
                parent,
		XmNbottomOffset, 27,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrightOffset, 0,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 35,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNseparatorType, XmSHADOW_ETCHED_IN,
                XmNorientation, XmHORIZONTAL,
                XmNheight, 7,
                XmNwidth, 490,
                XmNy, 115,
                XmNx, 0,
                NULL);
    }
    if (instance->separator == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_save_button_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(DT_catd, 1, 94, "Save"));
    if (instance->save_button == NULL) {
        instance->save_button =
            XtVaCreateWidget("save_button",
                xmPushButtonWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 0,
                XmNrightPosition, 30,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNleftOffset, 0,
                XmNleftPosition, 10,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->save_button == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_cancel_button_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(DT_catd, 1, 182, "Cancel"));
    if (instance->cancel_button == NULL) {
        instance->cancel_button =
            XtVaCreateWidget("cancel_button",
                xmPushButtonWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 0,
                XmNrightPosition, 59,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNleftOffset, 0,
                XmNleftPosition, 40,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNy, 5,
                XmNx, 196,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->cancel_button == NULL)
        return -1;

    return 0;
}



static int 
dtb_module_help_button_create(
    DtbModuleDialogInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(DT_catd, 1, 76, "Help"));
    if (instance->help_button == NULL) {
        instance->help_button =
            XtVaCreateWidget("help_button",
                xmPushButtonWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 0,
                XmNrightPosition, 90,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNleftOffset, 0,
                XmNleftPosition, 70,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->help_button == NULL)
        return -1;

    return 0;
}

void
save_butCB(
    Widget widget,
    XtPointer ,
    XtPointer
)
{
    SaveAsTemplateCmd *cmd_ptr = NULL;
 
    XtVaGetValues(widget,
                XmNuserData, &cmd_ptr,
                NULL);
 
    if(cmd_ptr != NULL)
                cmd_ptr->getNameAndSave();
}
void
cancel_butCB(
    Widget widget,
    XtPointer,
    XtPointer
)
{
    SaveAsTemplateCmd *cmd_ptr = NULL;
 
    XtVaGetValues(widget,
                XmNuserData, &cmd_ptr,
                NULL);
 
    if(cmd_ptr != NULL)
                cmd_ptr->quit();
}

