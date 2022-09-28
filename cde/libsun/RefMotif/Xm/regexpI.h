#if defined(SUN_MOTIF) || defined(NOVELL_MOTIF)
/*
 *      $Revision: 1.2 $
 *
 *      @(#)regexpI.h	1.3 04 Mar 1994
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

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
