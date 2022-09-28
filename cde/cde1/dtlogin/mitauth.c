/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: mitauth.c /main/cde1_maint/1 1995/10/09 00:10:25 pascale $
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
 * mitauth
 *
 * generate cryptographically secure authorization keys
 * for MIT-MAGIC-COOKIE-1 type authorization
 */

# include   <X11/Xauth.h>
# include   <X11/Xos.h>

# include   "dm.h"


# define AUTH_DATA_LEN	16	/* bytes of authorization data */
static char	auth_name[256];
static int	auth_name_len;

void
#ifdef _NO_PROTO
MitInitAuth( name_len, name )
        unsigned short name_len ;
        char *name ;
#else
MitInitAuth(
#if NeedWidePrototypes
        unsigned int name_len,
#else
        unsigned short name_len,
#endif /* NeedWidePrototypes */
        char *name )
#endif /* _NO_PROTO */
{
    InitCryptoKey ();
    if (name_len > 256)
	name_len = 256;
    auth_name_len = name_len;
    bcopy (name, auth_name, name_len);
}

Xauth * 
#ifdef _NO_PROTO
MitGetAuth( namelen, name )
        unsigned short namelen ;
        char *name ;
#else
MitGetAuth(
#if NeedWidePrototypes
        unsigned int namelen,
#else
        unsigned short namelen,
#endif /* NeedWidePrototypes */
        char *name )
#endif /* _NO_PROTO */
{
    Xauth   *new;
    char    *data;
    
    new = (Xauth *) malloc (sizeof (Xauth));

    if (!new)
	return (Xauth *) 0;
    new->family = FamilyWild;
    new->address_length = 0;
    new->address = 0;
    new->number_length = 0;
    new->number = 0;

    /*
     *  new->data is defined as a "const char *" in Xauth.h. It therefore
     *  cannot be passed as an argument to GenerateCryptoKey() with the 
     *  intent that that routine will put the data directly into the
     *  structure. A non-const data field must be passed and then the pointer
     *  placed into new->data...   (-prr)
     *
     */

    data = (char *) malloc (AUTH_DATA_LEN);
    if (!data)
    {
	free ((char *) new);
	return (Xauth *) 0;
    }
    new->name = (char *) malloc (namelen);
    if (!new->name)
    {
	free ((char *) data);
	free ((char *) new);
	return (Xauth *) 0;
    }
    bcopy (name, new->name, namelen);
    new->name_length = namelen;
    GenerateCryptoKey (data, AUTH_DATA_LEN);
    new->data=data;
    new->data_length = AUTH_DATA_LEN;
    return new;
}
