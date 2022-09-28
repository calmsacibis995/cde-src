/* $XConsortium: CEUtilI.h /main/cde1_maint/1 1995/07/17 17:22:15 drk $ */
/*************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        CEUtilI.h
 **
 **   Project:     TextGraphic Display routines
 **
 **  
 **   Description: Header file for CEUtilI.c
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _CECEUtilI_h
#define _CECEUtilI_h

/****************************************************************************
 *			Semi Public Routines
 ****************************************************************************/
#ifdef	_NO_PROTO
extern	int	  _DtHelpCeAddCharToBuf ();
extern	int	  _DtHelpCeAddOctalToBuf();
extern	int	  _DtHelpCeAddStrToBuf ();
extern	int	  _DtHelpCeGetMbLen();
extern	int	  _DtHelpCeGetNxtBuf();
extern	char *	  _DtHelpCeGetNxtToken ();
extern	int	  _DtHelpCeReadBuf();
extern	void	  _DtHelpCeXlateOpToStdLocale();
extern	void	  _DtHelpCeXlateStdToOpLocale();
#else
extern	int	_DtHelpCeAddCharToBuf (
			char	**src,
			char	**dst,
			int	 *dst_size,
			int	 *dst_max,
			int	  grow_size );
extern	int	_DtHelpCeAddOctalToBuf(
			char     *src,
			char    **dst,
			int      *dst_size,
			int      *dst_max,
			int       grow_size );
extern	int	_DtHelpCeAddStrToBuf (
			char	**src,
			char	**dst,
			int	 *dst_size,
			int	 *dst_max,
			int	  copy_size,
			int	  grow_size );
extern	int	_DtHelpCeGetMbLen(
			char	*lang,
			char	*char_set);
extern	int	_DtHelpCeGetNxtBuf(
			BufFilePtr	  file,
			char		 *dst,
			char		**src,
			int		  max_size);
extern	char *	_DtHelpCeGetNxtToken (
			char		 *str,
			char		**retToken);
extern	int	_DtHelpCeReadBuf(
			BufFilePtr	 file,
			char		*buffer,
			int		 size);
extern void	_DtHelpCeXlateOpToStdLocale (
			char       *operation,
			char       *opLocale,
			char       **ret_locale,
			char       **ret_lang,
			char       **ret_set);
extern void	_DtHelpCeXlateStdToOpLocale (
			char       *operation,
			char       *stdLocale,
			char       *dflt_opLocale,
			char       **ret_opLocale);
#endif

#endif /* _CECEUtilI_h */
