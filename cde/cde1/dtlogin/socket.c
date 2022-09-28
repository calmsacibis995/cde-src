/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: socket.c /main/cde1_maint/3 1995/12/18 13:07:36 mgreess $
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
 * socket.c.  Support for XDMCP
 */

# include "dm.h"
# include "vgmsg.h"

# include	<sys/socket.h>
# include	<sys/un.h>
# include	<X11/X.h>
# include	<netdb.h>
# include	<time.h>


/*
 * interface to policy routines
 */


#define pS(s)	((s) ? ((char *) (s)) : "empty string")


/***************************************************************************
 *
 *  External variable declarations
 *
 ***************************************************************************/



/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static int  ProcessRequestSocket() ;
static int  all_query_respond() ;
static void broadcast_respond() ;
static int  direct_query_respond() ;
static void forward_respond() ;
static void indirect_respond() ;
static void manage() ;
static int  registerForwardHost() ;
static int  registerHostname() ;
static void request_respond() ;
static void send_accept() ;
static void send_decline() ;
static void send_failed() ;
static void send_refuse() ;
static void send_alive() ;
static void query_respond() ;
static void send_willing() ;
static void send_unwilling() ;


#else

static int  ProcessRequestSocket( void ) ;
static int  all_query_respond(
			struct sockaddr *from,
			int fromlen,
			ARRAYofARRAY8Ptr authenticationNames,
			xdmOpCode type) ;
static void broadcast_respond(
			struct sockaddr *from,
			int fromlen,
			int length) ;
static int  direct_query_respond(
			struct sockaddr *from,
			int fromlen,
			int length,
			xdmOpCode type) ;
static void forward_respond(
			struct sockaddr *from,
			int fromlen,
			int length) ;
static void indirect_respond(
			struct sockaddr *from,
			int fromlen,
			int length) ;
static void manage( 
                        struct sockaddr *from,
                        int fromlen,
                        int length) ;
static int  registerForwardHost(
			struct sockaddr *addr,
			int addrlen) ;
static int  registerHostname(
			char *name,
			int  namelen) ;
static void request_respond( 
                        struct sockaddr *from,
                        int fromlen,
                        int length) ;
static void send_alive( 
                        struct sockaddr *from,
                        int fromlen,
                        int length) ;
static void send_accept( 
			struct sockaddr *to,
                        int tolen,
                        CARD32 sessionID,
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr authenticationData,
                        ARRAY8Ptr authorizationName,
                        ARRAY8Ptr authorizationData) ;
static void send_decline( 
                        struct sockaddr *to,
                        int tolen,
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr authenticationData,
                        ARRAY8Ptr status) ;
static void send_failed( 
                        struct sockaddr *from,
                        int fromlen,
                        char *name,
                        CARD32 sessionID,
                        char *reason) ;
static void send_refuse( 
                        struct sockaddr *from,
                        int fromlen,
                        CARD32 sessionID) ;
static void send_willing(
                        struct sockaddr *from,
                        int fromlen,
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr status) ;
static void send_unwilling( 
                        struct sockaddr *from,
                        int fromlen,
                        ARRAY8Ptr authenticationName,
                        ARRAY8Ptr status) ;
static void query_respond(
			struct sockaddr *from,
			int fromlen,
			int length) ;

#endif /* _NO_PROTO */




/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

int	socketFd = -1;
int     chooserFd = -1;

FD_TYPE	WellKnownSocketsMask = {0};
int	WellKnownSocketsMax  = -1;



/***************************************************************************
 ***************************************************************************/

int 
#ifdef _NO_PROTO
CreateWellKnownSockets()
#else
CreateWellKnownSockets( void )
#endif /* _NO_PROTO */
{
    struct sockaddr_in	sock_addr;
    char		*name;

    if (request_port == 0)
	    return -1;
    Debug ("Creating socket %d\n", request_port);
    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if (socketFd == -1) {
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_FAIL_SOCK,MC_DEF_LOG_FAIL_SOCK),
		request_port);
	return -1;
    }
    name = localHostname ();
    registerHostname (name, strlen (name));
    RegisterCloseOnFork (socketFd);
    /* zero out the entire structure; this avoids 4.4 incompatibilities */
    bzero ((char *) &sock_addr, sizeof (sock_addr));
#ifdef BSD44SOCKETS
    sock_addr.sin_len = sizeof(sock_addr);
#endif
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons ((short) request_port);
    sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (socketFd, (struct sockaddr *)&sock_addr, sizeof (sock_addr)) == -1)
    {
	LogError(ReadCatalog(MC_LOG_SET,MC_LOG_ERR_BIND,MC_DEF_LOG_ERR_BIND), 
		   request_port, errno);
	close (socketFd);
	socketFd = -1;
        return -1;
    }
    WellKnownSocketsMax = socketFd;
    FD_SET (socketFd, &WellKnownSocketsMask);

    chooserFd = socket (AF_INET, SOCK_STREAM, 0);
    Debug ("Created chooser socket %d\n", chooserFd);
    if (chooserFd == -1)
    {
        LogError ((unsigned char *) "chooser socket creation failed\n");
        return -1;
    }
    listen (chooserFd, 5);
    if (chooserFd > WellKnownSocketsMax)
        WellKnownSocketsMax = chooserFd;
    FD_SET (chooserFd, &WellKnownSocketsMask);

    return 0;
}

