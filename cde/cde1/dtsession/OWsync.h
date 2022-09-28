/*******************************************************************************
**
**  OWsync.h
**
**  Defines functions to synchronize CDE and OpenWindows resources.
**
**  Copyright 1995 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*
 * External function declarations
 */

extern void	OWsyncColorResources(
			Display *	display,
			int 		monitorType, 
			ColorSet * 	colors);

extern void	OWsyncLocaleResources(
			Display *	display);

