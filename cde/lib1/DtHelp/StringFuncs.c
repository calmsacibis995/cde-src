/* $XConsortium: StringFuncs.c /main/cde1_maint/3 1995/10/08 17:21:21 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     StringFuncs.c
 **
 **   Project:  Cde DtHelp
 **
 **   Description: Semi private string functions - can be platform dependent.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/*
 * system includes
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef __osf__
#include <errno.h>
#endif /* __osf__ */
#include <locale.h>  /* getlocale(), LOCALE_STATUS, LC_xxx */

#if defined(_AIX) || defined(USL) || defined(__uxp__)
#include <ctype.h>
#endif

#include "StringFuncsI.h"     /* for _CEStrcollProc */

#if !defined(sun) && !defined(USL) && !defined(__uxp__)  /* SunOS doesn't have iconv */
#include <iconv.h>
#endif

/**** Types ****/
/* Private structure of the IconvContext.  Public
   structure doesn't reveal implementation. */
typedef struct _DtHelpCeIconvContextRec
{
#if defined(sun) || defined(USL) || defined(__uxp__)
    int             cd;
#elif defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3)
    iconvd          cd;
#else   /* XPG4 iconv */
    iconv_t         cd;
#endif
    int             tableSize;
    unsigned char * table;        /* translation table */
    char *          fromCodeset;
    char *          toCodeset;
} _DtHelpCeIconvContextRec;


#ifndef True
#define True    1
#endif
#ifndef TRUE
#define TRUE    1
#endif
#ifndef False
#define False   0
#endif
#ifndef FALSE
#define FALSE   0
#endif

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
#define REALLOC_INCR    10

#define EOS '\0'

/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function: _DtHelpCeStrcspn (const char *s1, const char *s2, max_len, &ret_len)
 *
 *	Returns in 'ret_len' the length of the initial segment of string
 *	s1 which consists entirely of characters not found in string s2.
 *
 * Returns:
 *      -1  If found an invalid character.
 *       0  If found a character in string s2
 *       1  If found the null byte character.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrcspn (s1, s2, max_len, ret_len)
    const char    *s1;
    const char    *s2;
    int            max_len;
    int           *ret_len;
#else
_DtHelpCeStrcspn (
    const char    *s1,
    const char    *s2,
    int            max_len,
    int           *ret_len )
#endif /* _NO_PROTO */
{
    int    len;
    int    len2;
    const char  *p1;
    const char  *p2;

    if (!s1)
      {
	*ret_len = 0;
	return 0;
      }

    if (*s1 == '\0')
      {
	*ret_len = 0;
	return 1;
      }

    if (!s2 || *s2 == '\0')
      {
	*ret_len = strlen (s1);
	return 1;
      }

    if (max_len == 1)
      {
	/*
	 * no need to go through any hassle, just use the 3C function
	 */
	*ret_len = strcspn (s1, s2);
	if (s1[*ret_len] == '\0')
	    return 1;
	return 0;
      }

    p1 = s1;
    while (*p1 != '\0')
      {
	len = mblen (p1, max_len);
	if (len == -1)
	  {
	    /*
	     * we found an invalid character
	     * return the length found so far and the flag.
	     */
	    *ret_len = p1 - s1;
	    return -1;
	  }

	p2 = s2;
	while (*p2 != '\0')
	  {
	    len2 = mblen (p2, max_len);
	    if (len2 == -1)
		len2 = 1;

	    if (len2 == len && strncmp (p1, p2, len) == 0)
	      {
		*ret_len = p1 - s1;
		return 0;
	      }
	    p2 += len2;
	  }
	p1 += len;
      }

    *ret_len = p1 - s1;
    return 1;
}

/******************************************************************************
 * Function: _DtHelpCeStrspn (const char *s1, const char *s2, max_len, &ret_len)
 *
 *	Returns in 'ret_len' the length of the initial segment of string
 *	s1 which consists entirely of characters found in string s2.
 *
 * Returns:
 *      -1  If found an invalid character.
 *       0  If found a character not in string s2
 *       1  If found the null byte character.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrspn (s1, s2, max_len, ret_len)
    char    *s1;
    char    *s2;
    int      max_len;
    int     *ret_len;
#else
_DtHelpCeStrspn (
    char    *s1,
    char    *s2,
    int      max_len,
    int     *ret_len )
#endif /* _NO_PROTO */
{
    int    len;
    int    len2;
    char  *p1;
    char  *p2;
    char   found;

    if (!s1 || !s2 || *s2 == '\0')
      {
	*ret_len = 0;
	return 0;
      }
    if (*s1 == '\0')
      {
	*ret_len = 0;
	return 1;
      }

    if (max_len == 1)
      {
	/*
	 * no need to go through any hassle, just use the 3C function
	 */
	*ret_len = strspn (s1, s2);
	if (s1[*ret_len] == '\0')
	    return 1;
	return 0;
      }

    p1 = s1;
    found = True;
    while (*p1 != '\0' && found)
      {
	len = mblen (p1, max_len);
	if (len == -1)
	  {
	    /*
	     * we found an invalid character
	     * return the length found so far and the flag.
	     */
	    *ret_len = p1 - s1;
	    return -1;
	  }

	p2 = s2;
	found = False;
	while (*p2 != '\0' && !found)
	  {
	    len2 = mblen (p2, max_len);
	    if (len2 == -1)
		len2 = 1;

	    if (len2 == len && strncmp (p1, p2, len) == 0)
		found = True;
	    p2 += len2;
	  }

	if (found)
	    p1 += len;
      }

    *ret_len = p1 - s1;

    if (found)
        return 1;

    return 0;
}

/******************************************************************************
 * Function: _DtHelpCeStrchr (char *s1, char *value, max_len, ret_ptr)
 *
 *	Returns in 'ret_ptr' the address of the first occurence of 'value'
 *	in string s1.
 *
 * Returns:
 *      -1  If found an invalid character.
 *       0  If found value in string s2
 *       1  If found the null byte character without finding 'value'.
 *	    'ret_ptr' will also be null in this case.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrchr (s1, value, max_len, ret_ptr)
    const char    *s1;
    const char    *value;
    int            max_len;
    char         **ret_ptr;
#else
_DtHelpCeStrchr (
    const char    *s1,
    const char    *value,
    int            max_len,
    char         **ret_ptr )
#endif /* _NO_PROTO */
{
    int      len;
    int      valLen;
    const char *p1;

    *ret_ptr = NULL;

    if (s1 == NULL || *s1 == '\0')
	return 1;

    if (max_len == 1)
      {
	*ret_ptr = strchr (s1, ((int)*value));
	if (*ret_ptr)
	    return 0;
	return 1;
      }

    p1 = s1;
    valLen = mblen(value, max_len);
    if (valLen < 1)
	return -1;

    while (*p1 != '\0')
      {
	len = mblen (p1, max_len);
	if (len == -1)
	    return -1;
	if (len == valLen && strncmp(p1, value, len) == 0)
	  {
	    *ret_ptr = (char *)p1;
	    return 0;
	  }
	p1 += len;
      }

    return 1;
}

