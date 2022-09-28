/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <strings.h>

char *
realpath(pathname, finalpath)
char *pathname;
char *finalpath;
{
  struct stat sbuf;
  char curpath[MAXPATHLEN],
  workpath[MAXPATHLEN],
  linkpath[MAXPATHLEN],
  namebuf[MAXPATHLEN],
  *where,
  *ptr,
  *last;
  int len;

   strcpy(curpath, pathname);

   if (*pathname != '/') {
#if defined(__STDC__) && defined(hp)
      if (!getcwd(workpath)) {
#else
      if (!getwd(workpath)) {
#endif
         strcpy(finalpath, ".");
         return(NULL);
       }
    } else *workpath = NULL;

   /* curpath is the path we're still resolving      */
   /* linkpath is the path a symbolic link points to */
   /* workpath is the path we've resolved            */

 loop:
   where = curpath;
   while (*where != NULL) {
      if (!strcmp(where, ".")) {
         where++;
         continue;
       }

      /* deal with "./" */
      if (!strncmp(where, "./", 2)) {
         where += 2;
         continue;
       }

      /* deal with "../" */
      if (!strncmp(where, "../", 3)) {
         where += 3;
         ptr = last = workpath;
         while (*ptr) {
            if (*ptr == '/') last = ptr;
            ptr++;
	  }
         *last = NULL;
         continue;
       }

      ptr = strchr(where, '/');
      if (!ptr)
         ptr = where + strlen(where) - 1;
      else
         *ptr = NULL;

      strcpy(namebuf, workpath);
      for (last = namebuf; *last; last++) continue;
      /* tack on a trailing, or leading, `/` */
      if (last == namebuf || *--last != '/') {
        strcat(namebuf, "/");
      }
      strcat(namebuf, where);

      where = ++ptr;
      if (lstat(namebuf, &sbuf) == -1) {
         strcpy(finalpath, namebuf);
         return(NULL);
       }

      if ((sbuf.st_mode & S_IFLNK) == S_IFLNK) {
         len = readlink(namebuf, linkpath, MAXPATHLEN);
         if (len == 0) {
            strcpy(finalpath, namebuf);
            return(NULL);
	  }
         *(linkpath + len) = NULL; /* readlink doesn't null-terminate result */
         if (*linkpath == '/') *workpath = NULL;
         if (*where) {
            strcat(linkpath, "/");
            strcat(linkpath, where);
	  }
         strcpy(curpath, linkpath);
         goto loop;
       }

      if ((sbuf.st_mode & S_IFDIR) == S_IFDIR) {
         strcpy(workpath, namebuf);
         continue;
       }

      if (*where) {
         strcpy(finalpath, namebuf);
         return(NULL);  /* path/notadir/morepath */
       } else
         strcpy(workpath, namebuf);
    }
   strcpy(finalpath, workpath);
   return(finalpath);

}

