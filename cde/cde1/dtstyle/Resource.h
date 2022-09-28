/* $XConsortium: Resource.h /main/cde1_maint/2 1995/10/23 11:26:00 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Resource.h
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
#ifndef _resource_h
#define _resource_h

/* External Interface */

#ifdef _NO_PROTO

extern void GetUserFontResource();
extern void GetSysFontResource();
extern void GetUserBoldFontResource();

extern void GetApplicationResources( ) ;
extern void AddResourceConverters( ) ;

#else

extern void GetUserFontResource(int);
extern void GetSysFontResource(int);
extern void GetUserBoldFontResource(int);

extern void GetApplicationResources( void ) ;
extern void AddResourceConverters( void ) ;

#endif /* _NO_PROTO */

#endif /* _resource_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
