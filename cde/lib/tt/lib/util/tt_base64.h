/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * tt_base64.h
 *
 * Simple routines for encoding and decoding unsigned longs as base-64 numbers
 * Note that encode returns a pointer to internal static storage.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#ifndef _TT_BASE64_H
#define _TT_BASE64_H

#include "util/tt_string.h"

unsigned long _tt_base64_decode(const char *s);
_Tt_string _tt_base64_encode(unsigned long n);

#endif /* _TT_BASE64_H */

