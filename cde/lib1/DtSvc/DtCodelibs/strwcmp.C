/*
 * File:	strwcmp.C $XConsortium: strwcmp.C /main/cde1_maint/1 1995/07/14 20:09:17 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#include <stdio.h>
#include <codelibs/nl_hack.h>
#include <string.h>
#include "stringx.h"

#define QUOTE 0x40000000

#ifdef DEBUG
    static char tabs[] = "          ";
#   define TABS (&tabs[10 - depth])

    static int
    RETURN(int ret, int depth)
    {
	printf("%sreturning %s\n", TABS,
		(ret ? "SUCCEEDED" : "FAILED"));
	return ret;
    }
#else
#define RETURN(x,y) (x)
#endif

/* FORWARD */
static int match(const char *, const char *, int);
static const char *next_patt(const char *, int advance = 1);
static int match_class(const char *, char);

/* INLINE */
static int
next_char(register const char *pattern, const char **cpp = NULL)
{
    register int ret = pattern ? (int)CHARAT(pattern) : '\0';

    if (ret != '\0')
    {
	ADVANCE(pattern);
        /* AIX needs line broken to get around macro bug (Temporary Fix) */
        if (ret == '\\' &&
            CHARAT(pattern) != '\0')
	    ret = QUOTE | (int)CHARADV(pattern);
    }

    if (cpp != NULL)
	*cpp = pattern;
    return ret;
}

int
strwcmp(const char *pattern, const char *string)
/*
 * String  'pattern'  is matched  against  string  'string'.  Zero is
 * returned  if the match is  successful.  'pattern'  may  contain the
 * shell metas * and ?  and the  semantics are the same.  ?  and * may
 * be escaped with \
 */
{
    return !match(pattern, string, 0);
}

int 
strwcmpi(const char *apattern, const char *astring)
/*
 * This is a case-insensitive version of strwcmp().
 *
 * String  'pattern'  is matched  against  string  'string'.  Zero is
 * returned  if the match is  successful.  'pattern'  may  contain the
 * shell metas * and ?  and the  semantics are the same.  ?  and * may
 * be escaped with \
 */
{
    /* make upper-case copies of the strings, and then do */
    /* the normal comparison.                             */

    static char *pattern = NULL;
    static char *string = NULL;
    static int plen = -1;
    static int slen = -1;

    register int l;
    if ((l = strlen((char *)apattern)) > plen)
    {
	strfree(pattern);
	pattern = strnew(plen = l);
    }
    if ((l = strlen((char *)astring)) > slen)
    {
	strfree(string);
	string = strnew(slen = l);
    }
    (void)strupper(strcpy(pattern, apattern));
    (void)strupper(strcpy(string, astring));

    return !match(pattern, string, 0);
}

// stwpat returns a pointer to the first meta-character if the string
// is a pattern, else NULL
char *
strwpat(register const char *pattern)
{
    register int ch;
    register char *prev_pattern = (char *)pattern;

    while ((ch = next_char(pattern, &pattern)) != '\0')
    {
	switch (ch)
	{
	case '*': 
	    return prev_pattern;
	case '?': 
	    return prev_pattern;
	case '[': {
	    register const char *eop = next_patt(prev_pattern, 0);
	    if (CHARAT(eop) == ']')
		return prev_pattern;
	    break;
	}
	}

	prev_pattern = (char *)pattern;
    }

    return NULL;
}

/*
 * match will check to see if pattern can successfully be applied to
 * the beginning of string.
 */
