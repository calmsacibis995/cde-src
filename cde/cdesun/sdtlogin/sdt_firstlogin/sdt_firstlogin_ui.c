/*
 * File: sdt_firstlogin_ui.c
 * Contains: user module object initialize and create functions
 *
 * This file was generated by dtcodegen, from module sdt_firstlogin
 *
 *    ** DO NOT MODIFY BY HAND - ALL MODIFICATIONS WILL BE LOST **
 */


#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/PanedW.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include "dtb_utils.h"
#include "sdt_firstlogin.h"
#include "sdt_firstlogin_ui.h"


DtbSdtFirstloginMainwindowInfoRec	dtb_sdt_firstlogin_mainwindow = 
{
    False	 /* initialized */
};

/*
 * Widget create procedure decls
 */
static int dtb_sdt_firstlogin_mainwindow_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_controlpane2_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_separator_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_label2_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_radiobox_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_label3_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_separator2_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_button_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);
static int dtb_sdt_firstlogin_button2_create(
    DtbSdtFirstloginMainwindowInfo	instance,
    Widget	parent
);

int 
dtbSdtFirstloginMainwindowInfo_clear(DtbSdtFirstloginMainwindowInfo instance)
{
    memset((void *)(instance), 0, sizeof(*instance));
    return 0;
}

int 
dtb_sdt_firstlogin_mainwindow_initialize(
    DtbSdtFirstloginMainwindowInfo instance,
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

    dtb_sdt_firstlogin_mainwindow_create(instance,
        parent);
    dtb_sdt_firstlogin_controlpane2_create(instance,
        instance->mainwindow_form);
    dtb_sdt_firstlogin_separator_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_label2_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_radiobox_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_label3_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_separator2_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_button_create(instance,
        instance->controlpane2);
    dtb_sdt_firstlogin_button2_create(instance,
        instance->controlpane2);
    
    /*
     * Add widget-reference resources.
     */
    
    XtVaSetValues(instance->mainwindow_mainwin,
        XmNworkWindow, instance->mainwindow_form,
        NULL);
    
    XtVaSetValues(instance->separator,
        XmNtopWidget, instance->radiobox_rowcolumn,
        NULL);
    
    XtVaSetValues(instance->radiobox_rowcolumn,
        XmNtopWidget, instance->label3,
        NULL);
    
    XtVaSetValues(instance->label3,
        XmNtopWidget, instance->label2,
        NULL);
    
    XtVaSetValues(instance->separator2,
        XmNtopWidget, instance->button2,
        NULL);
    
    XtVaSetValues(instance->button,
        XmNtopWidget, instance->separator,
        NULL);
    
    XtVaSetValues(instance->button2,
        XmNtopWidget, instance->separator,
        NULL);
    
    /*
     * Call utility functions to do group layout
     */
    
    
    /*
     * Manage the tree, from the bottom up.
     */
    XtVaGetValues(instance->radiobox,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->radiobox_rowcolumn,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->controlpane2,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtVaGetValues(instance->mainwindow_form,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    XtManageChild(instance->mainwindow_form);
    XtVaGetValues(instance->mainwindow,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL);
    XtManageChildren(children, numChildren);
    
    /*
     * Add User and Connection callbacks
     */
    	XtAddCallback(instance->button,
		XmNactivateCallback, FL_Select_OK_Button,
    		(XtPointer)&(*instance));
	XtAddCallback(instance->button2,
		XmNactivateCallback, sdt_firstlogin_button2_CB1,
		(XtPointer)&(*instance));
return 0;
}



static int 
dtb_sdt_firstlogin_mainwindow_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    Display	*display= (parent == NULL? NULL:XtDisplay(parent));
    String	title_str = NULL;
    
    if (instance->mainwindow == NULL) {
        title_str = XtNewString(catgets(Dtb_project_catd, 2, 1, "First Login"));
        instance->mainwindow =
            XtVaCreatePopupShell("mainwindow",
                topLevelShellWidgetClass,
                parent,
                XmNallowShellResize, True,
                XmNtitle, title_str,
                XmNinitialState, NormalState,
                XmNbackground, dtb_cvt_string_to_pixel(parent, "Black"),
                NULL);
        XtFree(title_str);
        title_str = NULL;
    }
    if (instance->mainwindow == NULL)
        return -1;

    if (instance->mainwindow_mainwin == NULL) {
        instance->mainwindow_mainwin =
            XtVaCreateWidget("dtb_sdt_firstlogin_mainwindow_mainwin",
                xmMainWindowWidgetClass,
                instance->mainwindow,
                XmNbackground, dtb_cvt_string_to_pixel(instance->mainwindow, "Black"),
                NULL);
    }
    if (instance->mainwindow_mainwin == NULL)
        return -1;

    if (instance->mainwindow_form == NULL) {
        instance->mainwindow_form =
            XtVaCreateWidget("mainwindow_form",
                xmFormWidgetClass,
                instance->mainwindow_mainwin,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNresizePolicy, XmRESIZE_GROW,
                XmNheight, 852,
                XmNwidth, 999,
                XmNbackground, dtb_cvt_string_to_pixel(instance->mainwindow_mainwin, "Black"),
                NULL);
    }
    if (instance->mainwindow_form == NULL)
        return -1;

    
    /*
     * Call user (Post-)Create procs
     */
    FL_Main_Window_Created(instance->mainwindow, (XtPointer)NULL, (XtPointer)instance);
    return 0;
}



static int 
dtb_sdt_firstlogin_controlpane2_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    
    if (instance->controlpane2 == NULL) {
        instance->controlpane2 =
            XtVaCreateWidget("controlpane2",
                xmFormWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightAttachment, XmATTACH_NONE,
                XmNleftOffset, 0,
                XmNleftPosition, 50,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 0,
                XmNtopPosition, 50,
                XmNtopAttachment, XmATTACH_POSITION,
                XmNresizePolicy, XmRESIZE_ANY,
                XmNmarginHeight, 0,
                XmNmarginWidth, 0,
                XmNy, 0,
                XmNx, 0,
                NULL);
    }
    if (instance->controlpane2 == NULL)
        return -1;

    	dtb_center(instance->controlpane2, DTB_CENTER_POSITION_BOTH);
    return 0;
}



static int 
dtb_sdt_firstlogin_separator_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    
    if (instance->separator == NULL) {
        instance->separator =
            XtVaCreateWidget("separator",
                xmSeparatorWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, -1,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 10,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNseparatorType, XmSHADOW_ETCHED_IN,
                XmNorientation, XmHORIZONTAL,
                XmNheight, 8,
                XmNy, 219,
                XmNx, 0,
                NULL);
    }
    if (instance->separator == NULL)
        return -1;

    return 0;
}



static int 
dtb_sdt_firstlogin_label2_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 2, "Welcome to Solaris.  The following desktops are\navailable.  Which one would you like to use as\nyour default desktop?"));
    if (instance->label2 == NULL) {
        instance->label2 =
            XtVaCreateWidget("label2",
                xmLabelWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 37,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 37,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 20,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_BEGINNING,
                XmNy, 20,
                XmNx, 37,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->label2 == NULL)
        return -1;

    return 0;
}



static int 
dtb_sdt_firstlogin_radiobox_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    Arg	args[23];	/* need 18 args (add 5 to be safe) */
    int	n = 0;
    Widget	radiobox_choice_items[1];
    
    if (instance->radiobox_rowcolumn == NULL) {
        instance->radiobox_rowcolumn =
            XtVaCreateWidget("radiobox_rowcolumn",
                xmRowColumnWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightAttachment, XmATTACH_NONE,
                XmNleftOffset, 38,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 15,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNentryVerticalAlignment, XmALIGNMENT_BASELINE_BOTTOM,
                XmNpacking, XmPACK_TIGHT,
                XmNisHomogeneous, False,
                XmNadjustLast, True,
                XmNspacing, 0,
                XmNmarginWidth, 0,
                XmNmarginHeight, 0,
                XmNentryAlignment, XmALIGNMENT_BEGINNING,
                XmNorientation, XmVERTICAL,
                XmNy, 145,
                XmNx, 38,
                NULL);
    }
    if (instance->radiobox_rowcolumn == NULL)
        return -1;

    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 3, "Choose One:"));
    if (instance->radiobox_label == NULL) {
        instance->radiobox_label =
            XtVaCreateWidget("radiobox_label",
                xmLabelWidgetClass,
                instance->radiobox_rowcolumn,
                XmNmarginHeight, 8,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->radiobox_label == NULL)
        return -1;

    if (instance->radiobox == NULL) {
        n = 0;
        XtSetArg(args[n], XmNradioBehavior, True);  ++n;
        XtSetArg(args[n], XmNentryClass, xmToggleButtonWidgetClass);  ++n;
        XtSetArg(args[n], XmNnumColumns, 1);  ++n;
        XtSetArg(args[n], XmNisHomogeneous, True);  ++n;
        XtSetArg(args[n], XmNspacing, 0);  ++n;
        XtSetArg(args[n], XmNrowColumnType, XmWORK_AREA);  ++n;
        XtSetArg(args[n], XmNpacking, XmPACK_TIGHT);  ++n;
        XtSetArg(args[n], XmNorientation, XmVERTICAL);  ++n;
        instance->radiobox =
            XmCreateRadioBox(instance->radiobox_rowcolumn,
                "radiobox", args, n);
    }
    if (instance->radiobox == NULL)
        return -1;

    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 4, "Item1"));
    if (instance->radiobox_items.Item1_item == NULL) {
        instance->radiobox_items.Item1_item =
            XtVaCreateManagedWidget("Item1_item",
                xmToggleButtonWidgetClass,
                instance->radiobox,
                XmNset, True,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->radiobox_items.Item1_item == NULL)
        return -1;

    
    /*
     * Call user (Post-)Create procs
     */
    FL_Create_Choices(instance->radiobox, (XtPointer)NULL, (XtPointer)instance);
    return 0;
}



