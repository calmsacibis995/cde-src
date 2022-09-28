#ifndef lint
static char sccsid[] = "@(#)ui_imagetool.c 1.53 97/04/25";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * ui_imagetool.c - User interface object initialization functions.
 */

#include <sys/param.h>
#include <dlfcn.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SeparatoG.h>
#include <Dt/Dnd.h>
#include <Dt/Icon.h>
#include <Dt/IconFile.h>
#include <xil/xil.h>

#include "DrawingAVis.h"
#include "display.h"
#include "image.h"
#include "misc.h"
#include "props.h"
#include "imagetool.h"
#include "ui_imagetool.h"
#include "help.h"

#define	DPS_EXTENSION	"Adobe-DPS-Extension"
#define DRAG_ICON_NAME  "SDtimage.DND"

Pixmap stipple_pixmap = NULL;

/*
 * Initialize an instance of object `file_menu'.
 */
Widget
FileMenuCreate(b, parent)
     BaseObjects  	*b;
     Widget		 parent;
{
     register int ac = 0;
     Arg          al[64];
     Atom         delete_window;
     XmString     xmstrings[16];
     Widget       children[16];
     Widget       sep1_item;
     Widget       sep2_item;
     Widget       sep3_item;
     Widget       sep4_item;
     Widget       sep5_item;

     extern void  OpenCallback();
     extern void  OpenAsCallback();
     extern void  SaveCallback();
     extern void  SaveAsCallback();
     extern void  PrintOneCallback();
     extern void  PrintPrevCallback();
     extern void  PrintCallback();
     extern void  ExitCallback();
     extern void  PopupSnapshotCallback();
/*
 * Create the File Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized(catgets (prog->catd, 1, 354, 
						     "File"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->file_button = XmCreateCascadeButton ( parent, "FileButton", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * Create the File Menu
 */
     ac = 0;
     b->file_menu = XmCreatePulldownMenu (parent, "FileMenu", al, ac);

/*
 * Create the File Menu Items
 *
 * "Open..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized(catgets (prog->catd, 1, 355, 
						     "Open..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->open_item = XmCreatePushButton (b->file_menu, "OpenItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Open As..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized(catgets (prog->catd, 1, 356, 
						     "Open As..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->openas_item = XmCreatePushButton (b->file_menu, "OpenAsItem", 
					  al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     sep1_item = XmCreateSeparatorGadget (b->file_menu, "FileSep1", al, ac);
/*
 * "Save..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 357, 
						      "Save..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->save_item = XmCreatePushButton (b->file_menu, "SaveItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Save As..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 358, 
						      "Save As..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->saveas_item = XmCreatePushButton (b->file_menu, "SaveAsItem", 
					  al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Save Selection As..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 359, 
						      "Save Selection As..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->savesel_item = XmCreatePushButton (b->file_menu, "SaveSelItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Save Page As Image..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 360, 
						      "Save Page As Image..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->savepage_item = XmCreatePushButton (b->file_menu, "SavePageItem", 
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * Separator
 */
     ac = 0;
     sep2_item = XmCreateSeparatorGadget (b->file_menu, "FileSep2", al, ac);

/*
 * "Snapshot..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 451, 
						      "Snapshot..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->snap_item = XmCreatePushButton (b->file_menu, "SnapshotItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

/* 
 * Separator
 */
     ac = 0;
     sep3_item = XmCreateSeparatorGadget (b->file_menu, "FileSep3", al, ac);
/* 
 * "Print One..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 361, 
						      "Print One"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->printone_item = XmCreatePushButton (b->file_menu, "PrintOneItem",
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Print Preview..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 362, 
						      "Print Preview..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->printprev_item = XmCreatePushButton (b->file_menu, "PrintPrevItem",
					     al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * "Print..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 363, 
						      "Print..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->print_item = XmCreatePushButton (b->file_menu, "PrintItem", al, ac);
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * Separator
 */
     ac = 0;
     sep4_item = XmCreateSeparatorGadget (b->file_menu, "FileSep4", al, ac);
/*
 * "Options..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 364, 
						      "Options..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->props_item = XmCreatePushButton (b->file_menu, "OptionsItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

/* 
 * Separator
 */
     ac = 0;
     sep5_item = XmCreateSeparatorGadget (b->file_menu, "FileSep5", al, ac);

/* 
 * "Exit"
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 365, 
						      "Exit"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->exit_item = XmCreatePushButton (b->file_menu, "ExitItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );

/*
 * Set Callbacks
 */
     XtAddCallback (b->open_item, XmNactivateCallback, OpenCallback, b);
     XtAddCallback (b->openas_item, XmNactivateCallback, OpenAsCallback, b);
     XtAddCallback (b->save_item, XmNactivateCallback, SaveCallback, b);
     XtAddCallback (b->saveas_item, XmNactivateCallback, SaveAsCallback, 
		    (XtPointer)SAVEAS);
     XtAddCallback (b->savesel_item, XmNactivateCallback, SaveAsCallback,
		    (XtPointer)SAVESELECTIONAS);
     XtAddCallback (b->savepage_item, XmNactivateCallback, SaveAsCallback,
		    (XtPointer)SAVEPAGEASIMAGE);
     XtAddCallback (b->printone_item, XmNactivateCallback, PrintOneCallback,
		    b);
     XtAddCallback (b->printprev_item, XmNactivateCallback, 
		    PrintPrevCallback, b);
     XtAddCallback (b->print_item, XmNactivateCallback, PrintCallback, b);
     XtAddCallback (b->snap_item, XmNactivateCallback, 
		    PopupSnapshotCallback, b);
     XtAddCallback (b->exit_item, XmNactivateCallback, ExitCallback, b);
/*
 * Set Quit callback if quitting from frame menu.
 */
     delete_window = XmInternAtom (XtDisplay (b->top_level), 
				   "WM_DELETE_WINDOW", False);
     XmAddWMProtocolCallback (b->top_level, delete_window, ExitCallback, b);
/*
 * Set Help Callbacks
 */
     XtAddCallback (b->file_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_FILE_MENU);
     XtAddCallback (b->file_menu, XmNhelpCallback, HelpItemCallback, 
		    HELP_FILE_MENU);
     XtAddCallback (b->open_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_OPEN_ITEM);
     XtAddCallback (b->openas_item, XmNhelpCallback, HelpItemCallback, 
		       HELP_OPENAS_ITEM);
     XtAddCallback (b->save_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_SAVE_ITEM);
     XtAddCallback (b->saveas_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_SAVEAS_ITEM);
     XtAddCallback (b->savesel_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_SAVESEL_ITEM);
     XtAddCallback (b->savepage_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_SAVEPAGE_ITEM);
     XtAddCallback (b->printone_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_PRINTONE_ITEM);
     XtAddCallback (b->printprev_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_PRINTPREV_ITEM);
     XtAddCallback (b->print_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_PRINT_ITEM);
     XtAddCallback (b->exit_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_EXIT_ITEM);
/*
 * Manage Children
 */
     ac = 0;
     children[ac++] = b->open_item;
     children[ac++] = b->openas_item;
     children[ac++] = sep1_item;
     children[ac++] = b->save_item;
     children[ac++] = b->saveas_item;
     children[ac++] = b->savesel_item;
     children[ac++] = b->savepage_item;
     children[ac++] = sep2_item;
     children[ac++] = b->printone_item;
     children[ac++] = b->printprev_item;
     children[ac++] = b->print_item;
     children[ac++] = sep3_item;
     children[ac++] = b->props_item;
     children[ac++] = sep4_item;
     children[ac++] = b->snap_item;
     children[ac++] = sep5_item;
     children[ac++] = b->exit_item;
     XtManageChildren(children, ac);
     ac = 0;
     XtSetArg(al[ac], XmNsubMenuId, b->file_menu); ac++;
     XtSetValues ( b->file_button, al, ac );
     
     return b->file_button;
}


/*
 * Initialize an instance of object `view_menu'.
 */
