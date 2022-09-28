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
	int i;

	i = atoi("456");
	system("mv /tmp/foo/* /tmp/foo2");
}