GetChooserAddr (addr, lenp)
    char        *addr;
    int         *lenp;
{
    struct sockaddr     in_addr;
    int                 len;

    len = sizeof in_addr;
    if (getsockname (chooserFd, &in_addr, &len) < 0)
        return -1;
    bcopy ((char *) &in_addr, addr, len);
    *lenp = len;
    return 0;
}

void
#ifdef _NO_PROTO
DestroyWellKnownSockets()
#else
DestroyWellKnownSockets( void )
#endif /* _NO_PROTO */
{
    if (socketFd != -1)
    {
	close (socketFd);
	socketFd = -1;
    }
    if (chooserFd != -1)
    {
        close (chooserFd);
        chooserFd = -1;
    }
}

int 
#ifdef _NO_PROTO
AnyWellKnownSockets()
#else
AnyWellKnownSockets( void )
#endif /* _NO_PROTO */
{
    return socketFd != -1 || chooserFd != -1;
}

void
#ifdef _NO_PROTO
WaitForSomething()
#else
WaitForSomething( void )
#endif /* _NO_PROTO */
{
    FD_TYPE reads;
    struct timeval	timeout, *ptimeout;
    int	nready;
    extern int Rescan, ChildReady, wakeupTime;

    Debug ("WaitForSomething\n");
    if (AnyWellKnownSockets () && !ChildReady || wakeupTime > 0 ) {
	reads = WellKnownSocketsMask;

	if (wakeupTime >= 0 ) {
	    timeout.tv_sec  = wakeupTime;
	    timeout.tv_usec = 10;
	    ptimeout = &timeout;
	    Debug("Setting timer on select() for %d seconds.\n", wakeupTime);
	}
	else
	    ptimeout = NULL;
	    
	nready = select (WellKnownSocketsMax+1, &reads, 0, 0, ptimeout);
	Debug ("select() returns %d.  Rescan: %d  ChildReady: %d\n",
		nready, Rescan, ChildReady);
        if (nready > 0)
        {
            if (chooserFd >= 0 && FD_ISSET (chooserFd, &reads))
                ProcessChooserSocket (chooserFd);
            if (socketFd >= 0 && FD_ISSET (socketFd, &reads))
                ProcessRequestSocket ();
        }
	if (ChildReady)
	{
	    WaitForChild ();
	}
	else
	    StartDisplays();
    } else
	WaitForChild ();
}

/*
 * respond to a request on the UDP socket.
 */

static ARRAY8	Hostname;

static int 
#ifdef _NO_PROTO
registerHostname( name, namelen )
        char *name ;
        int namelen ;
#else
registerHostname( char *name, int namelen )
#endif /* _NO_PROTO */
{
    int	i;

    if (!XdmcpReallocARRAY8 (&Hostname, namelen))
	return -1;
    for (i = 0; i < namelen; i++)
	Hostname.data[i] = name[i];
    return 0;
}

static XdmcpBuffer	buffer;

static int 
#ifdef _NO_PROTO
ProcessRequestSocket()
#else
ProcessRequestSocket( void )
#endif /* _NO_PROTO */
{
    XdmcpHeader		header;
    struct sockaddr_in	addr;
    int			addrlen = sizeof addr;
    char		buf[64];
    
    Debug ("Processing request from socket\n");
    if (!XdmcpFill (socketFd, &buffer, &addr, &addrlen)) {
        Debug ("XdmcpFill failed\n");
	return -1;
    }
    if (!XdmcpReadHeader (&buffer, &header)) {
        Debug ("XdmcpReadHeader failed\n");
	return -1;
    }
    if (header.version != XDM_PROTOCOL_VERSION) {
        Debug ("XDMCP header version read was %d, expected %d\n",
               header.version, XDM_PROTOCOL_VERSION);
	return -1;
    }

    /*
     * put socket request type in debug message (if known)...
     */
     
    switch (header.opcode) {
    
    case BROADCAST_QUERY:	strcpy(buf, "BROADCAST_QUERY");	break;
    case QUERY:			strcpy(buf, "QUERY");		break;
    case INDIRECT_QUERY:	strcpy(buf, "INDIRECT_QUERY");	break;
    case FORWARD_QUERY:		strcpy(buf, "FORWARD_QUERY");	break;
    case REQUEST:		strcpy(buf, "REQUEST");		break;
    case MANAGE:		strcpy(buf, "MANAGE");		break;
    case KEEPALIVE:		strcpy(buf, "KEEPALIVE");	break;
    default:			strcpy(buf, "UNKNOWN");		break;
    }

    Debug ("header: version = %d, opcode = %d (%s), length = %d\n", 
    	    header.version, header.opcode, buf, header.length);
    switch (header.opcode)
    {
    case BROADCAST_QUERY:
	broadcast_respond ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case QUERY:
	query_respond ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case INDIRECT_QUERY:
	indirect_respond ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case FORWARD_QUERY:
	forward_respond ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case REQUEST:
	request_respond ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case MANAGE:
	manage ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    case KEEPALIVE:
	send_alive ((struct sockaddr *) &addr, addrlen, header.length);
	break;
    }

    return 0;
}