/******************************************************************************
 * Function: _DtHelpCeStrrchr (char *s1, char *value, max_len, ret_ptr)
 *
 *	Returns in 'ret_ptr' the address of the last occurence of 'value'
 *	in string s1.
 *
 * Returns:
 *      -1  If found an invalid character.
 *       0  If found value in string s2
 *       1  If found the null byte character without finding 'value'.
 *	    'ret_ptr' will also be null in this case.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrrchr (s1, value, max_len, ret_ptr)
    const char    *s1;
    const char    *value;
    int            max_len;
    char         **ret_ptr;
#else
_DtHelpCeStrrchr (
    const char    *s1,
    const char    *value,
    int            max_len,
    char         **ret_ptr )
#endif /* _NO_PROTO */
{
    int      len;
    int      valLen;
    const char *p1;

    *ret_ptr = NULL;

    if (s1 == NULL || *s1 == '\0')
	return 1;

    if (max_len == 1)
      {
	*ret_ptr = strrchr (s1, ((int)*value));
	if (*ret_ptr != NULL)
	    return 0;
	return 1;
      }

    p1 = s1;
    valLen = mblen(value, max_len);
    if (valLen < 1)
	return -1;

    while (*p1 != '\0')
      {
	len = mblen (p1, max_len);
	if (len == -1)
	    return -1;
	if (len == valLen && strncmp(p1, value, len) == 0)
	    *ret_ptr = (char *)p1;

	p1 += len;
      }

    if (*ret_ptr != NULL)
	return 0;

    return 1;
}

/******************************************************************************
 * Function: _DtHelpCeCountChars (char *s1, max_len, ret_len)
 *
 *	Returns in 'ret_len' the number of characters (not bytes)
 *	in string s1.
 *
 * Returns:
 *      -1  If found an invalid character. 'ret_len' contains the
 *	    number of 'good' characters found.
 *       0  If successful.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeCountChars (s1, max_len, ret_len)
    char    *s1;
    int      max_len;
    int     *ret_len;
#else
_DtHelpCeCountChars (
    char    *s1,
    int      max_len,
    int     *ret_len )
#endif /* _NO_PROTO */
{
    int    len;

    *ret_len = 0;

    if (s1)
      {
        if (max_len == 1)
	    *ret_len = strlen (s1);
	else
	  {
	    while (*s1)
	      {
		len = mblen (s1, max_len);
		if (len == -1)
		    return -1;
		*ret_len = *ret_len + 1;
		s1 += len;
	      }
	  }
      }
    return 0;
}

/*****************************************************************************
 * Function: _DtHelpCeUpperCase
 *
 * Parameters:  string          Specifies the string to change into
 *                              upper case.
 *
 * Return Value: Nothing
 *
 * Purpose:     To change all lower case characters into upper case.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeUpperCase ( string )
        char *string;
#else
_DtHelpCeUpperCase ( char *string )
#endif /* _NO_PROTO */
{
    int   len;

    if (string)
      {
	while (*string != '\0')
	  {
	    len = mblen (string, MB_CUR_MAX);
	    if (len == 1)
	      {
		*string = (unsigned char) toupper (*(unsigned char *)string);
		string++;
	      }
	    else if (len > 0)
		string += len;
	    else
		return;
	  }
      }
}

#ifdef	not_used
/*****************************************************************************
 * Function: _DtHelpCeLowerCase
 *
 * Parameters:  string          Specifies the string to change into
 *                              upper case.
 *
 * Return Value: Nothing
 *
 * Purpose:     To change all upper case characters into lower case.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeLowerCase ( string )
        char *string;
#else
_DtHelpCeLowerCase ( char *string )
#endif /* _NO_PROTO */
{
    int   len;

    if (string)
      {
	while (*string != '\0')
	  {
	    len = mblen (string, MB_CUR_MAX);
	    if (len == 1)
	      {
		*string = (unsigned char) tolower (*(unsigned char *)string);
		string++;
	      }
	    else if (len > 0)
		string += len;
	    else
		return;
	  }
      }
}
#endif /* not_used */

/************************************************************************
 * Function: _DtHelpCeStrHashToKey
 *
 * Parameters:  C-format string to hash
 *
 * Purpose:
 *   Does a very simple hash operation on the string and returns the value
 *
 * Returns:  hash value
 *
 ************************************************************************/
#ifdef _NO_PROTO
int _DtHelpCeStrHashToKey(str)
        const char * str;
#else
int _DtHelpCeStrHashToKey(
        const char * str)
#endif /* _NO_PROTO */
{
    register char c;
    register const char * tstr;
    register int key = 0;

    if (!str) return 0;                  /* RETURN */

    for (tstr = str; c = *tstr++; )
        key = (key << 1) + c;

    return key;
}



/******************************************************************************
 * Function:    _DtHelpCeGetStrcollProc
 *
 * Parameters:  none
 *
 * Returns:	Ptr to the proper collation function to use
 *              If the codeset of the locale is "C", then it is 
 *              strcasecmp(). If it's not, then it is strcoll().
 *
 * Purpose:	When the codeset of the locale "C", strcoll() 
 *              performs collation identical to strcmp(), which is 
 *              strictly bitwise.
 *
 *              To get case-insensitive collation, you need to use
 *              strcasecmp() instead.  If codeset != "C", then 
 *              strcoll() collates according to the language
 *              setting.
 * 
 * Warning:     This code is not multi-thread safe.  The multi-thread
 *              safe setlocale must be used instead to make it so.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
   _CEStrcollProc _DtHelpCeGetStrcollProc()
#else
   _CEStrcollProc _DtHelpCeGetStrcollProc(void)
#endif
{
   int Clang = 0;
#if defined(__hpux)
   struct locale_data * li;
#else
   char * locale;
#endif

   extern int strcoll(const char *,const char *);
#if defined(_AIX) || defined(USL) || defined(__uxp__)
   extern int _DtHelpCeStrCaseCmp(const char *,const char *);
#else
   extern int strcasecmp(const char *,const char *);
#endif

#define C_LANG  "C"

   /* if locale is C, use the explicit case insensitive compare */
#if defined(__hpux)
   li = getlocale(LOCALE_STATUS);
   if ( NULL == li->LC_COLLATE_D || strcmp(C_LANG,li->LC_COLLATE_D) == 0 )
     Clang = 1;
#else
   locale = setlocale(LC_COLLATE,NULL); /* put locale in buf */
   if (strcmp(locale,C_LANG) == 0)
     Clang = 1;
#endif

   if (Clang)
