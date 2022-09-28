/* $XConsortium: UtilSDL.c /main/cde1_maint/1 1995/07/17 17:53:14 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	   UtilSDL.c
 **
 **   Project:     Cde Help System
 **
 **   Description: Utility functions for parsing an SDL volume.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "CanvasError.h"
#include "bufioI.h"
#include "CESegmentI.h"
#include "CEUtilI.h"
#include "CanvasI.h"
#include "FormatSDLI.h"
#include "StringFuncsI.h"
#include "UtilSDLI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/******************************************************************************
 *
 * Private defines.
 *
 *****************************************************************************/
#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	False
#define	False	0
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	True
#define	True	1
#endif

#define	GROW_SIZE   10
#define	MAX_ELEMENT_SIZE   30

typedef struct  _classStyleMatrix
  {
    enum SdlOption      start;
    enum SdlOption      end;
    enum SdlElement     style;
  } ClassStyleMatrix;

/******************************************************************************
 *
 * Private macros.
 *
 *****************************************************************************/
#define	IsWhiteSpace(x) \
		(((x) == '\n' || (x) == ' ' || (x) == '\t') ? 1 : 0)

#define	IsNumber(x) \
		(((x) >= '0' && (x) <= '9') ? 1 : 0)

/******************************************************************************
 *
 * Private data.
 *
 *****************************************************************************/
static  CESegment DefaultSegment = { True, 0, -1, NULL, NULL, NULL };

static ClassStyleMatrix ClassToStyle[] =
  {
    { SdlClassAcro  , SdlClassUdefkey  , SdlElementKeyStyle  },
    { SdlClassLabel , SdlClassUdefhead , SdlElementHeadStyle },
    { SdlClassTable , SdlClassUdeffrmt , SdlElementFormStyle },
    { SdlClassTable , SdlClassUdeffrmt , SdlElementFrmtStyle },
    { SdlClassFigure, SdlClassUdefgraph, SdlElementGrphStyle },
    { SdlOptionBad  , SdlOptionBad     , SdlElementNone      },
  };

static	const SDLAttrStrings DefaultStrings =
    {
                                /*              el_strs / doc_strs/not_strs */
        NULL,                   /* SDLCdata     colw    /product  /product  */
        NULL,                   /* SDLCdata     colj    /license  /license  */
        NULL,                   /* SDLCdata     text    /timestamp/format   */
        NULL,                   /* SDLCdata     branches/prodpn   /method   */
        NULL,                   /* SDLCdata     abbrev  /prodver  /xid      */
        NULL,                   /* SDLCdata     enter   /author   /command  */
        NULL,                   /* SDLCdata     exit    /srcdtd   /data     */
    };

static  const CEFrmtSpecs DefaultFrmtSpecs =
    {                           /* CEFrmtSpecs  frmt_specs; */
        SdlBorderNone,              /* enum SdlOption   border;  */
        SdlJustifyLeft,             /* enum SdlOption   justify; */
        SdlJustifyTop,              /* enum SdlOption   vjust;   */
         0,                         /* SDLNumber        fmargin; */
         0,                         /* SDLNumber        lmargin; */
         0,                         /* SDLNumber        rmargin; */
         0,                         /* SDLNumber        tmargin; */
         0,                         /* SDLNumber        bmargin; */
    };

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    int FindBreak (char *ptr, int size)
 *
 * Parameters:
 *              ptr     Specifies the string to check.
 *              size    Specifies the character length.
 *
 * Returns      number of bytes that have a charater length of 'size'.
 *
 * errno Values:
 *
 * Purpose:     Find a length of 'ptr' comprised of characters of 'size'
 *              length.
 *
 *****************************************************************************/
static  int
#ifdef _NO_PROTO
FindBreak (ptr, size)
    char *ptr;
    int   size;
#else
FindBreak (
    char *ptr,
    int   size )
#endif /* _NO_PROTO */
{
    int   len = 0;

    while (mblen(ptr, size) == size)
      {
        ptr += size;
        len += size;
      }

    return len;
}

/******************************************************************************
 * Function:	static int SkipWhiteSpace (BufFilePtr f)
 *
 * Parameters:
 *
 * Returns	 0 if successfully skipped white space.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
SkipWhiteSpace (f)
    BufFilePtr	 f;
#else
SkipWhiteSpace (
    BufFilePtr	 f)
#endif /* _NO_PROTO */
{
    char   c;

    do {
        c = BufFileGet(f);
	if (c == BUFFILEEOF)
	    return -1;

    } while (IsWhiteSpace(c));

    BufFilePutBack(c, f);


    return 0;
}

/******************************************************************************
 * Function:	static int CheckSDLAttributeNameStart (char c)
 *
 * Parameters:
 *
 * Returns	 0 if starts correctly,
 *		-1 if does not start correctly for an attribute name
 *
 * errno Values:
 *
 * Purpose:
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
CheckSDLAttributeNameStart (c)
    char	c;
#else
CheckSDLAttributeNameStart (char c)
#endif /* _NO_PROTO */
{
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
	return 0;

    return -1;
}

