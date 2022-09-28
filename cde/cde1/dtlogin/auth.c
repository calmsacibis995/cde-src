/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: auth.c /main/cde1_maint/5 1995/11/22 10:52:41 rcs $
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
 * auth.c
 *
 * maintain the authorization generation daemon
 */

# include   "vgmsg.h"
# include   "dm.h"
# include   <sys/signal.h>
# include   <setjmp.h>
# include   <sys/ioctl.h>
# include   <sys/socket.h>
# include   <sys/stat.h>
# include   <pwd.h>

#ifdef SVR4
# include <netdb.h>
# include <sys/sockio.h>
#ifdef USL
# include <sys/stropts.h>
#endif
#endif

#ifdef hpux
# include   <sys/utsname.h>
# include   <netdb.h>
#else
# include    <net/if.h>
#endif
#ifdef TCPCONN
# include   <netinet/in.h>
#endif
#ifdef DNETCONN
# include    <netdnet/dn.h>
# include    <netdnet/dnetdb.h>
#endif
# include    <X11/X.h>


/***************************************************************************
 *
 *  External declarations
 *
 ***************************************************************************/

extern char	*mktemp ();

#ifdef HASDES
extern int	XdmInitAuth ();
extern Xauth	*XdmGetAuth ();
extern int	XdmGetXdmcpAuth ();
#endif




/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static struct 	AuthProtocol * findProtocol() ;
static void 	CleanUpFileName() ;
static int	DefineSelf() ;
static int 	DefineLocal() ;
static Xauth *	GenerateAuthorization() ;
static int	MakeServerAuthFile() ;
static int	binaryEqual() ;
static int	checkAddr() ;
static void	doneAddrs() ;
static void	dumpAuth() ;
static void	dumpBytes() ;
static void	initAddrs() ;
static int 	openFiles() ;
static void	saveAddr() ;
static void	setAuthNumber() ;
static int	writeAuth() ;
static int	writeAddr() ;
static int	writeLocalAuth() ;
static void	writeRemoteAuth() ;

#else

static struct AuthProtocol * findProtocol( 
#if NeedWidePrototypes
                        unsigned int name_length,
#else
                        unsigned short name_length,
#endif /* NeedWidePrototypes */
                        char *name) ;
static void CleanUpFileName(
			char *src,
			char *dst,
			int len) ;
static int DefineSelf(
			int fd,
			FILE *file,
			Xauth *auth) ;
static int DefineLocal(
			FILE *file,
			Xauth *auth) ;

static Xauth * GenerateAuthorization( 
#if NeedWidePrototypes
                        unsigned int name_length,
#else
                        unsigned short name_length,
#endif /* NeedWidePrototypes */
                        char *name) ;

static int MakeServerAuthFile(
			struct display *d) ;
static int  binaryEqual(
			const char *a,
			const char *b,
#if NeedWidePrototypes
                        unsigned int len) ;
#else
                        unsigned short len) ;
#endif /* NeedWidePrototypes */

static int  checkAddr( 
#if NeedWidePrototypes
                        unsigned int family,
                        unsigned int address_length,
#else
                        unsigned short family,
                        unsigned short address_length,
#endif /* NeedWidePrototypes */
                        const char *address,
#if NeedWidePrototypes
                        unsigned int number_length,
#else
                        unsigned short number_length,
#endif /* NeedWidePrototypes */
                        const char *number) ;

static void  doneAddrs( void ) ;
static void  dumpAuth(
			Xauth *auth) ;

static void  dumpBytes( 
#if NeedWidePrototypes
                        unsigned int len,
#else
                        unsigned short len,
#endif /* NeedWidePrototypes */
                        const char *data) ;

static void  initAddrs( void ) ;

static int openFiles( char *name, char *new_name, FILE **oldp, FILE **newp) ;

static void saveAddr( 
#if NeedWidePrototypes
                        unsigned int family,
                        unsigned int address_length,
#else
                        unsigned short family,
                        unsigned short address_length,
#endif /* NeedWidePrototypes */
                        const char *address,
#if NeedWidePrototypes
                        unsigned int number_length,
#else
                        unsigned short number_length,
#endif /* NeedWidePrototypes */
                        const char *number) ;

static void  setAuthNumber(
			Xauth *auth,
			char *name) ;
static int  writeAuth(
			FILE *file,
			Xauth *auth) ;
static int writeAddr(
			int family,
			int addr_length,
			char *addr,
			FILE *file,
                        Xauth *auth) ;