#if defined(_AIX) || defined(USL) || defined(__uxp__)
     return _DtHelpCeStrCaseCmp;
#else
     return strcasecmp; 
#endif
   return strcoll;
}

/*****************************************************************************
 * Function:        String _DtHelpCeStripSpaces (string)
 *
 *
 * Parameters:    String to process
 *
 * Return Value:  Processed string
 *
 * Purpose:       Strip all leading and trailing spaces.
 *                Processing is in place
 *
 *****************************************************************************/
#ifdef _NO_PROTO
char * _DtHelpCeStripSpaces (string)
    char * string;
#else
char * _DtHelpCeStripSpaces (
    char * string)
#endif
{
   int i;
   int multiLen = MB_CUR_MAX;
   char * space;

   if (string == NULL)
      return (string);

   /* Strip off leading spaces first */
   i = 0;
   while ((multiLen == 1 || (mblen(string + i, MB_LEN_MAX) == 1)) &&
          isspace((unsigned char) string[i]))
   {
      i++;
   }
   /* Copy over the leading spaces */
   strcpy(string, string + i);

   /* Drop out, if the string is now empty */
   if ((i = strlen(string) - 1) < 0)
      return(string);

   /* Strip off trailing spaces */
   if (multiLen == 1)
   {
      /* No multibyte; simply work back through the string */
      while ((i >= 0) && (isspace((unsigned char) string[i])))
         i--;
      string[i + 1] = '\0';
   }
   else
   {
      /* Work forward, looking for a trailing space of spaces */
      int len;

      i = 0;
      space = NULL;

      while (string[i])
      {
         if (    ((len =mblen(string + i, MB_LEN_MAX)) == 1) 
              && isspace((unsigned char) string[i]))
         {
            /* Found a space */
            if (space == NULL)
               space = string + i;
         }
         else if (space)
            space = NULL;

         /* if there is an invalid character, treat as a valid one-byte */
         if (len == -1)
           len = 1;
         i += len;

      }

      if (space)
         *space = '\0';
   }

   return (string);
}


/*****************************************************************************
 * Function:        void _DtHelpCeCompressSpace (string)
 *
 * Parameters:      string to process
 *
 * Return Value:    processed string
 *
 * Purpose:           This function strips all leading and trailing spaces
 *                    from the string; it also compresses any intervening
 *                    spaces into a single space.  This is useful when
 *                    comparing name strings.  For instance, the string:
 *                    "    First    Middle    Last   "
 *
 *                    would compress to:
 *
 *                    "First Middle Last"
 *
 *                     Processing is in place.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpCeCompressSpace (string)
    char * string;
#else
void _DtHelpCeCompressSpace (
    char * string)
#endif

{
   char * ptr;

   /* Strip leading and trailing spaces */
   _DtHelpCeStripSpaces(string);

   /* Compress intervening spaces */
   _DtHelpCeStrchr(string, " ", 1, &ptr);
   while (ptr)
   {
      /* Skip over the one space we plan to keep */
      ptr++;
      _DtHelpCeStripSpaces(ptr);
      _DtHelpCeStrchr(ptr, " ", 1, &ptr);
   }
}

/*****************************************************************************
 * Function:        void _DtHelpCeIconvStr1Step (string)
 *
 * Parameters:      
 *    fromCode;      codeset name
 *    fromStr;       string to convert
 *    toCode;        codeset name
 *    ret_toStr;     converted str; this string is malloc'd by this routine
 *                   and the CALLER MUST FREE it when no longer needed.
 *    dflt1;         1-byte default char
 *    dflt2;         2-byte default char
 *
 * Return Value:    
 *                  0: ok
 *                 -1: missing (NULL) argument
 *                 -2: no translation available from fromCode to toCode
 *                 -3: couldn't allocate memory 
 *                 -4: couldn't start conversion 
 *                 -5: incomplete multibyte char
 *                 -6: invalid char found
 *
 * Purpose:
 *                  Converts string from fromCode to toCode using iconv(3)
 *                  It expects the codeset strings to be iconv(3) compatible.
 *		    Generally, compatible strings can be retrieved using
 *		    the _DtHelpCeXlateStdToOpLocale() call.
 *
 * Comments:
 *                  iconv(3) is standardized in XPG4, which is just starting
 *                  to be supported.  Below are several different implementations 
 *                  of _DtHelpCeIconvStr, each using what is available on different 
 *                  platforms.  If no code is #ifdef'd, the XPG4 code defaults.
 *****************************************************************************/

#ifdef _NO_PROTO
int _DtHelpCeIconvStr1Step(fromCode,fromStr,toCode,toStr,dflt1,dflt2)
      const char * fromCode;   /* codeset name */
      const char * fromStr;    /* string to convert */
      const char * toCode;     /* codeset name */
      char * *     ret_toStr;  /* converted str */
      int          dflt1;      /* 1-byte default char */
      int          dflt2;      /* 2-byte default char */
#else
int _DtHelpCeIconvStr1Step(
      const char * fromCode,   /* codeset name */
      const char * fromStr,    /* string to convert */
      const char * toCode,     /* codeset name */
      char * *     ret_toStr,  /* converted str */
      int          dflt1,      /* 1-byte default char */
      int          dflt2)      /* 2-byte default char */
#endif

#if 0    /* inside the #if 0 section is a straight implementation */
#if defined(sun)
#define _DONT_USE_XPG4_ICONV
{   /* SunOS doesn't have iconv() */
    *ret_toStr = strdup(fromStr);
    return (NULL != *ret_toStr ? 0 : -3);    /* RETURN result */
}
#endif