Widget
ViewMenuCreate(b, parent)
     BaseObjects  	*b;
     Widget		 parent;
{
     register int ac = 0;
     Arg          al[64];
     XmString     xmstrings[16];
     Widget       children[16];
     Widget       separator;

     extern void  ImageInfoCallback();
     extern void  PgOverviewCallback();
     extern void  PgControlsCallback();
/*
 * Create the View Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 366, 
						      "View"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->view_button = XmCreateCascadeButton ( parent, "ViewButton", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * Create the View Menu
 */
     ac = 0;
     b->view_menu = XmCreatePulldownMenu ( parent, "ViewMenu", al, ac );
/*
 * Create the View Menu Items
 *
 * "Image Info..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 367, 
						      "Image Info..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->imageinfo_item = XmCreatePushButton (b->view_menu, "ImageInfoItem", 
					     al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     separator = XmCreateSeparatorGadget (b->view_menu, "ViewSep1", al, ac );
/*
 * "Page Overview..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 368, 
						      "Page Overview..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->overview_item = XmCreatePushButton (b->view_menu, "OverviewItem", 
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/* 
 * "Page Viewing Controls..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 369, 
					     "Page Viewing Controls..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->controls_item = XmCreatePushButton (b->view_menu, "ControlsItem",
					    al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Add Callbacks
 */
     XtAddCallback (b->imageinfo_item, XmNactivateCallback, ImageInfoCallback, 
		    b);
     XtAddCallback (b->overview_item, XmNactivateCallback, PgOverviewCallback, 
		    b);
     XtAddCallback (b->controls_item, XmNactivateCallback, 
		    PgControlsCallback, b);

/*
 * Set help callbacks
 */
     XtAddCallback (b->view_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_VIEW_MENU);
     XtAddCallback (b->view_menu, XmNhelpCallback, HelpItemCallback, 
		    HELP_VIEW_MENU);
     XtAddCallback (b->imageinfo_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_IMAGEINFO_ITEM);
     XtAddCallback (b->overview_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_OVERVIEW_ITEM);
     XtAddCallback (b->controls_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_CONTROLS_ITEM);
/*
 * Manage the children
 */
     ac = 0;
     children[ac++] = b->imageinfo_item;
     children[ac++] = separator;
     children[ac++] = b->overview_item;
     children[ac++] = b->controls_item;
     XtManageChildren(children, ac);
     ac = 0;
     XtSetArg(al[ac], XmNsubMenuId, b->view_menu); ac++;
     XtSetValues ( b->view_button, al, ac );
     
     return b->view_button;

}

/*
 * Initialize an instance of object `edit_menu'.
 */
Widget
EditMenuCreate (b, parent)
     BaseObjects  	*b;
     Widget		 parent;
{
     register int ac = 0;
     Arg          al[64];
     XmString     xmstrings[16];
     Widget       children[16];
     Widget       separator;
     
     extern void  UndoCallback();
     extern void  PaletteCallback();
     extern void  PropsCallback();

/*
 * Create the Edit Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 370, 
						      "Edit"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     b->edit_button = XmCreateCascadeButton (parent, "EditButton", al, ac);
/*
 * Create the Edit Menu 
 */
     ac = 0;
     XmStringFree ( xmstrings [ 0 ] );
     b->edit_menu = XmCreatePulldownMenu ( parent, "EditMenu", al, ac );
/*
 * Create the Edit Menu Items
 *
 * "Undo"
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 371, 
						      "Undo"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->undo_item = XmCreatePushButton (b->edit_menu, "UndoItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     separator = XmCreateSeparatorGadget (b->edit_menu, "EditSep1", al, ac );
/*
 * "Palette..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 372, 
						      "Palette..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     b->palette_item = XmCreatePushButton (b->edit_menu, "PaletteItem", 
					   al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Set Callbacks
 */
     XtAddCallback (b->undo_item, XmNactivateCallback, UndoCallback, b);
     XtAddCallback (b->palette_item, XmNactivateCallback, PaletteCallback, b);
     XtAddCallback (b->props_item, XmNactivateCallback, PropsCallback, b);
/*
 * Set help callbacks
 */
     XtAddCallback (b->edit_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_EDIT_MENU);
     XtAddCallback (b->edit_menu, XmNhelpCallback, HelpItemCallback, 
		    HELP_EDIT_MENU);
     XtAddCallback (b->undo_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_UNDO_ITEM);
     XtAddCallback (b->palette_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_PALETTE_ITEM);
     XtAddCallback (b->props_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_PROPS_ITEM);
/*
 * Manage the children
 */
     ac = 0;
     children[ac++] = b->undo_item;
     children[ac++] = separator;
     children[ac++] = b->palette_item;
     XtManageChildren(children, ac);
     ac = 0;
     XtSetArg(al[ac], XmNsubMenuId, b->edit_menu); ac++;
     XtSetValues ( b->edit_button, al, ac );
     
     return b->edit_button;

}

/*
 * Initialize an instance of object `help_menu'.
 */
Widget
HelpMenuCreate(ip, parent)
     BaseObjects  	*ip;
     Widget		 parent;
{
     register int ac = 0;
     Arg          al[64];
     XmString     xmstrings[16];
     Widget       children[16];
     Widget       help_button;
     Widget       help_menu;
     Widget       overview_item;
     Widget       sep1_item;
     Widget       tasks_item;
     Widget       ref_item;
     Widget       on_item;
     Widget       sep2_item;
     Widget       using_item;
     Widget       sep3_item;
     Widget       about_item;
     extern void  HelpOnItemCallback();
     extern void  HelpMenuCallback();
     extern void  HelpAboutCallback();

/*
 * Create the Help Button
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 373, 
						      "Help"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     help_button = XmCreateCascadeButton ( parent, "HelpButton", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Create the Help Menu 
 */
     ac = 0;
     help_menu = XmCreatePulldownMenu ( parent, "HelpMenu", al, ac );
/*
 * Create the Help Menu Items
 *
 * "Overview..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 374, 
						      "Overview..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     overview_item = XmCreatePushButton ( help_menu, "OverviewItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     sep1_item = XmCreateSeparatorGadget ( help_menu, "HelpSep1", al, ac );
/*
 * "Tasks..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 375, 
						      "Tasks..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     tasks_item = XmCreatePushButton ( help_menu, "TasksItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "Reference..."
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 376, 
						      "Reference..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     ref_item = XmCreatePushButton ( help_menu, "RefItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * "On Item"
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 377, 
						      "On Item"));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     on_item = XmCreatePushButton ( help_menu, "OnItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     sep2_item = XmCreateSeparatorGadget ( help_menu, "HelpSep3", al, ac );
/*
 * "Using Help..."
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 378, 
						      "Using Help..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     using_item = XmCreatePushButton ( help_menu, "UsingHelpItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Separator
 */
     ac = 0;
     sep3_item = XmCreateSeparatorGadget ( help_menu, "HelpSep3", al, ac );
/*
 * "About Image Viewer..."
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 379, 
						      "About Image Viewer..."));
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     about_item = XmCreatePushButton ( help_menu, "AboutItem", al, ac );
     XmStringFree ( xmstrings [ 0 ] );
/*
 * Set the help button to be the help widget
 */
     ac = 0;
     XtSetArg(al[ac], XmNmenuHelpWidget, help_button); ac++;
     XtSetValues ( parent, al, ac );

/*
 * Set callbacks on menu items
 */
     XtAddCallback (overview_item, XmNactivateCallback,
		    HelpMenuCallback, (XtPointer)HELP_OVERVIEW_EVENT);
     XtAddCallback (tasks_item, XmNactivateCallback,
		    HelpMenuCallback, (XtPointer)HELP_TASKS_EVENT);
     XtAddCallback (ref_item, XmNactivateCallback,
		    HelpMenuCallback, (XtPointer)HELP_REF_EVENT);
     XtAddCallback (on_item, XmNactivateCallback,
		    HelpOnItemCallback, (XtPointer)HELP_ONITEM_EVENT);
     XtAddCallback (using_item, XmNactivateCallback,
		    HelpMenuCallback, (XtPointer)HELP_USING_HELP_EVENT);
     XtAddCallback (about_item, XmNactivateCallback,
		    HelpAboutCallback, NULL);
/*
 * Set help callbacks
 */
     XtAddCallback (help_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_HELP_MENU);
     XtAddCallback (help_menu, XmNhelpCallback, HelpItemCallback, 
		    HELP_HELP_MENU);
     XtAddCallback (overview_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_HELP_OVERVIEW_ITEM);
     XtAddCallback (tasks_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_TASKS_ITEM);
     XtAddCallback (ref_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_REF_ITEM);
     XtAddCallback (on_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_ON_ITEM);
     XtAddCallback (using_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_USING_HELP_ITEM);
     XtAddCallback (about_item, XmNhelpCallback, HelpItemCallback, 
		    HELP_ABOUT_ITEM);
     
/*
 * Manage the children
 */
     ac = 0;
     children[ac++] = overview_item;
     children[ac++] = sep1_item;
     children[ac++] = tasks_item;
     children[ac++] = ref_item;
     children[ac++] = on_item;
     children[ac++] = sep2_item;
     children[ac++] = using_item;
     children[ac++] = sep3_item;
     children[ac++] = about_item;
     XtManageChildren(children, ac);
     ac = 0;
     XtSetArg(al[ac], XmNsubMenuId, help_menu); ac++;
     XtSetValues ( help_button, al, ac );
     
     return help_button;

}


