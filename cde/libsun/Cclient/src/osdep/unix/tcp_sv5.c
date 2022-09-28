/*
 * @(#)tcp_sv5.c	1.24 97/06/13
 *
 * Program:	UNIX TCP/IP routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	1  August 1988
 * Last Edited:	21 October 1996
 *
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made available
 * "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This code is based on the above copyright and is a solaris extension
 * written by 
 *       William Yeager of SunSoft, Inc.
 *       Copyright (c) SunSoft, Inc. June 7, 1995
 * 10 juin 1996   DO NOT overwrite passed hostname in tcp_open.
 *                This is because varying name services return
 *                Different names. Some are NOT fully qualified.
 * [09-27-96]  gethostbyname_r implemented to ensure MT-Safe.
 * [10-01-96]  gethostbyaddr_r implemented to ensure MT-Safe.
 * [10-02-96]  getservbyname_r implemented to ensure MT-Safe.
 * [10-02-96]  getprotobyname_r implemented to ensure MT-Safe.
 * [10-07-96]  fixed the free memory read problem in tcp_open.
 * [10-21-96]  Added a new routine for MT-safe purpose: tcp_init_globals.
 * [10-21-96]  Passing stream on tcp_parameters. 
 *		
 *		     Globals put onto the stream:
 *			tcptimeout
 *			tcptimeout_read
 *			tcptimeout_write
 * [11-08-96]  stream passed in mm_log calls and tcp_open. 
 *
 * [12-23-96] Remove tcp_globals definition from mail.h.
 *	      Remove tcp_globals macros.
 * [01-01-97] Add a new routine tcp_global_free.
 */
const char	*	connectionTimeout ="210 connection timed out";

#include "UnixDefs.h"
#include "os_sv5.h"
#include "tcp.h"

/* TCP timeout handler routine */
/* TCP timeouts, in seconds */

/*
 * TCP/IP manipulate parameters
 * Accepts: function code
 *	    function-dependent value
 * Returns: function-dependent return value
 */
void tcp_global_init(MAILSTREAM * stream);
int process_urgent_data(TCPSTREAM * stream);
int received_tcp_urgent_reply(TCPSTREAM * stream);

void *
tcp_parameters(MAILSTREAM * stream, long function, void * value)
{
  TCP_GLOBALS *	tg = (TCP_GLOBALS *)stream->tcp_globals;

  switch ((int) function) {

  case SET_TIMEOUT:
    tg->tcptimeout = (tcptimeout_t) value;
    break;

  case GET_TIMEOUT:
    value = (void *) tg->tcptimeout;
    break;

  case SET_READTIMEOUT:
    tg->tcptimeout_read = (long) value;
    break;

  case GET_READTIMEOUT:
    value = (void *) tg->tcptimeout_read;
    break;

  case SET_WRITETIMEOUT:
    tg->tcptimeout_write = (long) value;
    break;

  case GET_WRITETIMEOUT:
    value = (void *) tg->tcptimeout_write;
    break;

  default:
    value = NIL;		/* error case */
    break;
  }

  return(value);
}

/*
 * TCP/IP open
 * Accepts: host name
 *	    contact port number
 * Returns: TCP/IP stream if success else NULL
 *
 * NOTE change: passes char * service
 */
