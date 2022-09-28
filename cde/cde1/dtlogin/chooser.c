/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * $XConsortium: chooser.c /main/cde1_maint/3 1995/10/09 00:07:36 pascale $
 *
Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Chooser - display a menu of names and let the user select one
 */

/*
 * Layout:
 *
 *  +--------------------------------------------------+
 *  |             +------------------+                 |
 *  |             |      Label       |                 |
 *  |             +------------------+                 |
 *  |    +-+--------------+                            |
 *  |    |^| name-1       |                            |
 *  |    ||| name-2       |                            |
 *  |    |v| name-3       |                            |
 *  |    | | name-4       |                            |
 *  |    | | name-5       |                            |
 *  |    | | name-6       |                            |
 *  |    +----------------+                            |
 *  |    cancel  accept  ping                          |
 *  +--------------------------------------------------+
 */

#include    <X11/Intrinsic.h>
#include    <X11/StringDefs.h>
#include    <X11/Xatom.h>


#include    <Xm/DragC.h>
#include    <Xm/List.h>

#define     CHOOSER

#include    "dm.h"

#include    "vg.h"
#include    "vgmsg.h"

#include    <X11/Xdmcp.h>

#include    <sys/signal.h>
#include    <sys/types.h>
#include    <stdio.h>
#include    <ctype.h>
#include    <locale.h>
#include    <nl_types.h>
#ifndef NL_CAT_LOCALE
#define NL_CAT_LOCALE 0
#endif

#ifdef SVR4
#include    <sys/sockio.h>
#endif
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <sys/ioctl.h>

#define BROADCAST_HOSTNAME  "BROADCAST"

#ifndef ishexdigit
#define ishexdigit(c)	(isdigit(c) || 'a' <= (c) && (c) <= 'f')
#endif

#ifdef hpux
# include <sys/utsname.h>
# ifdef HAS_IFREQ
#  include <net/if.h>
# endif
#else
#ifdef __convex__
# include <sync/queue.h>
# include <sync/sema.h>
#endif
# include <net/if.h>
#endif /* hpux */

#include    <netdb.h>

Widget	    toplevel, label, viewport, paned, list, box, cancel, acceptit, ping;

extern Widget chooser_list;
extern int orig_argc;
extern char **orig_argv;
extern void MakeOptionsProc();

static Arg  chooserArgs[25];     /** Hopefully enough args **/

static void	CvtStringToARRAY8();

static struct _app_resources {
    ARRAY8Ptr   xdmAddress;
    ARRAY8Ptr	clientAddress;
    int		connectionType;
} app_resources;

#define offset(field) XtOffsetOf(struct _app_resources, field)

#define XtRARRAY8   "ARRAY8"

static XtResource  resources[] = {
    {"xdmAddress",	"XdmAddress",  XtRARRAY8,	sizeof (ARRAY8Ptr),
	offset (xdmAddress),	    XtRString,	NULL },
    {"clientAddress",	"ClientAddress",  XtRARRAY8,	sizeof (ARRAY8Ptr),
	offset (clientAddress),	    XtRString,	NULL },
    {"connectionType",	"ConnectionType",   XtRInt,	sizeof (int),
	offset (connectionType),    XtRImmediate,	(XtPointer) 0 }
};
#undef offset

static XrmOptionDescRec options[] = {
    "-xdmaddress",	"*xdmAddress",	    XrmoptionSepArg,	NULL,
    "-clientaddress",	"*clientAddress",   XrmoptionSepArg,	NULL,
    "-connectionType",	"*connectionType",  XrmoptionSepArg,	NULL,
};

typedef struct _hostAddr {
    struct _hostAddr	*next;
    struct sockaddr	*addr;
    int			addrlen;
    xdmOpCode		type;
} HostAddr;

static HostAddr    *hostAddrdb;

typedef struct _hostName {
    struct _hostName	*next;
    char		*fullname;
    int			willing;
    ARRAY8		hostname, status;
    CARD16		connectionType;
    ARRAY8		hostaddr;
} HostName;