static void
CreateApplicationIcon (top_level, fg, bg)
     Widget top_level;
     Pixel  fg, bg;
{
     register int    ac = 0;
     Arg             al[64];
     char            actual_filename[MAXPATHLEN];
     char           *icon_filename;
     static Pixmap   it_pixmap = XmUNSPECIFIED_PIXMAP;       /* icon pixmap */
     static Pixmap   it_pixmap_mask = XmUNSPECIFIED_PIXMAP;  /* icon pixmap */

     icon_filename = XmGetIconFileName (XtScreen (top_level), NULL,
				        IMAGETOOL_ICON, NULL, DtLARGE);
     actual_filename[0] = '\0';
     if (icon_filename) {
/*
 * Use bitmap if depth == 1
 */
       if (DefaultDepthOfScreen (XtScreen (top_level)) == 1) {
	 int len = strlen (icon_filename);
         if (len > 3 && 
	     icon_filename [len-1] == 'm' && icon_filename [len-2] == 'p' &&
	     icon_filename [len-3] == '.') {
	   icon_filename [len-2] = 'b';
	   if (access (icon_filename, R_OK) != 0)
	     icon_filename[len-2] = 'p';  /* put it back if not found */
	 }
       }
       sprintf (actual_filename, "%s", icon_filename);
       XtFree (icon_filename);
     }
     it_pixmap = XmGetPixmap (XtScreen (top_level), actual_filename, 
			      fg, bg);
     it_pixmap_mask = _DtGetMask (XtScreen (top_level), actual_filename);
     
     ac = 0;
     if (it_pixmap != XmUNSPECIFIED_PIXMAP) {
       XtSetArg(al[ac], XmNiconPixmap, it_pixmap); 
       ac++;
     }
     if (it_pixmap_mask != XmUNSPECIFIED_PIXMAP) {
       XtSetArg(al[ac], XmNiconMask, it_pixmap_mask); 
       ac++;
     }
     XtSetValues (top_level, al, ac );

}

