/* $XConsortium: Access.h /main/cde1_maint/1 1995/07/17 17:16:44 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Access.h
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
#ifndef _DtHelpAccess_h
#define _DtHelpAccess_h

/****************************************************************************
 *			Public Defines
 ****************************************************************************/
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

#ifndef	_CEVolumeOpen
#define	_CEVolumeOpen(canvas, filename, ret_vol)	\
		_DtHelpCeOpenVolume(canvas, filename, ret_vol)
#endif
#ifndef	_CEVolumeClose
#define	_CEVolumeClose(canvas, volume)	\
		_DtHelpCeCloseVolume(canvas, volume)
#endif

/****************************************************************************
 *			Semi-Private Externals
 ****************************************************************************/
#ifdef _NO_PROTO
extern	int	 _DtHelpCeCompressPathname ();
extern	char	*_DtHelpCeTraceFilenamePath ();
extern	char	*_DtHelpCeTracePathName ();
#else
extern	int	 _DtHelpCeCompressPathname ( char *basePath );
extern	char	*_DtHelpCeTraceFilenamePath ( char *file_path );
extern	char	*_DtHelpCeTracePathName ( char *path );
#endif /* _NO_PROTO */

/****************************************************************************
 *			Semi-Public Externals
 ****************************************************************************/
#ifdef _NO_PROTO
extern	int		 _DtHelpCeCloseVolume ();
extern	CEBoolean	 _DtHelpCeFindId ();
extern	int		 _DtHelpCeFindKeyword ();
extern	int		 _DtHelpCeGetKeywordList ();
extern	char		*_DtHelpCeGetVolumeName();
extern	CEBoolean	 _DtHelpCeGetTopTopicId ();
extern	int		 _DtHelpCeOpenVolume ();
extern	int		 _DtHelpCeUpVolumeOpenCnt ();
#else
extern	int		 _DtHelpCeCloseVolume (
				CanvasHandle canvas_handle,
				VolumeHandle vol );
extern	CEBoolean	 _DtHelpCeFindId (
				VolumeHandle	  vol,
				char		 *target_id,
				int		  fd,
				char		**ret_name,
				int		 *ret_offset );
extern	int		 _DtHelpCeFindKeyword (
				VolumeHandle	   vol,
				char		  *target,
				char		***ret_ids );
extern	int		 _DtHelpCeGetKeywordList (
				VolumeHandle	   vol,
				char		***ret_keywords );
extern	CEBoolean	 _DtHelpCeGetTopTopicId (
				VolumeHandle	  vol,
				char		**ret_idString );
extern	char		*_DtHelpCeGetVolumeName(
				VolumeHandle	   vol);
extern	int		 _DtHelpCeOpenVolume (
				CanvasHandle canvas_handle,
				char		*volFile,
				VolumeHandle	*retVol );
extern	int		 _DtHelpCeUpVolumeOpenCnt (
				VolumeHandle	   volume);
#endif /* _NO_PROTO */
#endif /* _DtHelpAccess_h */