static HostName    *hostNamedb;

static int  socketFD;

static int  pingTry;

#define PING_INTERVAL	2000
#define TRIES		3

static XdmcpBuffer	directBuffer, broadcastBuffer;
static XdmcpBuffer	buffer;


/* ARGSUSED */
static void
PingHosts (closure, id)
    XtPointer closure;
    XtIntervalId *id;
{
    HostAddr	*hosts;

    for (hosts = hostAddrdb; hosts; hosts = hosts->next)
    {
	if (hosts->type == QUERY)
	    XdmcpFlush (socketFD, &directBuffer, hosts->addr, hosts->addrlen);
	else
	    XdmcpFlush (socketFD, &broadcastBuffer, hosts->addr, hosts->addrlen);
    }
    if (++pingTry < TRIES)
	XtAddTimeOut (PING_INTERVAL, PingHosts, (XtPointer) 0);
}

char	**NameTable=0;
int	NameTableSize=0;

static int
HostnameCompare (a, b)
#ifdef __STDC__
    const void *a, *b;
#else
    char *a, *b;
#endif
{
    return strcmp (*(char **)a, *(char **)b);
}

static void
RebuildTable (size)
{
    char	**newTable = 0;
    HostName	*names;
    int		i;
    XmStringTable newStringTable;
    Arg		listArgs[10];
    int		numArgs;
    char        *temphostname;
    char        localHostbuf[256];
    int 	choos_on_local = False;


    if (strcmp(getenv(LOCATION), "local") == 0) {
        gethostname (localHostbuf, sizeof (localHostbuf) - 1);
	choos_on_local = True;
    }
    else
	choos_on_local = False;

    if (size)
    {
	newTable = (char **) malloc (size * sizeof (char *));
	if (!newTable)
	    return;
	for (names = hostNamedb, i = 0; names; names = names->next, i++) {
	 if(choos_on_local)
           if(strncmp(names->fullname, localHostbuf, strlen(localHostbuf)) == 0)
		temphostname = names->fullname;

	    newTable[i] = names->fullname;
	}
	qsort (newTable, size, sizeof (char *), HostnameCompare);
    }
      else  {
        XmListDeleteAllItems(chooser_list);
	if (NameTable) {
	    free((char *)NameTable);
	    NameTable=0;
	}
	NameTableSize = 0;
	return;
    }

    /***********************************/
    /** Reload the data in the XmList **/
    /***********************************/
    newStringTable = (XmStringTable)malloc(size * sizeof(XmString));
    if (!newStringTable) {
	free((char *)newTable);
	return;
    }
    for (i = 0; i < size; i++) {
	newStringTable[i] = XmStringCreateLtoR(newTable[i],
					       XmFONTLIST_DEFAULT_TAG);
    }
    numArgs = 0;
    XtSetArg(listArgs[numArgs], XmNitemCount, size); numArgs++;
    XtSetArg(listArgs[numArgs], XmNitems, newStringTable); numArgs++;
    XtSetValues(chooser_list, listArgs, numArgs);

    if(choos_on_local)
      XmListDeleteItem(chooser_list, XmStringCreateLtoR(temphostname,
                                               XmFONTLIST_DEFAULT_TAG));

    free((char *) newStringTable);

    if (NameTable)
	free ((char *) NameTable);
    NameTable = newTable;
    NameTableSize = size;
}