/******************************************************************************
 * Function:	int GetSDLElement (BufFilePtr f, char **string)
 *
 * Parameters:
 *
 * Returns	 0 if got an element (subject to verification).
 *		 1 if end of file/compressed section.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Memory owned:  the returned string.
 *
 * Purpose:
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
GetSDLElement (f, max_allowed, string)
    BufFilePtr	 f;
    int		 max_allowed;
    char	*string;
#else
GetSDLElement (
    BufFilePtr	 f,
    int		 max_allowed,
    char	*string)
#endif /* _NO_PROTO */
{
    CEBoolean     done     = False;
    CEBoolean     first    = True;
    CEBoolean     nonWhite = False;
    CEBoolean     startEl  = False;
    int           len      = 0;
    char    c;

    do {
	c = BufFileGet(f);

	/*
	 * [chars][space]	(nonWhite == True  && IsWhiteSpace(c))
	 * <element>		(startEl  == True  && c == '>')
	 * [chars]<element..	(first    == False && c == '<')
	 */
	if (c == BUFFILEEOF || (nonWhite == True && IsWhiteSpace(c))
		|| (startEl == True  && c == '>')
		|| (first   == False && c == '<'))
	  {
	    if (c == '>'  || c == '<' || (nonWhite == True && IsWhiteSpace(c)))
		BufFilePutBack(c,f);
	    done = True;
	  }
	else
	  {
	    if (first == True && c == '<')
		startEl = True;

	    first = False;
	    if (nonWhite == False && IsWhiteSpace(c) == False)
		nonWhite = True;

	    string[len++] = c;

	    if (len >= max_allowed)
		done = True;
	  }
    } while (!done);

    string[len] = '\0';

    if (len == 0)
      {
	if (c != BUFFILEEOF)
	    return -1;

        else /* if (c == BUFFILEEOF) */
	    return 1;
      }

    return 0;
}

/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	int _DtHelpCeMatchSdlElement (BufFilePtr my_file,
 *					const char *element_str, int sig_chars)
 *
 * Parameters:
 *
 * Returns	 0 if successfully matched element_str,
 *		 1 if did not match,
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeMatchSdlElement (f, element_str, sig_chars)
    BufFilePtr	 f;
    const char	*element_str;
    int		 sig_chars;
#else
_DtHelpCeMatchSdlElement (
    BufFilePtr	 f,
    const char	*element_str,
    int		 sig_chars)
