/*****************************************************************************
 *****************************************************************************
 **
 **   File:         dtdnddemo.c
 **
 **   Description:  The Drag-n-Drop Demo program demonstrates the
 **		    CDE DnD functions in the Desktop Services library:
 **		  
 **		    	DtDndDragStart.3x
 **		    	DrDropRegister.3x
 **		  
 **		    The demo consists of a row of three different
 **		    sources for text, filename and app-named data drags.
 **		    It also has a type-in field that can accept either
 **		    text or filename drops.  Finally there is a data
 **		    area that accepts filename or data drops.
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>

#include <Xm/DragDrop.h>
#include <Xm/Screen.h>

#include <Dt/Dt.h>
#include <Dt/Dnd.h>

 /*************************************************************************
 *
 *	Structures, defines, global data
 *
 *************************************************************************/

#define DRAG_THRESHOLD 4

/* Absolute value macro */
#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#endif

/*
 * Data for text list of fruit
 */

char *todaysFruit[] = {
	"Oranges",
	"Peaches",
	"Lemons",
	"Watermelons",
	"Apples",
	"Bananas",
	"Plums",
	"Limes",
	"Cantaloupes",
	"Nectarines",
	"Papayas",
	"Mangos",
	NULL
};

/*
 * Types for dnd of icons
 */

#define FOLDER_STRING	"Folder"
#define FILE_STRING	"File"
#define APPT_STRING	"Appointment"

#define ICON_TEXT_YGAP	2

typedef struct _IconInfo {
	XRectangle	icon;
	int		iconType;
	char	       *name;
	Pixmap		bitmap;
	Pixmap		mask;
	Widget		dragIcon;
	struct _IconInfo *next;
} IconInfo;

typedef enum _IconType {
	SourceIconType,
	FileIconType,
	FolderIconType,
	TextIconType,
	ApptIconType,
	TotalIconTypes
} IconType;

/*
 * Data and structures for appointment list
 */

char	today[9];	/* initialized in createAppointmentList() */

typedef struct _Appointment {
	char	       *date;
	char	       *start;
	char	       *end;
	char	       *what;
} Appointment;

Appointment todaysApptList[] = {
	{ today, " 9:00 am", " 9:30 am", "Staff Meeting" },
	{ today, " 9:30 am", "10:00 am", "Will's Party" },
	{ today, "10:00 am", "10:30 am", "Conference Call" },
	{ today, "10:30 am", "11:30 am", "Work on Mail" },
	{ today, "11:00 am", "11:30 pm", "B'fast w/ Robert" },
	{ today, " 1:30 pm", " 2:30 pm", "Design Meeting" },
	{ today, " 3:00 pm", " 4:00 pm", "Communications" },
	{ today, " 4:00 pm", " 4:30 pm", "Pick up Dogs" },
	{ today, " 5:00 pm", " 6:30 pm", "Beer Bust" },
	{ today, " 7:00 pm", " 9:00 pm", "Dinner - Stuart" },
	{ NULL, NULL, NULL, NULL }
};

char *apptFormat =
"	** Calendar Appointment **\n\
\n\
	Date:	 %s\n\
	Start:	 %s\n\
	End:	 %s\n\
	What:	 %s\n";

/*
 * Global variables
 */

Widget		topLevel;
XtAppContext	appContext;
Boolean		doingDrag = False;
Pixel		motifBackground, motifForeground, motifSelect;

/*
 * Icon bitmap data
 */

/* All icon bitmaps here are 32 by 32 */
#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 32

#define appt_bm_width 32
#define appt_bm_height 32
static unsigned char appt_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xfe, 0xfd, 0xf7, 0x9f,
   0x01, 0x01, 0x04, 0xc8, 0x01, 0x05, 0x14, 0x74, 0x81, 0x05, 0x16, 0x3a,
   0x81, 0x05, 0x16, 0x59, 0x01, 0x03, 0x8c, 0x2c, 0x01, 0x00, 0x40, 0x56,
   0x01, 0x00, 0x20, 0x33, 0x01, 0x2b, 0x96, 0x51, 0x01, 0x22, 0xc9, 0x38,
   0x01, 0x12, 0x65, 0x55, 0x01, 0x7f, 0xb2, 0x3a, 0x01, 0x00, 0x59, 0x55,
   0x19, 0x80, 0xac, 0x30, 0x59, 0x55, 0x56, 0x50, 0x01, 0x20, 0x0b, 0x30,
   0x19, 0x90, 0x05, 0x50, 0x59, 0xd5, 0xaa, 0x32, 0x01, 0x68, 0x01, 0x50,
   0x19, 0x18, 0x00, 0x30, 0x59, 0x45, 0x55, 0x51, 0x01, 0x00, 0x00, 0x30,
   0x01, 0x00, 0x00, 0x50, 0xfe, 0xff, 0xff, 0x3f, 0x54, 0x55, 0x55, 0x55,
   0xa8, 0xaa, 0xaa, 0x2a, 0x00, 0x00, 0x00, 0x00
};

#define appt_m_bm_width 32
#define appt_m_bm_height 32
static unsigned char appt_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xfe, 0xfd, 0xf7, 0xff,
   0xff, 0xfd, 0xf7, 0xff, 0xff, 0xfd, 0xf7, 0x7f, 0xff, 0xfd, 0xf7, 0x3f,
   0xff, 0xfd, 0xf7, 0x7f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f,
   0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x3f,
   0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f,
   0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x3f,
   0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f,
   0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x3f,
   0xff, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0xff, 0x7f,
   0xa8, 0xaa, 0xaa, 0x2a, 0x00, 0x00, 0x00, 0x00
};

