/* $XConsortium: CEUtil.c /main/cde1_maint/1 1995/07/17 17:21:58 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	CEUtil.c
 **
 **   Project:     Text Graphic Display Library
 **
 **  
 **   Description: Semi private format utility functions that do not
 **		   require the Display Area, Motif, Xt or X11.
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
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


extern	int	errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "CanvasError.h"
#include "bufioI.h"
#include "CESegmentI.h"
#include "CEUtilI.h"
#include "LocaleXlate.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
#define GROW_SIZE	 5

static const char *DfltStdCharset = "ISO-8859-1";
static const char *DfltStdLang = "C";

static char       MyPlatform[_DtPLATFORM_MAX_LEN+1];
static _DtXlateDb MyDb = NULL;
static CEBoolean  MyProcess = False;
static CEBoolean  MyFirst   = True;
static int        ExecVer;
static int        CompVer;

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function: int _DtHelpCeAddOctalToBuf (char *src, char **dst,
 *					int *dst_size,
 *					int *dst_max, int grow_size)
 *
 * Parameters:
 *		src		Specifies a pointer to a string.
 *		dst		Specifies a pointer to the buffer to
 *					to hold the information.
 *		dst_size	Specifies the current size of 'dst'.
 *				Returns the new size of 'dst'.
 *		dst_max		Specifies the current maximum size of 'dst'.
 *				Returns the new maximum size of 'dst'.
 *		grow_size	Specifies the minimum grow size of 'dst'
 *					when a malloc/realloc occurs.
 *				If this is less than one, 'dst' will
 *					grow only large enough to hold
 *					the new character.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		EINVAL
 *		CEErrorMalloc
 *
 * Purpose:	(Re-)Allocates, if necessary, enough memory to hold the old
 *			information plus the byte.
 *		Coverts the 0xXX value pointed to by src to a 0-256 by value.
 *		Appends the character to the buffer pointed to 'dst'.
 *		Updates 'dst_size' to include the new character.
 *		Updates 'dst_max' to the new size of 'dst' if a
 *			malloc/realloc occurred.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAddOctalToBuf (src, dst, dst_size, dst_max, grow_size)
	char	 *src;
	char	**dst;
	int	 *dst_size;
	int	 *dst_max;
	int	  grow_size;
#else
_DtHelpCeAddOctalToBuf(
	char	 *src,
	char	**dst,
	int	 *dst_size,
	int	 *dst_max,
	int	  grow_size )
#endif /* _NO_PROTO */
{
    char     tmp;
    char    *dstPtr;
    unsigned long value;

    if (src == NULL ||
		dst == NULL || dst_size == NULL || dst_max == NULL ||
				(*dst == NULL && (*dst_size || *dst_max)))
      {
	errno = EINVAL;
	return -1;
      }

    dstPtr = *dst;
    if ((*dst_size + 2) >= *dst_max)
      {
	if (grow_size > *dst_size + 3 - *dst_max)
	    *dst_max = *dst_max + grow_size;
	else
	    *dst_max = *dst_size + 3;

	if (dstPtr)
	    dstPtr = (char *) realloc ((void *) dstPtr, *dst_max);
	else
	  {
	    dstPtr = (char *) malloc (sizeof(char) * (*dst_max));
	    *dst_size = 0;
	  }
	*dst = dstPtr;
      }

    /*
     * check to see if we have good memory
     */
    if (!dstPtr)
      {
	errno = CEErrorMalloc;
	return -1;
      }

    tmp = src[4];
    src[4] = '\0';
    value = strtoul (src, NULL, 16);
    src[4] = tmp;

    if ((value == ULONG_MAX && errno == ERANGE) || value > 255 || value < 1)
      {
	errno = CEErrorFormattingValue;
	return -1;
      }

    /*
     * copy the source into the destination
     */
    dstPtr[*dst_size] = (char ) value;

    /*
     * adjust the pointers.
     */
    *dst_size = *dst_size + 1;

    /*
     * null the end of the buffer.
     */
    dstPtr[*dst_size] = '\0';

    return 0;
}

