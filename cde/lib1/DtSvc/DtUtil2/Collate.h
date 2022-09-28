/* $XConsortium: Collate.h /main/cde1_maint/1 1995/07/17 18:08:49 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* Hp DT's version of an 8.0 include file; needed for Fnmatch */

#ifndef DtCOLLATE_INCLUDED
#define DtCOLLATE_INCLUDED


#define MASK077		077
#define ENDTABLE	0377		/* end mark of 2 to 1 character		*/

struct col_21tab {
	unsigned char	ch1;		/* first char of 2 to 1			*/
	unsigned char	ch2;		/* second char of 2 to 1		*/
	unsigned char	seqnum;		/* sequence number			*/
	unsigned char	priority;	/* priority				*/
};

struct col_12tab {
	unsigned char	seqnum;		/* seqnum of second char of 1 to 2	*/
	unsigned char	priority;	/* priority of 1 to 2 char		*/
};

#endif /* DtCOLLATE_INCLUDED */