static void
#ifdef _NO_PROTO
query_respond( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
query_respond( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    Debug ("Query respond, request length = %d\n", length);
    direct_query_respond (from, fromlen, length, QUERY);
}

static void
#ifdef _NO_PROTO
broadcast_respond( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
broadcast_respond( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    direct_query_respond (from, fromlen, length, BROADCAST_QUERY);
}

static int 
#ifdef _NO_PROTO
direct_query_respond( from, fromlen, length, type )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
        xdmOpCode type ;
#else
direct_query_respond( struct sockaddr *from, int fromlen, int length,
		      xdmOpCode type )
#endif /* _NO_PROTO */
{
    ARRAYofARRAY8   queryAuthenticationNames;
    int		    expectedLen;
    int		    i;
    
    if (!XdmcpReadARRAYofARRAY8 (&buffer, &queryAuthenticationNames))
	return -1;
    expectedLen = 1;
    for (i = 0; i < queryAuthenticationNames.length; i++)
	expectedLen += 2 + queryAuthenticationNames.data[i].length;
    if (length == expectedLen)
	all_query_respond (from, fromlen, &queryAuthenticationNames, type);
    XdmcpDisposeARRAYofARRAY8 (&queryAuthenticationNames);
    return 0;
}

/*ARGSUSED*/
static void
sendForward (connectionType, address, closure)
    CARD16      connectionType;
    ARRAY8Ptr   address;
    char        *closure;
{
#ifdef AF_INET
    struct sockaddr_in      in_addr;
#endif
#ifdef AF_DECnet
#endif
    struct sockaddr         *addr;
    int                     addrlen;

    switch (connectionType)
    {
#ifdef AF_INET
    case FamilyInternet:
        addr = (struct sockaddr *) &in_addr;
        bzero ((char *) &in_addr, sizeof (in_addr));
        in_addr.sin_family = AF_INET;
        in_addr.sin_port = htons ((short) XDM_UDP_PORT);
        if (address->length != 4)
            return;
        bcopy (address->data, (char *) &in_addr.sin_addr, address->length);
        addrlen = sizeof (struct sockaddr_in);
        break;
#endif
#ifdef AF_DECnet
    case FamilyDECnet:
#endif
    default:
        return;
    }
    XdmcpFlush (socketFd, &buffer, addr, addrlen);
}

extern char *NetaddrAddress();
extern char *NetaddrPort();

static void
ClientAddress (from, addr, port, type)
    struct sockaddr *from;
    ARRAY8Ptr       addr, port; /* return */
    CARD16          *type;      /* return */
{
    int length, family;
    char *data;

    data = NetaddrPort(from, &length);
    XdmcpAllocARRAY8 (port, length);
    bcopy (data, port->data, length);
    port->length = length;

    family = ConvertAddr((XdmcpNetaddr) from, &length, &data);
    XdmcpAllocARRAY8 (addr, length);
    bcopy (data, addr->data, length);
    addr->length = length;

    *type = family;
}

static void
#ifdef _NO_PROTO
indirect_respond( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
indirect_respond( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    ARRAYofARRAY8   queryAuthenticationNames;
    ARRAY8          clientAddress;
    ARRAY8          clientPort;
    CARD16          connectionType;
    int             expectedLen;
    int             i;
    XdmcpHeader     header;
    int             localHostAsWell;

    Debug ("Indirect respond %d\n", length);
    if (!XdmcpReadARRAYofARRAY8 (&buffer, &queryAuthenticationNames))
        return;
    expectedLen = 1;
    for (i = 0; i < (int)queryAuthenticationNames.length; i++)
        expectedLen += 2 + queryAuthenticationNames.data[i].length;
    if (length == expectedLen)
    {
        ClientAddress (from, &clientAddress, &clientPort, &connectionType);
        /*
         * set up the forward query packet
         */
        header.version = XDM_PROTOCOL_VERSION;
        header.opcode = (CARD16) FORWARD_QUERY;
        header.length = 0;
        header.length += 2 + clientAddress.length;
        header.length += 2 + clientPort.length;
        header.length += 1;
        for (i = 0; i < (int)queryAuthenticationNames.length; i++)
            header.length += 2 + queryAuthenticationNames.data[i].length;
        XdmcpWriteHeader (&buffer, &header);
        XdmcpWriteARRAY8 (&buffer, &clientAddress);
        XdmcpWriteARRAY8 (&buffer, &clientPort);
        XdmcpWriteARRAYofARRAY8 (&buffer, &queryAuthenticationNames);

        localHostAsWell = ForEachMatchingIndirectHost (&clientAddress, connectionType, sendForward, (char *) 0);

        XdmcpDisposeARRAY8 (&clientAddress);
        XdmcpDisposeARRAY8 (&clientPort);
        if (localHostAsWell)
            all_query_respond (from, fromlen, &queryAuthenticationNames,
                           INDIRECT_QUERY);
    }
    else
    {
        Debug ("Indirect length error got %d expect %d\n", length, expectedLen);
    }
    XdmcpDisposeARRAYofARRAY8 (&queryAuthenticationNames);
}

static void
#ifdef _NO_PROTO
forward_respond( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
forward_respond( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    ARRAY8	    clientAddress;
    ARRAY8	    clientPort;
    ARRAYofARRAY8   authenticationNames;
    struct sockaddr *client;
    int		    clientlen;
    int		    expectedLen;
    int		    i;
    
    Debug ("Forward respond, request length = %d\n", length);
    clientAddress.length = 0;
    clientAddress.data = 0;
    clientPort.length = 0;
    clientPort.data = 0;
    authenticationNames.length = 0;
    authenticationNames.data = 0;
    if (XdmcpReadARRAY8 (&buffer, &clientAddress) &&
	XdmcpReadARRAY8 (&buffer, &clientPort) &&
	XdmcpReadARRAYofARRAY8 (&buffer, &authenticationNames))
    {
	expectedLen = 0;
	expectedLen += 2 + clientAddress.length;
	expectedLen += 2 + clientPort.length;
        expectedLen += 1;           /* authenticationNames */
	for (i = 0; i < authenticationNames.length; i++)
	    expectedLen += 2 + authenticationNames.data[i].length;
	if (length == expectedLen)
	{
            int j;

            j = 0;
            for (i = 0; i < (int)clientPort.length; i++)
                j = j * 256 + clientPort.data[i];
            Debug ("Forward client address (port %d)", j);
            for (i = 0; i < (int)clientAddress.length; i++)
                Debug (" %d", clientAddress.data[i]);
            Debug ("\n");

    	    switch (from->sa_family)
    	    {
#ifdef AF_INET
	    case AF_INET:
		{
		    struct sockaddr_in	in_addr;

		    if (clientAddress.length != 4 ||
		        clientPort.length != 2)
		    {
			goto badAddress;
		    }
                    bzero ((char *) &in_addr, sizeof (in_addr));
		    in_addr.sin_family = AF_INET;
		    bcopy (clientAddress.data, &in_addr.sin_addr, 4);
		    bcopy (clientPort.data, &in_addr.sin_port, 2);
		    client = (struct sockaddr *) &in_addr;
		    clientlen = sizeof (in_addr);
		}
		break;
#endif
#ifdef AF_UNIX
	    case AF_UNIX:
		{
		    struct sockaddr_un	un_addr;

		    if (clientAddress.length >= sizeof (un_addr.sun_path))
			goto badAddress;
                    bzero ((char *) &un_addr, sizeof (un_addr));
		    un_addr.sun_family = AF_UNIX;
		    bcopy (clientAddress.data, un_addr.sun_path, clientAddress.length);
		    un_addr.sun_path[clientAddress.length] = '\0';
		    client = (struct sockaddr *) &un_addr;
		    clientlen = sizeof (un_addr);
		}
		break;
#endif
#ifdef AF_CHAOS
	    case AF_CHAOS:
		goto badAddress;
		break;
#endif
#ifdef AF_DECnet
	    case AF_DECnet:
		goto badAddress;
		break;
#endif
    	    }
	    all_query_respond (client, clientlen, &authenticationNames,
			       FORWARD_QUERY);
	}
	else
	{
	    Debug ("Forward length error got %d expect %d\n", length, expectedLen);
	}
    }
badAddress:
    XdmcpDisposeARRAY8 (&clientAddress);
    XdmcpDisposeARRAY8 (&clientPort);
    XdmcpDisposeARRAYofARRAY8 (&authenticationNames);
}

static int 
#ifdef _NO_PROTO
all_query_respond( from, fromlen, authenticationNames, type )
        struct sockaddr *from ;
        int fromlen ;
        ARRAYofARRAY8Ptr authenticationNames ;
        xdmOpCode type ;
#else
all_query_respond( struct sockaddr *from, int fromlen,
	           ARRAYofARRAY8Ptr authenticationNames,
	           xdmOpCode type )
#endif /* _NO_PROTO */
{
    ARRAY8Ptr   authenticationName;
    ARRAY8      status;
    ARRAY8      addr;
    CARD16      connectionType;
    int         length;

    connectionType =
	ConvertAddr((XdmcpNetaddr) from, &length, (char**) &(addr.data));
    addr.length = length;       /* convert int to short */
    Debug ("all_query_respond: conntype=%d, addr=%lx, len=%d\n",
           connectionType, *(addr.data), addr.length);
    if (connectionType < 0)
        return -1;

    if (type == INDIRECT_QUERY)
        RememberIndirectClient (&addr, connectionType);
    else
        ForgetIndirectClient (&addr, connectionType);

    authenticationName = ChooseAuthentication (authenticationNames);
    if (Willing (&addr, connectionType, authenticationName, &status, type))
        send_willing (from, fromlen, authenticationName, &status);
    else
        if (type == QUERY)
            send_unwilling (from, fromlen, authenticationName, &status);
   XdmcpDisposeARRAY8 (&status);

   return 0;
}

static void
#ifdef _NO_PROTO
send_willing( from, fromlen, authenticationName, status )
        struct sockaddr *from ;
        int fromlen ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr status ;
#else
send_willing( struct sockaddr *from, int fromlen, ARRAY8Ptr authenticationName,
	      ARRAY8Ptr status )
#endif /* _NO_PROTO */
{
    XdmcpHeader	header;

    Debug ("send WILLING %*.*s %*.*s\n", authenticationName->length,
					 authenticationName->length,
					 pS(authenticationName->data),
					 status->length,
					 status->length,
					 pS(status->data));
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) WILLING;
    header.length = 6 + authenticationName->length +
		    Hostname.length + status->length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteARRAY8 (&buffer, authenticationName);
    XdmcpWriteARRAY8 (&buffer, &Hostname);
    XdmcpWriteARRAY8 (&buffer, status);
    XdmcpFlush (socketFd, &buffer, from, fromlen);
}

static void 
#ifdef _NO_PROTO
send_unwilling( from, fromlen, authenticationName, status )
        struct sockaddr *from ;
        int fromlen ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr status ;
#else
send_unwilling( struct sockaddr *from, int fromlen,
		ARRAY8Ptr authenticationName,
	        ARRAY8Ptr status )
#endif /* _NO_PROTO */
{
    XdmcpHeader	header;

    Debug ("Send UNWILLING %*.*s %*.*s\n", authenticationName->length,
					 authenticationName->length,
					 pS(authenticationName->data),
					 status->length,
					 status->length,
					 pS(status->data));
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) UNWILLING;
    header.length = 4 + Hostname.length + status->length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteARRAY8 (&buffer, &Hostname);
    XdmcpWriteARRAY8 (&buffer, status);
    XdmcpFlush (socketFd, &buffer, from, fromlen);
}

static unsigned long	globalSessionID;

#define NextSessionID()    (++globalSessionID)
    
static ARRAY8 outOfMemory = { (CARD16) 13, (CARD8Ptr) "Out of memory" };
static ARRAY8 noValidAddr = { (CARD16) 16, (CARD8Ptr) "No valid address" };
static ARRAY8 noValidAuth = { (CARD16) 22, (CARD8Ptr) "No valid authorization" };
static ARRAY8 noAuthentic = { (CARD16) 29, (CARD8Ptr) "XDM has no authentication key" };

static void
#ifdef _NO_PROTO
request_respond( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
request_respond( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    CARD16	    displayNumber;
    ARRAY16	    connectionTypes;
    ARRAYofARRAY8   connectionAddresses;
    ARRAY8	    authenticationName;
    ARRAY8	    authenticationData;
    ARRAYofARRAY8   authorizationNames;
    ARRAY8	    manufacturerDisplayID;
    ARRAY8Ptr	    reason;
    int		    expectlen;
    int		    i, j;
    struct protoDisplay  *pdpy;
    ARRAY8	    authorizationName, authorizationData;
    ARRAY8Ptr	    connectionAddress;

    Debug ("Request respond, request length = %d\n", length);
    connectionTypes.data = 0;
    connectionAddresses.data = 0;
    authenticationName.data = 0;
    authenticationData.data = 0;
    authorizationNames.data = 0;
    authorizationName.length = 0;
    authorizationData.length = 0;
    manufacturerDisplayID.data = 0;
    if (XdmcpReadCARD16 (&buffer, &displayNumber) &&
	XdmcpReadARRAY16 (&buffer, &connectionTypes) &&
	XdmcpReadARRAYofARRAY8 (&buffer, &connectionAddresses) &&
	XdmcpReadARRAY8 (&buffer, &authenticationName) &&
	XdmcpReadARRAY8 (&buffer, &authenticationData) &&
	XdmcpReadARRAYofARRAY8 (&buffer, &authorizationNames) &&
	XdmcpReadARRAY8 (&buffer, &manufacturerDisplayID))
    {
	expectlen = 0;
	expectlen += 2;				    /* displayNumber */
	expectlen += 1 + 2*connectionTypes.length;  /* connectionTypes */
	expectlen += 1;				    /* connectionAddresses */
	for (i = 0; i < connectionAddresses.length; i++)
	    expectlen += 2 + connectionAddresses.data[i].length;
	expectlen += 2 + authenticationName.length; /* authenticationName */
	expectlen += 2 + authenticationData.length; /* authenticationData */
	expectlen += 1;				    /* authoriationNames */
	for (i = 0; i < authorizationNames.length; i++)
	    expectlen += 2 + authorizationNames.data[i].length;
	expectlen += 2 + manufacturerDisplayID.length;	/* displayID */
	if (expectlen != length)
	{
	    Debug ("Request length error got %d expect %d\n", length, expectlen);
	    goto abort;
	}
	if (connectionTypes.length == 0 ||
	    connectionAddresses.length != connectionTypes.length)
	{
	    reason = &noValidAddr;
	    pdpy = 0;
	    goto decline;
	}
	if (pdpy = FindProtoDisplay (from, fromlen, displayNumber))
	    goto accept;
	reason = Accept (from, fromlen, displayNumber);
	if (reason)
	    goto decline;
	i = SelectConnectionTypeIndex (&connectionTypes,
				       &connectionAddresses);
	if (i < 0)
	{
	    reason = &noValidAddr;
	    goto decline;
	}
	if (authorizationNames.length == 0)
	    j = 0;
	else
	    j = SelectAuthorizationTypeIndex (&authenticationName,
					      &authorizationNames);
	if (j < 0)
	{
	    reason = &noValidAuth;
	    goto decline;
	}
	connectionAddress = &connectionAddresses.data[i];
	pdpy = NewProtoDisplay (from, fromlen,
				displayNumber,
				connectionTypes.data[i],
				connectionAddress,
				NextSessionID());
        Debug ("NewProtoDisplay 0x%x\n", pdpy);
	if (!pdpy)
	{
	    reason = &outOfMemory;
	    goto decline;
	}
	if (!CheckAuthentication (pdpy,
				  &manufacturerDisplayID,
				  &authenticationName,
				  &authenticationData))
	{
	    reason = &noAuthentic;
	    goto decline;
	}
	if (j < authorizationNames.length)
	{
	    Xauth   *auth;
	    SetProtoDisplayAuthorization (pdpy,
		(unsigned short) authorizationNames.data[j].length,
		(char *) authorizationNames.data[j].data);
	    auth = pdpy->xdmcpAuthorization;
	    if (!auth)
		auth = pdpy->fileAuthorization;
	    if (auth)
	    {
		authorizationName.length = auth->name_length;
		authorizationName.data = (CARD8Ptr) auth->name;
		authorizationData.length = auth->data_length;
		authorizationData.data = (CARD8Ptr) auth->data;
	    }
	}
	if (pdpy)
	{
accept:	    ;
	    send_accept (from, fromlen, pdpy->sessionID,
				        &authenticationName,
					&authenticationData,
					&authorizationName,
					&authorizationData);
	}
	else
	{
decline:    ;
	    send_decline (from, fromlen, &authenticationName,
				 &authenticationData,
				 reason);
	}
    }
abort:
    XdmcpDisposeARRAY16 (&connectionTypes);
    XdmcpDisposeARRAYofARRAY8 (&connectionAddresses);
    XdmcpDisposeARRAY8 (&authenticationName);
    XdmcpDisposeARRAY8 (&authenticationData);
    XdmcpDisposeARRAYofARRAY8 (&authorizationNames);
    XdmcpDisposeARRAY8 (&manufacturerDisplayID);
}

static void
#ifdef _NO_PROTO
send_accept( to, tolen, sessionID, authenticationName, authenticationData, authorizationName, authorizationData )
        struct sockaddr *to ;
        int tolen ;
        CARD32 sessionID ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr authenticationData ;
        ARRAY8Ptr authorizationName ;
        ARRAY8Ptr authorizationData ;
#else
send_accept( struct sockaddr *to, int tolen, CARD32 sessionID,
	     ARRAY8Ptr authenticationName,
	     ARRAY8Ptr authenticationData,
	     ARRAY8Ptr authorizationName,
	     ARRAY8Ptr authorizationData )
#endif /* _NO_PROTO */
{
    XdmcpHeader	header;

    Debug ("Send ACCEPT to session ID %d\n", sessionID);
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) ACCEPT;
    header.length = 4;			    /* session ID */
    header.length += 2 + authenticationName->length;
    header.length += 2 + authenticationData->length;
    header.length += 2 + authorizationName->length;
    header.length += 2 + authorizationData->length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteCARD32 (&buffer, sessionID);
    XdmcpWriteARRAY8 (&buffer, authenticationName);
    XdmcpWriteARRAY8 (&buffer, authenticationData);
    XdmcpWriteARRAY8 (&buffer, authorizationName);
    XdmcpWriteARRAY8 (&buffer, authorizationData);
    XdmcpFlush (socketFd, &buffer, to, tolen);
}
   
static void
#ifdef _NO_PROTO
send_decline( to, tolen, authenticationName, authenticationData, status )
        struct sockaddr *to ;
        int tolen ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr authenticationData ;
        ARRAY8Ptr status ;
#else
send_decline( struct sockaddr *to, int tolen,
	      ARRAY8Ptr authenticationName,
	      ARRAY8Ptr authenticationData,
	      ARRAY8Ptr status )
#endif /* _NO_PROTO */
{
    XdmcpHeader	header;

    Debug ("Send DECLINE %*.*s\n", status->length, status->length, pS(status->data));
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) DECLINE;
    header.length = 0;
    header.length += 2 + status->length;
    header.length += 2 + authenticationName->length;
    header.length += 2 + authenticationData->length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteARRAY8 (&buffer, status);
    XdmcpWriteARRAY8 (&buffer, authenticationName);
    XdmcpWriteARRAY8 (&buffer, authenticationData);
    XdmcpFlush (socketFd, &buffer, to, tolen);
}

static void
#ifdef _NO_PROTO
manage( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
manage( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    CARD32		sessionID;
    CARD16		displayNumber;
    ARRAY8		displayClass;
    int			expectlen;
    struct protoDisplay	*pdpy;
    struct display	*d;
    char		*name = NULL;
    char		*class = NULL;
    struct sockaddr	*from_save;
    ARRAY8              clientAddress, clientPort;
    CARD16              connectionType;

    Debug ("Manage respond, request length = %d\n", length);
    displayClass.data = 0;
    displayClass.length = 0;
    if (XdmcpReadCARD32 (&buffer, &sessionID) &&
	XdmcpReadCARD16 (&buffer, &displayNumber) &&
	XdmcpReadARRAY8 (&buffer, &displayClass))
    {
	expectlen = 4 +				/* session ID */
		    2 +				/* displayNumber */
		    2 + displayClass.length;	/* displayClass */
	if (expectlen != length)
	{
	    Debug ("Manage length error got %d expect %d\n", length, expectlen);
	    goto abort;
	}
	pdpy = FindProtoDisplay (from, fromlen, displayNumber);
        Debug ("Manage Session ID %d, pdpy 0x%x\n", sessionID, pdpy);
	if (!pdpy || pdpy->sessionID != sessionID)
	{
	    /*
	     * We may have already started a session for this display
	     * but it hasn't seen the response in the form of an
	     * XOpenDisplay() yet. So check if it is in the list of active
	     * displays, and if so check that the session id's match.
	     * If all this is true, then we have a duplicate request that
	     * can be ignored.
	     */
	    if (!pdpy 
		&& (d = FindDisplayByAddress(from, fromlen, displayNumber))
		&& d->sessionID == sessionID) {
		     Debug("Manage respond: got duplicate pkt, ignoring\n");
		     goto abort;
	    }
            Debug ("Session ID %d refused\n", sessionID);
            if (pdpy)
                Debug ("Existing Session ID %d\n", pdpy->sessionID);
	    send_refuse (from, fromlen, sessionID);
	}
	else
	{


	    name = NetworkAddressToName (pdpy->connectionType,
					 &pdpy->connectionAddress,
					 pdpy->displayNumber);
	    if (!name)
	    {
		send_failed (from, fromlen, "(no name)", sessionID, "out of memory");
		goto abort;
	    } 
	    else Debug ("Computed display name: %s\n", name);

	    d = FindDisplayByName (name);
	    if (d)
	    {
		if (d->name) 
		   Debug ("Terminating active session for %s\n", d->name);
		StopDisplay (d);
	    }
	    class = malloc (displayClass.length + 1);
	    if (!class)
	    {
		send_failed (from, fromlen, name, sessionID, "out of memory");
		goto abort;
	    }
	    if (displayClass.length)
	    {
		bcopy (displayClass.data, class, displayClass.length);
		class[displayClass.length] = '\0';
	    }
	    else {
		free(class);
		class = (char *) 0;
	    }
	    from_save = (struct sockaddr *) malloc (fromlen);
	    if (!from_save)
	    {
		send_failed (from, fromlen, name, sessionID, "out of memory");
		goto abort;
	    }
	    bcopy (from, from_save, fromlen);
	    d = NewDisplay (name, class);
            if (class) free(class);
            if (!d)
            {
                free ((char *) from_save);
                send_failed (from, fromlen, name, sessionID, "out of memory");
                goto abort;
            }
	    d->displayType.location = Foreign;
	    d->displayType.lifetime = Transient;
	    d->displayType.origin = FromXDMCP;
	    d->sessionID = pdpy->sessionID;
	    d->from = from_save;
	    d->fromlen = fromlen;
	    d->displayNumber = pdpy->displayNumber;
#ifdef BYPASSLOGIN
	    d->bypassLogin = 0;
#endif /* BYPASSLOGIN */
            ClientAddress (from, &clientAddress, &clientPort, &connectionType);
            d->useChooser = 0;
            if (IsIndirectClient (&clientAddress, connectionType))
            {
                Debug ("IsIndirectClient\n");
                ForgetIndirectClient (&clientAddress, connectionType);
                if (UseChooser (&clientAddress, connectionType))
                {
                    d->useChooser = 1;
                    if (d->name)
			Debug ("Use chooser for %s\n", d->name);
                }
            }
            d->clientAddr = clientAddress;
            d->connectionType = connectionType;
            XdmcpDisposeARRAY8 (&clientPort);
            d->authorization = pdpy->fileAuthorization;
            pdpy->fileAuthorization = 0;
            DisposeProtoDisplay (pdpy);
	    if (d->name && d->class) 
               Debug ("Starting display %s,%s\n", d->name, d->class);
	    else if (d->name)
               Debug ("Starting display %s\n", d->name);
            StartDisplay (d);
        }
    }
abort:
    XdmcpDisposeARRAY8 (&displayClass);
    if (name) free(name);
    if (class) free(class);
}

void
#ifdef _NO_PROTO
SendFailed( d, reason )
        struct display *d ;
        char *reason ;
#else
SendFailed( struct display *d, char *reason )
#endif /* _NO_PROTO */
{
    Debug ("Display start failed, sending Failed\n");
    send_failed (d->from, d->fromlen, d->name, d->sessionID, reason);
}

static void
#ifdef _NO_PROTO
send_failed( from, fromlen, name, sessionID, reason )
        struct sockaddr *from ;
        int fromlen ;
        char *name ;
        CARD32 sessionID ;
        char *reason ;
#else
send_failed( struct sockaddr *from, int fromlen, char *name, CARD32 sessionID,
	     char *reason )
#endif /* _NO_PROTO */
{
    static char	buf[256];
    XdmcpHeader	header;
    ARRAY8	status;

    sprintf (buf, "Session %d failed for display %s: %s",
	     sessionID, name, reason);
    Debug ("Send FAILED to session ID %d, msg = %s\n", sessionID, buf);
    status.length = strlen (buf);
    status.data = (CARD8Ptr) buf;
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) FAILED;
    header.length = 6 + status.length;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteCARD32 (&buffer, sessionID);
    XdmcpWriteARRAY8 (&buffer, &status);
    XdmcpFlush (socketFd, &buffer, from, fromlen);
}

static void
#ifdef _NO_PROTO
send_refuse( from, fromlen, sessionID )
        struct sockaddr *from ;
        int fromlen ;
        CARD32 sessionID ;
#else
send_refuse( struct sockaddr *from, int fromlen, CARD32 sessionID )
#endif /* _NO_PROTO */
{
    XdmcpHeader	header;

    Debug ("Send REFUSE to session ID %d\n", sessionID);
    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) REFUSE;
    header.length = 4;
    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteCARD32 (&buffer, sessionID);
    XdmcpFlush (socketFd, &buffer, from, fromlen);
}

