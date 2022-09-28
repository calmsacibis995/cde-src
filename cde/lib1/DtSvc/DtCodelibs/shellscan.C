/*
 * File:	shellscan.C $XConsortium: shellscan.C /main/cde1_maint/2 1995/10/08 22:09:44 pascale $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
# if defined(apollo) && !defined(___GID_T)
// This kludge is needed for the include conflicts mentioned below
// Remove when no longer necessary
#  define _NEED___PID_T
# endif

#ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#include "/usr/include/apollo/shlib.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <codelibs/nl_hack.h>

#ifdef apollo
// This kludge because of include conflicts between stdlib.h and unistd.h
// Remove when problem is fixed

# ifdef __cplusplus
    extern "C" {
# endif
    _DECL_FUNC(__pid_t,    getpid,   (void))
# ifdef __cplusplus
    }
# endif
#else /* not apollo */
# include <unistd.h>
#endif /* not apollo */

#include "stringio.h"
#include "buf.h"
#include "shellutils.h"

#include <codelibs/boolean.h>
#include <codelibs/stringx.h>

#define ISIDENT(CH) (isalnum(CH) || (CH) == '_')

static _SHXbuf *buf = NULL;
static const char *getvar(const char *var);

// Make this a global someday:
static const char *(*shellvarfn)(const char *) = getvar;

// Parse a sequence of the ksh meta-characters ;&|<> and whitespace
// into a single ksh token.  Return a pointer to the token as a
// string.  All whitespace characters are mapped to a single space
// character.  If ch is not a meta-character, return a NULL pointer.
static char *
parsemeta(int ch, _StringIO &in, char *ifs, unsigned opts)
{
    if (ch == '\0')
	return " ";		// whitespace

    if (buf->quote() != NOQUOTE)
	return NULL;		// normal character

    if (!(opts & SHX_NOSPACE) && strchr(ifs, ch) != NULL)
	return " ";		// whitespace

    if (!(opts & SHX_NOMETA))
    {
	int len = 0;
	static char meta[4];

	if (buf->new_token() && isascii(ch) && isdigit(ch))
	    if (in.next() == '<' || in.next() == '>')
	    {
		meta[len++] = (char)ch;
		ch = in.get();
	    }

	switch (meta[len++] = (char)ch, ch)
	{
	case ';': 
	case '&': 
	case '(': 
	case ')': 
	    meta[len++] = (in.next() == ch) ? in.get() : '\0';
	    meta[len] = '\0';
	    return meta;
	case '|': 
	    meta[len++] = (in.next() == '|' || in.next() == '&') ?
		in.get() : '\0';
	    meta[len] = '\0';
	    return meta;
	case '>': 
	case '<': 
		if (in.next() == ch || in.next() == '&')
		    meta[len++] = (char)in.get();
		meta[len] = '\0';
	    return meta;
	}
    }

    return NULL; // normal character
}

// Takes the name of a variable, and looks up it's value.  Someday,
// this will be replaceable by the user.
static const char *
getvar(const char *name)
{
    static char buf[10];

    if (name[0] != '\0' && name[1] == '\0')
	switch (name[0])
	{
	case '$': 
	    sprintf(buf, "%d", getpid());
	    return buf;
	case '#':
	case '?':
	    return "0";
	}

    return getenv(name);
}

// Parse an environment variable name from the _StringIO stream,
// and push its value into the _StringIO stream stack.
static boolean
pushvar(_StringIO &in)
{
    _StringIO tmp;
    privbuf_charbuf name;

    tmp = in;
    int ch = tmp.get();		// get the first character after the $

    if (!isascii(ch))
	return FALSE;

    if (ch == '{')
	while ((ch = tmp.get()) != '\0')
	{
	    // ${foo!bar} form, grab everything inside {} as name
	    if (ch == '\\')	// Only \ does quoting inside ${}
		    ch = tmp.get();
	    else if (ch == '}')
		break;
	    name.end() = ch;
	}
    else if (ispunct(ch))
	switch (ch)		// Special non-alnum shell variables
	{
	case '#': 
	case '?': 
	case '$': 
	case '!': 
	case '-': 
	case '*': 
	case '@': 
	case '_':
	    name.end() = ch;
	    break;
	default: 
	    return FALSE;
	}
    else if (isdigit(ch))
	name.end() = ch;	// single-digit variables
    else if (ISIDENT(ch))
    {
	// normal variable
	do
	    name.end() = ch;
	while (isascii(ch = tmp.get()) && ISIDENT(ch));
	tmp.unget();
    }
    else
	return FALSE;

    name.end() = '\0';

    in = tmp;
    in.push(shellvarfn(name.getarr()));
    return TRUE;
}

static boolean
pushenv(_StringIO &in, char const *name)
{
    register char *str = getenv(name);
    if (str == NULL || *str == '\0')
	return FALSE;
    else
    {
	in.push(str);
	return TRUE;
    }
}

static boolean
pushtilde(_StringIO &in)
{
    _StringIO tmp;
    int namelen = 0;
    privbuf_charbuf name;

    tmp = in;
    int ch;

    while ((ch = tmp.get()) != '\0' && ch != '/')
	name[namelen++] = ch;
    name[namelen] = '\0';
    tmp.unget();

    char *str = name.getarr();
    switch (*str)
    {
    case '\0': 
	if (!pushenv(tmp, "HOME"))
	    return FALSE;
	break;
    case '+': 
	if (!pushenv(tmp, "PWD"))
	    return FALSE;
	break;
    case '-': 
	if (!pushenv(tmp, "OLDPWD"))
	    return FALSE;
	break;
    default: 
	{
	    passwd *pwent = getpwnam(str);
	    if (pwent == NULL)
		return FALSE;
	    tmp.push(pwent->pw_dir);
	}
	break;
    }

    in = tmp;
    return TRUE;
}