TCPSTREAM *
tcp_open(char * host, char * service, long port, MAILSTREAM * mstream)
{
  TCPSTREAM 	*	stream = NULL;
  int 			i;
  int			sock;
  char 		*	s;
  struct sockaddr_in 	sin;
  struct hostent *	host_name;
  struct hostent 	host_res1;
  struct hostent	host_res2; 

  /* Just allocate them statically -- a lot easier! */
  char 			host_buf1[MAXHOSTNAMELEN + 1]; 
  char 			host_buf2[MAXHOSTNAMELEN + 1]; 
  char 			serv_buf[ NSS_BUFLEN_SERVICES +1]; 
  char 			proto_buf[NSS_BUFLEN_PROTOCOLS + 1 ];
  int 			h_errno;
  char 			hostname[MAILTMPLEN];
  char 			tmp[MAILTMPLEN];
  struct servent *	sv;
  struct servent	serv_res;
  struct servent *	serv_ret;
  struct protoent*	pt;
  struct protoent	proto_res;
  struct protoent*	proto_ret; 

  serv_ret = getservbyname_r(service, "tcp", &serv_res, serv_buf,
			     NSS_BUFLEN_SERVICES+1);
  sv = service ? serv_ret: NULL;

  pt = getprotobyname_r("ip", &proto_res, proto_buf,
			NSS_BUFLEN_PROTOCOLS + 1);

  if (s = strchr(host, ':')) {	/* port number specified? */
    *s++ = '\0';		/* yes, tie off port */
    port = strtol(s, &s, 10);	/* parse port */
    if (s && *s) {
      sprintf(tmp, "Junk after port number: %.80s",s);
      mm_log(tmp, ERROR,mstream);
      return NULL;
    }
    sin.sin_port = htons(port);
  } else {		/* copy port number in network format */
    sin.sin_port = sv ? sv->s_port : htons(port);
  }

  /*
   * The domain literal form is used (rather than simply the dotted decimal
   * as with other Unix programs) because it has to be a valid "host name"
   * in mailsystem terminology. 
   */

  /* look like domain literal? */
  if (host[0] == '[' && host[(strlen(host))-1] == ']') {
    strcpy(hostname, host+1);	/* yes, copy number part */
    hostname[(strlen(hostname))-1] = '\0';
    if ((sin.sin_addr.s_addr = inet_addr(hostname)) != -1) {
      sin.sin_family = AF_INET;	/* family is always Internet */
      strcpy(hostname,host);	/* hostname is user's argument */
    } else {
      sprintf(tmp, "Bad format domain-literal: %.80s", host);
      mm_log(tmp,ERROR,mstream);
      return(NULL);
    }
  } else {			
    strcpy(hostname, host);	/* in case host is in write-protected memory */

    /* requires lowercase! */
    if ((host_name = gethostbyname_r(lcase(hostname),
				     &host_res1,
				     host_buf1,
				     MAXHOSTNAMELEN + 1,
				     &h_errno))) {

      /* copy address type */
      sin.sin_family = host_name->h_addrtype;

      /* copy host addresses */
      memcpy (&sin.sin_addr, host_name->h_addr, host_name->h_length);
    } else {
      sprintf (tmp,"No such host as %.80s",host);
      mm_log (tmp,ERROR,mstream);
      return(NULL);
    }
  }

  /* get a TCP stream */
  if ((sock = socket(sin.sin_family, SOCK_STREAM, pt ? pt->p_proto : 0)) < 0) {
    sprintf(tmp, "Unable to create TCP socket: %s", strerror(errno));
    mm_log(tmp, ERROR, mstream);
    return(NULL);
  }

  /* open connection */
  while ((i = connect(sock, (struct sockaddr *)&sin, sizeof (sin))) < 0 &&
	 errno == EINTR);

  if ((i < 0) && (errno != EISCONN)) {
    sprintf(tmp, "Can't connect to %.80s,%d: %s", hostname, port,
	    strerror (errno));
    mm_log(tmp, ERROR, mstream);
    return(NULL);
  }

  /* create TCP/IP stream */
  stream = (TCPSTREAM *) fs_get(sizeof (TCPSTREAM));

  /* copy passed hostname */
  stream->host = cpystr(hostname);

  /* get local name */
  gethostname(tmp, MAILTMPLEN-1);

  stream->localhost = cpystr((host_name=gethostbyname_r(tmp,
							&host_res2,
							host_buf2,
							MAXHOSTNAMELEN + 1,
							&h_errno ) )
			     ? host_name->h_name : tmp);

  stream->port = port;		/* port number */

  /* init sockets */
  stream->tcpsi = stream->tcpso = sock;
  stream->ictr = 0;		/* init input counter */
  stream->pid = 0;
  stream->mailstr = 0;		/* NO mail stream yet */
  stream->io_bytect = 0;	/* clear data counter */
  stream->disallow_callback = 0;/* initially allowed */
  stream->os_send_urgent = 0;   /* clear the urgent ctr */
  stream->max_wait_on = T;      /* On by default */

  return(stream);		/* return success */
}

