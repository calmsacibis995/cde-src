#ifndef lint
static char sccsid[] = "@(#)propsui.c 1.11 95/05/04";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * propsui.c - User interface object initialization functions 
 * for the properties pop up.
 */

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Dt/SpinBox.h>

#include "help.h"
#include "imagetool.h" 
#include "ui_imagetool.h"

/*
 * Initialize an instance of object `properties'.
 */
PropertiesObjects *
PropertiesObjectsInitialize (parent)
     Widget   parent;
{
     Widget              children[12], sep1, sep2;   /* Children to manage */
     Arg                 al[64];             /* Arg List */
     register int        ac = 0;             /* Arg Count */
     XmString            xmstrings[16];   
     PropertiesObjects  *p;
     Widget              form, longest_label;
     Dimension           width1, width2, width3, width4, longest, height;
     int                 count1, count2, count3, divisor, sep_offset;
     Atom                delete_window;
     extern void         PopupEventHandler();
     extern void         PropsQuitHandler();
     extern void         PropsOKCallback();
     extern void         PropsResetCallback();
     extern void         PropsCancelCallback();
     extern void         PropsViewChangedCallback();
     extern void         PropsColorChangedCallback();
     extern void         PropsOpendocChangedCallback();
     extern void         PropsPSChangedCallback();

     p = (PropertiesObjects *) calloc (1, sizeof (PropertiesObjects));
/*
 * Create Shell, Dialog, Forms.
 */	
     ac = 0;
     XtSetArg(al[ac], XmNallowShellResize, TRUE); ac++;
     XtSetArg(al[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
     XtSetArg(al[ac], XmNtitle, catgets (prog->catd, 1, 222, 
					 "Image Viewer - Options")); ac++;
     p->shell = XmCreateDialogShell ( parent, "PropsShell", al, ac );
/*
 * Add Quit handler to catch unsaved changes.
 */
     delete_window = XmInternAtom (XtDisplay (p->shell), "WM_DELETE_WINDOW",
				   False);
     XmAddWMProtocolCallback (p->shell, delete_window, PropsQuitHandler, p);
/*
 * Create Message Dialog Box
 */
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNdialogType, XmDIALOG_TEMPLATE); ac++;	
     XtSetArg(al[ac], XmNnoResize, FALSE); ac++;
     p->dialog = XmCreateMessageBox (p->shell, "PropsDialog", al, ac );
/*
 * Add event handler to catch Maps/Unmaps.
 */
     XtAddEventHandler (p->shell, StructureNotifyMask, False,
			PopupEventHandler, p->dialog);
/*
 * Create Form
 */
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNfractionBase, 60); ac++;
     form = XmCreateForm ( p->dialog, "PropsForm", al, ac );
/*
 * View Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 223,
						      "View Image In:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     p->view_label = XmCreateLabel ( form, "ViewInLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Colors Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 224,
						      "Colors:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     p->colors_label = XmCreateLabel ( form, "ColorsLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Display Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 225,
						      "Display Palette:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     p->display_label = XmCreateLabel ( form, "DisplayLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNleftOffset, 5); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNrightOffset, 5); ac++;
     sep1 = XmCreateSeparatorGadget (form, "PropsSep1", al, ac);
     sep2 = XmCreateSeparatorGadget (form, "PropsSep2", al, ac);
/*
 * View PostScript Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 439,
						      "Display PostScript In:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     p->ps_label = XmCreateLabel ( form, "DisplayLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
    
/*
 * Find the longest label
 */
     XtVaGetValues (p->view_label, XmNwidth, &width1, NULL);
     XtVaGetValues (p->colors_label, XmNwidth, &width2, NULL);
     XtVaGetValues (p->display_label, XmNwidth, &width3, NULL);
     XtVaGetValues (p->ps_label, XmNwidth, &width4, NULL);
     
     longest = width1;
     longest_label = p->view_label;						
     if (width2 > longest) {
       longest = width2;
       longest_label = p->colors_label;
     }
     if (width3 > longest){
       longest = width3;
       longest_label = p->display_label;
     }
     if (width4 > longest){
       longest = width4;
       longest_label = p->ps_label;
     }
/*
 * Now that we know which label is the longest,
 * align the other labels with this label.
 */
     if (p->view_label != longest_label) 
       XtVaSetValues (p->view_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->colors_label != longest_label) 
       XtVaSetValues (p->colors_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->display_label != longest_label)
       XtVaSetValues (p->display_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->ps_label != longest_label)
       XtVaSetValues (p->ps_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);

/*
 * View Image In radio box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->view_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->radio1 = XmCreateRadioBox ( form, "PropsRadio1", al, ac );
/*
 * Create the toggle buttons in the radio boxes
 * based on the visual.
 */
     count1 = PropsViewButtonsCreate (p); 

/*
 * Colors radio box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, count1 + 1); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->colors_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->radio2 = XmCreateRadioBox ( form, "PropsRadio2", al, ac );
/*
 * Create the toggle buttons in the radio boxes
 * based on the visual.
 */
     count2 = PropsColorButtonsCreate (p);
     if (count2 >= 3) 
	count2++;
/*
 * Separator1
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, count1 + count2 + 2); ac++;
/*
 * PS radio box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, count1 + count2 + 3); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->ps_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->radio3 = XmCreateRadioBox ( form, "PropsRadio3", al, ac );
/*
 * Create the View PS toggle buttons in the radio boxes
 * based on the visual.
 */
     count3 = PropsPSButtonsCreate (p);
/*
 * Separator2
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, count1 + count2 + count3 + 3); ac++;
/*
 * Opening Document checkbox button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 226,
						      "On Opening Document"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, count1 + count2 + count3 + 5); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->display_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->opendoc_button = XmCreateToggleButtonGadget (form, "OpenDocButton", 
						     al, ac);
     XmStringFree ( xmstrings [ 0 ] );


/*
 * Set fraction base on form base on the number of
 * toggle buttons created.
 * Set Colors and Display label according to new fraction base.
 * Adjust the labels to align with toggle button, not radio boxes.
 */	
     XtVaGetValues (p->colors_label, XmNheight, &height, NULL);
     XtVaSetValues (form, XmNfractionBase, 
		            (int) (count1 + count2 + count3 + 6), NULL);
     if (height >= 32) {
       divisor = 5;
       sep_offset = 6;
     }
     else if (height >= 20) {
       divisor = 4;
       sep_offset = 5;
     }
     else if (height >= 17) {
       divisor = 3;
       sep_offset = 4;
     }
     else {
       divisor = 2;
       sep_offset = 3;
     } 

     XtVaSetValues (p->view_label, XmNtopOffset, ((int) height / divisor), NULL);
     XtVaSetValues (p->colors_label, XmNtopPosition, (int) (count1 + 1),  
		    XmNtopOffset, ((int) height / divisor), NULL);
     XtVaSetValues (sep1, XmNtopPosition, (int) (count1 + count2 + 2), 
		    XmNtopOffset, (int) (height / 4), NULL);
     XtVaSetValues (p->ps_label, 
		    XmNtopPosition, (int) (count1 + count2 + 3), 
		    XmNtopOffset, ((int) height / divisor), NULL);
     XtVaSetValues (sep2, XmNtopPosition, (int) (count1 + count2 + count3 + 4), 
		    XmNtopOffset, (int) (height / 4) + sep_offset, NULL);
     XtVaSetValues (p->display_label, 
		    XmNtopPosition, (int) (count1 + count2 + count3 + 5), 
		    XmNtopOffset, (int) height / (divisor+2), NULL);
/*
 * Create OK Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 227, 
						      "OK"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->ok_button = XmCreatePushButton (p->dialog, "PropsOKButton", al, ac);
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Apply Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 228, 
						      "Apply"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->apply_button = XmCreatePushButton (p->dialog, "PropsApplyButton", al, ac);
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Reset Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 229, 
						      "Reset"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->reset_button = XmCreatePushButton (p->dialog, "PropsResetButton", 
					   al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Cancel Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 230, 
						      "Cancel"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->cancel_button = XmCreatePushButton (p->dialog, "PropsCancelButton", 
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Help Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 231, 
						      "Help"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->help_button = XmCreatePushButton ( p->dialog, "PropsHelpButton", 
					  al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Set Cancel to be default button until edits on properties window.
 */
     XtVaSetValues (p->dialog, XmNdefaultButton, p->cancel_button, NULL);
     
     p->showing = False;

/*
 * Set help callback on dialog
 */
     XtAddCallback (p->dialog, XmNhelpCallback, HelpItemCallback, 
		    HELP_PROPS_ITEM);
/*
 * Set callbacks on buttons
 */
     XtAddCallback (p->ok_button, XmNactivateCallback, 
		    PropsOKCallback, p);
     XtAddCallback (p->apply_button, XmNactivateCallback, 
		    PropsOKCallback, p);
     XtAddCallback (p->reset_button, XmNactivateCallback, 
		    PropsResetCallback, p);
     XtAddCallback (p->cancel_button, XmNactivateCallback, 
		    PropsCancelCallback, p);
     XtAddCallback (p->help_button, XmNactivateCallback, HelpItemCallback, 
		    HELP_PROPS_ITEM);

/* 
 * Set value changed callbacks on editable items.
 */
     if (p->grayscale_toggle)
       XtAddCallback (p->grayscale_toggle, XmNvalueChangedCallback,
		      PropsViewChangedCallback, p);
     if (p->color_toggle)
       XtAddCallback (p->color_toggle, XmNvalueChangedCallback,
		      PropsViewChangedCallback, p);
     
     if (p->depth1_toggle)
       XtAddCallback (p->depth1_toggle, XmNvalueChangedCallback,
		      PropsColorChangedCallback, p);
     if (p->depth4_toggle)
       XtAddCallback (p->depth4_toggle, XmNvalueChangedCallback,
		      PropsColorChangedCallback, p);
     if (p->depth8_toggle)
       XtAddCallback (p->depth8_toggle, XmNvalueChangedCallback,
		      PropsColorChangedCallback, p);
     if (p->depth16_toggle)
       XtAddCallback (p->depth16_toggle, XmNvalueChangedCallback,
		      PropsColorChangedCallback, p);
     if (p->depth24_toggle)
       XtAddCallback (p->depth24_toggle, XmNvalueChangedCallback,
		      PropsColorChangedCallback, p);
     
     XtAddCallback (p->opendoc_button, XmNvalueChangedCallback,
		    PropsOpendocChangedCallback, p);

     if (p->colorps_toggle)
       XtAddCallback (p->colorps_toggle, XmNvalueChangedCallback,
		      PropsPSChangedCallback, p);
     if (p->bwps_toggle)
       XtAddCallback (p->bwps_toggle, XmNvalueChangedCallback,
		      PropsPSChangedCallback, p);

/*
 * Manage the widgets
 */
     ac = 0;
     if (p->grayscale_toggle)
       children[ac++] = p->grayscale_toggle;
     if (p->color_toggle)
       children[ac++] = p->color_toggle;
     if (ac > 0)
       XtManageChildren (children, ac);
     
     ac = 0;
     if (p->depth1_toggle)
       children[ac++] = p->depth1_toggle;
     if (p->depth4_toggle)
       children[ac++] = p->depth4_toggle;
     if (p->depth8_toggle)
       children[ac++] = p->depth8_toggle;
     if (p->depth16_toggle)
       children[ac++] = p->depth16_toggle;
     if (p->depth24_toggle)
       children[ac++] = p->depth24_toggle;
     if (ac > 0)
       XtManageChildren (children, ac);
     
     ac = 0;
     if (p->colorps_toggle)
       children[ac++] = p->colorps_toggle;
     children[ac++] = p->bwps_toggle;
     XtManageChildren (children, ac);

     ac = 0;
     children[ac++] = p->view_label;
     children[ac++] = p->colors_label;
     children[ac++] = p->display_label;
     children[ac++] = sep1;
     children[ac++] = sep2;
     children[ac++] = p->ps_label;
     children[ac++] = p->radio1;
     children[ac++] = p->radio2;
     children[ac++] = p->radio3;
     children[ac++] = p->opendoc_button;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = form;
     children[ac++] = p->ok_button;
     children[ac++] = p->apply_button;
     children[ac++] = p->reset_button;
     children[ac++] = p->cancel_button;
     children[ac++] = p->help_button;
     XtManageChildren(children, ac);
     
     return p;
}