static void
RebuildTableAdd (size)
	int size;
{
    char        **newTable = 0;
    HostName    *names;
    int         i;
    XmString    tempString;
    int		position;

    if (size)
    {
        newTable = (char **) malloc (size * sizeof (char *));
        if (!newTable)
            return;
        for (names = hostNamedb, i = 0; names; names = names->next, i++)
            newTable[i] = names->fullname;
        qsort (newTable, size, sizeof (char *), HostnameCompare);
    }

    for (i = 0; i < size; i++) {
        tempString = XmStringCreateLtoR(newTable[i], XmFONTLIST_DEFAULT_TAG);
        if ((position = XmListItemPos(chooser_list, tempString)) == 0) {
	    /****************************/
	    /** need to add a new host **/
	    /****************************/
	    XmListAddItemUnselected(chooser_list, tempString, (i+1));
	    XmStringFree(tempString);
	    break;
	}
	XmStringFree(tempString);
    }

    if (NameTable)
        free ((char *) NameTable);
    NameTable = newTable;
    NameTableSize = size;
}


static int
AddHostname (hostname, status, addr, willing)
    ARRAY8Ptr	    hostname, status;
    struct sockaddr *addr;
    int		    willing;
{
    HostName	*new, **names, *name;
    ARRAY8	hostAddr;
    CARD16	connectionType;
    int		fulllen;
    int         update = 0;

    switch (addr->sa_family)
    {
    case AF_INET:
	hostAddr.data = (CARD8 *) &((struct sockaddr_in *) addr)->sin_addr;
	hostAddr.length = 4;
	connectionType = FamilyInternet;
	break;
    default:
	hostAddr.data = (CARD8 *) "";
	hostAddr.length = 0;
	connectionType = FamilyLocal;
	break;
    }
    for (names = &hostNamedb; *names; names = & (*names)->next)
    {
	name = *names;
	if (connectionType == name->connectionType &&
	    XdmcpARRAY8Equal (&hostAddr, &name->hostaddr))
	{
	    if (XdmcpARRAY8Equal (status, &name->status))
	    {
		return 0;
	    }
	    update = 1;
	    break;
	}
    }
    if (!*names)
    {
	new = (HostName *) malloc (sizeof (HostName));
    	if (!new)
	    return 0;
	if (hostname->length)
	{
	    switch (addr->sa_family)
	    {
	    case AF_INET:
	    	{
	    	    struct hostent  *hostent;
		    char	    *host;
    	
	    	    hostent = gethostbyaddr ((char *)hostAddr.data, hostAddr.length, AF_INET);
	    	    if (hostent)
	    	    {
			XdmcpDisposeARRAY8 (hostname);
		    	host = hostent->h_name;
			XdmcpAllocARRAY8 (hostname, strlen (host));
			memmove( hostname->data, host, hostname->length);
	    	    }
	    	}
	    }
	}
    	if (!XdmcpAllocARRAY8 (&new->hostaddr, hostAddr.length))
    	{
	    free ((char *) new->fullname);
	    free ((char *) new);
	    return 0;
    	}
    	memmove( new->hostaddr.data, hostAddr.data, hostAddr.length);
	new->connectionType = connectionType;
	new->hostname = *hostname;

    	*names = new;
    	new->next = 0;
	NameTableSize++;
    }
    else
    {
	new = *names;
	free (new->fullname);
	XdmcpDisposeARRAY8 (&new->status);
	XdmcpDisposeARRAY8 (hostname);
    }
    new->willing = willing;
    new->status = *status;

    hostname = &new->hostname;
    fulllen = hostname->length;
    if (fulllen < 30)
	fulllen = 30;
    new->fullname = malloc (fulllen + status->length + 10);
    if (!new->fullname)
    {
        new->fullname = (char *) ReadCatalog(MC_CHOOSER_SET, 
				    	     MC_HOST_UNKNOWN_LABEL, 
				    	     MC_DEF_HOST_UNKNOWN_LABEL );
    }
    else
    {
	unsigned char* host=hostname->data;
	int host_len=hostname->length;

	if (host_len <= 0) {
            host = ReadCatalog(MC_CHOOSER_SET, 
			       MC_HOST_UNKNOWN_LABEL, 
			       MC_DEF_HOST_UNKNOWN_LABEL );
	    host_len = strlen((char*) host);
	}

	sprintf (new->fullname, "%-30.*s %*.*s", host_len, host,
		 status->length, status->length, status->data);
    }
    if (update)
	RebuildTable (NameTableSize);
    else
	RebuildTableAdd (NameTableSize);
    return 1;
}

