/* $XConsortium: ActionUtilP.h /main/cde1_maint/2 1995/08/26 22:45:08 barstow $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ActionUtilP.h
 **
 **   Project:     DT
 **
 **   Description: Private include file for the Action Library Utilities.
 **
 **
 ** (c) Copyright 1993, 1994 Hewlett-Packard Company
 ** (c) Copyright 1993, 1994 International Business Machines Corp.
 ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 ** (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _ActionUtilP_h
#define _ActionUtilP_h

#include <X11/Intrinsic.h>		/* for Display struct definition */
#include "ActionP.h"


/*****************************************************************************
 *
 *	Macro to protect against sending a NULL pointer to certain
 *	library functions (i.e. sprintf, strlen, ...) with on some
 *	systems choke on a NULL pointer.
 *
 ****************************************************************************/
#define	_DtActNULL_GUARD(s)	((s) ? (s) : "")

/******************************************************************************
	    External Utility Function Declarations

	These functions are for internal use and are not part of the
	public Action API. Each of the following functions returns a
	newly allocated version of the desired string. It is up to the
	caller to free the strings obtained.
******************************************************************************/

extern char	*_DtBasename(const char *s);
extern char	*_DtDirname(const char *s);
extern char	*_DtPathname(const char *s);
extern char	*_DtHostString(const char *s);
extern char	*_DtGetSessionHostName( void );
extern char	*_DtGetDisplayHostName( Display *d);
extern char	*_DtGetLocalHostName( void );
extern char	*_DtGetExecHostsDefault (void);
extern char	*_DtGetActionIconDefault (void);
extern char	*_DtGetDtTmpDir(void);
extern char 	*_DtActGenerateTmpFile(char *dir,
	char *format,
	mode_t mode,
	int *fd );
extern int	_DtIsSameHost( const char *host1, const char *host2 );
extern void	_DtRemoveTrailingBlanksInPlace(char **s);
extern int	_DtExecuteAccess(const char *path);

extern DtActionInvocationID _DtActAllocID();
extern _DtActInvRecT *_DtActAllocInvRec();
extern _DtActChildRecT *_DtActAllocChildRec( _DtActInvRecT *recp );
extern int _DtActDeleteInvRec( DtActionInvocationID id);
extern _DtActInvRecT *_DtActFindInvRec( DtActionInvocationID id);
extern _DtActChildRecT *_DtActFindChildRec( 
	DtActionInvocationID id,
        unsigned long        childId);
extern unsigned long _DtActEvalChildren(DtActionInvocationID id);
extern void _DtActExecutionLeafNodeCleanup(
	DtActionInvocationID  id,
	DtActionArg           *newArgp,
	int                   newArgc,
	int                   respectQuitBlock);
extern DtActionArg *_DtActMallocEmptyArgArray(int ac);
extern void _DtActFreeArgArray( DtActionArg *argp, int ac );
extern void *_DtActReadTmpFileToBuffer ( 
	char *fname,
	int *sizep);
extern int _DtActGetCmdReturnArgs ( 
        DtActionInvocationID invId,
	_DtActChildRecT *childp,
        DtActionArg     **aargv );

/******************************************************************************/


#endif /* _ActionUtilP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */


