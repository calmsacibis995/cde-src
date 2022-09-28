/* $XConsortium: Help.h /main/cde1_maint/2 1995/10/23 11:16:00 gtsang $ */
/*
 ****************************************************************************
 **
 **   File:        Help.h
 **
 **   Project:     DT 3.0
 **
 **   Description: header file for the help portion of Dtstyle
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************/
#ifndef _help_h
#define _help_h

/*
 * defines for help on the main window
 */
#define HELP_MAIN_WINDOW	38
#define HELP_MAIN_WINDOW_STR	"mainWindow"

/*
 * defines for help on the menubar
 */
#define HELP_MENUBAR	        1
#define HELP_MENUBAR_STR	"menubar"

/*
 * defines for help on the menubar buttons
 */
#define HELP_FILE_BUTTON	2
#define HELP_HELP_BUTTON	3

#define HELP_FILE_BUTTON_STR	"fileButton"
#define HELP_HELP_BUTTON_STR	"helpButton"

/*
 * defines for help on the menu panes
 */
#define HELP_FILE_MENU		4
#define HELP_HELP_MENU		5

#define HELP_FILE_MENU_STR	"fileMenu"
#define HELP_HELP_MENU_STR	"helpMenu"

/*
 * defines for help on the File menu items
 */
#define HELP_EXIT		6
#define HELP_EXIT_STR	        "exitMenu"

/*
 * defines for help on the Help menu items
 */
#define HELP_INTRODUCTION       7
#define HELP_TASKS	        8		/* unused */
#define HELP_REFERENCE	        9
#define HELP_ITEM	        10
#define HELP_USING	        11		/* unused */
#define HELP_VERSION	        12

#define HELP_USING_HELP_VOLUME  "Help4Help"
#define HELP_VOLUME             "Stylemgr"

#define HELP_INTRODUCTION_STR	"_homeTopic"
#define HELP_TASKS_STR      	"tasks"
#define HELP_REFERENCE_STR	"reference"
#define HELP_ITEM_STR	        "onItem"
#define HELP_USING_STR  	"_hometopic"
#define HELP_VERSION_STR	"_copyright"

/*
 * defines for the Dtstyle main window buttons
 */
#define HELP_COLOR_BUTTON	        14
#define HELP_FONT_BUTTON	        15
#define HELP_BACKDROP_BUTTON	        16
#define HELP_KEYBOARD_BUTTON	        17
#define HELP_MOUSE_BUTTON	        18
#define HELP_AUDIO_BUTTON	        19
#define HELP_SCREEN_BUTTON	        20
#define HELP_DTWM_BUTTON	        21
#define HELP_STARTUP_BUTTON	        22

#define HELP_COLOR_BUTTON_STR	        "colorButton"
#define HELP_FONT_BUTTON_STR	        "fontButton"
#define HELP_BACKDROP_BUTTON_STR	"backdropButton"
#define HELP_KEYBOARD_BUTTON_STR	"keyboardButton"
#define HELP_MOUSE_BUTTON_STR	        "mouseButton"
#define HELP_AUDIO_BUTTON_STR	        "audioButton"
#define HELP_SCREEN_BUTTON_STR	        "screenButton"
#define HELP_DTWM_BUTTON_STR	        "dtwmButton"
#define HELP_STARTUP_BUTTON_STR	        "startupButton"

/*
 * defines for help on the Color dialogs
 */
#define HELP_COLOR_DIALOG	                23
#define HELP_ADD_PALETTE_DIALOG	                24
#define HELP_ADD_PALETTE_WARNING_DIALOG         25
#define HELP_DELETE_PALETTE_DIALOG              26
#define HELP_DELETE_PALETTE_WARNING_DIALOG      27
#define HELP_MODIFY_PALETTE_DIALOG              28
#define HELP_COLOR_USE_DIALOG	                29

