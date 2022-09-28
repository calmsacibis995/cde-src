/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        xmcvt.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for xmcvt.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_xmcvt_h
#define _Dtksh_xmcvt_h

#include <Tt/tttk.h>

extern struct named_integer NI_TT_STATUS[];


#ifdef _NO_PROTO

extern char * XmStringToString() ;
extern void DtkshCvtXmStringToString() ;
extern void DtkshCvtKeySymToString() ;
extern void DtkshCvtListItemsToString() ;
extern void DtkshCvtNamedValueToString() ;
extern void DtkshCvtStringToNamedValue() ;
extern void DtkshRegisterNamedIntConverters() ;
extern void DtkshCvtXEModifierStateToString() ;
extern void DtkshCvtXEMotionHintToString() ;
extern void DtkshCvtTtStatusToString();
extern void DtkshCvtStringToTtStatus();
extern void DtkshCvtStringToBool();
extern void DtkshCvtStringToTtOp();
extern void DtkshCvtTtOpToString();
extern void DtkshCvtStringToTtScope();

#else

extern char * XmStringToString( 
                        XmString string) ;
extern void DtkshCvtXmStringToString( 
                        Display *dpy,
                        XrmValuePtr args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval,
                        XtPointer data) ;
extern void DtkshCvtKeySymToString( 
                        XrmValuePtr args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval) ;
extern void DtkshCvtListItemsToString( 
                        Display *dpy,
                        XrmValuePtr args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval,
                        XtPointer data) ;
extern void DtkshCvtNamedValueToString( 
                        XrmValue *args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval) ;
extern void DtkshCvtStringToNamedValue( 
                        XrmValue *args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval) ;
extern void DtkshRegisterNamedIntConverters( void ) ;
extern void DtkshCvtXEModifierStateToString( 
                        XrmValuePtr args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval) ;
extern void DtkshCvtXEMotionHintToString( 
                        XrmValuePtr args,
                        Cardinal *nargs,
                        XrmValuePtr fval,
                        XrmValuePtr toval) ;
extern void DtkshCvtTtStatusToString(
			Tt_status ttStatus,
			XrmValue *pToVal);
extern void DtkshCvtStringToTtStatus(
			char *pString,
			XrmValue *pToVal);
extern void DtkshCvtStringToBool(
			char *pString,
			XrmValue *pToVal);
extern void DtkshCvtStringToTtOp(
			char *pString,
			XrmValue *pToVal);
extern void DtkshCvtTtOpToString(
			Tttk_op op,
			XrmValue *pToVal);
extern void DtkshCvtStringToTtScope(
			char *pString,
			XrmValue *pToVal);

#endif /* _NO_PROTO */

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern char * _CvtXmStringTableToString() ;

#else

extern char * _CvtXmStringTableToString( 
                        XmStringTable stringtable,
                        int itemCount) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_xmcvt_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
