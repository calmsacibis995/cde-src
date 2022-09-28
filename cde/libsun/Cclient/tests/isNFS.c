/*
 * @(#)isNFS.c	1.1 97/05/27
 *
 * Exits ZERO if nfs mounted on solaris.
 * Exits ONE if it is NOT nfs mounted.
 */
#include "../c-client/UnixDefs.h"
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argv, char ** argc)
{
  struct statvfs	statvfsBuf;
  struct stat		statBuf;

  boolean_t		isNfs = _B_FALSE;

  /* Must exist */
  if (stat(argc[1], &statBuf) == 0) {

    /* And must be a directory */
    if (statBuf.st_mode & S_IFDIR) {

      /* And we must have I/O permission */
      if (access(argc[1], R_OK | W_OK | X_OK) == 0) {

	/* And it must be NFS mounted */
	if (statvfs(argc[1], &statvfsBuf) == 0) {
	  if (strcasecmp(statvfsBuf.f_basetype, "nfs") == 0) {
	    isNfs = _B_TRUE;
	  } else {
	    fprintf(stderr, "%s: ERROR: %s is not NFS mounted.\n",
		    argc[0], argc[1]);
	  }
	} else {
	  fprintf(stderr, "%s: ERROR: can not statvfs() %s .\n",
		  argc[0], argc[1]);
	}
      } else {
	fprintf(stderr, "%s: ERROR: can not access(%s, R_OK|W_OK|X_OK).\n",
		argc[0], argc[1]);
      }
    } else {
      fprintf(stderr, "%s: ERROR: %s is not a directory.\n",
	      argc[0], argc[1]);
    }

  } else {
    fprintf(stderr, "%s: ERROR: can not stat(%s, ...)\n",
	    argc[0], argc[1]);
  }
  return(isNfs ? 0 : 1);
}