#if defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3)
#define _DONT_USE_XPG4_ICONV
{ /* HPUX 9.00-9.03 (args dflt1 & dflt2 are used in this code) */
   iconvd           cd;           /* conversion descriptor */
   int              tableSize;         /* translation table size */
   unsigned char *  table;        /* translation table */
   char *           toStr;        /* ptr to tostr memory */
   int              toStrSize;    /* size of mallocd string */
   int              inBytesLeft;  /* bytes left to use from input buf */
   const unsigned char *  inChar;       /* ptr into fromstr */
   unsigned char *  outChar;      /* ptr into tostr  */
   int              outBytesLeft; /* bytes left in the output str */
   int              err;          /* error code of function */

#define BAD (-1)
#define MEM_INC 20

   /* check args */
   if (!fromCode || !toCode || !fromStr || !ret_toStr)
      return -1;                         /* RETURN error */

   /* init state */
   *ret_toStr = NULL;

   /* get info on conversion */
   tableSize=iconvsize(toCode,fromCode);
   if ( tableSize == BAD ) return -2;    /* RETURN error */

   if ( tableSize == 0 ) /* no conversion needed */
   {
      *ret_toStr = strdup(fromStr);
      return 0;                          /* RETURN ok */
   }

   /* conversion needed; create table */
   table = (unsigned char *) malloc(tableSize);
   if ( NULL == table ) return -3;       /* RETURN error */
   cd = iconvopen(toCode,fromCode,table,dflt1,dflt2);
   if ( cd == (iconvd) BAD ) 
   {
      free(table);
      return -4;                         /* RETURN ERROR */
   }

   /* init ptrs */
   inChar = (const unsigned char *)fromStr;
   inBytesLeft = strlen(fromStr);
   outChar = NULL;
   outBytesLeft = 0;
   toStr = NULL;
   toStrSize = 0;

   /* translate the string */
   err = -3;      /* default case */
   while ( inBytesLeft > 0 || 0 == outBytesLeft )
   {
      int ret;
      ret = 3;     /* default case */
      /* convert a character */
      if (   0 == outBytesLeft
          || (ret=ICONV(cd,&inChar,&inBytesLeft,&outChar,&outBytesLeft))!=0)
      {
         switch(ret)
         {
         case 0: /* no error */
            continue;
         case 1: /* inChar pts to incomplete multibyte char */
            inBytesLeft = 0;       /* end the translation */
            err = -5;
            break;
         case 2: /* invalid char at inChar */
            inBytesLeft = 0;       /* end the translation */
            err = -6;
            break;
         case 3: /* no room in toStr */
            /* provide enough mem in the toStr */
            if (outBytesLeft < sizeof(wchar_t))
            {
               size_t offset = outChar - (unsigned char *) toStr;
   
               outBytesLeft += MEM_INC;
               toStrSize += MEM_INC;
               toStr = realloc(toStr,toStrSize * sizeof(char));
               if (NULL == toStr)
               {
                  inBytesLeft = 0;       /* end the translation */
                  err = -3;
                  break;                 /* BREAK */
               }
               outChar = (unsigned char *) (toStr + offset); /* recalc ptr */
            }
            break;
   
         }  /* switch on convertsion result */
      }  /* if no bytes left or ret != 0 */
   } /* while chars left to translate */

   /* set the EOS */
   if(outChar) *outChar = EOS;

   iconvclose(cd);                    /* ignore return value */
   free(table);
   *ret_toStr = (char *) toStr;

   return (NULL != toStr ? 0 : err);    /* RETURN result */
}
#endif /* HPUX 9.00-9.03 */


#if !defined(_DONT_USE_XPG4_ICONV)
{  /* XPG4-compliant code (args dflt1 & dflt2 are ignored in this code) */
   iconv_t          cd;           /* conversion descriptor */
   char *           toStr;        /* ptr to tostr memory */
   int              toStrSize;    /* size of mallocd string */
   size_t           inBytesLeft;  /* bytes left to use from input buf */
   const char *     inChar;       /* ptr into fromstr */
   char *           outChar;      /* ptr into tostr  */
   size_t           outBytesLeft; /* bytes left in the output str */
   int              err;          /* error code of function */

#define BAD (-1)
#define MEM_INC 20

   /* check args */
   if (!fromCode || !toCode || !fromStr || !ret_toStr)
      return -1;                         /* RETURN error */

   /* init state */
   *ret_toStr = NULL;

   cd = iconv_open(toCode,fromCode);
   if ( cd == (iconv_t) BAD ) 
   {
      return -4;                         /* RETURN ERROR */
   }

   /* init ptrs */
   inChar = (const unsigned char *)fromStr;
   inBytesLeft = strlen(fromStr);
   outChar = NULL;
   outBytesLeft = 0;
   toStr = NULL;
   toStrSize = 0;

   /* translate the string and make sure there's room for the EOS */
   err = -3;
   while ( inBytesLeft > 0 || 0 == outBytesLeft )
   {
      errno = E2BIG;          /* default case */
      /* convert a character */
      if(   0 == outBytesLeft
         || iconv(cd,&inChar,&inBytesLeft,&outChar,&outBytesLeft) == -1)
      {
         switch(errno)
         {
         case 0: /* no error */
            continue;
         case EINVAL: /* inChar pts to incomplete multibyte char */
            inBytesLeft = 0;       /* end the translation */
            err = -5;
            break;
         case EILSEQ: /* invalid char at inChar */
            inBytesLeft = 0;       /* end the translation */
            err = -6;
            break;
         case E2BIG: /* no room in toStr */
            /* provide enough mem in the toStr */
            if (outBytesLeft < sizeof(wchar_t))
            {
               size_t offset = outChar - (unsigned char *) toStr;
   
               outBytesLeft += MEM_INC;
               toStrSize += MEM_INC;
               toStr = realloc(toStr,toStrSize * sizeof(char));
               if (NULL == toStr)
               {
                  inBytesLeft = 0;       /* end the translation */
                  err = -3;
                  break;                 /* BREAK */
               }
               outChar = (unsigned char *) (toStr + offset); /* recalc ptr */
            }
            break;
         default:
            inBytesLeft = 0;             /* breakout of loop */
            break;
         }  /* switch on convertsion result */
      } /* if an error */
   } /* while chars left to translate */

   /* set the EOS  */
   if(outChar) *outChar = EOS;

   iconv_close(cd);                    /* ignore return value */
   *ret_toStr = (char *) toStr;

   return (NULL != toStr ? 0 : err);    /* RETURN result */
}
#endif  /* XPG4-compliant systems */

#else   /* inside the #if 0 section is a straight implementation */
{
   _DtHelpCeIconvContextRec * iconvContext;
   int ret;

   ret = _DtHelpCeIconvOpen(&iconvContext,fromCode,toCode,dflt1,dflt2);
   if (0 == ret) 
      ret = _DtHelpCeIconvStr(iconvContext,fromStr, ret_toStr,NULL,NULL,0);
   _DtHelpCeIconvClose(&iconvContext);
   return ret;
}

#endif   /* inside the #if 0 section is a straight implementation */


/*****************************************************************************
 * Function:        void _DtHelpCeIconvOpen (string)
 *
 * Parameters:      
 *    iconvContext   context 
 *    fromStr;       string to convert
 *    ret_toStr;     converted str; this string is malloc'd by this routine
 *                   and the CALLER MUST FREE it when no longer needed.
 *    dflt1;         1-byte default char
 *    dflt2;         2-byte default char
 *
 * Return Value:    
 *                  0: ok
 *                 -1: missing (NULL) argument
 *                 -2: no translation available from fromCode to toCode
 *                 -3: couldn't allocate memory 
 *                 -4: couldn't start conversion 
 *                 -5: incomplete multibyte char
 *                 -6: invalid char found
 *
 * Purpose:
 *              Opens an iconv table/algorithm to convert string from 
 *              fromCode to toCode using iconv(3)
 *              It expects the codeset strings to be iconv(3) compatible.
 *		Generally, compatible strings can be retrieved using
 *		the _DtHelpCeXlateStdToOpLocale() call.
 *
 * Comments:
 *              iconv(3) is standardized in XPG4, which is just starting
 *              to be supported.  Below are several different implementations 
 *              of _DtHelpCeIconvStr, each using what is available on different 
 *              platforms.  If no code is #ifdef'd, the XPG4 code defaults.
 *****************************************************************************/
