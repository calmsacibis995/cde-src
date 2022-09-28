/*
 * File:         connect.h $XConsortium: connect.h /main/cde1_maint/1 1995/07/14 20:13:28 drk $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#ifndef _connect_h
#define _connect_h

#if defined(__cplusplus) && defined(__c_callable)
extern "C" {
#endif

int XeParseFileString 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString line, XeString *host_addr, XeString *path_addr);
#else
   ();
#endif
#if defined(__cplusplus) && defined(__c_callable)
}
#endif
   
/*
	Take the filespec as a line and return a path the host and path
	components as separate fields (finds the ':' and replaces it with
        NULL).  THIS FUNCTION MUNGES THE ORIGINAL LINE.
*/
     
extern XeString XeFindHost 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString host_spec);
#else
   ();
#endif

extern XeString XeFindShortHost 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString host_spec);
#else
   ();
#endif
/*
	Returns a host name corresponding to host_spec. 

	In all cases a NEW STRING, OWNED BY THE
	CALLER, is returned.  XeFindHost returns the full host specification
	including domain if there is one.  XeFindShortHost does not
	include the domain.  These functions can be used to convert between
	domain and simple names when needed.  The domain names should be
	used always internally and the simple names only used for display.
*/

extern int Xegethostname 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString hostname, unsigned int size);
#else
   ();
#endif

#if defined(__cplusplus) && defined(__c_callable)
extern "C" {
#else
extern 
#endif   
int Xegetshorthostname 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString hostname, unsigned int size);
#else
   ();
#endif
#if defined(__cplusplus) && defined(__c_callable)
}
#endif
   
/*
	Identical to the libc function gethostname, except that Xegethostname
	returns a full domain qualified name and Xegetshorthostname returns
	a simple name.  These functions are necessary because the system
	hostname may or may not contain a domain name and the internal
	representation should always be canonical form (domain qualified).
	Use Xegethostname except when the name is being used for display
	purposes only.  Storage allocation is identical to gethostname
	(it copies into the caller's buffer).
*/

#if defined(__cplusplus) && defined(__c_callable)
extern "C" {
#else
extern 
#endif   
Boolean XeIsLocalHostP
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString hostname);
#else
   ();
#endif
#if defined(__cplusplus) && defined(__c_callable)
}
#endif
   
/*
	Returns true if the hostname passed in identifies the host on which
	this function is executed.  This is needed in order to handle all
	combinations of simple and domain-qualified names for either the
	hostname passed in or the one defined on the local host.  Be sure
	to include Xe.h or provide a local declaration for this function
	as Boolean is not the same length as int and it won't function
	correctly without the declaration.
*/
#if defined(__cplusplus) && defined(__c_callable)
extern "C" {
#else
extern
#endif   
Boolean XeIsSameHostP
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString host1, XeString host2);
#else
   ();
#endif
#if defined(__cplusplus) && defined(__c_callable)
}
#endif
   
/*
	Returns true if host1 and host2 specify the same host.  This is
	needed, because either or both hosts may be specified with or
	without domain qualifiers, and the correct result must be
	obtained.  This function canonicalizes both and compares them
	only in canonical form.
*/

extern Boolean XeIsSameHostP 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString host1, XeString host2);
#else
   ();
#endif
/*
	Returns true if host1 and host2 specify the same host.  This is
	needed, because either or both hosts may be specified with or
	without domain qualifiers, and the correct result must be
	obtained.  This function canonicalizes both and compares them
	only in canonical form.
*/

XeString XeCreateContextString 
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
   (XeString host, XeString directory, XeString file);
#else
   ();
#endif
/* 
	XeCreateContextString changes the given context into a
	label which is exactly the same as the one shown in a
	fileview modeline.  A NEW STRING, OWNED BY THE CALLER,
	is returned.
*/

#define XeEliminateDots(path) (XeString)pathcollapse(path, path, FALSE)   
/*
	Removes /./'s and /../ 's from path.  THIS ROUTINE OVERWRITES
	THE path IT WAS PASSED.  If there are too many ..'s in path,
	NULL is returned, so you better keep a pointer to path if you hope
	to reclaim it.  Does not handle host:/path, shell variables or other
	exotic animals. 
*/	

int Xechdir
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)   
	   (const char *path);
#else
	   ();
#endif
/*
	performs a chdir and caches the new directory in $PWD so that
	Xegetcwd() can get the current directory without slow stat calls
*/

char *Xegetcwd
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
		(char *buf,
		 int size);
#else
		();
#endif
/*
	version of getcwd() which uses cached $PWD (or $PWD from shell)
	if available and calls getcwd only when PWD not set
*/

/* DON'T ADD STUFF AFTER THIS #endif */
#endif /* _connect_h */