static int writeLocalAuth(
			FILE *file,
			Xauth *auth,
			char *name) ;
static void writeRemoteAuth( FILE *file, Xauth *auth, struct sockaddr *peer,
                            int peerlen, char *name) ;

#endif /* _NO_PROTO */


/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/
struct AuthProtocol {
    unsigned short  name_length;
    char	    *name;
    void	    (*InitAuth)();
    Xauth	    *(*GetAuth)();
    int		    (*GetXdmcpAuth)();
    int		    inited;
};

static struct AuthProtocol AuthProtocols[] = {
{ (unsigned short) 18,	"MIT-MAGIC-COOKIE-1",
    MitInitAuth, MitGetAuth, NULL,
},
#ifdef HASDES
{ (unsigned short) 19,	"XDM-AUTHORIZATION-1",
    XdmInitAuth, XdmGetAuth, XdmGetXdmcpAuth,
}
#endif
};

#define NUM_AUTHORIZATION (sizeof (AuthProtocols) / sizeof (AuthProtocols[0]))



/****************************************************************************/

static struct AuthProtocol * 
#ifdef _NO_PROTO
findProtocol( name_length, name )
        unsigned short name_length ;
        char *name ;
#else
findProtocol(
#if NeedWidePrototypes
        unsigned int name_length,
#else
        unsigned short name_length,
#endif /* NeedWidePrototypes */
        char *name )
#endif /* _NO_PROTO */
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++)
	if (AuthProtocols[i].name_length == name_length &&
	    bcmp (AuthProtocols[i].name, name, name_length) == 0)
	{
	    return &AuthProtocols[i];
	}
    return (struct AuthProtocol *) 0;
}

int 
#ifdef _NO_PROTO
ValidAuthorization( name_length, name )
        unsigned short name_length ;
        char *name ;
#else
ValidAuthorization(
#if NeedWidePrototypes
        unsigned int name_length,
#else
        unsigned short name_length,
#endif /* NeedWidePrototypes */
        char *name )
#endif /* _NO_PROTO */
{
    if (findProtocol (name_length, name))
	return TRUE;
    return FALSE;
}

static Xauth * 
#ifdef _NO_PROTO
GenerateAuthorization( name_length, name )
        unsigned short name_length ;
        char *name ;
#else
GenerateAuthorization(
#if NeedWidePrototypes
        unsigned int name_length,
#else
        unsigned short name_length,
#endif /* NeedWidePrototypes */
        char *name )
#endif /* _NO_PROTO */
{
    struct AuthProtocol	*a;
    Xauth   *auth = 0;

    Debug ("GenerateAuthorization(): %*.*s\n",
	    name_length, name_length, name);
    a = findProtocol (name_length, name);
    if (a)
    {
	if (!a->inited)
	{
	    (*a->InitAuth) (name_length, name);
	    a->inited = TRUE;
	}
	auth = (*a->GetAuth) (name_length, name);
	if (auth)
	    Debug ("Got authorization 0x%x (%d %*.*s)\n", auth,
		auth->name_length, auth->name_length,
 		auth->name_length, auth->name);
	else
	    Debug ("Got (null)\n");
    }
    return auth;
}

void 
#ifdef _NO_PROTO
SetProtoDisplayAuthorization( pdpy, authorizationNameLen, authorizationName )
        struct protoDisplay *pdpy ;
        unsigned short authorizationNameLen ;
        char *authorizationName ;
#else
SetProtoDisplayAuthorization(
        struct protoDisplay *pdpy,
#if NeedWidePrototypes
        unsigned int authorizationNameLen,
#else
        unsigned short authorizationNameLen,
#endif /* NeedWidePrototypes */
        char *authorizationName )
#endif /* _NO_PROTO */
{
    struct AuthProtocol	*a;
    Xauth   *auth;

    Debug ("SetProtoDisplayAuthorization(): %*.*s\n",
	    authorizationNameLen, authorizationNameLen, authorizationName);

    a = findProtocol (authorizationNameLen, authorizationName);
    pdpy->xdmcpAuthorization = pdpy->fileAuthorization = 0;
    if (a)
    {
	if (!a->inited)
	{
	    (*a->InitAuth) (authorizationNameLen, authorizationName);
	    a->inited = TRUE;
	}
	if (a->GetXdmcpAuth)
	{
	    (*a->GetXdmcpAuth) (pdpy, authorizationNameLen, authorizationName);
	    auth = pdpy->xdmcpAuthorization;
	}
	else
	{
	    auth = (*a->GetAuth) (authorizationNameLen, authorizationName);
	    pdpy->fileAuthorization = auth;
	    pdpy->xdmcpAuthorization = 0;
	}
	if (auth)
	    Debug ("Got authorization 0x%x (%d %*.*s)\n", auth,
		auth->name_length, auth->name_length,
 		auth->name_length, auth->name);
	else
	    Debug ("Got (null)\n");
    }
}

