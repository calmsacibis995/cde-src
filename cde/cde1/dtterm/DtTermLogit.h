/*
 * $XConsortium: DtTermLogit.h /main/cde1_maint/1 1995/07/15 01:13:17 drk $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_DtTermLogit_h
#define	_DtTermLogit_h

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

void LogStart(int noFork, int argc, char **argv);
void LogFinish(int noFork, int sessions);
void LogBumpSessionCount(int count);

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

/* DON'T ADD ANYTHING AFTER THIS #endif... */
#endif	/* _DtTermLogit_h */