#ifdef _NO_PROTO
int _DtHelpCeIconvOpen(iconvContext,fromCode,toCode,dflt1,dflt2)
      _DtHelpCeIconvContext * ret_iconvContext;  /* iconv */
      const char * fromCode;   /* codeset name */
      const char * toCode;     /* codeset name */
      int          dflt1;      /* 1-byte default char */
      int          dflt2;      /* 2-byte default char */
#else
int _DtHelpCeIconvOpen(
      _DtHelpCeIconvContext * ret_iconvContext,  /* iconv */
      const char * fromCode,   /* codeset name */
      const char * toCode,     /* codeset name */
      int          dflt1,      /* 1-byte default char */
      int          dflt2)      /* 2-byte default char */
#endif

#if defined(sun) || defined(USL) || defined(__uxp__)
#define _DONT_USE_XPG4_ICONV
{   /* SunOS doesn't have iconv() */
   _DtHelpCeIconvContextRec * ic;

#define BAD (-1)

    if (!ret_iconvContext)
       return -1;                         /* RETURN bad arg */

    ic = *ret_iconvContext = calloc(1,sizeof(_DtHelpCeIconvContextRec));
    if (NULL == ic)  return -3;   /* RETURN error */

    /* rely upon calloc to set contents to 0; 
      cd == BAD means to use strdup() */

    ic->fromCodeset = strdup(fromCode);
    ic->toCodeset = strdup(toCode);
    ic->cd = BAD;

    return 0;                               /* RETURN ok */
}
#endif

#if defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3)
#define _DONT_USE_XPG4_ICONV
{ /* HPUX 9.00-9.03 (args dflt1 & dflt2 are used in this code) */
   iconvd           cd;           /* conversion descriptor */
   int              err;          /* error code of function */
   _DtHelpCeIconvContextRec * ic;

#define BAD (-1)
#define MEM_INC 20

   err = 0;  /* ok */

   /* check args */
   if (!ret_iconvContext || !fromCode || !toCode)
      return -1;                         /* RETURN error */

   /* init state */
   ic = *ret_iconvContext = calloc(1,sizeof(_DtHelpCeIconvContextRec));
   if (NULL == ic) return -3;   /* RETURN error */

   /* get info on conversion */
   ic->cd = BAD;  /* until made good */
   ic->tableSize = iconvsize(toCode,fromCode);
   if ( ic->tableSize == BAD ) return -2;    /* RETURN error */

   if ( ic->tableSize == 0 ) /* no conversion needed */
   {
      /* ic->cd = BAD means use strdup() */
      goto success;
   }

   /* conversion needed; create table */
   ic->table = (unsigned char *) malloc(ic->tableSize);
   if ( NULL == ic->table ) return -3;       /* RETURN error */
   ic->cd = iconvopen(toCode,fromCode,ic->table,dflt1,dflt2);
   if ( ic->cd == (iconvd) BAD ) 
   {
      free(ic->table);
      ic->table = 0;           /* no table */
      err = -4;                         /* RETURN ERROR */
   }

success:
   ic->fromCodeset = strdup(fromCode);
   ic->toCodeset = strdup(toCode);
   return err;                             /* RETURN ok */
}
#endif /* defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3) */

#if !defined(_DONT_USE_XPG4_ICONV)
{  /* XPG4-compliant code (args dflt1 & dflt2 are ignored in this code) */
   int              err;          /* error code of function */
   _DtHelpCeIconvContextRec * ic;

#define BAD (-1)
#define MEM_INC 20

   err = 0;  /* ok */

   /* check args */
   if (!ret_iconvContext || !fromCode || !toCode )
      return -1;                         /* RETURN error */

   /* init state */
   ic = *ret_iconvContext = calloc(1,sizeof(_DtHelpCeIconvContextRec));
   if (NULL == ic) return -3;   /* RETURN error */

   if ( strcmp(fromCode,toCode) == 0 )
   {
      ic->cd = (iconv_t) BAD;           /* BAD means use strdup() */
      goto success;
   }

   ic->cd = iconv_open(toCode,fromCode);
   if ( ic->cd == (iconv_t) BAD ) 
   {
      err = -4;                         /* error */
      goto success;
   }

success:
   ic->fromCodeset = strdup(fromCode);
   ic->toCodeset = strdup(toCode);
   return err;                             /* RETURN status */
}
#endif /* !defined(_DONT_USE_XPG4_ICONV) */

/*****************************************************************************
 * Function:        void _DtHelpCeIconvStr (string)
 *
 * Parameters:      
 *    iconvContext   context for the conversion
 *    fromStr;       string to convert
 *    ret_toStr;     converted str; this string is malloc'd by this routine
 *                   and the CALLER MUST FREE it when no longer needed.
 *    toStrBuf;      for efficiency, can pass in a buf 
 *    toStrBufLen;   length of buf
 *
 * Return Value:    
 *                  0: ok
 *                 -1: missing (NULL) argument
 *                 -2: no translation available from fromCode to toCode
 *                 -3: couldn't allocate memory 
 *                 -4: couldn't start conversion 
 *                 -5: incomplete multibyte char
 *                 -6: invalid char found
 *
 * Purpose:
 *              Converts string from fromCode to toCode using iconv(3)
 *              If toStrBuf is NULL, memory for the converted string
 *                will be malloced as needed.
 *              If toStrBuf is not NULL, the conversion will use up 
 *                to toStrBufLen bytes of the buffer and then realloc
 *                more memory if needed.
 *              If toStrBuf is not NULL, the size of the buf is
 *                returned in ret_toStrLen; otherwise, the value is
 *                not set.
 *              ret_toStr receives the pointer to the buf, which may
 *                be different from toStrBuf if memory was allocated
 *                or NULL if an error occurred.  If toStrBuf is
 *                not NULL and memory must be allocated, a realloc()
 *                call is used, possibly invalidating the toStrBuf ptr.
 *              ret_toStrLen receives the length of the buffer if
 *                toStrBuf is not NULL.  If it is NULL, the length
 *                is not returned.
 * Comments:
 *              iconv(3) is standardized in XPG4, which is just starting
 *              to be supported.  Below are several different implementations 
 *              of _DtHelpCeIconvStr, each using what is available on different 
 *              platforms.  If no code is #ifdef'd, the XPG4 code defaults.
 *****************************************************************************/
