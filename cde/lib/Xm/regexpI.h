/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
#if defined(SUN_MOTIF) || defined(NOVELL_MOTIF)
/*
 *      $XConsortium: regexpI.h /main/cde1_maint/2 1995/08/18 19:41:30 drk $
 *
 *      @(#)regexpI.h	1.3 04 Mar 1994
 */
/*************************************************************************
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *************************************************************************/

#pragma ident "@(#)regexpI.h	1.3 94/03/04 SMI"
/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#define NSUBEXP  10
#define	MAGIC	0234
typedef struct __sun_regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} _sun_regexp;

extern _sun_regexp *_sun_regcomp(char *s);
extern int _sun_regexec(_sun_regexp *r, char *s);
#endif /* SUN_MOTIF */
