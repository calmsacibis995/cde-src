//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_port.C /main/cde1_maint/4 1995/10/07 19:09:50 pascale $ 			 				
/* @(#)tt_port.C	1.20 93/09/07
 *
 * tt_port.cc
 *
 * Routines for centralizing non-portable constructs.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#include "tt_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#if defined(OPT_BUG_HPUX) && !defined(hpV4)
extern "C" {
	int syslog(int priority, const char *message, ...);
	int openlog(const char *ident, int logopt, int facility);
}
#endif
#include <errno.h>
#include "tt_port.h"
#include "tt_global_env.h"
#include "tt_string.h"

#if defined(OPT_BUG_SUNOS_4)
#	if defined(__GNUG__)
#		define uname NotAnAnsiCPrototypeForUname
#		include <sys/utsname.h>
#		undef uname
		extern "C" { int uname( struct utsname * ); }
#	else
#		include <sysent.h>
#		include <sys/utsname.h>
#	endif
#else
#	include <sys/utsname.h>
#endif

#if defined(OPT_SYSINFO)
#	include <sys/systeminfo.h>
#else
#	include <stdlib.h>	// atol()
#endif

#if defined(OPT_BUG_SGI) || defined(OPT_BUG_AIX)
	extern "C" {extern int getdtablesize(void);}
#endif


/*
 * Get the name of the host.
 */
_Tt_string
_tt_gethostname()
{
	/*
	 * If for some reason we can't get a host name, use "localhost".
	 * This at least has a chance of doing the right thing, say on
	 * a non-networked machine, and so is more useful than just returning
	 * null or an error and having our callers bail out.
	 */

	const char *default_hostname = "localhost";
	_Tt_string result;

#if !defined(OPT_GETHOSTNAME)
	struct utsname uts_name;
	

	if (uname(&uts_name) >= 0) {
		result = uts_name.nodename;
	} else {
		result = default_hostname;
	}
#else
	// It's not completely clear to me whether or not MAXHOSTNAMELEN is
	// supposed to have room for the null byte at the end.  Leave an
	// extra byte just to be sure.
	
	char curhostname[MAXHOSTNAMELEN+1];
	if (0==gethostname(curhostname, sizeof curhostname)) {
		result = curhostname;
	} else {
		result = default_hostname;
	}
#endif /* OPT_GETHOSTNAME */

#ifdef __osf__
// An environment variable will be created by /usr/dt/bin/Xsession if
// we have started the system without a network configured. This will
// be true during an initial system installation.

    if (getenv("DTNONETWORK"))
        result = default_hostname;
#endif

	return result;
}

/*
 * Get the number of currently possible file descriptors.
 */
int
_tt_getdtablesize(void)
{
#if !defined(OPT_GETDTABLESIZE)
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
		return (int)rl.rlim_cur;
	} else {
		return 0;
	}
#else
	return getdtablesize();
#endif /* OPT_GETDTABLESIZE */
}


static struct rlimit original_dtablesize = { 0, 0 };
/*
 * Set the number of possible file descriptors.
 *
 * Returns
 *	0	Success
 *	-1	Error (errno is set)
 */
int
_tt_setdtablesize(int limit)
{
#if defined(_AIX)
	return 1;
#else
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
		return -1;
	}
	if (original_dtablesize.rlim_cur == 0) {
		original_dtablesize = rl;
	}
	if (rl.rlim_cur < limit) {
		rl.rlim_cur = limit;
		return setrlimit(RLIMIT_NOFILE, &rl);
	}
	return 0;
#endif
}

/*
 * Maximize the number of possible file descriptors.
 *
 * Returns
 *	0	Success
 *	-1	Error (errno is set)
 */
int
_tt_zoomdtablesize(void)
{
#if defined(_AIX)
	return -1;	
#else
	rlim_t new_fd_rlim_cur;
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
		return -1;
	}
	if (original_dtablesize.rlim_cur == 0) {
		original_dtablesize = rl;
	}

	// Try to go all the way up to the hard limit.

	new_fd_rlim_cur = rl.rlim_max;

	// Bug 1244273: the RPC library code statically allocates
	// fd_sets with the default FD_SETSIZE.  If we were to
	// increase the current fd limit past FD_SETSIZE, we'd
	// have core overlays in the RPC library, which cannot
	// do anybody any good.

	if (FD_SETSIZE < new_fd_rlim_cur) {
		new_fd_rlim_cur = FD_SETSIZE;
	}

	// as a (very minor, and probably debatable) optimization,
	// don't make the setrlimit system call if the value's
	// not really changing.

	if (rl.rlim_cur == new_fd_rlim_cur) {
		return 0;
	} else {
		rl.rlim_cur = new_fd_rlim_cur;
		return setrlimit(RLIMIT_NOFILE, &rl);
	}
