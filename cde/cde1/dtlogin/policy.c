/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: policy.c /main/cde1_maint/1 1995/10/09 00:11:15 pascale $
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
 * policy.c.  Implement site-dependent policy for XDMCP connections
 */

# include "dm.h"

static ARRAY8 noAuthentication = { (CARD16) 0, (CARD8Ptr) 0 };
static ARRAY8 loopbackAddress  = { (CARD16) 0, (CARD8Ptr) 0 };

typedef struct _XdmAuth {
    ARRAY8  authentication;
    ARRAY8  authorization;
} XdmAuthRec, *XdmAuthPtr;

XdmAuthRec auth[] = {
#ifdef HASDES
{ {(CARD16) 20, (CARD8 *) "XDM-AUTHENTICATION-1"},
  {(CARD16) 19, (CARD8 *) "XDM-AUTHORIZATION-1"},
},
#endif
{ {(CARD16) 0, (CARD8 *) 0},
  {(CARD16) 0, (CARD8 *) 0},
}
};

#define NumAuth	(sizeof auth / sizeof auth[0])



/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/
#ifdef _NO_PROTO

static char * WillingMsg() ;

#else

static char * WillingMsg( void ) ;


#endif /* _NO_PROTO */
/********    End Local Function Declarations    ********/





ARRAY8Ptr 
#ifdef _NO_PROTO
ChooseAuthentication( authenticationNames )
        ARRAYofARRAY8Ptr authenticationNames ;
#else
ChooseAuthentication( ARRAYofARRAY8Ptr authenticationNames )
#endif /* _NO_PROTO */
{
    int	i, j;

    for (i = 0; i < authenticationNames->length; i++)
	for (j = 0; j < NumAuth; j++)
	    if (XdmcpARRAY8Equal (&authenticationNames->data[i],
				  &auth[j].authentication))
		return &authenticationNames->data[i];
    return &noAuthentication;
}

int 
#ifdef _NO_PROTO
CheckAuthentication( pdpy, displayID, name, data )
        struct protoDisplay *pdpy ;
        ARRAY8Ptr displayID ;
        ARRAY8Ptr name ;
        ARRAY8Ptr data ;
#else
CheckAuthentication( struct protoDisplay *pdpy, ARRAY8Ptr displayID,
        	     ARRAY8Ptr name, ARRAY8Ptr data )
#endif /* _NO_PROTO */
{
#ifdef HASDES
    if (name->length && !strncmp (name->data, "XDM-AUTHENTICATION-1", 20))
	return XdmCheckAuthentication (pdpy, displayID, name, data);
#endif
    return TRUE;
}

int 
#ifdef _NO_PROTO
SelectAuthorizationTypeIndex( authenticationName, authorizationNames )
        ARRAY8Ptr authenticationName ;
        ARRAYofARRAY8Ptr authorizationNames ;
#else
SelectAuthorizationTypeIndex( ARRAY8Ptr authenticationName,
			      ARRAYofARRAY8Ptr authorizationNames )
#endif /* _NO_PROTO */
{
    int	i, j;

    for (j = 0; j < NumAuth; j++)
	if (XdmcpARRAY8Equal (authenticationName,
			      &auth[j].authentication))
	    break;
    if (j < NumAuth)
    {
    	for (i = 0; i < authorizationNames->length; i++)
	    if (XdmcpARRAY8Equal (&authorizationNames->data[i],
				  &auth[j].authorization))
	    	return i;
    }
    for (i = 0; i < authorizationNames->length; i++)
	if (ValidAuthorization (authorizationNames->data[i].length,
				(char *) authorizationNames->data[i].data))
	    return i;
    return -1;
}

#if 0
int 
#ifdef _NO_PROTO
Willing( addr, addrlen, authenticationName, status )
        struct sockaddr *addr ;
        int addrlen ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr status ;
#else
Willing( struct sockaddr *addr, int addrlen, ARRAY8Ptr authenticationName,
	 ARRAY8Ptr status )
#endif /* _NO_PROTO */
#endif

int 
#ifdef _NO_PROTO
Willing( addr, connectionType, authenticationName, status, type )
        ARRAY8Ptr addr ;
        CARD16 connectionType ;
        ARRAY8Ptr authenticationName ;
        ARRAY8Ptr status ;
        xdmOpCode type ;