#define data_bm_width 32
#define data_bm_height 32
static unsigned char data_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x0f, 0x08, 0x00, 0x00, 0x08,
   0x08, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08, 0x88, 0x9d, 0x6d, 0x08,
   0x08, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08, 0x88, 0xf9, 0xed, 0x08,
   0x08, 0x00, 0x00, 0x08, 0xc8, 0x6e, 0x7d, 0x08, 0x08, 0x00, 0x00, 0x08,
   0xc8, 0x9d, 0xef, 0x09, 0x08, 0x00, 0x00, 0x08, 0xc8, 0xdd, 0xb3, 0x08,
   0x08, 0x00, 0x00, 0x08, 0xc8, 0xdf, 0xfd, 0x09, 0x08, 0x00, 0x00, 0x08,
   0xc8, 0xf6, 0xde, 0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08,
   0x88, 0xed, 0xd2, 0x08, 0x08, 0x00, 0x00, 0x08, 0xc8, 0xbb, 0xf3, 0x08,
   0x08, 0x00, 0x00, 0x08, 0x48, 0xdf, 0x5e, 0x08, 0x08, 0x00, 0x00, 0x08,
   0xc8, 0x36, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08,
   0x08, 0x00, 0x00, 0x08, 0xf8, 0xff, 0xff, 0x0f};

#define data_m_bm_width 32
#define data_m_bm_height 32
static unsigned char data_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f};

#define folder_bm_width 32
#define folder_bm_height 32
static unsigned char folder_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f,
   0x00, 0x00, 0xfc, 0x7f, 0xf8, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x80,
   0xfe, 0xff, 0xff, 0xbf, 0x02, 0x00, 0x00, 0xa0, 0x52, 0x55, 0x55, 0xb5,
   0xaa, 0xaa, 0xaa, 0xba, 0x52, 0x55, 0x55, 0xb5, 0xaa, 0xaa, 0xaa, 0xba,
   0x52, 0x55, 0x55, 0xb5, 0xaa, 0xaa, 0xaa, 0xba, 0x52, 0x55, 0x55, 0xb5,
   0xaa, 0xaa, 0xaa, 0xba, 0x52, 0x55, 0x55, 0xb5, 0xaa, 0xaa, 0xaa, 0xba,
   0x52, 0x55, 0x55, 0xb5, 0xaa, 0xaa, 0xaa, 0xba, 0x52, 0x55, 0x55, 0xb5,
   0xaa, 0xaa, 0xaa, 0xba, 0x52, 0x55, 0x55, 0xb5, 0xaa, 0xaa, 0xaa, 0xba,
   0xfa, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x7f};

#define folder_m_bm_width 32
#define folder_m_bm_height 32
static unsigned char folder_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f,
   0x00, 0x00, 0xfc, 0x7f, 0xf8, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x7f};

#define source_bm_width 32
#define source_bm_height 32
static unsigned char source_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x7f, 0x00, 0x10, 0x00, 0xc0, 0x00,
   0x10, 0x00, 0x40, 0x01, 0x10, 0x00, 0x40, 0x02, 0x10, 0x00, 0x40, 0x04,
   0x10, 0x00, 0xc0, 0x0f, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08,
   0x10, 0x00, 0x00, 0x08, 0xf0, 0xff, 0xff, 0x0f};

#define source_m_width 32
#define source_m_height 32
static unsigned char source_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x7f, 0x00, 0xf0, 0xff, 0xff, 0x00,
   0xf0, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x07,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0f};

#define text_bm_width 32
#define text_bm_height 32
static unsigned char text_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xb7, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x60, 0xb7, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xce, 0xf7, 0x03,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0xee, 0xd9, 0x01, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0xef, 0xfe, 0x03, 0x00, 0x00, 0x00, 0x00, 0x60, 0x7b, 0xef, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define text_m_bm_width 32
#define text_m_bm_height 32
static unsigned char text_m_bm_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xf8, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


 /*************************************************************************
 *
 *	Utility Functions
 *
 *************************************************************************/


/*
 * typeIconByName
 *
 * Return an icon type based on a string
 */
IconType
typeIconByName(
	char	       *iconName)
{
	if (strcmp(iconName, FOLDER_STRING) == 0) {
		return FolderIconType;
	}

	if (strcmp(iconName, FILE_STRING) == 0) {
		return FileIconType;
	}

	if (strcmp(iconName, APPT_STRING) == 0) {
		return ApptIconType;
	}

	return SourceIconType;
}

/*
 * getIconByType
 *
 * Returns a new IconInfo structure with bitmap, mask, width, height,
 * icon type and name.
 */
IconInfo*
getIconByType(
	IconType	iconType,
	char	       *iconName)
{
	IconInfo       *iconInfo = (IconInfo *) XtMalloc(sizeof(IconInfo));
	Display	       *display = XtDisplayOfObject(topLevel);
	Window	        window = XtWindow(topLevel);
	unsigned char  *bitmapData, *bitmapMask;
	static IconInfo icons[TotalIconTypes];
	static Boolean  firstTime = True;

	if (firstTime) {
		memset(&icons, 0x00, sizeof(IconInfo) * TotalIconTypes);
		firstTime = False;
	}

	switch (iconType) {
	case FileIconType:
		bitmapData = data_bm_bits;
		bitmapMask = data_m_bm_bits;
		break;
	case FolderIconType:
		bitmapData = folder_bm_bits;
		bitmapMask = folder_m_bm_bits;
		break;
	case TextIconType:
		bitmapData = text_bm_bits;
		bitmapMask = text_m_bm_bits;
		break;
	case ApptIconType:
		bitmapData = appt_bm_bits;
		bitmapMask = appt_m_bm_bits;
		break;
	default:
		bitmapData = source_bm_bits;
		bitmapMask = source_m_bm_bits;
		break;
	}
	if (icons[iconType].bitmap == NULL) {
		icons[iconType].bitmap = XCreateBitmapFromData(display,
			window, (char *) bitmapData,
			BITMAP_WIDTH, BITMAP_HEIGHT);
		if (icons[iconType].bitmap == NULL) {
			printf("XCreateBitmapFromData() failed for bitmap.\n");
			return NULL;
		}	
	}
	if (icons[iconType].mask == NULL) {
		icons[iconType].mask = XCreateBitmapFromData(display,
			window, (char *) bitmapMask,
			BITMAP_WIDTH, BITMAP_HEIGHT);
		if (icons[iconType].mask == NULL) {
			printf("XCreateBitmapFromData() failed for mask.\n");
			return NULL;
		}	
		icons[iconType].icon.width = BITMAP_WIDTH;
		icons[iconType].icon.height = BITMAP_HEIGHT;
	}	

	iconInfo->icon		= icons[iconType].icon;
	iconInfo->iconType 	= iconType;
	iconInfo->bitmap 	= icons[iconType].bitmap;
	iconInfo->mask 		= icons[iconType].mask;
	iconInfo->dragIcon	= (Widget)NULL;
	iconInfo->next 		= (IconInfo *)NULL;
	iconInfo->name 		= XtNewString(iconName);

	return iconInfo;
}