/*
 * TCP/IP authenticated open
 * Accepts: host name
 *	    service name
 * Returns: TCP/IP stream if success else NIL
 */
TCPSTREAM *
tcp_aopen(char * host, char * service)
{
  TCPSTREAM 	*	stream = NIL;
  char 			hostname[MAILTMPLEN];
  int 			i;
  int 			pipei[2];
  int			pipeo[2];
  struct hostent *	host_name;
  struct hostent	host_res1;
  struct hostent	host_res2;

  char			host_buf1[MAXHOSTNAMELEN + 1];
  char			host_buf2[MAXHOSTNAMELEN + 1];
  int 			h_errno;

  /*
   * The domain literal form is used (rather than simply the dotted decimal
   * as with other Unix programs) because it has to be a valid "host name"
   * in mailsystem terminology.
   */

  /* look like domain literal? */
  if (host[0] == '[' && host[i = (strlen (host))-1] == ']') {
    strcpy(hostname, host+1);	/* yes, copy without brackets */
    hostname[i-1] = '\0';
  } else {

    strcpy(hostname, host); /* in case host is in write-protected memory */
    if (host_name = gethostbyname_r(lcase(hostname),
				    &host_res1,
				    host_buf1,
				    MAXHOSTNAMELEN + 1,
				    &h_errno) )
      strcpy(hostname,host_name->h_name);
  }
				/* make command pipes */
  if (pipe(pipei) < 0) {
    return(NULL);
  }
  if (pipe(pipeo) < 0) {
    close (pipei[0]);
    close (pipei[1]);
    return(NULL);
  }
  if ((i = fork ()) < 0) {	/* make inferior process */
    close(pipei[0]);
    close(pipei[1]);
    close(pipeo[0]);
    close(pipeo[1]);
    return(NULL);
  }
  if (!i) {			/* if child */
    dup2(pipei[1],1);		/* parent's input is my output */
    dup2(pipei[1],2);		/* parent's input is my error output too */
    close(pipei[0]);
    close(pipei[1]);
    dup2(pipeo[0],0);		/* parent's output is my input */
    close(pipeo[0]);
    close(pipeo[1]);
    setpgrp();			/* be our own process group */
    /* now run it */
    execl("/bin/rsh", "rsh", hostname, "exec", service, 0);
    _exit(1);			/* spazzed */
  }

  close(pipei[1]);		/* parent, close child's side of the pipes */
  close(pipeo[0]);

  /* create TCP/IP stream */
  stream = (TCPSTREAM *) fs_get(sizeof (TCPSTREAM));
				
  /* copy official host name */
  stream->host = cpystr(hostname);
  /* get local name */
  gethostname (hostname,MAILTMPLEN-1);

  stream->localhost = cpystr ((host_name=gethostbyname_r(hostname,
							 &host_res2,
							 host_buf2,
							 MAXHOSTNAMELEN + 1,
							 &h_errno ) )
			      ? host_name->h_name : hostname);
  stream->tcpsi = pipei[0];	/* init sockets */
  stream->tcpso = pipeo[1];
  stream->ictr = 0;		/* init input counter */
  stream->pid = i;
  return(stream);		/* return success */
}

/*
 *TCP/IP receive line
 * Accepts: TCP/IP stream
 * Returns: text line string or NULL if failure
 */
