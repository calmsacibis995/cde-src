/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: file.c /main/cde1_maint/2 1995/10/09 00:09:53 pascale $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * file.c
 */

# include	"dm.h"
# include	"vgmsg.h"
# include	<ctype.h>
# include	<pwd.h>

# include	<sys/socket.h>
# include	<netinet/in.h>
# include	<netdb.h>



/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static int  DisplayTypeMatch() ;
static void freeArgs() ;
static void freeSomeArgs() ;
static DisplayType parseDisplayType() ;
static char ** splitIntoWords() ;
static char ** copyArgs() ;

#else

static int DisplayTypeMatch( DisplayType d1, DisplayType d2) ;
static void freeArgs( char **args) ;
static void freeSomeArgs( char **args, int n) ;
static DisplayType parseDisplayType( char *string, int *usedDefaultType, int *parse_uid) ;
static char ** splitIntoWords( char *s) ;
static char ** copyArgs( char **args) ;

#endif /* _NO_PROTO */



/***************************************************************************
 *
 *  main
 *
 ***************************************************************************/

static int 
#ifdef _NO_PROTO
DisplayTypeMatch( d1, d2 )
        DisplayType d1 ;
        DisplayType d2 ;
#else
DisplayTypeMatch( DisplayType d1, DisplayType d2 )
#endif /* _NO_PROTO */
{
	return d1.location == d2.location &&
	       d1.lifetime == d2.lifetime &&
	       d1.origin == d2.origin;
}

static void 
#ifdef _NO_PROTO
freeArgs( args )
        char **args ;
#else
freeArgs( char **args )
#endif /* _NO_PROTO */
{
    char    **a;

    for (a = args; *a; a++)
	free (*a);
    free ((char *) args);
}

static char ** 
#ifdef _NO_PROTO
splitIntoWords( s )
        char *s ;
#else
splitIntoWords( char *s )
#endif /* _NO_PROTO */
{
    char    **args, **newargs;
    char    *wordStart;
    int	    nargs;

    args = 0;
    nargs = 0;
    while (*s)
    {
	while (*s && isspace (*s))
	    ++s;
	if (!*s || *s == '#')
	    break;
	wordStart = s;
	while (*s && *s != '#' && !isspace (*s))
	    ++s;
	if (!args)
	{
    	    args = (char **) malloc (2 * sizeof (char *));
    	    if (!args)
	    	return NULL;
	}
	else
	{
	    newargs = (char **) realloc ((char *) args,
					 (nargs+2)*sizeof (char *));
	    if (!newargs)
	    {
	    	freeArgs (args);
	    	return NULL;
	    }
	    args = newargs;
	}
	args[nargs] = malloc (s - wordStart + 1);
	if (!args[nargs])
	{
	    freeArgs (args);
	    return NULL;
	}
	strncpy (args[nargs], wordStart, s - wordStart);
	args[nargs][s-wordStart] = '\0';
	++nargs;
	args[nargs] = NULL;
    }
    return args;
}

static char ** 
#ifdef _NO_PROTO
copyArgs( args )
        char **args ;
#else
copyArgs( char **args )
#endif /* _NO_PROTO */
{
    char    **a, **new, **n;

    for (a = args; *a; a++)
	;
    new = (char **) malloc ((a - args + 1) * sizeof (char *));
    if (!new)
	return NULL;
    n = new;
    a = args;
    while (*n++ = *a++)
	;
    return new;
}

static void 
#ifdef _NO_PROTO
freeSomeArgs( args, n )
        char **args ;
        int n ;
#else
freeSomeArgs( char **args, int n )
#endif /* _NO_PROTO */
{
    char    **a;

    a = args;
    while (n--)
	free (*a++);
    free ((char *) args);
}

void
#ifdef _NO_PROTO
ParseDisplay( source, acceptableTypes, numAcceptable, puser)
        char *source ;
        DisplayType *acceptableTypes ;
        int numAcceptable ;
	struct passwd *puser;
#else
ParseDisplay( char *source, 
	      DisplayType *acceptableTypes, 
	      int numAcceptable,
	      struct passwd *puser)
#endif /* _NO_PROTO */
{
    char		**args, **argv, **a;
    char		*name, *class, *type;
    struct display	*d;
    int			usedDefaultType;
    int			parse_uid;
    DisplayType		displayType;

    char		*device=NULL; /* ITE device associated with display */


    args = splitIntoWords (source);
    if (!args)
	return;
    if (!args[0])
    {
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_MISS_NAME,MC_DEF_LOG_MISS_NAME));
	freeArgs (args);
	return;
    }
    name = args[0];
    if (!args[1])
    {
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_MISS_TYPE,MC_DEF_LOG_MISS_TYPE),
		args[0]);
	freeArgs (args);
	return;
    }

    /*
     *  strip off display device if found in second field...
     */

    if ( (device = strchr(args[1],'@')) != NULL) {
	*device = '\0';
	device++;
    }
    	
    displayType = parseDisplayType (args[1], &usedDefaultType, &parse_uid);
    class = NULL;
    type = args[1];
    argv = args + 2;

    /*
     *  check for special syntax "*" and expand to host name.
     *  if hostname cannot be found in a database, assume invalid and
     *  delete.
     */
    if ( strcmp(name, "*") == 0) {
	char 		tname[128];
	struct hostent	*hostent;

	strcpy(tname,"");
	gethostname(tname, sizeof(tname));
	if ( (hostent = gethostbyname(tname)) == NULL ) {
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_INV_HOSTNM,MC_DEF_LOG_INV_HOSTNM),
		      tname);
	    strcpy(tname,"");
	}
