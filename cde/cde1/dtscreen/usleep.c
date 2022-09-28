/* $XConsortium: usleep.c /main/cde1_maint/3 1995/10/19 16:15:13 pascale $ */
/*
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*-
 * usleep.c - OS dependant implementation of usleep().
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Revision History:
 * 30-Aug-90: written.
 *
 */

#include "dtscreen.h"

int
usleep(usec)
    unsigned long usec;
{
#ifdef SYSV
#ifdef hpV4
    poll((struct pollfd *) 0, (int) 0, (int) usec / 1000);	
#else
    poll((struct poll *) 0, (size_t) 0, usec / 1000);	/* ms resolution */
#endif
#else
    struct timeval timeout;
    timeout.tv_usec = usec % (unsigned long) 1000000;
    timeout.tv_sec = usec / (unsigned long) 1000000;
    select(0, (void *) 0, (void *) 0, (void *) 0, &timeout);
#endif
    return 0;
}

/*
 * returns the number of seconds since 01-Jan-70.
 * This is used to control rate and timeout in many of the animations.
 */
long
seconds()
{
    struct timeval now;

    gettimeofday(&now, (struct timezone *) 0);
    return now.tv_sec;
}
