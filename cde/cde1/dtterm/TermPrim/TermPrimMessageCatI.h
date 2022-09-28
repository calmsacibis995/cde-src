/*
 * $XConsortium: TermPrimMessageCatI.h /main/cde1_maint/1 1995/07/15 01:28:14 drk $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef	_Dt_TermPrimMessageCatI_h
#define	_Dt_TermPrimMessageCatI_h

 /*
 * macro to get message catalog strings
 * This is for dtterm only.  
 * The set number is for the file e.g. NL_SETN_Syntax is used by
 *     DtTermSyntax.c.
 */

#include "TermPrimOSDepI.h"	/* for MESSAGE_CAT			*/

#define NL_SETN_Main 1
#define NL_SETN_Syntax 2
#define NL_SETN_Function 3
#define NL_SETN_Prim 4
#define NL_SETN_PrimRend 5
#define NL_SETN_View 6 
#define NL_SETN_ViewGlobalDialog 7
#define NL_SETN_ViewMenu 8
#define NL_SETN_ViewTerminalDialog 9

#ifdef MESSAGE_CAT
extern char *_DtTermPrimGetMessage( char *filename, int set, int n, char *s );

#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtterm.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtterm"
#endif /* __ultrix */
#define GETMESSAGE(set, number, string)\
        _DtTermPrimGetMessage(_CLIENT_CAT_NAME, set, number, string)
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

#endif	/* _Dt_TermPrimMessageCatI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