void
pushgrave(_StringIO &in, const char endchar, boolean quotes)
{
    int ch;
    char quote = NOQUOTE;
    privbuf_charbuf cmd;

    do
    {
	ch = in.get();

	if (quotes)
	    switch (ch)
	    {
	    case '"': 
		quote = DOUBLEQUOTE;
		continue;
	    case '\'': 
		if (quote == '"')
		    break;	// not recognized inside of ""
		do
		    cmd.end() = ch;
		while ((ch = in.get()) != '\'' && ch != '\0');
		cmd.end() = '\'';
		quote = NOQUOTE;
		continue;
	    case '\\': 
		cmd.end() = ch;
		ch = in.get();
		if (ch != '\0')
		    cmd.end() = ch;
		continue;
	    }

	if (ch == endchar)
	    ch = '\0';

	cmd.end() = ch;
    } while (ch != '\0');

    static privbuf_charbuf result;
    result.reset();

    FILE *fp = popen(cmd.getarr(), "r");
    if (fp == NULL)
	return;
    while ((ch = getc(fp)) != EOF)
	result.end() = ch;
    pclose(fp);

    // Remove trailing newline, if any
    long end = result.size() - 1;
    if (result[end] == '\n')
	result.reset(end);

    result.end() = '\0';
    in.push(result.getarr());
}

char const *const *
shellscan(char const *str, int *argc, unsigned opts)
{
    if (opts & SHX_COMPLETE)
	opts |= SHX_NOSPACE | SHX_NOMETA;

    char *ifs = getenv("IFS");

    if (ifs == NULL)
	ifs = " \t\n";

    if (buf == NULL)
	buf = new _SHXbuf;
    buf->reset((boolean)!(opts & SHX_NOGLOB), (boolean)(opts & SHX_COMPLETE));

    _StringIO in(str);
    int ch;

    do
    {
	ch = in.get();

	// Don't recognize special characters if this is a shell
	// variable or command substitution.
	if (!in.in_expansion())
	{
	    // Handle quoting rules, setting the flag array and
	    // quote variable appropriately.
	    if (!(opts & SHX_NOQUOTES))
		switch (ch)
		{
		case '"': 
		    buf->quote(DOUBLEQUOTE);
		    continue;
		case '\'': 
		    if (buf->quote() == DOUBLEQUOTE)
			break;	// not recognized inside of ""
		    buf->quote(SINGLEQUOTE);
		    while ((ch = in.get()) != '\'' && ch != '\0')
			buf->append(ch);
		    buf->quote(SINGLEQUOTE);
		    continue;
		case '\\': 
		    ch = in.get();
		    if (ch == '\n') // ignore \<newline>
			continue;
		    if (ch == '\0')
		    {
#if defined(__aix)  /* Our Macro doesn't like '\\' (ignores rest of line) */
			buf->append('\\',
                           SINGLEQUOTE);
#else
			buf->append('\\', SINGLEQUOTE);
#endif
			break;
		    }
		    if (buf->quote() == NOQUOTE)
		    {
			buf->append(ch, SINGLEQUOTE);
			continue;
		    }
		    else
		    {
			// inside "", \ only quotes these 4 characters:
			switch (ch)
			{
			case '$': 
			case '\\': 
			case '`': 
			case '"': 
			    buf->append(ch, SINGLEQUOTE);
			    continue;
			default: 
			    // treat the \ and the following char normally
			    buf->append('\\');
			    break;
			}
		    }
		    break;
		}

	    if (!(opts & SHX_NOCMD))
		switch (ch)
		{
		case '`': 
		    pushgrave(in, '`', (boolean)!(opts & SHX_NOQUOTES));
		    continue;
		case '$': 
		    if (in.next() != '(')
			break;
		    in.get();	// skip the '('
		    pushgrave(in, ')', (boolean)!(opts & SHX_NOQUOTES));
		    continue;
		}

	    if (ch == '~' && buf->new_token() && buf->quote() == NOQUOTE)
		if (!(opts & SHX_NOTILDE))
		{
		    if (pushtilde(in))
			continue;
		    buf->append('~');
		    continue;
		}

	    if (ch == '$' && !(opts & SHX_NOVARS))
	    {
		if (pushvar(in))
		    continue;
		buf->append('$');
		continue;
	    }
	}

	// If the next item is an unquoted whitespace character or
	// metacharacter token, terminate the current token.  The NUL
	// character is considered to be whitespace.
	{
	    int curr_opts = opts;

	    if (in.in_expansion())
		curr_opts |= SHX_NOMETA;

	    char *meta = parsemeta(ch, in, ifs, curr_opts);

	    if (meta != NULL)	// is it a meta-character?
	    {
		// Terminate current token, if any
		if (!buf->new_token())
		    buf->append('\0');
		if (*meta == ' ')   // whitespace
		{
		    // ignore contiguous whitespace chars
		    if (buf->new_token())
			continue;
		}
		else		// append the metachar token
		    buf->append(meta);
		continue;
	    }
	}

	buf->append(ch);
    } while (ch != '\0');

    if (argc != NULL)
	*argc = buf->ntokens();

    return ( (char const *const *) buf->vector() );
/* !!! error 1325: `)' missing at end of input */
}
