/* $XConsortium: dtfns.h /main/cde1_maint/1 1995/07/17 19:45:47 drk $ */
/*	@(#)dtfns.h 1.5 96/06/17 SMI	*/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */


#ifndef _DTFNS_H
#define _DTFNS_H


#ifdef FNS

#define DTFNS_SEPARATOR		'/'
#define DTFNS_BUFSIZE		1024

extern int
dtfns_lookup_calendar(const char *fullname, 
		      const char* cal_ref_type, 
		      const char* cal_addr_type,
		      char *cal_addr, int max_size);

extern int
dtfns_register_calendar(const char *fullname, 
			const char* cal_ref_type, 
			const char* cal_addr_type,
			const char *cal_addr);

extern int dtfns_init(void);
extern int dtfns_available(void);
#endif /* FNS */

#endif /* _DTFNS_H */

