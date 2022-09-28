/*
 * File:	walkfs.c $Revision: 1.3 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#if !defined(_HPUX_SOURCE)
#define _HPUX_SOURCE
#endif
#define _APOLLO_SOURCE
#define _SUN_SOURCE
#define _OSF_SOURCE
#define SYMLINKS

#if defined(hpux) || defined(__hpux)
#define DISKLESS
#endif

#if defined(apollo) || defined(USL) || defined(osf) || defined(sun) || defined(__aix)
#define FSTYPE_KLUDGE
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>

#ifndef apollo
/*#ifndef FSTYPE_KLUDGE*/
# include <sys/mount.h>
/*#endif*/
#endif

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <codelibs/walkfs.h>
#include "ftw.h"

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#define TRUE         1
#define FALSE        0
#define ENDOF(s) (&s[strlen(s)])

#ifdef SYMLINKS
#ifndef sun
extern int lstat();
#endif
#endif /* SYMLINKS */

#if defined(DUX) || defined(DISKLESS)
#define ISCDF(st) (((st).st_mode & (S_IFMT|S_CDF)) == (S_IFDIR|S_CDF))
static char Fdocdf;
static char Fshowcdf;
#endif

typedef int (*statfn_type)(char *, struct stat *);

static char Fftw;
static char Fquick;
static char Fpop;
static char Fstripdot;
static int (*statfn)(char *, struct stat *);
static char *fullpath;
static char *relpath;

#ifdef __cplusplus
static int (*fn)(...);
#else
static int (*fn)();
#endif


typedef struct
{
    struct walkfs_info i;
    int dot_removed;		/* TRUE if path ended in /.           */
    DIR *dirp;			/* DIR file descriptor                */
    long here;                  /* saved place in dirp                */
} state_t;

/*
 * maybe_mountpoint() -- Return TRUE if 'kid' might be a mount point
 */
static int
maybe_mountpoint(struct stat *dad, struct stat *kid)
{
    /* 
     * If this isn't a directory, or the st_dev fields of the
     * parent match the kid and the kid's inode is not 2 (HFS
     * mount point) it is not a mount point.  Since '/' is a
     * mount point but "." has the same dev field as "..", we
     * have to have this extra test for kid->st-ino != 2.
     */
    if ((kid->st_mode & S_IFMT) != S_IFDIR ||
	(kid->st_dev == dad->st_dev && kid->st_ino != 2))
	return FALSE;

#ifdef FSTYPE_KLUDGE  /* no st_fstype field returned by stat(2) */
    /*
     * Don't know what type of filesystem we are on so just return
     * TRUE if the inode is 2 (HFS mountpoint) or the dev fields are
     * different.
     */
    return kid->st_ino == 2 || kid->st_dev != dad->st_dev;
#else
    /* 
     * If the current directory is of type HFS, we just see if
     * the inode is 2 (the mount point is always inode 2 on HFS).
     * If this is not HFS, we can only see if the dev fields 
     * of this entry and its parent are different.
     */
    return (kid->st_fstype == MOUNT_UFS) ?
	    kid->st_ino == 2 : kid->st_dev != dad->st_dev;
#endif
}

/*
 * ismountpoint() -- Determine if this is a mount point.
 */
static int
ismountpoint(char *path, struct stat *dad, struct stat *kid)
{
    int rc = maybe_mountpoint(dad, kid);

#ifndef SYMLINKS
    return rc;
#else
    /* 
     * If maybe_mountpoint() returned FALSE or we are using lstat()
     * to stat things, we just return what maybe_mountpoint()
     * returned.
     */
    if (!rc || statfn == (statfn_type)lstat)
	return rc;

    /* 
     * This might be a mount point and we are not using lstat().
     * Lstat() the file and see if it is a symlink.  If it is, return
     * FALSE, otherwise return TRUE.
     */
    {
	struct stat st;

	return lstat(path, &st) != -1 &&
	    (st.st_mode & S_IFMT) != S_IFLNK;
    }
#endif /* SYMLINKS */
}

/*
 * checkmount() -- Determine if the starting point is a mount point.
 */
