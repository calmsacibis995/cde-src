#ifndef lint
static char sccsid[] = "@(#)printui.c 1.20 96/03/07";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * printui.c - User interface object initialization functions.
 * for print pop up.
 * This file was generated by `gxv' from `imagetool.G'.
 */

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Dt/SpinBox.h>

#include "help.h"
#include "imagetool.h"
#include "ui_imagetool.h"

/*
 * Initialize an instance of object `printing'.
 */
PrintObjects *
PrintObjectsInitialize (parent)
     Widget  parent;
{
     Widget            children[24];     /* Children to manage */
     Arg               al[64];           /* Arg List */
     register int      ac = 0;           /* Arg Count */
     XmString         *list_items;       /* For list items */
     int               list_item;        /* Index for list_items */
     XmString          xmstrings[16];    /* temporary storage for XmStrings */
     PrintObjects     *p;
     Widget            form, longest_label, menu;
     Dimension         width1, width2, width3, width4, width5, width6;
     Dimension         longest, height;
     extern void       PrintCancelCallback();
     extern void       PopupEventHandler();
     extern void       PrintButtonCallback();
     extern void       PageRangeCallback();
     extern void       ImagePositionCallback();
     extern void       ImageOrientationCallback();
     extern void       PrintUnitsCallback();
     extern void       ImageSizeTextCallback();
     extern void       ImageSizeSliderCallback();
     extern void       CopiesTextCallback();
     extern void       MarginTextCallback();
     extern void       SetPrinterList();

     p = (PrintObjects *) calloc (1, sizeof (PrintObjects));
/*
 * Create Shell, Dialog, Forms.
 */
     ac = 0;
     XtSetArg(al[ac], XmNallowShellResize, False); ac++;
     XtSetArg(al[ac], XmNtitle, catgets (prog->catd, 1, 172, 
					 "Image Viewer - Print")); ac++;
     p->shell = XmCreateDialogShell ( parent, "PrintShell", al, ac );
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
     XtSetArg(al[ac], XmNfractionBase, 37); ac++;
     form = XmCreateForm ( p->dialog, "PrintForm", al, ac );
/*
 * Copies Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 173,
					     "Copies:")); ac++;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     p->copies_label = XmCreateLabel ( form, "CopiesLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Copies Spin Box
 */
     ac = 0;
     XtSetArg(al[ac], DtNspinBoxChildType, DtNUMERIC); ac++;
     XtSetArg(al[ac], DtNmaximumValue, 100); ac++;
     XtSetArg(al[ac], DtNminimumValue, 1); ac++;
     XtSetArg(al[ac], DtNposition, 1); ac++;
     XtSetArg(al[ac], XmNshadowThickness, 0); ac++;
     XtSetArg(al[ac], XmNcolumns, 4); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++; 
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->copies_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->copies_value = DtCreateSpinBox (form, "CopiesSpinBox", al, ac);
     XtVaGetValues (p->copies_value, DtNtextField, &p->copies_text, NULL);
/*
 * Header Check Box
 */ 
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 174,
						      "Print"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++; 
     XtSetArg(al[ac], XmNset, True); ac++; 
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++;
     XtSetArg(al[ac], XmNtopOffset, 1); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->header_checkbox = XmCreateToggleButtonGadget (form, "HeaderCheckbox", 
						      al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Header Page Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 175,
						      "Header Page:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 0); ac++; 
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNrightWidget, p->header_checkbox); ac++;
     XtSetArg(al[ac], XmNrightOffset, 10); ac++;
     p->header_label = XmCreateLabel (form, "HeaderPageLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Printer List Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 176,
						      "Printer:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 3); ac++;
     p->printer_label = XmCreateLabel (form, "PrinterLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Page Size Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 177,
						      "Page Size:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 8); ac++;
     p->size_label = XmCreateLabel (form, "PageSizeLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Page Range Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 178,
						      "Page Range:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 13); ac++;
     p->range_label = XmCreateLabel (form, "PageRangeLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Image Size label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 179,
						      "Image Size:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 18); ac++;
     p->image_size_label = XmCreateLabel (form, "ImageSizeLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Orientation Label 
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 180,
						      "Image Orientation:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 24); ac++;
     p->orientation_label = XmCreateLabel (form, "OrientationLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Image Position Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 181,
						      "Image Position:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 29); ac++;
     p->position_label = XmCreateLabel (form, "ImagePositionLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Find the longest label
 */
     XtVaGetValues (p->printer_label, XmNwidth, &width1, NULL);
     XtVaGetValues (p->size_label, XmNwidth, &width2, NULL);
     XtVaGetValues (p->range_label, XmNwidth, &width3, NULL);
     XtVaGetValues (p->image_size_label, XmNwidth, &width4, NULL);
     XtVaGetValues (p->orientation_label, XmNwidth, &width5, NULL);
     XtVaGetValues (p->position_label, XmNwidth, &width6, NULL);

     longest = width1;
     longest_label = p->printer_label;						
     if (width2 > longest) {
       longest = width2;
       longest_label = p->size_label;
     }
     if (width3 > longest){
       longest = width3;
       longest_label = p->range_label;
     }
     if (width4 > longest) {
       longest = width4;
       longest_label = p->image_size_label;
     }
     if (width5 > longest){
       longest = width5;
       longest_label = p->orientation_label;
     }
     if (width6 > longest){
       longest = width6;
       longest_label = p->position_label;
     }
/*
 * Now that we know which label is the longest,
 * align the other labels with this label.
 */
     if (p->printer_label != longest_label) 
       XtVaSetValues (p->printer_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->size_label != longest_label) 
       XtVaSetValues (p->size_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->range_label != longest_label)
       XtVaSetValues (p->range_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->image_size_label != longest_label) 
       XtVaSetValues (p->image_size_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->orientation_label != longest_label) 
       XtVaSetValues (p->orientation_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);
     if (p->position_label != longest_label)
       XtVaSetValues (p->position_label, 
		      XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		      XmNrightWidget, longest_label, 
		      NULL);

/*
 * Printer Scrolled List
 */
     ac = 0;
     XtSetArg(al[ac], XmNvisibleItemCount, 3); ac++; 
     XtSetArg(al[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 3); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->position_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++; 
     p->printer_list = XmCreateScrolledList (form, "PrinterList", al, ac );
     SetPrinterList (p->printer_list);
/*
 * Page Size List
 */
     ac = 0;
     XtSetArg(al[ac], XmNvisibleItemCount, 3); ac++;
     XtSetArg(al[ac], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); ac++;
     XtSetArg(al[ac], XmNitemCount, 6); ac++;
     list_items = (XmString *) XtMalloc ( sizeof (XmString) * 7 );
     list_items[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 182,
						      "8.5 x 11 in (Letter)"));
     list_items[1] = XmStringCreateLocalized (catgets (prog->catd, 1, 183,
						      "8.5 x 14 in (Legal)"));
     list_items[2] = XmStringCreateLocalized (catgets (prog->catd, 1, 184,
						      "11 x 17 in (Ledger)"));
     list_items[3] = XmStringCreateLocalized (catgets (prog->catd, 1, 185,
						      "29.7 x 42 cm (A3)"));
     list_items[4] = XmStringCreateLocalized (catgets (prog->catd, 1, 186,
						      "21 x 29.7 cm (A4)"));
     list_items[5] = XmStringCreateLocalized (catgets (prog->catd, 1, 187,
						      "17.6 x 25 cm (B5)"));
     XtSetArg(al[ac], XmNitems, list_items); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 8); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->size_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     p->size_list = XmCreateScrolledList (form, "PageSizeList", al, ac );
     
     for (list_item = 0; list_item < 6; list_item++ )
       XmStringFree (list_items [list_item]);
     XtFree ( (char *) list_items ); 
/*
 * Select first item on list.
 */
     XmListSelectPos (p->size_list, 1, False);
/*
 * Page Range Radio Box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 13); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->range_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->range_radio = XmCreateRadioBox (form, "PageRangeRadioBox", al, ac );
/*
 * Page Range Radio Toggle Buttons
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 188,
						      "This Page (As Image)"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNset, False); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->this_page_toggle = XmCreateToggleButtonGadget (p->range_radio, 
						    "ThisPageToggle", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 189,
					   "All Original Document, No Edits"));
     XtSetArg(al[ac], XmNset, True); ac++;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->orig_toggle = XmCreateToggleButtonGadget (p->range_radio, 
						  "AllOrigToggle", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Image Size Spin Box
 */
     ac = 0;
     XtSetArg(al[ac], DtNspinBoxChildType, DtNUMERIC); ac++;
     XtSetArg(al[ac], DtNmaximumValue, 400); ac++;
     XtSetArg(al[ac], DtNminimumValue, 25); ac++;
     XtSetArg(al[ac], DtNposition, 100); ac++; 
     XtSetArg(al[ac], XmNshadowThickness, 0); ac++;
     XtSetArg(al[ac], XmNcolumns, 4); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 18); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->image_size_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->image_size_value = DtCreateSpinBox (form, "ImageSizeSpinBox", al, ac);
     XtVaGetValues (p->image_size_value, DtNtextField, &p->image_size_text, NULL);
/*
 * Create Percent Label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 190,
						      "%"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 18); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->image_size_value); ac++;
     p->percent_label = XmCreateLabel ( form, "PercentLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Image Size Slider
 */
     ac = 0;
     XtSetArg(al[ac], XmNshowValue, TRUE); ac++;
     XtSetArg(al[ac], XmNminimum, 25); ac++;
     XtSetArg(al[ac], XmNmaximum, 400); ac++;
     XtSetArg(al[ac], XmNscaleMultiple, 1); ac++;
     XtSetArg(al[ac], XmNvalue, 100); ac++;
     XtSetArg(al[ac], XmNscaleWidth, 160); ac++;
     XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
     XtSetArg(al[ac], XmNprocessingDirection, XmMAX_ON_RIGHT); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->image_size_value); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->image_size_value); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     p->image_size_slider = XmCreateScale (form, "ImageSizeSlider", al, ac );
/*
 * Orientation Radio Box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 24); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->orientation_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->orientation_radio = XmCreateRadioBox (form, "OrientationRadio", al, ac );
/*
 * Orientation Toggle Buttons
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 191,
						      "Portrait"));
     XtSetArg(al[ac], XmNset, True); ac++;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->portrait_toggle = XmCreateToggleButtonGadget (p->orientation_radio, 
						    "PortraitToggle", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

     ac = 0;

     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 192,
						      "Landscape"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++; 
     XtSetArg(al[ac], XmNset, False); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->landscape_toggle = XmCreateToggleButtonGadget (p->orientation_radio, 
						   "LandscapeToggle", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Image Position Radio Box
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNtopPosition, 29); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->orientation_label); ac++;
     XtSetArg(al[ac], XmNleftOffset, 10); ac++;
     p->position_radio = XmCreateRadioBox (form, "PositionRadio", al, ac);
/*
 * Image Position Toggle Buttons
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 193,
						      "Centered"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNset, True); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->centered_toggle = XmCreateToggleButtonGadget (p->position_radio, 
						    "CenteredToggle", al, ac);
     XmStringFree ( xmstrings [ 0 ] );

     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 194,
						      "Margins:"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNset, False); ac++;
     XtSetArg(al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->margin_toggle = XmCreateToggleButtonGadget (p->position_radio, 
						    "MarginToggle", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Text Fields for Top/Left Text
 */
     ac = 0; 
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->position_radio); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->orientation_radio); ac++;
     XtSetArg(al[ac], XmNleftOffset, 5); ac++;
     XtSetArg(al[ac], XmNcolumns, 4); ac++;
     XtSetArg(al[ac], XmNvalue, catgets (prog->catd, 1, 195, "1.0")); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++; 
     p->top_text = XmCreateTextField (form, "TopText", al, ac );

     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->top_text); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->top_text); ac++; 
     XtSetArg(al[ac], XmNcolumns, 4); ac++;
     XtSetArg(al[ac], XmNvalue, catgets (prog->catd, 1, 196, "1.0")); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     p->left_text = XmCreateTextField (form, "LeftText", al, ac );