static
DisposeHostname (host)
    HostName	*host;
{
    XdmcpDisposeARRAY8 (&host->hostname);
    XdmcpDisposeARRAY8 (&host->hostaddr);
    XdmcpDisposeARRAY8 (&host->status);
    free ((char *) host->fullname);
    free ((char *) host);
}

static void
RemoveHostname (host)
    HostName	*host;
{
    HostName	**prev, *hosts;

    prev = &hostNamedb;;
    for (hosts = hostNamedb; hosts; hosts = hosts->next)
    {
	if (hosts == host)
	    break;
	prev = &hosts->next;
    }
    if (!hosts)
	return;
    *prev = host->next;
    DisposeHostname (host);
    NameTableSize--;
    RebuildTable (NameTableSize);
}

static
EmptyHostnames ()
{
    HostName	*hosts, *next;

    for (hosts = hostNamedb; hosts; hosts = next)
    {
	next = hosts->next;
	DisposeHostname (hosts);
    }
    NameTableSize = 0;
    hostNamedb = 0;
    RebuildTable (NameTableSize);
}

/* ARGSUSED */
static void
ReceivePacket (closure, source, id)
    XtPointer	closure;
    int		*source;
    XtInputId	*id;
{
    XdmcpHeader	    header;
    ARRAY8	    authenticationName;
    ARRAY8	    hostname;
    ARRAY8	    status;
    int		    saveHostname = 0;
    struct sockaddr addr;
    int		    addrlen;

    addrlen = sizeof (addr);
    if (!XdmcpFill (socketFD, &buffer, &addr, &addrlen))
	return;
    if (!XdmcpReadHeader (&buffer, &header))
	return;
    if (header.version != XDM_PROTOCOL_VERSION)
	return;
    hostname.data = 0;
    status.data = 0;
    authenticationName.data = 0;
    switch (header.opcode) {
    case WILLING:
    	if (XdmcpReadARRAY8 (&buffer, &authenticationName) &&
	    XdmcpReadARRAY8 (&buffer, &hostname) &&
	    XdmcpReadARRAY8 (&buffer, &status))
    	{
	    if (header.length == 6 + authenticationName.length +
	    	hostname.length + status.length)
	    {
		if (AddHostname (&hostname, &status, &addr, header.opcode == (int) WILLING))
		    saveHostname = 1;
	    }
    	}
	XdmcpDisposeARRAY8 (&authenticationName);
	break;
    case UNWILLING:
    	if (XdmcpReadARRAY8 (&buffer, &hostname) &&
	    XdmcpReadARRAY8 (&buffer, &status))
    	{
	    if (header.length == 4 + hostname.length + status.length)
	    {
		if (AddHostname (&hostname, &status, &addr, header.opcode == (int) WILLING))
		    saveHostname = 1;

	    }
    	}
	break;
    default:
	break;
    }
    if (!saveHostname)
    {
    	XdmcpDisposeARRAY8 (&hostname);
    	XdmcpDisposeARRAY8 (&status);
    }
}

void
RegisterHostaddr (addr, len, type)
    struct sockaddr *addr;
    int		    len;
    xdmOpCode	    type;
{
    HostAddr		*host, **prev;

    host = (HostAddr *) malloc (sizeof (HostAddr));
    if (!host)
	return;
    host->addr = (struct sockaddr *) malloc (len);
    if (!host->addr)
    {
	free ((char *) host);
	return;
    }
    memmove( (char *) host->addr, (char *) addr, len);
    host->addrlen = len;
    host->type = type;
    for (prev = &hostAddrdb; *prev; prev = &(*prev)->next)
	;
    *prev = host;
    host->next = NULL;
}

/*
 * Register the address for this host.
 * Called with each of the names on the command line.
 * The special name "BROADCAST" looks up all the broadcast
 *  addresses on the local host.
 */