/******************************************************************************
 * Function: int __CEAppendCharToInfo (char **src, char **dst, int *dst_size,
 *					int *dst_max, int grow_size)
 *
 * Parameters:
 *		src		Specifies a pointer to a string.
 *		dst		Specifies a pointer to the buffer to
 *					to hold the information.
 *		dst_size	Specifies the current size of 'dst'.
 *				Returns the new size of 'dst'.
 *		dst_max		Specifies the current maximum size of 'dst'.
 *				Returns the new maximum size of 'dst'.
 *		grow_size	Specifies the minimum grow size of 'dst'
 *					when a malloc/realloc occurs.
 *				If this is less than one, 'dst' will
 *					grow only large enough to hold
 *					the new character.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		EINVAL
 *		CEErrorMalloc
 *
 * Purpose:	(Re-)Allocates, if necessary, enough memory to hold the old
 *			information plus the new.
 *		Appends the character pointed to by 'src' to the buffer
 *			pointed to 'dst'.
 *		Updates 'src' to point to the next character after the
 *			one appended to 'dst'.
 *		Updates 'dst_size' to include the new character.
 *		Updates 'dst_max' to the new size of 'dst' if a
 *			malloc/realloc occurred.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAddCharToBuf (src, dst, dst_size, dst_max, grow_size)
	char	**src;
	char	**dst;
	int	 *dst_size;
	int	 *dst_max;
	int	  grow_size;
#else
_DtHelpCeAddCharToBuf(
	char	**src,
	char	**dst,
	int	 *dst_size,
	int	 *dst_max,
	int	  grow_size )
#endif /* _NO_PROTO */
{
    char    *srcPtr;
    char    *dstPtr;

    if (src == NULL || *src == NULL ||
		dst == NULL || dst_size == NULL || dst_max == NULL ||
				(*dst == NULL && (*dst_size || *dst_max)))
      {
	errno = EINVAL;
	return -1;
      }

    srcPtr = *src;
    dstPtr = *dst;
    if ((*dst_size + 2) >= *dst_max)
      {
	if (grow_size > *dst_size + 3 - *dst_max)
	    *dst_max = *dst_max + grow_size;
	else
	    *dst_max = *dst_size + 3;

	if (dstPtr)
	    dstPtr = (char *) realloc ((void *) dstPtr, *dst_max);
	else
	  {
	    dstPtr = (char *) malloc (*dst_max);
	    *dst_size = 0;
	  }
	*dst = dstPtr;
      }

    /*
     * check to see if we have good memory
     */
    if (!dstPtr)
      {
	errno = CEErrorMalloc;
	return -1;
      }

    /*
     * copy the source into the destination
     */
    dstPtr[*dst_size] = *srcPtr++;

    /*
     * adjust the pointers.
     */
    *src = srcPtr;
    *dst_size = *dst_size + 1;

    /*
     * null the end of the buffer.
     */
    dstPtr[*dst_size] = '\0';

    return 0;
}

/******************************************************************************
 * Function: int _DtHelpCeAddStrToBuf (char **src, char **dst, int *dst_size,
 *				int *dst_max, int copy_size, int grow_size)
 * 
 * Parameters:
 *		src		Specifies a pointer to a string.
 *		dst		Specifies a pointer to the buffer to
 *					to hold the information.
 *		dst_size	Specifies the current size of 'dst'.
 *				Returns the new size of 'dst'.
 *		dst_max		Specifies the current maximum size of 'dst'.
 *				Returns the new maximum size of 'dst'.
 *		copy_size	Specifies the number of characters to
 *					copy from 'src' to 'dst'.
 *		grow_size	Specifies the minimum grow size of 'dst'
 *					when a malloc/realloc occurs.
 *				If this is less than one, 'dst' will
 *					grow only large enough to hold
 *					the new character.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		EINVAL
 *		CEErrorMalloc
 *
 * Purpose:	Copys 'copy_size' number of characters of 'src'
 *			to 'dst'.
 *		Updates 'src', to point after 'copy_size' number of
 *			characters.
 *		Updates the 'dst_size' to reflect the number of characters
 *			copied.
 *		If required, increments dst_max and (re)allocs memory
 *			to hold the extra 'copy_size' number of characters.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAddStrToBuf (src, dst, dst_size, dst_max, copy_size, grow_size)
	char	**src;
	char	**dst;
	int	 *dst_size;
	int	 *dst_max;
	int	  copy_size;
	int	  grow_size;
#else
_DtHelpCeAddStrToBuf (
	char	**src,
	char	**dst,
	int	 *dst_size,
	int	 *dst_max,
	int	  copy_size,
	int	  grow_size )
#endif /* _NO_PROTO */
{
    char *srcPtr;
    char *dstPtr;

    /*
     * check the input
     */
    if (src == NULL || *src == NULL || (((int)strlen(*src)) < copy_size)
		|| dst == NULL || dst_size == NULL || dst_max == NULL
		|| (*dst == NULL && (*dst_size || *dst_max)))
      {
	errno = EINVAL;
	return -1;
      }

    srcPtr = *src;
    dstPtr = *dst;

    if ((*dst_size + copy_size + 1) >= *dst_max)
      {
	if (grow_size > (*dst_size + copy_size + 2 - *dst_max))
	    *dst_max = *dst_max + grow_size;
	else
	    *dst_max = *dst_size + copy_size + 2;

	if (dstPtr)
	    dstPtr = (char *) realloc ((void *) dstPtr, *dst_max);
	else
	  {
	    dstPtr = (char *) malloc (*dst_max);
	    *dst_size = 0;
	  }
	*dst = dstPtr;
      }

    if (!dstPtr)
      {
	errno = CEErrorMalloc;
	return -1;
      }

    /*
     * make sure there is a null byte to append to.
     */
    dstPtr[*dst_size] = '\0';

    /*
     * copy the source into the destination
     */
    strncat (dstPtr, srcPtr, copy_size);

    /*
     * adjust the pointers
     */
    *src = srcPtr + copy_size;
    *dst_size = *dst_size + copy_size;

    return 0;
}

