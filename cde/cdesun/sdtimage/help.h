/*
 * @(#)help.h 1.14 97/03/17
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#ifndef	_HELP_H
#define	_HELP_H

/*
 * Different help events from the help menu.
 */
typedef enum {
  HELP_OVERVIEW_EVENT   = 7001,
  HELP_TASKS_EVENT      = 7003,
  HELP_REF_EVENT        = 7004,
  HELP_ONITEM_EVENT     = 7006,
  HELP_USING_HELP_EVENT = 7007
} HELP_EVENT;


#define IMAGETOOL_HELP_VOLUME 	"Sdtimage"

#define HELP_OVERVIEW_STR	"_HOMETOPIC"
#define HELP_TASKS_STR		"TASKS"
#define HELP_REF_STR		"REFERENCE"
#define HELP_USING_HELP_STR	"_HOMETOPIC"

/*
 * Location IDs for help on individual items.
 */

/*
 * Menus, Menu Buttons
 */
#define HELP_TOP_LEVEL          "_hometopic"
#define HELP_BASE_FORM          "ImageVViewWindow"
#define HELP_MENUBAR            "ImageVViewWindow"
#define HELP_PGSLIDER           "PagingThroughTA"
#define HELP_PGBACK_BUTTON      "PagingThroughTA"
#define HELP_PGFWD_BUTTON       "PagingThroughTA"
#define HELP_SCROLLED_WINDOW    "ImageVViewWindow"
#define HELP_VSCROLL            "ImageVViewWindow"
#define HELP_HSCROLL            "ImageVViewWindow"
#define HELP_LEFT_FOOTER        "ImageVViewWindow"
#define HELP_RIGHT_FOOTER       "ImageVViewWindow"
#define HELP_IMAGE_CANVAS       "ImageVViewWindow"
#define HELP_PS_CANVAS          "ImageVViewWindow"

/*
 * IDs for File Menu Items
 */
#define HELP_FILE_MENU          "ImageVFileMenu"
#define HELP_OPEN_ITEM          "ImageVOpenDialog"
#define HELP_OPENAS_ITEM        "ImageVOpenAsDialog"
#define HELP_SAVE_ITEM          "ImageVSaveDialog"
#define HELP_SAVEAS_ITEM        "ImageVSaveAsDialog"
#define HELP_SAVESEL_ITEM       "ImageVSaveSelectionAsDialog"
#define HELP_SAVEPAGE_ITEM      "ImageVSavePageAsDialog"
#define HELP_PRINTONE_ITEM      "ImageVFileMenu"
#define HELP_PRINTPREV_ITEM     "ImageVPrintPreviewDialog"
#define HELP_PRINT_ITEM         "ImageVPrintDialog"
#define HELP_PROPS_ITEM         "ImageVOptionsDialog"
#define HELP_EXIT_ITEM          "ImageVFileMenu"
/*
 * IDs for Edit Menu Items
 */
#define HELP_EDIT_MENU          "ImageVEditMenu"
#define HELP_UNDO_ITEM          "ImageVEditMenu"
#define HELP_PALETTE_ITEM       "ImageVPaletteDialog"
/*
 * IDs for View Menu Items
 */
#define HELP_VIEW_MENU          "ImageVViewMenu"
#define HELP_IMAGEINFO_ITEM     "ImageVImageInfoDialog"
#define HELP_OVERVIEW_ITEM      "ImageVPageOverviewDialog"
#define HELP_CONTROLS_ITEM      "ImageVPageViewingControlsDialog"
/*
 * IDs for Help Menu Items
 */
#define HELP_HELP_MENU          "ImageVHelpMenu"
#define HELP_HELP_OVERVIEW_ITEM "ImageVHelpMenu"
#define HELP_TASKS_ITEM         "ImageVHelpMenu"
#define HELP_REF_ITEM           "ImageVHelpMenu"
#define HELP_ON_ITEM            "ImageVHelpMenu"
#define HELP_USING_HELP_ITEM    "ImageVHelpMenu"
#define HELP_ABOUT_ITEM         "ImageVHelpMenu"

#define HELP_DND_ITEM           "ImageVDNDError"

/*
 * IDs for snapshot items. 
 */
#define HELP_SNAPSHOT           "ImageVSnapshotWindow"
#define	HELP_SNAP_WINDOW	"ToSnapWindowTA"
#define	HELP_SNAP_REGION	"ToSnapRegionTA"
#define	HELP_SNAP_SCREEN	"ToSnapScreenTA"
#define HELP_SNAP_DELAY		"ToTakeDelayedSnapTA"
#define HELP_SNAP_HIDE		"ToHideSnapTA"
#define	HELP_SNAP_SAVE		"ToSaveSnapTA"


typedef struct {
       Widget           shell;
       Widget           dialog;
       Widget           about_icon;
       Widget           close_button;
       Boolean          showing;
} AboutObjects;

extern AboutObjects   *about;

extern void HelpItemCallback();

#endif /* _HELP_H */