/*
 * Create Inches from top label
 */
     ac = 0;     
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 197, 
					     "inches from top"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->top_text); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->position_radio); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     p->top_inches_label = XmCreateLabel (form, "TopInchesLabel", al, ac);
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Inches from left label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 198,
					     "inches from left"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->left_text); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->top_text); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     p->left_inches_label = XmCreateLabel (form, "LeftInchesLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create cm from top label
 */
     ac = 0;     
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 199, 
					     "cm from top"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->top_text); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->position_radio); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     p->top_cm_label = XmCreateLabel (form, "TopInchesLabel", al, ac);
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create cm from left label
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 200,
					     "cm from left"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, p->left_text); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, p->top_text); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     p->left_cm_label = XmCreateLabel (form, "LeftInchesLabel", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Inches/CM pulldown menu.
 */
     ac = 0;
     XtSetArg (al[ac], XmNpacking, XmPACK_COLUMN); ac++;
     XtSetArg (al[ac], XmNorientation, XmHORIZONTAL); ac++;
     XtSetArg (al[ac], XmNnumColumns, 2); ac++;
     menu = XmCreatePulldownMenu (form, "PositionPulldown", al, ac);

     ac = 0;
     XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
     XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg (al[ac], XmNleftWidget, p->position_radio); ac++;
     XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg (al[ac], XmNbottomWidget, p->position_radio); ac++;  
     XtSetArg (al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     p->units_menu = XmCreateOptionMenu (form, "PositionOptionMenu", al, ac);
     XtVaSetValues (XmOptionButtonGadget (p->units_menu), 
		    XmNsensitive, False, 
		    XmNuserData, "in", NULL);

     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 201, "in"));
     xmstrings[1] = XmStringCreateLocalized (catgets (prog->catd, 1, 202, "cm"));
     p->in_item = XtVaCreateWidget ("InchesItem", 
				     xmPushButtonGadgetClass, menu,
				     XmNlabelString, xmstrings[0],
				     NULL);
     p->cm_item = XtVaCreateWidget ("CMItem", 
				     xmPushButtonGadgetClass, menu,
				     XmNlabelString, xmstrings[1],
				     NULL);
     XmStringFree ( xmstrings [ 0 ] );
     XmStringFree ( xmstrings [ 1 ] );

