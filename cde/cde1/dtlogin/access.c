/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * $XConsortium: access.c /main/cde1_maint/2 1995/10/18 17:30:07 mgreess $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Access control for XDMCP - keep a database of allowable display addresses
 * and (potentially) a list of hosts to send ForwardQuery packets to
 */

# include   "dm.h"
# include   "vgmsg.h"

# include   <X11/Xos.h>
# include   <X11/Xdmcp.h>
# include   <X11/X.h>
# include   <stdio.h>
# include   <netinet/in.h>
# include   <netdb.h>
# include   <sys/socket.h>
# include   <ctype.h>

#define ALIAS_CHARACTER	    '%'
#define NEGATE_CHARACTER    '!'
#define CHOOSER_STRING	    "CHOOSER"
#define BROADCAST_STRING    "BROADCAST"
#ifdef BYPASSLOGIN
#define BYPASS_STRING	    "BYPASS_LOGIN"
#endif /* BYPASSLOGIN */

#define HOST_ALIAS	0
#define HOST_ADDRESS	1
#define HOST_BROADCAST	2
#define HOST_CHOOSER	3
#ifdef BYPASSLOGIN
#define HOST_BYPASS	4
#endif /* BYPASSLOGIN */

typedef struct _hostEntry {
    struct _hostEntry	*next;
    int	    type;
    union _hostOrAlias {
	char	*aliasName;
	ARRAY8	hostAddress;
    } entry;
} HostEntry;

#define DISPLAY_ALIAS	    0
#define DISPLAY_PATTERN	    1
#define DISPLAY_ADDRESS	    2

typedef struct _displayEntry {
    struct _displayEntry    *next;
    int			    type;
    int			    notAllowed;
    int			    chooser;
#ifdef BYPASSLOGIN
    int			    bypass;
#endif /* BYPASSLOGIN */
    union _displayType {
	char		    *aliasName;
	char		    *displayPattern;
	struct _display {
	    ARRAY8	    clientAddress;
	    CARD16	    connectionType;
	} displayAddress;
    } entry;
    HostEntry		    *hosts;
} DisplayEntry;


/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static void FreeHostEntry() ;
static void FreeDisplayEntry() ;
static void FreeAccessDatabase() ;
static char * ReadWord() ;
static HostEntry * ReadHostEntry() ;
static int HasGlobCharacters() ;
static DisplayEntry * ReadDisplayEntry() ;
static void ReadAccessDatabase() ;
static int scanHostlist() ;
static int patternMatch() ;
static int indirectAlias() ;

#else

static void FreeHostEntry( 
                        HostEntry *h) ;
static void FreeDisplayEntry( 
                        DisplayEntry *d) ;
static void FreeAccessDatabase( void ) ;
static char * ReadWord( 
                        FILE *file,
                        int EOFatEOL) ;
static HostEntry * ReadHostEntry( 
                        FILE *file) ;
static int HasGlobCharacters( 
                        char *s) ;
static DisplayEntry * ReadDisplayEntry( 
                        FILE *file) ;
static void ReadAccessDatabase( 
                        FILE *file) ;
static int scanHostlist( 
                        HostEntry *h,
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        void (*function)(),
                        char *closure,
                        int depth,
                        int broadcast) ;
static int patternMatch( 
                        char *string,
                        char *pattern) ;
static int indirectAlias( 
                        char *alias,
                        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
                        int connectionType,
#else
                        CARD16 connectionType,
#endif /* NeedWidePrototypes */
                        void (*function)(),
                        char *closure,
                        int depth,
                        int broadcast) ;


#endif /* _NO_PROTO */




/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

static DisplayEntry	*database = 0;

static ARRAY8		localAddress;



/***************************************************************************
 ***************************************************************************/

static void
#ifdef _NO_PROTO
FreeHostEntry( h )
        HostEntry *h ;
#else
FreeHostEntry(
        HostEntry *h )
#endif /* _NO_PROTO */
{
    switch (h->type) {
#ifdef BYPASSLOGIN
    case HOST_BYPASS:
#endif /* BYPASSLOGIN */
    case HOST_ALIAS:
	free (h->entry.aliasName);
	break;
    case HOST_ADDRESS:
	XdmcpDisposeARRAY8 (&h->entry.hostAddress);
	break;
    case HOST_CHOOSER:
	break;
    }
    free ((char *) h);
}

