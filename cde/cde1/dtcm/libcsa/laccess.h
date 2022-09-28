/* $XConsortium: laccess.h /main/cde1_maint/1 1995/07/17 19:59:40 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _LACCESS_H
#define _LACCESS_H

#pragma ident "@(#)laccess.h	1.4 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

extern boolean_t _DtCmIsSamePath(char *str1, char *str2);

extern boolean_t _DtCmIsSameUser(char *user1, char *user2);

#endif