static int
checkmount(struct walkfs_info *info)
{
    struct stat pst;
    int rc;

#if defined(DUX) || defined(DISKLESS)
    strcpy(info->endp, "/..+");
    if ((rc = (*statfn)(info->relpath, &pst)) == -1 || !ISCDF(pst))
#endif
    {
	strcpy(info->endp, "/..");
	rc = (*statfn)(info->relpath, &pst);
    }

    *(info->endp) = '\0';
    if (rc == -1)
	return FALSE;
    errno = 0;

    return ismountpoint(info->shortpath, &pst, &info->st);
}

/*
 * Requires info->endp and info->basep to be set properly when called.
 *
 * do_stat --
 *    Stat a file, returning one of the following
 *    codes:
 *       WALKFS_NOSTAT  -- No stat, Stat failed
 *       WALKFS_NONDIR  -- File
 *       WALKFS_DIR     -- Directory
 *
 *       If WALKFS_NOSTAT is returned, the global "errno" will indicate
 *       the error.
 *
 * As a side effect, the following info fields will be set:
 *	info->st
 *	info->shortpath
 *	info->ismountpoint
 */
static
int
do_stat(struct walkfs_info *info, ino_t inode)
{
    int rc = -1;
    if (inode) ;

    /*
     * Build the name to use for the stat() call.
     * Use relpath if in slow mode or this is the start point,
     * otherwise use basep.
     */
    info->shortpath =
	       (Fquick && info->parent != NULL) ? info->basep : relpath;

#if defined(DUX) || defined(DISKLESS)
    if (Fdocdf)
    {
	/* 
	 * Optimization:  If we know what the inode of the
	 * file should be, use that to detect CDF's instead
	 * of stat-ing the file with a trailing +.  Since
	 * CDF's are rare, this first stat will usually
	 * succeed.  If the stat without the '+' succeeds but
	 * is not the file that we expected, we pretend that the
	 * stat failed (so that we will stat it later with a '+').
	 */
	if (inode != 0)
	{
	    rc = (*statfn)(info->shortpath, &info->st);
	    if (rc != -1 && info->st.st_ino != inode)
		rc = -1;
	}

	/* See if the file is a CDF:  */
	if (rc == -1)
	{
	    *info->endp++ = '+';
	    *info->endp = '\0';
	    if ((rc = (*statfn)(info->shortpath, &info->st)) == -1 ||
	        !ISCDF(info->st))
	    {
		*--info->endp = '\0';
		rc = -1;
	    }
	}
    }
#endif /* defined(DUX) || defined(DISKLESS) */

    /* Not a CDF, try stat-ing the file */
    if (rc == -1)
	rc = (*statfn)(info->shortpath, &info->st);

    if (rc == -1)
    {
	info->ismountpoint = FALSE;
	return WALKFS_NOSTAT;
    }

    errno = 0;
    info->ismountpoint = info->parent == NULL ?
			   checkmount(info) : 
			   ismountpoint(info->shortpath, &info->parent->st, &info->st);

    return (info->st.st_mode & S_IFMT) == S_IFDIR ? 
	WALKFS_DIR : WALKFS_NONDIR;
}

/*
 * See if this entry is executable (searchable).  Root (uid=0)
 * can search no matter what.  For non-root, we must
 * check the correct execute bits.
 */
static int
isexec(struct stat *st)
{
    uid_t uid = geteuid();

    if (uid == 0)
	return TRUE;
    if (uid == st->st_uid)
	return (st->st_mode & S_IEXEC) != 0;
    if (getegid() == st->st_gid)
	return (st->st_mode & (S_IEXEC>>3)) != 0;
    return (st->st_mode & (S_IEXEC>>6)) != 0;
}

/*
 * getdir --
 *   Open a directory, saving our place in the parent directory if we
 *   have used all the file descriptors that we were told we could or
 *   we run out.  If here is not -1, we don't save and close the
 *   parent directory (we assume that it has already been done).
 */
static void
getdir(state_t *parentstate, state_t *state, int depth)
{
    register char *path = Fquick ? state->i.shortpath : relpath;

    if (parentstate && parentstate->here == -1 && depth < 1)
    {
	parentstate->here = telldir(parentstate->dirp);
	closedir(parentstate->dirp);
    }

    if ((state->dirp = opendir(path)) == NULL &&
	errno == EMFILE && parentstate && parentstate->here == -1)
    {
	parentstate->here = telldir(parentstate->dirp);
	closedir(parentstate->dirp);
	state->dirp = opendir(path);
    }
}

