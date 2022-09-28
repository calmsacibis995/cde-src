/* $XConsortium: IconicPathP.h /main/cde1_maint/1 1995/07/17 20:44:45 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           IconicPathP.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Private header file for IconicPath.c
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _DtIconicPathP_h
#define _DtIconicPathP_h

#include <Xm/ManagerP.h>
#include "IconicPath.h"

#ifdef __cplusplus
extern "C" {
#endif


/*  New fields for the IconicPath widget class record  */

typedef struct
{
   int mumble;   /* No new procedures */
} DtIconicPathClassPart;


/* Full class record declaration */

typedef struct _DtIconicPathClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	DtIconicPathClassPart	iconic_path_class;
} DtIconicPathClassRec;

externalref DtIconicPathClassRec dtIconicPathClassRec;


/* New fields for the IconicPath widget record */

typedef struct
{
	Dimension		margin_width;
	Dimension		margin_height;
	Dimension		spacing;
	Dimension		small_min_width;
	Dimension		large_min_width;
	Boolean			force_small_icons;
	Boolean			buttons;
	Boolean			dropzone;
	Boolean			status_msg;
	Boolean			force_large_icons;
	Boolean			icons_changed;

	char * file_mgr_rec;
	char * current_directory;
	Boolean large_icons;
	char *msg_text;
	
	char * directory_shown;
	Boolean large_shown;
	Widget dotdot_button;
	Widget dropzone_icon;
	Widget status_label;
	int num_components;
	struct _IconicPathComponent {
		char *path;
		char *icon_name;
		Widget icon;
		Widget button;
		int width;
	} *components;
	int left_component;
	GC gc;

} DtIconicPathPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _DtIconicPathRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	DtIconicPathPart	iconic_path;
} DtIconicPathRec;



/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _DtIconicPathInput() ;

#else

extern void _DtIconicPathInput(
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DtIconicPathP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
