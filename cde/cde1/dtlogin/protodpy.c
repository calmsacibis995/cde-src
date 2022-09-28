/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * $XConsortium: protodpy.c /main/cde1_maint/1 1995/10/09 00:11:32 pascale $
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * protodpy.c
 *
 * manage a collection of proto-displays.  These are displays for
 * which sessionID's have been generated, but no session has been
 * started.
 */

#include "dm.h"

struct protoDisplay	*protoDisplays;

struct protoDisplay * 
#ifdef _NO_PROTO
FindProtoDisplay( address, addrlen, displayNumber )
        struct sockaddr *address ;
        int addrlen ;
        CARD16 displayNumber ;
#else
FindProtoDisplay( struct sockaddr *address, int addrlen,
#if NeedWidePrototypes
        int displayNumber )
#else
        CARD16 displayNumber )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    struct protoDisplay	*pdpy;

    for (pdpy = protoDisplays; pdpy; pdpy=pdpy->next)
	if (pdpy->displayNumber == displayNumber &&
	    addressEqual ((char *)address, addrlen, (char *)pdpy->address, pdpy->addrlen))
	{
	    return pdpy;
	}
    return (struct protoDisplay *) 0;
}

void
#ifdef _NO_PROTO
TimeoutProtoDisplays( now )
        long now ;
#else
TimeoutProtoDisplays( long now )
#endif /* _NO_PROTO */
{
    struct protoDisplay	*pdpy, *next;

    for (pdpy = protoDisplays; pdpy; pdpy = next)
    {
	next = pdpy->next;
	if (pdpy->date < now - PROTO_TIMEOUT)
	    DisposeProtoDisplay (pdpy);
    }
}

struct protoDisplay * 
#ifdef _NO_PROTO
NewProtoDisplay( address, addrlen, displayNumber, connectionType, connectionAddress, sessionID )
        struct sockaddr *address ;
        int addrlen ;
        CARD16 displayNumber ;
        CARD16 connectionType ;
        ARRAY8Ptr connectionAddress ;
        CARD32 sessionID ;
#else
NewProtoDisplay( struct sockaddr *address, int addrlen,
#if NeedWidePrototypes
        int displayNumber,
        int connectionType,
#else
        CARD16 displayNumber,
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        ARRAY8Ptr connectionAddress, CARD32 sessionID )
#endif /* _NO_PROTO */
{
    struct protoDisplay	*pdpy;
    time_t  date;

    time (&date);
    TimeoutProtoDisplays (date);
    pdpy = (struct protoDisplay *) malloc (sizeof *pdpy);
    if (!pdpy)
	return NULL;
    pdpy->address = (struct sockaddr *) malloc (addrlen);
    if (!pdpy->address)
    {
	free ((char *) pdpy);
	return NULL;
    }
    pdpy->addrlen = addrlen;
    bcopy ((char *)address, (char *)pdpy->address, addrlen);
    pdpy->displayNumber = displayNumber;
    pdpy->connectionType = connectionType;
    pdpy->date = date;
    if (!XdmcpCopyARRAY8 (connectionAddress, &pdpy->connectionAddress))
    {
	free ((char *) pdpy->address);
	free ((char *) pdpy);
	return NULL;
    }
    pdpy->sessionID = sessionID;
    pdpy->fileAuthorization = (Xauth *) NULL;
    pdpy->xdmcpAuthorization = (Xauth *) NULL;
    pdpy->next = protoDisplays;
    protoDisplays = pdpy;
    return pdpy;
}

void
#ifdef _NO_PROTO
DisposeProtoDisplay( pdpy )
        struct protoDisplay *pdpy ;
#else
DisposeProtoDisplay( struct protoDisplay *pdpy )
#endif /* _NO_PROTO */
{
    struct protoDisplay	*p, *prev;

    prev = 0;
    for (p = protoDisplays; p; p=p->next)
    {
	if (p == pdpy)
	    break;
	prev = p;
    }
    if (!p)
	return;
    if (prev)
	prev->next = pdpy->next;
    else
	protoDisplays = pdpy->next;
    XdmcpDisposeARRAY8 (&pdpy->connectionAddress);
    if (pdpy->fileAuthorization)
	XauDisposeAuth (pdpy->fileAuthorization);
    if (pdpy->xdmcpAuthorization)
	XauDisposeAuth (pdpy->xdmcpAuthorization);
    free ((char *) pdpy->address);
    free ((char *) pdpy);
}
