/* $XConsortium: svcmain.c /main/cde1_maint/7 1995/10/10 13:36:36 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)svcmain.c	1.19 96/09/27 Sun Microsystems, Inc."

/* #define TIMING */

#ifdef MT
#define MAXTHREADS 32
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#ifdef SVR4
#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif /* _NETINET_IN_H */
#endif

#include <sys/types.h>
#include <sys/statvfs.h>
#include <syslog.h>


#if defined(SunOS) || defined(USL) || defined(__uxp__)
#include <netconfig.h>
#include <netdir.h>
#include <sys/stropts.h>
#include <tiuser.h>
#endif /* SunOS || USL || __uxp__ */
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rpc/rpc.h>
#include <sys/file.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#ifdef MT
#include <pthread.h>
#endif

#include "rpcextras.h"
#include "log.h"
#include "cmscalendar.h"
#include "repeat.h"
#include "lutil.h"
#include "cmsdata.h"
#include "utility.h"

#ifndef	S_IRWXU
#define	S_IRWXU		(S_IRUSR|S_IWUSR|S_IXUSR)
#endif
#ifndef	S_IRWXG
#define	S_IRWXG		(S_IRGRP|S_IWGRP|S_IXGRP)
#endif
#ifndef	S_IRWXO
#define	S_IRWXO		(S_IROTH|S_IWOTH|S_IXOTH)
#endif

#define S_MASK  (S_INPUT|S_HIPRI|S_ERROR|S_HANGUP)

extern int	standalone; /* 0 if invaked by inetd, 1 otherwise */

#ifdef MT
extern void *garbage_collect(void *);
#else /* MT */
extern void garbage_collect();
#endif /* MT */

/* global variables */
#ifdef DEBUG
boolean_t	debug = _B_TRUE;
#else
boolean_t	debug = _B_FALSE;
#endif /* DEBUG */

char	  *pgname;
int	  daemon_gid, daemon_uid;
/* Signal mask for all threads.  Signals are handled by handler thread. */
#ifdef MT
sigset_t  signals;
int       first_garbage_collection_time;
#endif /* MT */

/*
 * By default strict_csa_model is set to false, i.e. the server
 * will simulate OpenWindows behavior and allow users who have only
 * CSA_VIEW_PUBLIC_ENTRIES access to see the time of CSA_CLASS_CONFIDENTIAL
 * entries (actually we only change the content of the summary attribute
 * to "Appointment" for events, "To Do" for todos, "Memo" for memos and
 * "Calendar Entry" for other entry types.
 * The -c command line optiion specifies that we follow csa access model
 * and only allow users with the corresponding access to view the specific
 * class of entries.
 */
boolean_t	strict_csa_model = _B_FALSE;

static int received_sighup = 0;		/* 1 means we got SIGHUP */
static int rpc_in_process = 0;		/* 1 means processing client request */
static int garbage_collection_time = 240; /* in min; default time is 4:00am */

static void sigusr1_handler(int);

#ifdef MT
pthread_mutex_t received_sighup_lock;
pthread_mutex_t rpc_in_process_lock; 
#endif

/*
 * get garbage collection time
 * the given string should be in the format hhmm
 * where hh is 0 - 23 and mm is 00 - 59
 */
static int
_GetGtime(char *timestr)
{
	int	hour, minute, len, i;

	if (timestr == NULL)
		goto error;

	if ((len = strlen(timestr)) > 4)
		goto error;

	for (i = 0; i < len; i++) {
		if (timestr[i] < '0' || timestr[i] > '9')
			goto error;
	}

	minute = atoi(&timestr[len - 2]);
	timestr[len - 2] = NULL;
	hour = atoi(timestr);

	if (hour > 23 || minute > 59)
		goto error;

	garbage_collection_time = hour * 60 + minute;

	return (0);

error:
	_DtCmsSyslog("The garbage collection time specified is invalid.\n");
	return (-1);
}

