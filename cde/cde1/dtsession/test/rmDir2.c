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
#include <stdio.h>
#include <X11/Xlib.h>
#include <time.h>

Display *display;
main(argc,argv)
int		argc;
char	*argv[];
{
    pid_t  clientFork;
    int	   execStatus, i;
	char   *foo;
	 
	foo = (char *) malloc(1 * sizeof(char));
  	strcpy(foo, '\\');
	printf("hello world\\%c\n", foo);
}