/*
 * drawIcon
 *
 * Copy icon bitmap (based on mask) into given window with the colors
 * in the given graphics context. Render icon name centered below the icon.
 */
void
drawIcon(
	IconInfo       *iconPtr,
	GC		graphicsContext,
	Window		window)
{
	XGCValues	gcValues;
	Display	       *display = XtDisplayOfObject(topLevel);
	Screen	       *screen = XtScreen(topLevel);

	/*
	 * Draw icon
	 */
	if (iconPtr->bitmap != NULL && iconPtr->mask != NULL) {
		gcValues.clip_mask = iconPtr->mask;
		gcValues.clip_x_origin = iconPtr->icon.x;
		gcValues.clip_y_origin = iconPtr->icon.y;
		gcValues.foreground = BlackPixelOfScreen(screen);
		gcValues.background = WhitePixelOfScreen(screen);
		XChangeGC(display, graphicsContext,
			GCClipMask | GCClipXOrigin | GCClipYOrigin |
			GCForeground | GCBackground,
			&gcValues);
		XCopyPlane(display, iconPtr->bitmap, window,
			graphicsContext, 0, 0,
			iconPtr->icon.width, iconPtr->icon.height,
			iconPtr->icon.x, iconPtr->icon.y, 1L);
	}
	/*
	 * Render icon name string centered below icon
	 */
	if (iconPtr->name != NULL) {
		int 		nameX, nameY;
		int		direction, ascent, decent;
		XCharStruct	overall;
		static XFontStruct *fontStruct;
		Widget          xmScreen;

		if (fontStruct == NULL) {
			xmScreen = XmGetXmScreen(XtScreen(topLevel));
			XtVaGetValues(xmScreen, XmNfont, &fontStruct, NULL);
		}
		XTextExtents(fontStruct, iconPtr->name, strlen(iconPtr->name),
			&direction, &ascent, &decent, &overall);

		nameX = (iconPtr->icon.x + (iconPtr->icon.width/2)) -
			(overall.width/2);
		nameY = iconPtr->icon.y + iconPtr->icon.height +
			ICON_TEXT_YGAP + ascent;

		gcValues.font = fontStruct->fid;
		gcValues.clip_mask = None;
		gcValues.foreground = motifForeground;
		gcValues.background = motifBackground;
		XChangeGC(display, graphicsContext,
			GCFont | GCClipMask | GCForeground | GCBackground,
			&gcValues);
		XDrawString(display, window, graphicsContext, nameX, nameY,
			iconPtr->name, strlen(iconPtr->name));
	}
}

/*
 * getApptLabel
 *
 * Creates a label for an appointment icon given an appointment.
 */
char *
getApptLabel(
	char		*appt)
{
	char		start[128];
	int		count;

	if (appt == NULL) {
		return NULL;
	}

	if ((appt = strstr(appt, "Start:")) == NULL) {
		return NULL;
	}
	
	count = sscanf(appt, "Start:%*[ \t]%[^\n]", start);

	if (count != 1) {
		return APPT_STRING;
	}

	return XtNewString(start);
}

/*
 * getApptFromListEntry
 *
 * Returns the full appointment based on the text of the appointment as
 * given in the text entry from the scrolled list of appointments.
 */
Appointment*
getApptFromListEntry(
	XmString	listEntry)
{
	int		ii;
	char		*entryText,
			*string;

	/*
	 * Get text string from XmString for use in comparisons
	 */

	XmStringGetLtoR(listEntry, XmFONTLIST_DEFAULT_TAG, &entryText);
	
	for (ii = 0; todaysApptList[ii].what != NULL; ii++) {
		string = strstr(entryText, todaysApptList[ii].what);
		if (string != NULL) {
			XtFree(entryText);
			return(&todaysApptList[ii]);
		}
	}
	XtFree(entryText);
	return NULL;
}

 /*************************************************************************
 *
 *	Drag & Drop Callbacks and Functions
 *
 *************************************************************************/

/*
 * showErrorNotice
 *
 * Constructs an error message using the given parameters. Displays the
 * error message in a notice and asks the user whether or not to continue.
 */

void
errorNoticeCallback(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	XmAnyCallbackStruct *callInfo = (XmAnyCallbackStruct *) callData;

	if (callInfo->reason == XmCR_OK) {
		XtVaSetValues(widget, XmNuserData, DtDND_SUCCESS, NULL);
	} else {
		XtVaSetValues(widget, XmNuserData, DtDND_FAILURE, NULL);
	}
}

/*
 * convertApptCB
 *
 * Fills in buffer object with calendar appointment string based on which
 * appointments are selected in the scrolled list when the drag is started.
 * When no appointments are selected, the appointment under the pointer is
 * used. Supply a label for the calendar appointment based on the contents
 * of the appointment.
 */
void
convertApptCB(
	Widget		dragContext,
	XtPointer	clientData,
	XtPointer	callData)
{
	DtDndConvertCallbackStruct *convertInfo =
				  (DtDndConvertCallbackStruct *) callData;
	DtDndBuffer	*buffers = convertInfo->dragData->data.buffers;
	Widget		apptList = (Widget)clientData;
	int		selectedPos, ii,
			selectedItemCount;
	XmStringTable	selectedItems;
	char		apptStr[1024];
	String		labelString;
	Appointment	*appt;

	/*
	 * Verify the validity of the callback reason
	 */

	if (convertInfo->reason != DtCR_DND_CONVERT_DATA &&
	    convertInfo->reason != DtCR_DND_CONVERT_DELETE) {
		convertInfo->status = DtDND_FAILURE;
		return;
	}

	/*
	 * Get selected items from the list
	 */

	XtVaGetValues(apptList,
		XmNuserData,		&selectedPos,
		XmNselectedItemCount,	&selectedItemCount,
		XmNselectedItems,	&selectedItems,
		NULL);

	for (ii = 0; ii < convertInfo->dragData->numItems; ii++) {

		/*
		 * Get the actual appointment(s)
		 */

		if (selectedItemCount == 0) {
			appt = &todaysApptList[selectedPos-1+ii];
		} else {
			appt = getApptFromListEntry(selectedItems[ii]);
		}
		sprintf(apptStr, apptFormat,
			appt->date, appt->start, appt->end, appt->what);

		/*
	 	 * Supply the appointment(s) for transfer 
		 */

		if (convertInfo->reason == DtCR_DND_CONVERT_DATA) {

			/* Copy the appointment into the buffer for transfer */

			buffers[ii].bp = XtNewString(apptStr);
			buffers[ii].size = strlen(buffers[ii].bp);

			/* Supply the name for the appointment */

			labelString = getApptLabel(apptStr);
			buffers[ii].name = (char *) XtNewString(labelString);
			XtFree(labelString);

		/*
		 * Delete the moved appointment(s)
		 */

		} else if (convertInfo->reason == DtCR_DND_CONVERT_DELETE) {
			printf("Delete appointment for %s\n", appt->what);
		}
	}
}