static int 
match(register const char *pattern, register const char *string, int depth)
{
#ifdef DEBUG
    printf("%smatch(\"%s\", \"%s\")\n", TABS, pattern, string);
#endif
    int ch;
    const char *cp;

    while ((ch = next_char(pattern, &cp)) != '\0')
    {
	const char *laststr = string;
	register int testchar = (int)CHARADV(string);

	switch (ch)
	{
	case '*': {
	    pattern = cp;	/* skip over '*' */
	    string = laststr;	/* reverse - testchar not used */

	    const char *s = string;
	    do
		if (match(pattern, s, depth + 1))
		    return RETURN(1, depth);
	    while (CHARADV(s) != '\0');
	    return RETURN(0, depth);
	}
	case '?': 
	    break;
	case '[': {
	    int mt = match_class(pattern, testchar);
	    if (mt == 0)
		return RETURN(0, depth);
	    else if (mt == 2 && ch != testchar)
		return RETURN(0, depth);
	    break;
	}
	default: 
	    if ((ch & ~QUOTE) != testchar)
		return RETURN(0, depth);
	    break;
	}

	if (testchar == '\0')
	    string = laststr;	// reverse string

	pattern = next_patt(pattern);
    }

    return RETURN(CHARAT(string) == '\0', depth);
}

static int
match_class(register const char *clss, register char testchar)
/*
 *	pattern is a pointer to the leading [ of
 *	a shell-type class.  testchar is the character to match against
 *	the class.
 */
{
    int match = 1;		/* false if first char is '!' */

    /* find end of class, ie an un-escaped ']' */
    register const char *eop = next_patt(clss, 0);
    ADVANCE(clss);

    if (CHARAT(eop) != ']')
	return 2;

    if (CHARAT(clss) == '!')
    {
	match = 0;
	ADVANCE(clss);
    }

    while (clss < eop)
    {
	register int ch = next_char(clss, &clss);
	char const *clss_end = clss;
	int sep = next_char(clss_end, &clss_end);
	int ch2 = next_char(clss_end, &clss_end);

	/* check if next three chars are a range */
	if (sep == '-' && ch2 != ']')
	{
	    /* check range - we have to use strcoll to do it right */
	    char c1[4], c2[4], tc[4];
	    c1[1] = c2[1] = tc[1] = '\0';
	    c1[2] = c2[2] = tc[2] = '\0';
	    ch &= ~QUOTE;
	    WCHAR(ch, c1);
	    ch2 &= ~QUOTE;
	    WCHAR(ch2, c2);
	    WCHAR(testchar, tc);

	    /* if (ch <= testchar && testchar <= ch2) // Original code */

	    /* Second implementation:
	     * if (nl_strncmp(c1, tc, 1) <= 0 && nl_strncmp(tc, c2, 1) <= 0)
	     *     return match;
	     */

	    /* Third, portable implementation: */
	    if (strcoll(c1, tc) <= 0 && strcoll(tc, c2) <= 0)
		return match;
	    clss = clss_end;
	}
	else			/* they are not a range, check simple
				   match */
	{
	    if ((ch & ~QUOTE) == testchar)
		return match;
	}
    }

    return !match;
}

static const char *
next_patt(register const char *pattern, int advance)
{
    if (CHARAT(pattern) == '[')
    {
	int ch;
	const char *pp = pattern;
	ADVANCE(pp);

	if (CHARAT(pp) == '^')
	    ADVANCE(pp);

	if (CHARAT(pp) == ']')
	    ADVANCE(pp);

	char const *np;
	for (; (ch = next_char(pp, &np)) != '\0'; pp = np)
	    if (ch == ']')
		return (advance ? np : pp);
    }

    next_char(pattern, &pattern);
    return pattern;
}

#ifdef DEBUG
#define MAIN main
MAIN()
{
    char pattern[50], string[50];

    while (1)
    {
	putchar('\n');
	printf("pattern:  ");
	if (gets(pattern) == NULL)
	    break;
	printf("string:   ");
	if (gets(string) == NULL)
	    break;
	printf("MATCH is %s\n",
		((strwcmp(pattern, string) == 0) ? "SUCCEEDED" : "FAILED"));
	putchar('\n');
	printf("MATCHI is %s\n",
		((strwcmpi(pattern, string) == 0) ? "SUCCEEDED" : "FAILED"));
    }

    return 0;
}
#endif