static void
parse_args(int argc, char **argv)
{
	int	opt;

	if (pgname = strrchr (argv[0], '/'))
		pgname++;
	else
		pgname = argv[0];

	while ((opt = getopt (argc, argv, "cdsg:")) != -1)
	{
		switch (opt)
		{
		case 'c':
			strict_csa_model = _B_TRUE;
			break;
		case 'd':
			debug = _B_TRUE;
			break;
		case 'g':
			if (_GetGtime(optarg))
				goto error;
			break;
		case '?':
			goto error;
		}
	}

	if (optind == argc)
		return;

#if defined(_aix)
	/*
	 * rpc.cmsd gets started by the inetd.
	 * On AIX inetd requires that two arguments be supplied to the RPC
	 * programs as follows (from the inetd.conf man page):
	 *
	 * ServerArgs      Specifies the command line arguments that the
	 * inetd daemon should use to execute the server. The maximum number
	 * of arguments is five. The first argument specifies the name of the
	 * server used.  If the SocketType parameter is sunrpc_tcp or
	 * sunrpc_udp, * the second argument specifies the program name and
	 * the third argument specifies the version of the program. For
	 * services that the inetd daemon provides internally, this field
	 * should be empty.
	 */

	else if (optind == 1 && argc >= 3)
	{
		int i,j;
		char **argv_r;
		
		if (argc == 3)
		  return;
		  
		argv_r = (char **) malloc(argc * sizeof(char *));

		argv_r[0] = argv[0];
		for (i=optind+2, j=1; i<argc; i++,j++)
		  argv_r[j] = argv[i];
		parse_args(argc-2, argv_r);

		free((void *) argv_r);
		return;
	}
#endif
		
error:
	_DtCmsSyslog("Usage: %s [-c] [-d] [-g hhmm]\n", pgname);
	exit (-1);
}

static void
init_dir()
{
	char *dir = _DtCMS_DEFAULT_DIR;
	int create_dir;
	struct stat info;
	mode_t mode;

	if (geteuid() != 0)
	{
		_DtCmsSyslog("must be run in super-user mode!  Exited.\n");
		exit (-1);
	}

	create_dir = 0;
	if (stat(dir, &info))
	{
		/* if directory does not exist, create the directory */
		if ((errno != ENOENT) || mkdir(dir, S_IRWXU|S_IRWXG|S_IRWXO))
		{
			if (errno == ENOENT)
				_DtCmsSyslog("Cannot create %s. %s: %s\n",
						dir, "System error",
						strerror(errno));
			else
				_DtCmsSyslog("Cannot access %s. %s: %s\n",
						dir, "System error",
						strerror(errno));
			exit (-1);
		}
		create_dir = 1;
	}

	/* if dir is just created, we need to do chmod and chown.
	 * Otherwise, only do chmod and/or chown if permssion and/or
	 * ownership is wrong.
	 */
	mode = S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO;

	if (create_dir || info.st_mode != (mode | S_IFDIR)) {

		/* set directory permission to be "rwxrwsrwt" */
		if (chmod(dir, mode)) {
			_DtCmsSyslog("Permission on %s%s\n%s\n%s: %s\n",
				dir, " is wrong but cannot be corrected.",
				"This might happen if you are mounting the directory.",
				"System error", strerror(errno));
			if (create_dir)
				rmdir(dir);
			exit(-1);
		}
	}

	if (create_dir || info.st_uid!=daemon_uid || info.st_gid!=daemon_gid) {
		/* set directory ownership to: owner = 1, group = 1 */
		if (chown(dir, daemon_uid, daemon_gid)) {
			_DtCmsSyslog("Ownership on %s%s\n%s\n%s: %s\n",
				dir, " is wrong but cannot be corrected.",
				"This might happen if you are mounting the directory.",
				"System error", strerror(errno));
			if (create_dir)
				rmdir(dir);
			exit(-1);
		}
	}

	/* Change current directory, so core file can be dumped. */
	chdir (dir);
}

/*
 * send a SIGHUP signal to the rpc.cmsd that is already running
 */