/*
 * real_walkfs --
 *    Recursive function that processing directories.  The parameter
 *    'inode' indicates the inode # that we expect from a stat of
 *    the current entry.  If we don't know what we expect (initial
 *    condition), 'inode' will be 0 (no file ever has an st_ino == 0).
 */
static
int
real_walkfs(state_t *parentstate, int depth, ino_t inode)
{
    state_t state;		/* information about this call's file */
    struct dirent *dp;		/* directory entry                    */
    int rc;			/* Return code from user function     */
    int retry = 0;		/* Retry bit to OR into flags         */
    int did_chdir = FALSE;	/* did we do a chdir successfully     */
    int can_srch;		/* can we search this directory       */

    if (parentstate == NULL)	/* is this the start point? */
    {
	state.i.parent = NULL;
	state.i.basep = strrchr(relpath, '/');
	if (state.i.basep == NULL)
	    state.i.basep = relpath;
	else
	    state.i.basep++;
    }
    else
    {
	state.i.parent = &parentstate->i;
	state.i.basep = parentstate->i.endp;
    }

    state.i.endp = ENDOF(state.i.basep);
    state.i.relpath = relpath;
    state.i.fullpath = fullpath;
    state.dirp = NULL;
    state.here = -1;

    /* 
     * Repeatedly stat this entry until the stat succeeds or the user
     * says to give up.  If it is just a file, call the user function.
     * If it is a directory, set up for a recursive call to ourselves.
     */
    for (;;)
    {
	switch (do_stat(&state.i, inode))
	{
	case WALKFS_NOSTAT: 
	    /* 
	     * We ignore ENOENT errors except at the start point. 
	     * These errors can occur in non-WALKFS_DOCDF mode when
	     * there is no match for our context or if the file
	     * disappears between the time we read the directory and
	     * we get around to stating it.
	     */
	    if (parentstate != NULL && errno == ENOENT)
	    {
		errno = 0;
		return WALKFS_OK;
	    }

	    if (Fftw)
		return (*fn)(relpath, &state.i.st, FTW_NS);

	    rc = (*fn)(&state.i, retry | WALKFS_NOSTAT);
	    if (rc == WALKFS_RETRY)
	    {
		retry = WALKFS_RETRY;
		continue;
	    }
	    return rc == WALKFS_SKIP ? WALKFS_OK : rc;
	case WALKFS_NONDIR: 
	    if (Fftw)
		return (*fn)(relpath, &state.i.st, FTW_F);
	    return (*fn)(&state.i, retry | WALKFS_NONDIR);
	case WALKFS_DIR: 
	    break;
	}

	/* 
	 * Directory processing.  Keep trying to open and chdir (Fquick)
	 * or search (!Fquick) this directory until we succeed or the
	 * user function tells us to give up.
	 */
	getdir(parentstate, &state, depth);

	if (Fftw)
	{
	    if (state.dirp == NULL)
		return (*fn)(relpath, &state.i.st, FTW_DNR);
	    if ((rc = (*fn)(relpath, &state.i.st, FTW_D)) != 0)
		return rc;
	    break;
	}

	if (Fquick && state.dirp != NULL)
	{
	    if (can_srch = did_chdir =
		    (chdir(state.i.shortpath) != -1))
		state.i.shortpath = ".";
	}
	else if (!(can_srch = isexec(&(state.i.st))))
	    errno = EACCES;

	if (state.dirp == NULL || !can_srch)
	{
	    int flags = retry | WALKFS_ERROR;

	    if (state.dirp == NULL)
		flags |= WALKFS_NOREAD;
	    if (!can_srch)
		flags |= WALKFS_NOSEARCH;

	    if ((rc = (*fn)(&state.i, flags)) == WALKFS_RETRY)
	    {
		retry = WALKFS_RETRY;
		if (state.dirp != NULL)
		{
		    closedir(state.dirp);
		    state.dirp = NULL;
		}
		continue;
	    }
	    rc = WALKFS_SKIP;
	    break;
	}

	fcntl(state.dirp->dd_fd, F_SETFD, 1); /* close dir on exec */

	/*
	 * Call the user function for the directory
	 */
	if ((rc = (*fn)(&state.i, retry | WALKFS_DIR)) == WALKFS_RETRY)
	    rc = WALKFS_OK;
	break;
    }

    /* 
     * Read the directory one entry at a time.  We ignore "." and
     * "..", stat'ing all other entries.  All entries get processed
     * by a recursive call.
     */
    if (rc == WALKFS_OK)
    {
	int slash_added = FALSE;

	state.dot_removed = FALSE;
	/* 
	 * This endp[-x] stuff is dependent on an absolute path
	 * name being in 'fullpath'.  Since Fstripdot is always
	 * FALSE in Fftw mode, this is not a problem.
	 */
	if (Fstripdot && state.i.endp[-1] == '.' &&
		state.i.endp[-2] == '/')
	{
	    /* remove redundant "." from end of path */
	    *--state.i.endp = '\0';
	    state.dot_removed = TRUE;
	}
	else if (state.i.endp[-1] != '/')
	{
	    *state.i.endp++ = '/';  /* Add a trailing slash */
	    slash_added = TRUE;
	}

	while ((rc == WALKFS_OK || (rc == WALKFS_RETRY && !Fftw)) &&
		(dp = readdir(state.dirp)) != NULL)
	{
	    register char *nm = dp->d_name;

	    /* skip entries for deleted links */
	    if (dp->d_ino == 0 || *nm == '\0')
		continue;

	    /* skip . and .. entries */
	    if (nm[0] == '.' &&
		    (nm[1] == '\0' || (nm[1] == '.' && nm[2] == '\0')))
		continue;

	    strcpy(state.i.endp, nm);
	    rc = real_walkfs(&state, depth - 1, dp->d_ino);
	}

	/* restore original directory name */
	if (slash_added)
	    state.i.endp--;
	if (state.dot_removed)
	    *state.i.endp++ = '.';
	*state.i.endp = '\0';
    }

    /* 
     * Call the user function to indicate that we are done with
     * this directory.  We do this before modifying any of the
     * fields of state.i so that it has the same stuff as when we
     * called with WALKFS_DIR.  Note that we just called the user
     * function with WALKFS_DIR if we are in depth first mode.
     * (i.e. WALKFS_TELLPOPDIR is silly with WALKFS_DEPTH, but
     * it is allowed).
     */
    if (Fpop)
	(*fn)(&state.i, WALKFS_POPDIR);

    /* 
     * Restore parent's absolute path.  We need the absolute path
     * for chdir() to avoid problems with RFA and SYMLINKS.
     */
    if (state.i.basep != fullpath)
    {
	if (parentstate && parentstate->dot_removed)
	{
	    state.i.basep[0] = '.';
	    state.i.basep[1] = '\0';
	}
	else
	    state.i.basep[0] = '\0';
    }

    /*
     * Chdir to our parent directory.
     *
     * No need to chdir back if this is the top level of the recursion.
     * The "walkfs" function that called us originally will do that
     * for us.
     *
     * If an absolute chdir fails, try a relative one, but make sure
     * that we are where we expect to be (in case of symlinks, etc.).
     * If we still aren't where we want to be, call the user function
     * with WALKFS_NOCHDIR.
     *
     * At this point, the user function may return WALKFS_RETRY.
     * We then see if he got us to where we wanted to be -- If he
     * did, we proceed as if nothing happened; if not we again
     * call him with WALKFS_RETRY (until he gets us where we want
     * or he returns something other than WALKFS_RETRY).
     *
     * If the user function returns a value other than WALKFS_RETRY,
     * we return WALKFS_NOCHDIR, terminating all the levels of
     * recursion.
     */
    if (did_chdir && parentstate != NULL && chdir(fullpath) == -1)
    {
	struct stat dot;
	dev_t dev = parentstate->i.st.st_dev;
	ino_t ino = parentstate->i.st.st_ino;

	if (chdir("..") == -1 || stat(".", &dot) == -1 ||
	    dot.st_ino != ino || dot.st_dev != dev)
	{
	    do
	    {
		rc = (*fn)(&(parentstate->i), WALKFS_NOCHDIR);
	    } while (rc == WALKFS_RETRY && (stat(".", &dot) == -1 ||
		      dot.st_ino != ino || dot.st_dev != dev));

	    /*
	     * If rc is WALKFS_RETRY, we must be where we wanted
	     * since we are out of the loop.  Otherwise, we abort with
	     * WALKFS_NOCHDIR.
	     */
	    if (rc != WALKFS_RETRY)
		return WALKFS_NOCHDIR;
	}
    }

    if (state.dirp != NULL)
	closedir(state.dirp);

    if (parentstate && parentstate->here != -1)
    {
	/*
	 * If we are in "Fquick" mode, use a relative path to re-open
	 * the directory, in case fullpath is no longer valid but
	 * ".." was.
	 */
	seekdir((parentstate->dirp = opendir(Fquick ? "." : fullpath)),
		parentstate->here);
	parentstate->here = -1;
	fcntl(parentstate->dirp->dd_fd, F_SETFD, 1);
    }

    if (Fftw)
	return rc;
    return (rc == WALKFS_SKIP || rc == WALKFS_RETRY) ? WALKFS_OK : rc;
}

