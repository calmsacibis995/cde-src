//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_host.C /main/cde1_maint/4 1995/10/07 19:08:45 pascale $ 			 				
/*
 *
 * @(#)tt_host.C	1.12 93/09/07
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <sys/types.h>
#include <util/tt_host.h>
#include <sys/param.h>

#if defined(_AIX)
/* AIX's arpa/inet.h has a buggy declaration of inet_addr */
extern "C" in_addr_t inet_addr(const char *);
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#ifdef __osf__
#include <unistd.h>
#else
#if !defined(USL) && !defined(__uxp__)
#include <osfcn.h>
#endif
#endif /* __osf */

#include "util/tt_port.h"   

#if defined(OPT_BUG_SUNOS_4) && defined(__GNUG__)
	extern "C" { int endhostent(); }
#endif

#define		IP_QUALIFIER	"tcp"
#define		TLI_QUALIFIER	"tli"



/* 
 * Null constructor.
 */
_Tt_host::
_Tt_host()
{
	_addr = (char *)0;
	_name = (char *)0;
	_netname = (char *)0;
}


/* 
 * Destructor. Sets _name to NULL so that reference counting can free up the
 * space if possible.
 */
_Tt_host::
~_Tt_host()
{
	_addr = (char *)0;
	_name = (char *)0;
	_netname = (char *)0;
}


/* 
 * Initializes a _Tt_host object from a hostent structure which is the
 * standard structure returned by gethostbyaddr and gethostbyname.
 */
int _Tt_host::
init_from_hostent(hostent *h)
{
	char		str[40];
	char		*ip_addr;

	if (h != (hostent *)0) {
		_name = h->h_name;
		_addr.set((const unsigned char *)h->h_addr, h->h_length);
		ip_addr = (char *)_addr;
		sprintf(str, "%d.%d.%d.%d",
		       (int)((unsigned char *)ip_addr)[0],
		       (int)((unsigned char *)ip_addr)[1],
		       (int)((unsigned char *)ip_addr)[2],
		       (int)((unsigned char *)ip_addr)[3]);
		_string_address = str;
		// We use the ip address but we qualify it with the
		// keyword IP_QUALIFIER so that SVR4 clients can
		// interoperate with this name.

		_name = h->h_name;
		_netname = IP_QUALIFIER;
		_netname = _netname.cat(":").cat(_string_address);
		return(1);
	} else {
		return(0);
	}
}


/* 
 * Initializes a _Tt_host object from an IP address.
 * 
 * --> This method should attempt to cache results as much as possible to
 * avoid repeated calls to gethostbyaddr (which may, in the abscence of
 * DNS trigger YP lookups). (and in the presence of DNS trigger DNS
 * lookups which can be just as bad...)
 *
 * Implementation note: gethostbyaddr() takes  the 4  byte raw IP
 * address, not an ASCIII "A.B.C.D" string.  Other classes that
 * use this method should  be sure to use it correctly.
 * 
 */
int _Tt_host::
init_byaddr(_Tt_string addr)
{
	int result;
	result = init_from_hostent(gethostbyaddr((char *)addr, 4, AF_INET));
#ifdef OPT_BUG_SUNOS_5
	// gethostbyaddr is effectively built on gethostent.  If
	// endhostent is not called, storage is left around to save the
	// name service connection, etc.  bug 1111175
	endhostent();
#endif
	return result;
}


/* 
 * Initializes a _Tt_host object from an ASCII string representing the host
 * address in Internet '.' notation.
 */
int _Tt_host::
init_bystringaddr(_Tt_string addr)
{
	
#ifdef __osf__
        unsigned int    *ip_address;
        unsigned int    ip_address_buf;
#else
	unsigned long	*ip_address;
	unsigned long 	ip_address_buf;
#endif

	ip_address = &ip_address_buf;

	*ip_address = inet_addr((char *)addr);
	if (*ip_address == -1) {
		return(0);
	}
	_addr.set((const unsigned char *)ip_address, 4);
	if (! init_from_hostent(gethostbyaddr((char *)_addr, 4, AF_INET))) {
		// given an ip address we can still communicate with
		// this host but we may not know it's name
		_string_address = addr;
		_netname = IP_QUALIFIER;
		_netname = _netname.cat(":").cat(_string_address);
		// we couldn't find out the name of the host so we'll
		// just call it by it's ip address
		_name = addr;
	}
#ifdef OPT_BUG_SUNOS_5
	// gethostbyaddr is effectively built on gethostent.  If
	// endhostent is not called, storage is left around to save the
	// name service connection, etc.  bug 1111175
	endhostent();
#endif
	return(1);
}

/* 
 * Initializes a _Tt_host object from a host name.
 * 
 */
int _Tt_host::
init_byname(_Tt_string name)
{
	_Tt_string	qual;
	_Tt_string	host;
	hostent		*h;
	int		result;

	if (name.len() == 0) {
		_name = _tt_gethostname();
	} else {
		qual = name.split(':',_name);
		if (_name.len()== 0) {
			// unqualified name
			_name = qual;
		} else {
			if (qual == IP_QUALIFIER) {
				return(init_bystringaddr(_name));
			} else if (qual == TLI_QUALIFIER) {
				/* just leave _name alone */;
			} else {
				// unknown qualifier!
				return(0);
			}
		}
	}
	if (!(h=gethostbyname((char *)_name))) {
		result = init_bystringaddr(_name);
	} else {
		result = init_from_hostent(h);
	}
#ifdef OPT_BUG_SUNOS_5
	// gethostbyname is effectively built on gethostent.  If
	// endhostent is not called, storage is left around to save the
	// name service connection, etc.  bug 1111175
	endhostent();
#endif
	return result;
}


/* 
 * Returns the length of the host address.
 */
int _Tt_host::
addr_length() const
{
	return(_addr.len());
}


bool_t _Tt_host::
xdr(XDR *xdrs)
{
	return(_addr.xdr(xdrs));
}

_Tt_string
_tt_host_addr(_Tt_object_ptr &o)
{
	return(((_Tt_host *)o.c_pointer())->addr());
}


_Tt_string
_tt_host_name(_Tt_object_ptr &o)
{
	return(((_Tt_host *)o.c_pointer())->name());
}
