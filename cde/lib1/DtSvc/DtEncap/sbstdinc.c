/*
 * File:         sbstdinc.c $XConsortium: sbstdinc.c /main/cde1_maint/1 1995/07/14 20:17:29 drk $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#define  SBSTDINC_H_NO_REDEFINE

#include <bms/sbport.h> /* NOTE: sbport.h must be the first include. */

#include <stdarg.h>

static XeChar XESTRING_SAFE_NULL[1] = {(XeChar)0};

/************************************************************************/
/* Routines from <string.h> 						*/
/************************************************************************/

XeString Xestrcat(XeString s1, const XeChar * const s2) 
{ 
    if (!s1) return XeString_NULL; 
    if (!s2) return s1;
  
   return (XeString) strcat((char *)s1, (char *) s2); 
}

XeString Xestrncat(const XeString s1, const XeChar * const s2, const size_t n) 
{
    if (!s1) return XeString_NULL; 
    if (!s2) return s1; 

    return (XeString) strncat((char *) s1, (char *) s2, n); 
}

int Xestrcmp(const XeChar * const s1, const XeChar * const s2) 
{ 
    if (s1 == s2) return 0; 
    {
        const XeChar * p1 = (s1) ? s1 : XESTRING_SAFE_NULL;
	const XeChar * p2 = (s2) ? s2 : XESTRING_SAFE_NULL;

	return strcmp((char *)p1, (char *)p2); 
    }
}

int Xestrncmp(ConstXeString s1, ConstXeString s2, size_t n) 
{
    if (s1 == s2) return 0; 
    {
	ConstXeString p1 = (s1) ? s1 : XESTRING_SAFE_NULL; 
	ConstXeString p2 = (s2) ? s2 : XESTRING_SAFE_NULL; 
	return strncmp( (char *) p1, (char *) p2, n); 
    }
}

XeString Xestrcpy(const XeString s1, const XeChar * const s2) 
{ 
    if (!s1) return s1; 
    if (!s2) { 
	*s1 = (XeChar)0; 
	return s1; 
    } 

   return (XeString) strcpy((char *) s1, (char *) s2); 
}

XeString Xestrncpy(const XeString s1, const XeChar * const s2, const size_t n) 
{
    if (!s1) return s1; 
    if (!s2 && n) { 
	*s1 = (XeChar)0; 
	return s1; 
    } 

   return (XeString) strncpy((char *) s1, (char *) s2, n); 
}

int Xestrcoll(const XeString s1, const XeString s2) 
{ 
    if (s1 == s2) return 0; 
    {
	XeString p1 = (s1) ? s1 : XESTRING_SAFE_NULL; 
	XeString p2 = (s2) ? s2 : XESTRING_SAFE_NULL; 
	return strcoll((char *)p1, (char *)p2);
    }
}

size_t Xestrxfrm(XeString s1, const XeString s2, size_t n) 
{ 
    return strxfrm((char *)s1, (char *)s2, n); 
}

XeString Xestrchr(const XeString s, int c) 
{ 
    if (!s) return XeString_NULL; 

    return (XeString) strchr((const char *) s, c); 
}

XeString Xestrrchr(const XeString s, int c) 
{ 
    if (!s) return XeString_NULL; 

    return (XeString) strrchr((const char *) s, c); 
}

XeString Xestrpbrk(const XeString s1, const XeString s2) 
{ 
    if (!s1) return XeString_NULL; 
    if (!s2) return XeString_NULL; 

    return (XeString) strpbrk((const char *) s1, (const char *) s2); 
}

XeString Xestrstr(const XeString s1, const XeString s2) 
{ 
    if (!s1) return XeString_NULL; 
    if (!s2) return XeString_NULL;

    return (XeString) strstr((const char *) s1, (const char *) s2); 
}

XeString Xestrtok(XeString s1, const XeString s2) 
{ 
    /* s1 is null except after the first call */
    if (!s2) return XeString_NULL;

    return (XeString) strtok((char *) s1, (const char *) s2); 
}

size_t Xestrlen(const XeChar * const s) 
{ 
    if (!s) return (size_t)0; 

    return strlen((char *) s); 
}

XeString Xestrdup(XeString s) 
{ 
    if (!s) return XeString_NULL; 

    return (XeString) strdup((char *) s); 
}