/******************************************************************************
 * Function: int _DtHelpCeGetNxtBuf (FILE *file, char *dst, char **src,
 *					int max_size)
 * 
 * Parameters:
 *		file		Specifies a stream to read from.
 *		dst		Specifies the buffer where new information
 *					is placed.
 *		src		Specifies a pointer into 'dst'. If there
 *					is information left over, it
 *					is moved to the begining of 'dst'.
 *				Returns 'src' pointing to 'dst'.
 *		max_size	Specifies the maximum size of 'dst'.
 *
 * Returns:	 0 if this is the last buffer that can be read for the topic.
 *		-1 if errors.
 *		>0 if more to be read.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorReadEmpty
 *
 * Purpose:	Reads the next buffer of information.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetNxtBuf (file, dst, src, max_size)
    BufFilePtr	  file;
    char	 *dst;
    char	**src;
    int		  max_size;
#else
_DtHelpCeGetNxtBuf(
    BufFilePtr	  file,
    char	 *dst,
    char	**src,
    int		  max_size)
#endif /* _NO_PROTO */
{
    int	leftOver;
    int	result;

    if (file == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    (void ) strcpy (dst, (*src));
    leftOver = strlen (dst);

    result = _DtHelpCeReadBuf (file, &(dst[leftOver]), (max_size - leftOver));

    /*
     * check to see if we ran into trouble reading this buffer
     * of information. If not reset the pointer to the beginning
     * of the buffer.
     */
    if (result != -1)
	*src = dst;

    return result;
}


/******************************************************************************
 * Function: 	int  _DtHelpCeReadBuf (FILE *file, char *buffer, int size)
 *
 * Parameters:	FILE		Specifies the stream to read from.
 *		buffer		Specifies a buffer to read information
 *					into.
 *		size	 	Specifies the maximum number of bytes
 *				'buffer' can contain. It should never be
 *				larger than 'buffer' can hold, but it can
 *					be smaller.
 *
 * Returns:	 0 if this is the last buffer that can be read for the topic.
 *		-1 if errors.
 *		>0 if more to be read.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *
 * Purpose:	Get size-1 number of bytes into a buffer and possibly
 *		check for page markers imbedded within the text.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeReadBuf(file, buffer, size)
    BufFilePtr	 file;
    char	*buffer;
    int		 size;
#else
_DtHelpCeReadBuf(
    BufFilePtr	 file,
    char	*buffer,
    int		 size)
#endif /* _NO_PROTO */
{
    int    flag;

    /*
     * take into account the last byte must be an end of string marker.
     */
    size--;

    flag = _DtHelpCeBufFileRd(file, buffer, size);

    if (flag != -1)
	buffer[flag] = '\0';

    return flag;

} /* End _DtHelpCeReadBuf */

/******************************************************************************
 * Function:    char *_DtHelpCeGetNxtToken (char *str, char **retToken)
 *
 * Parameters:
 *		str             The string (in memory) which is being
 *					parsed.
 *              retToken        Returns the next token from the input.
 *				Valid tokens are strings of non-whitespace
 *                              characters, newline ("\n"), and
 *                              end-of-data (indicated by a zero length
 *                              string).
 *
 *				A NULL value indicates an error.
 *
 *                              Newline or zero length strings are
 *                              not owned by the caller.
 *
 *                              Otherwise, the memory for the returned
 *                              token is owned by the caller.
 *
 * Return Value:	Returns the pointer to the next unparsed character in
 *			the input string. A NULL value indicates an error.
 *
 * errno Values:
 *		EINVAL
 *		CEErrorMalloc
 *
 * Purpose:     Parse tokens in resource string values.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetNxtToken (str, retToken)
     char       *str;
     char       **retToken;
#else
_DtHelpCeGetNxtToken (
    char        *str,
    char        **retToken)
#endif /* _NO_PROTO */
{
    int          len = 1;
    char        *start;
    char        *token;
    short	 quote = False;
    short	 done = False;

    if (retToken) *retToken = NULL;  /* tested in caller code */

    if (str == NULL || *str == '\0' || retToken == NULL)
      {
	errno = EINVAL;
	return NULL;
      }

    /* Find the next token in the string.  The parsing rules are:

         - Whitespace (except for \n) separates tokens.
         - \n is a token itself.
         - The \0 at the end of the string is a token.
     */

    /* Skip all of the whitespace (except for \n). */
    while (*str && (*str != '\n') && isspace (*str))
        str++;

    /* Str is pointing at the start of the next token.  Depending on the
       type of token, malloc the memory and copy the token value. */
    if (*str == '\0')
        token = str;

    else if (*str == '\n') {
        token = str;
        str++;
    }

    else {
        /* We have some non-whitespace characters.  Find the end of */
        /* them and copy them into new memory. */
	if ((MB_CUR_MAX == 1 || mblen (str, MB_CUR_MAX) == 1) && *str == '\"')
	  {
	    /*
	     * found a quoted token - skip the quote.
	     */
	    quote = True;
	    str++;
	  }

        start = str;
        while (*str && !done)
	  {
	    /*
	     * get the length of the item.
	     */
	    len = 1;
	    if (MB_CUR_MAX != 1)
	      {
	        len = mblen (str, MB_CUR_MAX);
	        if (len < 0)
		    len = 1;
	      }
	    if (len == 1)
	      {
		/*
		 * check for the token terminator
		 */
	        if ((quote && *str == '\"') ||
				(!quote && (isspace (*str) || *str == '\n')))
		    done = True;
		else
		    str++;
	      }
	    else
	        str += len;
	  }

	/*
	 * determine the length of the token.
	 */
        token = (char *) malloc ((str - start + 1) * sizeof (char));
	if (token)
	  {
            strncpy (token, start, str - start);
            *(token + (str - start)) = '\0';
	  }
	else
	    errno = CEErrorMalloc;

	/*
	 * skip the quote terminator
	 */
	if (quote && len == 1 && *str == '\"')
	    str++;
    }

    *retToken = token;
    return (str);
}


/******************************************************************************
 * Function:    static int OpenLcxDb ()
 *
 * Parameters:   none
 *
 * Return Value:  0: ok
 *               -1: error
 *
 * errno Values:
 *
 * Purpose: Opens the Ce-private Lcx database
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
OpenLcxDb ()
#else
OpenLcxDb (void)
#endif /* _NO_PROTO */
{
    time_t	time1  = 0;
    time_t	time2  = 0;

    /*
     * wait 30 sec. until another thread or enter is done modifying the table
     */
    while (MyProcess == True) 
      {
        /* if time out, return */
	if (time(&time2) == (time_t)-1)
	    return -1;

        if (time1 == 0)
	    time1 = time2;
	else if (time2 - time1 >= (time_t)30)
	    return -1;
      }

    if (MyFirst == True)
      {
        MyProcess = True;
        if (_DtLcxOpenAllDbs(&MyDb) == 0 &&
			_DtXlateGetXlateEnv(MyDb,MyPlatform,&ExecVer,&CompVer) != 0)
	  {
	    _DtLcxCloseDb(&MyDb);
	    MyDb = NULL;
	  }
	MyFirst = False;
        MyProcess = False;
      }

    return (MyDb == NULL ? -1 : 0 );
}

/******************************************************************************
 * Function:    int _DtHelpCeGetMbLen (char *lang, char *char_set)
 *
 * Parameters:
 *		lang            Represents the language. A NULL value
 *				defaults to "C".
 *              char_set        Represents the character set. A NULL
 *				value value defaults to "ISO-8859-1"
 *
 * Return Value:	Returns the MB_CUR_MAX for the combination
 *			lang.charset.
 *
 * errno Values:
 *
 * Purpose: To determine the maximum number of bytes required to display
 *          a character if/when the environment is set to 'lang.charset'
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetMbLen (lang, char_set)
     char       *lang;
     char       *char_set;
#else
_DtHelpCeGetMbLen (
     char       *lang,
     char       *char_set)
#endif /* _NO_PROTO */
{
    int		  retLen = 1;

    if (lang == NULL)
	lang = (char *)DfltStdLang;

    if (char_set == NULL)
	char_set = (char *)DfltStdCharset;

    if (OpenLcxDb() == 0)
      {
        /* if translation is present, lang.charset are a multibyte locale */
	if (_DtLcxXlateStdToOp(MyDb, MyPlatform, CompVer, DtLCX_OPER_MULTIBYTE,
				NULL, lang, char_set, NULL, NULL) == 0)
	    retLen = MB_CUR_MAX;
      }

    return retLen;
}

/******************************************************************************
 * Function:    int _DtHelpXlateGetStdToOpLocale ( char *operation, char *stdLocale,
 *                                      char *dflt_opLocale, char **ret_opLocale)
 *
 * Parameters:
 *    operation		operation whose locale value will be retrieved
 *    stdLocale		standard locale value
 *    dflt_opLocale	operation-specific locale-value
 *			This is the default value used in error case
 *    ret_opLocale	operation-specific locale-value placed here
 *			Caller must free this string.
 *
 * Return Value:
 *
 * Purpose: Gets an operation-specific locale string given the standard string
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeXlateStdToOpLocale (operation, stdLocale, dflt_opLocale, ret_opLocale)
     char       *operation;
     char       *stdLocale;
     char       *dflt_opLocale;
     char       **ret_opLocale;
#else
_DtHelpCeXlateStdToOpLocale (
     char       *operation,
     char       *stdLocale,
     char       *dflt_opLocale,
     char       **ret_opLocale)
#endif /* _NO_PROTO */
{
    int result = OpenLcxDb();

    if (result == 0)
      {
	(void) _DtLcxXlateStdToOp(MyDb, MyPlatform, CompVer,
			operation, stdLocale, NULL, NULL, NULL, ret_opLocale);
      }

    /* if translation fails, use a default value */
    if (ret_opLocale && (result != 0 || *ret_opLocale == NULL))
      {
        if (dflt_opLocale) *ret_opLocale = strdup(dflt_opLocale);
        else if (stdLocale) *ret_opLocale = strdup(stdLocale);
      }
}


/******************************************************************************
 * Function:    int _DtHelpCeXlateOpToStdLocale (char *operation, char *opLocale,
 *                         char **ret_stdLocale, char **ret_stdLang, char **ret_stdSet)
 *
 * Parameters:
 *              operation       Operation associated with the locale value
 *              opLocale	An operation-specific locale string
 *              ret_locale      Returns the std locale
 *				Caller must free this string.
 *		ret_stdLang        Returns the std language & territory string.
 *				Caller must free this string.
 *              ret_stdSet         Returns the std code set string.
 *				Caller must free this string.
 *
 * Return Value:
 *
 * Purpose:  Gets the standard locale given an operation and its locale
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeXlateOpToStdLocale (operation, opLocale, ret_stdLocale, ret_stdLang, ret_stdSet)
     char       *operation;
     char       *opLocale;
     char       **ret_stdLocale;
     char       **ret_stdLang;
     char       **ret_stdSet;
#else
_DtHelpCeXlateOpToStdLocale (
     char       *operation,
     char       *opLocale,
     char       **ret_stdLocale,
     char       **ret_stdLang,
     char       **ret_stdSet)
#endif /* _NO_PROTO */
{
    int result = OpenLcxDb();

    if (result == 0)
      {
	(void) _DtLcxXlateOpToStd(MyDb, MyPlatform, CompVer,
				operation,opLocale,
				ret_stdLocale, ret_stdLang, ret_stdSet, NULL);
      }

    /* if failed, give default values */
    if (ret_stdLocale != NULL && (result != 0 || *ret_stdLocale == NULL))
      {
        *ret_stdLocale = malloc(
				strlen(DfltStdLang)+strlen(DfltStdCharset)+3);
	sprintf(*ret_stdLocale,"%s.%s",DfltStdLang,DfltStdCharset);
      }
    if (ret_stdLang != NULL && (result != 0 || *ret_stdLang == NULL))
	*ret_stdLang = strdup(DfltStdLang);
    if (ret_stdSet != NULL && (result != 0 || *ret_stdSet == NULL))
	*ret_stdSet = strdup(DfltStdCharset);
}