static void
#ifdef _NO_PROTO
FreeDisplayEntry( d )
        DisplayEntry *d ;
#else
FreeDisplayEntry(
        DisplayEntry *d )
#endif /* _NO_PROTO */
{
    HostEntry	*h, *next;
    switch (d->type) {
    case DISPLAY_ALIAS:
	free (d->entry.aliasName);
	break;
    case DISPLAY_PATTERN:
	free (d->entry.displayPattern);
	break;
    case DISPLAY_ADDRESS:
	XdmcpDisposeARRAY8 (&d->entry.displayAddress);
	break;
    }
    for (h = d->hosts; h; h = next) {
	next = h->next;
	FreeHostEntry (h);
    }
    free ((char *) d);
}

static void 
#ifdef _NO_PROTO
FreeAccessDatabase()
#else
FreeAccessDatabase( void )
#endif /* _NO_PROTO */
{
    DisplayEntry    *d, *next;

    for (d = database; d; d = next)
    {
	next = d->next;
	FreeDisplayEntry (d);
    }
    database = 0;
}

#define WORD_LEN    256
static char	wordBuffer[WORD_LEN];
static int	nextIsEOF;

static char * 
#ifdef _NO_PROTO
ReadWord( file, EOFatEOL )
        FILE *file ;
        int EOFatEOL ;
#else
ReadWord(
        FILE *file,
        int EOFatEOL )
#endif /* _NO_PROTO */
{
    int	    c;
    char    *wordp;
    int	    quoted;

    wordp = wordBuffer;
    if (nextIsEOF)
    {
	nextIsEOF = FALSE;
	return NULL;
    }
    quoted = FALSE;
    for (;;) {
	c = getc (file);
	switch (c) {
	case '#':
	    if (quoted)
	    {
		*wordp++ = c;
		break;
	    }
	    while ((c = getc (file)) != EOF && c != '\n')
		;
	case '\n':
	case EOF:
	    if (c == EOF || (EOFatEOL && !quoted))
	    {
		ungetc (c, file);
		if (wordp == wordBuffer)
		    return NULL;
		*wordp = '\0';
		nextIsEOF = TRUE;
		return wordBuffer;
	    }
	case ' ':
	case '\t':
	    if (wordp != wordBuffer)
	    {
		ungetc (c, file);
		*wordp = '\0';
		return wordBuffer;
	    }
	    break;
	case '\\':
	    if (!quoted)
	    {
		quoted = TRUE;
		continue;
	    }
	default:
	    *wordp++ = c;
	    break;
	}
	quoted = FALSE;
    }
}

static HostEntry * 
#ifdef _NO_PROTO
ReadHostEntry( file )
        FILE *file ;
#else
ReadHostEntry(
        FILE *file )
