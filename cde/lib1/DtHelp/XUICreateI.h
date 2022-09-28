/* $XConsortium: XUICreateI.h /main/cde1_maint/1 1995/07/17 17:55:53 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        XUICreateI.h
 **
 **   Project:     Cde Help System
 **
 **   Description: Internal file for XUICreate.c
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef	_DtHelpCreateI_h
#define	_DtHelpCreateI_h

/*****************************************************************************
 *                 Defines
 *****************************************************************************/
/*
 * scroll bar flags
 */
#define _DtHelpNONE         0
#define _DtHelpSTATIC       1
#define _DtHelpAS_NEEDED    2

#define	_DtHelpVERTICAL_SCROLLBAR		0
#define	_DtHelpHORIZONTAL_SCROLLBAR	1

/*
 * scroll bar macros
 */
#define	_DtHelpSET_AS_NEEDED(x,y)		((x) | (1 << y))
#define _DtHelpIS_AS_NEEDED(x, y)		((x) & (1 << y))

/*****************************************************************************
 *                 Semi-Public Routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	void	__DtHelpInitializeFontList ();
#else
extern	void	__DtHelpInitializeFontList (
			Display		*dpy,
			XFontStruct	*default_font);
#endif

/*****************************************************************************
 *                 Public Routines
 *****************************************************************************/
#ifdef _NO_PROTO
extern	DtHelpDispAreaStruct	*_DtHelpCreateDisplayArea ();
#else
extern	DtHelpDispAreaStruct	*_DtHelpCreateDisplayArea (
				Widget  parent,
				char   *name,
				short   vert_flag,
				short   horiz_flag,
				Boolean marker_flag,
				int	rows,
				int	columns,
				void	(*hyperTextCB)(),
				void	(*resizeCB)(),
				VStatus	(*exec_ok_routine)(),
				caddr_t	client_data,
				XmFontList  default_list );
#endif

#endif /* _DtHelpCreateI_h */
