/* $XConsortium: icon.h /main/cde1_maint/1 1995/07/17 16:43:24 drk $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         icon.h
 **
 **   Description:  Header for icon handling portion of CDE Drag & Drop Demo.
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <sys/param.h>
#include <stdio.h>

/*
 * Icon Information Structure
 */

typedef struct _IconInfo {
	XRectangle	icon;
	char		*name;
	void		*data;
	int		dataCount;
	Pixmap		bitmap;
	Pixmap		mask;
	Widget		dragIcon;
	int		type;
	struct _IconInfo *next;
	struct _IconInfo *prev;
} IconInfo;

/*
 * Icon Typedefs
 */

typedef enum {
	IconBitmap,
	IconMask
} IconBitmapPart;

typedef enum {
	IconByFile,
	IconByData,
	IconByType
} IconTyping;

/*
 * Public Icon Handling Function Declarations
 */

void 		IconCreateDouble(Widget, IconInfo*, int, int, IconInfo*,
			short, short);
void		IconDelete(Widget, IconInfo*);
void		IconDraw(Widget, IconInfo*);
void		IconInitialize(Widget, IconInfo*, short, short, void*, int, 
			char*, IconTyping);
IconInfo	*IconNew();