static void
send_hup()
{
	FILE	*fp = NULL;
	int	fd[2];
	char	buf[BUFSIZ];
	char    cmd[BUFSIZ];
	int	pid, mypid = getpid();
	int	c_pid;
	int	status;

	char *env[] = {
                "PATH=/usr/bin:/usr/sbin:/usr/dt/bin:/usr/lib",
                "IFS=' '",
                "LD_LIBRARY_PATH=/usr/lib",
                 NULL
        };
	
	sprintf(cmd, "ps -e|grep rpc.cmsd|grep -v grep");

	pipe(fd);

	if ((c_pid = fork()) < 0)
		_DtCmsSyslog("fork failed when trying to send SIGHUP\n");
	else {
	    if (c_pid == 0) { /* The child. */
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                execle("/usr/bin/sh", "sh", "-c", cmd, (char *)0, env);
                _exit(1);
            }
            else if (c_pid > 0) { /* The parent. */
                close(fd[1]);
                fp = fdopen(fd[0], "r");
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if ((pid = atoi(buf)) != mypid) {
				if (kill(pid, SIGHUP))
				    _DtCmsSyslog("failed to send SIGHUP\n");
				if (debug)
				    fprintf(stderr, "rpc.cmsd: %s %d\n",
						"sent SIGHUP to", pid);
			}
		}
		fclose(fp);
		close(fd[0]);
		wait(&status);
	    }
	}
}

/*
 * We only allow one rpc.cmsd to run on each machine.
 */
