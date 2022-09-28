/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: error.c /main/cde1_maint/1 1995/10/09 00:09:37 pascale $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include <sys/stat.h>
# include <time.h>
# include "dm.h"
# include "vgmsg.h"

#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif

extern char	DisplayName[];

void
#ifdef _NO_PROTO
InitErrorLog()
#else
InitErrorLog( void )
#endif /* _NO_PROTO */
{
	int	i;

	static	char tz[32];
	
	
	/*
	 *  add TimeZone to our environment so error message time stamps
	 *  have the correct value...
	 */
	 
	if (timeZone != NULL && strlen(timeZone) < 29) {
	    strcpy(tz,"TZ=");
	    strcat(tz,timeZone);
	    putenv(tz);
	}


	if (errorLogFile && errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError(ReadCatalog(
			  MC_LOG_SET,MC_LOG_NO_ERRLOG,MC_DEF_LOG_NO_ERRLOG),
			  errorLogFile);
	}
}

/****************************************************************************
 *
 * CheckErrorFile
 *
 * Just a quick check to verify that we can open an error log.
 * Do this before we do BecomeDeamon.
 *
 ****************************************************************************/
void
#ifdef _NO_PROTO
CheckErrorFile()
#else
CheckErrorFile( void )
#endif /* _NO_PROTO */
{
        int     i;

        if (errorLogFile && errorLogFile[0]) {
                i = creat (errorLogFile, 0666);
                if (i != -1) {
                	close (i);
                } else {
                	fprintf(stderr, (char *)ReadCatalog(
                              MC_LOG_SET,MC_LOG_NO_ERRLOG,MC_DEF_LOG_NO_ERRLOG),
                              errorLogFile);
		}
        } else {
		fprintf(stderr, (char *)ReadCatalog(
                        MC_LOG_SET,MC_LOG_NO_ERRLOG,MC_DEF_LOG_NO_ERRLOG),
			"\"\"");
	}
}


/****************************************************************************
 *
 *  SyncErrorFile
 *
 *  point the stderr stream to the most current Error Log File. This allows
 *  the user to muck with the file while XDM is running and have everything
 *  sync up later.
 *
 *  optionally, write a time stamp to the file
 ****************************************************************************/

int 
#ifdef _NO_PROTO
SyncErrorFile( stamp )
        int stamp ;
#else
SyncErrorFile( int stamp )
#endif /* _NO_PROTO */
{
    time_t   timer;
    
    if (errorLogFile && errorLogFile[0] && 
        (freopen(errorLogFile, "a", stderr) != NULL)) {

	if (stamp) {
	    timer = time(NULL);
	    fprintf(stderr, "\n%s", ctime(&timer));
	}	    
	return(1);
    }
    else
	return(0);
}


/****************************************************************************
 *
 *  TrimErrorFile
 *
 *  Trim the length of the error log file until it is 75% of the maximum
 *  specified by the resource "errorLogSize".
 *
 ****************************************************************************/

void
#ifdef _NO_PROTO
TrimErrorFile()
#else
TrimErrorFile( void )
#endif /* _NO_PROTO */
{

    int  f1, f2;
    int  deleteBytes;
    
    char buf[BUFSIZ];
    char *p;
    int  n;

    off_t status;
    struct stat	statb;


    /*
     *  convert user-specified units of 1Kb to bytes...
     *  put an upper cap of 200Kb on the file...
     */

    if (errorLogSize < 1024) errorLogSize *= 1024;
         
    if (errorLogSize > (200*1024) ) {
	errorLogSize = 200*1024;
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_MAX_LOGFILE,MC_DEF_LOG_MAX_LOGFILE));
    }
	    
    if ( errorLogFile && errorLogFile[0]  && 
        (stat(errorLogFile, &statb) == 0) && 
	(statb.st_size > errorLogSize)		 ) {
    
	deleteBytes = (statb.st_size - errorLogSize) + (errorLogSize / 4);

	Debug("TrimErrorLog(): discarding oldest %d bytes from logfile %s\n",
	         deleteBytes, errorLogFile);


	/*
	 *  get two pointers to the file...
	 */
	 
	if ( (f1 = open(errorLogFile, O_RDWR)) < 0 ||
	     (f2 = open(errorLogFile, O_RDWR)) < 0    ) {
	    Debug("TrimErrorLog(): Cannot open file %s, error number = %d\n",
	    	   errorLogFile, errno);
	    return;
	}
	    		    

	/*
	 *  position read pointer to the first full line after the trim
	 *  point...
	 */
	 	 
	if ( (status = lseek(f2, deleteBytes, SEEK_SET)) < 0 ) {
	    Debug("TrimErrorLog(): Cannot lseek() in file %s, error number = %d\n",
	    	   errorLogFile, errno);
	    return;
	}

	n = read(f2, buf, BUFSIZ);

	if ( (p = strchr(buf,'\n')) != NULL ) {
	    p++; 
	    n -= p - buf;
	    deleteBytes += p - buf;
	}	    
	else {
	    p = buf;
	}
	

	/*
	 *  shift bytes to be saved to the beginning of the file...
	 */
	 
	write (f1, p, n);
	
	while ( (n = read(f2, buf, BUFSIZ)) > 0 )
	    write(f1, buf, n);

	/*
	 *  truncate file to new length and close file pointers...
	 */
	 
	truncate(errorLogFile, statb.st_size - deleteBytes);
	close(f1);
	close(f2);
    }

}



