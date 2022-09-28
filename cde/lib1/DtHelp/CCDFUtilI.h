/* $XConsortium: CCDFUtilI.h /main/cde1_maint/1 1995/07/17 17:21:24 drk $ */
/*************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        CCDFUtilI.h
 **
 **   Project:     Un*x Desktop Help
 **
 **   Description: Header file for CCDFUtil.c
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
#ifndef _CECCDFUtilI_h
#define _CECCDFUtilI_h

/*****************************************************************************
 *  Parsing flags and macros
 *****************************************************************************/
#define CE_ABBREV_CMD          (1 <<  0)
#define CE_FIGURE_CMD          (1 <<  1)
#define CE_FONT_CMD            (1 <<  2)
#define CE_FORMAT_END          (1 <<  3)
#define CE_GRAPHIC_CMD         (1 <<  4)
#define CE_ID_CMD              (1 <<  5)
#define CE_LABEL_CMD           (1 <<  6)
#define CE_LINK_CMD            (1 <<  7)
#define CE_NEWLINE_CMD         (1 <<  8)
#define CE_OCTAL_CMD           (1 <<  9)
#define CE_PARAGRAPH_CMD       (1 << 10)
#define CE_TITLE_CMD           (1 << 11)
#define CE_TOPIC_CMD           (1 << 12)

#define CE_ALLOW_NO_CMDS                 0
#define CE_ALLOW_ALL_CMDS              ~(0)
#define CE_ALLOW_CMD(x, y)             ((x) & (y))
#define CE_NOT_ALLOW_CMD(x, y)         ((~(x)) & (y))

/****************************************************************************
 *			Semi Public Routines
 ****************************************************************************/
#ifdef	_NO_PROTO
extern	int	_DtHelpCeGetCcdfAbbrevCmd();
extern	int	_DtHelpCeSkipCcdfAbbrev();
extern	int	_DtHelpCeCheckNextCcdfCmd();
extern	int	_DtHelpCeGetCcdfEndMark();
extern	int	_DtHelpCeGetCcdfFontType();
extern	int	_DtHelpCeGetCcdfStrParam();
extern	int	_DtHelpCeGetCcdfValueParam();
extern	int	_DtHelpCeGetCcdfTopicAbbrev();
extern	int	_DtHelpCeGetCcdfTopicCmd();
extern	int	_DtHelpCeGetCcdfParaOptions();
extern	int	_DtHelpCeGetCcdfCmd();
extern	int	_DtHelpCeSkipToNextCcdfToken();
#else
extern	int	_DtHelpCeGetCcdfAbbrevCmd(
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			int		 cur_mb_len,
			char		**in_ptr,
			char		**ret_string);
extern	int	_DtHelpCeSkipCcdfAbbrev(
			BufFilePtr	 in_file,
			char		*in_buf,
			char		**in_ptr,
			int		 in_size,
			int		 cur_mb_len);
extern	int	_DtHelpCeCheckNextCcdfCmd(
			char		*token,
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			int		 cur_mb_len,
			char		**in_ptr);
extern	int	_DtHelpCeGetCcdfEndMark(
			BufFilePtr	  file,
			char		 *buffer,
			char		**buf_ptr,
			int		  buf_size,
			int		  cur_mb_len);
extern	int	_DtHelpCeGetCcdfFontType(
			char		*code);
extern	int	_DtHelpCeGetCcdfValueParam(
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			char		**in_ptr,
			CEBoolean	 flag,
			int		 cur_mb_len,
			int		*ret_value);
extern	int	_DtHelpCeGetCcdfStrParam(
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			int		 cur_mb_len,
			char		**in_ptr,
			CEBoolean	 flag,
			CEBoolean	 eat_escape,
			CEBoolean	 ignore_quotes,
			CEBoolean	 less_test,
			char		**ret_string);
extern	int	_DtHelpCeGetCcdfTopicAbbrev(
			void		*dpy,
			BufFilePtr	 in_file,
			char		*in_buf,
			char		**in_ptr,
			int		 in_size,
			int		 cur_mb_len,
			char		**ret_title,
			char		**ret_charSet,
			char		**ret_abbrev);
extern	int	_DtHelpCeGetCcdfTopicCmd(
			void		*dpy,
			BufFilePtr	 in_file,
			char		*in_buf,
			char		**in_ptr,
			int		 in_size,
			int		 cur_mb_len,
			char		**ret_charSet);
extern	int	_DtHelpCeGetCcdfParaOptions(
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			int		 cur_mb_len,
			char		**in_ptr,
			int		*seg_type,
			int		*graphic_type,
			int		*first_indent,
			int		*left_indent,
			int		*right_indent,
			int		*lines_before,
			int		*lines_after,
			int		*spacing,
			char		**ret_label,
			char		**ret_name,
			char		**ret_link,
			int		*ret_link_type,
			char		**ret_descript,
			char		**ret_idString );
extern	int	_DtHelpCeGetCcdfCmd(
			int		 current,
			char		*in_buf,
			char		**in_ptr,
			BufFilePtr	 in_file,
			int		 size,
			int		 allowed);
extern	int	_DtHelpCeSkipToNextCcdfToken(
			BufFilePtr	 in_file,
			char		*in_buf,
			int		 in_size,
			int		 cur_mb_len,
			char		**in_ptr,
			CEBoolean	 flag );
#endif

#endif /* _CECCDFUtilI_h */