char *
tcp_getline(TCPSTREAM * stream)
{
  int 		n;
  int		m;
  char 	*	st;
  char	*	ret;
  char	*	stp;
  char 		c = '\0';
  char 		d;

  /* make sure have data */
  if (!tcp_getdata(stream)) {
    return(NULL);
  }
  st = stream->iptr;		/* save start of string */
  n = 0;			/* init string count */
  while (stream->ictr--) {	/* look for end of line */
    d = *stream->iptr++;	/* slurp another character */
    if ((c == '\015') && (d == '\012')) {
      ret = (char *)fs_get(n--);
      memcpy(ret, st, n);	/* copy into a free storage string */
      ret[n] = '\0';		/* tie off string with null */
      return(ret);
    }
    n++;			/* count another character searched */
    c = d;			/* remember previous character */
  }

  /* copy partial string from buffer */
  memcpy((ret = stp = (char *) fs_get(n)), st, n);

  /* get more data from the net */
  if (!tcp_getdata (stream)) {
    fs_give ((void **) &ret);

  } else if ((c == '\015') && (*stream->iptr == '\012')) {
    /* special case of newline broken by buffer */
    stream->iptr++;		/* eat the line feed */
    stream->ictr--;
    ret[n - 1] = '\0';		/* tie off string with null */

  }  else if (st = tcp_getline(stream)) {

    /* else recurse to get remainder */
    ret = (char *)fs_get(n + 1 + (m = strlen(st)));

    memcpy(ret, stp ,n);		/* copy first part */
    memcpy(ret + n, st, m);	/* and second part */
    fs_give((void **)&stp);	/* flush first part */
    fs_give((void **)&st);	/* flush second part */
    ret[n + m] = '\0';		/* tie off string with null */
  }
  return(ret);
}

/*
 * TCP/IP receive buffer
 * Accepts: TCP/IP stream
 *	    size in bytes
 *	    buffer to read into
 * Returns: T if success, NIL otherwise
 */
long
tcp_getbuffer(TCPSTREAM * stream, unsigned long size, char * buffer)
{
  unsigned long 	n;
  int 			return_value;
  char		*	bufptr = buffer;

  while (size > 0) {		/* until request satisfied */
    return_value = tcp_getdata(stream);
    if (!return_value || stream->urgent_exit) {
      return(return_value);
    }

    /* OK, got some data in the buffer */
    n = min(size, stream->ictr);/* number of bytes to transfer */

    /* do the copy */
    memcpy(bufptr, stream->iptr, n);
    bufptr += n;		/* update pointer */
    stream->iptr += n;
    size -= n;			/* update # of bytes to do */
    stream->ictr -= n;
  }
  bufptr[0] = '\0';		/* tie off string */
  stream->os_send_urgent = NIL; /* disable urgentcy */
  return(_B_TRUE);
}

/*
 * TCP/IP receive data
 * Accepts: TCP/IP stream
 * Returns: _B_TRUE  if success,
 *          _B_TRUE  if user flushed SUCCESSFULLY,
 *          _B_FALSE io-error.
 */