/****************************************************************************
 *
 *  LogInfo
 *
 *  Write an information message to the log file
 *
 ****************************************************************************/

void 
#ifdef _NO_PROTO
/*VARARGS1*/
LogInfo( fmt, va_alist )
        unsigned char *fmt ;
	va_dcl
#else
LogInfo( unsigned char *fmt, ...)
#endif /* _NO_PROTO */
{
    va_list  args;

    Va_start(args,fmt);

    if ( SyncErrorFile(1) ) {
	fprintf (stderr, "info (pid %d): ", getpid());
	vfprintf (stderr, (char *)fmt, args);
	fflush (stderr);
    }

    va_end(args);
}



/****************************************************************************
 *
 *  LogError
 *
 *  Write an error message to the log file.
 *
 ****************************************************************************/

void 
#ifdef _NO_PROTO
/*VARARGS1*/
LogError( fmt, va_alist )
        unsigned char *fmt ;
	va_dcl
#else
LogError( unsigned char *fmt, ...)
#endif /* _NO_PROTO */
{
    va_list  args;

    Va_start(args,fmt);

    if ( SyncErrorFile(1) ) {
	fprintf (stderr, "error (pid %d): ", getpid());
	vfprintf (stderr, (char *)fmt, args);
	fflush (stderr);
    }

    va_end(args);
}



/****************************************************************************
 *
 *  LogOutOfMem
 *
 *  Write an "out of memory" message to the log file.
 *
 ****************************************************************************/

void 
#ifdef _NO_PROTO
/*VARARGS1*/
LogOutOfMem( fmt, va_alist )
        unsigned char *fmt ;
	va_dcl
#else
LogOutOfMem( unsigned char *fmt, ...)
#endif /* _NO_PROTO */
{
    va_list  args;

    Va_start(args,fmt);

    if ( SyncErrorFile(1) ) {
	fprintf(stderr,(char *)ReadCatalog(MC_ERROR_SET,MC_NO_MEMORY,MC_DEF_NO_MEMORY));
	vfprintf (stderr, (char *)fmt, args);
	fflush (stderr);
    }

    va_end(args);
}



/***************************************************************************
 *
 *  CODE DISABLED!!! The following routines...
 *
 *			getLocalAddress()
 *			scanHostList()
 *			indirectAlias()
 *			ForEachMatchingIndirectHost()
 *			UseChooser()
 *			ForEachChooserHost()
 *
 *		     are disabled until indirect queries are supported...
 *
 ***************************************************************************/
#if 0


/****************************************************************************
 *
 *  LogPanic
 *
 *  Write a panic message to the log file.
 *
 ****************************************************************************/
void 
#ifdef _NO_PROTO
/*VARARGS1*/
LogPanic( fmt, va_alist )
        char *fmt ;
	va_dcl
#else
LogPanic( char *fmt, ...)
#endif /* _NO_PROTO */
{
    va_list  args;

    Va_start(args,fmt);

    if ( SyncErrorFile(1) ) {
	fprintf (stderr, "panic (pid %d): ", getpid());
	vfprintf (stderr, fmt, args);
	fflush (stderr);
    }

    va_end(args);
}



/****************************************************************************
 *
 *  Panic
 *
 *  Write a panic message to the console
 *
 ****************************************************************************/

int 
#ifdef _NO_PROTO
Panic( mesg )
        char *mesg ;
#else
Panic( char *mesg )
#endif /* _NO_PROTO */
{
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "panic: ", 7);
    write (i, mesg, strlen (mesg));
    exit (1);
}
#endif /* DISABLED CODE */



/****************************************************************************
 *
 *  Debug
 *
 *  Write a debug message to stdout
 *
 ****************************************************************************/

static int  DoName=True;

void 
#ifdef _NO_PROTO
/*VARARGS1*/
Debug( fmt, va_alist )
        char *fmt ;
	va_dcl
#else
Debug( char *fmt, ...)
#endif /* _NO_PROTO */
{
    va_list  args;

    Va_start(args,fmt);

    if (debugLevel > 0)
    {
	if ( strlen(DisplayName) > 0 && DoName)
	    fprintf(stdout, "(%s) ", DisplayName);

	vprintf (fmt, args);
	fflush (stdout);

	/*
	 * don't prepend the display name next time if this debug message
	 * does not contain a "new line" character...
	 */

	if ( strchr(fmt,'\n') == NULL )
	    DoName=False;
	else
	    DoName=True;
	    
    }

    va_end(args);
}
