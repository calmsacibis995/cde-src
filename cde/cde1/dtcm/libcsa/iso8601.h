/* $XConsortium: iso8601.h /main/cde1_maint/2 1995/10/10 13:30:01 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef ISO8601_H
#define ISO8601_H

#pragma ident "@(#)iso8601.h	1.6 96/03/01 Sun Microsystems, Inc."

/*
 * iso8601.h
 * 
 * header file for functions to convert between tick and
 * ISO 8601 times, ranges and durations.
 */

/* extern functions */
extern int	_csa_iso8601_to_tick(char *, time_t *);
extern int	_csa_tick_to_iso8601(time_t, char *);
extern int	_csa_iso8601_to_range(char *, time_t *, time_t *);
extern int	_csa_range_to_iso8601(time_t, time_t, char *);
extern int	_csa_iso8601_to_duration(char *, time_t *);
extern int	_csa_duration_to_iso8601(time_t, char *);

#endif /* ISO8601_H */