static void 
#ifdef _NO_PROTO
CleanUpFileName( src, dst, len )
        char *src ;
        char *dst ;
        int len ;
#else
CleanUpFileName( char *src, char *dst, int len )
#endif /* _NO_PROTO */
{
    while (*src) {
	if (--len <= 0)
		break;
	switch (*src & 0x7f)
	{
	case '/':
	    *dst++ = '_';
	    break;
	case '-':
	    *dst++ = '.';
	    break;
	default:
	    *dst++ = (*src & 0x7f);
	}
	++src;
    }
    *dst = '\0';
}

static int 
#ifdef _NO_PROTO
MakeServerAuthFile( d )
        struct display *d ;
#else
MakeServerAuthFile( struct display *d )
#endif /* _NO_PROTO */
{
    int len;
#ifdef SYSV
#define NAMELEN	14
#else
#define NAMELEN	255
#endif
    char    cleanname[NAMELEN];

    if (d->clientAuthFile && *d->clientAuthFile)
	len = strlen (d->clientAuthFile) + 1;
    else
    {
    	CleanUpFileName (d->name, cleanname, NAMELEN - 8);
    	len = strlen (authDir) + strlen (cleanname) + 12;
    }
    if (d->authFile)
	free (d->authFile);
    d->authFile = malloc ((unsigned) len);
    if (!d->authFile)
	return FALSE;
    if (d->clientAuthFile && *d->clientAuthFile)
	strcpy (d->authFile, d->clientAuthFile);
    else
    {
    	sprintf (d->authFile, "%s/A%s-XXXXXX", authDir, cleanname);
    	(void) mktemp (d->authFile);
    }
    return TRUE;
}

extern struct passwd   puser;	/* pseudo_user password entry	*/

int 
#ifdef _NO_PROTO
SaveServerAuthorization( d, auth )
        struct display *d ;
        Xauth *auth ;
#else
SaveServerAuthorization( struct display *d, Xauth *auth )
#endif /* _NO_PROTO */
{
    FILE	*auth_file;
    int		mask;
    int		ret;

    mask = umask (0077);
    if ((!d->authFile || d->authFile[0] == '\0') && !MakeServerAuthFile (d))
	return FALSE;
    (void) unlink (d->authFile);
    auth_file = fopen (d->authFile, "w");
    umask (mask);
    if (!auth_file) {
	Debug ("Can't create auth file '%s'\n", d->authFile);
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_SRV_OPEN,MC_DEF_LOG_SRV_OPEN),
		d->authFile);
	free (d->authFile);
	d->authFile = NULL;
	ret = FALSE;
    }
    else
    {
    	Debug ("Authorization file: %s, auth: %x\n", d->authFile, auth);
    	if (!XauWriteAuth (auth_file, auth) || fflush (auth_file) == EOF) 
    	{
		LogError(
		  ReadCatalog(MC_LOG_SET,MC_LOG_SRV_WRT,MC_DEF_LOG_SRV_WRT),
		       d->authFile);
	    free (d->authFile);
	    d->authFile = NULL;
	    ret = FALSE;
    	}
	else
	    ret = TRUE;

	fclose (auth_file);

	/*
	 * change ownership of file so server can read it... (X server
	 * does not run as root)
	 */

	chown(d->authFile, puser.pw_uid, GID_NO_CHANGE);
    }
    return ret;
}

void 
#ifdef _NO_PROTO
SetLocalAuthorization( d )
        struct display *d ;
#else
SetLocalAuthorization( struct display *d )
#endif /* _NO_PROTO */
{
    Xauth	*auth;

    if (d->authorization)
    {
	XauDisposeAuth (d->authorization);
	d->authorization = (Xauth *) NULL;
    }
    if (d->authName && !d->authNameLen)
    	d->authNameLen = strlen (d->authName);
    auth = GenerateAuthorization (d->authNameLen, d->authName);
    if (!auth)
	return;
    if (SaveServerAuthorization (d, auth))
	d->authorization = auth;
    else
	XauDisposeAuth (auth);
}