#endif /* _NO_PROTO */
{
    char	    *hostOrAlias;
    HostEntry	    *h;
    struct hostent  *hostent;

tryagain:
    hostOrAlias = ReadWord (file, TRUE);
    if (!hostOrAlias)
	return NULL;
    h = (HostEntry *) malloc (sizeof (DisplayEntry));
    if (*hostOrAlias == ALIAS_CHARACTER)
    {
	h->type = HOST_ALIAS;
	h->entry.aliasName = malloc (strlen (hostOrAlias) + 1);
	if (!h->entry.aliasName) {
	    free ((char *) h);
	    return NULL;
	}
	strcpy (h->entry.aliasName, hostOrAlias);
    }
    else if (!strcmp (hostOrAlias, CHOOSER_STRING))
    {
	h->type = HOST_CHOOSER;
    }
    else if (!strcmp (hostOrAlias, BROADCAST_STRING))
    {
	h->type = HOST_BROADCAST;
    }
#ifdef BYPASSLOGIN
    else if (!strcmp (hostOrAlias, BYPASS_STRING))
    {
 	h->type = HOST_BYPASS;
	hostOrAlias = ReadWord (file, TRUE);
	if (!hostOrAlias)
 	{
	    Debug ("No username specified for login bypass.\n");
	    LogError ((unsigned char *) "Access file \"%s\", No username "
		      "specified for login bypass\n", accessFile);
	    free ((char *) h);
	    goto tryagain;
 	}
	if (!strcmp (hostOrAlias, "root"))
	{
	    LogError ((unsigned char *)
		      "Access file \"%s\", root bypass disallowed\n",
		      accessFile);
	    free ((char *) h);
	    return NULL;
	}
 	h->entry.aliasName = malloc (strlen (hostOrAlias) + 1);
 	if (!h->entry.aliasName) {
	    free ((char *) h);
	    return NULL;
 	}
 	strcpy (h->entry.aliasName, hostOrAlias);
    }
#endif	/* BYPASSLOGIN */
    else
    {
	h->type = HOST_ADDRESS;
	hostent = gethostbyname (hostOrAlias);
	if (!hostent)
	{
	    Debug ("No such host %s\n", hostOrAlias);
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_ACC_FILE,MC_DEF_LOG_ACC_FILE),
		accessFile,hostOrAlias);
	    free ((char *) h);
	    goto tryagain;
	}
	if (!XdmcpAllocARRAY8 (&h->entry.hostAddress, hostent->h_length))
	{
	    LogOutOfMem(
		ReadCatalog(MC_LOG_SET,MC_LOG_HOST_ENT,MC_DEF_LOG_HOST_ENT));
	    free ((char *) h);
	    return NULL;
	}
	bcopy (hostent->h_addr, (char *) h->entry.hostAddress.data, hostent->h_length);
    }
    return h;
}

static int 
#ifdef _NO_PROTO
HasGlobCharacters( s )
        char *s ;
#else
HasGlobCharacters(
        char *s )
#endif /* _NO_PROTO */
{
    for (;;)
	switch (*s++) {
	case '?':
	case '*':
	    return 1;
	case '\0':
	    return 0;
	}
}

static DisplayEntry * 
#ifdef _NO_PROTO
ReadDisplayEntry( file )
        FILE *file ;
#else
ReadDisplayEntry(
        FILE *file )
#endif /* _NO_PROTO */
{
    char	    *displayOrAlias;
    DisplayEntry    *d;
    struct _display *display;
    HostEntry	    *h, **prev;
    struct hostent  *hostent;
    
    displayOrAlias = ReadWord (file, FALSE);
    if (!displayOrAlias)
    	return NULL;
    d = (DisplayEntry *) malloc (sizeof (DisplayEntry));
    d->notAllowed = 0;
    d->chooser = 0;
#ifdef BYPASSLOGIN
    d->bypass = 0;
#endif /* BYPASSLOGIN */
    if (*displayOrAlias == ALIAS_CHARACTER)
    {
	d->type = DISPLAY_ALIAS;
	d->entry.aliasName = malloc (strlen (displayOrAlias) + 1);
	if (!d->entry.aliasName)
	{
	    free ((char *) d);
	    return NULL;
	}
	strcpy (d->entry.aliasName, displayOrAlias);
    }
    else
    {
	if (*displayOrAlias == NEGATE_CHARACTER)
	{
	    d->notAllowed = 1;
	    ++displayOrAlias;
	}
    	if (HasGlobCharacters (displayOrAlias))
    	{
	    d->type = DISPLAY_PATTERN;
	    d->entry.displayPattern = malloc (strlen (displayOrAlias) + 1);
	    if (!d->entry.displayPattern)
	    {
	    	free ((char *) d);
	    	return NULL;
	    }
	    strcpy (d->entry.displayPattern, displayOrAlias);
    	}
    	else
    	{
	    if ((hostent = gethostbyname (displayOrAlias)) == NULL)
	    {
		LogError(
		  ReadCatalog(MC_LOG_SET,MC_LOG_ACC_DPY,MC_DEF_LOG_ACC_DPY),
		  accessFile,displayOrAlias);
		free ((char *) d);
		return NULL;
	    }
	    d->type = DISPLAY_ADDRESS;
	    display = &d->entry.displayAddress;
	    if (!XdmcpAllocARRAY8 (&display->clientAddress, hostent->h_length))
	    {
	    	free ((char *) d);
	    	return NULL;
	    }
	    bcopy (hostent->h_addr, (char *) display->clientAddress.data, hostent->h_length);
	    switch (hostent->h_addrtype)
	    {
#ifdef AF_UNIX
	    case AF_UNIX:
	    	display->connectionType = FamilyLocal;
	    	break;
#endif
#ifdef AF_INET
	    case AF_INET:
	    	display->connectionType = FamilyInternet;
	    	break;
#endif
#ifdef AF_DECnet
	    case AF_DECnet:
	    	display->connectionType = FamilyDECnet;
	    	break;
#endif
	    default:
	    	display->connectionType = FamilyLocal;
	    	break;
	    }
    	}
    }
    prev = &d->hosts;
    while (h = ReadHostEntry (file))
    {
	if (h->type == HOST_CHOOSER)
	{
	    FreeHostEntry (h);
	    d->chooser = 1;
	} else {
	    *prev = h;
	    prev = &h->next;
	}
#ifdef BYPASSLOGIN
	if (h->type == HOST_BYPASS)
	{
	    d->bypass = 1;
	}
#endif /* BYPASSLOGIN */
    }
    *prev = NULL;
    return d;
}