static int
lock_it()
{
	char *dir = _DtCMS_DEFAULT_DIR;
	char	buff [MAXPATHLEN];
	int	error;
	int	fd;
#ifdef SVR4
	struct flock locker;
	locker.l_type = F_WRLCK;
	locker.l_whence = 0;
	locker.l_start = 0;
	locker.l_len = 0;
#endif /* SVR4 */

	strcpy (buff, dir);
	strcat (buff, "/.lock.");

	/* 
	 * /var/spool might be mounted.  Use .lock.hostname to
	 * prevent more than one cms running in each host.
	 */
	strcat(buff, _DtCmGetLocalHost());

	fd = open(buff, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if (fd < 0)
	{
		_DtCmsSyslog("failed to open lock file %s\n", buff);
		exit (-1);
	}

	/*
	 * Note, we have to use flock() instead of lockf() because cms process
	 * is run in each host.
	 */
#ifdef SVR4
	if (fcntl (fd, F_SETLK, &locker) != 0)
#else
	if (flock (fd, LOCK_EX|LOCK_NB) != 0)
#endif /* SVR4 */
	{
		error = errno;

		close(fd);

		if (error != EWOULDBLOCK && error != EACCES) {

			_DtCmsSyslog("failed to lock lockfile: %s\n",
					strerror(error));
			exit (-1);

		} else {
			if (debug)
				fprintf(stderr, "rpc.cmsd: %s\n",
				    "lock_it failed due to another process");
			
			/* cms has been running.... */
			return(error);
		}
	}

	return (0);
}

/*
 * reset to default configuration and
 * reread the configuration file
 */

/* Instrumentation for benchmarking. */
#ifdef TIMING
static void
_sigusr1_handler(int dummy)
#else
static void
sigusr1_handler(int dummy)
#endif /* TIMING */
{
	if (debug) fprintf(stderr, "%s: SIGUSR1 received\n", pgname);

	/* read /etc/cmsd.conf again */
	if (_DtCmsGetConfigInfo() < 0)
	  exit(-1);
}

/* Instumentation */
#ifdef TIMING
static int measure = 0;
static time_t time1, time2;
static int count = 0;
#ifdef MT
pthread_mutex_t timing_lock; 
#endif /* MT */

static void
sigusr1_handler(int dummy)
{
        int i, diff;

	#ifdef MT
	pthread_mutex_lock(&timing_lock);
        #endif /* MT */

        if (measure == 0) {
                measure = 1;
		time1 = time(0);
                count = 0;
        } else {
                measure = 0;
		time2 = time(0);
		diff = time2 - time1;
                printf("%d requests processed in %d seconds: %.2f requests/seconds\n", count, diff, (double)count/diff);
	}

	#ifdef MT
	pthread_mutex_unlock(&timing_lock);
        #endif /* MT */
}
#endif /* TIMING */

static void
program(struct svc_req *rqstp, register SVCXPRT *transp)
{
        /* Bool type return value. */
        bool_t retval;

        char *result = NULL;
        char *argument = NULL;
        program_handle ph = getph();
        struct rpcgen_table *proc;

	/* Instrumentation */
	#ifdef TIMING
	#ifdef MT
	pthread_mutex_lock(&timing_lock);
	count++;
	pthread_mutex_unlock(&timing_lock);
	#else /* MT */
	count++;
	#endif /* MT */
	#endif /* TIMING */

#ifdef MT
	/* If sigup has been received, don't start any new rpc processes. */
	pthread_mutex_lock(&received_sighup_lock);
	if (received_sighup == 1) {
	  pthread_mutex_unlock(&received_sighup_lock);
	  return;
	}
	else
	  pthread_mutex_unlock(&received_sighup_lock);
	/* set rpc_in_process so that sighup handler won't exit right away */
	/* Increment instead of set to 1 because many rpc calls may be in */
	/* process at once. */
	pthread_mutex_lock(&rpc_in_process_lock);
	rpc_in_process++;
 	pthread_mutex_unlock(&rpc_in_process_lock);
#else /* MT */
	/* If sigup has been received, don't start any new rpc processes. */
	if (received_sighup == 1)
	  return;
	/* set rpc_in_process so that sighup handler won't exit right away */
	/* Increment instead of set to 1 because many rpc calls may be in */
	/* process at once. */
	rpc_in_process++;
#endif /* MT */

        /* first do some bounds checking: */
        if (rqstp->rq_vers >= ph->nvers) {
                svcerr_noproc(transp);
                goto done;
        }
        if (ph->prog[rqstp->rq_vers].nproc == 0) {
                svcerr_noproc(transp);
                goto done;
        }
        if (rqstp->rq_proc >= ph->prog[rqstp->rq_vers].nproc) {
                svcerr_noproc(transp);
                goto done;
        }

	if (rqstp->rq_proc == NULLPROC) {
		if (debug) fprintf(stderr, "rpc.cmsd: ping\n");
		(void)svc_sendreply(transp, (xdrproc_t)xdr_void, (caddr_t)NULL);
		goto done;
	}

        /* assert - the program number, version and proc numbers are valid */
        proc = &(ph->prog[rqstp->rq_vers].vers[rqstp->rq_proc]);
        argument = (char*)calloc(proc->len_arg, sizeof(char));
	result = (char*)calloc(proc->len_res, sizeof(char));
        if (!svc_getargs(transp, proc->xdr_arg, argument)) {
                svcerr_decode(transp);
                goto done;
        }

        retval = (bool_t)(*proc->proc)(argument, result, rqstp);
        if (retval > 0 && !svc_sendreply(transp, proc->xdr_res, result)) {
                svcerr_systemerr(transp);
        }

        if (!svc_freeargs(transp, proc->xdr_arg, argument)) {
                _DtCmsSyslog("unable to free arguments\n");
                exit(1);
        }
        free(argument);

	if (!svc_freeargs(transp, proc->xdr_res, result)) {
                _DtCmsSyslog("unable to free arguments\n");
                exit(1);
        }
	free(result);

done:

#ifdef MT
	/* Decrement the number of the rpc calls in process. */
	pthread_mutex_lock(&rpc_in_process_lock);
	rpc_in_process--;
	/* Only exit if no more threads in process. */
	if (rpc_in_process == 0) {
	  pthread_mutex_unlock(&rpc_in_process_lock);
	  /* exit if we have received the SIGHUP signal */
	  pthread_mutex_lock(&received_sighup_lock);
	  if (received_sighup == 1) {
	    pthread_mutex_unlock(&received_sighup_lock);
	    if (debug)
	      fprintf(stderr, "rpc.cmsd: received SIGHUP, %s",
		      "exiting after finished processing\n");
	    exit(0);
	  }
	  else
	    pthread_mutex_unlock(&received_sighup_lock);
	}
	else
	  pthread_mutex_unlock(&rpc_in_process_lock);
#else /* MT */
	/* Decrement the number of the rpc calls in process. */
	rpc_in_process--;
	/* Only exit if no more threads in process. */
	if (rpc_in_process == 0) {
	  /* exit if we have received the SIGHUP signal */
	  if (received_sighup == 1) {
	    if (debug)
	      fprintf(stderr, "rpc.cmsd: received SIGHUP, %s",
		      "exiting after finished processing\n");
	    exit(0);
	  }
	}
#endif /* MT */

	/* The thread will now exit. */
}

/*
 * Signal handler for SIGHUP.
 * If we are in the middle of processing a client request,
 * finish processing before we exit.
 */
static void
sighup_handler(int dummy)
{
	if (debug)
		fprintf(stderr, "rpc.cmsd: sighup received\n");

#ifdef MT
	pthread_mutex_lock(&rpc_in_process_lock);
	if (rpc_in_process == 0) {
	  pthread_mutex_unlock(&rpc_in_process_lock);
	  if (debug)
	    fprintf(stderr, "rpc.cmsd: exit from sighup_handler\n");
	  exit(0);
	} else {
	  pthread_mutex_unlock(&rpc_in_process_lock);
	  if (debug)
	    fprintf(stderr, "rpc.cmsd: set received_sighup to 1\n");
	  pthread_mutex_lock(&received_sighup_lock);
	  received_sighup = 1;
	  pthread_mutex_unlock(&received_sighup_lock);
	}
#else /* MT */
	if (rpc_in_process == 0) {
	  if (debug)
	    fprintf(stderr, "rpc.cmsd: exit from sighup_handler\n");
	  exit(0);
	} else {
	  if (debug)
	    fprintf(stderr, "rpc.cmsd: set received_sighup to 1\n");
	  received_sighup = 1;
	}
#endif /* MT */
}

/* 
 * Dummy Handler for Signals Handled by Signal Handler Thread
 */
void
dummy_handler(int dummy)
{ }

/* 
 * Handler for all Signals
 * Call other Handlers from Here
 */
#ifdef MT
void *
signal_handler(void *arg)
{
  int sig, err;

  if (debug)
    printf("Signal Handler Initialized\n");

  for (;;) {
    if (debug)
      printf("Waiting for Signal\n");
    err = sigwait(&signals, &sig);
    if (debug)
      printf("Got signal %d at time %d\n", sig, time(0));
    if (err)
      abort();
    switch (sig) {
    case SIGHUP:
      sighup_handler(0);
      break;
    case SIGUSR1:
      sigusr1_handler(0);
      break;
    case SIGUSR2:
      debug_switch();
      break;
    default:
      abort();
    }
  }
}
#endif /* MT */

/*
 * garbage_collection_time (in min) is the time to do garbage collection
 * each day
 * This routine returns the difference between the first garbage collection
 * time and now so that the calling routine can set the alarm.
 */
static int
_GetFirstGarbageCollectionTime()
{
	int n=0, midnight=0, gtime=0;

	n = time(0);

	/* try today first */
	midnight = next_ndays(n, 0);
	gtime = next_nmins(midnight, garbage_collection_time);

	if (gtime < n) {
		/* the first garbage collection will be done tomorrow */
		midnight = next_ndays(n, 1);
		gtime = next_nmins(midnight, garbage_collection_time);
	}

	return (gtime - n);
}

static void
init_alarm()
{
	int next;
	extern void debug_switch();

#ifdef MT
	struct sigaction act;

	act.sa_handler = dummy_handler;
	act.sa_mask = signals;
	act.sa_flags = 0;;
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
#else /* MT */
#ifdef SVR4
	sigset(SIGUSR1, sigusr1_handler);
	sigset(SIGALRM, garbage_collect);
	sigset(SIGUSR2, debug_switch);
#else /* SVR4 */
	signal(SIGUSR1, sigusr1_handler);
	signal(SIGALRM, garbage_collect);
	signal(SIGUSR2, debug_switch);
#endif /* SVR4 */
	next = _GetFirstGarbageCollectionTime();
	if (debug)
	  printf("Initial alarm set for %d.\n\n", time(0)+next);
	alarm((unsigned) next);
#endif /* MT */
}


static int
check_convert_lock()
{
        char *dir = _DtCMS_DEFAULT_DIR;
        char    buff [MAXPATHLEN];
        int     error;
        int     fd;
#ifdef SVR4
        struct flock locker;
        locker.l_type = F_WRLCK;
        locker.l_whence = 0;
        locker.l_start = 0;
        locker.l_len = 0;
#endif /* SVR4 */

        strcpy (buff, dir);
        strcat (buff, "/.lock.convert.");

        strcat(buff, _DtCmGetLocalHost());

        fd = open(buff, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
        if (fd < 0)
        {
                _DtCmsSyslog("failed to open lock file %s\n", buff);
                exit (-1);
        }

#ifdef SVR4
        if (fcntl (fd, F_SETLK, &locker) != 0)
#else
        if (flock (fd, LOCK_EX|LOCK_NB) != 0)
#endif /* SVR4 */
        {
           close(fd);
           exit( -1);
        }
        else 
        {
           char    buff[MAXPATHLEN];
           sprintf(buff, "/bin/rm -f %s/.lock.convert.", _DtCMS_DEFAULT_DIR);
           strcat(buff, _DtCmGetLocalHost());
           system(buff);
           return (0);
        }
}



main(int argc, char **argv)
{
        u_long version;
        program_handle ph = newph();
	struct passwd *pw;
	struct group *gr;
	struct rlimit rl;
	struct sockaddr_in saddr;
	int asize = sizeof (saddr);
	SVCXPRT *tcp_transp = (SVCXPRT *)-1;
	SVCXPRT *udp_transp = (SVCXPRT *)-1;
	int fd, error;
	int mode;

        char directory[64], *path;
        struct statvfs file_info;
        struct statvfs *buf;
        int retcode;
        char *msg_ptr;


#ifdef MT
	int getReturn;
	int maxThreads = MAXTHREADS;
	pthread_t signal_thread, gc_thread;
	pthread_attr_t attr;
	extern void *signal_handler(void *);
#endif /* MT */

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	struct netconfig *nconf_udp;
	struct netconfig *nconf_tcp;
	struct t_info info;
#if !defined(USL) || (defined(USL) && (OSMAJORVERSION > 1))
	char mname[FMNAMESZ+1];
#endif
#endif /* SunOS || USL */

#ifdef __osf__
      svc_init();
#endif

#ifdef MT
      /* Put the server into MT mode. */
      mode = RPC_SVC_MT_AUTO;
      if (rpc_control(RPC_SVC_MTMODE_SET, &mode))
	if (debug)
	  printf("Putting the server in MT mode (1).\n");
	else 
	  if (debug)
	    printf("Server failed to go into MT mode.\n");
      if (rpc_control(RPC_SVC_MTMODE_GET, &getReturn))
	if (debug)
	  printf("Got mode: %d\n", getReturn);
	else
	  if (debug)
	    printf("RPC_SVC_MTMODE_GET failed\n");
      if (rpc_control(RPC_SVC_THRMAX_SET, &maxThreads))
	if (debug)
	  printf("Setting the maximum number of threads.\n");
	else
	  if (debug)
	    printf("Server failed to increase maximum number of threads.\n");
      if (rpc_control(RPC_SVC_THRMAX_GET, &getReturn))
	if (debug)
	  printf("Maximum threads: %d\n", getReturn);
	else
	  if (debug)
	    printf("RPC_SVC_THRMAX_GET failed\n");
#else /* MT */
      mode = RPC_SVC_MT_NONE;
      rpc_control(RPC_SVC_MTMODE_SET, &mode);
#endif /* MT */
	

	pw = (struct passwd *)getpwnam("daemon");
	gr = (struct group *)getgrnam("daemon");
	if (pw != NULL) 
		daemon_uid = pw->pw_uid;
	else
		daemon_uid = 1;
	if (gr != NULL)
		daemon_gid = gr->gr_gid;
	else 
		daemon_gid = 1;

	/* check to see if we are started by inetd */
	if (getsockname(0, (struct sockaddr *)&saddr, &asize) == 0) {

		standalone = 0;

#if defined(SunOS) || defined(USL) || defined(__uxp__)
#if !defined(USL) || (defined(USL) && (OSMAJORVERSION > 1))
		/* we need a TLI endpoint rather than a socket */
		if (ioctl(0, I_LOOK, mname) != 0) {
			_DtCmsSyslog("ioctl failed to get module name\n");
			exit(1);
		}
		if (strcmp(mname, "sockmod") == 0) {
			/* Change socket fd to TLI fd */
			if (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, "timod")) {
				_DtCmsSyslog("ioctl I_POP/I_PUSH failed\n");
				exit(1);
			}
		} else if (strcmp(mname, "timod") != 0) {
			_DtCmsSyslog("fd 0 is not timod\n");
			exit(1);
		}
#else  /* !USL || (USL && OSMAJORVERSION > 1) */
		if (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, "timod")) {
			_DtCmsSyslog("ioctl I_POP/I_PUSH failed\n");
			exit(1);
		}
#endif /* !USL || (USL && OSMAJORVERSION > 1) */

	} else if (t_getinfo(0, &info) == 0) {
		standalone = 0;

#endif /* SunOS || USL */

	} else
		standalone = 1;

	/*
	 * if it is started by inetd, make stderr to be
	 * output to console.
	 */
	if (!standalone) {
		if ((fd = open ("/dev/console", O_WRONLY)) >= 0) {
			if (fd != 2) {
				dup2(fd, 2);
				close (fd);
			}
		}
	}

	parse_args(argc, argv);

	/* Set up private directory and switch euid/egid to daemon. */
	umask (S_IWOTH);
	init_dir();

        /* 
        **  Complain if /var/spool/calender is NFS mounted
        */
        buf = &file_info;
        path =  directory;
        sprintf(path,"%s","/var/spool/calendar");              /* ufs ? */ 
        retcode =  statvfs(path, buf); 
        if (!strcmp(buf->f_basetype,"nfs")) {
           if (debug) printf("rpc.cmsd: NFS mounted /var/spool/calendar Not supported \n");
           msg_ptr = (char *)malloc(256);
           sprintf(msg_ptr,"rpc.cmsd: /var/spool/calendar NFS mount Not supported.");
           syslog(LOG_ALERT, msg_ptr);
           free(msg_ptr);
           /* exit(0); */
        }
                 

	/* Don't allow multiple cms processes running in the same host. */
	if ((error = lock_it()) != 0 && !standalone) {
		/* we are invoked by inetd but another rpc.cmsd
		 * is alreay running, so send SIGHUP to it
		 */

		send_hup();

		/* try to lock it again */
		if (lock_it() != 0) {
			_DtCmsSyslog("rpc.cmsd is still running\n");
			exit(0);
		}

	} else if (error != 0) {
		_DtCmsSyslog("rpc.cmsd is already running.\n");
		exit(0);
	}

#ifdef MT
	/* Find first garbage collection time. */
	first_garbage_collection_time = _GetFirstGarbageCollectionTime();
	if (debug)
	  printf("The first garbage collection will be at %d.\n", first_garbage_collection_time);

	/* Set Signal Mask for All Threads */
	sigemptyset(&signals);
	sigaddset(&signals, SIGHUP);
	sigaddset(&signals, SIGUSR1);
	sigaddset(&signals, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &signals, NULL);

	/* Create Bound Thread to Handle Signals */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&signal_thread, &attr, signal_handler, NULL);

	/* Create Unbound Thread to do Garbage Collection */
	pthread_create(&gc_thread, NULL, garbage_collect, NULL);
