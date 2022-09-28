/* $XConsortium: AccessP.h /main/cde1_maint/1 1995/07/17 17:18:42 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        AccessP.h
 **
 **   Project:     Run Time Project File Access
 **
 **  
 **   Description: Private header file for Access.h
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
#ifndef _DtAccessP_h
#define _DtAccessP_h

#include <sys/stat.h>
typedef	void*	SdlVolumeHandle;
typedef	void*	CcdfVolumeHandle;

typedef	union _dthelpVolumes {
	SdlVolumeHandle	sdl_vol;
	CcdfVolumeHandle ccdf_vol;
} DtHelpVols;

/*
 * The following structure holds loaded volumes.  The fields of this 
 * structure should not be accessed by any code outside of the volume
 * module.
 */
struct _DtHelpVolumeRec {
    CEBoolean	sdl_flag;	/* The type of volume */
    char *volFile;		/* The name of the volume file in the */
				/* form it was passed to _DtVolumeOpen. */

    char **keywords;		/* A pointer to a string array */
				/* containing all of the keywords in */
				/* sorted order.  This field is not loaded */
				/* until it is needed. */

    char ***keywordTopics;	/* A pointer to an array of string */
				/* arrays.  Each string array specifies */
				/* the list of topics which contain the */
				/* corresponding keyword.  This field is  */
				/* not loaded until it is needed. */

    DtHelpVols vols;		/* Handles to format specific volume info */
    int openCount;		/* A count of the number of times this */
				/* volume has been opened. */

    time_t check_time;		/* Time this volume was last modified */
    struct _DtHelpVolumeRec *nextVol;
				/* A pointer to the next volume, used to */
				/* chain all of the open volumes together. */
};

typedef struct _DtHelpVolumeRec *_DtHelpVolume;

#endif /* _DtAccessP_h */