static void
#ifdef _NO_PROTO
ReadAccessDatabase( file )
        FILE *file ;
#else
ReadAccessDatabase(
        FILE *file )
#endif /* _NO_PROTO */
{
    DisplayEntry    *d, **prev;

    prev = &database;
    while (d = ReadDisplayEntry (file))
    {
	*prev = d;
	prev = &d->next;
    }
    *prev = NULL;
}

int 
#ifdef _NO_PROTO
ScanAccessDatabase()
#else
ScanAccessDatabase( void )
#endif /* _NO_PROTO */
{
    FILE	*datafile;

    FreeAccessDatabase ();
    if (accessFile && strlen(accessFile) > 0)
    {
    	datafile = fopen (accessFile, "r");
    	if (!datafile)
	{
	    LogError(ReadCatalog(MC_LOG_SET,MC_LOG_ACC_CTL,MC_DEF_LOG_ACC_CTL),
		accessFile);
	    return 0;
	}
	ReadAccessDatabase (datafile);
	fclose (datafile);
    }
    return 1;
}


static int 
#ifdef _NO_PROTO
patternMatch( string, pattern )
        char *string ;
        char *pattern ;
#else
patternMatch(
        char *string,
        char *pattern )
#endif /* _NO_PROTO */
{
    int	    p, s;

    if (!string)
        string = "";

    for (;;)
    {
	s = *string++;
	switch (p = *pattern++) {
	case '*':
	    if (!*pattern)
		return TRUE;
	    for (string--; *string; string++)
		if (patternMatch (string, pattern))
		    return 1;
	    return 0;
	case '?':
	    if (s == 0)
		return 0;
	    break;
	case '\0':
	    return s == 0;
	case '\\':
	    p = *pattern++;
	default:
	    if (p != s)
		return 0;
	}
    }
}


ARRAY8Ptr
#ifdef _NO_PROTO
getLocalAddress()
#else
getLocalAddress( void )
#endif /* _NO_PROTO */
{
    static int	haveLocalAddress;
    
    if (!haveLocalAddress)
    {
	struct hostent	*hostent;

	hostent = gethostbyname (localHostname());
	XdmcpAllocARRAY8 (&localAddress, hostent->h_length);
	bcopy (hostent->h_addr, (char *) localAddress.data, hostent->h_length);
    }
    return &localAddress;
}


/*
 * calls the given function for each valid indirect entry.  Returns TRUE if
 * the local host exists on any of the lists, else FALSE
 */

#define MAX_DEPTH   32

static int indirectAlias ();

static int 
#ifdef _NO_PROTO
scanHostlist( h, clientAddress, connectionType, function, closure, depth, broadcast )
        HostEntry *h ;
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
        void (*function)() ;
        char *closure ;
        int depth ;
        int broadcast ;
#else
scanHostlist(
        HostEntry *h,
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        void (*function)(),
        char *closure,
        int depth,
        int broadcast )
