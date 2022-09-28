/* $XConsortium: hash.h /main/cde1_maint/1 1995/07/17 19:58:23 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _HASH_H
#define _HASH_H

#pragma ident "@(#)hash.h	1.6 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

extern void * _DtCmMakeHash(int size);

extern void ** _DtCmGetHash(void * tbl, const unsigned char * key);

extern void ** _DtCmFindHash(void * tbl,const unsigned char * key);

extern void _DtCmDestroyHash (void * tbl, int (*des_func)(), void * usr_arg);

#endif /* _HASH_H */