#ifdef __cplusplus
/* This is required to get around a bug in C++.  Since struct stat and */
/* the function stat have the same name, C++ barf when you try and use */
/* "stat" as a function pointer.  Use one level of indirection here.   */
int
mystat (const char * path, struct stat *buf)
{
    return stat(path, buf);
}
#endif


int
#ifdef __cplusplus
walkfs(char *in_path, int (*in_fn)(...), int depth, int cntl)
#else
walkfs(char *in_path, int (*in_fn)(), int depth, int cntl)
#endif
{
    char cwd[MAXPATHLEN];
    char pathbuf[MAXPATHLEN];

    fullpath = pathbuf;
    fn = in_fn;

    Fftw   = !!(cntl & HIBITI); /* undocumented hook for ftw[h](LIBC) */

#if defined(DUX) || defined(DISKLESS)
    Fdocdf = !!(cntl & WALKFS_DOCDF);
    if (Fftw)
	Fshowcdf = FALSE;
    else
	Fshowcdf  = !!(cntl & WALKFS_SHOWCDF);
#endif

    if (Fftw)
        Fpop = Fstripdot = Fquick = FALSE;
    else
    {
	/*
	 * Save commonly used flag values in global variables for
	 * efficiency.
	 */
	Fpop      = !!(cntl & WALKFS_TELLPOPDIR);
	Fstripdot =  !(cntl & WALKFS_LEAVEDOT);
	Fquick    =  !(cntl & WALKFS_SLOW);
    }

#ifdef SYMLINKS
    if (!Fftw && (cntl & WALKFS_LSTAT))
	statfn = (statfn_type)lstat;
    else
#endif /* SYMLINKS */
#ifdef __cplusplus	
	statfn = (statfn_type)mystat;
#else
        statfn = (statfn_type)stat;
#endif    

    if (!Fftw && (Fquick || in_path[0] != '/'))
#if defined(DUX) || defined(DISKLESS)
	if (Fshowcdf)
	    gethcwd(cwd, sizeof cwd);
	else
#endif /* defined(DUX) || defined(DISKLESS) */
	    getcwd(cwd, sizeof cwd);

    /* 
     * Make an absolute pathname out of in_path so we can change
     * to it safely even if we have SYMLINKS.  We don't make it
     * an absolute path name in WALKFS_FTW mode since we don't
     * need the absolute path in this case.
     */
    if (!Fftw && in_path[0] != '/')
    {
	strcpy(fullpath, cwd);
	relpath = ENDOF(fullpath);
	*relpath++ = '/';
    }
    else
	relpath = fullpath;
    *relpath = '\0';

    /*
     * Copy in_path to relpath.  If Fshowcdf is set, copy the
     * expanded version of in_path to relpath.
     */
#if defined(DUX) || defined(DISKLESS)
    if (Fshowcdf)
    {
	if (!getcdf(in_path, relpath, sizeof cwd - strlen(fullpath)))
	    strcpy(relpath, in_path);
    }
    else
#endif /* defined(DUX) || defined(DISKLESS) */
	strcpy(relpath, in_path);

    /*
     * Now call real_walkfs to do the real work.  Note that in
     * WALKFS_FTW mode, cwd is garbage, but Fquick is always false
     * in this case, so we don't have to worry (be happy!).
     */
    {
	int rc = real_walkfs(NULL, depth, 0);

	if (Fquick)
	    chdir(cwd);

	return rc;
    }
}