void
#ifdef _NO_PROTO
SetAuthorization( d )
        struct display *d ;
#else
SetAuthorization( struct display *d )
#endif /* _NO_PROTO */
{
    Xauth   *auth;

    auth = d->authorization;
    if (auth)
	XSetAuthorization (auth->name, (int) auth->name_length,
			   auth->data, (int) auth->data_length);
}

static int 
#ifdef _NO_PROTO
openFiles( name, new_name, oldp, newp )
        char *name ;
        char *new_name ;
        FILE **oldp ;
        FILE **newp ;
#else
openFiles( char *name, char *new_name, FILE **oldp, FILE **newp )
#endif /* _NO_PROTO */
{
	int	mask;

	strcpy (new_name, name);
	strcat (new_name, "-n");
	mask = umask (0077);
	(void) unlink (new_name);
	*newp = fopen (new_name, "w");
	(void) umask (mask);
	if (!*newp) {
		Debug ("Can't open new file %s\n", new_name);
		return 0;
	}
	*oldp = fopen (name, "r");
	Debug ("Opens succeeded for files %s and %s\n", name, new_name);
	return 1;
}

static int 
#ifdef _NO_PROTO
binaryEqual( a, b, len )
        char *a ;
        char *b ;
        unsigned short len ;
#else
binaryEqual( const char *a, const char *b,
#if NeedWidePrototypes
        unsigned int len )
#else
        unsigned short len )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
	while (len-- > 0)
		if (*a++ != *b++)
			return 0;
	return 1;
}

static void
#ifdef _NO_PROTO
dumpBytes( len, data )
        unsigned short len ;
        char *data ;
#else
dumpBytes(
#if NeedWidePrototypes
        unsigned int len,
#else
        unsigned short len,
#endif /* NeedWidePrototypes */
        const char *data )
#endif /* _NO_PROTO */
{
	unsigned short	i;

	Debug ("%d: ", len);
	for (i = 0; i < len; i++)
		Debug ("%02x ", data[i] & 0377);
	Debug ("\n");
}

static void
#ifdef _NO_PROTO
dumpAuth( auth )
        Xauth *auth ;
#else
dumpAuth( Xauth *auth )
#endif /* _NO_PROTO */
{
	Debug ("family: %d\n", auth->family);
	Debug ("addr:   ");
	dumpBytes (auth->address_length, auth->address);
	Debug ("number: ");
	dumpBytes (auth->number_length, auth->number);
	Debug ("name:   ");
	dumpBytes (auth->name_length, auth->name);
	Debug ("data:   ");
	dumpBytes (auth->data_length, auth->data);
}

struct addrList {
	unsigned short	family;
	unsigned short	address_length;
	char	*address;
	unsigned short	number_length;
	char	*number;
	struct addrList	*next;
};

static struct addrList	*addrs;

static void
#ifdef _NO_PROTO
initAddrs()
#else
initAddrs( void )
#endif /* _NO_PROTO */
{
	addrs = 0;
}

static void
#ifdef _NO_PROTO
doneAddrs()
#else
doneAddrs( void )
#endif /* _NO_PROTO */
{
	struct addrList	*a, *n;
	for (a = addrs; a; a = n) {
		n = a->next;
		if (a->address)
			free (a->address);
		if (a->number)
			free (a->number);
		free ((char *) a);
	}
}

static void 
#ifdef _NO_PROTO
saveAddr( family, address_length, address, number_length, number )
        unsigned short family ;
        unsigned short address_length ;
        char *address ;
        unsigned short number_length ;
        char *number ;
#else
saveAddr(
#if NeedWidePrototypes
        unsigned int family,
        unsigned int address_length,
#else
        unsigned short family,
        unsigned short address_length,
#endif /* NeedWidePrototypes */
        const char *address,
#if NeedWidePrototypes
        unsigned int number_length,
#else
        unsigned short number_length,
#endif /* NeedWidePrototypes */
        const char *number )