boolean_t
tcp_getdata(TCPSTREAM * stream)
{
  unsigned long 	n;
  struct pollfd 	pollfd;
  int 			pollstatus;
  long 			poll0;
  long 			pollwait;
  long 			max_wait;
  int 			urgent_ok;

  if (stream->tcpsi < 0) {
    return(_B_FALSE);
  }

  max_wait = MAXWAIT;
  stream->u_flushed_input = NIL; /* true if successful urgent flush */
  stream->urgent_exit = NIL;     /* true if we exited URGENTly */

  while (stream->ictr < 1) {	/* if nothing in the buffer */
    /* look for urgent processing */
    if (stream->os_send_urgent) {
      urgent_ok = process_urgent_data(stream);
      stream->urgent_exit = T;
      return urgent_ok;
    }

    pollfd.fd = stream->tcpsi;	/* initialize selection vector */
    pollfd.events = POLLIN;
    poll0 = elapsed_ms();     /* for our max poll wait time */

    while ((pollstatus = poll(&pollfd, 1, IOWAIT)) <= 0) {
      if (pollstatus == 0) {
				/* TIME OUT callback */
	mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);
	/* See if we should send an urgent signal */
	if (stream->os_send_urgent) {
	  urgent_ok = process_urgent_data(stream);
	  stream->urgent_exit = T;
	  return urgent_ok;
	}
      } else {
	/* < 0, Check the error */
	if (errno != EAGAIN) {	/* error - abort on EAGAIN */
	  close(stream->tcpsi);	/* nuke the socket */
	  if (stream->tcpsi != stream->tcpso) {
	    close(stream->tcpso);
	  }
	  stream->tcpsi = stream->tcpso = -1;
	  if (!stream->disallow_callback) {
	    /* io error - str dead */
	    mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
	  }
	  return(_B_FALSE);
	}
      }

      /*
       * EAGAIN or TIMEOUT WAIT:
       * See if our max time has expired
       */
      if (stream->max_wait_on) {
	pollwait = elapsed_ms();
	pollwait -= poll0;
	if (pollwait > max_wait) {
	  close(stream->tcpsi);	/* nuke the socket */
	  if (stream->tcpsi != stream->tcpso) {
	    close(stream->tcpso);
	  }
	  stream->tcpsi = stream->tcpso = -1;
	  if (!stream->disallow_callback) {
	    mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
	  }

	  /* The the upper layers what happened */
	  mm_log((char*)connectionTimeout, WARN, NULL);

	  return(_B_FALSE);

	} else if (pollwait > MSGWAIT) {

	  mm_io_callback(stream->mailstr,
			 (max_wait - pollwait)/1000,	
			 CBWAITING);
	}
      }
      pollfd.fd = stream->tcpsi;	/* initialize selection vector */
      pollfd.events = POLLIN;
    }

    /*
     * GOT SOME DATA:
     * Always call back once per data block
     */
    mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);

    /* look for urgent processing */
    if (stream->os_send_urgent) {
      urgent_ok = process_urgent_data(stream);
      stream->urgent_exit = _B_TRUE;
      return(urgent_ok);
    }
    if ((stream->ictr = read(stream->tcpsi, stream->ibuf, BUFLEN)) < 1) {
      close(stream->tcpsi);	/* nuke the socket */
      if (stream->tcpsi != stream->tcpso) {
	close(stream->tcpso);
      }
      stream->tcpsi = stream->tcpso = -1;
      if (!stream->disallow_callback)
	/* io error - str dead */
	mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
      return(_B_FALSE);
    }

    /* point at TCP buffer */
    stream->iptr = stream->ibuf;
    stream->io_bytect += stream->ictr;
  }
  /* once for each chunk  */
  mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);
  return(_B_TRUE);
}

/*
 * TCP/IP send string as record
 * Accepts: TCP/IP stream
 *	    string pointer
 * Returns: _B_TRUE if success else _B_FALSE
 */
long
tcp_soutr(TCPSTREAM * stream, char * string)
{
  return tcp_sout(stream, string, (unsigned long) strlen(string));
}

/*
 * TCP/IP send string
 * Accepts: TCP/IP stream
 *	    string pointer
 *	    byte count
 * Returns: _B_TRUE if success else _B_FALSE
 */
