/*
 * $XConsortium: DtTermServer.h /main/cde1_maint/1 1995/07/15 01:14:17 drk $";
 */

/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_DtTermServer_h
#define	_DtTermServer_h

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

extern char *ServerFailureMessage;
extern int ServerFailureErrno;
extern int InstanceCount;
extern int PingInterval;

extern Boolean ServerStartSession(
    Widget		  topLevel,
    int			  argc,
    char		**argv,
    Boolean		  server,
    char		 *serverId,
    Boolean		  exitOnLastClose,
    Boolean		  block,
    Boolean		  loginShell,
    char		**commandToExec
);

extern void ServerInstanceTerminated(
    Widget		  w
);

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

/* DON'T ADD ANYTHING AFTER THIS #endif... */
#endif	/* _DtTermServer_h */
