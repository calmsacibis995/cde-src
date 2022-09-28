/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <stdio.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
		
int CreateTerminateButton();
void BringUpEditor();

Widget topLevelWid;
Arg	uiArgs[20];

void main (argc, argv)
unsigned int argc;
char **argv;
{
    XEvent 			next;
    XtAppContext 		app;
    Display			*display;
	int				i;
    
    XtToolkitInitialize();
    app = XtCreateApplicationContext();
    display = XtOpenDisplay(app, NULL, argv[0], "ResTest",
			    NULL, 0, &argc, argv);

	i = 0;
    XtSetArg(uiArgs[i], XmNargc, argc);i++;
    XtSetArg(uiArgs[i], XmNargv, argv);i++;
    topLevelWid = XtAppCreateShell ("resTest", "ResTest",
				    applicationShellWidgetClass,
				    display, uiArgs, i);
    CreateTerminateButton();
    XtRealizeWidget(topLevelWid);
    
    while(1)
    {
	XtAppNextEvent(app, &next);

	XtDispatchEvent(&next);
    
    }
}

int CreateTerminateButton ()
{
    Widget button, lock, form;
    register int n; 
    char *button_text, *lock_text;

    button_text = XmStringCreateLtoR ("Bring Up Editor", 
		    XmSTRING_DEFAULT_CHARSET);

    n = 0;
    XtSetArg(uiArgs[n], XmNhorizontalSpacing, 5);n++;
    XtSetArg(uiArgs[n], XmNverticalSpacing, 5);n++;
    XtSetArg(uiArgs[n], XmNwidth, 150);n++;
    XtSetArg(uiArgs[n], XmNheight, 50);n++;
    form = XmCreateForm(topLevelWid, "topForm", uiArgs, n);
    XtManageChild(form);
    
    n = 0;
    XtSetArg (uiArgs[n], XmNlabelType, XmSTRING);		n++;
    XtSetArg (uiArgs[n], XmNlabelString, button_text);	n++;
    XtSetArg(uiArgs[n], XmNtopAttachment, XmATTACH_FORM);n++;
    XtSetArg(uiArgs[n], XmNtopWidget, form);n++;
    XtSetArg(uiArgs[n], XmNbottomAttachment, XmATTACH_FORM);n++;
    XtSetArg(uiArgs[n], XmNbottomWidget, form);n++;
    XtSetArg(uiArgs[n], XmNleftAttachment, XmATTACH_FORM);n++;
    XtSetArg(uiArgs[n], XmNleftWidget, form);n++;
    XtSetArg(uiArgs[n], XmNrightAttachment, XmATTACH_FORM);n++;
    XtSetArg(uiArgs[n], XmNrightWidget, form);n++;
    button = XtCreateManagedWidget ("terminateButton", 
				    xmPushButtonWidgetClass,
				    form, uiArgs, n);
    XtAddCallback (button, XmNactivateCallback, BringUpEditor, NULL);

    return(0);
}

/*
 * Creates the resource editor and displays it
 */
void BringUpEditor (w, client_data, call_data)
Widget w;
caddr_t client_data;
caddr_t call_data;
{
    static Widget	resEditor = NULL, activeLabel;
    Widget	okButton, cancelButton, helpButton;
    Widget	addButton, fileButton, removeButton;
    String	activeResources, okString, cancelString, helpString;
    String	addString, fileString, removeString;
    int		i;

    if(resEditor == NULL)
    {
	activeResources = XmStringCreateLtoR("Currently Active Resources:",
					     XmSTRING_DEFAULT_CHARSET);
	okString = XmStringCreateLtoR("OK", XmSTRING_DEFAULT_CHARSET);
	cancelString = XmStringCreateLtoR("Cancel", XmSTRING_DEFAULT_CHARSET);
	helpString = XmStringCreateLtoR("Help", XmSTRING_DEFAULT_CHARSET);
	addString = XmStringCreateLtoR("Add a Resource",
				       XmSTRING_DEFAULT_CHARSET);
	fileString = XmStringCreateLtoR("Add a Resource File",
					XmSTRING_DEFAULT_CHARSET);
	removeString = XmStringCreateLtoR("Remove Selected Resources",
					  XmSTRING_DEFAULT_CHARSET);

	i = 0;
	XtSetArg(uiArgs[i], XmNautoUnmanage, False); i++;
	resEditor = XmCreateFormDialog(topLevelWid, "resEditor",
				       uiArgs, i);
    
	i = 0;
	XtSetArg(uiArgs[i], XmNshadowThickness, 0); i++;
	XtSetValues(resEditor, uiArgs, i);

	i = 0;
	XtSetArg(uiArgs[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;
	XtSetArg(uiArgs[i], XmNlabelString,	activeResources); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_FORM);i++;
	XtSetArg(uiArgs[i], XmNtopWidget, resEditor);i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_FORM);i++;
	XtSetArg(uiArgs[i], XmNleftWidget, resEditor);i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_FORM);i++;
	XtSetArg(uiArgs[i], XmNrightWidget, resEditor);i++;
	activeLabel = XmCreateLabelGadget(resEditor, "activeLabel", uiArgs, i);
	XtManageChild(activeLabel);
	XmStringFree(activeResources);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, okString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 0); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 33); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, activeLabel); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment, XmATTACH_FORM); i++;
	XtSetArg(uiArgs[i], XmNtopOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNbottomOffset, 10); i++;
	XtSetArg(uiArgs[i], XmNmarginTop, 5); i++;
	XtSetArg(uiArgs[i], XmNmarginBottom, 5); i++;
        okButton = XmCreatePushButton(resEditor, "okButton", uiArgs, i);
	XtManageChild(okButton);
	
	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, cancelString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 33); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 66); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, okButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, okButton); i++;
        cancelButton = XmCreatePushButton(resEditor, "cancelButton",
					  uiArgs, i);
	XtManageChild(cancelButton);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, helpString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 66); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 99); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, okButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, okButton); i++;
        helpButton = XmCreatePushButton(resEditor, "helpButton", uiArgs, i);
	XtManageChild(helpButton);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, addString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 0); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 33); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment,
		 XmATTACH_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, activeLabel); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, okButton); i++;
        addButton = XmCreatePushButton(resEditor, "addButton", uiArgs, i);
	XtManageChild(addButton);

	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, fileString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 33); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 66); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, addButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, addButton); i++;
        fileButton = XmCreatePushButton(resEditor, "fileButton", uiArgs, i);
	XtManageChild(fileButton);
	
	i = 0;
	XtSetArg(uiArgs[i], XmNlabelString, removeString); i++;
	XtSetArg(uiArgs[i], XmNleftAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNleftPosition, 66); i++;
	XtSetArg(uiArgs[i], XmNleftOffset, 5); i++;
	XtSetArg(uiArgs[i], XmNrightAttachment, XmATTACH_POSITION); i++;
	XtSetArg(uiArgs[i], XmNrightPosition, 99); i++;
	XtSetArg(uiArgs[i], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNtopWidget, addButton); i++;
	XtSetArg(uiArgs[i], XmNbottomAttachment,
		 XmATTACH_OPPOSITE_WIDGET); i++;
	XtSetArg(uiArgs[i], XmNbottomWidget, addButton); i++;
        removeButton = XmCreatePushButton(resEditor, "removeButton",
					  uiArgs, i);
	XtManageChild(removeButton);
	
	XtManageChild(resEditor);
    }
}