#ifdef _NO_PROTO
int _DtHelpCeIconvStr(iconvContext,fromStr,
                       ret_toStr,ret_toStrlen,toStrBuf,toStrBufLen)
      _DtHelpCeIconvContext iconvContext;  /* iconv */
      const char * fromStr;      /* string to convert */
      char * *     ret_toStr;    /* converted str */
      size_t *     ret_toStrLen; /* converted str */
      char *       toStrBuf;     /* for efficiency, can pass in a buf */
      size_t       toStrBufLen;  /* length of buf */
#else
int _DtHelpCeIconvStr(
      _DtHelpCeIconvContext iconvContext, /* iconv */
      const char * fromStr,      /* string to convert */
      char * *     ret_toStr,    /* converted str */
      size_t *     ret_toStrLen, /* converted str */
      char *       toStrBuf,     /* for efficiency, can pass in a buf */
      size_t       toStrBufLen)  /* length of buf */
#endif
#if defined(sun) || defined(USL) || defined(__uxp__)
#define _DONT_USE_XPG4_ICONV
{   /* SunOS doesn't have iconv() */
    /* init ret values; allows function to be called nicely in a loop.  */
    if (ret_toStr) *ret_toStr = toStrBuf;
    if (ret_toStrLen) *ret_toStrLen = toStrBufLen;

    if (!iconvContext || !fromStr || !ret_toStr 
        || (!ret_toStrLen && toStrBuf))
       return -1;                 /* RETURN bad arg */

    if (NULL == toStrBuf)
    {
       *ret_toStr = strdup(fromStr);
    }
    else  /* reuse the buffer */
    {
       int len = strlen(fromStr);
       if (len > toStrBufLen)
          *ret_toStr = realloc(toStrBuf,len);
       else 
       {
          *ret_toStr = toStrBuf;
          len = toStrBufLen;
       }
       /* set return values */
       strcpy(*ret_toStr,fromStr);
       *ret_toStrLen = len;
    }
    return (NULL != *ret_toStr ? 0 : -3);    /* RETURN result */
}
#endif

#if defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3)
#define _DONT_USE_XPG4_ICONV
{
   char *           toStr;        /* ptr to tostr memory */
   int              toStrSize;    /* size of mallocd string */
   size_t           inBytesLeft;  /* bytes left to use from input buf */
   const char *     inChar;       /* ptr into fromstr */
   char *           outChar;      /* ptr into tostr  */
   size_t           outBytesLeft; /* bytes left in the output str */
   int              err;          /* error code of function */

   /* init ret values; allows function to be called nicely in a loop.  */
   if (ret_toStr) *ret_toStr = toStrBuf;
   if (ret_toStrLen) *ret_toStrLen = toStrBufLen;

   if (!iconvContext || !fromStr || !ret_toStr 
       || (!ret_toStrLen && toStrBuf))
   {
      return -1;                               /* RETURN bad arg */
   }

   /* just do a straight copy if codesets the same or invalid context */
   if ( iconvContext->cd == (iconv_t) BAD ) 
   {
      if (NULL == toStrBuf)
      {
         *ret_toStr = strdup(fromStr);
      }
      else  /* reuse the buffer */
      {
         int len = strlen(fromStr);
         if (len > toStrBufLen)
            *ret_toStr = realloc(toStrBuf,len);
         else 
         {
            *ret_toStr = toStrBuf;
            len = toStrBufLen;
         }
         /* set return values */
         strcpy(*ret_toStr,fromStr);
         *ret_toStrLen = len;
      }
      return (NULL != *ret_toStr ? 0 : -3);    /* RETURN result */
   }

   /* init ptrs */
   toStr = toStrBuf;
   toStrSize = (NULL == toStrBuf ? 0 : toStrBufLen);
   inChar = (const char *)fromStr;
   inBytesLeft = strlen(fromStr);
   outChar = toStr;
   outBytesLeft = toStrSize;

   /* translate the string */
   err = -3;
   while ( inBytesLeft > 0 )
   {
      /* convert a character */
      switch(ICONV(iconvContext->cd,
                       &inChar,&inBytesLeft,&outChar,&outBytesLeft))
      {
      case 0: /* no error */
         continue;
      case 1: /* inChar pts to incomplete multibyte char */
         inBytesLeft = 0;       /* end the translation */
         err = -5;
         break;
      case 2: /* invalid char at inChar */
         inBytesLeft = 0;       /* end the translation */
         err = -6;
         break;
      case 3: /* no room in toStr */
         /* provide enough mem in the toStr */
         if (outBytesLeft < sizeof(wchar_t))
         {
            size_t offset = outChar - (char *) toStr;

            outBytesLeft += MEM_INC;
            toStrSize += MEM_INC;
            toStr = realloc(toStr,toStrSize * sizeof(char));
            if (NULL == toStr)
            {
               inBytesLeft = 0;       /* end the translation */
               err = -3;
               break;                 /* BREAK */
            }
            outChar = (char *) (toStr + offset); /* recalc ptr */
         }
         break;

      }  /* switch on convertsion result */
   } /* while chars left to translate */

   /* set the EOS */
   if(outChar) *outChar = EOS;

   /* set return values */
   *ret_toStr = (char *) toStr;
   if (toStrBuf) *ret_toStrLen = toStrSize;

   return (NULL != toStr ? 0 : err);    /* RETURN result */
}
#endif /* defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3) */

