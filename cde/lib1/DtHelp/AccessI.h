/* $XConsortium: AccessI.h /main/cde1_maint/1 1995/07/17 17:18:25 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        AccessI.h
 **
 **   Project:     Run Time Project File Access
 **
 **   Description: Header file for Access.h
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
#ifndef _DtHelpAccessI_h
#define _DtHelpAccessI_h


#ifndef	_XtIntrinsic_h
/*
 * typedef Boolean
 */
#ifdef CRAY
typedef long            Boolean;
#else
typedef char            Boolean;
#endif
#endif

#ifndef	_XLIB_H_
#ifndef	True
#define	True	1
#endif
#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	False
#define	False	0
#endif
#ifndef	FALSE
#define	FALSE	0
#endif
#endif

/******************************************************************************
 *                   Semi-Public Structures
 ******************************************************************************/
typedef struct {
	char  match;
	char *substitution;
} _DtSubstitutionRec;

typedef	struct _dtHelpCeLockInfo {
	int		fd;
	VolumeHandle	volume;
} _DtHelpCeLockInfo;

/******************************************************************************
 *                   Semi-Public Access Functions
 ******************************************************************************/
#ifdef _NO_PROTO
extern	char	*_DtHelpCeExpandPathname ();
extern	int	 _DtHelpCeFileOpenAndSeek();
extern	int	 _DtHelpCeGetAsciiVolumeAbstract ();
extern	int	 _DtHelpCeGetDocStamp ();
extern	int	 _DtHelpCeGetLangSubParts ();
extern	int	 _DtHelpCeGetTopicChildren();
extern	int	 _DtHelpCeGetTopicTitle();
extern	int	 _DtHelpCeGetUncompressedFileName ();
extern	const char *_DtHelpCeGetVolumeCharSet ();
extern	int	 _DtHelpCeGetVolumeFlag ();
extern	char	*_DtHelpCeGetVolumeLocale ();
extern	int	 _DtHelpCeGetVolumeTitle();
extern	int	 _DtHelpCeIsTopTopic();
extern	int	 _DtHelpCeLockVolume();
extern	int	 _DtHelpCeMapTargetToId ();
extern	int	 _DtHelpCeUnlockVolume();
#else
extern	char	*_DtHelpCeExpandPathname (
			char		   *spec,
			char		   *filename,
			char		   *type,
			char		   *suffix,
			char		   *lang,
			_DtSubstitutionRec *subs,
			int		    num );
extern	int	 _DtHelpCeFileOpenAndSeek(
			char		*filename,
			int		 offset,
			int		 fd,
			BufFilePtr	*ret_file,
			time_t		*ret_time);
extern	int	 _DtHelpCeGetAsciiVolumeAbstract (
			CanvasHandle	  canvas_handle,
			VolumeHandle	  vol,
			char		**retAbs);
extern	int	 _DtHelpCeGetDocStamp (
			VolumeHandle	  volume,
			char		**ret_doc,
			char		**ret_time);
extern	int	 _DtHelpCeGetLangSubParts (
			char		 *lang,
			char		**subLang,
			char		**subTer,
			char		**subCodeSet );
extern	int	 _DtHelpCeGetTopicChildren(
			VolumeHandle	  volume,
			char		 *topic_id,
			char		***ret_childs);
extern	int	 _DtHelpCeGetTopicTitle(
			CanvasHandle	  canvas,
			VolumeHandle	  volume,
			char		 *target_id,
			char		**ret_title);
extern	int	 _DtHelpCeGetUncompressedFileName (
			char		  *name,
			char		 **ret_name);
extern	const char *_DtHelpCeGetVolumeCharSet (
			VolumeHandle	  volume);
extern	int	 _DtHelpCeGetVolumeFlag (
			VolumeHandle	  volume);
extern	char	*_DtHelpCeGetVolumeLocale (
			VolumeHandle	  volume);
extern	int	 _DtHelpCeGetVolumeTitle(
			CanvasHandle	canvas,
			VolumeHandle	volume,
			char		**ret_title);
extern	int	 _DtHelpCeIsTopTopic(
			VolumeHandle	   volume,
			const char	  *id);
extern	int	 _DtHelpCeLockVolume(
			CanvasHandle	   canvas,
			VolumeHandle	   volume,
			_DtHelpCeLockInfo *ret_info);
extern	int	 _DtHelpCeMapTargetToId (
			VolumeHandle	  volume,
			char		 *target_id,
			char		**ret_id);
extern	int	 _DtHelpCeUnlockVolume(_DtHelpCeLockInfo lock_info);
#endif /* _NO_PROTO */
#endif /* _DtHelpAccessI_h */