static void
#ifdef _NO_PROTO
send_alive( from, fromlen, length )
        struct sockaddr *from ;
        int fromlen ;
        int length ;
#else
send_alive( struct sockaddr *from, int fromlen, int length )
#endif /* _NO_PROTO */
{
    CARD32		sessionID;
    CARD16		displayNumber;
    struct display	*d;
    XdmcpHeader		header;
    CARD8		sendRunning;
    CARD32		sendSessionID;

    Debug ("Send ALIVE\n");
    if (XdmcpReadCARD16 (&buffer, &displayNumber) &&
	XdmcpReadCARD32 (&buffer, &sessionID))
    {
	if (length == 6)
	{
	    d = FindDisplayBySessionID (sessionID);
	    if (!d) {
		d = FindDisplayByAddress (from, fromlen, displayNumber);
	    }
	    sendRunning = 0;
	    sendSessionID = 0;
	    if (d && d->status == running)
 	    {
		if (d->sessionID == sessionID)
		    sendRunning = 1;
		sendSessionID = d->sessionID;
	    }
	    header.version = XDM_PROTOCOL_VERSION;
	    header.opcode = (CARD16) ALIVE;
	    header.length = 5;
	    Debug ("send_alive(): %d %d\n", sendRunning, sendSessionID);
	    XdmcpWriteHeader (&buffer, &header);
	    XdmcpWriteCARD8 (&buffer, sendRunning);
	    XdmcpWriteCARD32 (&buffer, sendSessionID);
	    XdmcpFlush (socketFd, &buffer, from, fromlen);
	}
    }
}


