/* $XConsortium: AccessCCDFI.h /main/cde1_maint/1 1995/07/17 17:17:48 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        AccessCCDFI.h
 **
 **   Project:     Run Time Project File Access
 **
 **   Description: Header file for AccessCCDF.h
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
#ifndef _DtHelpAccessCcdfI_h
#define _DtHelpAccessCcdfI_h

/******************************************************************************
 *                   Resource Database Access Routines
 ******************************************************************************/
#ifdef _NO_PROTO
extern	char		 *_DtHelpCeGetResourceString();
extern	char		**_DtHelpCeGetResourceStringArray();
#else
extern	char		 *_DtHelpCeGetResourceString(
					XrmDatabase	 db,
					char		*topic,
					char		*res_class,
					char		*res_name);
extern	char		**_DtHelpCeGetResourceStringArray(
					XrmDatabase	 db,
					char		*topic,
					char		*res_class,
					char		*res_name);
#endif /* _NO_PROTO */

/******************************************************************************
 *                   CCDF Access Functions
 ******************************************************************************/
#ifdef _NO_PROTO
extern  void		 _DtHelpCeCloseCcdfVolume();
extern  CEBoolean	 _DtHelpCeFindCcdfId();
extern  int		 _DtHelpCeGetCcdfDocStamp();
extern  int		 _DtHelpCeFindCcdfKeyword();
extern  int		 _DtHelpCeGetCcdfIdPath();
extern	int		 _DtHelpCeGetCcdfKeywordList ();
extern	int		 _DtHelpCeGetCcdfTopicChildren ();
extern	int		 _DtHelpCeGetCcdfTopicParent ();
extern	int		 _DtHelpCeGetCcdfTopTopic ();
extern	int		 _DtHelpCeGetCcdfVolIdList ();
extern	char		*_DtHelpCeGetCcdfVolLocale ();
extern	char		*_DtHelpCeGetCcdfVolTitle();
extern	int		 _DtHelpCeGetCcdfVolumeAbstract ();
extern	int		 _DtHelpCeGetCcdfVolumeTitle();
extern	int		 _DtHelpCeMapCcdfTargetToId ();
extern	int		 _DtHelpCeOpenCcdfVolume ();
extern	int		 _DtHelpCeRereadCcdfVolume ();
#else
extern  void		 _DtHelpCeCloseCcdfVolume(
				_DtHelpVolume      vol);
extern  CEBoolean	 _DtHelpCeFindCcdfId(
				_DtHelpVolume      vol,
				char              *target_id,
				char		 **ret_name,
				int		  *ret_offset);
extern  int		 _DtHelpCeGetCcdfDocStamp(
				_DtHelpVolume      vol,
				char		 **ret_doc,
				char		 **ret_time);
extern  int		 _DtHelpCeFindCcdfKeyword(
				_DtHelpVolume      vol,
				char              *target_id,
				char            ***ret_ids );
extern  int		 _DtHelpCeGetCcdfIdPath(
				_DtHelpVolume      vol,
				char              *target_id,
				char            ***ret_ids );
extern	int		 _DtHelpCeGetCcdfKeywordList (
				_DtHelpVolume      vol);
extern	int		 _DtHelpCeGetCcdfTopicChildren (
				_DtHelpVolume      vol,
				char		  *topic_id,
				char		***retTopics);
extern	int		 _DtHelpCeGetCcdfTopicParent (
				_DtHelpVolume      vol,
				char		  *topic_id,
				char		 **retTopics);
extern	int		 _DtHelpCeGetCcdfTopTopic (
				_DtHelpVolume      vol,
				char		 **ret_topic);
extern	int		 _DtHelpCeGetCcdfVolIdList (
				_DtHelpVolume      vol,
				char		***retTopics);
extern	char		*_DtHelpCeGetCcdfVolLocale (
				_DtHelpVolume      vol);
extern	char		*_DtHelpCeGetCcdfVolTitle(
				_DtHelpVolume      vol);
extern	int		 _DtHelpCeGetCcdfVolumeAbstract (
				_DtHelpVolume      vol,
				char		 **ret_abs);
extern	int		 _DtHelpCeGetCcdfVolumeTitle(
				_DtHelpVolume      vol,
				char		 **ret_title);
extern	int		 _DtHelpCeMapCcdfTargetToId (
				_DtHelpVolume      vol,
				const char        *target_id,
				char             **ret_id);
extern	int		 _DtHelpCeOpenCcdfVolume (
				_DtHelpVolume      vol);
extern	int		 _DtHelpCeRereadCcdfVolume (
				_DtHelpVolume      vol);
#endif /* _NO_PROTO */
#endif /* _DtHelpAccessCcdfI_h */