/*
 * Adjust the labels to align with toggle button, not radio boxes.
 */	
     XtVaGetValues (p->range_label, XmNheight, &height, NULL);
     if (height >= 21) {
       XtVaSetValues (p->copies_label, XmNtopOffset, (int) (height / 5), NULL);
       XtVaSetValues (p->header_label, XmNtopOffset, (int) (height / 5), NULL);
       XtVaSetValues (p->range_label, XmNtopOffset, (int) (height / 5), NULL);
       XtVaSetValues (p->image_size_label, XmNtopOffset, (int) (height / 5), NULL);
       XtVaSetValues (p->orientation_label, XmNtopOffset, (int) (height / 5), NULL);
       XtVaSetValues (p->position_label, XmNtopOffset, (int) (height / 5), NULL);
     }
     else {
       XtVaSetValues (p->copies_label, XmNtopOffset, (int) (height / 4), NULL);
       XtVaSetValues (p->header_label, XmNtopOffset, (int) (height / 4), NULL);
       XtVaSetValues (p->range_label, XmNtopOffset, (int) (height / 4), NULL);
       XtVaSetValues (p->image_size_label, XmNtopOffset, (int) (height / 4), NULL);
       XtVaSetValues (p->orientation_label, XmNtopOffset, (int) (height / 4), NULL);
       XtVaSetValues (p->position_label, XmNtopOffset, (int) (height / 4), NULL);
     }
	