#define MORCEAU 1024
long
tcp_sout(TCPSTREAM * stream, char * string, unsigned long size)
{
  int 		i;
  struct pollfd pollfd;
  int 		pollstatus;
  long 		t0;
  long		t1;
  long 		chunks;
  long		le_reste;
  long 		write_size;
  long 		wsize;
  long 		poll0;
  long 		pollwait;
  long 		max_wait;

  if (stream->tcpso < 0) {
    return(_B_FALSE);
  }

  max_wait = MAXWAIT;
  chunks = size/MORCEAU;
  le_reste = size % MORCEAU;
  t0 = elapsed_ms();
  while (size > 0) {		/* until request satisfied */
    pollfd.fd = stream->tcpso;	/* initialize selection vector */
    pollfd.events = POLLOUT;

    poll0 = elapsed_ms();     /* for our max poll wait time */

    while ((pollstatus = poll(&pollfd, 1, IOWAIT)) <= 0) {
      if (pollstatus == 0) {
				/* TIME OUT callback */
	mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT); 
	t0 = elapsed_ms();

      } else {
	/* error - abort if not EAGAIN */
	if (errno != EAGAIN) {	
	  close(stream->tcpsi);	/* nuke the socket */
	  if (stream->tcpsi != stream->tcpso) {
	    close(stream->tcpso);
	  }
	  stream->tcpsi = stream->tcpso = -1;
	  if (!stream->disallow_callback)
	    /* io error - str dead */
	    mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
	  return(_B_FALSE);
	}
      }

      /*
       * See if our max time has expired 
       */
      if (stream->max_wait_on) {
	pollwait = elapsed_ms();
	pollwait -= poll0;
	if (pollwait > max_wait) {      /* a fatal error */
	  close(stream->tcpsi);	/* nuke the socket */
	  if (stream->tcpsi != stream->tcpso) {
	    close(stream->tcpso);
	  }
	  stream->tcpsi = stream->tcpso = -1;
	  if (!stream->disallow_callback) {
	    mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
	  }
	  return(_B_FALSE);
	}  else if (pollwait > MSGWAIT) {
	  mm_io_callback(stream->mailstr, (max_wait-pollwait)/1000, CBWAITING);
	}
      }
      pollfd.fd = stream->tcpso;	/* initialize selection vector */
      pollfd.events = POLLOUT;
    }

    /* See if our elapsed time for a call back has happened */
    t1 = elapsed_ms();
    if ((t1 - t0) >= IOWAIT) {
      mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);
      t0 = t1;
    }

    /* 
     * write always writes exactly wsize bytes before
     * returning
     */
    wsize = (chunks == 0 ? le_reste : MORCEAU);
    if((i = write(stream->tcpso, string, wsize)) < 0) {
      puts(strerror (errno));
      close(stream->tcpsi);	/* nuke the socket */
      if (stream->tcpsi != stream->tcpso) {
	close(stream->tcpso);
      }
      stream->tcpsi = stream->tcpso = -1;
      if (!stream->disallow_callback) { /* io error - stream dead*/
	mm_io_callback(stream->mailstr, stream->io_bytect, CBSTRDEAD);
      }
      return(_B_FALSE);
    }
    size -= i;			/* count this size */
    string += i;
    chunks -= 1;		/* one less MORCEAU */
				/* report the changed byte count */
    stream->io_bytect += i;
    mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);
  }
  return(_B_TRUE);		/* all done */
}

/*
 * TCP/IP close
 * Accepts: TCP/IP stream
 */
void
tcp_close(TCPSTREAM * stream)
{
  tcp_abort(stream);		/* nuke the stream */
				/* flush host names */
  fs_give((void **) &stream->host);
  fs_give((void **) &stream->localhost);
  fs_give((void **) &stream);	/* flush the stream */
}


/* 
 * TCP/IP abort stream
 * Accepts: TCP/IP stream
 * Returns: _B_FALSE always
 */
long
tcp_abort(TCPSTREAM * stream)
{
  int 		i;

  if (stream->tcpsi >= 0) {	/* no-op if no socket */
    close(stream->tcpsi);	/* nuke the socket */
    if (stream->tcpsi != stream->tcpso) {
      close (stream->tcpso);
    }
    stream->tcpsi = stream->tcpso = -1;
  }
  if (stream->pid) {		/* reap any PID we may have */
    grim_pid_reap_status(stream->pid,_B_TRUE, NULL);
    stream->pid = 0;
  }
  return(_B_FALSE);
}


/*
 * TCP/IP get host name
 * Accepts: TCP/IP stream
 * Returns: host name for this stream
 */
char *
tcp_host(TCPSTREAM * stream)
{
  return(stream->host);		/* return host name */
}

/*
 * TCP/IP return port for this stream
 * Accepts: TCP/IP stream
 * Returns: port number for this stream
 */
unsigned long
tcp_port(TCPSTREAM * stream)
{
  return(stream->port);		/* return port number */
}

/*
 * TCP/IP get local host name
 * Accepts: TCP/IP stream
 * Returns: local host name
 */
char *
tcp_localhost(TCPSTREAM * stream)
{
  return(stream->localhost);	/* return local host name */
}

/*
 * TCP/IP get server host name
 * Accepts: pointer to destination
 * Returns: string pointer if got results, else NIL
 */
