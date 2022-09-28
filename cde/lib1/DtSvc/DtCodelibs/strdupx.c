/*
 * File:	strdupx.c $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include "stringx.h"

char *strdupx(const char *s)
{
    return s == NULL ? NULL : strcpy(strnew(strlen(s)), s);
}
