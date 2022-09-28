/* $XConsortium: AccessSDLP.h /main/cde1_maint/1 1995/07/17 17:19:40 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        AccessSDLP.h
 **
 **   Project:     Run Time Project File Access
 **
 **   Description: Header file for Access.h
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
#ifndef _DtHelpAccessSDLP_h
#define _DtHelpAccessSDLP_h

#include "CESegmentI.h"
#include "SDLI.h"

/****************************************************************************
 *			Private Structure
 ****************************************************************************/
/*
 * The following structure holds loaded volumes.  The fields of this 
 * structure should not be accessed by any code outside of the volume
 * module.
 */
typedef	struct {
    CESegment *sdl_info;	/* A pointer to the SDL doc info     */
    CESegment *toss;		/* A pointer to the Table of Styles  */
				/* and Semantics withing the volume  */
    CESegment *loids;		/* A pointer to the List of Ids      */
    CESegment *index;		/* A pointer to the List of Keywords */
    CESegment *title;		/* A pointer to the document head    */
    CESegment *snb;		/* A pointer to the document's snb   */
    short      minor_no;	/* The minor number of the sdl version */
    CEBoolean  title_processed;	/* If the title has already been searched for */

} CESDLVolume;

#endif /* _DtHelpAccessSDLP_h */