/*
 * convertFileCB
 *
 * Fills in file object's file name based on the name associated with
 * the file's icon.  Ensures that the file exists and has some minimal
 * content.
 */
void
convertFileCB(
	Widget		dragContext,
	XtPointer	clientData,
	XtPointer	callData)
{
	DtDndConvertCallbackStruct *convertInfo =
					(DtDndConvertCallbackStruct *) callData;
	IconInfo        *iconArray = (IconInfo *) clientData;
	char		*fileName;
	FILE		*filePtr;
	int		ii, numFiles;

	switch (convertInfo->reason) {
	case DtCR_DND_CONVERT_DATA:

		numFiles = convertInfo->dragData->numItems;

		for (ii = 0; ii < numFiles; ii++) {

			fileName = iconArray[ii].name;

			convertInfo->dragData->data.files[ii] = fileName;


			if ((filePtr = fopen(fileName, "w")) == NULL) {
				convertInfo->status = DtDND_FAILURE;
				return;
			}

			if (fwrite(fileName,strlen(fileName),1,filePtr) != 1) {
				convertInfo->status = DtDND_FAILURE;
				return;
			}

			if (strcmp(fileName,APPT_STRING) == 0) {
				rewind(filePtr);
				fprintf(filePtr, apptFormat,
					today, "11:45 am", " 1:00 pm", "Lunch");
			}

			fclose(filePtr);
		}
		break;

	case DtCR_DND_CONVERT_DELETE:

		numFiles = convertInfo->dragData->numItems;

		printf("Delete file(s):\n");
		for (ii = 0; ii < numFiles; ii++) {
			printf("\t\"%s\"\n", iconArray[ii].name);
		}
		break;
	}
}

/*
 * convertTextCB
 *
 * Sets the text object's text to the text in the fruit list based on where
 * the pointer was when the drag started.
 */
void
convertTextCB(
	Widget		dragContext,
	XtPointer	clientData,
	XtPointer	callData)
{
	DtDndConvertCallbackStruct *convertInfo =
					(DtDndConvertCallbackStruct *) callData;
	Widget		fruitList = (Widget) clientData;
	int		selectedPos;
	XmString       *items;
	Cardinal	itemCount;

	if (fruitList == NULL)
		return;

	switch (convertInfo->reason) {
	case DtCR_DND_CONVERT_DATA:

		XtVaGetValues(fruitList, 
			XmNuserData, 	&selectedPos, 
			XmNitems, 	&items,
			XmNitemCount, 	&itemCount,
			NULL);

		if (itemCount > 0 && selectedPos < itemCount) {
			convertInfo->dragData->data.strings[0] = 
					items[selectedPos-1];
		} else {
			convertInfo->status = DtDND_FAILURE;
		}
		break;
	DtCR_DND_CONVERT_DELETE:
		printf("Delete fruit item #%d\n", 	
			convertInfo->dragData->data.strings[0]);
		break;
	}
}

/*
 * dragFinishCB
 *
 * Resets drag state to indicate the drag is over. Free memory allocated 
 * with the drag.
 */
static void
dragFinishCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	doingDrag = False;
	/* REMIND: Free any memory allocated for the drag */
}

/*
 * drawTransferCB
 *
 * Handles the transfer of a file or appointment to the draw area.
 * Adds the appropriate icon to the list of icons on the draw area.
 */
void
drawTransferCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	Display	       *display	= XtDisplay(widget);
	DtDndTransferCallbackStruct *transferInfo =
				(DtDndTransferCallbackStruct *) callData;
	IconType	iconType;
	IconInfo       *iconList = NULL, *iconPtr;
	char	       *filePath, *name;
	int		ii, numItems;

        switch (transferInfo->dropData->protocol) {
	case DtDND_FILENAME_TRANSFER:

		numItems = transferInfo->dropData->numItems;

		printf("FileDrop: \n");

		for (ii = 0; ii < numItems; ii++) {
			filePath = transferInfo->dropData->data.files[ii];
			printf("\t\"%s\"\n",filePath);
			if ((name = strrchr(filePath,'/')) == NULL) {
				name = filePath;
			} else {
				name++;
			}
			iconType = typeIconByName(name);
			iconPtr = getIconByType(iconType, name);
			iconPtr->next = iconList;
			iconPtr->icon.x = transferInfo->x + ii * 10;
			iconPtr->icon.y = transferInfo->y + ii * 10;

			XtVaSetValues(widget, XmNuserData, iconPtr, NULL);

			iconList = iconPtr;
		}
		break;
	case DtDND_BUFFER_TRANSFER:
		
		numItems = transferInfo->dropData->numItems;

		printf("BufferDrop: \n");

		for (ii = 0; ii < numItems; ii++) {
			XtVaGetValues(widget, XmNuserData, &iconList, NULL);

			name = getApptLabel(
				transferInfo->dropData->data.buffers[ii].bp);
			if (name == NULL) {
				name = 
				  transferInfo->dropData->data.buffers[ii].name;
			}
			printf("\t\"%s\"\n", name);

			iconType = ApptIconType;

			iconPtr = getIconByType(iconType, name);
			iconPtr->next = iconList;
			iconPtr->icon.x = transferInfo->x + ii * 10;
			iconPtr->icon.y = transferInfo->y + ii * 10;

			XtVaSetValues(widget, XmNuserData, iconPtr, NULL);

			iconList = iconPtr;
		}
		break;
        default:
                transferInfo->status = DtDND_FAILURE;
		return;
        }

}