#endif /* _NO_PROTO */
{
    int	haveLocalhost = 0;

    for (; h; h = h->next)
    {
	switch (h->type) {
	case HOST_ALIAS:
	    if (indirectAlias (h->entry.aliasName, clientAddress,
			       connectionType, function, closure, depth,
			       broadcast))
		haveLocalhost = 1;
	    break;
	case HOST_ADDRESS:
	    if (XdmcpARRAY8Equal (getLocalAddress(), &h->entry.hostAddress))
		haveLocalhost = 1;
	    else if (function)
		(*function) (connectionType, &h->entry.hostAddress, closure);
	    break;
	case HOST_BROADCAST:
	    if (broadcast)
	    {
		ARRAY8	temp;

		if (function)
		{
		    temp.data = (BYTE *) BROADCAST_STRING;
		    temp.length = strlen ((char *)temp.data);
		    (*function) (connectionType, &temp, closure);
		}
	    }
	    break;
	}
    }
    return haveLocalhost;
}


static int 
#ifdef _NO_PROTO
indirectAlias( alias, clientAddress, connectionType, function, closure, depth, broadcast )
        char *alias ;
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
        void (*function)() ;
        char *closure ;
        int depth ;
        int broadcast ;
#else
indirectAlias(
        char *alias,
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        void (*function)(),
        char *closure,
        int depth,
        int broadcast )
#endif /* _NO_PROTO */
{
    DisplayEntry    *d;
    int		    haveLocalhost = 0;

    if (depth == MAX_DEPTH)
	return 0;
    for (d = database; d; d = d->next)
    {
	if (d->type != DISPLAY_ALIAS || !patternMatch (alias, d->entry.aliasName))
	    continue;
	if (scanHostlist (d->hosts, clientAddress, connectionType,
			  function, closure, depth + 1, broadcast))
	{
	    haveLocalhost = 1;
	}
    }
    return haveLocalhost;
}


ARRAY8Ptr IndirectChoice ();

int 
#ifdef _NO_PROTO
ForEachMatchingIndirectHost( clientAddress, connectionType, function, closure )
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
        void (*function)() ;
        char *closure ;
#else
ForEachMatchingIndirectHost(
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        void (*function)(),
        char *closure )
#endif /* _NO_PROTO */
{
    int		    haveLocalhost = 0;
    DisplayEntry    *d;
    char	    *clientName = 0, *NetworkAddressToHostname ();

    for (d = database; d; d = d->next)
    {
    	switch (d->type) {
    	case DISPLAY_ALIAS:
	    continue;
    	case DISPLAY_PATTERN:
	    if (!clientName)
		clientName = NetworkAddressToHostname (connectionType,
						       clientAddress);
	    if (!patternMatch (clientName, d->entry.displayPattern))
		continue;
	    break;
    	case DISPLAY_ADDRESS:
	    if (d->entry.displayAddress.connectionType != connectionType ||
	    	!XdmcpARRAY8Equal (&d->entry.displayAddress.clientAddress,
				  clientAddress))
	    {
		continue;
	    }
	    break;
    	}
	if (!d->hosts)
	    continue;
	if (d->notAllowed)
	    break;
#ifdef BYPASSLOGIN
	if (d->bypass)
	    break;
#endif /* BYPASSLOGIN */
	if (d->chooser)
	{
	    ARRAY8Ptr	choice;

	    choice = IndirectChoice (clientAddress, connectionType);
	    if (!choice || XdmcpARRAY8Equal (getLocalAddress(), choice))
		haveLocalhost = 1;
	    else
		(*function) (connectionType, choice, closure);
	}
	else if (scanHostlist (d->hosts, clientAddress, connectionType,
			  function, closure, 0, FALSE))
	{
	    haveLocalhost = 1;
	}
	break;
    }
    if (clientName)
	free (clientName);
    return haveLocalhost;
}

