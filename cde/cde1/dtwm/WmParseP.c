#ifdef VERBOSE_REV_INFO
static char rcsid[] = "$XConsortium: WmParseP.c /main/cde1_maint/1 1995/07/15 01:56:22 drk $";
#endif /* VERBOSE_REV_INFO */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmParseP.c
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains the common strings used in parsing the 
 **  dtwmrc file
 **
 **
 *********************************************************************
 **
 ** (c) Copyright 1987, 1988, 1989, 1990, 1991 HEWLETT-PACKARD COMPANY
 ** ALL RIGHTS RESERVED
 **
 **********************************************************************
 **********************************************************************
 **
 **
 **********************************************************************
 ******************************<+>*************************************/

/*
 * Included Files:
 */

/*
 * Panel keywords (2.x compatibility)
 */
char _dtwmFpPanelOld[]		= "frontpanel";
char _dtwmFpRowOld[]		= "row";

char _dtwmFpDropEffects[]	= "dropeffects";

/*
 * Panel keywords (3.0 syntax)
 */
char _dtwmFpPanel[]		= "panel";
char _dtwmFpBox[]		= "box";
char _dtwmFpCommand[]		= "command";
char _dtwmFpControl[]		= "control";
char _dtwmFpSwitch[]		= "switch";

char _dtwmFpAnimation[]	= "animation";
char _dtwmFpInclude[]		= "include";

/*
 * Panel, Box, Control common attribute keywords
 */
char _dtwmFpaColorSet[]	= "color_set";
char _dtwmFpaBackgroundTile[]	= "background_tile";
char _dtwmFpaLayoutPolicy[]	= "layout_policy";
char _dtwmFpaOrientation[]	= "orientation";
char _dtwmFpaSubpanelDirection[]	= "subpanel_direction";
char _dtwmFpaNumberOfRows[]	= "number_of_rows";
char _dtwmFpaNumberOfColumns[]	= "number_of_columns";
char _dtwmFpaTitle[]		= "title";

/*
 * Control old reserved names and control types
 */
char	_dtwmFpnDtwmblank[]		= "dtwmblank";
char	_dtwmFpnDtwmbox[]		= "dtwmbox";
char	_dtwmFpnDtwmbusy[]		= "dtwmbusy";
char	_dtwmFpnDtwmclock[]		= "dtwmclock";
char	_dtwmFpnDtwmdate[]		= "dtwmdate";
char	_dtwmFpnDtwmload[]		= "dtload";
char	_dtwmFpnDtwmname[]		= "dtwmname";
char	_dtwmFpnDtwmrib[]		= "dtwmrib";
char	_dtwmFpnDtwmswitch[]		= "dtwmswtch";

char	_dtwmFptClock[]		= "clock";
char	_dtwmFptDate[]			= "date";
char	_dtwmFptBlank[]		= "blank";
char	_dtwmFptBusy[]			= "busy";
char	_dtwmFptButton[]		= "button";
char 	_dtwmFptClient[]		= "client";
char 	_dtwmFptIconbox[]		= "iconbox";
char 	_dtwmFptMail[]			= "mail";
char	_dtwmFptMonitorFile[]		= "monitor_file";

/*
 * Panel type keywords
 */
char	_dtwmFptFrontPanel[]		= "front_panel";

/*
 * Box type keywords
 */
char	_dtwmFptPrimary[]		= "primary";
char	_dtwmFptSecondary[]		= "secondary";
char	_dtwmFptSubpanel[]		= "subpanel";
char	_dtwmFptRowColumn[]		= "row_column";

/* 
 * Layout types
 */
char	_dtwmFptStandard[]		= "standard";
char	_dtwmFptAsNeeded[]		= "as_needed";

/* 
 * Orientation keywords
 */
char	_dtwmFptHorizontal[]		= "horizontal";
char	_dtwmFptVertical[]		= "vertical";

/* 
 * Subpanel Direction keywords
 */
char	_dtwmFptNorth[]		= "north";
char	_dtwmFptSouth[]		= "south";
char	_dtwmFptEast[]			= "east";
char	_dtwmFptWest[]			= "west";

/* 
 * Control-specific attribute keywords 
 */
char	_dtwmFpaClientGeometry[]	= "client_geometry";
char	_dtwmFpaClientName[]		= "client_name";
char	_dtwmFpaContextMenu[]		= "context_menu";
char	_dtwmFpaDropAction[]		= "drop_action";
char	_dtwmFpaDropAnimation[]	= "drop_animation";
char	_dtwmFpaPushAnimation[]	= "push_animation";
char	_dtwmFpaAlternateImage[]	= "alternate_image";
char	_dtwmFpaHelpString[]		= "help_string";
char	_dtwmFpaHelpTopic[]		= "help_topic";
char	_dtwmFpaImage[]		= "image";
char	_dtwmFpaLabel[]		= "label";
char	_dtwmFpaPushAction[]		= "push_action";
char	_dtwmFpaPushRecall[]		= "push_recall";
char	_dtwmFpaTrue[]			= "true";
char	_dtwmFpaType[]			= "type";
char	_dtwmFpaSubPanel[]		= "subpanel";