/*
 * drawAnimateCB
 *
 */
void
drawAnimateCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	/* WARNING: widget is the dragContext which is being destroyed */

	Widget		dropDraw 	= (Widget)clientData;
	Display        *display 	= XtDisplayOfObject(dropDraw);
	Screen	       *screen 		= XtScreen(dropDraw);
	Window          window 		= XtWindow(dropDraw);
	DtDndTransferCallbackStruct *animateInfo =
					(DtDndTransferCallbackStruct *) callData;
	int		expandWidth, expandHeight, sourceX, sourceY;
static 	GC		graphicsContext = NULL;
	XGCValues	gcValues;
	IconInfo       *iconPtr;
	
	/* Create GC if we haven't */

	if (graphicsContext == NULL) {
		gcValues.foreground = BlackPixelOfScreen(screen);
		gcValues.background = WhitePixelOfScreen(screen);
		graphicsContext = XCreateGC(display, window,
				GCForeground | GCBackground, &gcValues);
	}

	/* Get the dragged icon from the dropDraw area */

	XtVaGetValues(dropDraw, XmNuserData, &iconPtr, NULL);

	if (iconPtr == NULL)
		return;

	/* Set the bitmaps and coordinates for this icon in the GC */

	gcValues.clip_mask = iconPtr->mask;
	gcValues.clip_x_origin = iconPtr->icon.x;
	gcValues.clip_y_origin = iconPtr->icon.y;
	XChangeGC(display, graphicsContext,
			GCClipMask | GCClipXOrigin | GCClipYOrigin,
			&gcValues);

	/* Reconstitute the icon after Motif melts it */

	for (expandWidth = expandHeight = 0;
	     expandWidth < (int)iconPtr->icon.width &&
	     	expandHeight < (int)iconPtr->icon.height;
	     expandWidth += 2, expandHeight += 2) {

		sourceX = ((int)iconPtr->icon.width - expandWidth)/2;
		sourceY = ((int)iconPtr->icon.height - expandHeight)/2;

		XCopyPlane(display, iconPtr->bitmap, window,
			graphicsContext, sourceX, sourceY,
			expandWidth, expandHeight,
			iconPtr->icon.x + sourceX, iconPtr->icon.y + sourceY,
			1L);

		_XmMicroSleep(25000L);
		XFlush(display);
	}

}

/*
 * textTransferCB
 *
 * Handles transfer of files or text to the text edit. Files are transfered
 * by placing their name in the field, text by inserting the text into the
 * field.
 */
void
textTransferCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	DtDndTransferCallbackStruct *transferInfo =
					(DtDndTransferCallbackStruct *) callData;
	String 	text;

        switch (transferInfo->dropData->protocol) {

	case DtDND_FILENAME_TRANSFER:
	
		XtVaSetValues(widget, 
			XmNvalue, transferInfo->dropData->data.files[0], 
			NULL);

		break;

        case DtDND_TEXT_TRANSFER:

		XmStringGetLtoR(transferInfo->dropData->data.strings[0],
			XmFONTLIST_DEFAULT_TAG, &text);

		XtVaSetValues(widget, XmNvalue, text, NULL);

		break;
	}
}

/*
 * textAnimateCB
 *
 * Not implemented.
 */
void
textAnimateCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	/* REMIND: Do something! */
}

/*
 * dropDrawExposeCB
 *
 * Draws all the icons that have been dropped on the draw area.
 */
void
dropDrawExposeCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	static GC	graphicsContext = NULL;
	XGCValues	gcValues;
	IconInfo       *iconPtr;
	Display	       *display = XtDisplayOfObject(widget);
	Screen         *screen = XtScreen(widget);
	Window		window = XtWindow(widget);

	XtVaGetValues(widget, XmNuserData, &iconPtr, NULL);

	while (iconPtr != NULL) {
		if (graphicsContext == NULL) {
			gcValues.foreground = BlackPixelOfScreen(screen);
			gcValues.background = WhitePixelOfScreen(screen);
			graphicsContext = XCreateGC(display, window,
				GCForeground | GCBackground, &gcValues);
		}
		drawIcon(iconPtr, graphicsContext, window);
		iconPtr = iconPtr->next;
	}
}

/*
 * fileDrawExposeCB
 *
 * Draws all the icons that reside in the file drag source area.
 * Creates the file icons the first time it is called.
 */
void
fileDrawExposeCB(
	Widget		widget,
	XtPointer	clientData,
	XtPointer	callData)
{
	static Boolean	firstTime = True;
	static GC	graphicsContext;	
	Display	       *display = XtDisplayOfObject(widget);
	Window		window = XtWindow(widget);
	Screen	       *screen = XtScreen(widget);
	XGCValues	gcValues;
	IconInfo       *iconPtr = NULL;

	if (firstTime) {
		iconPtr = getIconByType(FolderIconType, FOLDER_STRING);
		iconPtr->icon.x = 30;
		iconPtr->icon.y = 10;

		iconPtr->next = getIconByType(FileIconType, FILE_STRING);
		iconPtr->next->icon.x = 90;
		iconPtr->next->icon.y = 10;

		iconPtr->next->next = getIconByType(ApptIconType, APPT_STRING);
		iconPtr->next->next->icon.x = 60;
		iconPtr->next->next->icon.y = 70;

		/* Create GC for rendering */
		gcValues.foreground = BlackPixelOfScreen(screen);
		gcValues.background = WhitePixelOfScreen(screen);
		graphicsContext = XCreateGC(display, window,
			GCForeground | GCBackground, &gcValues);
			
		XtVaSetValues(widget, XmNuserData, (XtPointer)iconPtr, NULL);
		firstTime = False;
	} else {
		XtVaGetValues(widget, XmNuserData, &iconPtr, NULL);
	}
	while (iconPtr != NULL) {
		drawIcon(iconPtr, graphicsContext, window);
		iconPtr = iconPtr->next;
	}
}

