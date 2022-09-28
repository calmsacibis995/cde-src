
/*
 *	$XConsortium: util_string.c /main/cde1_maint/2 1995/10/16 17:08:34 rcs $
 *
 * @(#)util_string.c	1.7 24 Aug 1994	cde_app_builder/src/libAButil
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */


/*
 * File: util_string.c
 */
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE 1
#endif

#include <string.h>
#include <ctype.h>
#include "utilP.h"

/*************************************************************************
**                                                                      **
**       Private Function Declarations                                  **
**                                                                      **
**************************************************************************/


/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

int
util_strncpy(STRING to, STRING from, int toSize)
{
    if (toSize < 1)
    {
	return -1;
    }

    if (from == NULL)
    {
	from = Util_empty_string;
    }
    strncpy(to, from, toSize);
    to[toSize-1]= 0;
    return 0;
}


/*
 * Finds the substring in string, case-insensitive
 * 
 * Returns index of substring, or -1 if not found
 */
int
util_strcasestr(STRING str, STRING substr)
{
    int		substr_len = strlen(substr);
    int		str_max = strlen(str) - substr_len;
    int		i;
    int		substr_start = -1;

    if (str_max < 0)
    {
	return -1;
    }

    for (i = 0; i <= str_max; ++i)
    {
	if (util_strncasecmp(&(str[i]), substr, substr_len) == 0)
	{
	    substr_start = i;
	    break;
	}
    }

    return substr_start;
}


/*
 * Case-insensitive compares the first n bytes of the string 
 * (or up to NULL),
 */
int
util_strncasecmp(STRING s1, STRING s2, int max_chars)
{
    int		diff = 0;
    int		i;

    for (i = 0; (i < max_chars) && (s1[i] != 0) && (s2[i] != 0); ++i)
    {
	if ((diff = (((int)tolower(s1[i])) - ((int)tolower(s2[i])))) != 0)
	{
	    break;
	}
    }

    return diff;
}


/*
 * REMIND: this function needs a better name (it converts more than case)
 */
int
util_strcvt_to_lower(
   STRING       lc_string,
   STRING       string,
   int          lc_string_size 
)
{
    int i;
    char c;
 
    for (i=0; i < lc_string_size; i++)
    {
        c = string[i];

	if (c != '\0' && (isspace(c) || !isalnum(c)))
	   lc_string[i] = '-'; 
        else if (c == '\0')
	{
           lc_string[i] = c;
	   break;
	}
        else if (islower(c))
           lc_string[i] = c;
        else
           lc_string[i] = tolower(c);
    }
    lc_string[lc_string_size-1] = 0;	/* make sure */
    return 0;
}

STRING
util_strip_white_space(
    STRING	string
)
{
    STRING	newString = NULL,
		index = NULL;
    
    index = string;
    /* Strip off any beginning blanks */
    while ((index != NULL) && isspace(*index))
    {
	index++;
    }

    if (index != NULL)
    {
	newString = strdup(index);
 	index = newString;
	while ((index != NULL) && !(isspace(*index)))
	{
	    index++;
	}
	/* Strip off any ending blanks */
	if (index != NULL)
	    *index = '\0';
    }
    return (newString);
}
	