#endif /* _NO_PROTO */
{
	struct addrList	*new;

	if (checkAddr (family, address_length, address, number_length, number))
		return;
	new = (struct addrList *) malloc (sizeof (struct addrList));
	if (!new) {
		LogOutOfMem(ReadCatalog(
			MC_LOG_SET,MC_LOG_SAVE_ADDR,MC_DEF_LOG_SAVE_ADDR));
		return;
	}
	if ((new->address_length = address_length) > 0) {
		new->address = malloc (address_length);
		if (!new->address) {
			LogOutOfMem(ReadCatalog(
			  MC_LOG_SET,MC_LOG_SAVE_ADDR,MC_DEF_LOG_SAVE_ADDR));
			free ((char *) new);
			return;
		}
		bcopy (address, new->address, (int) address_length);
	} else
		new->address = 0;
	if ((new->number_length = number_length) > 0) {
		new->number = malloc (number_length);
		if (!new->number) {
			LogOutOfMem(ReadCatalog(
			  MC_LOG_SET,MC_LOG_SAVE_ADDR,MC_DEF_LOG_SAVE_ADDR));
			free (new->address);
			free ((char *) new);
			return;
		}
		bcopy (number, new->number, (int) number_length);
	} else
		new->number = 0;
	new->family = family;
	new->next = addrs;
	addrs = new;
}

static int 
#ifdef _NO_PROTO
checkAddr( family, address_length, address, number_length, number )
        unsigned short family ;
        unsigned short address_length ;
        char *address ;
        unsigned short number_length ;
        char *number ;
#else
checkAddr(
#if NeedWidePrototypes
        unsigned int family,
        unsigned int address_length,
#else
        unsigned short family,
        unsigned short address_length,
#endif /* NeedWidePrototypes */
        const char *address,
#if NeedWidePrototypes
        unsigned int number_length,
#else
        unsigned short number_length,
#endif /* NeedWidePrototypes */
        const char *number )
#endif /* _NO_PROTO */
{
	struct addrList	*a;

	for (a = addrs; a; a = a->next) {
		if (a->family == family &&
		    a->address_length == address_length &&
 		    binaryEqual (a->address, address, address_length) &&
		    a->number_length == number_length &&
 		    binaryEqual (a->number, number, number_length))
		{
			return 1;
		}
	}
	return 0;
}

static int
#ifdef _NO_PROTO
writeAuth( file, auth )
        FILE *file ;
        Xauth *auth ;
#else
writeAuth( FILE *file, Xauth *auth )
#endif /* _NO_PROTO */
{
	saveAddr (auth->family, auth->address_length, auth->address,
				auth->number_length,  auth->number);
    	if (!XauWriteAuth (file, auth) || fflush (file) == EOF)  {
                LogError(
                  ReadCatalog(MC_LOG_SET,MC_LOG_SRV_WRT,MC_DEF_LOG_SRV_WRT),
		  file);
		return 0;
	}
	else
		return 1;


}

static int
#ifdef _NO_PROTO
writeAddr( family, addr_length, addr, file, auth )
        int family ;
        int addr_length ;
        char *addr ;
        FILE *file ;
        Xauth *auth ;
#else
writeAddr( int family, int addr_length, char *addr, FILE *file, Xauth *auth )
#endif /* _NO_PROTO */
{
	Debug ("writeAddr()\n");
	auth->family = (unsigned short) family;
	auth->address_length = addr_length;
	auth->address = addr;
	dumpAuth (auth);
	return(writeAuth (file, auth));
}

static int
#ifdef _NO_PROTO
DefineLocal( file, auth )
        FILE *file ;
        Xauth *auth ;
#else
DefineLocal( FILE *file, Xauth *auth )
#endif /* _NO_PROTO */
{
	char	displayname[100];
	
	/* stolen from xinit.c */
#ifdef hpux

	/* Why not use gethostname()?  Well, at least on my system, I've had to
	 * make an ugly kernel patch to get a name longer than 8 characters, and
	 * uname() lets me access to the whole string (it smashes release, you
	 * see), whereas gethostname() kindly truncates it for me.
	 */

	/*
	 * In order to transition from using uname() to gethostname() it is
	 * necessary to provide both identities for a while. Clients using 
	 * older versions of xlib (pre HP-UX 9.0) will be looking for the 
	 * uname() identity. Newer clients will need gethostname().  
	 * Dt 3.0 7/7/92
	 */
	{
	struct utsname name;

	uname(&name);
	strcpy(displayname, name.nodename);
	}
	writeAddr (FamilyLocal, strlen (displayname), displayname, file, auth);
#endif

	gethostname(displayname, sizeof(displayname));
	return(writeAddr (FamilyLocal, strlen (displayname), displayname, file, auth));
}