int 
#ifdef _NO_PROTO
UseChooser( clientAddress, connectionType )
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
#else
UseChooser(
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType )
#else
        CARD16 connectionType )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    DisplayEntry    *d;
    char	    *clientName = 0, *NetworkAddressToHostname ();

    for (d = database; d; d = d->next)
    {
    	switch (d->type) {
    	case DISPLAY_ALIAS:
	    continue;
    	case DISPLAY_PATTERN:
	    if (!clientName)
		clientName = NetworkAddressToHostname (connectionType,
						       clientAddress);
	    if (!patternMatch (clientName, d->entry.displayPattern))
		continue;
	    break;
    	case DISPLAY_ADDRESS:
	    if (d->entry.displayAddress.connectionType != connectionType ||
	    	!XdmcpARRAY8Equal (&d->entry.displayAddress.clientAddress,
				  clientAddress))
	    {
		continue;
	    }
	    break;
    	}
	if (!d->hosts)
	    continue;
	if (d->notAllowed)
	    break;
#ifdef BYPASSLOGIN
	if (d->bypass)
	    break;
#endif /* BYPASSLOGIN */
	if (d->chooser && !IndirectChoice (clientAddress, connectionType))
	    return 1;
	break;
    }
    return 0;
}

void 
#ifdef _NO_PROTO
ForEachChooserHost( clientAddress, connectionType, function, closure )
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
        void (*function)() ;
        char *closure ;
#else
ForEachChooserHost(
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        void (*function)(),
        char *closure )
#endif /* _NO_PROTO */
{
    int		    haveLocalhost = 0;
    DisplayEntry    *d;
    char	    *clientName = 0, *NetworkAddressToHostname ();

    for (d = database; d; d = d->next)
    {
    	switch (d->type) {
    	case DISPLAY_ALIAS:
	    continue;
    	case DISPLAY_PATTERN:
	    if (!clientName)
		clientName = NetworkAddressToHostname (connectionType,
						       clientAddress);
	    if (!patternMatch (clientName, d->entry.displayPattern))
		continue;
	    break;
    	case DISPLAY_ADDRESS:
	    if (d->entry.displayAddress.connectionType != connectionType ||
	    	!XdmcpARRAY8Equal (&d->entry.displayAddress.clientAddress,
				  clientAddress))
	    {
		continue;
	    }
	    break;
    	}
	if (!d->hosts)
	    continue;
	if (d->notAllowed)
	    break;
#ifdef BYPASSLOGIN
	if (d->bypass)
	    break;
#endif /* BYPASSLOGIN */
	if (!d->chooser)
	    break;
	if (scanHostlist (d->hosts, clientAddress, connectionType,
			  function, closure, 0, TRUE))
	{
	    haveLocalhost = 1;
	}
	break;
    }
    if (clientName)
	free (clientName);
    if (haveLocalhost)
	(*function) (connectionType, getLocalAddress(), closure);
}

/*
 * returns TRUE if the given client is acceptable to the local host.  The
 * given display client is acceptable if it occurs without a host list.
 */

int 
#ifdef _NO_PROTO
AcceptableDisplayAddress( clientAddress, connectionType, type )
        ARRAY8Ptr clientAddress ;
        CARD16 connectionType ;
        xdmOpCode type ;
#else
AcceptableDisplayAddress(
        ARRAY8Ptr clientAddress,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        xdmOpCode type )
#endif /* _NO_PROTO */
{
    DisplayEntry    *d;
    char	    *clientName = 0, *NetworkAddressToHostname ();

    if (!accessFile || strlen(accessFile) == 0)
	return 1;
    if (type == INDIRECT_QUERY)
	return 1;
    for (d = database; d; d = d->next)
    {
	if (d->hosts)
	    continue;
    	switch (d->type) {
    	case DISPLAY_ALIAS:
	    continue;
    	case DISPLAY_PATTERN:
	    if (!clientName)
		clientName = NetworkAddressToHostname (connectionType,
						       clientAddress);
	    if (!patternMatch (clientName, d->entry.displayPattern))
		continue;
	    break;
    	case DISPLAY_ADDRESS:
	    if (d->entry.displayAddress.connectionType != connectionType ||
	    	!XdmcpARRAY8Equal (&d->entry.displayAddress.clientAddress,
				  clientAddress))
	    {
		continue;
	    }
	    break;
    	}
	break;
    }
    if (clientName)
	free (clientName);
    return (d != 0) && (d->notAllowed == 0);
}

#ifdef BYPASSLOGIN

char *
#ifdef _NO_PROTO
BypassLogin (displayName)
    char *displayName;