/*
 * Create Print Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 203, 
                                                      "Print"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->print_button = XmCreatePushButton (p->dialog, "PrintButton", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Cancel Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 230, 
                                                      "Cancel"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->cancel_button = XmCreatePushButton (p->dialog, "PrintCancelButton", 
                                            al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create Help Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 205, 
                                                      "Help"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     p->help_button = XmCreatePushButton ( p->dialog, "PrintButton", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Set Print to be default button
 * Taking this out because hitting return causes print button to activate.
 */
     XtVaSetValues (p->dialog, XmNdefaultButton, p->print_button, NULL);

     p->showing = False;

/*
 * Set help callback on dialog
 */
     XtAddCallback (p->dialog, XmNhelpCallback, HelpItemCallback, 
		    HELP_PRINT_ITEM);
/*
 * Set callbacks on buttons
 */ 
     XtAddCallback (p->copies_text, XmNmodifyVerifyCallback, 
		    CopiesTextCallback, p);

     XtAddCallback (p->image_size_text, XmNmodifyVerifyCallback, 
		    ImageSizeTextCallback, p);
     XtAddCallback (p->image_size_text, XmNvalueChangedCallback, 
		    ImageSizeTextCallback, p);
     XtAddCallback (p->image_size_text, XmNactivateCallback, 
		    ImageSizeTextCallback, p);

     XtAddCallback (p->image_size_slider, XmNdragCallback, 
		    ImageSizeSliderCallback, p);
     XtAddCallback (p->image_size_slider, XmNvalueChangedCallback, 
		    ImageSizeSliderCallback, p);

     XtAddCallback (p->this_page_toggle, XmNvalueChangedCallback, 
		    PageRangeCallback, p);

     XtAddCallback (p->portrait_toggle, XmNvalueChangedCallback, 
		    ImageOrientationCallback, PORTRAIT);

     XtAddCallback (p->centered_toggle, XmNvalueChangedCallback, 
		    ImagePositionCallback, p);

     XtAddCallback (p->in_item, XmNactivateCallback, 
		    PrintUnitsCallback, INCHES);
     XtAddCallback (p->cm_item, XmNactivateCallback, 
		    PrintUnitsCallback, (XtPointer)CM);

     XtAddCallback (p->top_text, XmNmodifyVerifyCallback, 
		    MarginTextCallback, p);
     XtAddCallback (p->top_text, XmNactivateCallback, 
		    MarginTextCallback, p);

     XtAddCallback (p->left_text, XmNmodifyVerifyCallback, 
		    MarginTextCallback, p);
     XtAddCallback (p->left_text, XmNactivateCallback, 
		    MarginTextCallback, p);

     XtAddCallback (p->print_button, XmNactivateCallback, 
		    PrintButtonCallback, p);
     XtAddCallback (p->cancel_button, XmNactivateCallback, 
		    PrintCancelCallback, p);
     XtAddCallback (p->help_button, XmNactivateCallback, HelpItemCallback, 
		    HELP_PRINT_ITEM);
