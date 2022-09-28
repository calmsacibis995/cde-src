/* $XConsortium: UErrNoBMS.c /main/cde1_maint/1 1995/07/17 18:17:21 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         UErrNoBMS.c
 **
 **   Project:	    DT
 **
 **   Description:  This file contains the set of library functions which
 **                 allow non-BMS clients to use the DT error reporting
 **                 mechanism.
 **		  
 **
 **   (c) Copyright 1987, 1988, 1989, 1990 by Hewlett-Packard Company
 **
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _NO_PROTO
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

/*
 * Account for the various macros on different systems which indicate that
 * stdarg.h has been included.	Code in this file only checks for 
 * _STDARG_INCLUDED.  If a given system defines another macro that means the
 * same thing -- then define _STDARG_INCLUDED here.
 *
 *	System		Macro Indicating stdarg.h has been included
 *     --------        ---------------------------------------------
 *	HPUX		_STDARG_INCLUDED
 *	AIX		_H_STDARG
 *	SOLARIS		_STDARG_H
 */
#ifdef _H_STDARG
#define	_STDARG_INCLUDED
#endif

#ifdef  _STDARG_H
#define	_STDARG_INCLUDED
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <X11/Intrinsic.h>
#include <Dt/DtP.h>
#include <Dt/UserMsg.h>


#ifdef _NO_PROTO

static char * ErrMnemonic() ;
static void DtIntUserMsgWithCallback() ;

#else

static char * ErrMnemonic( 
                        unsigned int errn) ;
static void DtIntUserMsgWithCallback( 
                        char *progName,
                        char *help,
                        char *format,
                        DtSeverity severity,
                        int errnoset,
                        void *client_data,
                        ...) ;

#endif /* _NO_PROTO */


/*
 * The functions in this module parallel those supported by the BMS.
 * However, these functions are for those clients which want this
 * functionality, but do not what the overhead of having to link with
 * the BMS library.
 */


static char * 
#ifdef _NO_PROTO
ErrMnemonic( errn )
        unsigned int errn ;
#else
ErrMnemonic(
        unsigned int errn )
#endif /* _NO_PROTO */
{
    char * s;
    static char buff[40];
    
    switch (errn) {
	
    /* These are all in POSIX 1003.1 and/or X/Open XPG3     */
    /* ---------------------------------------------------- */
    case EPERM		: s = "EPERM"; 	break;
    case ENOENT		: s = "ENOENT"; break;
    case ESRCH		: s = "ESRCH"; 	break;
    case EINTR		: s = "EINTR"; 	break;
    case EIO		: s = "EIO"; 	break;
    case ENXIO		: s = "ENXIO"; 	break;
    case E2BIG		: s = "E2BIG"; 	break;
    case ENOEXEC	: s = "ENOEXEC";break;
    case EBADF		: s = "EBADF"; 	break;
    case ECHILD		: s = "ECHILD"; break;
    case EAGAIN		: s = "EAGAIN"; break;
    case ENOMEM		: s = "ENOMEM"; break;
    case EACCES		: s = "EACCES"; break;
    case EFAULT		: s = "EFAULT"; break;
    case ENOTBLK	: s = "ENOTBLK";break;
    case EBUSY		: s = "EBUSY"; 	break;
    case EEXIST		: s = "EEXIST"; break;
    case EXDEV		: s = "EXDEV"; 	break;
    case ENODEV		: s = "ENODEV"; break;
    case ENOTDIR	: s = "ENOTDIR";break;
    case EISDIR		: s = "EISDIR"; break;
    case EINVAL		: s = "EINVAL"; break;
    case ENFILE		: s = "ENFILE"; break;
    case EMFILE		: s = "EMFILE"; break;
    case ENOTTY		: s = "ENOTTY"; break;
    case ETXTBSY	: s = "ETXTBSY";break;
    case EFBIG		: s = "EFBIG"; 	break;
    case ENOSPC		: s = "ENOSPC"; break;
    case ESPIPE		: s = "ESPIPE"; break;
    case EROFS		: s = "EROFS"; 	break;
    case EMLINK		: s = "EMLINK"; break;
    case EPIPE		: s = "EPIPE"; 	break;
    case ENOMSG		: s = "ENOMSG"; break;
    case EIDRM		: s = "EIDRM"; 	break;
    case EDEADLK	: s = "EDEADLK";break;
    case ENOLCK		: s = "ENOLCK"; break;
#ifndef _AIX
    case ENOTEMPTY	: s = "ENOTEMPTY"; break;
#endif
    case ENAMETOOLONG	: s = "ENAMETOOLONG"; break;
    case ENOSYS		: s = "ENOSYS"; break;
    default             : s = NULL;     break;
    }
    

    if (s)
	sprintf(buff, "%s (%d)",s,errn);
    else
	sprintf(buff, "(%d)",errn);
    
    return (buff);
}