#endif /* _NO_PROTO */
{
    int   len;
    int   different = False;
    char  nextEl[MAX_ELEMENT_SIZE + 2];
    char *ptr;
    char  c;

    /*
     * get the next element in the stream
     */
    do {
        if (GetSDLElement(f, MAX_ELEMENT_SIZE, nextEl) != 0)
	    return -1;

        /*
         * now skip the white space
         */
        ptr = nextEl;
        while (IsWhiteSpace(*ptr)) ptr++;

      } while (*ptr == '\0');

    len = strlen(element_str);

    do {
	/*
	 * get the lowered version of the character
	 */
	c = tolower((int)(*ptr));
	if (c != *element_str)
	    different = True;

	len--;
	sig_chars--;
	element_str++;
	ptr++;

    } while (len && sig_chars && !different && *ptr != '\0');

    if (!different && sig_chars > 0 && len > 0)
	different = True;

    return different;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetSdlAttribute (BufFilePtr f,
 *					char *attribute_name)
 *
 * Parameters:
 *		f		Specifies the buf file to read.
 *		max_len		Specifies the maximum number of characters
 *				that can fit in 'attribute_name'. Does
 *				*NOT* include the terminating null byte.
 *		attribute_name	Specifies the buffer in which to place
 *				the attribute.
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		 1 if there is no more attributes (eats the element end).
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Reads the an attribute.
 *		Lower cases the attribute.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlAttribute (f, max_len, attribute_name)
    BufFilePtr	 f;
    int		 max_len;
    char	*attribute_name;
#else
_DtHelpCeGetSdlAttribute (
    BufFilePtr	 f,
    int		 max_len,
    char	*attribute_name)
#endif /* _NO_PROTO */
{
    int   len = 0;
    char  c;

    if (SkipWhiteSpace(f) != 0)
	return -1;

    /*
     * check that the attribute name starts correctly
     */
    c = BufFileGet(f);
    if (c == '>')
	return 1;

    if (CheckSDLAttributeNameStart(c) != 0)
	return -1;

    do {
	c = tolower((int)c);

	attribute_name[len++] = c;

	c = BufFileGet(f);

    } while (c != BUFFILEEOF && !IsWhiteSpace(c) && c != '=' && len < max_len);

    while (c != '=')
      {
	c = BufFileGet(f);
	if (c == BUFFILEEOF)
	    return -1;
      }

    if (len == 0 || len >= max_len)
	return -1;

    attribute_name[len] = '\0';

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeReturnSdlElement ()
 *
 * Parameters:
 *
 *
 * Returns	 0 if matched an element in the list
 *		 1 hit eof on the file/compressed section.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	number_str
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeReturnSdlElement (f, el_list, cdata_flag,
			ret_el_define, ret_data, ret_end_flag)
    BufFilePtr		  f;
    const SDLElement	 *el_list;
    CEBoolean		  cdata_flag;
    enum SdlElement	 *ret_el_define;
    char		**ret_data;
    CEBoolean		 *ret_end_flag;
#else
_DtHelpCeReturnSdlElement (
    BufFilePtr		  f,
    const SDLElement	 *el_list,
    CEBoolean		  cdata_flag,
    enum SdlElement	 *ret_el_define,
    char	        **ret_data,
    CEBoolean		 *ret_end_flag)
#endif /* _NO_PROTO */
{
    int        result;
    int        lowerChar;
    CEBoolean  endFlag   = False;
    CEBoolean  different = True;
    char       nextEl[MAX_ELEMENT_SIZE + 2];
    char      *ptr;

    *ret_el_define = SdlElementCdata;
    *ret_end_flag  = False;
    if (ret_data)
	*ret_data = NULL;

    /*
     * get the next element
     */
    do {
        result = GetSDLElement(f, MAX_ELEMENT_SIZE, nextEl);
	if (result == 1)
	  {
	    *ret_el_define = SdlElementNone;
	    return 1;
	  }
        else if (result == -1)
	    return -1;

        /*
         * lower case the element. String dup it first incase this is data
         * and not an element. Then skip the leading white space.
         */
        ptr     = nextEl;
        while (IsWhiteSpace(*ptr) == True) ptr++;
      } while (cdata_flag == False && *ptr == '\0');

    /*
     * An element starts with a less than sign,
     * An end element starts with a less than sign and a forward slash.
     */

    /*
     * if all we've got is white space, or a non starting character,
     * then skip the loop
     */
    if (*ptr == '<')
      {
	/*
	 * skip the less than sign.
	 */
	ptr++;

	/*
	 * is this an end element?
	 */
	if (*ptr == '/')
	  {
	    endFlag = True;
	    ptr++;
	  }

	lowerChar = tolower((int)(*ptr));

        while (different && el_list->element_str != NULL)
          {
	    if (endFlag == True && el_list->element_end_str != NULL &&
		lowerChar == ((int)(el_list->element_end_str[2])) &&
		_DtHelpCeStrNCaseCmp(&(el_list->element_end_str[2]),
					ptr, el_list->end_sig_chars - 2) == 0)
	      {
		different = False;
	        *ret_end_flag = True;
	      }
	    else if (endFlag == False &&
		lowerChar == ((int)(el_list->element_str[1])) &&
		 _DtHelpCeStrNCaseCmp(&(el_list->element_str[1]), ptr,
					el_list->str_sig_chars - 1) == 0)
	        different = False;
	    else
	        el_list++;
          }
	*ret_el_define = el_list->sdl_element;
      }

    if (*ret_el_define == SdlElementCdata && ret_data != NULL)
	*ret_data = strdup(nextEl);

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetSdlCdata (BufFilePtr f, char **string)
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved data.
 *		 1 if returning because type != SdlTypeDynamic and a newline
 *		   was found.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	attribute_name
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlCdata (f, abbrev, type, max_mb, nl_to_space, cur_space, cur_nl, cur_mb, string, str_max)
    BufFilePtr	 f;
    SDLCdata abbrev; /* Abbreviation character string */
    enum SdlOption type;
    int		  max_mb;
    CEBoolean     nl_to_space;
    CEBoolean    *cur_space;
    CEBoolean    *cur_nl;
    CEBoolean    *cur_mb;
    char	**string;
    int		  str_max;
#else
_DtHelpCeGetSdlCdata (
    BufFilePtr	 f,
    SDLCdata abbrev,
    enum SdlOption type,
    int		  max_mb,
    CEBoolean     nl_to_space,
    CEBoolean    *cur_space,
    CEBoolean    *cur_nl,
    CEBoolean    *cur_mb,
    char	**string,
    int		  str_max)
#endif /* _NO_PROTO */
{
    CEBoolean  lastWasSpace = *cur_space;
    CEBoolean  lastWasNl    = *cur_nl;
    CEBoolean  lastWasMb    = *cur_mb;
    CEBoolean  ignore  = False;
    CEBoolean  done    = False;
    CEBoolean  escaped = False;
    int        strSize = 0;
    int        reason  = 0;
    int        len     = 0;
    int        strMB   = 1;
    char      *ptr;
    char       buf[MB_LEN_MAX + 1];
#define	ESC_STR_LEN	4
    char       escStr[ESC_STR_LEN];
    char       c;
    int        i;

    if (string == NULL || abbrev != NULL)
	ignore = True;
    else if (*string != NULL)
      {
	strSize = strlen(*string);
	if (str_max < strSize)
	    str_max = strSize;
      }

    /* ------------------------------------------------------------- */
    /*  Get the text either from the buffer file or from its abbrev  */
    /* ------------------------------------------------------------- */

    i = 0;
    do
      {
        if (abbrev != NULL)
          c = abbrev[i];
        else
          c = BufFileGet(f);

	buf[len++] = c;
	buf[len]   = '\0';

	if (c != BUFFILEEOF && max_mb != 1)
	    strMB = mblen(buf, max_mb);

	if (c == BUFFILEEOF || (escaped == False && strMB == 1 &&
		(c == '<' ||
		    ((type == SdlTypeLiteral || type == SdlTypeLined) &&
			c == '\n'))))
	  {
	    if (c == '\n')
		reason = 1;
	    else if (c == '<')
		BufFilePutBack(c,f);

	    done = True;
	  }
	else if (strMB == 1)
	  {
	    len = 0;

	    if (lastWasNl == True)
	      {
		ptr = " ";
	        if (_DtHelpCeAddCharToBuf(&ptr, string, &strSize,
							&str_max, 32) == -1)
		    return -1;
		lastWasSpace = True;
	      }

	    lastWasMb = False;
	    lastWasNl = False;

	    if (escaped == True)
	      {
		if (c == '#')
		  {
		    /*
		     * process the SGML numeric character reference
		     * entity.  '&#d[d[d]][;]' - 'ddd' represent
		     * characters of '0' to '9'.  The semi-colon is
		     * required iff the next character is a numeric
		     * character of '0' to '9'.  Otherwise it is
		     * optional.
		     */
		    int  value;

		    len = 0; /* len should be zero here, but let's make sure */
		    do
		      {
			/*
			 * get the next character.
			 */
			c = BufFileGet(f);

			/*
			 * if it is the first character of the numeric entity,
			 * it has to be a numeric character.
			 */
			if (c == BUFFILEEOF ||
					(len == 0 && (c < '0' || c > '9')))
			    return -1;

			/*
			 * put the character in the buffer
			 */
			escStr[len++] = c;

		      } while (len < ESC_STR_LEN && c >= '0' && c <= '9');

		    /*
		     * the ending character of the entity has to be a
		     * non-numeric character.
		     */
		    if (c >= '0' && c <= '9')
			return -1;
		    /*
		     * if the optional semi-colon is not present, replace
		     * the terminating character back in the input buffer
		     * to be processed in the next pass.
		     */
		    else if (c != ';')
			BufFilePutBack(c, f);

		    /*
		     * get the integer value. It has to be less than 256.
		     */
		    value = atoi(escStr);
		    if (value > 255)
			return -1;

		    /*
		     * transfer the value to the character variable
		     * and zero the index for the general buffer.
		     */
		    c   = value;
		    len = 0;
		  }

		/*
		 * clear the escaped flag, so that the character gets
		 * put in the output buffer.
		 */
	 	escaped = False;
	      }
	    else if (c == '\n')
	      {
		if (type == SdlTypeCdata)
		    lastWasSpace = False;
		else if (lastWasSpace == False)
		    lastWasNl = True;
		continue;
	      }
	    else if (c == '&')
		escaped = True;

	    if (escaped == False && ignore == False)
	      {
	        if (c == '\t')
		    c = ' ';

	        ptr = &c;
	        if (c == '\n')
		  {
		    lastWasSpace = True;
		    reason       = 1;
		    done         = True;
		  }
		else
		  {
		    if (c != ' ')
		        lastWasSpace = False;

		    if (lastWasSpace == False || type == SdlTypeLiteral ||
						type == SdlTypeUnlinedLiteral)
	                if (_DtHelpCeAddCharToBuf(&ptr, string, &strSize,
							&str_max, 32) == -1)
		            return -1;

		    if (type != SdlTypeLiteral &&
				type != SdlTypeUnlinedLiteral && c == ' ')
		        lastWasSpace = True;
		  }
	      }
	  }
	else if (strMB > 1)
	  {
	    if (ignore == False)
	      {
	        if (lastWasNl == True)
	          {
	            if (nl_to_space == True || lastWasMb == False)
	              {
		        ptr = " ";
	                if (_DtHelpCeAddCharToBuf(&ptr, string, &strSize,
							&str_max, 32) == -1)
		            return -1;
	              }
	          }

	        ptr = buf;
	        if (_DtHelpCeAddStrToBuf(&ptr,string,&strSize,&str_max,len,32)
						== -1)
		    return -1;
	      }
	    lastWasMb    = True;
	    lastWasNl    = False;
	    lastWasSpace = False;
	    escaped      = False;
	    len          = 0;
	  }
	else if (strMB == 0 || len > max_mb || len >= MB_LEN_MAX)
	    return -1;

        /* ------------------------------------------------------- */
        /*  In case we are using the abbreviated character string  */
        /* ------------------------------------------------------- */
        if (abbrev != NULL)
          {
            i++;
            if (i == strlen(abbrev))
              done = True;
          }

    } while (done == False);

    *cur_space = lastWasSpace;
    *cur_nl    = lastWasNl;
    *cur_mb    = lastWasMb;
    return reason;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetSdlAttributeCdata (BufFilePtr f, char **string);
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Parses the CDATA value for an element attribute. Expects the
 *		pointer to be after the equals sign of the attribute.  The
 *		CDATA must be included in quotes (either single or double)
 *		and can have white space (spaces, newlines or tabs) preceeding
 *		it.  Any newline encountered within the CDATA value is turned
 *		into a space.  Any quote enclosed in the CDATA must not be the
 *		same type as what is used to delimit the CDATA value.  I.E.
 *		if a single quote is used in the CDATA, the CDATA value must
 *		be enclosed in double quotes.
 *
 * Memory Owned By Caller:	attribute_name
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlAttributeCdata (f, limit, string)
    BufFilePtr	  f;
    CEBoolean	  limit;
    char	**string;
#else
_DtHelpCeGetSdlAttributeCdata (
    BufFilePtr	  f,
    CEBoolean	  limit,
    char	**string)
#endif /* _NO_PROTO */
{
    CEBoolean   ignore      = False;
    CEBoolean   done        = False;
    CEBoolean   firstChar   = True;
    int         strSize     = 0;
    int         strMax      = 0;
    char       *myStr       = NULL;
    char        c;
    char        testChar;

    if (string == NULL)
	ignore = True;
    else if (limit == True)
	myStr = *string;

    if (SkipWhiteSpace(f) != 0)
	return -1;

    do {
	c = BufFileGet(f);
	if (c == BUFFILEEOF)
	  {
	    if (myStr != NULL)
		free(myStr);
	    return -1;
	  }

	if (firstChar)
	  {
	    if (c == '\'' || c == '\"')
	      {
	        testChar  = c;
	        firstChar = False;
	      }
	    else
	      {
	        if (myStr != NULL)
		    free(myStr);
	        return -1;
	      }
	  }
	else if (c == testChar)
	    done = True;
	else if (ignore == False)
	  {
	    if (c == '\n')
		c = ' ';

	    if (limit == True && strSize >= DTD_NAMELEN)
	      {
		if (myStr != NULL)
		    free(myStr);
		return -1;
	      }
	    else if (limit == False && strSize >= strMax)
	      {
		strMax += 64;
		if (strSize == 0)
		    myStr = (char *) malloc (sizeof(char) * 65);
		else
		    myStr = (char *) realloc (myStr,(sizeof(char)*(strMax+1)));

		if (myStr == NULL)
		    return -1;
	      }

	    myStr[strSize++] = c;
	  }
    } while (done == False);

    if (ignore == False)
      {
	if (myStr != NULL)
            myStr[strSize] = '\0';
	if (limit == False)
            *string = myStr;
      }

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeSkipCdata (BufFilePtr f, CEBoolean Flag)
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	attribute_name
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeSkipCdata (f)
    BufFilePtr	  f;
    CEBoolean	  flag;
#else
_DtHelpCeSkipCdata (
    BufFilePtr	  f,
    CEBoolean	  flag)
#endif /* _NO_PROTO */
{
    CEBoolean   done        = False;
    CEBoolean   singleQuote = False;
    CEBoolean   doubleQuote = False;
    CEBoolean   comment     = False;
    CEBoolean   element     = False;
    CEBoolean   bracket     = False;
    CEBoolean   processing  = True;
    char        c;

    do {
	c = BufFileGet(f);
	if (c == BUFFILEEOF)
	    return -1;

	if (processing)
	  {
	    if (c == '\'')
	      {
		singleQuote = True;
		processing  = False;
	      }
	    else if (c == '\"')
	      {
		doubleQuote = True;
		processing  = False;
	      }
	    else if (c == '-')
	      {
		c = BufFileGet(f);
		if (c == BUFFILEEOF)
	    	return -1;

		if (c == '-')
		  {
		    comment    = True;
		    processing = False;
		  }
		else
	    	    BufFilePutBack(c,f);
	      }
	    else if (!element && c == '<')
		  element = True;

	    else if (c == '>')
	      {
		if (element)
		    element = False;
		else
		  {
	    	    BufFilePutBack(c,f);
		    done = True;
		  }
	      }
	    else if (flag && c == '[')
		bracket = True;

	    else if (bracket && c == ']')
	      {
		bracket = False;
		flag    = False;
	      }
	  }
	else
	  {
	    if (singleQuote && c == '\'')
	      {
		singleQuote = False;
		processing  = True;
	      }
	    else if (doubleQuote && c == '\"')
	      {
		doubleQuote = False;
		processing  = True;
	      }
	    else if (comment && c == '-')
	      {
		c = BufFileGet(f);
		if (c == BUFFILEEOF)
		    return -1;

		if (c == '-')
		  {
		    comment     = False;
		    processing  = True;
		  }
		else
		    BufFilePutBack(c,f);
	      }
	  }
    } while (done == False);

    if (flag)
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetSdlId (BufFilePtr f, char **id_value)
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	id_value
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlId (f, id_value)
    BufFilePtr	 f;
    char	**id_value;
#else
_DtHelpCeGetSdlId (
    BufFilePtr	 f,
    char	**id_value)
#endif /* _NO_PROTO */
{
    *id_value = NULL;

    if (_DtHelpCeGetSdlAttributeCdata (f, False, id_value) != 0)
	return -1;

    /*
     * check that the attribute name starts correctly
     */
    if (CheckSDLAttributeNameStart(**id_value) != 0)
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetSdlNumber (BufFilePtr f, char **number_str)
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	number_str
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlNumber (f, number_str)
    BufFilePtr	 f;
    char	**number_str;
#else
_DtHelpCeGetSdlNumber (
    BufFilePtr	 f,
    char	*number_str)
#endif /* _NO_PROTO */
{
    if (_DtHelpCeGetSdlAttributeCdata(f, True, &number_str) != 0)
	return -1;

    /*
     * check that the attribute name starts correctly
     */
    if (!IsNumber(*number_str))
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int _DtHelpCeFindSkipSdlElementEnd (BufFilePtr f)
 *
 * Parameters:
 *
 *
 * Returns	 0 if successfully retrieved an attribute name.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:
 *
 * Memory Owned By Caller:	attribute_name
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFindSkipSdlElementEnd (f)
    BufFilePtr	 f;
#else
_DtHelpCeFindSkipSdlElementEnd (
    BufFilePtr	 f)
#endif /* _NO_PROTO */
{
    char  c;

    if (SkipWhiteSpace(f) != 0)
	return -1;

    do {
	c = BufFileGet(f);
	if (c == BUFFILEEOF)
	  return -1;

    } while (c != '>');

    return 0;
}

/******************************************************************************
 * Function:    int _DtHelpCeMergeSdlAttribInfo (CECanvasStruct canvas,
 *				CEElement *src_el,
 *				CEElement *dst_el);
 *
 * Parameters:
 *
 * Returns:     nothing
 *
 * Purpose: Merge the src specified attributes into the dst.
 *	    THIS WILL NOT MERGE THE SSI or RSSI values.
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeMergeSdlAttribInfo(src_el, dst_el)
    CEElement		*src_el;
    CEElement		*dst_el;
#else
_DtHelpCeMergeSdlAttribInfo(
    CEElement		*src_el,
    CEElement		*dst_el)
#endif /* _NO_PROTO */
{
    CESDLAttrStruct *src, *dst;
    const SDLAttribute	*pAttr;
    enum SdlOption	*dstEnum;
    enum SdlOption	*srcEnum;
    SDLNumber		*dstNum;
    SDLNumber		*srcNum;
    SDLCdata		*dstStr;
    SDLCdata		*srcStr;

    if (src_el != NULL)
      {
	src   = &(src_el->attributes);
	dst   = &(dst_el->attributes);

        for (pAttr = _DtHelpCeGetSdlAttributeList();
			pAttr->data_type != SdlAttrDataTypeInvalid; pAttr++)
          {
	    if (_CEIsAttrSet(*src, pAttr->sdl_attr_define) &&
		!(ATTRS_EQUAL(pAttr->data_type, pAttr->sdl_attr_define,
				SDL_ATTR_SSI) ||
		  ATTRS_EQUAL(pAttr->data_type, pAttr->sdl_attr_define,
				SDL_ATTR_RSSI)))
	      {
	        switch (_CEAttrFlagType(pAttr->sdl_attr_define))
	          {
		    case ENUM_VALUE:
		        dstEnum  = (enum SdlOption *)
					(((char *)dst) + pAttr->field_ptr);
		        srcEnum  = (enum SdlOption *)
					(((char *)src) + pAttr->field_ptr);
		        *dstEnum = *srcEnum;
		        break;

		    case NUMBER_VALUE:
		        dstNum  = (SDLNumber *)
					(((char *)dst) + pAttr->field_ptr);
		        srcNum  = (SDLNumber *)
					(((char *)src) + pAttr->field_ptr);
		        *dstNum = *srcNum;
		        break;

		    case STRING1_VALUE:
		    case STRING2_VALUE:
		        dstStr  = (SDLCdata *)
					(((char *)dst) + pAttr->field_ptr);
		        srcStr  = (SDLCdata *)
					(((char *)src) + pAttr->field_ptr);
		        *dstStr = *srcStr;
		        break;

	          }
	      }
          }
      }
}

/******************************************************************************
 * Function:    void _DtHelpCeAddSegToList (CESegment *seg_ptr,
 *				CESegment **seg_list,
 *				CESegment **last_seg)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Allocates a memory
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeAddSegToList(seg_ptr, seg_list, last_seg)
    CESegment	*seg_ptr;
    CESegment **seg_list;
    CESegment **last_seg;
#else
_DtHelpCeAddSegToList(
    CESegment	*seg_ptr,
    CESegment **seg_list,
    CESegment **last_seg)
#endif /* _NO_PROTO */
{
    if (*seg_list == NULL)
        *seg_list = seg_ptr;
    else
        (*last_seg)->next_seg = seg_ptr;

    *last_seg = seg_ptr;
}

/******************************************************************************
 * Function:    int _DtHelpCeAllocElement (CEElement **el_ptr);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Allocates a memory
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAllocElement(el_value, old_attrs, new_el)
    enum SdlElement	  el_value;
    CESDLAttrStruct	 *old_attrs;
    CEElement		**new_el;
#else
_DtHelpCeAllocElement(
    enum SdlElement	  el_value,
    CESDLAttrStruct	 *old_attrs,
    CEElement		**new_el)
#endif /* _NO_PROTO */
{
    CEElement	*newEl;

    newEl = (CEElement *) malloc (sizeof(CEElement));
    if (newEl == NULL)
        return -1;

    newEl->el_type    = el_value;
    newEl->seg_list   = NULL;

    /*
     * this element's attributes are the same as the parent's
     */
    newEl->attributes         = *old_attrs;
    newEl->attributes.strings = DefaultStrings;

    /*
     * zero the value flags to indicate that these values were inheritted
     */
    newEl->attributes.enum_values = 0;
    newEl->attributes.num_values  = 0;
    newEl->attributes.str1_values = 0;
    newEl->attributes.str2_values = 0;

    /*
     * Default the formatting style.
     */
    newEl->attributes.frmt_specs  = DefaultFrmtSpecs;

    /*
     * except, inherit some of the formatting style values.
     */
    newEl->attributes.frmt_specs.justify = old_attrs->frmt_specs.justify;
    newEl->attributes.frmt_specs.vjust   = old_attrs->frmt_specs.vjust;

    *new_el = newEl;
    return 0;
}

/******************************************************************************
 * Function:    int _DtHelpCeAllocSegment (
 *					int  malloc_size,
 *					int *alloc_size,
 *					CESegment **next_seg,
 *					CESegment **seg_ptr);
 *
 * Parameters:
 *		malloc_size	Specifies the number of blocks to
 *				malloc. The first one is returned in
 *				'seg_ptr' with the rest in 'next_seg',
 *				if 'next_seg' is non-NULL.
 *		alloc_size	Ignored if NULL. If non-NULL, indicates
 *				the number of blocks in 'next_seg' if
 *				'next_seg' is non-NULL and the 'next_seg'
 *				pointer is non-NULL.
 *		next_seg	Specifies the block of memory to take the
 *				next segment from.
 *				Returns the pointer to the next segment.
 *		seg_ptr		Returns a pointer to the segment to use.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:     Allocates a memory
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeAllocSegment(malloc_size, alloc_size, next_seg, seg_ptr)
    int		  malloc_size;
    int		 *alloc_size;
    CESegment   **next_seg;
    CESegment   **seg_ptr;
#else
_DtHelpCeAllocSegment(
    int		  malloc_size,
    int		 *alloc_size,
    CESegment   **next_seg,
    CESegment   **seg_ptr)
#endif /* _NO_PROTO */
{
    int		 cnt    = 0;
    CESegment	*newSeg = NULL;

    if (next_seg != NULL)
	newSeg = *next_seg;

    if (alloc_size != NULL && newSeg != NULL)
	cnt = *alloc_size;

    if (cnt < 1)
      {
	CESegment  *pSeg;

	if (malloc_size < 1)
	    malloc_size = 1;

        pSeg = (CESegment *) malloc (sizeof(CESegment) * malloc_size);
        if (pSeg == NULL)
            return -1;

        cnt   = --malloc_size;
        *pSeg = DefaultSegment;

	newSeg = pSeg++;
        while (malloc_size--)
          {
	    *pSeg     = DefaultSegment;
	    pSeg->top = False;
	    pSeg++;
          }
      }
    else
	cnt--;

    *seg_ptr    = newSeg++;
    if (next_seg != NULL)
      {
	*next_seg = NULL;
	if (cnt > 0)
            *next_seg   = newSeg;
      }
    if (alloc_size != NULL)
        *alloc_size = cnt;

    return 0;
}

/******************************************************************************
 * Function:    int _DtHelpCeSaveString (
 *                              string, multi_len)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page beginning.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeSaveString(seg_list, last_seg, prev_seg, string,
						multi_len, flags, newline)
    CESegment	**seg_list;
    CESegment	**last_seg;
    CESegment	**prev_seg;
    char         *string;
    CanvasFontSpec *font_specs;
    int           link_index;
    int           multi_len;
    int           flags;
    CEBoolean     newline;
#else
_DtHelpCeSaveString(
    CESegment	**seg_list,
    CESegment	**last_seg,
    CESegment	**prev_seg,
    char         *string,
    CanvasFontSpec *font_specs,
    int           link_index,
    int           multi_len,
    int           flags,
    CEBoolean     newline)
#endif /* _NO_PROTO */
{

    int        len;
    int        size;
    int        junk    = 0;
    char       tmpChar;
    char      *ptr     = string;
    CEString  *strSeg;
    CESegment *pSeg;
    CESegment *junkSeg  = NULL;
    CEBoolean addToList = False;

    if (*ptr == '\0')
      {
    	if (newline == True)
	  {
	    if (_DtHelpCeAllocSegment(1, &junk, &junkSeg, &pSeg) != 0)
	        return -1;
	    addToList = True;
	  }
      }
    else
      {
	while (*ptr != '\0')
	  {
	    strSeg = _DtHelpCeAllocStrStruct();
	    if (strSeg == NULL ||
			_DtHelpCeAllocSegment(1, &junk, &junkSeg, &pSeg) != 0)
		return -1;

	    len = 1;
	    if (multi_len != 1)
	      {
		len  = mblen (ptr, MB_CUR_MAX);
		size = FindBreak(ptr, len);
	      }
	    else
		size = strlen(ptr);

	    pSeg->seg_type = _CESetCharLen(CESetTypeToString(flags), len);

	    if (link_index != -1)
	      {
		pSeg->seg_type = _CESetHyperFlag(pSeg->seg_type);
		pSeg->link_index = link_index;
	      }

	    tmpChar   = ptr[size];
	    ptr[size] = '\0';

	    pSeg->seg_handle.str_handle = strSeg;
	    strSeg->string              = strdup(string);
	    strSeg->font_specs          = font_specs;
	    if (*prev_seg != NULL)
		(*prev_seg)->next_disp = pSeg;

	    *prev_seg = pSeg;

	    _DtHelpCeAddSegToList(pSeg, seg_list, last_seg);

	    ptr[size] = tmpChar;
	    ptr      += size;
	    string    = ptr;
	  }
      }

    if (newline == True)
      {
	pSeg->seg_type = _CESetNewLineFlag(pSeg->seg_type);

	if (addToList == True)
	  {
	    if (*prev_seg != NULL)
	        (*prev_seg)->next_disp = pSeg;
	    *prev_seg = pSeg;
	    _DtHelpCeAddSegToList(pSeg, seg_list, last_seg);
	  }
      }

    return 0;
}

/*********************************************************************
 * Function: _DtHelpCeRunInterp
 *
 *    _DtHelpCeRunInterp calls a script and maybe gets data.
 *
 *********************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeRunInterp(filter_exec, client_data, interp_type, data, ret_data)
    VStatus		(*filter_exec)();
    ClientData		  client_data;
    enum SdlOption        interp_type;
    char                 *data;
    char                **ret_data;
#else
_DtHelpCeRunInterp(
    VStatus		(*filter_exec)(),
    ClientData		  client_data,
    enum SdlOption        interp_type,
    char                 *data,
    char                **ret_data)
#endif /* _NO_PROTO */
{
    int          result;
    int          myFd;
    FILE        *myFile;
    int          size;
    int          writeBufSize = 0;
    char        *writeBuf     = NULL;
    char        *ptr;
    char        *fileName;
    char        *execString;
    char        *newData;
    char         readBuf[BUFSIZ];
    BufFilePtr   myBufFile;
    static char  devNullString[] = " 2>/dev/null";

    /*
     * ask for permission to run the interperator command.
     */
    newData = data;
    if (filter_exec != NULL && (*filter_exec)(client_data,data,&newData) != 0)
	return -1;

    /*
     * Get the interperter string
     */
    execString = _DtHelpCeGetInterpCmd(interp_type);

    if (execString == NULL)
      {
	if (newData != data)
	    free(newData);
        return -1;
      }

    /*
     * open a temporary file to write the data to.
     */
    fileName = tempnam(NULL, NULL);
    if (fileName == NULL)
      {
	if (newData != data)
	    free(newData);
	return -1;
      }

    /*
     * write the data to file.
     */
    result = -1;
    myFd   = open(fileName, O_WRONLY | O_CREAT | O_TRUNC);
    if (myFd != -1)
      {
	/*
	 * write the data to the file
	 */
        result = write(myFd, newData, strlen(newData));

	if (result != -1)
	  {
	    /*
	     * change the access permissions so the interpreter can read
	     * the file.
	     */
	    result = fchmod(myFd, S_IRUSR | S_IRGRP | S_IROTH);
	  }

        /*
         * close the file.
         */
        close(myFd);
      }

    if (newData != data)
	free(newData);

    if (result == -1)
      {
	unlink(fileName);
	free(fileName);
	return -1;
      }

    /*
     * pre-append the man command to man specification
     */
    ptr = (char *) malloc(sizeof(execString) + strlen(fileName) + 1);
    if (!ptr)
      {
	unlink(fileName);
	free(fileName);
        return -1;
      }

    strcpy (ptr, execString);
    strcat (ptr, " ");
    strcat (ptr, fileName);

    myFile = popen(ptr, "r");

    /*
     * free the command
     */
    free (ptr);

    /*
     * check for problems
     */
    if (!myFile) /* couldn't create execString process */
      {
	unlink(fileName);
	free(fileName);
        return -1;
      }

    /*
     * make sure we don't try to read compressed.
     */
    myBufFile = _DtHelpCeCreatePipeBufFile(myFile);
    if (myBufFile == NULL)
      {
        (void) pclose(myFile); /* don't check for error, it was popen'd */
	unlink(fileName);
	free(fileName);
        return -1;
      }

    do {
        readBuf[0] = '\0';
        ptr        = readBuf;

        result = _DtHelpCeGetNxtBuf(myBufFile, readBuf, &ptr, BUFSIZ);

        if (result > 0)
          {
            size = strlen(readBuf);
            if (writeBuf == NULL)
                writeBuf = (char *) malloc (size + 1);
            else
                writeBuf = (char *) realloc (writeBuf, writeBufSize + size + 1);

            if (writeBuf != NULL)
              {
                writeBuf[writeBufSize] = '\0';
                strcat(writeBuf, readBuf);
                writeBufSize += size;
              }
            else
                result = -1;
          }
    } while (result != -1 && !feof(FileStream(myBufFile)));

    /*
     * close the pipe
     */
    _DtHelpCeBufFileClose (myBufFile, True);

    if (result == -1)
      {
	if (writeBuf != NULL)
	    free(writeBuf);

	writeBuf = NULL;
      }
    else
	result = 0;

    /*
     * unlink the temporary file and free the memory.
     */
    unlink(fileName);
    free(fileName);

    /*
     * return the data
     */
    *ret_data = writeBuf;

    return result;

}  /* End _DtHelpCeRunInterp */

/******************************************************************************
 * Function:    int _DtHelpCeAllocStrStruct (char *lang, char *char_set)
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
static const CEString defaultStrStruct = { NULL, NULL, NULL };

CEString *
#ifdef _NO_PROTO
_DtHelpCeAllocStrStruct ()
#else
_DtHelpCeAllocStrStruct (void)
#endif /* _NO_PROTO */
{
    CEString *newPtr = (CEString *) malloc (sizeof (CEString));

    if (newPtr != NULL)
	*newPtr = defaultStrStruct;

    return newPtr;
}

/******************************************************************************
 * Function:    CEElement *_DtHelpCeMatchSemanticStyle (CESDLSegment *toss,
 *						enum SDLClass clan,
 *						char *ssi);
 * Parameters:
 *
 * Returns:     Nothing.
 *
 *****************************************************************************/
CEElement *
#ifdef _NO_PROTO
_DtHelpCeMatchSemanticStyle (toss, clan, level, ssi)
    CESegment		*toss;
    enum SdlOption	 clan;
    int			 level;
    char		*ssi;
#else
_DtHelpCeMatchSemanticStyle (
    CESegment		*toss,
    enum SdlOption	 clan,
    int			 level,
    char		*ssi)
#endif /* _NO_PROTO */
{
    CESegment		*pSeg;
    CEElement		*pEl;
    const ClassStyleMatrix *pClassStyle;

    /*
     * initalize the starting style.
     */
    pSeg = toss;
    pClassStyle = ClassToStyle;

    do
      {
        /*
         * figure out which element style this class is associated with
         */
        while (pClassStyle->start != SdlOptionBad &&
		!(pClassStyle->start <= clan && clan <= pClassStyle->end))
	    pClassStyle++;

        if (pClassStyle->start != SdlOptionBad)
	  {
            /*
             * skip those not matching the style this class belongs to.
	     * But don't go past the ones after, because they might match
	     * too.
             */
            while (pSeg != NULL &&
		_CEElementOfSegment(pSeg)->el_type != pClassStyle->style &&
		_CEElementOfSegment(pSeg)->el_type != pClassStyle[1].style)
	        pSeg = pSeg->next_seg;

            /*
             * the class matches the style, look for a specific class, ssi and
             * level match.
             */
            while (pSeg != NULL &&
		_CEElementOfSegment(pSeg)->el_type == pClassStyle->style)
              {
	        pEl = _CEElementOfSegment(pSeg);
	        if (pEl->attributes.clan == clan &&
	            (pEl->attributes.lev.rlevel == -1 ||
				    pEl->attributes.lev.rlevel == level) &&
	            (_DtHelpCeSdlSsiString(pEl) == NULL ||
		       (ssi != NULL &&
		       _DtHelpCeStrCaseCmp(_DtHelpCeSdlSsiString(pEl),ssi)==0)))
	            return pEl;

	        /*
	         * skip to the next element
	         */
	        pSeg = pSeg->next_seg;
              }

	    /*
	     * went through those styles, try more
	     */
	    pClassStyle++;
          }
      } while (pClassStyle->start != SdlOptionBad);

    return NULL;
}
