/*******************************************************************************
**
**  help.h
**
**  #pragma ident "@(#)help.h	1.19 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: help.h /main/cde1_maint/1 1995/07/14 23:11:45 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _HELP_H
#define _HELP_H

/* for help */
#define HELP_OVERVIEW       			100
#define HELP_TASKS              		101
#define HELP_REFERENCE          		102
#define HELP_ONITEM             		103
#define HELP_USINGHELP          		104
#define HELP_ABOUTCALENDAR                  	105
#define HELP_TOC                        	106

/* location id's for on item help */
#define CALENDAR_HELP_FILE_BUTTON		"CalendarFileMenu"
#define CALENDAR_HELP_EDIT_BUTTON		"CalendarEditMenu"
#define CALENDAR_HELP_VIEW_BUTTON		"CalendarViewMenu"
#define CALENDAR_HELP_BROWSE_BUTTON		"CalendarBrowseMenu"
#define CALENDAR_HELP_HELP_BUTTON		"CalendarHelpMenu"
#define CALENDAR_HELP_APPT_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_TODO_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_PREV_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_NEXT_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_TODAY_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_DAY_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_WEEK_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_MONTH_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_YEAR_BUTTON		"CalendarToolBar"
#define CALENDAR_HELP_MONTH_WINDOW		"CalendarMonthWindow"
#define CALENDAR_HELP_WEEK_WINDOW		"CalendarWeekWindow"
#define CALENDAR_HELP_DAY_WINDOW		"CalendarDayWindow"
#define CALENDAR_HELP_YEAR_WINDOW		"CalendarYearWindow"

/* help buttons for popups */
#define OPTIONS_HELP_BUTTON		"CalendarOptionsCategoryDialog"
#define APPT_EDITOR_HELP_BUTTON		"CalendarApptEditorDialog"
#define TODO_EDITOR_HELP_BUTTON		"CalendarToDoEditorDialog"
#define FIND_HELP_BUTTON		"CalendarFindDialog"
#define	GOTO_HELP_BUTTON		"CalendarGotoDateDialog"
#define SHOW_OTHER_CAL_HELP_BUTTON	"CalendarShowOtherCalendarDialog"
#define COMPARE_CALS_HELP_BUTTON	"CalendarCompareCalsDialog"
#define MENU_EDITOR_HELP_BUTTON		"CalendarMenuEditorDialog"
#define GROUP_APPT_EDITOR_HELP_BUTTON	"CalendarGroupApptEditorDialog"
#define APPT_LIST_HELP_BUTTON		"CalendarApptListDialog"
#define TODO_LIST_HELP_BUTTON		"CalendarTodoListDialog"
#define PRINT_HELP_BUTTON		"CalendarPrintDialog"
#define TIMEZONE_HELP_BUTTON		"CalendarTimeZoneDialog"

/* help buttons for popups */
#define CMSD_ERROR_HELP			"CalendarCmsdError"
#define CAL_LOCATION_ERROR_HELP		"CalendarLocationError"
#define DND_ERROR_HELP			"CalendarDnDError"
#define RESELECT_ERROR_HELP		"CalendarReselectError"
#define DATE_ERROR_HELP			"CalendarDateFieldError"
#define START_ERROR_HELP		"CalendarStartFieldError"
#define STOP_ERROR_HELP			"CalendarStopFieldError"
#define MISSING_FIELD_ERROR_HELP	"CalendarMisssingFieldError"
#define REPEAT_FOR_ERROR_HELP		"CalendarRepeatForFieldError"
#define NO_TIME_ERROR_HELP		"CalendarNoTimeError"
#define BACK_END_ERROR_HELP		"CalendarBackEndError"
#define MEMORY_ALLOC_ERROR_HELP		"CalendarMemoryAllocError"
#define ACCESS_RIGHTS_ERROR_HELP	"CalendarAccessRightsError"
#define DESKSET_DEFAULTS_ERROR_HELP	"CalendarDesksetDefaultsError"
#define DURATION_ERROR_HELP		"CalendarDurationError"
#define ADVANCE_ERROR_HELP		"CalendarAdvanceError"

void 		show_main_help(Widget, XtPointer, XtPointer);
XtCallbackProc	help_cb	(Widget, XtPointer, XtPointer);
XtCallbackProc	help_view_cb(Widget, XtPointer, XtPointer);

#endif


