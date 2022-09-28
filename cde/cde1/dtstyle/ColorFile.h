/* $XConsortium: ColorFile.h /main/cde1_maint/3 1995/10/23 11:10:28 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ColorFile.h
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
#ifndef _colorfile_h
#define _colorfile_h

/* External variables */

#define DT_PAL_DIR  "/.dt/palettes/"
/* 
extern char *DT_PAL_DIR = "/.dt/palettes/"; 
*/

/* External Interface */

#ifdef _NO_PROTO

extern void ReadPalette();
extern void AddSystemPath();
extern void AddDirectories();
extern void AddHomePath();
extern Boolean ReadPaletteLoop();
extern void CheckFileType();

#else

extern void ReadPalette( char *directory, 
			 char *in_filename,
#if NeedWidePrototypes
			 int length );
#else
			 short length );
#endif
extern void AddSystemPath( void );
extern void AddDirectories( char *text );
extern void AddHomePath( void );
#if NeedWidePrototypes
extern Boolean ReadPaletteLoop( int startup );
#else
extern Boolean ReadPaletteLoop( Boolean startup );
#endif
extern void BackdropDialog( Widget parent) ;
extern void CheckWorkspace( void ) ;
extern void restoreBackdrop( Widget shell, XrmDatabase db) ;
extern void saveBackdrop( int fd) ;
extern void CheckFileType( void );

#endif /* _NO_PROTO */

#endif /* _colorfile_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