static void
CreateDndIcon (b, fg, bg)
     BaseObjects *b;
     Pixel        fg,bg;
{
     register int    ac = 0;
     Arg             al[64];
     char           *icon_filename;
     char            actual_filename[MAXPATHLEN];
     static Pixmap   drag_pixmap = XmUNSPECIFIED_PIXMAP;
     XmString        xmstring;
     extern void     DndEventHandler ();

     icon_filename = XmGetIconFileName (XtScreen (b->top_level), NULL,
				        DRAG_ICON_NAME, NULL, DtMEDIUM);
     actual_filename[0] = '\0';
     if (icon_filename) {
       sprintf (actual_filename, "%s", icon_filename);
       free (icon_filename);
     }
     
     drag_pixmap = XmGetPixmap (XtScreen (b->top_level), actual_filename,
				fg, bg);
     ac = 0;
     xmstring = XmStringCreateLocalized (catgets (prog->catd, 1, 380, 
						  "Drag Image"));
     XtSetArg (al[ac], XmNpixmapPosition, XmPIXMAP_TOP); ac++;
     XtSetArg (al[ac], XmNstringPosition, XmSTRING_BOTTOM); ac++;
     XtSetArg (al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
     XtSetArg (al[ac], XmNstring, xmstring); ac++;
     XtSetArg (al[ac], XmNtraversalOn, False); ac++;
     if (drag_pixmap) {
       XtSetArg (al[ac], XmNpixmap, drag_pixmap); 
       ac++;
     }
     b->drag_icon = XtCreateWidget ("DragSourceIcon", dtIconGadgetClass,
				     b->toolbar, al, ac);
     XtAddEventHandler (XtParent (b->drag_icon), Button1MotionMask, False,
			DndEventHandler, (XtPointer)DtDND_BUFFER_TRANSFER);
}

/*
 * Initialize an instance of object `base'.
 */
BaseObjects *
BaseObjectsInitialize(ip, parent)
     BaseObjects  	*ip;
     Widget		 parent;
{
     register int    ac = 0;
     Arg             al[64];
     Pixel           fg, bg;           /* colour values for pixmaps */
     char            label[256];
     Widget          children[16];
     XmString        xmstrings[4];
     Dimension       width1, width2, width3, width4, width5;
     Dimension       height1, height2;
     Dimension       min_width, min_width2, min_height;
     Dimension       max_width, max_height;
     Atom            xa_WM_SAVE_YOURSELF;
     Widget          toolbar_frame;
     static Pixmap   pgfwd_pixmap, pgfwd_insen_pixmap, 
		     pgback_pixmap, pgback_insen_pixmap;

     extern void     SaveSessionCallback ();
     extern void     MainWEventHandler ();
     extern void     KeyEventHandler ();
     extern void     PropsKeyEventHandler ();
     extern void     FormEventHandler ();
     extern void     ScrollBarCallback ();
     extern void     PgfwdButtonCallback ();
     extern void     PgbackButtonCallback ();
     extern void     PageSliderCallback ();
     extern void     DndTransferCallback ();

     static XtCallbackRec transfer_rec [] = {{DndTransferCallback, NULL},
                                             {NULL, NULL}};

/*
 * Set up the Handlers for saving the session information.
 */
     xa_WM_SAVE_YOURSELF = XInternAtom (XtDisplay (ip->top_level), 
					"WM_SAVE_YOURSELF", False);
     XmAddWMProtocolCallback (ip->top_level, xa_WM_SAVE_YOURSELF,
			      SaveSessionCallback, NULL);
/*
 * Must call this to cause Motif to recognise tear_off_enabled in a
 * resource file.
 */
     XmRepTypeInstallTearOffModelConverter();

/*
 * Check if any command line options were
 * specified that pertain to the main window.
 */
     if (appresources->title != NULL) 
       XtVaSetValues (ip->top_level, XmNtitle, appresources->title, NULL);
     else {
       sprintf (label, "%s %s", catgets (prog->catd, 1, 433, "Image Viewer"), prog->rel);
       XtVaSetValues (ip->top_level, XmNtitle, label, NULL);
     }
     if (appresources->icon_name != NULL)
       XtVaSetValues (ip->top_level, XmNiconName, appresources->icon_name, NULL);

/* 
 * Create the Main Window
 */
     ac = 0;
     ip->mainw = XtVaCreateWidget ("MainWindow",
				   xmMainWindowWidgetClass, parent,
				   NULL);
     if (ip->mainw == NULL)
       OutOfMemory();

     XtAddEventHandler (ip->mainw, 
			StructureNotifyMask | SubstructureNotifyMask, False,
			MainWEventHandler, ip); 


     ps_display = (DisplayInfo *) calloc (1, sizeof (DisplayInfo));
     image_display = (DisplayInfo *) calloc (1, sizeof (DisplayInfo));
     image_display->xdisplay = XtDisplay (ip->top_level);

/*
 * Set Max size of frame.
 * Need to add in height of title bar and borders (hard-coded, yuk!)
 */
     max_width = DisplayWidth (image_display->xdisplay, DefaultScreen (image_display->xdisplay)) - 30;
     max_height = DisplayHeight (image_display->xdisplay, DefaultScreen (image_display->xdisplay)) - 30;
     XtVaSetValues (ip->top_level, XmNmaxWidth, max_width,
		                   XmNmaxHeight, max_height, NULL);
/*
 * Create the Base Form
 */ 
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNfractionBase, 100); ac++;
     XtSetArg(al[ac], XmNborderWidth, 0); ac++;
     ip->form = XmCreateForm ( ip->mainw, "BaseForm", al, ac );
     XtAddEventHandler (ip->form, StructureNotifyMask, False, FormEventHandler, ip);
     XtVaGetValues (ip->form, XmNforeground, &fg, 
		              XmNbackground, &bg, NULL);
/*
 * Create the Application Icon
 */
     CreateApplicationIcon (ip->top_level, fg, bg);

/*
 * Create the Menu Bar
 */
     ac = 0;
     ip->menu_bar = XmCreateMenuBar ( ip->form, "MenuBar", al, ac );

/*
 * Create the File, Edit, and View Menus
 */
     ip->file_button = FileMenuCreate (ip, ip->menu_bar);
     ip->edit_button = EditMenuCreate (ip, ip->menu_bar);
     ip->view_button = ViewMenuCreate (ip, ip->menu_bar);
     ip->help_button = HelpMenuCreate (ip, ip->menu_bar);

     /* Menu bar's children */
     ac = 0;
     children[ac++] = ip->file_button;
     children[ac++] = ip->edit_button;
     children[ac++] = ip->view_button;
     children[ac++] = ip->help_button;
     XtManageChildren(children, ac);
/*
 * Create the toolbar frame.
 */
     ac = 0;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     toolbar_frame = XmCreateFrame (ip->form, "ToolbarFrame", al, ac);
/*
 * Create the toolbar form
 */
     ac = 0;
     XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
     XtSetArg(al[ac], XmNfractionBase, 100); ac++;
     XtSetArg(al[ac], XmNborderWidth, 0); ac++;
     ip->toolbar = XmCreateForm (toolbar_frame, "ToolBarForm", al, ac );
     XtAddEventHandler (ip->toolbar, KeyPressMask, False, 
			KeyEventHandler, ip);
/*
 * Create a label for Go To Page Scale
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 381,
					     "Page"));  
     ac = 0;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     ip->goto_label = XmCreateLabel (ip->toolbar, "PageLabel", al, ac);
     XmStringFree (xmstrings[0]);
/*
 * Create the Go To Page Scale
 */
     ac = 0;
     XtSetArg(al[ac], XmNminimum, 1); ac++;
     XtSetArg(al[ac], XmNmaximum, 100); ac++;
     XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
     XtSetArg(al[ac], XmNscaleMultiple, 1); ac++;
     XtSetArg(al[ac], XmNscaleWidth, 125); ac++;
     XtSetArg(al[ac], XmNvalue, 1); ac++;
     XtSetArg(al[ac], XmNshowValue, False); ac++;
     ip->goto_scale = XmCreateScale (ip->toolbar, "PageScale", al, ac );
/*
 * Create a label for Go To page number
 */
     xmstrings[0] = XmStringCreateLocalized (catgets (prog->catd, 1, 382,
					     "1"));  
     ac = 0;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetArg(al[ac], XmNsensitive, False); ac++;
     ip->goto_pgno = XmCreateLabel (ip->toolbar, "PageNumber", al, ac);
     XmStringFree (xmstrings[0]);
/*
 * Create Drag Source icon
 */
#ifdef DRAG_ICON
     CreateDndIcon (ip, fg, bg);
#endif
/*
 * Create Page Back/Fwd Buttons
 */
     if (DefaultDepthOfScreen (XtScreen (ip->top_level)) > 1) {
       pgfwd_pixmap = XmGetPixmap (XtScreen (ip->toolbar), "SDtimage.pgR.s.pm", fg, bg);
       pgfwd_insen_pixmap = XmGetPixmap (XtScreen (ip->toolbar), "SDtimage.pgRdim.s.pm", fg, bg);
       pgback_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgL.s.pm", fg, bg);
       pgback_insen_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgLdim.s.pm", fg, bg);
     }
     else {	
       pgfwd_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgR.s.bm", fg, bg);
       pgfwd_insen_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgRdim.s.bm", fg, bg);
       pgback_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgL.s.bm", fg, bg);
       pgback_insen_pixmap = XmGetPixmap (XtScreen (ip->top_level), "SDtimage.pgLdim.s.bm", fg, bg);
     }		    

     ac = 0;
     XtSetArg (al[ac], XmNlabelType, XmPIXMAP); ac++;
     XtSetArg (al[ac], XmNlabelPixmap, pgfwd_pixmap); ac++;
     XtSetArg (al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     XtSetArg (al[ac], XmNlabelInsensitivePixmap, pgfwd_insen_pixmap); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++; 

     ip->pgfwd_button = XmCreatePushButton (ip->toolbar, "PgFwdButton", al, ac); 
     ac = 0;
     XtSetArg (al[ac], XmNlabelType, XmPIXMAP); ac++;
     XtSetArg (al[ac], XmNlabelPixmap, pgback_pixmap); ac++;
     XtSetArg (al[ac], XmNnavigationType, XmTAB_GROUP); ac++;
     XtSetArg (al[ac], XmNlabelInsensitivePixmap, pgback_insen_pixmap); ac++;
     XtSetArg (al[ac], XmNsensitive, False); ac++;
     ip->pgback_button = XmCreatePushButton (ip->toolbar, "PgBackButton", al, ac );

     /* toolbar's children */
     ac = 0;
     children[ac++] = ip->goto_label;
     children[ac++] = ip->goto_scale;
     children[ac++] = ip->goto_pgno;
     children[ac++] = ip->pgback_button;
     children[ac++] = ip->pgfwd_button;
#ifdef DRAG_ICON
     children[ac++] = ip->drag_icon; 
#endif
     XtManageChildren(children, ac);
     XtManageChild (ip->toolbar);


/*
 * Create an empty Scrolled Window.
 */
     ac = 0;
     XtSetArg(al[ac], XmNspacing, 0); ac++;
     XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC); ac++;
     XtSetArg(al[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED); ac++;
     ip->scrolledw = XmCreateScrolledWindow ( ip->form, "ScrolledW", al, ac );
     XtAddEventHandler (ip->scrolledw, KeyPressMask, False, 
			PropsKeyEventHandler, ip);

/*
 * Retrieve the handles to the scrollbars.
 */
/*
     ac = 0;
     XtSetArg(al[ac], XmNhorizontalScrollBar, &ip->hscroll ); ac++;
     XtSetArg(al[ac], XmNverticalScrollBar, &ip->vscroll ); ac++;
     XtGetValues(ip->scrolledw, al, ac );
*/
     ac = 0;
     XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
     XtSetArg(al[ac], XmNincrement, 15); ac++;
     ip->vscroll = XmCreateScrollBar (ip->scrolledw, "VerticalScrollbar", al, ac);

     ac = 0;
     XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
     XtSetArg(al[ac], XmNincrement, 15); ac++;
     ip->hscroll = XmCreateScrollBar (ip->scrolledw, "HorizontalScrollbar", al, ac);
     
/*
 * Create the footers.
 */
     ac = 0;
     ip->left_footer = XmCreateLabel (ip->form, "LeftFooter", al, ac);
     ip->right_footer = XmCreateLabel (ip->form, "RightFooter", al, ac);
     
/*
 * Position the Menu Bar at the top.
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNtopOffset, 0); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNleftOffset, 0); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetValues ( ip->menu_bar, al, ac );
/*
 * Position the Page Forward button to the right of the pgfwd button
 * and below the Menu Bar.
 */
#ifdef DRAG_ICON
     XtVaGetValues (ip->drag_icon, XmNheight, &height1, NULL);
#endif
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetValues ( ip->pgfwd_button, al, ac );
/*
 * Position the Page Back button to the far left below the Menu Bar .
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNrightWidget, ip->pgfwd_button); ac++;
     XtSetArg(al[ac], XmNrightOffset, 2); ac++; 
     XtSetValues ( ip->pgback_button, al, ac );
/*
 * Position drag icon to the left of the page back button.
 */
#ifdef DRAG_ICON
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     XtSetArg(al[ac], XmNtopOffset, 0); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
     XtSetArg(al[ac], XmNleftPosition, 50); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_NONE); ac++;
     XtSetValues ( ip->drag_icon, al, ac );
/*
 * Center drag icon
 */
     XtVaGetValues (ip->drag_icon, XmNwidth, &width1, NULL);
     XtVaSetValues (ip->drag_icon, XmNleftOffset, (int) width1 / -2, NULL);
#endif
/*
 * Position goto label to the far left.
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++; 
     XtSetArg(al[ac], XmNtopOffset, 2); ac++; 
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++; 
     XtSetArg(al[ac], XmNleftOffset, 2); ac++; 
    XtSetValues (ip->goto_label, al, ac );
/*
 * Position goto scale to the right of goto label.
 */
     XtVaGetValues (ip->goto_label, XmNheight, &height1, NULL);
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNbottomWidget, ip->goto_label); ac++;
     if (height1 > 21)
       XtSetArg(al[ac], XmNbottomOffset, (height1 / 5)); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, ip->goto_label); ac++;  
     XtSetArg(al[ac], XmNleftOffset, 5); ac++; 
     XtSetArg(al[ac], XmNsensitive, False); ac++; 
     XtSetValues (ip->goto_scale, al, ac );
/*
 * Position goto page number to right of slider
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->menu_bar); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNbottomWidget, ip->goto_label); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNleftWidget, ip->goto_scale); ac++; 
     XtSetArg(al[ac], XmNleftOffset, 5); ac++; 
     XtSetArg(al[ac], XmNsensitive, False); ac++; 
     XtSetValues (ip->goto_pgno, al, ac );
/*
 * Position the Scrolled Window below the Page Forward button.
 */
     ac = 0;
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, toolbar_frame); ac++;
     XtSetArg(al[ac], XmNtopOffset, -1); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetValues ( ip->scrolledw, al, ac );
     