#endif
}

int
_tt_restoredtablesize(void)
{
#if defined(_AIX)
	return 1;
#else
	if (original_dtablesize.rlim_cur == 0) {
		return 0;
	} else {
		return setrlimit(RLIMIT_NOFILE, &original_dtablesize);
	}
#endif
}


long
_tt_gethostid(void)
{
	long _hostid;
#if defined(OPT_SYSINFO)
	{
		char		serial_num[40];

		if (sysinfo(SI_HW_SERIAL, serial_num, 40) > 0) {
			sscanf(serial_num, "%12x", &_hostid);
		}
	}
#elif defined(hpux) || defined(_AIX) || defined(__osf__)
	struct utsname uts_name;
	
	uname(&uts_name);
#	if defined(_AIX) || defined(__osf__)
		_hostid = atol(uts_name.machine);
#	else
		_hostid = atol(uts_name.idnumber);
#	endif
#else
	_hostid = gethostid();
#endif
	return _hostid;
}

/*
 * _tt_sigset sets up a signal handler in such a way that the handler
 * is *not* unregistered when the signal handler is entered.  If
 * returns 0 if an error occurs, else 1, and leaves errno set according
 * to the underlying system call (sigaction, signal, sigset.)
 */
int
_tt_sigset(
	int	sig,
	SIG_PF	handler )
{
#if defined(OPT_POSIX_SIGNAL)
/* 
 * There seems to be some controversy over the type of sa_handler in
 * C++ programs.  Everybody\'s man page seems to say it is of type
 * "void (*)()", and that\'s what Solaris does, and I think that\'s what
 * POSIX says, but both HP and IBM define it as the arguably much more
 * useful "void (*)(int)", a/k/a SIG_PF.
 * 
 * [4 Apr 95] Solaris 2.5 has switched to use void (*)(int), which
 * is nice for the long run but causes us some short-run problems
 * as we want this level of the source to compile both on
 * Solaris 2.4 and Solaris 2.5 for a while. To solve this, we use
 * the "sa_sigaction" element of the sigaction structure, which is the
 * three-argument flavor of the function pointer.  This is, strictly, 
 * a lie, but it's safe since our signal handlers never look at the
 * arguments anyway.  sa_sigaction is, fortunately, the same on all
 * Solaris versions.
 * Once the requirement to compile on Solaris 2.4 goes away, we can
 * simply remove the OPT_BUG_SUNOS_5 ifdefs here, leaving only the
 * UnixWare one.
 */
        struct sigaction act;
#if defined(OPT_BUG_SUNOS_5)	
	act.sa_sigaction = (void (*)(int, siginfo_t *, void *)) handler;
#elif defined(OPT_BUG_USL) || defined(OPT_BUG_UXP)
	act.sa_handler = (void (*)()) handler;
#else
        act.sa_handler = handler;
#endif	
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	return 0==sigaction(sig, &act, NULL);
#elif defined(OPT_BSD_SIGNAL)
	return SIG_ERR!=signal(sig, handler);
#else
	return SIG_ERR!=sigset(sig, handler);
#endif
}

/*
 * _tt_putenv - putenv()s a buffer containing "variable=value"
 * On success, returns the malloc()ed buffer that was passed to putenv().
 * On failure, returns 0.
 *
 * Note: the returned buffer becomes a storage leak after this or any
 * other form of putenv() replaces variable in the environment.
 */
char *
_tt_putenv(
	const char *variable,
	const char *value
)
{
	if (variable == 0) {
		return 0;
	}
	if (value == 0) {
		value = "";
	}
	char *buf = (char *)malloc( strlen(variable) + strlen(value) + 2 );
	if (buf == 0) {
		return 0;
	}
	sprintf( buf, "%s=%s", variable, value );
	if (putenv( buf ) != 0) {
		free(buf);
		return 0;
	} else {
		return buf;
	}
}

// prefix passed to _tt_openlog()
static _Tt_string *_tt_syslog_prefix = 0;

