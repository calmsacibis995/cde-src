/* $XConsortium: mesg.h /main/cde1_maint/3 1995/10/19 13:03:06 mgreess $ */

/*
 * COMPONENT_NAME: INC
 *
 * FUNCTIONS: mesg.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*       
 * (c) Copyright 1993, 1994 Hewlett-Packard Company 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc. 
 * (c) Copyright 1993, 1994 Novell, Inc. 
 */


#if defined(__osf__) && !defined(_MESG_H_)
#define _MESG_H_
#elif !defined(_H_MESG)
#define _H_MESG

#include <nl_types.h>
#include <limits.h>
#define CAT_MAGIC 	505
#define CATD_ERR 	((nl_catd) -1)
#define NL_MAXOPEN	10

struct _message {
	unsigned short 	_set,
			_msg;
	char 		*_text;
	unsigned	_old;
};

struct _header {
	int 		_magic;
	unsigned short	_n_sets,
			_setmax;
	char 		_filler[20];
};
struct _catset {
	unsigned short 	_setno,
			_n_msgs;
	struct _msgptr 	*_mp;
	char	**_msgtxt;
};

#if defined(__osf__) && !defined(_STDIO_H_)
#include <stdio.h>
#elif !defined(_H_STDIO_)
#include <stdio.h>
#endif
struct catalog_descriptor {
	char		*_mem;
	char		*_name;
	FILE 		*_fd;
	struct _header 	*_hd;
	struct _catset 	*_set;
	int		_setmax;
	int 		_count;
	int		_pid;
};


struct _msgptr {
	unsigned short 	_msgno,
			_msglen;
	unsigned long	_offset;
};
#endif