# include	<arpa/nameser.h>
# include	<resolv.h>

char * 
#ifdef _NO_PROTO
NetworkAddressToName( connectionType, connectionAddress, displayNumber )
        CARD16 connectionType ;
        ARRAY8Ptr connectionAddress ;
        CARD16 displayNumber ;
#else
NetworkAddressToName(
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        ARRAY8Ptr connectionAddress,
#if NeedWidePrototypes
        int displayNumber )
#else
        CARD16 displayNumber )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    switch (connectionType)
    {
    case FamilyInternet:
	{
	    CARD8		*data;
	    struct hostent	*hostent;
	    char		*name;
	    char		*localhost;

	    data = connectionAddress->data;
	    hostent = gethostbyaddr ( (char *) data,
				     connectionAddress->length, AF_INET);

	    localhost = localHostname ();

#define getString(name,len)	((name = malloc (len + 1)) ? 1 : 0)

	    if (hostent)
	    {
		if (!strcmp (localhost, hostent->h_name))
		{
		    if (!getString (name, 10))
			return 0;
		    sprintf (name, ":%d", displayNumber);
		}
		else
		{
		    if (removeDomainname)
		    {
 			char localFDname[256];	/* Full domain name */
		    	char    *localDot, *remoteDot;
    
			/* check for a common domain name.  This
			 * could reduce names by recognising common
			 * super-domain names as well, but I don't think
			 * this is as useful, and will confuse more
			 * people
 			 */
 
			/* For fully qualified domain names:
 			   Get rid of domain part if it equals the
 			   local domain as this will prevent 
 			   a potential query to a (non-local) name server
 			   (performance! performance!)
 			 */
 			  
 			strcpy(localFDname,localhost);
 
 		    	if ((localDot = index (localFDname, '.')) &&
		            (remoteDot = index (hostent->h_name, '.')))
			{
 			    /* Get rid of domain part in name 
 			       if domains are identical
			    */

			    /* smash the name in place; it won't
			     * be needed later.
			     */

	Debug("Fully qualified local host domain name is %s\n",localFDname);
	Debug("Before domain part removal: foreign name is %s\n",hostent->h_name);

			    if (!strcmp (localDot+1, remoteDot+1))
				*remoteDot = '\0';

	Debug("After domain part removal:  foreign name is %s\n",hostent->h_name);
			}
		    }

		    if (!getString (name, strlen (hostent->h_name) + 10))
			return 0;
		    sprintf (name, "%s:%d", hostent->h_name, displayNumber);
		}
	    }
	    else
	    {
		if (!getString (name, 25))
		    return 0;
		sprintf(name, "%d.%d.%d.%d:%d",
			data[0], data[1], data[2], data[3], displayNumber);
	    }
	    return name;
	}
#ifdef DNET
    case FamilyDECnet:
	return NULL;
#endif
    default:
	return NULL;
    }
}