// prefix if _tt_openlog() has never been called
static _Tt_string *_tt_syslog_default_prefix = 0;

void
_tt_openlog(
	const char     *prefix,
	int		logopt,
	int		facility
)
{
	if (_tt_syslog_prefix == 0) {
		_tt_syslog_prefix = new _Tt_string;
	}
	//
	// Save the prefix, for when we write the prefix ourselves
	// instead of letting syslog() do it
	//
	*_tt_syslog_prefix = prefix;
	if (logopt & LOG_PID) {
		*_tt_syslog_prefix =
			_tt_syslog_prefix->cat("[").cat( getpid() ).cat("]");
	}
	*_tt_syslog_prefix = _tt_syslog_prefix->cat(": ");
	openlog( prefix, logopt, facility );
}

void
_tt_vsyslog(
	FILE	       *sink,
	int		priority,
	const char     *format,
	va_list		args
)
{
	// XXX also log to ~/.dt/errorlog
	if (_tt_global && _tt_global->silent) {
		return;
	}
	_Tt_string _format( format );
	if (   (_tt_syslog_default_prefix == 0)
	    && (_tt_syslog_prefix == 0))
	{
		//
		// Only initialize default prefix if openlog() is uncalled
		//
		_tt_syslog_default_prefix = new _Tt_string( "libtt[" );
		*_tt_syslog_default_prefix =
			_tt_syslog_default_prefix->cat( getpid() ).cat( "]: " );
	}
	if ((_tt_syslog_prefix == 0) && (_tt_syslog_default_prefix != 0)) {
		//
		// libtt cannot presume to call openlog(),
		// so we add our own prefix when openlog() is uncalled
		//
		_format = _tt_syslog_default_prefix->cat( _format );
	}
	if (sink != 0) {
		if (_tt_syslog_prefix != 0) {
			_tt_syslog_prefix->print( sink );
		}
		// XXX We do not handle %%m.
		_format = _format.replace( "%m", strerror( errno ) );
		vfprintf( sink, _format, args );
		fputc( '\n', sink );
		fflush( sink );
		return;
	}

#if defined(OPT_BUG_AIX) || defined(OPT_BUG_HPUX) || defined(OPT_BUG_USL) || defined(OPT_BUG_UXP) || defined(__osf__)
	char buf[5000];
	vsprintf( buf, _format, args );
	syslog( priority, buf );
#else
	vsyslog( priority, _format, args );
#endif
}

void
_tt_syslog(
	FILE	       *sink,
	int		priority,
	const char     *format,
	...
)
{
	va_list args;

	va_start( args, format );
	_tt_vsyslog( sink, priority, format, args );
	va_end( args );
}


//
// _tt_get_first_set_env_var - getenvs i_num_names char*s in the environment, and returns 
// the value of the first one found with a non-null value, or NULL if all have no value.
// The check is done from left to right across the supplied names.
// e.g. _tt_get_first_set_env_var (2, "FOO", "BAR") is functionally equivalent to 
// getenv("FOO") ? getenv("FOO") : getenv("BAR");
//

char* _tt_get_first_set_env_var (int i_num_names, ...) {
    
    // set up for variable number of arguments
    va_list p_var;
    va_start(p_var, i_num_names);
    
    char* pc_name_to_check;  // name of the environment var we're currently looking for
    char* pc_value = NULL;   // value of environment var we're currently looking for
    int i_index;	     // indexes through each of the names supplied
    
    // check each of the names in turn
    for (i_index = 0; i_index < i_num_names; i_index++) {
	
	// get the next name to check
	pc_name_to_check = va_arg(p_var, char*);
	
	if (!pc_name_to_check)  // ignore any null names
	    continue;
	
	// get the environment value (if any)
	pc_value = getenv(pc_name_to_check);
	
	if (pc_value) // we found one with a value
	    break;
    }
    
    // clean up var args
    va_end(p_var); 
    
    // return the value
    return pc_value;
}


// _tt_put_all_env_var - sets i_num_names char*s in the environment, to pc_val, using _tt_putenv.
// NOTE: xxx see the _tt_putenv comments about memory leakage.
// _tt_putenv are processed from left to right across the supplied names.  If any
// _tt_putenv fails, putenv'ing stops.  The number of successful putenv's is returned
// e.g. _tt_put_all_env_var (2, "HELLO", "FOO", "BAR") is functionally equivalent to 
// _tt_putenv("FOO", "HELLO") ? 0 : (_tt_putenv("BAR", "HELLO") ? 1 : 2);

