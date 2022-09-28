/* $XConsortium: logger.c /main/cde1_maint/3 1995/10/09 11:50:25 pascale $ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

static void serve(char *progName, int s, int logfd);

int
main(int argc, char **argv)
{
    int s;
    int i1;
    int noFork = 0;
    int pid;
    int logfd = 1;
    struct sockaddr_in myaddr_in;
    extern char *optarg;
    extern int optind, optopt;

    while(EOF != (i1 = getopt(argc, argv, "fl:"))) {
	switch(i1) {
	case 'f' :
	    noFork = 1;
	    break;

	case 'l' :
	    if ((logfd =
		    open(optarg, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
		(void) perror(optarg);
		(void) exit(1);
	    }
	    break;
	}
    }

    /* init data... */
#if defined(USL) || defined(__uxp__)
    (void) memset((void *) &myaddr_in, (int) '\0', sizeof(myaddr_in));
#else
    (void) memset(myaddr_in, '\0', sizeof(myaddr_in));
#endif

    /* set up my address for socket... */
    myaddr_in.sin_family = AF_INET;
    /* listen to the wildcard address... */
    myaddr_in.sin_addr.s_addr = INADDR_ANY;
    /* use port 4444... */
    myaddr_in.sin_port = 4444;

    /* create a socket... */
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	(void) perror("socket");
	(void) exit(1);
    }

    /* bind the servers addr to the socket... */
    if (bind(s, (struct sockaddr *)&myaddr_in, sizeof(myaddr_in)) == -1) {
	(void) perror("bind");
	(void) exit(1);
    }

    /* daemonize ourself... */
    (void) setsid();

    if (noFork) {
	pid = 0 ;
    } else {
	pid = fork();
    }

    switch(pid) {
    case -1:
	/* unable to fork... */
	(void) perror("fork");
	(void) _exit(1);
	break;

    case 0:
	/* child... */
	(void) serve(argv[0], s, logfd);
	break;

    default:
	/* parent... */
	(void) exit(0);
    }

    (void) exit(0);
}

static void
logStartStop(char *progName, int logfd, int start)
{
    static char *savedProgName = (char *) 0;
    static int savedLogfd = -1;
    time_t now;
    char buffer[BUFSIZ];
    char *tstring;

    if (progName && *progName)
	savedProgName = progName;
    if (logfd >= 0)
	savedLogfd = logfd;

    (void) time(&now);
    tstring = ctime(&now);
    /* remove the trailing '\n'... */
    tstring[strlen(tstring) - 1] = '\0';

    (void) sprintf(buffer, "%s: %s %s\n",
	    (savedProgName && *savedProgName) ? savedProgName : "logger",
	    start ? "starting" : "terminating",
	    tstring);

    if (savedLogfd >= 0)
	(void) write(savedLogfd, buffer, strlen(buffer));
}
    
void
shutDown(int sig)
{
    (void) logStartStop(NULL, -1, 0);
}

static void
serve(char *progName, int s, int logfd)
{
    int i;
    int cc;
    int addrlen;
    char *c1;
    char buffer[BUFSIZ];
    struct sockaddr_in clientaddr_in;
    struct hostent *hp;
    
    for (i = 0; i < 2; i++) {
	if ((i != s) && (i != logfd)) {
	    (void) close(i);
	}
    }

    (void) logStartStop(progName, logfd, 1);
    (void) signal(SIGINT, shutDown);
    (void) signal(SIGQUIT, shutDown);
    (void) signal(SIGTERM, shutDown);
    (void) signal(SIGUSR1, shutDown);
    (void) signal(SIGUSR2, shutDown);

    addrlen = sizeof(clientaddr_in);
    while (1) {
	(void) memset((char *) &clientaddr_in, 0, sizeof(clientaddr_in));
	cc = recvfrom(s, buffer, sizeof(buffer) - 1, 0, 
		      (struct sockaddr *)&clientaddr_in, &addrlen);

	if (cc < 0) {
	    (void) exit(1);
	}

	/* null term the buffer... */
	buffer[cc] = '\0';

	hp = gethostbyaddr((char *) &clientaddr_in.sin_addr,
		sizeof(clientaddr_in.sin_addr), clientaddr_in.sin_family);
	if (hp) {
	    c1 = hp->h_name;
	} else {
	    c1 = inet_ntoa(clientaddr_in.sin_addr);
	}
	(void) write(logfd, c1, strlen(c1));
	(void) write(logfd, ": ", 2);

	(void) write(logfd, buffer, strlen(buffer));
	(void) write(logfd, "\n", 1);

	/* find the first space after the id... */
	for (c1 = buffer; *c1 && (*c1 != ' ') && (*c1 != '\t'); c1++) {
	    ;
	}
	(void) strcpy(c1, " ACK");
	(void) sendto(s, buffer, strlen(buffer), 0, (struct sockaddr *)&clientaddr_in, addrlen);
    }
}