/*
 * Position the footers below the Scrolled Window.
 */
     ac = 0;
     xmstrings[0] = XmStringCreateLocalized (" ");
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++; 
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNleftOffset, 1); ac++;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetValues ( ip->left_footer,al, ac );
     XmStringFree (xmstrings[0]);
     
     ac = 0; 
     xmstrings[0] = XmStringCreateLocalized (" ");
     XtSetArg(al[ac], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET); ac++;
     XtSetArg(al[ac], XmNtopWidget, ip->left_footer); ac++;
     XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNleftAttachment, XmATTACH_NONE); ac++;
     XtSetArg(al[ac], XmNrightOffset, 1); ac++;
     XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
     XtSetArg(al[ac], XmNlabelString, xmstrings[0]); ac++;
     XtSetValues ( ip->right_footer,al, ac );
     XmStringFree (xmstrings[0]);
/*
 * Set Scrolled Window to be drop site 
 */
     DtDndVaDropRegister (ip->scrolledw,
			  DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
			  XmDROP_COPY, transfer_rec,
			  NULL);
/*
 * Set Callbacks on Widgets just created.
 */
     XtAddCallback (ip->pgfwd_button, XmNactivateCallback, 
		    PgfwdButtonCallback, ip);
     XtAddCallback (ip->pgback_button, XmNactivateCallback, 
		    PgbackButtonCallback, ip);
     XtAddCallback (ip->vscroll, XmNvalueChangedCallback, ScrollBarCallback,
		    (XtPointer)XmVERTICAL);
     XtAddCallback (ip->hscroll, XmNvalueChangedCallback, ScrollBarCallback, 
		    (XtPointer)XmHORIZONTAL);
     XtAddCallback (ip->vscroll, XmNdragCallback, ScrollBarCallback,
		    (XtPointer)XmVERTICAL);
     XtAddCallback (ip->hscroll, XmNdragCallback, ScrollBarCallback, 
		    (XtPointer)XmHORIZONTAL);
     XtAddCallback (ip->goto_scale, XmNdragCallback, PageSliderCallback, ip);
     XtAddCallback (ip->goto_scale, XmNvalueChangedCallback, PageSliderCallback, ip);
/*
 * Set Help Callbacks on Widgets just created.
 */
     XtAddCallback (ip->form, XmNhelpCallback, HelpItemCallback, 
		    HELP_BASE_FORM);
     XtAddCallback (ip->menu_bar, XmNhelpCallback, HelpItemCallback, 
		    HELP_MENUBAR);
#ifdef DRAG_ICON
     XtAddCallback (ip->drag_icon, XmNhelpCallback, HelpItemCallback, 
		    HELP_DRAG_ICON);
#endif
     XtAddCallback (ip->goto_label, XmNhelpCallback, HelpItemCallback, 
		    HELP_PGSLIDER);
     XtAddCallback (ip->goto_scale, XmNhelpCallback, HelpItemCallback, 
		    HELP_PGSLIDER);
     XtAddCallback (ip->pgback_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_PGBACK_BUTTON);
     XtAddCallback (ip->pgfwd_button, XmNhelpCallback, HelpItemCallback, 
		    HELP_PGFWD_BUTTON);
     XtAddCallback (ip->scrolledw, XmNhelpCallback, HelpItemCallback, 
		    HELP_SCROLLED_WINDOW);
     XtAddCallback (ip->vscroll, XmNhelpCallback, HelpItemCallback, 
		    HELP_VSCROLL);
     XtAddCallback (ip->hscroll, XmNhelpCallback, HelpItemCallback, 
		    HELP_HSCROLL);
     XtAddCallback (ip->left_footer, XmNhelpCallback, HelpItemCallback, 
		    HELP_LEFT_FOOTER);
     XtAddCallback (ip->right_footer, XmNhelpCallback, HelpItemCallback, 
		    HELP_RIGHT_FOOTER);

/* 
 * Set scrolledw height in case
 * there is no canvas at start up and
 * no resize events.
 * The 10 below is the two 5 pixel offsets above
 * and below the paging buttons.
 */
/*
 * Manage the children
 */

/*
 * Manage scrollbars to be on always for now.
 */
     ac = 0;
     children[ac++] = ip->hscroll;
     children[ac++] = ip->vscroll;
     XtManageChildren (children, ac);

     /* Base form's children */
     ac = 0;
     children[ac++] = toolbar_frame;
     children[ac++] = ip->menu_bar;
     children[ac++] = ip->scrolledw;
     children[ac++] = ip->left_footer;
     children[ac++] = ip->right_footer;
     XtManageChildren(children, ac);
     ac = 0;
     XtManageChild ( ip->form);
     XtManageChild ( ip->mainw);
/*
 * Save the footer height.
 */
     XtVaGetValues (ip->right_footer, XmNheight, &ip->footer_height, NULL);
     ip->footer_height += 5;  /* BottomOffset of footer */
/*
 * Save the width of the scrollbars
 * Add 10 pixels for space between display area and scrollbar.
 */
     XtVaGetValues (ip->vscroll, XmNwidth, &ip->sb_width, NULL);
     ip->sb_width = ip->sb_width + 4;
/*
 * Set the Scrolled Window above the footer
 * at the bottom.
 */
     XtVaSetValues (ip->scrolledw, XmNbottomAttachment, XmATTACH_FORM,
		                   XmNbottomOffset, ip->footer_height, NULL); 
/*
 * Set min frame sizes.
 */
     XtVaGetValues (base->file_button, XmNwidth, &width1, NULL);
     XtVaGetValues (base->edit_button, XmNwidth, &width2, NULL);
     XtVaGetValues (base->view_button, XmNwidth, &width3, NULL);
     XtVaGetValues (base->help_button, XmNwidth, &width4, NULL);
     min_width = 10 + width1 + 15 + width2 + 15 + width3 + 15 + width4 + 10;
     
     XtVaGetValues (base->goto_label, XmNwidth, &width1, NULL);
     XtVaGetValues (base->goto_scale, XmNscaleWidth, &width2, NULL);
     XtVaGetValues (base->goto_pgno, XmNwidth, &width3, NULL);
     XtVaGetValues (base->pgback_button, XmNwidth, &width4, NULL);
     XtVaGetValues (base->pgfwd_button, XmNwidth, &width5, NULL);
#ifdef DRAG_ICON
     XtVaGetValues (base->drag_icon, XmNwidth, &width3, NULL);
#endif
     min_width2 = width1 + 5 + width2 + 5 + width3 + 5 + 
                  width4 + 2 + width5 + 5;
     if (min_width2 > min_width)
       min_width = min_width2;
     XtVaGetValues (base->file_button, XmNheight, &height1, NULL);
     XtVaGetValues (base->pgback_button, XmNheight, &height2, NULL);
     min_height = 10 + height1 + 10 + 5 + height2 + 5 + 35 + 
       base->footer_height + 5;
     XtVaSetValues (base->top_level, XmNminWidth, min_width,
		                     XmNminHeight, min_height, NULL);

     return ip;
}

void
create_stipple_pixmap (disp)
DisplayInfo	*disp;
{
    int			tile_width = 16;
    int			tile_height = 16;
    unsigned char	tile_bits [] = {
   				0x01, 0x01, 0x00, 0x00, 
				0x80, 0x80, 0x00, 0x00, 
				0x08, 0x08, 0x00, 0x00,
   				0x00, 0x00, 0x00, 0x00, 
				0x01, 0x01, 0x00, 0x00, 
				0x80, 0x80, 0x00, 0x00,
   				0x08, 0x08, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00};

    stipple_pixmap = XCreateBitmapFromData (disp->xdisplay, disp->win, 
					    (char *) tile_bits, tile_width, 
					    tile_height);
}

