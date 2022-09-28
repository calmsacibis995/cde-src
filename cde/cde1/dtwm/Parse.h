/* $XConsortium: Parse.h /main/cde1_maint/1 1995/07/18 01:53:23 drk $ */
/*****************************************************************************
 *
 *   File:         Parse.h
 *
 *   Project:	   CDE
 *
 *   Description:  This file contains the external function definitions
 *                 for the Front Panel keyword-value parsing routines.
 *
 *
 ****************************************************************************/


#ifndef _parse_h
#define _parse_h


#ifdef _NO_PROTO

extern Boolean StringToString ();
extern Boolean StringToInt ();
extern Boolean StringToBoolean ();
extern Boolean StringToResolution ();
extern Boolean StringToColorUse ();
extern Boolean StringToControlBehavior ();
extern Boolean StringToGeometry ();
extern Boolean StringToAction ();
extern Boolean StringToControlType ();
extern Boolean StringToMonitorType ();
extern Boolean StringToControlContainerType ();
extern Boolean StringToPositionHints ();
extern Boolean StringToFileName ();
extern void FreeString ();
extern void FreeGeometry ();
extern void FreeAction ();

#else  /* _NO_PROTO */

extern Boolean StringToString (char *, void **);
extern Boolean StringToInt (char *, void **);
extern Boolean StringToBoolean (char *, void **);
extern Boolean StringToResolution (char *, void **);
extern Boolean StringToColorUse (char *, void **);
extern Boolean StringToControlBehavior (char *, void **);
extern Boolean StringToGeometry (char *, void **);
extern Boolean StringToAction (char *, void **);
extern Boolean StringToControlType (char *, void **);
extern Boolean StringToMonitorType (char *, void **);
extern Boolean StringToControlContainerType (char *, void **);
extern Boolean StringToPositionHints (char *, void **);
extern Boolean StringToFileName (char *, void **);
extern void FreeString (void **);
extern void FreeGeometry (void **);
extern void FreeAction (void **);

#endif /* _NO_PROTO */



#endif /* _parse_h */
/*  DON"T ADD ANYTHING AFTER THIS #endif  */