static int 
dtb_sdt_firstlogin_label3_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 9, "Note: You can change this default at any time by\nusing the Session Menu located on the Desktop\nLogin Screen's Options Menu."));
    if (instance->label3 == NULL) {
        instance->label3 =
            XtVaCreateWidget("label3",
                xmLabelWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 37,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 37,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 12,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_BEGINNING,
                XmNy, 81,
                XmNx, 37,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->label3 == NULL)
        return -1;

    return 0;
}



static int 
dtb_sdt_firstlogin_separator2_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    
    if (instance->separator2 == NULL) {
        instance->separator2 =
            XtVaCreateWidget("separator2",
                xmSeparatorWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, -2,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftOffset, 0,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 16,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNseparatorType, XmSHADOW_ETCHED_IN,
                XmNorientation, XmHORIZONTAL,
                XmNheight, 9,
                XmNy, 280,
                XmNx, 0,
                NULL);
    }
    if (instance->separator2 == NULL)
        return -1;

    return 0;
}



static int 
dtb_sdt_firstlogin_button_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 6, "OK"));
    if (instance->button == NULL) {
        instance->button =
            XtVaCreateWidget("button",
                xmPushButtonWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightAttachment, XmATTACH_NONE,
                XmNleftOffset, 45,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopOffset, 12,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNrecomputeSize, False,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNheight, 30,
                XmNwidth, 125,
                XmNy, 239,
                XmNx, 37,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->button == NULL)
        return -1;

    
    /*
     * Call user (Post-)Create procs
     */
    FL_Create_OK_Button(instance->button, (XtPointer)NULL, (XtPointer)instance);
    return 0;
}



static int 
dtb_sdt_firstlogin_button2_create(
    DtbSdtFirstloginMainwindowInfo instance,
    Widget parent
)
{
    XmString	label_xmstring = NULL;
    
    label_xmstring = XmStringCreateLocalized(catgets(Dtb_project_catd, 2, 7, "Cancel"));
    if (instance->button2 == NULL) {
        instance->button2 =
            XtVaCreateWidget("button2",
                xmPushButtonWidgetClass,
                parent,
                XmNbottomAttachment, XmATTACH_NONE,
                XmNrightOffset, 45,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_NONE,
                XmNtopOffset, 12,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNrecomputeSize, False,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNheight, 30,
                XmNwidth, 125,
                XmNy, 239,
                XmNx, 160,
                XmNlabelString, label_xmstring,
                NULL);
        XmStringFree(label_xmstring);
        label_xmstring = NULL;
    }
    if (instance->button2 == NULL)
        return -1;

    return 0;
}
