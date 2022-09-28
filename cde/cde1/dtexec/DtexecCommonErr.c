/* $XConsortium: DtexecCommonErr.c /main/cde1_maint/2 1995/10/09 13:56:36 pascale $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         DtexecCommonErr.c
 **
 **   Description:  This is a clone of CommonErr.c found in libDtSvc,
 **                 and should be kept in sync logically as much as
 **                 possible.
 **
 **                 The routine _DtexecUserMsgHandler() goes through
 **                 the same logic as _DtUserMsgHandler() to dump an
 **                 error message to the appropriate log file.
 **
 **   (c) Copyright 1987, 1988, 1989, 1990 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#define ERROR_LOG_FILE "/.dt/errorlog"
#define ERROR_LOG_FILE2 "/tmp/errorlog."

#define MAX_DATE_TIME_STR 256

/*
 * This function simply writes the error message to the file: 
 *
 *   ~/.dt/errorlog
 */

void 
#ifdef _NO_PROTO
_DtexecUserMsgHandler( progName, execString, execMessage )
        char * progName ;
	char **execString;
        char * execMessage ;
#else
_DtexecUserMsgHandler(
        char * progName,
	char **execString,
        char * execMessage)
#endif /* _NO_PROTO */
{
   char *errorfile = NULL;
   char *homedir;

   struct passwd *pword = NULL;
   FILE      *errlog;
   time_t     now;
   struct tm *tms;
   char      dateTimeStr[MAX_DATE_TIME_STR];

   /* Get the path to the error file; start in user's home directory */
   if ((homedir = getenv("HOME")) == NULL)
   {
      pword = getpwuid(getuid());
      homedir = pword->pw_dir;
   }
   errorfile = malloc(strlen(homedir) + strlen(ERROR_LOG_FILE) + 1);
   (void)strcpy(errorfile, homedir);
   (void)strcat(errorfile, ERROR_LOG_FILE);
   if ((errlog = fopen(errorfile, "a+")) == NULL)
   {
      /* 
       * If we can't create the file in the user's home directory,
       * then we'll try to create it in /tmp.  If that fails, then
       * we'll just write to stderr.
       */
      if (pword == NULL)
         pword = getpwuid(getuid());
      errorfile = realloc(errorfile, 
                  strlen(pword->pw_name) + strlen(ERROR_LOG_FILE2) + 1);
      (void)strcpy(errorfile, ERROR_LOG_FILE2);
      (void)strcat(errorfile, pword->pw_name);
      if ((errlog = fopen(errorfile, "a+")) == NULL)
         errlog = stderr;
   }

   (void)fprintf (errlog, "\n");

   now = time((time_t *)0);
   tms = localtime(&now);
   strftime( dateTimeStr , MAX_DATE_TIME_STR, "%c", tms );

   (void)fprintf (errlog, "%s (%s) %s\n", dateTimeStr, progName, execString[0]);
   (void)fprintf (errlog, "\n");
   (void)fprintf (errlog, "%s\n", execMessage);

   if (errlog != stderr)
      (void)fclose(errlog);

   free(errorfile);
   return;
}