#else
BypassLogin (char *displayName)
#endif /* _NO_PROTO */
{
    DisplayEntry    *d;
    HostEntry	    *h;
    ARRAY8	connectionAddress;
    CARD16	connectionType;
    CARD16	displayNumber;
    
    if (!NameToNetworkAddress (displayName, &connectionType, &connectionAddress,
			       &displayNumber))
 	return NULL;
    
    for (d = database; d; d = d->next)
    {
     	switch (d->type) {
     	case DISPLAY_ALIAS:
 	    continue;
     	case DISPLAY_PATTERN:
 	    if (!patternMatch (displayName, d->entry.displayPattern))
 		continue;
 	    break;
     	case DISPLAY_ADDRESS:
 	    if (d->entry.displayAddress.connectionType != connectionType ||
 	    	!XdmcpARRAY8Equal (&d->entry.displayAddress.clientAddress,
				   &connectionAddress))
 	    {
 		continue;
 	    }
 	    break;
     	}
 	if (!d->hosts)
 	    continue;
 	if (d->notAllowed)
 	    continue;
 	if (d->chooser)
 	    continue;
 	if (d->bypass)
 	{
 	    h = d->hosts;
 	    if (h->entry.aliasName != NULL)
 		return h->entry.aliasName;
 	}
    }
    return NULL;
}

#ifdef _NO_PROTO
HostnameToNetworkAddress (name, connectionType, connectionAddress)
char *name;
CARD16 connectionType;
ARRAY8Ptr connectionAddress;
#else
HostnameToNetworkAddress (char *name,
#if NeedWidePrototypes
			  int connectionType,
#else
			  CARD16 connectionType,
#endif /* NeedWidePrototypes */
			  ARRAY8Ptr connectionAddress )
#endif /* _NO_PROTO */
{
    switch (connectionType) {
    case FamilyInternet:
    {
	struct hostent	*hostent;
	hostent = gethostbyname (name);
	if (!hostent) return FALSE;
	if (!XdmcpAllocARRAY8 (connectionAddress, hostent->h_length))
	    return FALSE;
	bcopy (hostent->h_addr, connectionAddress->data,
	       hostent->h_length);
	return TRUE;
    }
#ifdef DNET
    case FamilyDECnet:
	return FALSE;
#endif
    }
    return FALSE;
}


/*
 * converts a display name into a network address, using
 * the same rules as XOpenDisplay (algorithm cribbed from there)
 */

#ifdef _NO_PROTO
NameToNetworkAddress(name, connectionTypep,
		     connectionAddress, displayNumber)
char	    *name;
CARD16Ptr   connectionTypep;
ARRAY8Ptr   connectionAddress;
CARD16Ptr   displayNumber;
#else
NameToNetworkAddress(char        *name,
		     CARD16Ptr   connectionTypep,
		     ARRAY8Ptr   connectionAddress,
		     CARD16Ptr   displayNumber )
#endif /* _NO_PROTO */
{
    char    *colon, *display_number;
    char    hostname[1024];
    int	    dnet = FALSE;
    CARD16  number;
    CARD16  connectionType;
    
    colon = index (name, ':');
    if (!colon) return FALSE;
    if (colon != name) {
 	if (colon - name > sizeof (hostname)) return FALSE;
 	strncpy (hostname, name, colon - name);
 	hostname[colon - name] = '\0';
    } else {
 	strcpy (hostname, localHostname ());
    }

    if (colon[1] == ':') {
 	dnet = TRUE;
 	colon++;
    }

#ifndef DNETCONN
    if (dnet)
 	return FALSE;
#endif

    display_number = colon + 1;
    while (*display_number && *display_number != '.') {
 	if (!isascii (*display_number) || !isdigit(*display_number))
 	    return FALSE;
 	display_number++;
    }
    if (display_number == colon + 1) return FALSE;
    number = atoi (colon + 1);

#ifdef DNETCONN
    if (dnet)
 	connectionType = FamilyDECnet;
    else
#endif
 	connectionType = FamilyInternet;

    if (!HostnameToNetworkAddress (hostname, connectionType,
				   connectionAddress)) return FALSE;
    *displayNumber = number;
    *connectionTypep = connectionType;
    return TRUE;
}

#endif /* BYPASSLOGIN */