void
RegisterHostname (name)
    char    *name;
{
    struct hostent	*hostent;
    struct sockaddr_in	in_addr;
    struct ifconf	ifc;
    register struct ifreq *ifr;
    struct sockaddr	broad_addr;
    char		buf[2048];
    int			n;

    if (!strcmp (name, BROADCAST_HOSTNAME))
    {
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl (socketFD, (int) SIOCGIFCONF, (char *) &ifc) < 0)
	    return;
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
	    if (ifr->ifr_addr.sa_family != AF_INET)
		continue;

	    broad_addr = ifr->ifr_addr;
	    ((struct sockaddr_in *) &broad_addr)->sin_addr.s_addr =
		htonl (INADDR_BROADCAST);
#ifdef SIOCGIFBRDADDR
	    {
		struct ifreq    broad_req;
    
		broad_req = *ifr;
		if (ioctl (socketFD, SIOCGIFFLAGS, (char *) &broad_req) != -1 &&
		    (broad_req.ifr_flags & IFF_BROADCAST) &&
		    (broad_req.ifr_flags & IFF_UP)
		    )
		{
		    broad_req = *ifr;
		    if (ioctl (socketFD, SIOCGIFBRDADDR, &broad_req) != -1)
			broad_addr = broad_req.ifr_addr;
		    else
			continue;
		}
		else
		    continue;
	    }
#endif
	    in_addr = *((struct sockaddr_in *) &broad_addr);
	    in_addr.sin_port = htons (XDM_UDP_PORT);
#ifdef BSD44SOCKETS
	    in_addr.sin_len = sizeof(in_addr);
#endif
	    RegisterHostaddr ((struct sockaddr *)&in_addr, sizeof (in_addr),
			      BROADCAST_QUERY);
	}
    }
    else
    {

	/* address as hex string, e.g., "12180022" (depreciated) */
	if (strlen(name) == 8 &&
	    FromHex(name, (char *)&in_addr.sin_addr, strlen(name)) == 0)
	{
	    in_addr.sin_family = AF_INET;
	}
	/* Per RFC 1123, check first for IP address in dotted-decimal form */
	else if ((in_addr.sin_addr.s_addr = inet_addr(name)) != -1)
	    in_addr.sin_family = AF_INET;
	else
	{
	    hostent = gethostbyname (name);
	    if (!hostent)
		return;
	    if (hostent->h_addrtype != AF_INET || hostent->h_length != 4)
	    	return;
	    in_addr.sin_family = hostent->h_addrtype;
	    memmove( &in_addr.sin_addr, hostent->h_addr, 4);
	}
	in_addr.sin_port = htons (XDM_UDP_PORT);
#ifdef BSD44SOCKETS
	in_addr.sin_len = sizeof(in_addr);
#endif
	RegisterHostaddr ((struct sockaddr *)&in_addr, sizeof (in_addr),
			  QUERY);
    }
}

static ARRAYofARRAY8	AuthenticationNames;

void
RegisterAuthenticationName (name, namelen)
    char    *name;
    int	    namelen;
{
    ARRAY8Ptr	authName;
    if (!XdmcpReallocARRAYofARRAY8 (&AuthenticationNames,
				    AuthenticationNames.length + 1))
	return;
    authName = &AuthenticationNames.data[AuthenticationNames.length-1];
    if (!XdmcpAllocARRAY8 (authName, namelen))
	return;
    memmove( authName->data, name, namelen);
}

