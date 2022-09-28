/*
 * File:         FileUtil.c $XConsortium: FileUtil.c /main/cde1_maint/3 1995/10/04 12:02:38 gtsang $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <sys/types.h>		/* stat(2) */
#include <sys/stat.h>		/* stat(2) */
#include <sys/param.h>		/* MAXPATHLEN */
#include <errno.h>		/* errno(2) */

#ifdef __hpux
#include <ndir.h>		/* opendir(), directory(3C) */
#else
#if defined(sun) || defined(USL) || defined(sco) || defined(__uxp__)
#include <dirent.h>		/* opendir(), directory(3C) */
#else
#include <sys/dir.h>
#endif /* sun || USL */
#endif /* __hpux */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>	/* Xt stuff */
#include <X11/StringDefs.h>	/* Cardinal */
#include <Dt/DtNlUtils.h>
#include <Dt/ActionUtilP.h>
#include <Tt/tt_c.h>

extern int errno;

/******************
 *
 * Function Name:  _DtCreateDirs
 *
 * Description:
 *
 *	This function is passed a directory path to create and the mode
 *	for the directory.  It will create any of the parent directories 
 *	on the path that do not already exist.
 *
 *	This function may fail if any of the directories on the path already
 *	exist and are not writable.  If some component of the path already
 *	exists and is not a directory, a failure will be returned.  
 *
 *	If some component of the path exists as a directory but does not have 
 *	the specified mode, this will NOT cause a failure to be returned.
 *	This implies that if this function is called to create a writeable
 *	directory, it is possible for the function to return successfully
 *	but the directory may not actually be writable.
 *
 * Synopsis:
 *
 *	status = _DtCreateDirs (path, mode);
 *
 *	int status;		Returns 0 on success and -1 on failure.
 *	char *path;		The directory path to create.
 *	int mode;		The file mode for setting any directories
 *				that are created.
 *
 ******************/

int 
#ifdef _NO_PROTO
_DtCreateDirs( path, mode )
        char *path ;
        int mode ;
#else
_DtCreateDirs(
        char *path,
        int mode )
#endif /* _NO_PROTO */
{
   struct stat st_buf;
   int st_status;
   int ret_status;
   char *parent_path;
   char *temp_s;

   /* If the path already exist, make sure it is a directory. */
   if ((st_status = stat (path, &st_buf)) == 0) {
      if (S_ISDIR (st_buf.st_mode))
	 ret_status = 0;
      else
         ret_status = -1;
   }

   /* If we can't stat it, make sure it is simply because some component
      of the path doesn't exist. */
   else if (errno != ENOENT)
      ret_status = -1;

   else {
      /* Some component of the path doesn't exist.  Recursively make the
	 parent of the current directory, then make the current directory. */
      parent_path = XtNewString (path);
      if ((temp_s = DtStrrchr (parent_path, '/')) != NULL) {
         *temp_s = '\0';
	 (void) _DtCreateDirs (parent_path, mode);
      }
      XtFree (parent_path);

      /* If no error has been encountered, now make the final directory
	 in the path. */
      if ((ret_status = mkdir (path, S_IFDIR)) == 0)
            (void) chmod (path, mode);
   }

   return (ret_status);
}

/******************
 *
 * Function Name:  _DtIsOpenableDirContext
 *
 * Description:
 *
 *      This function takes a path as an argument and determines whether
 *	the path is a directory that can be opened.  This function returns
 *	"1" if the path is an openable directory and "0" if it is not.
 *      In addition, if the calling function passes in another pointer,
 *      we will return the internal representation for the path.
 *
 *	The path can be in the Softbench "context" form of "host:/path/dir".
 *
 * Synopsis:
 *
 *	status = _DtIsOpenableDirContext (cpath, ret_ptr)
 *
 *	int status;		Returns 1 for openable directories, 
 *				0 otherwise.
 *	char *cpath;		The directory name to test.
 *      char ** ret_ptr;        Where to place internal format.
 *
 ******************/

int 
#ifdef _NO_PROTO
_DtIsOpenableDirContext( path, ret_path )
        char *path ;
        char **ret_path ;