/*
 * Manage the children
 */
     XtManageChild (p->printer_list);
     XtManageChild (p->size_list);

     ac = 0;
     children[ac++] = p->in_item;
     children[ac++] = p->cm_item;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = p->this_page_toggle;
     children[ac++] = p->orig_toggle;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = p->portrait_toggle;
     children[ac++] = p->landscape_toggle;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = p->centered_toggle;
     children[ac++] = p->margin_toggle;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = p->copies_label;
     children[ac++] = p->printer_label;
     children[ac++] = p->size_label;
     children[ac++] = p->range_label;
     children[ac++] = p->image_size_label;
     children[ac++] = p->orientation_label;
     children[ac++] = p->position_label;
     children[ac++] = p->copies_value;
     children[ac++] = p->header_label;
     children[ac++] = p->header_checkbox;
     children[ac++] = p->range_radio;
     children[ac++] = p->image_size_value;
     children[ac++] = p->percent_label;
     children[ac++] = p->image_size_slider;
     children[ac++] = p->orientation_radio;
     children[ac++] = p->position_radio;
     children[ac++] = p->top_text;
     children[ac++] = p->left_text;
     children[ac++] = p->top_inches_label;
     children[ac++] = p->left_inches_label;
     children[ac++] = p->units_menu;
     XtManageChildren(children, ac);
     ac = 0;
     children[ac++] = form;
     children[ac++] = p->print_button;
     children[ac++] = p->cancel_button;
     children[ac++] = p->help_button;
     XtManageChildren(children, ac);

     return p;
}