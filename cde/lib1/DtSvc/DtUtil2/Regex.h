/* $XConsortium: Regex.h /main/cde1_maint/3 1995/10/08 22:21:28 pascale $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/* Hp DT's version of an 8.0 include file; needed for Fnmatch */

#ifndef DtREGEX_INCLUDED
#define DtREGEX_INCLUDED

#include <sys/types.h>

#ifndef _SYS_STDSYMS_INCLUDED
#if defined(__apollo) || defined(sun) || defined(__hp_osf) || defined(USL) || defined(sco) || defined(__uxp__) || defined(__osf__)
#  define _INCLUDE_XOPEN_SOURCE
# else
#  include <sys/stdsyms.h>
# endif
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef _INCLUDE_XOPEN_SOURCE

/*
 *  WARNING!  This file is currently only provided to support <regexp.h>
 *            and the associated functionality defined on the regexp(3X)
 *            reference page.  The contents of this file will change as
 *            necessary to conform with a new standard for regular
 *            expression processing currently under development.  Any
 *            changes necessary to this file will be made without regard
 *            to backward compatibility for any application that chooses
 *            to directly use the items declared herein.  Until this file
 *            is fully supported as part of the new standard, use of only
 *            the entry points and data structures defined by regexp(3X)
 *            is recommended.
 */


/* Constants:	*/

#define	REG_NPAREN	255		/* upper limit on number of substrings that can be reported */
					/* Note: the upper limit for this number is 255			*/


typedef struct {
	unsigned char	*c_re;		/* pointer to compiled RE buffer				*/
	unsigned char	*c_re_end;	/* pointer to byte following end of compiled RE			*/
	unsigned char	*c_buf_end;	/* pointer to end of compiled RE buffer				*/
	size_t		re_nsub;	/* number of parenthesized subexpressions			*/
	int		anchor;		/* true if RE must be anchored					*/
	int		flags;		/* storage for flags						*/
} regex_t;

typedef struct {
	char	*rm_sp;		/* pointer to start of substring */
	char	*rm_ep;		/* pointer past end of substring */
} regmatch_t;


/* Regcomp Flags:	*/

#define	REG_EXTENDED	0001		/* use Extended Regular Expressions				*/
#define	REG_NEWLINE	0002		/* treat \n as a regular character				*/
#define	REG_ICASE	0004		/* ignore case in match						*/
#define	REG_NOSUB	0010		/* report success/fail only in regexec()			*/
#define	_REG_NOALLOC	0040		/* don't allocate memory for buffer--use only what's provided	*/
#define	_REG_C_ESC	0200		/* special flag to ease /usr/bin/awk implementation		*/
#define _REG_ALT	0400		/* special flag to ease /bin/grep implementation		*/

/* Regexec Flags:	*/

#define REG_NOTBOL	0001		/* never match ^ as special char */
#define REG_NOTEOL	0002		/* never matchj $ as special char */


/* Error Codes:	*/

#define	REG_NOMATCH	20		/* regexec() failed to match					*/
#define	REG_ECOLLATE	21		/* invalid collation element referenced				*/
#define	REG_EESCAPE	22		/* trailing \ in pattern					*/
#define	REG_ENEWLINE	36		/* \n found before end of pattern and REG_NEWLINE flag not set	*/
#define	REG_ENSUB	43		/* more than nine \( \) pairs or nesting level too deep		*/
#define	REG_ESUBREG	25		/* number in \digit invalid or in error				*/
#define	REG_EBRACK	49		/* [ ] imbalance						*/
#define	REG_EPAREN	42		/* \( \) imbalance or ( ) imbalance				*/
#define	REG_EBRACE	45		/* \{ \} imbalance						*/
#define	REG_ERANGE	23		/* invalid endpoint in range statement				*/
#define	REG_ESPACE	50		/* out of memory for compiled pattern				*/
#define	REG_EMEM	51		/* out of memory while matching expression */

#define	REG_EABRACE	16		/* number too large in \{ \} construct				*/
#define REG_EBBRACE	11		/* invalid number in \{ \} construct				*/
#define REG_ECBRACE	44		/* more than 2 numbers in \{ \} construct			*/
#define REG_EDBRACE	46		/* first number exceeds second in \{ \} construct		*/
#define	REG_ECTYPE	24		/* invalid character class type named				*/

#define	REG_ENOSEARCH	41		/* no remembered search string					*/
#define	REG_EDUPOPER	26		/* duplication operator in illegal position			*/
#define	REG_ENOEXPR	27		/* no expression within ( ) or on one side of an |		*/

#ifdef __STDC__
	extern int regcomp(regex_t *,const char *,int);
	extern int regexec(const regex_t *,const char *,
			   size_t,regmatch_t *,int);
	extern void regfree(regex_t *);
#else /* __STDC__ */
	extern int regcomp();
	extern int regexec();
	extern void regfree();
#endif /* __STDC__ */

#endif /* _INCLUDE_XOPEN_SOURCE */

#endif /* DtREGEX_INCLUDED */