static int
ResolveXilHandles ()
{

     xil = (XilProcs *) calloc (1, sizeof (XilProcs));
     if (xil == NULL)
        return (-1);

     xil->handle = dlopen ("libxil.so.1", RTLD_LAZY | RTLD_GLOBAL);
     if (xil->handle == NULL)
	return (-1);

     xil->open = (XilSystemState (*)()) dlsym (xil->handle, "xil_open");
#ifdef DEBUG
     xil->state_set_synchronize = (void (*)()) dlsym (xil->handle,
					     "xil_state_set_synchronize");
#endif
     xil->install_error_handler = (int (*)()) dlsym (xil->handle, "xil_install_error_handler");
     xil->roi_add_rect = (void (*)()) dlsym (xil->handle, "xil_roi_add_rect");
     xil->set_roi = (void (*)()) dlsym (xil->handle, "xil_set_roi");
     xil->error_get_string = (char * (*)()) dlsym (xil->handle, "xil_error_get_string");
     xil->error_get_category = (XilErrorCategory (*)()) dlsym (xil->handle, "xil_error_get_category");
     xil->error_get_primary = (Xil_boolean (*)()) dlsym (xil->handle, "xil_error_get_primary");
     xil->get_width = (unsigned int (*)()) dlsym (xil->handle, "xil_get_width");
     xil->get_height = (unsigned int (*)()) dlsym (xil->handle, "xil_get_height");
     xil->get_datatype = (XilDataType (*)()) dlsym (xil->handle, "xil_get_datatype");	
     xil->get_nbands = (unsigned int (*)()) dlsym (xil->handle, "xil_get_nbands");	
     xil->roi_create = (XilRoi (*)()) dlsym (xil->handle, "xil_roi_create");
     xil->roi_destroy = (void (*)()) dlsym (xil->handle, "xil_roi_destroy");
     xil->set_value = (void (*)()) dlsym (xil->handle, "xil_set_value");
     xil->set_origin = (void (*)(XilImage, float, float)) dlsym (xil->handle, "xil_set_origin"); 
     xil->copy = (void (*)()) dlsym (xil->handle, "xil_copy");
     xil->destroy = (void (*)()) dlsym (xil->handle, "xil_destroy");
     xil->rescale = (void (*)()) dlsym (xil->handle, "xil_rescale");
     xil->create = (XilImage (*)()) dlsym (xil->handle, "xil_create");
     xil->create_child = (XilImage (*)()) dlsym (xil->handle, "xil_create_child");
     xil->create_from_device = (XilImage (*)()) dlsym (xil->handle, "xil_create_from_device");
     xil->set_device_attribute = (int (*)()) dlsym (xil->handle, "xil_set_device_attribute");
     xil->get_device_attribute = (int (*)()) dlsym (xil->handle, "xil_get_device_attribute");
     xil->export_ptr = (int (*)()) dlsym (xil->handle, "xil_export");
     xil->import_ptr = (void (*)()) dlsym (xil->handle, "xil_import");
     xil->lookup = (void (*)()) dlsym (xil->handle, "xil_lookup");
     xil->lookup_get_values = (void (*)(XilLookup, short, unsigned int, void *)) dlsym (xil->handle, "xil_lookup_get_values");
     xil->lookup_get_by_name = (XilLookup (*)()) dlsym (xil->handle, "xil_lookup_get_by_name");
     xil->lookup_set_values = (void (*)(XilLookup, short, unsigned int, void *)) dlsym (xil->handle, "xil_lookup_set_values");
     xil->lookup_get_offset = (short (*)()) dlsym (xil->handle, "xil_lookup_get_offset");
     xil->lookup_get_num_entries = (unsigned int (*)()) dlsym (xil->handle, "xil_lookup_get_num_entries");
     xil->lookup_create = (XilLookupCreateProc) dlsym (xil->handle, "xil_lookup_create");
     xil->lookup_convert = (XilLookup (*)()) dlsym (xil->handle, "xil_lookup_convert");
     xil->lookup_destroy = (void (*)()) dlsym (xil->handle, "xil_lookup_destroy");
     xil->choose_colormap = (XilLookup (*)()) dlsym (xil->handle, "xil_choose_colormap");
     xil->get_memory_storage = (Xil_boolean (*)()) dlsym (xil->handle, "xil_get_memory_storage");
     xil->set_memory_storage = (void (*)()) dlsym (xil->handle, "xil_set_memory_storage");
     xil->get_info = (void (*)()) dlsym (xil->handle, "xil_get_info");
     xil->dithermask_create = (XilDitherMask (*)()) dlsym (xil->handle, "xil_dithermask_create");
     xil->dithermask_get_by_name = (XilDitherMask (*)()) dlsym (xil->handle, "xil_dithermask_get_by_name");
     xil->colorcube_create = (XilColorcubeCreateProc) dlsym (xil->handle, "xil_colorcube_create");
     xil->ordered_dither = (void (*)()) dlsym (xil->handle, "xil_ordered_dither");
     xil->colorspace_get_by_name = (XilColorspace (*)()) dlsym (xil->handle, "xil_colorspace_get_by_name");
     xil->set_colorspace = (void (*)()) dlsym (xil->handle, "xil_set_colorspace");
     xil->color_convert = (void (*)()) dlsym (xil->handle, "xil_color_convert");
     xil->threshold = (void (*)()) dlsym (xil->handle, "xil_threshold");
     xil->cast = (void (*)()) dlsym (xil->handle, "xil_cast");
     xil->multiply_const = (void (*)()) dlsym (xil->handle, "xil_multiply_const");
     xil->xor_const = (void (*)()) dlsym (xil->handle, "xil_xor_const");
     xil->subtract_const = (void (*)()) dlsym (xil->handle, "xil_subtract_const");	
     xil->add_const = (void (*)()) dlsym (xil->handle, "xil_add_const");	
     xil->kernel_get_by_name = (XilKernel (*)()) dlsym (xil->handle, "xil_kernel_get_by_name");	
     xil->cis_create = (XilCis (*)()) dlsym (xil->handle, "xil_cis_create");	
     xil->cis_destroy = (void (*)()) dlsym (xil->handle, "xil_cis_destroy");	
     xil->cis_put_bits_ptr = (void (*)()) dlsym (xil->handle, "xil_cis_put_bits_ptr");	
     xil->cis_get_bits_ptr = (void * (*)()) dlsym (xil->handle, "xil_cis_get_bits_ptr");	
     xil->cis_has_data = (int (*)()) dlsym (xil->handle, "xil_cis_has_data");	
     xil->cis_get_output_type = (XilImageType (*)()) dlsym (xil->handle, "xil_cis_get_output_type");	
     xil->affine = (void (*)()) dlsym (xil->handle, "xil_affine");	
     xil->create_from_window = (XilImage (*)()) dlsym (xil->handle, "xil_create_from_window");	
     xil->create_copy = (XilImage (*)()) dlsym (xil->handle, "xil_create_copy");	
     xil->error_diffusion = (void (*)()) dlsym (xil->handle, "xil_error_diffusion");	
     xil->imagetype_get_info = (void (*)()) dlsym (xil->handle, "xil_imagetype_get_info");	
     xil->decompress = (void (*)()) dlsym (xil->handle, "xil_decompress");	
     xil->compress = (void (*)()) dlsym (xil->handle, "xil_compress");	
     xil->rotate = (void (*)(XilImage, XilImage, char *, float)) dlsym (xil->handle, "xil_rotate");	
     xil->scale = (void (*)(XilImage, XilImage, char *, float, float)) dlsym (xil->handle, "xil_scale");	
     return 0;
}

void
BaseImageCanvasObjectsCreate (ip)
     BaseObjects	*ip;
{
     Dimension           cwidth, cheight;
     Colormap	         cmap;
     XColor	         ccolor;
     unsigned long       gc_mask;
     XGCValues 	         gc_vals;
     int                 width, widthmm, height, heightmm;
     Display            *xdisplay;
     XVisualInfo         visual_info;
     int                 vis_class;
     int                 status, depth, def_depth;
     Visual             *def_visual;
  
     extern Xil_boolean  xil_error_handler ();
     extern void         PropsKeyEventHandler ();
     extern void         CanvasRepaintCallback ();
     extern void         CanvasResizeCallback ();
     extern void         CanvasInputCallback ();
     extern void         SelectEventHandler ();

     xdisplay = XtDisplay (ip->top_level);

     XtVaGetValues (ip->scrolledw, XmNdepth, &def_depth, NULL);
/*
 * Create the image canvas.
 * Fix later - using defaultvisual for now.
 * Uncomment this when visual bug in motif is fixed
 */
     depth = canvas_depth_value(); 
     vis_class = canvas_visual_class (depth);

/*
 * Get the visual that matches this depth and visual class.
 */
     status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), depth, 
				vis_class, &visual_info);

/*
 * If visual desired not found, use default visual.
 */
     if (status == 0) {
       def_visual = DefaultVisual (xdisplay, DefaultScreen (xdisplay));
       status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), 
				  def_depth, def_visual->class, &visual_info);
       depth = def_depth;
       if (status == 0)
	 fprintf (stderr, catgets (prog->catd, 1, 383, 
			  "Could not find visual to display image.\n"));
     }
     def_visual = DefaultVisual (xdisplay, DefaultScreen (xdisplay));

     ip->canvas = XtVaCreateManagedWidget ("Canvas",
		    xmDrawingAreaVisWidgetClass, ip->scrolledw,
		    XmNdepth, depth,
		    XmNvisual, visual_info.visual, 
		    NULL);

     XLowerWindow (XtDisplay (ip->canvas), XtWindow (ip->canvas));
     XmScrolledWindowSetAreas(ip->scrolledw, ip->hscroll, ip->vscroll, ip->canvas);
     XtAddCallback (ip->canvas, XmNexposeCallback, CanvasRepaintCallback, ip);
     XtAddCallback (ip->canvas, XmNresizeCallback, CanvasResizeCallback, ip);
     XtAddCallback (ip->canvas, XmNhelpCallback, HelpItemCallback, HELP_IMAGE_CANVAS);
     XtAddCallback (ip->canvas, XmNinputCallback, CanvasInputCallback, ip); 
     XtAddEventHandler (ip->canvas, KeyPressMask, False, 
			PropsKeyEventHandler, ip)
