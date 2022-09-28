/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * isbytearray.c
 *
 * Description:
 *	Byte array handling functions.
 *
 */

#include "isam_impl.h"
#include <malloc.h>


/*
 * _bytearr_getempty()
 *
 * Return an empty byte array.
 */

Bytearray 
_bytearr_getempty(void)
{
    static Bytearray	empty_bytearray = { 0, NULL};

    return (empty_bytearray);
}

/*
 * bytearray = _bytearr_new(len, data)
 *
 * Create a byte array object.
 */

Bytearray
_bytearr_new(u_short len, char *data)
{
    Bytearray		bytearray;

    bytearray.length = len;
    bytearray.data = _ismalloc(len);
    memcpy( bytearray.data,data, (int)len);
    
    return (bytearray);
}

/*
 * new = _bytearr_dup(old)
 *
 * Duplicate a byte array object.
 */

Bytearray
_bytearr_dup(old)
    Bytearray		*old;
{
    return (_bytearr_new(old->length, old->data));
}

/*
 * _bytearr_free(barrray)
 *
 * Free byte array buffer, set barray to an empty byte array.
 */

void
_bytearr_free(barray)
    register Bytearray	*barray;
{
    if (barray->data)
	free(barray->data);

    *barray = _bytearr_getempty();
}

/*
 * _bytearr_cmp(l, r)
 *
 * Compare two byte array objects. Return 0 if they are equal, non-zero if
 * they differ.
 */

int
_bytearr_cmp(l,r)
    register Bytearray	*l, *r;
{
    if (l->length == r->length)
	return (memcmp(l->data, r->data, (int)l->length));
    else
	return (1);			     /* not equal */
}
