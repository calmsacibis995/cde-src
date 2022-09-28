/* $XConsortium: main.h /main/cde1_maint/3 1995/10/23 08:59:00 gtsang $ */
/*********************************************************************
*  (c) Copyright 1993, 1994 Hewlett-Packard Company	
*  (c) Copyright 1993, 1994 International Business Machines Corp.
*  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
*  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
*      Novell, Inc.
**********************************************************************/
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        main.h
 **
 **   Project:     DT 3.0
 **
 **  This file contains global header file information for dticon
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1992.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _main_h
#define _main_h


#ifdef MESSAGE_CAT
#  ifdef __ultrix
#    define _CLIENT_CAT_NAME "dticon.cat"
#  else
#    define _CLIENT_CAT_NAME "dticon"
#  endif  /* __ultrix */
#  ifdef __osf__
/*
 * Without this proto, standard C says that _DtGetMessage() returns
 * an int, even though it really returns a pointer.  The compiler is
 * then free to use the high 32-bits of the return register for
 * something else (like scratch), and that can garble the pointer.
 */
#  ifdef _NO_PROTO
extern char *_DtGetMessage();
#  else  /* _NO_PROTO */
extern char *_DtGetMessage(
                        char *filename,
                        int set,
                        int n,
                        char *s );
#  endif /* _NO_PROTO */
#  define GETSTR(set, number, string) \
          (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#  else 
#  define GETSTR(set, number, string) \
          ((char *) _DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#  endif /* __osf__ */
#else
#  define GETSTR(set, number, string) string
#endif /* MESSAGE_CAT */


/* get a message catlog string and convert it to an XmString */
#define GETXMSTR(set,num,str) \
        XmStringCreateLtoR (GETSTR(set, num, str), XmFONTLIST_DEFAULT_TAG)

/* convert a regular string to an XmString */
#define XMSTR(str) \
        XmStringCreateLtoR (str, XmFONTLIST_DEFAULT_TAG)


#endif /* _main_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