/*
 * startTextDrag
 *
 * Initiates a drag of a text item from the fruit list provided the pointer
 * is over an item in the list.
 */
void
startTextDrag(
	Widget		widget,
	XEvent	       *event)
{
	static XtCallbackRec convertCBRec[] = { {convertTextCB, NULL},
					        {NULL, NULL} };
	static XtCallbackRec dragFinishCBRec[] =  { {dragFinishCB, NULL},
						    {NULL, NULL} };
	int		itemCount, selectedPos;

	XtVaGetValues(widget, XmNitemCount, &itemCount, NULL);

	selectedPos = XmListYToPos(widget, event->xmotion.y);

	if (selectedPos == 0 || selectedPos > itemCount) {
		return;
	}

	XtVaSetValues(widget, XmNuserData, selectedPos, NULL);

	convertCBRec[0].closure = (XtPointer)widget;

	if (DtDndDragStart(widget, event, DtDND_TEXT_TRANSFER, 1,
	    XmDROP_COPY, convertCBRec, dragFinishCBRec, NULL, 0)
	    == NULL) {
		
		printf("DragStart returned NULL.\n");
	}
}

/*
 * startApptDrag
 *
 * Initiates a drag of an appointment from the appointment list provided
 * the pointer is over an appointment in the list.
 */
void
startApptDrag(
	Widget		widget,
	XEvent		*event)
{
	static XtCallbackRec convertCBRec[] = { {convertApptCB, NULL},
					        {NULL, NULL} };
	static XtCallbackRec dragFinishCBRec[] =  { {dragFinishCB, NULL},
						    {NULL, NULL} };
	static IconInfo *iconPtr = NULL;
	Widget		dragIcon;
	Display		*display = XtDisplay(widget);
	int		itemCount,
			selectedPos,
			selectedItemCount;

	XtVaGetValues(widget,
		XmNitemCount, &itemCount,
		XmNselectedItemCount, &selectedItemCount,
		NULL);

	selectedPos = XmListYToPos(widget, event->xmotion.y);

	if (selectedPos == 0 || selectedPos > itemCount) {
		return;
	}

	XtVaSetValues(widget, XmNuserData, selectedPos, NULL);

	if (iconPtr == NULL) {
		iconPtr = getIconByType(ApptIconType, APPT_STRING);
	}

	if (iconPtr->dragIcon == NULL) {
		iconPtr->dragIcon = DtDndCreateSourceIcon(widget,
			iconPtr->bitmap, iconPtr->mask);
	}
	if (selectedItemCount > 1) {
		dragIcon = NULL;
		itemCount = selectedItemCount;
	} else {
		dragIcon = iconPtr->dragIcon;
		itemCount = 1;
	}
	convertCBRec[0].closure = (XtPointer)widget;

	if (DtDndVaDragStart(widget, event, DtDND_BUFFER_TRANSFER, itemCount,
	    		XmDROP_COPY | XmDROP_MOVE,
			convertCBRec, dragFinishCBRec,
			DtNsourceIcon, dragIcon,
			NULL)
	    == NULL) {
		
		printf("DragStart returned NULL.\n");
	}
}

/*
 * startFileDrag
 *
 * Initiates a file drag. The function checkForFileDrag() first determines
 * if the pointer is over a file icon.
 */
void
startFileDrag(
	Widget		widget,
	XEvent	       *event,
	IconInfo       *iconArray,
	int		numFiles)
{
	static XtCallbackRec convertCBRec[] = { {convertFileCB, NULL},
					        {NULL, NULL} };
	static XtCallbackRec dragFinishCBRec[] =  { {dragFinishCB, NULL},
						    {NULL, NULL} };
	Widget		dragIcon;
	Arg		arg[1];

	convertCBRec[0].closure = (XtPointer) iconArray;

	if (iconArray[0].dragIcon == NULL) {
		iconArray[0].dragIcon = DtDndCreateSourceIcon(widget,
			iconArray[0].bitmap, iconArray[0].mask);
	}

	if (numFiles == 1) {
		dragIcon = iconArray[0].dragIcon;
	} else {
		dragIcon = NULL; /* Use default multiple provided by library */
	}

	XtSetArg(arg[0], DtNsourceIcon, (XtArgVal)dragIcon);

	if (DtDndDragStart(widget, event, DtDND_FILENAME_TRANSFER, numFiles,
			XmDROP_COPY | XmDROP_MOVE,
			convertCBRec, dragFinishCBRec, arg, 1)
    	    == NULL) {
		
		printf("DragStart returned NULL.\n");
	}
}

/*
 * checkForFileDrag
 *
 * Determine if the pointer is over a file icon (within the drag threshold)
 * when button 2 is pressed or when button 1 was pressed and the drag
 * threshold has been exceeded.
 */
XtActionProc
checkForFileDrag(
	Widget		widget,
	XEvent	       *event,
	String	       *params,
	Cardinal       *numParams)
{
	IconInfo       *iconList, *iconPtr, *iconArray;

	XtVaGetValues(widget, XmNuserData, &iconList, NULL);

	if (iconList == NULL) {
		printf("Unable to locate icon list.\n");
		return NULL;
	}

	for (iconPtr = iconList; iconPtr != NULL; iconPtr = iconPtr->next) {
		if ((event->xmotion.x > (iconPtr->icon.x - DRAG_THRESHOLD)) &&
		    (event->xmotion.x < (int)(iconPtr->icon.x +
					      iconPtr->icon.width +
					      DRAG_THRESHOLD)) &&
		    (event->xmotion.y > (iconPtr->icon.y - DRAG_THRESHOLD)) &&
		    (event->xmotion.y < (int)(iconPtr->icon.y +
				              iconPtr->icon.height +
					      DRAG_THRESHOLD))) {

			if (iconPtr == iconList) { /* Single-file drag */

				iconArray = (IconInfo *)
					XtMalloc(sizeof(IconInfo));
				iconArray[0] = *iconPtr;

				startFileDrag(widget, event, iconArray, 1);
			} else { /* Multi-file drag */
				iconArray = (IconInfo *)
					XtMalloc(2*sizeof(IconInfo));
				iconArray[0] = *iconPtr;
				iconArray[1] = *iconList;

				startFileDrag(widget, event, iconArray, 2);
			}
			break;
		}
	}
}

