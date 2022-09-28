/* $XConsortium: AsciiSpcI.h /main/cde1_maint/1 1995/07/17 17:20:52 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   AsciiSpcI.h
 **
 **  Project:
 **
 **  Description:  Public Header file for AsciiSpc.c
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtHelpAsciiSpc_h
#define _DtHelpAsciiSpc_h


#ifdef __cplusplus
extern "C" {
#endif

/********    Public Defines Declarations    ********/

/********    Public Typedef Declarations    ********/

/********    Public Structures Declarations    ********/

/********    Public Macro Declarations    ********/

/********    Semi-Private Function Declarations    ********/
#ifdef _NO_PROTO
extern	const char	*_DtHelpCeResolveSpcToAscii();
#else
extern	const char	*_DtHelpCeResolveSpcToAscii(
				const char	*spc_symbol);
#endif /* _NO_PROTO */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* _DtHelpAsciiSpc_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