#ifdef USL
/* Deal with different SIOCGIFCONF ioctl semantics on UnixWare */
static int
ifioctl (fd, cmd, arg)
    int fd;
    int cmd;
    char *arg;
{
    struct strioctl ioc;
    int ret;
  
    bzero((char *) &ioc, sizeof(ioc));
    ioc.ic_cmd = cmd;
    ioc.ic_timout = 0;
    if (cmd == SIOCGIFCONF)
    {
	ioc.ic_len = ((struct ifconf *) arg)->ifc_len;
	ioc.ic_dp = ((struct ifconf *) arg)->ifc_buf;
    }
    else
    {
	ioc.ic_len = sizeof(struct ifreq);
	ioc.ic_dp = arg;
    }
    ret = ioctl(fd, I_STR, (char *) &ioc);
    if (ret >= 0 && cmd == SIOCGIFCONF)
	((struct ifconf *) arg)->ifc_len = ioc.ic_len;
    return(ret);
}
#endif /* USL */


#ifdef hpux
/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 * HPUX version - hpux does not have SIOCGIFCONF ioctl;
 */
static int 
#ifdef _NO_PROTO
DefineSelf( fd, file, auth )
        int fd ;
        FILE *file ;
        Xauth *auth ;
#else
DefineSelf( int fd, FILE *file, Xauth *auth )
#endif /* _NO_PROTO */
{
    register int n;
    int	len;
    caddr_t	addr;
    int		family;

    struct utsname name;
    register struct hostent  *hp;

    union {
	struct  sockaddr   sa;
	struct  sockaddr_in  in;
    } saddr;
	
    struct	sockaddr_in	*inetaddr;

    /* Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
    uname(&name);
    hp = gethostbyname (name.nodename);
    if (hp != NULL) {
	saddr.sa.sa_family = hp->h_addrtype;
	inetaddr = (struct sockaddr_in *) (&(saddr.sa));
	bcopy ( (char *) hp->h_addr, (char *) &(inetaddr->sin_addr), (int) hp->h_length);
	family = ConvertAddr ( (XdmcpNetaddr) &(saddr.sa), &len, &addr);
	if ( family >= 0) {
	    return(writeAddr (FamilyInternet, sizeof (inetaddr->sin_addr),
			(char *) (&inetaddr->sin_addr), file, auth));
	}
    }
}

#else

/* Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 */
static int 
#ifdef _NO_PROTO
DefineSelf( fd, file, auth )
        int fd ;
        FILE *file ;
        Xauth *auth ;
#else
DefineSelf( int fd, FILE *file, Xauth *auth )
#endif /* _NO_PROTO */
{
    char		buf[2048];
    struct ifconf	ifc;
    register int	n;
    int 		len;
    char 		*addr;
    int 		family;
    register struct ifreq *ifr;
    
    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = buf;
    bzero(buf, sizeof(buf));

#ifdef USL
    if (ifioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0) {
#else
    if (ioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0) {
#endif
      LogError(ReadCatalog(MC_LOG_SET,MC_LOG_NET_CFG,MC_DEF_LOG_NET_CFG));
      return(0);
    }
    for (ifr = ifc.ifc_req
#if defined (__bsdi__) || defined(__NetBSD__)
         ; (char *)ifr < ifc.ifc_buf + ifc.ifc_len;
         ifr = (struct ifreq *)((char *)ifr + sizeof (struct ifreq) +
                   (ifr->ifr_addr.sa_len > sizeof (ifr->ifr_addr) ?
                    ifr->ifr_addr.sa_len - sizeof (ifr->ifr_addr) : 0))
#else
         , n = ifc.ifc_len / sizeof (struct ifreq); --n >= 0; ifr++
#endif
         )

    {
#ifdef DNETCONN
	/*
	 * this is ugly but SIOCGIFCONF returns decnet addresses in
	 * a different form from other decnet calls
	 */
	if (ifr->ifr_addr.sa_family == AF_DECnet) {
		len = sizeof (struct dn_naddr);
		addr = (char *)ifr->ifr_addr.sa_data;
		family = FamilyDECnet;
	} else
#endif
	{
        	if (ConvertAddr((XdmcpNetaddr) &ifr->ifr_addr, &len, &addr) < 0)
	    	    continue;
		/*
		 * don't write out 'localhost' entries, as
		 * they may conflict with other local entries.
		 * DefineLocal will always be called to add
		 * the local entry anyway, so this one can
		 * be tossed.
		 */
		if (len == 4 &&
		    addr[0] == 127 && addr[1] == 0 &&
		    addr[2] == 0 && addr[3] == 1)
 		{
			Debug ("Skipping localhost address\n");
			continue;
		}
		family = FamilyInternet;
	}
	Debug ("DefineSelf(): write network address, length %d\n", len);
	return(writeAddr (family, len, addr, file, auth));
    }
}
#endif /* hpux */

static void 
#ifdef _NO_PROTO
setAuthNumber( auth, name )
        Xauth *auth ;
        char *name ;
#else
setAuthNumber( Xauth *auth, char *name )
#endif /* _NO_PROTO */
{
    char	*colon;
    char	*dot, *number;

    Debug ("setAuthNumber(): %s\n", name);
    colon = rindex (name, ':');
    if (colon) {
	++colon;
	if (dot = index (colon, '.'))
	    auth->number_length = dot - colon;
	else
	    auth->number_length = strlen (colon);
	number = malloc (auth->number_length + 1);
	if (number) {
	    strncpy (number, colon, auth->number_length);
	    number[auth->number_length] = '\0';
	} else {
	    LogOutOfMem(
		ReadCatalog(MC_LOG_SET,MC_LOG_NET_CFG,MC_DEF_LOG_NET_CFG));
	    auth->number_length = 0;
	}
	auth->number = number;
	Debug ("setAuthNumber(): %s\n", number);
    }
}

static int 
#ifdef _NO_PROTO
writeLocalAuth( file, auth, name )
        FILE *file ;
        Xauth *auth ;
        char *name ;
#else
writeLocalAuth( FILE *file, Xauth *auth, char *name )
#endif /* _NO_PROTO */
{
    int	fd;
    Debug ("writeLocalAuth():\n");
    setAuthNumber (auth, name);
#ifdef TCPCONN
    fd = socket (AF_INET, SOCK_STREAM, 0);
    if(fd == -1 || !DefineSelf (fd, file, auth))
	return 0;
    close (fd);
#endif
#ifdef DNETCONN
    fd = socket (AF_DECnet, SOCK_STREAM, 0);
    DefineSelf (fd, file, auth);
    close (fd);
#endif
	return(DefineLocal (file, auth));
}

static void 
#ifdef _NO_PROTO
writeRemoteAuth( file, auth, peer, peerlen, name )
        FILE *file ;
        Xauth *auth ;
        struct sockaddr *peer ;
        int peerlen ;
        char *name ;
#else
writeRemoteAuth( FILE *file, Xauth *auth, struct sockaddr *peer, int peerlen,
		 char *name )
#endif /* _NO_PROTO */
{
    int	    family = FamilyLocal;
    char    *addr;
    
    if (!peer || peerlen < 2)
	return;
    setAuthNumber (auth, name);
    family = ConvertAddr ((XdmcpNetaddr) peer, &peerlen, &addr);
    Debug ("writeRemoteAuth(): family %d\n", family);
    if (family != FamilyLocal)
    {
	Debug ("writeRemoteAuth(): %d, %d, %x\n",
		family, peerlen, *(int *)addr);
	writeAddr (family, peerlen, addr, file, auth);
    }
    else
    {
	writeLocalAuth (file, auth, name);
    }
}

void 
#ifdef _NO_PROTO
SetUserAuthorization( d, verify )
        struct display *d ;
        struct verify_info *verify ;
#else
SetUserAuthorization( struct display *d, struct verify_info *verify )
#endif /* _NO_PROTO */
{
    FILE	*old, *new;
    char	home_name[1024], backup_name[1024], new_name[1024];
    char	*name;
    char	*home;
    char	*envname = 0;
    int	lockStatus;
    Xauth	*entry, *auth;
    int	setenv;
    struct stat	statb;

    Debug ("SetUserAuthorization():\n");
    if (auth = d->authorization) {
	home = getEnv (verify->userEnviron, "HOME");
	lockStatus = LOCK_ERROR;
	if (home) {
 	    strcpy (home_name, home);
 	    if (home[strlen(home) - 1] != '/')
 		strcat (home_name, "/");
 	    strcat (home_name, ".Xauthority");
	    Debug ("Calling XauLockAuth() for %s\n", home_name);
	    lockStatus = XauLockAuth (home_name, 1, 2, 10);
	    Debug ("Lock status is %d\n", lockStatus);
	    if (lockStatus == LOCK_SUCCESS) {
		if (openFiles (home_name, new_name, &old, &new)) {
		    name = home_name;
		    setenv = 0;
		} else {
		    Debug ("openFiles() failed\n");
		    XauUnlockAuth (home_name);
		    lockStatus = LOCK_ERROR;
		}	
	    }
	}

	if (lockStatus != LOCK_SUCCESS) {
	    sprintf (backup_name, "%s/.XauthXXXXXX", d->userAuthDir);
	    mktemp (backup_name);
	    Debug ("Calling XauLockAuth() for %s\n", backup_name);
	    lockStatus = XauLockAuth (backup_name, 1, 2, 10);
	    Debug ("Backup lock status is %d\n", lockStatus);
	    if (lockStatus == LOCK_SUCCESS) {
		if (openFiles (backup_name, new_name, &old, &new)) {
		    name = backup_name;
		    setenv = 1;
		} else {
		    XauUnlockAuth (backup_name);
		    lockStatus = LOCK_ERROR;
		}	
	    }
	}
	if (lockStatus != LOCK_SUCCESS) {
	    Debug ("Can't lock auth file %s or backup %s\n",
			    home_name, backup_name);
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_LCK_AUTH,MC_DEF_LOG_LCK_AUTH),
		home_name, backup_name);
	    return;
	}

	initAddrs ();
	if (d->displayType.location == Local) {
	  if(!writeLocalAuth (new, auth, d->name)) {
	    if (unlink (new_name) == -1) 
	        Debug ("unlink() on %s failed\n", new_name);
            XauUnlockAuth (name);
	    sprintf (backup_name, "%s/.XauthXXXXXX", d->userAuthDir);
            mktemp (backup_name);
            Debug ("Calling XauLockAuth() for %s\n", backup_name);
            lockStatus = XauLockAuth (backup_name, 1, 2, 10);
            Debug ("Backup lock status is %d\n", lockStatus);
            if (lockStatus == LOCK_SUCCESS) {
                if (openFiles (backup_name, new_name, &old, &new)) {
                    name = backup_name;
                    setenv = 1;
                } else {
                    XauUnlockAuth (backup_name);
                    lockStatus = LOCK_ERROR;
                }
            }
	    if (lockStatus != LOCK_SUCCESS) {
                Debug ("Can't lock auth file %s or backup %s\n",
                            home_name, backup_name);
                LogError(
                ReadCatalog(MC_LOG_SET,MC_LOG_LCK_AUTH,MC_DEF_LOG_LCK_AUTH),
                home_name, backup_name);
                return;
            }

	    writeLocalAuth (new, auth, d->name);
	  }
	}
	else
	    writeRemoteAuth (new, auth, d->peer, d->peerlen, d->name);

	if (old) {
	    if (fstat (fileno (old), &statb) != -1)
		chmod (new_name, (int) (statb.st_mode & 0777));
	    while (entry = XauReadAuth (old)) {
		if (!checkAddr (entry->family,
			       entry->address_length, entry->address,
			       entry->number_length, entry->number))
		{
		    Debug ("Saving an entry\n");
		    dumpAuth (entry);
		    writeAuth (new, entry);
		}
		XauDisposeAuth (entry);
	    }
	    fclose (old);
	}
	doneAddrs ();
	fclose (new);

	if (unlink (name) == -1) 
	    Debug ("unlink() on %s failed, errno=%d\n", name, errno);
	envname = name;
	if (link (new_name, name) == -1) {
	    Debug ("link() failed on %s %s\n", new_name, name);
	    LogError(
		ReadCatalog(MC_LOG_SET,MC_LOG_NOT_AUTH,MC_DEF_LOG_NOT_AUTH));
	    setenv = 1;
	    envname = new_name;
	} else {
	    Debug ("New is in place, go for it!\n");
	    unlink (new_name);
	}

	if (setenv) {
	    verify->userEnviron = setEnv (verify->userEnviron,
				    "XAUTHORITY", envname);
	    verify->systemEnviron = setEnv (verify->systemEnviron,
				    "XAUTHORITY", envname);
	}
	XauUnlockAuth (name);
	if (envname) {
#ifdef NGROUPS
	    chown (envname, verify->uid, verify->groups[0]);
#else
	    chown (envname, verify->uid, verify->gid);
#endif
	}
    }
    Debug ("Done SetUserAuthorization()\n");
}
