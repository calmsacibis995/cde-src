/* $XConsortium: fsrtns.h /main/cde1_maint/2 1995/08/29 19:45:14 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           fsrtns.h
 *
 *
 *   DESCRIPTION:    Header files used in dosync.c,fsrtns.c,main_dtcopy.c
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _fsrtns_h
#define _fsrtns_h

/*
 *  General comments:
 *
 *  The file systems routines below support renaming, moving, copying,
 *  and deleting single files as well as directories.  The operations
 *  check return codes on close, so they are safe to use with AFS.
 *
 *  If the operation failed, the the reason for the failure (errno value)
 *  is returned in *rcP.
 *
 *  The rename, move and copy routines all take a parameter 'replace'
 *  that determines what happens if the target file/directory already
 *  exists: if 'replace' is zero, the routines return an ENOENT error;
 *  if 'replace' is non-zero the routines will replace any existing
 *  files/directories.
 *
 *  The copy and move operations preserve the modified timestamps and
 *  file permissions of the source.  If the calling program is running
 *  as root, file ownership is also preserved.
 */

#ifdef _NO_PROTO
extern void fsRename();
extern void fsMove();
extern void fsCopy();
extern void fsCopyLink();
extern void fsErase();
extern void fsEmpty();
extern int (*progressCallback)();
extern int (*errorCallback)();
extern int (*periodicCallback)();
#else

/*
 *  fsRename:
 *    Renames or moves source to target.
 *    Source and target must be on the same file system.
 */
extern void fsRename(char *source, char *target, int replace, int *rcP);

/*
 *  fsMove:
 *    Like fsRename, but also works if source and target are on different
 *    file systems by copying source to target and then deleting source.
 */
extern void fsMove(char *source, char *target, int replace, int *rcP);

/*
 *  fsCopy:
 *    Copies source to target.
 */
extern void fsCopy(char *source, char *target, int replace, int *rcP);

/*
 *  fsCopyLink:
 *    Like fsCopy, but copies symbolic links as links, i.e.,creates a link
 *    that points to the same place as the source link.  Applies in the case
 *    where source itself is a symbolic link, as well as the case where
 *    source is a directory that contains symbolic links.
 */
extern void fsCopyLink(char *source, char *target, int replace, int *rcP);

/*
 *  fsErase:
 *    Deletes a file or directory.
 */
extern void fsErase(char *name, int *rcP);

/*
 *  fsEmpty:
 *    Deletes all files/directries in the named directory.
 *
 */
extern void fsEmpty(char *name, int *rcP);


/*
 * Callback functions:
 *
 *   progressCallback:
 *     called each time a new file or directory is being processed
 *
 *   errorCallback:
 *     called when a file operation fails
 *
 *   periodicCallback:
 *     called periodically
 *
 * Each callback function should return zero if the file operation
 * is supposed to continue.  To abort the operation, return a non-zero
 * value from one of the callback functions.
 */
extern int (*progressCallback)(char *fname);
extern int (*errorCallback)(char *fname, int errno);
extern int (*periodicCallback)(void);

#endif /* _NO_PROTO */

#endif /* _fsrtns_h */
