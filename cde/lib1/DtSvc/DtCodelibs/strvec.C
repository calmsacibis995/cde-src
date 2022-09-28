/*
 * File:	strvec.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

//  strvec - small string vector manipulation routines
//     - uses malloc/free and strdup/strfree
//

#include "stringx.h"

extern "C" {
    extern char * strdup(const char *);
};

int strvlen(const char **vec)
{
    int num = 0;
    if (vec != NULL)
	for (const char **v = vec; *v != NULL; v++)
	    num++;
    return num;
}

char **
strvdup(const char **vec, int num)
{
    if (vec == NULL)
	return NULL;
    if (num < 0)
	num = strvlen(vec);
    char **nvec = (char**)malloc(sizeof (char*) * (num + 1));
    if (nvec == NULL)
	return NULL;
    for (int i = 0; i < num; i++)
        nvec[i] = strdup(vec[i]);

    nvec[num] = NULL;
    return nvec;
}

void
strvfree(const char **vec, int num)
{
    if (vec == NULL)
	return;
    if (num < 0)
	num = strvlen(vec);
    for (int i = 0; i < num; i++)
	strfree(vec[i]);
    free((char*)vec);
}