char * 
#ifdef _NO_PROTO
NetworkAddressToHostname( connectionType, connectionAddress )
        CARD16 connectionType ;
        ARRAY8Ptr connectionAddress ;
#else
NetworkAddressToHostname(
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        ARRAY8Ptr connectionAddress )
#endif /* _NO_PROTO */
{
    char    *name = 0;

    switch (connectionType)
    {
    case FamilyInternet:
	{
            struct hostent      *hostent;
            char dotted[20];
            char *local_name;

            hostent = gethostbyaddr ((char *)connectionAddress->data,
                                     connectionAddress->length, AF_INET);

            if (hostent)
                local_name = hostent->h_name;
            else {
                /* can't get name, so use emergency fallback */
                sprintf(dotted, "%d.%d.%d.%d",
                        connectionAddress->data[0],
                        connectionAddress->data[1],
                        connectionAddress->data[2],
                        connectionAddress->data[3]);
                local_name = dotted;
            }
            if (!getString (name, strlen (local_name)))
                break;
            strcpy (name, local_name);
            break;
	}
#ifdef DNET
    case FamilyDECnet:
	break;
#endif /* DNET */
    default:
	break;
    }
    return name;
}

static char localHostbuf[256];
static int  gotLocalHostname;

char * 
#ifdef _NO_PROTO
localHostname()
#else
localHostname( void )
#endif /* _NO_PROTO */
{
    if (!gotLocalHostname)
    {
	XmuGetHostname (localHostbuf, sizeof (localHostbuf) - 1);
	gotLocalHostname = 1;
    }
    return localHostbuf;
}

