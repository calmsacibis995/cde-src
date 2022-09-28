/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/***********************************************************************
 *
 * This program takes over reset of the server.
 * Compile with -lXhp11 -lX11.
 *
 */

#include <X11/Xlib.h>
#include <time.h>

Display *display;
main(argc,argv)
int		argc;
char	*argv[];
{
	pid_t clientFork;
	int status;

	clientFork = vfork();
	if(clientFork < 0)
	{
		printf("fork failed\n");
	}

	if(clientFork == 0)
	{
/*		status = execlp("/bin/sort", "sort", "-o", "/usr/tmp/lsdmnf3_26333",
						"/usr/tmp/lsdmnf1_26333", (char *) 0);*/

		status = execlp("/bin/sh", "sh", "-c", 
		"/bin/sort -o /usr/tmp/lsdmnf3_26333 /usr/tmp/lsdmnf1_26333");
		if(status == -1)
		{
			printf("exec failed \n");
			exit(-1);
		}
	}

	while(waitpid(clientFork, &status, 0) != clientFork);

	exit(0);
}