static void 
#ifdef _NO_PROTO
DtIntUserMsgWithCallback( progName, help, format, severity, errnoset, 
                          client_data, va_alist )
        char *progName ;
        char *help ;
        char *format ;
        DtSeverity severity ;
        int errnoset ;
        void *client_data ;
	va_dcl
#else
DtIntUserMsgWithCallback(
        char *progName,
        char *help,
        char *format,
        DtSeverity severity,
        int errnoset,
        void *client_data,
        ... )
#endif /* _NO_PROTO */
{
   Boolean       abort;
   char          message[2048];
   char          msg[2048];
   va_list       args;

#ifdef _STDARG_INCLUDED
   va_start(args, client_data);
#else
   va_start(args);
#endif

   (void) vsprintf(message, format, args);	/* construct message */
   va_end(args);

   if (errnoset) {
      char *        errname;
      char *        errmsg;
      unsigned int    errn;

      if (errnoset == TRUE)		/* Use "errno" from <errno.h> ? */
	 errn = errno;			/* --- yep.			*/
      else
	 errn = errnoset;		/* No, not the magic value, use parm */

      errname = ErrMnemonic(errn);

      if (!(errmsg = strerror(errn)))
 	 errmsg = "unknown";
      
      abort = _DtUserMsgHandler(progName, help, message, severity, errn, 
                               errname, errmsg, client_data);
   } 
   else
   {
      abort = _DtUserMsgHandler(progName, help, message, severity, 0, NULL, 
                               NULL, client_data);
   }

   if (abort)
      exit(-1);
}


void 
#ifdef _NO_PROTO
_DtSimpleError( progName, severity, help, format, va_alist )
        char *progName ;
        DtSeverity severity ;
        char *help ;
        char *format ;
	va_dcl
#else
_DtSimpleError(
        char *progName,
        DtSeverity severity,
        char *help,
        char *format,
        ... )
#endif /* _NO_PROTO */
{
   va_list         args;
   char            message[2048];

#ifdef _STDARG_INCLUDED
   va_start(args, format);
#else
   va_start(args);
#endif

   (void) vsprintf(message, format, args);	/* construct message */
   va_end(args);

   DtIntUserMsgWithCallback(progName, help, "%s", severity, FALSE, NULL, 
                            message, NULL);
}

void 
#ifdef _NO_PROTO
_DtSimpleErrnoError( progName, severity, help, format, va_alist )
        char *progName ;
        DtSeverity severity ;
        char *help ;
        char *format ;
	va_dcl
#else
_DtSimpleErrnoError(
        char *progName,
        DtSeverity severity,
        char *help,
        char *format,
        ... )
#endif /* _NO_PROTO */
{
   va_list         args;
   char            message[2048];

#ifdef _STDARG_INCLUDED
   va_start(args, format);
#else
   va_start(args);
#endif

   (void) vsprintf(message, format, args);	/* construct message */
   va_end(args);

   DtIntUserMsgWithCallback(progName, help, "%s", severity, TRUE, NULL, 
                            message, NULL);
}