char *
tcp_clienthost(char *dst)
{
  struct hostent *	hn;
  struct hostent	host_res;
  struct sockaddr_in 	from;
  char 		*	host;
  char 			host_buf[MAXHOSTNAMELEN + 1];
  int 			h_errno ;
  int 			fromlen = sizeof (from);

  if (getpeername(0, (struct sockaddr *) &from, &fromlen)) {
    return "UNKNOWN";
  }

  hn = gethostbyaddr_r((char *) &from.sin_addr,
		       sizeof (struct in_addr),
		       from.sin_family,
		       &host_res,
		       host_buf,
		       MAXHOSTNAMELEN + 1,
		       &h_errno);

  host = ((hn != NIL) ? hn->h_name : inet_ntoa(from.sin_addr));
  strcpy (dst, host);

  return(dst);
}

/* OTHER Sun IMAPD extenstions */

/*
 * PLACE variables on the TCP STREAM:
 * for disallow_callback setting
 */
void
set_disallow_flag(TCPSTREAM * stream, int value)
{
  stream->disallow_callback = value;
}

void
set_outgoing_log(TCPSTREAM * stream, char *fname)
{
  stream->outgoing_log = fname;
  stream->wrote_ihdr = NIL;	/* not written yet */
}

void
set_use_maxwait(TCPSTREAM * stream, int value)
{
 
  if (stream) {
    stream->max_wait_on = value;
  }
}

/* Some accessors */
char *
get_outgoing_log(TCPSTREAM * stream)
{
  return(stream->outgoing_log);
}

int
outgoing_from_done(void * stream)
{
  TCPSTREAM 	*	tcpstr= (TCPSTREAM *) stream;
  int 			state= tcpstr->wrote_ihdr;

  if (!state) {
    tcpstr->wrote_ihdr = _B_TRUE;	/* done now */
  }

  return(state);
}

void
set_tcpstream_stream(TCPSTREAM * tcpstr, void * mailstr)
{
  tcpstr->mailstr = mailstr;
}

/*
 * Used for accumulating io count - called from imap2 driver
 */
void 
clear_tcp_iocounter(TCPSTREAM * tcpstr)
{
  tcpstr->io_bytect = 0;
}

int
tcp_flushed_input(TCPSTREAM * stream)
{
  return stream->u_flushed_input;
}

void
os_clear_send_urgent(TCPSTREAM * stream)
{
  stream->os_send_urgent = 0;
}

void
os_increment_send_urgent(TCPSTREAM * stream)
{
  stream->os_send_urgent += 1;	/* tell os to send an urgent signal */
}

/*
 * Some urgent byte processing
 * send the urgent byte
 */
int 
send_tcp_urgent(TCPSTREAM * stream)
{
  int 		n;
  int 		outfd = stream->tcpso;

  if ((n = send(outfd, "C", 1, MSG_OOB)) != 1) {
    mm_tcp_log(stream->mailstr, "TCP: urgent byte NOT sent");
    return(_B_FALSE);
  } else {
    mm_tcp_log(stream->mailstr, "TCP: urgent byte sent");
    /* Send URGENT DONE byte to clear the stream */
    if ((n = send(outfd, "D", 1, 0)) != 1) {
      mm_tcp_log(stream->mailstr, "TCP: Write error"); 
    }
    return(_B_TRUE);
  }
}

/* process tcp urgent bytes */
int process_urgent_data(TCPSTREAM * stream)
{
  stream->os_send_urgent = 0;
  if (send_tcp_urgent(stream)) {
    if (received_tcp_urgent_reply(stream)) {
      return(_B_TRUE);
    } else {
      return(_B_FALSE);
    }
  } else {
    return(_B_FALSE);
  }
}

