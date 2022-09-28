#pragma ident "@(#)fallback.h	1.20 96/11/12 Sun Microsystems, Inc."

/*******************************************************************************
**
**  fallbacks.h
**
**  $XConsortium: fallback.h /main/cde1_maint/2 1995/09/06 08:07:36 lehors $
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
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _FALLBACKS_H
#define _FALLBACKS_H

#include <X11/Intrinsic.h>

/*
 * Table of fallback resources.
 *
 * This is specified in the <fallback_resources> argument to XtAppInitialize
 * (calendarA.c), and tells Xt some basic resources to use if it can't load
 * an app-defaults file.
 */

String	fallback_resources[] = {
/* Mnemonics for Menu Items */
/* Menu Bar */
	"Dtcm*menuBar.file.mnemonic:			F",
	"Dtcm*menuBar.edit.mnemonic:			E",
	"Dtcm*menuBar.view.mnemonic:			V",
	"Dtcm*menuBar.browse.mnemonic:			B",
	"Dtcm*menuBar.help.mnemonic:			H",
/* File menu */
	"Dtcm*fileMenu.printCurrent.mnemonic:		C",
	"Dtcm*fileMenu.print.mnemonic:			P",
	"Dtcm*fileMenu.options.mnemonic:		O",
	"Dtcm*fileMenu.exit.mnemonic:			x",
/* Edit menu */
	"Dtcm*editMenu.appt.mnemonic:			A",
	"Dtcm*editMenu.toDo.mnemonic:			T",
/* View menu */
	"Dtcm*viewMenu.day.mnemonic:			D",
	"Dtcm*viewMenu.week.mnemonic:			W",
	"Dtcm*viewMenu.month.mnemonic:			M",
	"Dtcm*viewMenu.year.mnemonic:			Y",
	"Dtcm*viewMenu.apptList.mnemonic:		A",
	"Dtcm*viewMenu.toDoList.mnemonic:		T",
	"Dtcm*viewMenu.find.mnemonic:			F",
	"Dtcm*viewMenu.goTo.mnemonic:			G",
	"Dtcm*viewMenu.timeZone.mnemonic:		Z",
/* Browse menu */
	"Dtcm*browseMenu.showOther.mnemonic:		O",
	"Dtcm*browseMenu.compare.mnemonic:		C",
	"Dtcm*browseMenu.editMenu.mnemonic:		M",
/* Help menu */
	"Dtcm*helpMenu.overview.mnemonic:		O",
	"Dtcm*helpMenu.tasks.mnemonic:			T",
	"Dtcm*helpMenu.reference.mnemonic:		R",
	"Dtcm*helpMenu.onItem.mnemonic:			I",
	"Dtcm*helpMenu.using.mnemonic:			U",
	"Dtcm*helpMenu.about.mnemonic:			A",
/* Accelerators for selected commands */
	"Dtcm*day.accelerator:				Ctrl<Key>D",
	"Dtcm*week.accelerator: 			Ctrl<Key>W",
	"Dtcm*month.accelerator: 			Ctrl<Key>M",
	"Dtcm*year.accelerator: 			Ctrl<Key>Y",
	"Dtcm*options.accelerator: 			Ctrl<Key>I",
	"Dtcm*appt.accelerator: 			Ctrl<Key>A",
	"Dtcm*toDo.accelerator: 			Ctrl<Key>T",
	"Dtcm*find.accelerator: 			Ctrl<Key>F",
	"Dtcm*print.accelerator: 			Ctrl<Key>P",
	"Dtcm*exit.accelerator: 			Alt<Key>F4",
	"Dtcm*onItem.accelerator: 			<Key>F1",
/* Accelerator Text for above */
	"Dtcm*day.acceleratorText:			Ctrl+D",
	"Dtcm*week.acceleratorText: 			Ctrl+W",
	"Dtcm*month.acceleratorText: 			Ctrl+M",
	"Dtcm*year.acceleratorText: 			Ctrl+Y",
	"Dtcm*options.acceleratorText: 			Ctrl+I",
	"Dtcm*appt.acceleratorText: 			Ctrl+A",
	"Dtcm*toDo.acceleratorText: 			Ctrl+T",
	"Dtcm*find.acceleratorText: 			Ctrl+F",
	"Dtcm*print.acceleratorText: 			Ctrl+P",
	"Dtcm*exit.acceleratorText: 			Alt+F4",
	"Dtcm*onItem.acceleratorText: 			F1",
/* Tear-offs */
	"Dtcm*fileMenu.tearOffModel: tear_off_enabled",
	"Dtcm*editMenu.tearOffModel: tear_off_enabled",
	"Dtcm*viewMenu.tearOffModel: tear_off_enabled",
	"Dtcm*browseMenu.tearOffModel: tear_off_enabled",
	"Dtcm*helpMenu.tearOffModel: tear_off_enabled",
/* Programmable date formats */
	"Dtcm*XmMonthPanel.titleFormat: %B %Y",
	"Dtcm*yearForm.XmMonthPanel.titleFormat: %B",
/* Miscellaneous */
	"Dtcm*XmMonthPanel*fontList: -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
	"Dtcm*canvas*XmPushButton.fontList: -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
	"Dtcm*labelForm*fontList: -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
	"Dtcm*XmMonthPanel.activeHeader: True",
	"Dtcm*XmMonthPanel*recomputeSize: False",
        "Dtcm*lastMonth.activeDays: True",
        "Dtcm*thisMonth.activeDays: True",
        "Dtcm*nextMonth.activeDays: True",
	"Dtcm.iconFont:  -dt-application-bold-r-normal-sans-*-180-*-*-p-*-*-*:",
	"Dtcm.applicationFontFamily: application",
	NULL
};

#endif