InitXDMCP (argv)
    char    **argv;
{
    int	soopts = 1;
    XdmcpHeader	header;
    int	i;
    int optlen;

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) BROADCAST_QUERY;
    header.length = 1;
    for (i = 0; i < (int)AuthenticationNames.length; i++)
	header.length += 2 + AuthenticationNames.data[i].length;
    XdmcpWriteHeader (&broadcastBuffer, &header);
    XdmcpWriteARRAYofARRAY8 (&broadcastBuffer, &AuthenticationNames);

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) QUERY;
    header.length = 1;
    for (i = 0; i < (int)AuthenticationNames.length; i++)
	header.length += 2 + AuthenticationNames.data[i].length;
    XdmcpWriteHeader (&directBuffer, &header);
    XdmcpWriteARRAYofARRAY8 (&directBuffer, &AuthenticationNames);
    if ((socketFD = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	return 0;
#ifdef SO_BROADCAST
    soopts = 1;
    if (setsockopt (socketFD, SOL_SOCKET, SO_BROADCAST, (char *)&soopts, sizeof (soopts)) < 0)
	perror ("setsockopt");
#endif
    
    XtAddInput (socketFD, (XtPointer) XtInputReadMask, ReceivePacket,
		(XtPointer) 0);
    while (*argv)
    {
	RegisterHostname (*argv);
	++argv;
    }
    pingTry = 0;
    PingHosts ((XtPointer)NULL, (XtIntervalId *)NULL);
    return 1;
}

void
Choose (h)
    HostName	*h;
{
    if (app_resources.xdmAddress)
    {
	struct sockaddr_in  in_addr;
	struct sockaddr	*addr;
	int		family;
	int		len;
	int		fd;
	char		buf[1024];
	XdmcpBuffer	buffer;
	char		*xdm;

	xdm = (char *) app_resources.xdmAddress->data;
	family = (xdm[0] << 8) + xdm[1];
	switch (family) {
	case AF_INET:
#ifdef BSD44SOCKETS
	    in_addr.sin_len = sizeof(in_addr);
#endif
	    in_addr.sin_family = family;
	    memmove( &in_addr.sin_port, xdm + 2, 2);
	    memmove( &in_addr.sin_addr, xdm + 4, 4);
	    addr = (struct sockaddr *) &in_addr;
	    len = sizeof (in_addr);
	    break;
	}
	if ((fd = socket (family, SOCK_STREAM, 0)) == -1)
	{
	    fprintf (stderr, "Cannot create response socket\n");
	    exit (REMANAGE_DISPLAY);
	}
	if (connect (fd, addr, len) == -1)
	{
	    fprintf (stderr, "Cannot connect to xdm\n");
	    exit (REMANAGE_DISPLAY);
	}
	buffer.data = (BYTE *) buf;
	buffer.size = sizeof (buf);
	buffer.pointer = 0;
	buffer.count = 0;
	XdmcpWriteARRAY8 (&buffer, app_resources.clientAddress);
	XdmcpWriteCARD16 (&buffer, (CARD16) app_resources.connectionType);
	XdmcpWriteARRAY8 (&buffer, &h->hostaddr);
	write (fd, (char *)buffer.data, buffer.pointer);
	close (fd);
    }
    else
    {
	int i;

    	printf ("%u\n", h->connectionType);
    	for (i = 0; i < (int)h->hostaddr.length; i++)
	    printf ("%u%s", h->hostaddr.data[i],
		    i == h->hostaddr.length - 1 ? "\n" : " ");
    }
}

/* ARGSUSED */
void
DoAccept (w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    HostName		*h;
    XmStringTable	selectedItem;
    int			selectedCount;
    int			i;
    char		*text;

    /*********************************/
    /** see if anything is selected **/
    /*********************************/
    i = 0;
    XtSetArg(chooserArgs[i], XmNselectedItemCount, &selectedCount); i++;
    XtGetValues(chooser_list, chooserArgs, i);
    if (selectedCount != 1) {
	XBell (XtDisplay (toplevel), 0);
	return;
    }

    /**********************************************/
    /** retrieve the selected item from the list **/
    /**********************************************/
    i = 0;
    XtSetArg(chooserArgs[i], XmNselectedItems, &selectedItem); i++;
    XtGetValues(chooser_list, chooserArgs, i);
    if (!XmStringGetLtoR(selectedItem[0], XmFONTLIST_DEFAULT_TAG, &text))
	return;
    
    for (h = hostNamedb; h; h = h->next)
	if (!strcmp (text, h->fullname))
	{
	    Choose (h);
	}
    exit (OBEYSESS_DISPLAY);
}

/* ARGSUSED */
static void
DoCheckWilling (w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    HostName		*h;
    XmStringTable       selectedItem;
    int                 selectedCount;
    int                 i;
    char                *text;

    /*********************************/
    /** see if anything is selected **/
    /*********************************/
    i = 0;
    XtSetArg(chooserArgs[i], XmNselectedItemCount, &selectedCount); i++;
    XtGetValues(chooser_list, chooserArgs, i);
    if (selectedCount != 1) {
        return;
    }

    /**********************************************/
    /** retrieve the selected item from the list **/
    /**********************************************/
    i = 0;
    XtSetArg(chooserArgs[i], XmNselectedItems, &selectedItem); i++;
    XtGetValues(chooser_list, chooserArgs, i);
    if (!XmStringGetLtoR(selectedItem[0], XmFONTLIST_DEFAULT_TAG, &text))
        return;
    
    for (h = hostNamedb; h; h = h->next)
	if (!strcmp (text, h->fullname))
	    if (!h->willing)
		XmListDeselectAllItems (chooser_list);
}

/* ARGSUSED */
void
DoCancel (w, client_data, call_data)
    Widget w;
    int client_data;
    XtPointer call_data;
{
    exit (client_data);
}

/* ARGSUSED */
void
DoPing (w, client_data, call_data)
    Widget w;
    XtPointer client_data;
    XtPointer call_data;
{
    EmptyHostnames ();
    pingTry = 0;
    PingHosts ((XtPointer)NULL, (XtIntervalId *)NULL);
}

static XtActionsRec app_actions[] = {
    "Accept",	    (XtActionProc) DoAccept,
    "Cancel",	    (XtActionProc) DoCancel,
    "CheckWilling", (XtActionProc) DoCheckWilling,
    "Ping",	    (XtActionProc) DoPing,
};

static SIGVAL
#ifdef _NO_PROTO
Refresh()
#else
Refresh( int arg )
#endif /* _NO_PROTO */

{
	RefreshEH(NULL, NULL, NULL, NULL);
#if defined(SYSV) || defined(SVR4)
        signal (SIGHUP, Refresh);
#endif
}

main (argc, argv)
    char    **argv;
{
    Arg		position[3];
    Dimension   width, height;
    Position	x, y;
    int		i;
    char	*xsetup;

    /*
     *  set the HUP screen refresh signal handler 
     */

    (void) signal(SIGHUP, Refresh);

    /******************************/
    /** set the locale, and      **/
    /** open the message catalog **/
    /******************************/

    setlocale(LC_ALL, "");
    XtSetLanguageProc( NULL, NULL, NULL );

    if ( getenv("NLSPATH") == NULL )
        putenv(NLSPATH_ENV); 

    langenv = getenv("LANG");

    nl_fd = catopen(NLS_CATALOG,NL_CAT_LOCALE);

    /******************************************/
    /** set the font paths		     **/
    /******************************************/
    if ( (xsetup = getenv("XSETUP")) != NULL)
      if(system(xsetup) == -1)
	    fprintf (stderr, "dtchooser: Cannot source %s\n",xsetup);

    /******************************************/
    /** save argc and argv for RespondLangCB **/
    /******************************************/
    amChooser = 1;  /** tell RespondLangCB we are a chooser **/

    orig_argc = argc;
    orig_argv = (char **)malloc(sizeof(char *) * (orig_argc+1));
    for (i = 0; i < orig_argc; i++) {
	orig_argv[i] = argv[i];
    }
    orig_argv[orig_argc] = NULL;

    toplevel = XtInitialize (argv[0], "Dtlogin", options, XtNumber(options),
			     &argc, argv);

    XtVaSetValues(XmGetXmDisplay(XtDisplay(toplevel)),
                XmNdragInitiatorProtocolStyle, XmDRAG_NONE,
                NULL);

    XtAddConverter(XtRString, XtRARRAY8, CvtStringToARRAY8, NULL, 0);

    XtGetApplicationResources (toplevel, (XtPointer) &app_resources, resources,
			       XtNumber (resources), NULL, (Cardinal) 0);

    dpyinfo.dpy         = XtDisplay(toplevel);
/*    dpyinfo.name      = "";*/
    dpyinfo.screen      = DefaultScreen(dpyinfo.dpy);
    dpyinfo.root        = RootWindow   (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.depth       = DefaultDepth (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.width       = DisplayWidth (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.height      = DisplayHeight(dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.black_pixel = BlackPixel   (dpyinfo.dpy, dpyinfo.screen);
    dpyinfo.visual      = DefaultVisual(dpyinfo.dpy, dpyinfo.screen);

    /*
     *  build widgets...
     */

    MakeBackground();		/* login_shell, table, matte              */
    MakeLogo();                 /* logo, logo_pixmap, logo_shadow         */
    MakeButtons();		/* Push Buttons				  */
    MakeChooser();              /* Chooser List ...                       */

    if (appInfo.optionsDelay == 0 )
        MakeOptionsMenu();      /* make option_button pop-up menu         */
    else
        XtAddTimeOut((unsigned long) appInfo.optionsDelay * 1000, MakeOptionsProc, NULL);

#if 0
    /*
     * center ourselves on the screen
     */
    XtSetMappedWhenManaged(toplevel, FALSE);
    XtRealizeWidget (toplevel);
#endif
    XtRealizeWidget (login_shell);
XtPopup(login_shell, XtGrabNone);
#if 0
    XtSetArg (position[0], XtNwidth, &width);
    XtSetArg (position[1], XtNheight, &height);
    XtGetValues (login_shell, position, (Cardinal) 2);
    x = (Position)(WidthOfScreen (XtScreen (toplevel)) - width) / 2;
    y = (Position)(HeightOfScreen (XtScreen (toplevel)) - height) / 3;
    XtSetArg (position[0], XtNx, x);
    XtSetArg (position[1], XtNy, y);
    XtSetValues (login_shell, position, (Cardinal) 2);
#endif

    /*
     * Run
     */
#if 0
    XtMapWidget(toplevel);
#endif
    InitXDMCP (argv + 1);
    XtMainLoop ();
    exit(0);
    /*NOTREACHED*/
}


/* Converts the hex string s of length len into the byte array d.
   Returns 0 if s was a legal hex string, 1 otherwise.
   */
int
FromHex (s, d, len)
    char    *s, *d;
    int	    len;
{
    int	t;
    int ret = len&1;		/* odd-length hex strings are illegal */
    while (len >= 2)
    {
#define HexChar(c)  ('0' <= (c) && (c) <= '9' ? (c) - '0' : (c) - 'a' + 10)

	if (!ishexdigit(*s))
	    ret = 1;
	t = HexChar (*s) << 4;
	s++;
	if (!ishexdigit(*s))
	    ret = 1;
	t += HexChar (*s);
	s++;
	*d++ = t;
	len -= 2;
    }
    return ret;
}

/*ARGSUSED*/
static void
CvtStringToARRAY8 (args, num_args, fromVal, toVal)
    XrmValuePtr	args;
    Cardinal	*num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static ARRAY8Ptr	dest;
    char	*s;
    int		len;

    dest = (ARRAY8Ptr) XtMalloc (sizeof (ARRAY8));
    len = fromVal->size;
    s = (char *) fromVal->addr;
    if (!XdmcpAllocARRAY8 (dest, len >> 1))
	XtStringConversionWarning ((char *) fromVal->addr, XtRARRAY8);
    else
    {
	FromHex (s, (char *) dest->data, len);
    }
    toVal->addr = (caddr_t) &dest;
    toVal->size = sizeof (ARRAY8Ptr);
}