#define TROISMINS 60*1000	/* max wait for urgent byte */
int
received_tcp_urgent_reply(TCPSTREAM * stream)
{
  int 		infd = stream->tcpsi;
  char 		buf[BUFLEN];
  long 		t0 = elapsed_ms();
  long		tcb= t0;
  long 		t1;
  long		tel;
  char 		mark_byte[2];
  long 		nread = stream->io_bytect;

  mm_tcp_log(stream->mailstr, "TCP: Waiting for urgent byte");

  /*
   * Read using a select, flushing until an urgent exception 
   * is detected. Then we flush to the urgent byte
   */
  while (1) {
    fd_set		readfds;
    fd_set		exceptfds;
    int 		selectioner;
    int 		nfds = infd + 1;
    struct timeval	tv;

    FD_ZERO (&readfds);
    FD_ZERO (&exceptfds);

    FD_SET (infd, &readfds);
    FD_SET (infd, &exceptfds);

    tv.tv_sec = 0;
    tv.tv_usec = IOWAIT;
    selectioner = select(nfds, &readfds, 0, &exceptfds, &tv);
    if (selectioner < 0) {
      sprintf(buf, "TCP: Select error= %s", strerror(errno));
      mm_tcp_log(stream->mailstr, buf);
      return(_B_FALSE);
    } else if (selectioner == 0) { /* time out */
      	mm_io_callback(stream->mailstr, nread, CBTIMEOUT);
    } else if (FD_ISSET(infd, &exceptfds)) {
      mm_tcp_log(stream->mailstr, "TCP: select-Out of band data detected");
      break;
    } else {			/* must be read */
      int 	m;

      m = read(infd, buf, BUFLEN); /* at most n bytes */
      if (m <= 0) {
	sprintf(buf, "TCP: Read error= %s", strerror(errno));
	mm_tcp_log(stream->mailstr, buf);
	return(_B_FALSE);
      } else {
	nread += m;
      	mm_io_callback(stream->mailstr, nread, CBTIMEOUT);
      }
    }
    /* We don't wait forever */
    t1 = elapsed_ms() - t0;
    if (t1 > TROISMINS) {
      mm_tcp_log(stream->mailstr, 
		 "TCP: timeout, URGENT byte NOT read from server.");
      return(_B_FALSE);		/* Enough is enough. */
    }
  }
  /*
   * OK, select indicates that OUT-OF-BAND data has arrived.
   * We flush until we find this byte
   */
  t0 = elapsed_ms();
  tcb = t0;
  while (1) {
    int 	mark = 0;
    int		n;

    if (ioctl(infd, SIOCATMARK, &mark) < 0) {
      mm_tcp_log(stream->mailstr, "TCP: IOCTL system call error");
      return(_B_FALSE);		/* ioctl error */
    }
   
    if (mark) {			/* at the mark now */
      break;
    } else {			/* read some more data */
      n = read(infd, buf, BUFLEN); /* read upto mark */
      if (n <= 0) {
	mm_tcp_log(stream->mailstr, "TCP read error");
	return(_B_FALSE);	/* i/o error. Forget it. */
      }
    }
    tel = elapsed_ms();
    t1 = tel - t0;		/* milliseconds bien passe. */
    if (t1 > TROISMINS) {
      mm_tcp_log(stream->mailstr, 
		 "TCP: timeout, URGENT byte NOT read from server.");
      return(_B_FALSE);		/* Trop c'est trop. Enough is enough. */
    } else {
      t1 = tel - tcb;
      if (t1 > IOWAIT) {
	tcb = tel;
	mm_io_callback(stream->mailstr, stream->io_bytect, CBTIMEOUT);
      }
    }
  }
  /* READ the mark byte */
  if (recv(infd, mark_byte, 1, MSG_OOB) < 0) {
    mm_tcp_log(stream->mailstr, "TCP: System error reading urgent byte");
    return(_B_FALSE);
  }
  stream->u_flushed_input = _B_TRUE; /* we flushed it */
  mm_tcp_log(stream->mailstr, "TCP: URGENT byte read");
  return(_B_TRUE);
}

/*
 *	Input: stream
 *	Output: stream
 *	Purpose: initialize tcp_globals.
 */
void
tcp_global_init(MAILSTREAM *
stream)
{
  TCP_GLOBALS *tg = (TCP_GLOBALS *)fs_get(sizeof(TCP_GLOBALS));

  tg->tcptimeout = NIL;
  tg->tcptimeout_read = 0;
  tg->tcptimeout_write = 0;

  stream->tcp_globals = tg;
}

/*
 *        Input: stream
 *        Output: stream
 *        Purpose: flush tcp_globals.
 */
void
tcp_global_free(MAILSTREAM * stream)
{
  if (stream->tcp_globals) {
     fs_give ((void **) &stream->tcp_globals);
  }
}