/*
 * dragMotionHandler
 *
 * Determine if the pointer has moved beyond the drag threshold while button 1
 * was being held down.
 */
void
dragMotionHandler(
	Widget		dragInitiator,
	XtPointer	clientData,
	XEvent	       *event)
{
	static int	initialX = -1;
	static int	initialY = -1;
	int		diffX, diffY;
	long		dragProtocol = (long)clientData;

	if (!doingDrag) {
		/*
	 	 * If the drag is just starting, set initial button down coords
		 */
		if (initialX == -1 && initialY == -1) {
			initialX = event->xmotion.x;
			initialY = event->xmotion.y;
		}
		/*
		 * Find out how far pointer has moved since button press
		 */
		diffX = initialX - event->xmotion.x;
		diffY = initialY - event->xmotion.y;
		
		if ((ABS(diffX) >= DRAG_THRESHOLD) ||
		    (ABS(diffY) >= DRAG_THRESHOLD)) {
			doingDrag = True;
			switch (dragProtocol) {
			case DtDND_TEXT_TRANSFER:
				startTextDrag(dragInitiator, event);
				break;
			case DtDND_FILENAME_TRANSFER:
				checkForFileDrag(dragInitiator, event, NULL, 0);
				break;
			case DtDND_BUFFER_TRANSFER:
				startApptDrag(dragInitiator, event);
				break;
			}
			initialX = -1;
			initialY = -1;
		}
	}
}


 /*************************************************************************
 *
 *	Initialization & Creation Functions
 *
 *************************************************************************/


/*
 * createFruitList
 *
 * Creates a scrolling list filled with fruit names.
 */
Widget
createFruitList(
	Widget		parent,
	Pixel		background)
{
	Widget		fruitList;
	XmString       *fruits;
	Arg		args[3];
	int		ii, fruitCount;

	for (ii = 0; todaysFruit[ii] != NULL; ii++)
		;
	fruitCount = ii;

	fruits = (XmString *) XtMalloc(sizeof(XmString) * fruitCount);

	for (ii = 0; todaysFruit[ii]; ii++) {
		fruits[ii] = XmStringCreate(todaysFruit[ii],
				 	    XmFONTLIST_DEFAULT_TAG);
	}

	ii = 0;
	XtSetArg(args[ii], XmNbackground, background); ii++;
	XtSetArg(args[ii], XmNitems,      fruits);     ii++;
	XtSetArg(args[ii], XmNitemCount,  fruitCount); ii++;

	fruitList = XmCreateScrolledList(parent, "fruitList", args, ii);
	XtManageChild(fruitList);
	
	return fruitList;
}

/*
 * createAppointmentList
 *
 * Creates a list of XmStrings with appointment data in them.
 */
int
createAppointmentList(
	XmString      **appts)
{
	int		ii, apptCount;
	char		tmpStr[256];
	time_t		now;
	struct tm	*tm;

	now = time(&now);
	tm = localtime(&now);

	sprintf(today, "%2d/%2d/%2d", tm->tm_mon+1, tm->tm_mday, tm->tm_year);

	for (ii = 0; todaysApptList[ii].date; ii++)
		;
	apptCount = ii;

	*appts = (XmString *) XtMalloc(sizeof(XmString) * apptCount);

	for (ii = 0; todaysApptList[ii].date; ii++) {

		sprintf(tmpStr, "%s %s", todaysApptList[ii].start, 
					 todaysApptList[ii].what);

		(*appts)[ii] = XmStringCreate(tmpStr, XmFONTLIST_DEFAULT_TAG);
	
	}
	return apptCount;
}

/*
 * doDragSetupForFiles
 *
 * Prepares the file draw area to be a drag source.
 */
void
doDragSetupForFiles(
	Widget		fileDraw)
{
	char		translations[] = "<Btn2Down>: checkForFileDrag()";
	XtTranslations	newTranslations;
	XtActionsRec	actionTable[] = {
		{"checkForFileDrag", (XtActionProc)checkForFileDrag},
	};

	XtAppAddActions(appContext, actionTable, 1);
	newTranslations = XtParseTranslationTable(translations);
	XtVaSetValues(fileDraw, XmNtranslations, newTranslations, NULL);

	XtAddEventHandler(fileDraw, Button1MotionMask, False,
		(XtEventHandler)dragMotionHandler, 
		(XtPointer)DtDND_FILENAME_TRANSFER);
}

/*
 * doDragSetupForText
 *
 * Prepares the fruit list to source drags of text with button 1.
 */
void
doDragSetupForText(
	Widget		fruitList)
{
	XtAddEventHandler(fruitList, Button1MotionMask, False,
		(XtEventHandler)dragMotionHandler, 
		(XtPointer)DtDND_TEXT_TRANSFER);
}

/*
 * doDragSetupForBuffers
 *
 * Prepares the appointment list to source drags of appointments with button 1.
 */
void
doDragSetupForBuffers(
	Widget		apptList)
{
	XtAddEventHandler(apptList, Button1MotionMask, False,
		(XtEventHandler)dragMotionHandler, 
		(XtPointer)DtDND_BUFFER_TRANSFER);
}

/*
 * doDropSetupForText
 *
 * Registers text field to accept drops of files.
 */
void
doDropSetupForText(
	Widget		dropText)
{
	static XtCallbackRec transferCBRec[] = { {textTransferCB, NULL},
						 {NULL, NULL} };
	static XtCallbackRec animateCBRec[]  = { {textAnimateCB, NULL},
						 {NULL, NULL} };
	Arg		arg[1];

	transferCBRec[0].closure = (XtPointer)0;
	animateCBRec[0].closure  = (XtPointer)0;

	XtSetArg(arg[0], DtNdropAnimateCallback, animateCBRec);

	DtDndDropRegister(dropText, DtDND_FILENAME_TRANSFER, 
		XmDROP_COPY, transferCBRec, arg, 1);
}

/*
 * doDropSetupForDraw
 *
 * Registers draw area to accept drops of files or data such as appointments.
 */