#if !defined(_DONT_USE_XPG4_ICONV)
{  /* XPG4-compliant code (args dflt1 & dflt2 are ignored in this code) */
   char *           toStr;        /* ptr to tostr memory */
   int              toStrSize;    /* size of mallocd string */
   size_t           inBytesLeft;  /* bytes left to use from input buf */
   const char *     inChar;       /* ptr into fromstr */
   char *           outChar;      /* ptr into tostr  */
   size_t           outBytesLeft; /* bytes left in the output str */
   int              err;          /* error code of function */

#define BAD (-1)
#define MEM_INC 20

   /* init ret values; allows function to be called nicely in a loop.  */
   if (ret_toStr) *ret_toStr = toStrBuf;
   if (ret_toStrLen) *ret_toStrLen = toStrBufLen;

   /* check args */
   if (   !iconvContext || !fromStr || !ret_toStr
       || (!ret_toStrLen && toStrBuf))
      return -1;                         /* RETURN error */

   /* just do a straight copy if codesets the same or invalid context */
   if ( iconvContext->cd == (iconv_t) BAD ) 
   {
      if (NULL == toStrBuf)
      {
         *ret_toStr = strdup(fromStr);
      }
      else  /* reuse the buffer */
      {
         int len = strlen(fromStr);
         if (len > toStrBufLen)
            *ret_toStr = realloc(toStrBuf,len);
         else 
         {
            *ret_toStr = toStrBuf;
            len = toStrBufLen;
         }
         /* set return values */
         strcpy(*ret_toStr,fromStr);
         *ret_toStrLen = len;
      }
      return (NULL != *ret_toStr ? 0 : -3);    /* RETURN result */
   }


   /* init ptrs */
   toStr = toStrBuf;
   toStrSize = (NULL == toStrBuf ? 0 : toStrBufLen);
   inChar = (const char *)fromStr;
   inBytesLeft = strlen(fromStr);
   outChar = toStr;
   outBytesLeft = toStrSize;

   /* translate the string */
   err = -3;
   while ( inBytesLeft > 0 )
   {
      /* convert a character */
      if(iconv(iconvContext->cd,
                 (char **)&inChar,&inBytesLeft,&outChar,&outBytesLeft) == -1)
      {
         switch(errno)
         {
         case 0: /* no error */
            continue;
         case EINVAL: /* inChar pts to incomplete multibyte char */
            inBytesLeft = 0;       /* end the translation */
            err = -5;
            break;
         case EILSEQ: /* invalid char at inChar */
            inBytesLeft = 0;       /* end the translation */
            err = -6;
            break;
         case E2BIG: /* no room in toStr */
            /* provide enough mem in the toStr */
            if (outBytesLeft < sizeof(wchar_t))
            {
               size_t offset = outChar - (char *) toStr;
   
               outBytesLeft += MEM_INC;
               toStrSize += MEM_INC;
               toStr = realloc(toStr,toStrSize * sizeof(char));
               if (NULL == toStr)
               {
                  inBytesLeft = 0;       /* end the translation */
                  err = -3;
                  break;                 /* BREAK */
               }
               outChar = (char *) (toStr + offset); /* recalc ptr */
            }
            break;
         default:
            inBytesLeft = 0;             /* breakout of loop */
            break;
         }  /* switch on convertsion result */
      } /* if an error */
   } /* while chars left to translate */

   /* set the EOS */
   if(outChar) *outChar = EOS;

   /* set return values */
   *ret_toStr = (char *) toStr;
   if (toStrBuf) *ret_toStrLen = toStrSize;

   return (NULL != toStr ? 0 : err);    /* RETURN result */
}
#endif  /* XPG4-compliant systems */


/*****************************************************************************
 * Function:        void _DtHelpCeIconvClose()
 *
 * Parameters:      
 *    io_iconvContext;      context
 *
 * Return Value:    none
 *
 * Purpose:
 *              Closes an iconv context used to convert
 *              fromCode to toCode using iconv(3)
 *
 * Comments:
 *              iconv(3) is standardized in XPG4, which is just starting
 *              to be supported.  Below are several different implementations 
 *              of _DtHelpCeIconvStr, each using what is available on different 
 *              platforms.  If no code is #ifdef'd, the XPG4 code defaults.
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpCeIconvClose(iconvContext)
         _DtHelpCeIconvContext * io_iconvContext;
#else
void _DtHelpCeIconvClose(
         _DtHelpCeIconvContext * io_iconvContext)
#endif

#if defined(sun) || defined(USL) || defined(__uxp__)
#define _DONT_USE_XPG4_ICONV
{   /* SunOS doesn't have iconv() */
    _DtHelpCeIconvContextRec * ic;
    if (!io_iconvContext || NULL == *io_iconvContext) return;

    ic = *io_iconvContext;
    if (ic->fromCodeset) free(ic->fromCodeset);
    if (ic->toCodeset) free(ic->toCodeset);

    free(ic);
    *io_iconvContext = NULL;
}
#endif

#if defined(__hpux) && (OSMAJORVERSION == 9) && (OSMINORVERSION <= 3)
#define _DONT_USE_XPG4_ICONV
{
   _DtHelpCeIconvContextRec * ic;
   if (!io_iconvContext || NULL == *io_iconvContext) return;

   ic = *io_iconvContext;
   if ( ic->cd != (iconv_t) BAD ) 
        iconvclose(ic->cd);
   if (ic->table) free(ic->table);
   if (ic->fromCodeset) free(ic->fromCodeset);
   if (ic->toCodeset) free(ic->toCodeset);
   free(ic);
   *io_iconvContext = NULL;
}
#endif /* HPUX 9.00-9.03 */


#if !defined(_DONT_USE_XPG4_ICONV)
{  /* XPG4-compliant code (args dflt1 & dflt2 are ignored in this code) */
   _DtHelpCeIconvContextRec * ic;

   if (!io_iconvContext || NULL == *io_iconvContext) return;
   ic = *io_iconvContext;

   if ( ic->cd != (iconv_t) BAD ) 
        iconv_close(ic->cd);
   if (ic->fromCodeset) free(ic->fromCodeset);
   if (ic->toCodeset) free(ic->toCodeset);
   free(ic);
   *io_iconvContext = NULL;
}
#endif  /* XPG4-compliant systems */



/*****************************************************************************
 * Function:        void _DtHelpCeIconvContextSuitable()
 *
 * Parameters:      
 *    iconvContext:   context
 *    fromCode:       proposed fromCodeset
 *    toCode:         proposed toCodeset
 *
 * Return Value:    True: proposed conversion compatible with this context
 *                  False: proposed conversion is not compatible
 *
 * Purpose:
 *              Checks whether the proposed conversion from
 *              fromCodeset to toCodeset can be handled by
 *              the iconv context that already exists.
 *
 * Comments:
 *              This function is designed to allow a context
 *              to stay open as long as possible and avoid
 *              closing and then reopening the contexts for
 *              the same conversion.
 *****************************************************************************/
#ifdef _NO_PROTO
int _DtHelpCeIconvContextSuitable(iconvContext,fromCode,toCode)
         _DtHelpCeIconvContext iconvContext;
         const char *          fromCode;
         const char *          toCode;
#else
int _DtHelpCeIconvContextSuitable(
         _DtHelpCeIconvContext iconvContext,
         const char *          fromCode,
         const char *          toCode)
#endif
{
   if (   !iconvContext 
       || !iconvContext->fromCodeset 
       || !iconvContext->toCodeset
       || !fromCode 
       || !toCode) 
       return False;

   if (   strcmp(iconvContext->fromCodeset,fromCode) == 0
       && strcmp(iconvContext->toCodeset,toCode) == 0)
       return True;

   return False;
}


#if defined(_AIX) || defined (USL) || defined(__uxp__)
/*****************************************************************************
 * Function: _DtHelpCeStrCaseCmp
 *
 * Parameters:
 *
 * Return Value:
 *
 * Purpose: IBM and USL do not support the 'strcasecmp' routine. This takes it's
 *          place.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrCaseCmp ( s1, s2 )
    const char	*s1;
    const char	*s2;
#else
_DtHelpCeStrCaseCmp (
    const char	*s1,
    const char	*s2)
#endif /* _NO_PROTO */
{
    int   c1;
    int   c2;
    int   result = 0;

    if (s1 == s2)    return  0;
    if (NULL == s1)  return -1;
    if (NULL == s2)  return  1;


    while (result == 0 && *s1 != '\0' && *s2 != '\0')
      {
	c1 = (unsigned char) *s1;
	c2 = (unsigned char) *s2;

	if (isupper(c1))
	    c1 = _tolower(c1);
	if (isupper(c2))
	    c2 = _tolower(c2);

	result = c1 - c2;
	s1++;
	s2++;
      }

    if (result == 0 && (*s1 != '\0' || *s2 != '\0'))
      {
	c1 = (unsigned char) *s1;
	c2 = (unsigned char) *s2;

	if (isupper(c1))
	    c1 = _tolower(c1);
	if (isupper(c2))
	    c2 = _tolower(c2);

	result = c1 - c2;
      }

    return result;
}