int _tt_put_all_env_var (int i_num_names, const char* pc_val, ...) {
    
    // set up for variable number of arguments
    va_list p_var;
    va_start(p_var, pc_val);
    
    char* pc_name_to_set;  // name of the environment var we're currently looking for
    char* pc_buff;         // value of _tt_putenv call
    int i_index;	   // indexes through each of the names supplied
    
    // check each of the names in turn
    for (i_index = 0; i_index < i_num_names; i_index++) {
	
	// get the next name to check
	pc_name_to_set = va_arg(p_var, char*);
	
	if (!pc_name_to_set)  // ignore any null names
	    continue;
	
	// set the environment value (if any)
	pc_buff = _tt_putenv(pc_name_to_set, pc_val);
	
	if (!pc_buff) // we had a problem
	    break;
    }
    
    // clean up var args
    va_end(p_var); 
    
    // return the value
    return i_index;
}

// Do clnt_create_timed with the timeout specified in OPT_CLNT_CREATE_TIMEOUT
// if the OS has clnt_create_timed.  Otherwise, use the old long-timeout
// version of clnt_create_timed, which is bad because file scoped
// message sends may hang for a long time if dbservers or remote sessions
// are unreachable.

CLIENT *
_tt_clnt_create_timed(const char *hostname, const u_long prog,
		      const u_long vers, const char *nettype)
{
	CLIENT *result;

#if defined(OPT_HAS_CLNT_CREATE_TIMED)
	struct timeval		tv = { OPT_CLNT_CREATE_TIMEOUT, 0};

	result = clnt_create_timed(hostname, prog, vers, nettype, &tv);
#else
	result = clnt_create(hostname, prog, vers, nettype);
#endif
	return result;
}


#if ! defined(OPT_SYSTEM_MTSAFE)

#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread.h>
#include <errno.h>
#include <synch.h>

extern	int __xpg4;	/* defined in _xpg4.c; 0 if not xpg4-compiled program */

static struct sigaction _tt_sys_ignore = {
	0,
	SIG_IGN,
	0
};

static struct sigaction _tt_sys_default = {
	0,
	SIG_DFL,
	0
};

#endif	// #if undef(OPT_SYSTEM_MTSAFE)

int
_ttsystem(const char *s)
{
#if defined(OPT_SYSTEM_MTSAFE)
	return(system(s));
#else

	int	status, pid, w;
	struct sigaction ibuf, qbuf, cbuf;
	sigset_t savemask;
	struct stat buf;
	char *shpath, *shell;

	if (__xpg4 == 0) {	/* not XPG4 */
		shpath = "/bin/sh";
		shell = "sh";
	} else {
		/* XPG4 */
		shpath = "/bin/ksh";
		shell = "ksh";
	}
	if (s == NULL) {
		if (stat(shpath, &buf) != 0) {
			return (0);
		} else if (getuid() == buf.st_uid) {
			if ((buf.st_mode & 0100) == 0)
				return (0);
		} else if (getgid() == buf.st_gid) {
			if ((buf.st_mode & 0010) == 0)
				return (0);
		} else if ((buf.st_mode & 0001) == 0) {
			return (0);
		}
		return (1);
	}

	if ((pid = fork1()) == 0) {
		(void) execl(shpath, shell, (const char *)"-c", s, (char *)0);
		_exit(127);
	}

	(void) sigaction(SIGINT, &_tt_sys_ignore, &ibuf);
	(void) sigaction(SIGQUIT, &_tt_sys_ignore, &qbuf);

	sigaddset(&_tt_sys_ignore.sa_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &_tt_sys_ignore.sa_mask, &savemask);

	(void) sigaction(SIGCLD, &_tt_sys_default, &cbuf);

	do {
		w = waitpid(pid, &status, 0);
	} while (w == -1 && errno == EINTR);

	(void) sigaction(SIGINT, &ibuf, NULL);
	(void) sigaction(SIGQUIT, &qbuf, NULL);
	(void) sigaction(SIGCLD, &cbuf, NULL);
	sigprocmask(SIG_SETMASK, &savemask, NULL);

	return ((w == -1)? w: status);

#endif // defined(OPT_SYSTEM_MTSAFE)
}

