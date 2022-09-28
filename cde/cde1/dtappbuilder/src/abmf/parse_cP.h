
/*
 *	$XConsortium: parse_cP.h /main/cde1_maint/1 1995/07/14 22:21:42 drk $
 *
 *	@(#)parse_cP.h	1.5 13 Sep 1994	cde_app_builder/src/abmf
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * parse_cP.h
 *
 * Parses a C file into "segments" to help determine what changes the
 * user has made.
 */
#ifndef _ABMF_PARSE_CP_H_
#define _ABMF_PARSE_CP_H_

#include "abmfP.h"

#define MAX_USER_SEGS_PER_CSEG 10

typedef enum
{
    CSEG_UNDEF = 0,
    CSEG_AGGREGATE_TYPE,
    CSEG_AGGREGATE_VAR,
    CSEG_COMMENT,
    CSEG_FUNC,
    CSEG_GLOBAL,
    CSEG_TYPE_NUM_VALUES	/* must be last */
} CSEG_TYPE;

typedef struct C_USER_SEG_REC
{
    long		line;
    long		offset;
    long		length;
    STRING		text;
    struct C_USER_SEG_REC	*next;		/* next userseg, bypasses segments */
    void		*clientData;
} CUserSegRec, *CUserSeg;

typedef struct /* this struct tag is here as a test */ C_USER_SEGS_REG
{
    int		numSegs;
    CUserSegRec	segs[MAX_USER_SEGS_PER_CSEG];
} CUserSegsRec, *CUserSegs;


typedef struct
{
    CSEG_TYPE		type;
    STRING		name;		/* type or func name */
    long		offset;
    long		length;

    /* these fields may be NULL */
    STRING		text;		/* actual text from file */
    STRING		declaration;
    int			numUserSegs;
    CUserSegsRec	userSegs;
    void		*clientData;	/* client controls this */
} CSegRec, *CSeg;

typedef struct
{
    int			numSegs;
    CSeg		segs;
    CUserSeg		firstUserSeg;
} CSegArrayRec, *CSegArray;


int		abmfP_parse_c_file(
			FILE		*cFile,
			STRING		cFileName,
			CSegArray	*segmentsOut
		);

CSegArray	cseg_array_create(void);
int		cseg_array_destroy(CSegArray);


/*
 * Private symbols
 */
#define cseg_array_destroy(array) (cseg_arrayP_destroy_impl(&(array)))
int		cseg_arrayP_destroy_impl(CSegArray *);

#endif /* _ABMF_PARSE_CP_H_ */