#define HELP_COLOR_DIALOG_STR                   "colorDialog"
#define HELP_ADD_PALETTE_DIALOG_STR	        "addPaletteDialog"
#define HELP_ADD_PALETTE_WARNING_DIALOG_STR     "addPaletteWarningDialog"
#define HELP_DELETE_PALETTE_DIALOG_STR	        "deletePaletteDialog"
#define HELP_DELETE_PALETTE_WARNING_DIALOG_STR  "deletePaletteWarningDialog"
#define HELP_MODIFY_PALETTE_DIALOG_STR	        "modifyPaletteDialog"
#define HELP_COLOR_USE_DIALOG_STR               "colorUseDialog"

/*
 * defines for help on the Font dialog
 */
#define HELP_FONT_DIALOG	        30
#define HELP_FONT_DIALOG_STR            "fontDialog"

#define HELP_FONT_ATTRIB_DIALOG	        39
#define HELP_FONT_ATTRIB_DIALOG_STR     "fontAttribDialog"

#define HELP_FONT_ADD_GROUP_DLG	        40
#define HELP_FONT_ADD_GROUP_DLG_STR     "addFontGroupDialog"

#define HELP_FONT_DEL_GROUP_DLG         41
#define HELP_FONT_DEL_GROUP_DLG_STR     "deleteFontGroupDialog"

#define HELP_FONT_GROUP_NAME_DLG	42
#define HELP_FONT_GROUP_NAME_DLG_STR    "fontGroupNameDialog"


/*
 * defines for help on the Backdrop dialog
 */
#define HELP_BACKDROP_DIALOG	        31
#define HELP_BACKDROP_DIALOG_STR        "backdropDialog"

/*
 * defines for help on the Keyboard dialog
 */
#define HELP_KEYBOARD_DIALOG	        32
#define HELP_KEYBOARD_DIALOG_STR        "keyboardDialog"

/*
 * defines for help on the Mouse dialog
 */
#define HELP_MOUSE_DIALOG	        33
#define HELP_MOUSE_DIALOG_STR           "mouseDialog"

/*
 * defines for help on the Audio dialog
 */
#define HELP_AUDIO_DIALOG      	        34
#define HELP_AUDIO_DIALOG_STR           "audioDialog"

/*
 * defines for help on the Screen dialog
 */
#define HELP_SCREEN_DIALOG	        35
#define HELP_SCREEN_DIALOG_STR          "screenDialog"

/*
 * defines for help on the Dtwm dialog
 */
#define HELP_DTWM_DIALOG	        36
#define HELP_DTWM_DIALOG_STR           "dtwmDialog"

/*
 * defines for help on the Startup dialog
 */
#define HELP_STARTUP_DIALOG	        37
#define HELP_STARTUP_DIALOG_STR         "startupDialog"


/*  typedef statements */

typedef struct _helpStruct {
    struct _helpStruct *pNext;
    struct _helpStruct *pPrev;
    Widget dialog;
    Boolean inUse;
} HelpStruct;


/*  External Interface */

#ifdef _NO_PROTO

extern void HelpRequestCB( ) ;
extern void QuickHelpRequestCB( ) ;
extern void HelpModeCB( ) ;
extern void HelpHyperlinkCB( ) ;
extern void HelpCloseCB( ) ;
#else

extern void HelpRequestCB(
                            Widget w,
                            caddr_t client_data,
                            caddr_t call_data ) ;
extern void QuickHelpRequestCB(
                            Widget w,
                            caddr_t client_data,
                            caddr_t call_data ) ;
extern void HelpModeCB(
                            Widget w,
                            caddr_t client_data,
                            caddr_t call_data ) ;
extern void HelpHyperlinkCB(
                            Widget w,
                            caddr_t client_data,
                            caddr_t call_data ) ;
extern void HelpCloseCB(
                            Widget w,
                            caddr_t client_data,
                            caddr_t call_data ) ;

#endif /* _NO_PROTO */

#endif /*  _font_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