;
/*
 * Install event handler for Button1Motion events.
 */
     XtAddEventHandler (ip->canvas, Button1MotionMask, False, 
	         	SelectEventHandler, ip);

/*
 * Set the canvas height, width to the same as the scrolledw - sb_width.
 */
     XtVaGetValues (ip->scrolledw, XmNwidth, &cwidth,
		                   XmNheight, &cheight, NULL);
     XtVaSetValues (ip->canvas, XmNwidth, cwidth - ip->sb_width,
	                        XmNheight, cheight - ip->sb_width, NULL);
/*
 * Open XIL library.
 */
     status = ResolveXilHandles();
     if (status != -1)
       image_display->state = (*xil->open)(); 
     if (status == -1 || image_display->state == NULL) {
	AlertPrompt (base->top_level,
	 	     DIALOG_TYPE, XmDIALOG_WARNING,
		     DIALOG_TITLE, catgets (prog->catd, 1, 6, "Image Viewer"),
		     DIALOG_TEXT, catgets (prog->catd, 1, 384, "Unable to open XIL Library."),
		     BUTTON_IDENT, 0, catgets (prog->catd, 1, 108, "Continue"),
		     HELP_IDENT, HELP_OPEN_ITEM,
		     NULL);

     }
     else
       prog->xil_opened = TRUE;
#ifdef DEBUG
     if (prog->xil_opened)
       (*xil->state_set_synchronize) (image_display->state, ON);
#endif
     if (prog->xil_opened)
       (*xil->install_error_handler) (image_display->state, xil_error_handler);
     XtVaGetValues (ip->canvas, XmNdepth, &image_display->depth, 
	                        XmNcolormap, &cmap, NULL);
     image_display->visual = visual_info.visual; 
     image_display->new_canvas = ip->canvas;
     image_display->win = XtWindow (ip->canvas);
/*   image_display->display_win = 
       (*xil->create_from_window) (image_display->state,
			       image_display->xdisplay,
			     	image_display->win); */

     image_display->pixmap1 = NULL;
     image_display->pixmap2 = NULL;

     /*
      * (DKenny - 04 Apr, 1997)
      *
      * Fix for Bug# 1219732
      *
      * Figure out the correct resolution of the display. This code should
      * work a lot better than using the previous assumtion that all displays
      * are of a resoluion of 83.0 dpi, especially since most of the new
      * graphics cards are running at a resolution of 90dpi. A more important
      * factor in this is how on earth did Intel platforms work with a
      * resolution of 83 dpi?
      *
      */
     width = DisplayWidth (image_display->xdisplay, 
		       	           DefaultScreen (image_display->xdisplay));
     widthmm = DisplayWidthMM (image_display->xdisplay,
				       DefaultScreen (image_display->xdisplay));
     height = DisplayHeight (image_display->xdisplay, 
				     DefaultScreen (image_display->xdisplay));
     heightmm = DisplayHeightMM (image_display->xdisplay,
				         DefaultScreen (image_display->xdisplay));
     image_display->res_x = width / (widthmm / 25.4);
     image_display->res_y = height / (heightmm / 25.4);

/*
 * Next, get the pixel index for White
 */
/*
     cmap = DefaultColormap (image_display->xdisplay, 
			     DefaultScreen (image_display->xdisplay));
*/
     ccolor.red = ccolor.blue = ccolor.green = 65535;
     XAllocColor (image_display->xdisplay, cmap, &ccolor);
     gc_vals.background = ccolor.pixel;
 
/*
 * Next, get the pixel index for Black
 */
     ccolor.red = ccolor.blue = ccolor.green = 0;
     XAllocColor (image_display->xdisplay, cmap, &ccolor);
     gc_vals.foreground = ccolor.pixel;
 
     gc_mask = GCForeground | GCBackground;
     
     image_display->win_gc = XCreateGC (image_display->xdisplay, 
					image_display->win, gc_mask, 
					&gc_vals);

     ccolor.red = ccolor.blue = ccolor.green = (230 << 8) + 230;
     XAllocColor (image_display->xdisplay, cmap, &ccolor);
     gc_vals.background = ccolor.pixel;
/*
 * Create the stipple pixmap.
 */
     if (stipple_pixmap == NULL)
       create_stipple_pixmap (image_display);

     gc_vals.stipple = stipple_pixmap;
     gc_vals.fill_style = FillOpaqueStippled;

     gc_mask = GCForeground | GCBackground | GCFillStyle | GCStipple;
     
     if (image_display->depth == 24)
       image_display->background[0] = image_display->background[1] =
       image_display->background[2] = 230;
     else
       image_display->background[0] = ccolor.pixel; 
/*
 * Create the fillarea gc
 */
     image_display->fill_gc = XCreateGC (image_display->xdisplay, 
					 image_display->win, gc_mask, 
					 &gc_vals);
/*
 * Create the select gc too.
 */
     ccolor.red = ccolor.blue = ccolor.green = 40000;
     XAllocColor( image_display->xdisplay, cmap, &ccolor );
     gc_vals.background = ccolor.pixel;
     
     ccolor.red = ccolor.blue = ccolor.green = 0;
     XAllocColor( image_display->xdisplay, cmap, &ccolor );
     gc_vals.foreground = ccolor.pixel;
     
     gc_vals.function = GXxor;          
     gc_vals.line_width = 2;
     gc_vals.line_style = LineDoubleDash;
     gc_mask = GCForeground | GCBackground | GCFunction | 
               GCLineWidth | GCLineStyle;
     image_display->select_gc = XCreateGC (image_display->xdisplay, 
					   image_display->win, 
					   gc_mask, &gc_vals );
     if (depth != 24)
       XSetWindowColormap (image_display->xdisplay, image_display->win, cmap);

}

void
BasePSCanvasObjectsCreate (ip)
     BaseObjects	*ip;
{
     int	       vis_class, depth;
     Colormap	       cmap;
     XColor	       ccolor;
     XGCValues	       gc_vals;
     Display	      *xdisplay;
     unsigned long     gc_mask;
     int	       major_opcode, first_event, first_error;
     int               width, widthmm, height, heightmm;
     Visual	      *def_visual;
     int	       def_depth, def_vis;
     extern int	       display_postscript_present();
     XVisualInfo       visual_info;
     Status            status;
     extern void       KeyEventHandler ();
     extern void       SelectEventHandler ();
     extern void       CanvasEventHandler ();
     extern void       CanvasInputCallback ();
     extern void       CanvasRepaintCallback ();
     extern void       CanvasResizeCallback ();
     Dimension         cwidth, cheight;
     
     xdisplay = XtDisplay (ip->top_level);

     XtVaGetValues (ip->scrolledw, XmNdepth, &def_depth, NULL);

     depth = canvas_depth_value (); 
/*
 * Determine if we're running on a dps based server.
 */
     prog->dps = display_postscript_present(xdisplay);
                         
     if (prog->dps == TRUE) {
       
       def_visual = DefaultVisual (xdisplay, DefaultScreen (xdisplay));
       def_vis = def_visual->class;
/*
 * vis_class returns PseudoColor, GrayScale (depth 4, 8 or 16), 
 * TrueColor (depth 24) or StaticGray (depth 1).
 */ 
       vis_class = canvas_visual_class (depth); 

/*
 * Ok, now check and see if we can reduce flashing by making the
 * canvas from a `better' visual.
 */

       if ((def_depth != 1) && (depth == def_depth)) {
	 if ((vis_class == GrayScale) && (def_vis == StaticGray)) 
	   vis_class = StaticGray;
	 else if ((vis_class == PseudoColor) && (def_vis == StaticColor)) 
	   vis_class = StaticColor;
/*
	   else if ((vis_class == TrueColor) && (def_vis == DirectColor) &&
	   (depth != 8)) 
	   vis_class = DirectColor;
*/
       }
     }
/*
 * Get the visual that matches this depth and visual class.
 * If not found, use default.
 */
     status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), depth, 
				vis_class, &visual_info);
     if (status == 0) {
       if (def_depth == 1) 
	 status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), 
				    def_depth, StaticGray, &visual_info);
       else if (def_depth == 24)
	 status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), 
				    def_depth, TrueColor, &visual_info);
       else 
	 status = XMatchVisualInfo (xdisplay, DefaultScreen (xdisplay), 
				    def_depth, StaticColor, &visual_info);
     }
     if (status == 0)
       fprintf (stderr, catgets (prog->catd, 1, 385, 
			"Could not find visual to display PostScript.\n"));
     ip->ps_canvas = XtVaCreateManagedWidget ("PSCanvas",
		       xmDrawingAreaVisWidgetClass, ip->scrolledw,
		       XmNvisual, visual_info.visual,
		       XmNuserData, ip,
                       XmNdepth, depth,
		       NULL);
     XmScrolledWindowSetAreas(ip->scrolledw, ip->hscroll, ip->vscroll, 
			      ip->ps_canvas);
