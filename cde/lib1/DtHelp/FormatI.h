/* $XConsortium: FormatI.h /main/cde1_maint/1 1995/07/17 17:31:00 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FormatI.h
 **
 **   Project:     TextGraphic Display routines
 **
 **   Description: Header file for Format.c
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
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _DtHelpFormatI_h
#define _DtHelpFormatI_h

#ifdef	_NO_PROTO
extern	int		 _DtHelpFormatAsciiFile ();
extern	int		 _DtHelpFormatAsciiString ();
extern	int		 _DtHelpFormatAsciiStringDynamic ();
extern	int		 _DtFormatGetParagraphList();
extern	int		 _DtHelpFormatIndexEntries ();
extern	void		 _DtFormatInitializeFontQuarks();
extern	int		 _DtFormatInitializeVariables();
extern	int		 _DtFormatProcessString ();
extern	int		 _DtHelpFormatTopic ();
extern	int		 _DtHelpFormatTopicTitle ();
extern	int		 _DtHelpFormatVolumeTitle ();
#else
extern	int		 _DtHelpFormatAsciiFile (
				DtHelpDispAreaStruct *pDAS,
				char		 *filename,
				TopicHandle	 *ret_topic);
extern	int		 _DtHelpFormatAsciiString (
				DtHelpDispAreaStruct *pDAS,
				char		*input_string,
				TopicHandle	 *ret_topic);
extern	int		 _DtHelpFormatAsciiStringDynamic (
				DtHelpDispAreaStruct *pDAS,
				char		*input_string,
				TopicHandle	 *ret_topic);
extern	int		 _DtFormatGetParagraphList(
				TopicHandle	 *ret_topic);
extern	int		 _DtHelpFormatIndexEntries (
				DtHelpDispAreaStruct *pDAS,
				VolumeHandle     volume,
				int		 *ret_cnt,
				XmString	**ret_words,
				XmFontList	 *ret_list,
				Boolean		 *ret_mod);
extern	void		 _DtFormatInitializeFontQuarks(
				char	**font_attr );
extern	int		 _DtFormatInitializeVariables(
				char             *input_string,
				int               grow_size );
extern	int		 _DtFormatProcessString (
				char	**input_string,
				int       seg_type,
				char	**font_attr );
extern	int		 _DtHelpFormatTopic (
				DtHelpDispAreaStruct *pDAS,
				VolumeHandle     volume,
				char		*id_string,
				CEBoolean	 look_at_id,
				TopicHandle	*ret_handle);
extern	int		 _DtHelpFormatTopicTitle (
				DtHelpDispAreaStruct *pDAS,
				VolumeHandle     volume,
				char		 *location_id,
				XmString	 *ret_title,
				XmFontList	 *ret_list,
				Boolean		 *ret_mod);
extern	int		 _DtHelpFormatVolumeTitle (
				DtHelpDispAreaStruct *pDAS,
				VolumeHandle     volume,
				XmString	 *ret_title,
				XmFontList	 *ret_list,
				Boolean		 *ret_mod);
#endif

#endif /* _DtHelpFormatI_h */
