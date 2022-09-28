/*
 * File:	strend.C $XConsortium: strend.C /main/cde1_maint/1 1995/07/14 20:08:04 drk $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
#include "stringx.h"

char *
strend(register const char *str)
{
	if (str == NULL)
		return NULL;

	while (*str != '\0')
		str++;

	return (char *)str;
}