#else /* MT */
	/* use signal because we only need it once */
	signal(SIGHUP, sighup_handler);
#endif /* MT */

	/* check /etc/cmsd.conf */
	if (_DtCmsGetConfigInfo() < 0)
		exit(-1);

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	/* raise the soft limit of number of file descriptor */
	/* this is to prevent the backend from running out of open file des */
	getrlimit(RLIMIT_NOFILE, &rl);
	rl.rlim_cur = rl.rlim_max;
	setrlimit(RLIMIT_NOFILE, &rl);
#endif

#if defined(SunOS) || defined(USL) || defined(__uxp__)
	nconf_udp = getnetconfigent("udp");
	nconf_tcp = getnetconfigent("tcp");

	for (version = 0; version < ph->nvers; version++) {
		/* don't register unsupported versions: */
		if (ph->prog[version].nproc == 0) continue;

		if (standalone)
			rpcb_unset(ph->program_num, version, NULL);

		/* brought up by inetd, use fd 0 which must be a TLI fd */
		if (udp_transp == (SVCXPRT *)-1) {
			udp_transp = svc_tli_create(standalone ? RPC_ANYFD : 0,
				nconf_udp, (struct t_bind*) NULL, 0, 0); 

			if (udp_transp == NULL) {
				_DtCmsSyslog("svc_tli_create(udp) failed\n");
				t_error("rtable_main.c: svc_tli_create(udp)");
				exit(2);
			}
		}

		if (svc_reg(udp_transp, ph->program_num, version, program,
				standalone ? nconf_udp : NULL) == 0) {
			_DtCmsSyslog("svc_reg(udp) failed\n");
			t_error("rtable_main.c: svc_reg(udp)");
			exit(3);
		}

		/* Set up tcp for calls that potentially return */
		/* large amount of data.  This transport is not */
		/* registered with inetd so need to register it */
		/* with rpcbind ourselves.			*/

		rpcb_unset(ph->program_num, version, nconf_tcp);

		if (tcp_transp == (SVCXPRT *)-1) {
			tcp_transp = svc_tli_create(RPC_ANYFD, nconf_tcp,
					(struct t_bind *)NULL, 0, 0);

			if (tcp_transp == NULL) {
				_DtCmsSyslog("svc_til_create(tcp) failed\n");
				t_error("rtable_main.c: svc_til_create(tcp)");
				exit(2);
			}
		}

		if (svc_reg(tcp_transp, ph->program_num, version, program,
				nconf_tcp) == 0) {
			_DtCmsSyslog("svc_reg(tcp) failed\n");
			t_error("rtable_main.c: svc_reg(tcp)");
			exit(3);
		}
	}/*for*/

	if (nconf_udp)
		freenetconfigent(nconf_udp);
	if (nconf_tcp)
		freenetconfigent(nconf_tcp);