doDropSetupForDraw(
	Widget		dropDraw)
{
	static XtCallbackRec transferCBRec[] = { {drawTransferCB, NULL},
						     {NULL, NULL} };
	static XtCallbackRec animateCBRec[]  = { {drawAnimateCB, NULL},
						     {NULL, NULL} };
	Display		*display    = XtDisplayOfObject(dropDraw);

	XtVaSetValues(dropDraw, XmNuserData, NULL, NULL);

	transferCBRec[0].closure = (XtPointer)NULL;
	animateCBRec[0].closure = (XtPointer)dropDraw;

	DtDndVaDropRegister(dropDraw,
		DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
		XmDROP_COPY | XmDROP_MOVE, transferCBRec, 
		DtNdropAnimateCallback,	animateCBRec,
		NULL);
}

/*
 * doDropSetup
 *
 * Set up drop sites.
 */
void
doDropSetup(
	Widget		dropText,
	Widget		dropDraw)
{
	doDropSetupForText(dropText);
	doDropSetupForDraw(dropDraw);
}

/*
 * doDragSetup
 *
 * Set up drag sources.
 */
void
doDragSetup(
	Widget		fruitList,
	Widget		fileDraw,
	Widget		apptList)
{
	DtDndInitialize(topLevel);

	doDragSetupForText(fruitList);
	doDragSetupForFiles(fileDraw);
	doDragSetupForBuffers(apptList);
}

/*
 * Fallback resources; if app-defaults file not found
 */
String	fallbackResources[] = {
	"title:					CDE Drag & Drop Demo",

	"*outerRowColumn.orientation:		VERTICAL",
	"*outerRowColumn.spacing:		15",
	"*outerRowColumn.marginHeight:		15",
	"*outerRowColumn.marginWidth:		15",

	"*upperRowColumn.orientation:		HORIZONTAL",
	"*upperRowColumn.packing:		PACK_COLUMN",
	"*upperRowColumn.spacing:		15",
	"*upperRowColumn.marginHeight:		0",
	"*upperRowColumn.marginWidth:		0",

	"*fileDraw.height:			175",
	"*fileDraw.resizePolicy:		RESIZE_NONE",

	"*fruitList.listSizePolicy:		CONSTANT",
	"*fruitList.scrollBarDisplayPolicy:	STATIC",

	"*apptList.listSizePolicy:		CONSTANT",
	"*apptList.scrollBarDisplayPolicy:	STATIC",

	"*textRowColumn.orientation:		HORIZONTAL",
	"*textRowColumn.packing:		PACK_TIGHT",
	"*textRowColumn*textLabel.labelString: Name:",
	"*textRowColumn*dropText.width:		475",
	"*textRowColumn.marginWidth:		0",

	"*dropDraw.height:			100",
	"*dropDraw.resizePolicy:		RESIZE_NONE",
	NULL
};

/*
 * main
 */
void
main(
	int 		argc,
	String	       *argv)
{
	Widget		outerRowColumn,
			upperRowColumn,
			fruitList,
			fileFrame,
			fileDraw,
			apptList,
			separator,
			textRowColumn,
			textLabel,
			dropText,
			dropFrame,
			dropDraw;
	XmString       *apptListItems;
	Colormap	colormap;
	Pixel		topShadow, bottomShadow;
	int		ii, apptCount;

	topLevel = XtAppInitialize(&appContext, "Dtdnddemo", 
		(XrmOptionDescList)NULL, 0, &argc, argv, 
		fallbackResources, (ArgList)NULL, 0);

	DtAppInitialize(appContext, XtDisplay(topLevel), 
		topLevel, argv[0], "Dtdnddemo"); 

	XtVaGetValues(topLevel,
		XmNbackground, &motifBackground,
		XmNcolormap,   &colormap,
		NULL);

	XmGetColors(XtScreen(topLevel), colormap, motifBackground,
		&motifForeground, &topShadow, &bottomShadow, &motifSelect);

	outerRowColumn = XtVaCreateManagedWidget("outerRowColumn",
		xmRowColumnWidgetClass, topLevel,
		NULL);

	upperRowColumn = XtVaCreateManagedWidget("upperRowColumn",
		xmRowColumnWidgetClass, outerRowColumn,
		NULL);

	fruitList = createFruitList(upperRowColumn, motifSelect);

	fileFrame = XtVaCreateManagedWidget("fileFrame",
		xmFrameWidgetClass, upperRowColumn,
		NULL);

	fileDraw = XtVaCreateManagedWidget("fileDraw",
		xmDrawingAreaWidgetClass, fileFrame,
		XmNbackground, motifSelect,
		NULL);
	XtAddCallback(fileDraw, XmNexposeCallback, fileDrawExposeCB, NULL);

	apptCount = createAppointmentList(&apptListItems);
	
	apptList = XtVaCreateManagedWidget("apptList",
		xmListWidgetClass, upperRowColumn,
		XmNbackground, motifSelect,
		/* XmNselectionPolicy, XmMULTIPLE_SELECT, */
		XmNitems, apptListItems,
		XmNitemCount, apptCount,
		NULL);

	for (ii = 0; ii < apptCount; ii++) {
		XmStringFree(apptListItems[ii]);
	}

	separator = XtVaCreateManagedWidget("separator",
		xmSeparatorWidgetClass, outerRowColumn,
		NULL);
	
	textRowColumn = XtVaCreateManagedWidget("textRowColumn",
		xmRowColumnWidgetClass, outerRowColumn,
		NULL);

	textLabel = XtVaCreateManagedWidget("textLabel",
		xmLabelWidgetClass, textRowColumn,
		NULL);

	dropText = XtVaCreateManagedWidget("dropText",
		xmTextWidgetClass, textRowColumn,
		XmNbackground, motifSelect,
		NULL);
	
	dropFrame = XtVaCreateManagedWidget("dropFrame",
		xmFrameWidgetClass, outerRowColumn,
		NULL);

	dropDraw = XtVaCreateManagedWidget("dropDraw",
		xmDrawingAreaWidgetClass, dropFrame,
		XmNbackground, motifSelect,
		NULL);
	XtAddCallback(dropDraw, XmNexposeCallback, dropDrawExposeCB, NULL);

	XtRealizeWidget(topLevel);

	doDragSetup(fruitList, fileDraw, apptList);
	doDropSetup(dropText, dropDraw);

	XtAppMainLoop(appContext);
}