/*****************************************************************************
 * Function: _DtHelpCeStrNCaseCmp
 *
 * Parameters:
 *
 * Return Value:
 *
 * Purpose: IBM and USL do not support the 'strcasecmp' routine. This takes it's
 *          place.
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeStrNCaseCmp ( s1, s2, n )
    const char	*s1;
    const char	*s2;
    size_t	 n;
#else
_DtHelpCeStrNCaseCmp (
    const char	*s1,
    const char	*s2,
    size_t	 n)
#endif /* _NO_PROTO */
{
    int   c1;
    int   c2;
    int   result = 0;

    if (s1 == s2 || n < 1) return  0;
    if (NULL == s1)        return -1;
    if (NULL == s2)        return  1;


    while (result == 0 && *s1 != '\0' && *s2 != '\0' && n > 0)
      {
	c1 = (unsigned char) *s1;
	c2 = (unsigned char) *s2;

	if (isupper(c1))
	    c1 = _tolower(c1);
	if (isupper(c2))
	    c2 = _tolower(c2);

	result = c1 - c2;
	s1++;
	s2++;
	n--;
      }

    if (result == 0 && n > 0 && (*s1 != '\0' || *s2 != '\0'))
      {
	c1 = (unsigned char) *s1;
	c2 = (unsigned char) *s2;

	if (isupper(c1))
	    c1 = _tolower(c1);
	if (isupper(c2))
	    c2 = _tolower(c2);

	result = c1 - c2;
      }

    return result;
}
#endif /* _AIX or USL */

/******************************************************************************
 * Function:     int _DtHelpCeFreeStringArray (char **array)
 *
 * Parameters:   array           A pointer to the NULL-terminated
 *                               string array which is to be freed.
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * Purpose:      Free the memory used for a NULL-terminated string array.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFreeStringArray (array)
     char       **array;
#else
_DtHelpCeFreeStringArray (char **array)
#endif /* _NO_PROTO */
{
    char        **next;

    if (array == NULL)
        return -1;

    for (next = array; *next != NULL; next++)
      free(*next); /* use free instead of XtFree - was allocated using malloc */

    free (array);
    return (0);
}

/****************************************************************************
 * Function:    void **_DtHelpCeAddPtrToArray (void **array, void *ptr)
 *
 * Parameters:  array           A pointer to a NULL-terminated array
 *                              of pointers.
 *              ptr             The pointer which is to be added to
 *                              the end of the array.
 *
 * Returns:     A pointer to the NULL-terminated array created
 *              by adding 'ptr' to the end of 'array'.
 *
 * Purpose:     Add a new element to a NULL-terminated array of pointers.
 *              These are typed as "void *" so that they can be used with
 *              pointers to any type of data.
 *
 ****************************************************************************/
void **
#ifdef _NO_PROTO
_DtHelpCeAddPtrToArray (array, ptr)
     void  **array;
     void   *ptr;
#else
_DtHelpCeAddPtrToArray (
       void  **array,
       void   *ptr)
#endif /* _NO_PROTO */
{

    void **nextP = NULL;
    int numElements;

    if (ptr == NULL)
        return NULL;

    /* If this is the first item for the array, malloc the array and set
       nextP to point to the first element. */
    if (array == NULL || *array == NULL) {
        array = (void **) malloc (REALLOC_INCR * sizeof (void *));

        nextP = array;
    }

    else {

        /* Find the NULL pointer at the end of the array. */
        numElements = 0;
        for (nextP = array; *nextP != NULL; nextP++)
                numElements++;

        /* The array always grows by chunks of size REALLOC_INCR.  So see if
           it currently is an exact multiple of REALLOC_INCR size (remember to
           count the NULL pointer).  If it is then it must be full, so realloc
           another chunk.  Also remember to move 'nextP' because the array
           will probably move in memory. */
        if ((numElements + 1) % REALLOC_INCR == 0) {
            array = (void **) realloc (array,
                        (numElements + 1 + REALLOC_INCR) * sizeof (void *));
            if (array)
                nextP = array + numElements;
            else
                nextP = NULL;
        }
    }

    if (nextP)
      {
        *nextP++ = ptr;
        *nextP = NULL;
      }

    return (array);
}


/******************************************************************************
 * Function: _DtHelpCeStrtok (char *s1, char *s1, int max_len)
 *
 *    Returns a pointer to the span of characters in s1 terminated by
 *    one of the characters in s2.  Only s1 can be multibyte.
 *****************************************************************************/

char *
#ifdef _NO_PROTO
_DtHelpCeStrtok( s1, s2 , max_len)
        char *s1 ;
        char *s2 ;
        int max_len ;
#else
_DtHelpCeStrtok(
        char *s1,
        char *s2,
        int max_len )
#endif /* _NO_PROTO */
{
   static char * ptr = NULL;
   char * return_ptr;
   int len;
   int offset;
   int rtn_len = 0;

   /* Use standard libc function, if no multibyte */
   if (max_len == 1)
      return(strtok(s1, s2));

   /*
    * If this is the first call, save the string pointer, and bypass
    * any leading separators.
    */
   if (s1) {
	(void)_DtHelpCeStrspn(s1, s2, max_len, &rtn_len);
      ptr = s1 + rtn_len;
	rtn_len = 0;
   }

   /* A Null string pointer has no tokens */
   if (ptr == NULL)
      return(NULL);

   /* Find out where the first terminator is */
	(void)_DtHelpCeStrcspn(ptr, s2, max_len, &rtn_len);
   if ((len = rtn_len) <= 0)
   {
      /* No tokens left */
      return(NULL);
   }
   /* Keep track of where the token started */
   return_ptr = ptr;

   /* Null out the terminator; we need to know how many bytes are
    * occupied by the terminator, so that we can skip over it to
    * the next character.
    */
   offset = mblen(ptr + len, MB_CUR_MAX);
   *(ptr + len) = '\0';
   ptr += (len + offset);

  /*
   * In preparation for the next pass, skip any other occurrances of
   * the terminator characters which were joined with the terminator
   * we first encountered.
   */
	(void)_DtHelpCeStrspn(ptr, s2, max_len, &len);
   ptr += len;

   return(return_ptr);
}