#else
Willing(
        ARRAY8Ptr addr,
#if NeedWidePrototypes
        int connectionType,
#else
        CARD16 connectionType,
#endif /* NeedWidePrototypes */
        ARRAY8Ptr authenticationName,
        ARRAY8Ptr status,
        xdmOpCode type )
#endif /* _NO_PROTO */
{
    static char	statusBuf[256];
    int		ret;
#if 0
    extern char	*localHostname ();
#endif
    ret = AcceptableDisplayAddress (addr, connectionType, type);
    if (!ret)
	sprintf (statusBuf, "Display not authorized to connect");
    else
	sprintf (statusBuf, "%s", WillingMsg());
#if 0
    	sprintf (statusBuf, "host %s", localHostname());
#endif

    status->length = strlen (statusBuf);
    status->data = (CARD8Ptr) malloc (status->length);
    if (!status->data)
	status->length = 0;
    else
	bcopy (statusBuf, (char *)status->data, status->length);
    return ret;
}

ARRAY8Ptr 
#ifdef _NO_PROTO
Accept( from, fromlen, displayNumber )
        struct sockaddr *from ;
        int fromlen ;
        CARD16 displayNumber ;
#else
Accept( struct sockaddr *from, int fromlen,
#if NeedWidePrototypes
        int displayNumber )
#else
        CARD16 displayNumber )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    return 0;
}

int 
#ifdef _NO_PROTO
SelectConnectionTypeIndex( connectionTypes, connectionAddresses )
        ARRAY16Ptr connectionTypes ;
        ARRAYofARRAY8Ptr connectionAddresses ;
#else
SelectConnectionTypeIndex( ARRAY16Ptr connectionTypes,
			   ARRAYofARRAY8Ptr connectionAddresses )
#endif /* _NO_PROTO */
{

    int i;
    
    /*
     *  the current selection policy is to use the first connection address
     *  that is not the loopback address...
     */
     
    /*
     *  initialize loopback address array if not already done so...
     *
     */
    if (loopbackAddress.length == 0 &&
	XdmcpAllocARRAY8 (&loopbackAddress, 4) ) {

	loopbackAddress.data[0] = 127;
	loopbackAddress.data[1] = 0;
	loopbackAddress.data[2] = 0;
	loopbackAddress.data[3] = 1;
    }
    
    for (i = 0; i < connectionAddresses->length; i++) {
	if (!XdmcpARRAY8Equal (&connectionAddresses->data[i],
				&loopbackAddress))
	    break;
    }

    return i;
}




/***************************************************************************
 *
 *  WillingMsg
 *
 *  Generate a message for the "Willing" status field.
 *  
 ***************************************************************************/

# define LINEBUFSIZE 32

static char *
#ifdef _NO_PROTO
WillingMsg()
#else
WillingMsg( void )
#endif /* _NO_PROTO */
{
    static char retbuf[LINEBUFSIZE];
    char	tmpbuf[LINEBUFSIZE * 8];
    char	*cp, *tmpfilename;
    FILE	*f;


    /* Return selected part from an 'uptime' to Server 	*/
    /* for use in hosts status field when XDMCP broadcast is used */
    /* (useful for selecting host to be managed by) 	*/

    strcpy(retbuf, "Willing to Manage");
    
    strcpy(tmpbuf,"LC_ALL=C; export LC_ALL; uptime | ");
    strcat(tmpbuf,"awk '{printf(\"%s %-.5s  load: %.3s, %.3s, %.3s\",$(NF-6),$(NF-5),$(NF-2),$(NF-1),$NF)}'");
    strcat(tmpbuf," > ");

    if ( (tmpfilename = tmpnam(NULL)) != NULL ) {

	strcat(tmpbuf,tmpfilename);

	system(tmpbuf);

	if ((f = fopen(tmpfilename,"r")) != (FILE *) NULL) {
	    fgets(tmpbuf,LINEBUFSIZE,f);
	    if ( (cp = strchr(tmpbuf,'\n')) != NULL) 
		*cp = '\0';

	    if (strlen(tmpbuf) > 10) 	/* seems reasonable? */
    		strcpy(retbuf, tmpbuf);

	    fclose(f);
	}
	
	unlink(tmpfilename);
    }

    return (retbuf);
}
