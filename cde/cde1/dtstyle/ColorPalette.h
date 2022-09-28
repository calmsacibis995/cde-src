/* $XConsortium: ColorPalette.h /main/cde1_maint/2 1995/10/23 11:13:02 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ColorPalette.h
 **
 **   Project:     DT 3.0
 **
 **  This file contains function definitions for the corresponding .c
 **  file
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _colorpalette_h
#define _colorpalette_h

/* External Interface */

#ifdef _NO_PROTO

extern Bool AllocatePaletteCells() ;
extern int ReColorPalette() ;
extern void CheckMonitor() ;

#else

extern Bool AllocatePaletteCells( Widget shell) ;
extern int ReColorPalette( void ) ;
extern void CheckMonitor( Widget shell) ;

#endif /* _NO_PROTO */

#endif /* _colorpalette_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
