//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager - dm_magic.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_magic member functions.
 */

#include <util/tt_assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <dm/dm_magic.h>
#include <isam.h>

_Tt_magic::
_Tt_magic()
{
	magic_string = (char *)calloc(ISAPPLMAGICLEN, sizeof(char));
}

_Tt_magic::
~_Tt_magic()
{
	if (magic_string != 0) {
		free((MALLOCTYPE *) magic_string );
	}
}

int _Tt_magic::
versionnumber()
{
	return atoi(magic_string);
}

void _Tt_magic::
setverno(int ver_no)
{
	sprintf(magic_string, "%04d", ver_no);
}

char * _Tt_magic::
magicstring()
{
	return magic_string;
}