/*
	else
	    strcpy(tname,hostent->h_name);
*/

	strcat(tname, ":0");
	
	name = tname;
    }

    /*
     * extended syntax; if the second argument doesn't
     * exactly match a legal display type and the third
     * argument does, use the second argument as the
     * display class string
     */
    if (usedDefaultType && args[2])
    {

	/*
	 *  strip off display device if found in third field...
	 */

	if ( device == NULL && (device = strchr(args[2],'@')) != NULL) {
	    *device = '\0';
	    device++;
	}

	displayType = parseDisplayType (args[2], &usedDefaultType, &parse_uid);
	if (!usedDefaultType)
	{
	    class = args[1];
	    type = args[2];
	    argv = args + 3;
	}
    }
    /*
     * extended syntax; if the display type argument was 
     * "local_uid", then next argument is pseudo user id
     * under which the local Xserver is to be run.
     */
    if (parse_uid) {
	struct passwd *p;

        Debug("Xservers 'local_uid' pseudo user = %s\n", *argv);

        if ( (p = getpwnam (*argv)) != NULL) {
	    *puser = *p;
	} else {
	    Debug("Could not get password entry for user name '%s'\n", *argv);
	    Debug("Using default pseudo user = %s\n", puser->pw_name);
	}

	argv = argv + 1;
    } else {
        Debug("Default pseudo user = %s\n", puser->pw_name);
    }

    while (numAcceptable)
    {
	if (DisplayTypeMatch (*acceptableTypes, displayType))
	    break;
	--numAcceptable;
	++acceptableTypes;
    }
    if (!numAcceptable)
    {
	LogError(ReadCatalog(
		MC_LOG_SET,MC_LOG_BAD_DPYTYPE,MC_DEF_LOG_BAD_DPYTYPE),
		  type, name);
    }


    /*
     *  see if this display is already being managed...
     */

    d = FindDisplayByName (name);
    if (d)
    {
	d->state = OldEntry;
	if (class && strcmp (d->class, class))
	{
	    char    *newclass;

	    newclass = malloc ((unsigned) (strlen (class) + 1));
	    if (newclass)
	    {
		free (d->class);
		strcpy (newclass, class);
		d->class = newclass;
	    }
	}
	Debug ("Found existing display:  %s %s %s", d->name, d->class ? d->class : "", type);
	if (d->argv) freeArgs (d->argv);
    }
    else
    {
	char *colon=strstr(name, ":");

	d = NewDisplay (name, class);
	Debug ("Found new display:  %s %s %s", d->name, d->class ? d->class : "", type);

	if (colon != NULL) {
	    d->displayNumber= atoi(++colon);
	}

    }

    d->displayType = displayType;
    d->argv = copyArgs (argv);
    for (a = d->argv; a && *a; a++)
	Debug (" %s", *a);
    Debug ("\n");

    /*
     * add device to display information...
     */
     
    if ( device != NULL && strlen(device) != 0 ) {
	if (d->gettyLine != NULL)
	    free(d->gettyLine);

	d->gettyLine = strdup(device);

    }

    if (d->gettyLine && 
        (strcmp(d->gettyLine, "None") == 0 ||
         strcmp(d->gettyLine, "none") == 0   ) ) {

	strcpy(d->gettyLine,"??");
    }


    freeSomeArgs (args, argv - args);
}

static struct displayMatch {
	char		*name;
	DisplayType	type;
} displayTypes[] = {
	"local",		{ Local, Permanent, FromFile },
	"local_uid",		{ Local, Permanent, FromFile },
	"foreign",		{ Foreign, Permanent, FromFile },
	0,			{ Local, Permanent, FromFile },
};

static DisplayType 
#ifdef _NO_PROTO
parseDisplayType( string, usedDefaultType, parse_uid )
        char *string ;
        int *usedDefaultType ;
        int *parse_uid;
#else
parseDisplayType( char *string, int *usedDefaultType, int *parse_uid )
#endif /* _NO_PROTO */
{
	struct displayMatch	*d;

        *parse_uid = 0;

	for (d = displayTypes; d->name; d++) {
	    if (strcmp(d->name, string) == 0)
	    {
		if (strcmp(d->name, "local_uid") == 0) {
		   *parse_uid = 1; 
		}
		*usedDefaultType = 0;
		return d->type;
	    }
        }

	*usedDefaultType = 1;
	return d->type;
}
