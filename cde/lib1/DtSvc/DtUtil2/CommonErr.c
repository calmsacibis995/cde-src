/* $XConsortium: CommonErr.c /main/cde1_maint/2 1995/10/08 22:20:11 pascale $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         CommonErr.c
 **
 **   Project:	    DT
 **
 **   Description:  This file contains the common function used to
 **                 write an error message out to the user's errorlog
 **                 file. 
 **
 **   (c) Copyright 1987, 1988, 1989, 1990 by Hewlett-Packard Company
 **
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Dt/UserMsg.h>

#define ERROR_LOG_FILE "/.dt/errorlog"
#define ERROR_LOG_FILE2 "/tmp/errorlog."

/*
 * NLS macros and externs
 */
#ifdef _NO_PROTO
extern char *Dt11GetMessage ();
#else
extern char *Dt11GetMessage (
        char *filename,
        int set,
        int number,
        char *string);
#endif /* _NO_PROTO */

#ifdef MESSAGE_CAT
#define _MESSAGE_CAT_NAME       "dt"
#define Dt11GETMESSAGE(set, number, string)\
    Dt11GetMessage(_MESSAGE_CAT_NAME, set, number, string)
#else /* MESSAGE_CAT */
#define Dt11GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

/*
 * This function simply writes the error message to the file: 
 *
 *   ~/.dt/errorlog
 *
 * NOTE - Do not add any libXt or libXm dependencies to this
 *        file because it is used by dtspcd and that component
 *        does not link in libXt.
 *
 * The only parameters which are used are:
 *        help, message, severity, errnum, errname and errmsg
 */

Boolean 
#ifdef _NO_PROTO
_DtUserMsgHandler( progName, help, message, severity, errnum, errname, errmsg, 
                  client_data )
        String progName ;
        String help ;
        String message ;
        DtSeverity severity ;
        unsigned int errnum ;
        String errname ;
        String errmsg ;
        XtPointer client_data ;
#else
_DtUserMsgHandler(
        String progName,
        String help,
        String message,
        DtSeverity severity,
        unsigned int errnum,
        String errname,
        String errmsg,
        XtPointer client_data )
#endif /* _NO_PROTO */
{
   String errorfile = NULL;
   String homedir;
   struct passwd * pword = NULL;
   FILE * errlog;
   /* time library call takes time_t param */
   time_t now;

#ifdef NLS16
   struct tm * tms;
   char buf[256];
#endif

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
   now = time((time_t)0);
#ifdef NLS16
   tms = localtime(&now);
   (void)strftime(buf, 256, ((char *)Dt11GETMESSAGE(48, 1, "%a %b %d %H:%M:%S %Y\n")), tms);
   (void)fprintf(errlog, "%s", buf);
#else
   (void)fprintf (errlog, "%s", ctime(&now));
#endif

   switch (severity)
   {
      case DtFatalError:
      case DtError:
      case DtWarning:
      {
         (void)fprintf(errlog, "%s: %s\n", progName, message);
         break;
      }

      default:
      {
         (void)fprintf(errlog, "%s\n", message);
         break;
      }
   }

   if (help)
      (void)fprintf(errlog, "  %s\n", help);

   if (errnum)
      (void)fprintf(errlog, "  [%s] %s\n", errname, errmsg);

   if (errlog != stderr)
      (void)fclose(errlog);

   free(errorfile);
   return(severity == DtFatalError);
}