/*
 * Install event handler for ClientMessage events.
 */
     XtAddEventHandler (ip->ps_canvas, NULL, True, CanvasEventHandler, ip);
/*
 * Install event handler for KeyPress events.
 */
     XtAddEventHandler (ip->ps_canvas, KeyPressMask, False, 
			KeyEventHandler, ip);
/*
 * Install event handler for Button1Motion events.
 */
     XtAddEventHandler (ip->ps_canvas, Button1MotionMask, False, 
		        SelectEventHandler, ip);

     XtAddCallback (ip->ps_canvas, XmNexposeCallback, CanvasRepaintCallback, ip);
     XtAddCallback (ip->ps_canvas, XmNresizeCallback, CanvasResizeCallback, ip);
     XtAddCallback (ip->ps_canvas, XmNhelpCallback, HelpItemCallback,
		    HELP_PS_CANVAS);
     XtAddCallback (ip->ps_canvas, XmNinputCallback, CanvasInputCallback, ip);
/*
 * Set the canvas height, width to the same as the scrolledw - sb_width.
 */
     XtVaGetValues (ip->scrolledw, XmNwidth, &cwidth,
	                           XmNheight, &cheight, NULL);
     XtVaSetValues (ip->ps_canvas, XmNwidth, cwidth - ip->sb_width,
	                           XmNheight, cheight - ip->sb_width, NULL);

     if (prog->dps == TRUE) {
       MakeStdColormaps (ip->ps_canvas, vis_class);
       if (depth != def_depth || prog->ps_mono == True)
         MakeSmallStdColormaps (ip->mainw, def_vis);
     }

     ps_display->xdisplay = xdisplay;
     XtVaGetValues (ip->ps_canvas, XmNdepth, &ps_display->depth,
		                   XmNcolormap, &cmap, NULL);
     ps_display->visual = visual_info.visual;
     ps_display->new_canvas = ip->ps_canvas;
     ps_display->win = XtWindow (ip->ps_canvas);

     ps_display->pixmap1 = NULL;
     ps_display->pixmap2 = NULL;

     /*
      * (DKenny - 04 Apr, 1997)
      *
      * Fix for Bug# 1219732
      *
      * Figure out the correct resolution of the display. This code should
      * work a lot better than using the previous assumtion that all displays
      * are of a resoluion of 83.0 dpi, especially since most of the new
      * graphics cards are running at a resolution of 90dpi. A more important
      * factor in this is how on earth did Intel platforms work with a
      * resolution of 83 dpi?
      *
      */
     width = DisplayWidth (image_display->xdisplay, 
				   DefaultScreen (image_display->xdisplay));
     widthmm = DisplayWidthMM (image_display->xdisplay,
				       DefaultScreen (image_display->xdisplay));
     height = DisplayHeight (image_display->xdisplay, 
				     DefaultScreen (image_display->xdisplay));
     heightmm = DisplayHeightMM (image_display->xdisplay,
					 DefaultScreen (image_display->xdisplay));
     ps_display->res_x = (int)(width / (widthmm / 25.4));
     ps_display->res_y = (int)(height / (heightmm / 25.4));
     /*
      * (DKenny - 25 Apr, 1997)
      *
      * There seems to be a problem elsewhere that is causing strange things
      * to happen when the dpi of the screen goes below the PS standard of 72.
      * This will fix the bug, and cause no side effects in low resolutions.
      */
     ps_display->res_x = (ps_display->res_x<72)?(72):(ps_display->res_x);
     ps_display->res_y = (ps_display->res_y<72)?(72):(ps_display->res_y);
     
/*
 * Next, get the pixel index for White
 */
/*     cmap = DefaultColormap (image_display->xdisplay, 
			     DefaultScreen (image_display->xdisplay)); */
     ccolor.red = ccolor.blue = ccolor.green = 65535;
     XAllocColor (ps_display->xdisplay, cmap, &ccolor);
     gc_vals.background = ccolor.pixel;
 
/*
 * Next, get the pixel index for Black
 */
     ccolor.red = ccolor.blue = ccolor.green = 0;
     XAllocColor (ps_display->xdisplay, cmap, &ccolor);
     gc_vals.foreground = ccolor.pixel;
 
     gc_mask = GCForeground | GCBackground;
     ps_display->win_gc = XCreateGC (ps_display->xdisplay, 
				     ps_display->win, gc_mask, &gc_vals);

     ccolor.red = ccolor.blue = ccolor.green = (230 << 8) + 230;
     XAllocColor (ps_display->xdisplay, cmap, &ccolor);
     gc_vals.background = ccolor.pixel;

     if (stipple_pixmap == NULL)
       create_stipple_pixmap (ps_display);

     gc_vals.stipple = stipple_pixmap;
     gc_vals.fill_style = FillOpaqueStippled;

     gc_mask = GCForeground | GCBackground | GCFillStyle | GCStipple;
     
     ps_display->fill_gc = XCreateGC (ps_display->xdisplay, 
				      ps_display->win, gc_mask, &gc_vals);

/*
 * create the select gc too.
 */

     ccolor.red = ccolor.blue = ccolor.green = 40000;
     XAllocColor( ps_display->xdisplay, cmap, &ccolor );
     gc_vals.background = ccolor.pixel;
     
     ccolor.red = ccolor.blue = ccolor.green = 0;
     XAllocColor( ps_display->xdisplay, cmap, &ccolor );
     gc_vals.foreground = ccolor.pixel;

     gc_vals.function = GXxor;          
     gc_vals.line_width = 2;
     gc_vals.line_style = LineDoubleDash;
     gc_mask = GCForeground | GCBackground | GCFunction |
               GCLineWidth | GCLineStyle;
     ps_display->select_gc = XCreateGC (ps_display->xdisplay, 
					ps_display->win, gc_mask, &gc_vals );
     XtVaGetValues (ip->ps_canvas, XmNwidth, &ps_display->width,
		                   XmNheight, &ps_display->height, NULL);
/*
 * These should be retrieved from property data..
 */
     ps_display->pageheight = 11.0;
     ps_display->pagewidth = 8.5;

/*
 * Calculate the default ps scale.
 */
     PSDefaultScale ();
}

/*
 * Return the canvas depth base on either
 * command line arg or properties setting.
 */
int
canvas_depth_value ()
{
     int   depth = 8;

     if (strcmp (current_props->color, COLORS_BW) == 0)
       depth = 1;
     else if (strcmp (current_props->color, COLORS_16) == 0)
       depth = 4;
     else if (strcmp (current_props->color, COLORS_256) == 0)
       depth = 8;
     else if (strcmp (current_props->color, COLORS_THOUSANDS) == 0)
       depth = 16;
     else if (strcmp (current_props->color, COLORS_MILLIONS) == 0)
       depth = 24;
     
    return (depth);
}

/*
 * Determine the canvas visual class based on
 * the depth of the canvas.
 */
int
canvas_visual_class (depth)
{
    int          vis_class = 3;
    int          color_set = FALSE;
    extern int   gray_vis1;
    extern int   color_vis1;
    extern int   gray_vis4;
    extern int   color_vis4;
    extern int   gray_vis8;
    extern int   color_vis8;
    extern int   gray_vis16;
    extern int   color_vis16;
    extern int   gray_vis24;
    extern int   color_vis24;

    if (strcmp (current_props->view_in, VIEW_COLOR) == 0)
      color_set = TRUE;

    if (depth == 1) {
      if (color_set && color_vis1 != -1)    
        vis_class = color_vis1;
      else if (gray_vis1 != -1)
	vis_class = gray_vis1;
    }
    else if (depth == 4) {
      if (color_set && color_vis4 != -1)
        vis_class = color_vis4;
      else if (gray_vis4 != -1)
	vis_class = gray_vis4;
    }
    else if (depth == 8) {
      if (color_set && color_vis8 != -1)
        vis_class = color_vis8;
      else if (gray_vis8 != -1)
	vis_class = gray_vis8;
    }
    else if (depth == 16) {
      if (color_set && color_vis16 != -1)
        vis_class = color_vis16;
      else if (gray_vis16 != -1)
	vis_class = gray_vis16;
    }
    else if (depth == 24) {
      if (color_set && color_vis24 != -1)
        vis_class = color_vis24;
      else if (gray_vis24 != -1)
	vis_class = gray_vis24;
    }

    return (vis_class);

}