#else
_DtIsOpenableDirContext(
        char *path,
        char **ret_path )
#endif /* _NO_PROTO */
{
   char *real_path = NULL;
   char * tmp_real_path;
   DIR *dirp;
   int ret_status;
   Tt_status status;
   char * host;
   char * filename;
   char * netfile;

   if (ret_path)
      *ret_path = NULL;

   host = _DtHostString(path);
   filename = _DtPathname(path);
   if (host)
   {
      netfile = tt_host_file_netfile(host, filename);
      if ((status = tt_ptr_error(netfile)) == TT_OK)
      {
         tmp_real_path = tt_netfile_file(netfile);
         status = tt_ptr_error(real_path);
         tt_free(netfile);
      }

      if (status != TT_OK) {
         real_path = NULL;
      } else {
	 real_path = XtNewString(tmp_real_path);
	 tt_free(tmp_real_path);
      }

      XtFree(filename);
      XtFree(host);
   }
   else
      real_path = filename;

   if (real_path && ((dirp = opendir (real_path)) != NULL)) 
   {
      closedir (dirp);
      ret_status = 1;
      if (ret_path)
         *ret_path = real_path;
   }
   else
   {
      ret_status = 0;
      if (real_path)
         XtFree(real_path);
   }

   return (ret_status);
}

/******************
 *
 * Function Name:  _DtIsOpenableDir
 *
 * Description:
 *
 *      This function takes a path as an argument and determines whether
 *	the path is a directory that can be opened.  This function returns
 *	"1" if the path is an openable directory and "0" if it is not.
 *
 *	The path can be in the Softbench "context" form of "host:/path/dir".
 *
 * Synopsis:
 *
 *	status = _DtIsOpenableDir (cpath)
 *
 *	int status;		Returns 1 for openable directories, 
 *				0 otherwise.
 *	char *cpath;		The directory name to test.
 *
 ******************/

int 
#ifdef _NO_PROTO
_DtIsOpenableDir( path )
        char *path ;
#else
_DtIsOpenableDir(
        char *path )
#endif /* _NO_PROTO */
{
   return (_DtIsOpenableDirContext(path, NULL));
}

/******************
 *
 * Function Name:  _DtReaddirLstat
 *
 * Description:
 *
 *	This function reads the next entry out of a directory that has
 *	been opened with opendir and returns lstat information on it.  
 *	For more information on reading a directory, see directory(3C).
 *	For more information on lstat information, see stat(2).
 *
 *	This function returns a pointer to the full pathname of the directory
 *	entry.  This memory is owned by this function and must not be 
 *	freed.  If the caller wants to keep the filename, it must make its
 *	own copy.  When the end of the directory is encountered, NULL is
 *	returned.
 *
 * Synopsis:
 *
 *	dir_entry = _DtReaddirLstat (dir_name, dirp, st_buf);
 *
 *	char *dir_entry;	The name of the current entry within the
 *				directory.
 *
 *	char *dir_name;		The full path name of the directory.
 *
 *	DIR *dirp;		A pointer to the directory [obtained from
 *				opendir(3C)].
 *
 *	struct stat *st_buf;	The lstat(2) information.
 *
 ******************/

char stat_file_name[MAXPATHLEN];

char * 
#ifdef _NO_PROTO
_DtReaddirLstat( dir_name, dirp, st_buf )
        char *dir_name ;
        DIR *dirp ;
        struct stat *st_buf ;
#else
_DtReaddirLstat(
        char *dir_name,
        DIR *dirp,
        struct stat *st_buf )
#endif /* _NO_PROTO */
{
#ifdef __hpux
   struct direct *dir_entry;
#else
   struct dirent *dir_entry; 
#endif    /* __hpux */

   if ((dir_entry = readdir (dirp)) == NULL)
      return (NULL);

   else {
      sprintf (stat_file_name, "%s/%s", dir_name, dir_entry->d_name);
      if (lstat (stat_file_name, st_buf) == -1)
         return (NULL);
      else
         return (stat_file_name);
   }
}
