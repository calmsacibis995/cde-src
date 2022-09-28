/*
 * @(#)misc.h	1.4 97/05/19
 *
 * Program:	Miscellaneous utility routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	5 July 1988
 * Last Edited:	28 June 1995
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifndef _MISC_H_
#define _MISC_H_
#include "UnixDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Some C compilers have these as macros */

/* Compatibility definitions */
extern ssize_t	safe_write(const int fd, const void * buf, const size_t nbytes);

#define pmatch(s,pat) pmatch_full (s,pat,NIL)


/* Function prototypes */

char *		ucase(char * string);
char *		lcase(char * string);
char *		cpystr(const char * string);
unsigned long 	find_rightmost_bit(unsigned long * valptr);
long 		search(char * s, long c, char * pat, long patc);
long 		ssrc(char ** s,long * c,char * pat, long multiword);
long 		pmatch_full(char * s, char * pat, char delim);
long 		dmatch(char * s, char * pat, char delim);

#ifdef min
#undef min
#endif
#define min(i,j)	(i<j?i:j)
#ifdef max
#undef max
#endif
#define max(i,j)	(i>j?i:j)

#ifdef __cplusplus
}
#endif

#endif /* _MISC_H_ */