#else

	for (version = 0; version < ph->nvers; version++) {
		/* don't register unsupported versions: */
		if (ph->prog[version].nproc == 0) continue;

#ifndef HPUX
		if (standalone)
#endif
			(void) pmap_unset(ph->program_num, version);

		if (udp_transp == (SVCXPRT *)-1) {
			udp_transp = svcudp_create(standalone ? RPC_ANYSOCK : 0
#if defined(_AIX) || defined(hpV4) || defined(__osf__)
					);
#else
					,0,0);
#endif
			if (udp_transp == NULL) {
				fprintf(stderr, "cannot create udp service.\n");
				exit(1);
                	}
		}

#ifndef HPUX
		if (!svc_register(udp_transp, ph->program_num, version, program,
				standalone ? IPPROTO_UDP : 0)) {
#else
		if (!svc_register(udp_transp, ph->program_num, version, program,
				IPPROTO_UDP)) {
#endif
			fprintf(stderr, "unable to register");
			exit(1);
		}

		/* Set up tcp for calls that potentially return */
		/* large amount of data.  This transport is not */
		/* registered with inetd so need to register it */
		/* with rpcbind ourselves.			*/

		if (tcp_transp == (SVCXPRT *)-1) {
			tcp_transp = svctcp_create(RPC_ANYSOCK, 0, 0);
			if (tcp_transp == NULL) {
				(void)fprintf(stderr,
				"rtable_main.c: cannot create tcp service.\n");
				exit(1);
			}
		}

		if (!svc_register(tcp_transp, ph->program_num, version, program,
				IPPROTO_TCP)) {
			(void)fprintf(stderr, "rtable_main.c: unable to register(tcp)");
			exit(1);
		}
	}

#endif /* SunOS || USL */

#ifndef AIX
#ifdef HPUX
	setgid (daemon_gid);
	setuid (daemon_uid);
#else
	setegid (daemon_gid);
	seteuid (daemon_uid);
#endif /* HPUX */
#endif /* AIX */

	init_time();
	init_alarm();

	_DtCm_init_hash();

        check_convert_lock();

	svc_run();

	return(1);
}

