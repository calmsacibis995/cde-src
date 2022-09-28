/* $XConsortium: CCDFUtil.c /main/cde1_maint/1 1995/07/17 17:21:07 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	CCDFUtil.c
 **
 **   Project:    Un*x Desktop Help
 **
 **  
 **   Description: Semi private format utility functions for
 **		   formatting CCDF
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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

extern	int	errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "CanvasError.h"
#include "bufioI.h"
#include "CESegmentI.h"
#include "CEUtilI.h"
#include "CCDFUtilI.h"
#include "StringFuncsI.h"
#include "FontAttrI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	int	GetCmdData();
#else
static	int	GetCmdData(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	char     **in_ptr,
	int	   cur_mb_len,
	int	   allowed,
	int        strip,
	int	  *ret_size,
	int	  *ret_max,
	char     **ret_string );
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
#define MIN_GROW	16
#define REALLOC_INCR 10
#define	CMD_NOT_ALLOWED		-1000

typedef struct {
	char *cmd;
	int   type;
	int   significant;
} FormatCmds;

static const FormatCmds  CcdfFormatCmds[] =
  {
	{ "abbrev"      , CE_ABBREV_CMD   , 2 },
	{ "angle"       , CE_FONT_CMD     , 2 },
	{ "characterset", CE_FONT_CMD     , 1 },
	{ "figure"      , CE_FIGURE_CMD   , 1 },
	{ "graphic"     , CE_GRAPHIC_CMD  , 1 },
	{ "id"          , CE_ID_CMD       , 1 },
	{ "label"       , CE_LABEL_CMD    , 2 },
	{ "link"        , CE_LINK_CMD     , 2 },
	{ "newline"     , CE_NEWLINE_CMD  , 1 },
	{ "paragraph"   , CE_PARAGRAPH_CMD, 1 },
	{ "size"        , CE_FONT_CMD     , 2 },
	{ "spacing"     , CE_FONT_CMD     , 2 },
	{ "title"       , CE_TITLE_CMD    , 2 },
	{ "topic"       , CE_TOPIC_CMD    , 2 },
	{ "type"        , CE_FONT_CMD     , 2 },
	{ "weight"      , CE_FONT_CMD     , 1 },
	{ "0x"          , CE_OCTAL_CMD    , 2 },
	/* always leave this one as the last entry */
	{ "/"           , CE_FORMAT_END   , 1 }
  };

/******************************************************************************
 *
 * Semi Public variables.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function: static int GetCmdData (FILE *in_file, char *in_buf, int in_size,
 *				char **in_ptr, int allowed, int strip,
 *				int *ret_size, int *ret_max, char **ret_string)
 *
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		allowed		Specifies the formatting commands allowed
 *				in the data.
 *		strip		Specifies the formatting commands to strip.
 *				from the data.
 *		ret_size	Specifies the current size of
 *					'ret_string'.
 *				Returns the new size of 'ret_string'.
 *		ret_max		Specifies the current maximum size of
 *					'ret_string'
 *				Returns the new size of 'ret_string'.
 *		ret_string	Returns the data found.
 *				
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * 
 * Purpose:	Gets a string containing font change tags
 *		Incoming source:
 *			"data with font change tags </>"
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
GetCmdData (in_file, in_buf, in_size, in_ptr, cur_mb_len, allowed,
		strip, ret_size, ret_max, ret_string)
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	char     **in_ptr;
	int	   cur_mb_len;
	int	   allowed;
	int        strip;
	int	  *ret_size;
	int	  *ret_max;
	char     **ret_string;
#else
GetCmdData(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	char     **in_ptr,
	int	   cur_mb_len,
	int	   allowed,
	int        strip,
	int	  *ret_size,
	int	  *ret_max,
	char     **ret_string )
#endif /* _NO_PROTO */
{
    int       size;
    int       fontType;
    int       charSize = 1;
    int       result = 0;
    char     *myBufPtr   = *in_ptr;
    char     *tmpPtr;
    CEBoolean   skipString = False;
    CEBoolean   endToken;

    if (!ret_string)
	skipString = True;

    while (!result)
      {
	if (cur_mb_len != 1)
	    charSize = mblen(myBufPtr, cur_mb_len);

	if (charSize == 1)
	  {
            /*
              * Do we need to read more information?
              */
            if (((int) strlen (myBufPtr)) < 3 &&
		_DtHelpCeGetNxtBuf(in_file,in_buf,&myBufPtr,in_size) == -1)
                 return -1;

	    switch (*myBufPtr)
	      {
		case '<':
			/*
			 * Go to the next character.
                         */
                        myBufPtr++;
			endToken = True;

			/*
			 * end token
			 */
			if (*myBufPtr == '/')
			  {
			    *in_ptr = myBufPtr;
			    return _DtHelpCeGetCcdfEndMark (in_file, in_buf,
						in_ptr, in_size,cur_mb_len);
			  }

			fontType = _DtHelpCeGetCcdfFontType (myBufPtr);
			if (fontType == -1)
			  {
			    switch (tolower (*myBufPtr))
			      {
				/*
				 * <0xff>
				 */
				case '0':
				    if ((!skipString &&
						_DtHelpCeAddOctalToBuf (
							myBufPtr,
							ret_string, ret_size,
							ret_max, 0) == -1)
						||
					_DtHelpCeGetCcdfEndMark (in_file, in_buf,
						&myBufPtr,in_size,cur_mb_len) == -1)
					return -1;

				    endToken = False;
				    break;

				/*
				 * <abbrev>
				 */
				case 'a':
				    myBufPtr++;
				    if (tolower (*myBufPtr) == 'b')
				      {
					if (CE_NOT_ALLOW_CMD (allowed,
							CE_ABBREV_CMD))
					  {
					    errno = CMD_NOT_ALLOWED;
					    return -1;
				          }
					else if (CE_NOT_ALLOW_CMD (strip,
							CE_ABBREV_CMD))
					    ;
				      }
				    else
				      {
					errno = CEErrorFormattingCmd;
					return -1;
				      }

				    break;
					

				/*
				 * <figure>
				 */
				case 'f':
				    if (CE_NOT_ALLOW_CMD (allowed,
							CE_FIGURE_CMD))
				      {
					errno = CMD_NOT_ALLOWED;
					return -1;
				      }
				    else if (CE_NOT_ALLOW_CMD (strip,
							CE_FIGURE_CMD))
				        ;

				    break;
					
				/*
				 * <graphic>
				 */
				case 'g':
				    if (CE_NOT_ALLOW_CMD (allowed,
							CE_GRAPHIC_CMD))
				      {
					errno = CMD_NOT_ALLOWED;
					return -1;
				      }

				    if (_DtHelpCeGetCcdfEndMark (in_file, in_buf,
						&myBufPtr,in_size,cur_mb_len) == -1)
					return -1;

				    endToken = False;
				    break;

				/*
				 * <id>
				 */
				case 'i':
				    if (CE_NOT_ALLOW_CMD (allowed,
							CE_ID_CMD))
				      {
					errno = CMD_NOT_ALLOWED;
					return -1;
				      }
				    else if (CE_NOT_ALLOW_CMD (strip,
							CE_ID_CMD))
				        ;
				    break;

				/*
				 * <label>
				 * <link>
				 */
				case 'l':
				    myBufPtr++;
				    if (tolower (*myBufPtr) == 'a')
				      {
					if (CE_NOT_ALLOW_CMD (allowed,
							CE_LABEL_CMD))
				          {
					    errno = CMD_NOT_ALLOWED;
					    return -1;
				          }
					else if (CE_NOT_ALLOW_CMD (strip,
							CE_LABEL_CMD))
					    ;
				      }
				    else if (tolower (*myBufPtr) == 'i')
				      {
					if (CE_NOT_ALLOW_CMD (allowed,
							CE_LINK_CMD))
				          {
					    errno = CMD_NOT_ALLOWED;
					    return -1;
				          }
					else if (CE_NOT_ALLOW_CMD (strip,
							CE_LINK_CMD))
					    ;
				      }
				    else
				      {
					errno = CEErrorFormattingCmd;
					return -1;
				      }

				    break;

				/*
				 * <newline>
				 */
				case 'n':
				    if (CE_NOT_ALLOW_CMD (allowed,
							CE_NEWLINE_CMD))
				      {
					errno = CMD_NOT_ALLOWED;
					return -1;
				      }

				    if (_DtHelpCeGetCcdfEndMark (in_file, in_buf,
						&myBufPtr,in_size,cur_mb_len) == -1)
					return -1;

				    endToken = False;
				    break;

				/*
				 * <paragraph>
				 */
				case 'p':
				    if (CE_NOT_ALLOW_CMD (allowed,
							CE_PARAGRAPH_CMD))
				      {
					errno = CMD_NOT_ALLOWED;
					return -1;
				      }
				    else if (CE_NOT_ALLOW_CMD (strip,
							CE_PARAGRAPH_CMD))
				        ;
				    break;

				/*
				 * <title>
				 * <topic>
				 */
				case 't':
				    myBufPtr++;
				    if (tolower (*myBufPtr) == 'o')
				      {
					if (CE_NOT_ALLOW_CMD (allowed,
							CE_TOPIC_CMD))
				          {
					    errno = CMD_NOT_ALLOWED;
					    return -1;
				          }
				        else if (CE_NOT_ALLOW_CMD (strip,
							CE_TOPIC_CMD))
				            ;
				      }
				    else if (tolower (*myBufPtr) == 'i')
				      {
					if (CE_NOT_ALLOW_CMD (allowed,
							CE_TITLE_CMD))
				          {
					    errno = CMD_NOT_ALLOWED;
					    return -1;
				          }
				        else if (CE_NOT_ALLOW_CMD (strip,
							CE_TITLE_CMD))
				            ;
				      }
				    else
				      {
					errno = CEErrorFormattingCmd;
					return -1;
				      }

				    break;

				default:
				    errno = CEErrorFormattingCmd;
				    return -1;
			      }
			  }
			else if (CE_NOT_ALLOW_CMD (allowed, CE_FONT_CMD))
			  {
			    errno = CMD_NOT_ALLOWED;
			    return -1;
			  }
			else if (CE_NOT_ALLOW_CMD (strip, CE_FONT_CMD))
			    ;

			if (endToken)
			  {
			    /*
			     * This is a <token>....</token> construct.
			     * pass over the <token> part.
			     */
			    if (_DtHelpCeGetCcdfEndMark (in_file, in_buf,
					&myBufPtr, in_size,cur_mb_len) == -1 ||
			    	GetCmdData (in_file, in_buf, in_size,
					&myBufPtr, cur_mb_len, allowed, strip,
					ret_size, ret_max, ret_string) == -1)
				return -1;
			  }
			break;

		case ' ':
		case '\n':
			/*
			 * Put a space in the segment
			 */
			if (!skipString)
			  {
			    tmpPtr = " ";
			    result = _DtHelpCeAddCharToBuf (&tmpPtr,
				ret_string, ret_size, ret_max, MIN_GROW);
			  }
			myBufPtr++;
			break;

		case '\\':
			myBufPtr++;

			/*
			 * Save the escaped character
			 */
			if (!skipString)
			    result = _DtHelpCeAddCharToBuf (&myBufPtr,
				ret_string, ret_size, ret_max, MIN_GROW);
			else
			    myBufPtr++;
			break;

		default:
			/*
			 * Put the information in the buffer
			 */
			result = _DtHelpCeStrcspn (myBufPtr, "<\\\n ",
							cur_mb_len, &size);
			/*
			 * If _DtHelpCeStrcspn found an invalid character
			 * we don't want to quit yet. Allow another pass
			 * to try to read another buffer of information.
			 */
			result = 0;

			if (!skipString && size)
			    result = _DtHelpCeAddStrToBuf (&myBufPtr,
				ret_string, ret_size, ret_max, size, 0);
			else
			    myBufPtr += size;
	      }
	  }
	else if (charSize > 1)
	  {
	    result = _DtHelpCeStrcspn (myBufPtr, "<\\\n ", cur_mb_len, &size);

	    /*
	     * If _DtHelpCeStrcspn found an invalid character
	     * we don't want to quit yet. Allow another pass
	     * to try to read another buffer of information.
	     */
	    result = 0;

	    if (!skipString && size)
	        result = _DtHelpCeAddStrToBuf (&myBufPtr,
				ret_string, ret_size, ret_max, size, 0);
	    else
		myBufPtr += size;
	  }
	else /* if (charSize < 1) */
	  {
	    if (*myBufPtr == '\0' || ((int) strlen (in_buf)) < cur_mb_len)
	        result = _DtHelpCeGetNxtBuf(in_file,in_buf,&myBufPtr,in_size);
	    else
	      {
		errno = CEErrorIllegalInfo;
		result = -1;
	      }
	  }
      }
    return result;
}

/******************************************************************************
 * Function: int GetTitleCmd (FILE *in_file, char in_buf, int in_size,
 *				char **in_ptr, char **ret_string)
 * 
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		ret_string	Returns the data found.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Determine if the next formatting command is
 *			<ABBREV ...>.
 *		Returns the data found between <ABBREV> and its ending </>.
 *
 * Note:	The only formatting command allowed between <ABBREV> and
 *		its </> is the <NEWLINE> command. And it is stripped.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
GetTitleCmd (in_file, in_buf, in_size, cur_mb_len, in_ptr, ret_string)
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	int        cur_mb_len;
	char     **in_ptr;
	char     **ret_string;
#else
GetTitleCmd(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	int        cur_mb_len,
	char     **in_ptr,
	char     **ret_string )
#endif /* _NO_PROTO */
{
    int       result;
    int       junkSize  = 0;
    int       junkMax  = 0;

    /*
     * null the return string
     */
    if (ret_string)
	*ret_string = NULL;

    /*
     * check for the token
     */
    result = _DtHelpCeCheckNextCcdfCmd ("ti", in_file, in_buf, in_size, cur_mb_len, in_ptr);
    if (result != 0)
      {
	if (result == -2)
	    errno = CEErrorMissingTitleCmd;
	return -1;
      }

    if (_DtHelpCeGetCcdfEndMark(in_file,in_buf,in_ptr,in_size,cur_mb_len) != 0)
	return -1;

    result = GetCmdData (in_file, in_buf, in_size, in_ptr, cur_mb_len,
			(CE_NEWLINE_CMD | CE_GRAPHIC_CMD |
				CE_LINK_CMD | CE_FONT_CMD | CE_ID_CMD),
			(CE_NEWLINE_CMD | CE_GRAPHIC_CMD |
				CE_LINK_CMD | CE_FONT_CMD | CE_ID_CMD),
			&junkSize, &junkMax, ret_string);

    if (result == -1 && errno == CMD_NOT_ALLOWED)
	errno = CEErrorTitleSyntax;

    return result;
}

/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	int _DtHelpCeGetCcdfEndMark (FILE *file, char *buffer,
 *				char **buf_ptr, int buf_size, CEBoolean flag)
 *
 * Parameters:
 *		file		Specifies a stream to read from.
 *		buffer 		Specifies the buffer where new information
 *					is placed.
 *		buf_ptr		Specifies the pointer into 'buffer' to
 *					start processing.
 *				Returns the new location in 'buffer' to
 *					continue processing.
 *		buf_size	Specifies the maximum size of 'buffer'.
 *
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Find the end of tag marker '>'.
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeGetCcdfEndMark (file, buffer, buf_ptr, buf_size, cur_mb_len)
     BufFilePtr	  file;
     char	 *buffer;
     char	**buf_ptr;
     int	  buf_size;
     int	  cur_mb_len;
#else
_DtHelpCeGetCcdfEndMark (
     BufFilePtr	  file,
     char	 *buffer,
     char	**buf_ptr,
     int	  buf_size,
     int	  cur_mb_len)
#endif /* _NO_PROTO */
{
    int      len;
    int      result;
    char    *ptr;
    CEBoolean  done = False;

    ptr = *buf_ptr;
    do
      {
	/*
	 * Find the end of marker or end of topic
	 */
	result = _DtHelpCeStrcspn (ptr, "\\>", cur_mb_len, &len);
	ptr += len;

	if (result == 0)
	  {
	    /*
	     * found either a backslash or the end marker,
	     * update the pointer and if it was the '>'
	     * say we're done.
	     */
	    if (*ptr == '>')
	        done = True;
	    else
		ptr++;
	    ptr++;
	  }
	else /* result == -1 || result == 1 */
	  {
	    /*
	     * nothing here - get the next buffer and keep looking
	     * unless getting the next buffer has problems/eof
	     */
	    if (((int) strlen(ptr)) >=  cur_mb_len)
	      {
		errno = CEErrorIllegalInfo;
		return -1;
	      }

	    if (_DtHelpCeGetNxtBuf (file, buffer, &ptr, buf_size) == -1)
		return -1;
	  }
      } while (!done);

    *buf_ptr = ptr;
    return 0;
}

/******************************************************************************
 * Function: int _DtHelpCeGetCcdfStrParam (FILE *in_file, char *in_buf,
 *				int in_size, char **in_ptr, CEBoolean flag,
 *				CEBoolean eat_escape, Boolean ignore_quotes,
 *				CEBoolean less_test, char **ret_string)
 * 
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		flag		Specifies whether the routine returns
 *					a -1 if '>' is the next token.
 *		eat_secape	Specifies whether the backslash is not
 *					placed in the returned string.
 *					True - it is skipped.
 *					False - it is saved in 'ret_string'.
 *		ignore_quotes	Specifies whether quotes are to be included
 *					in the returned string.
 *		less_test	Specifies whether the routine should
 *					stop when it finds a '<' character.
 *		ret_string	If NULL, throws the information away.
 *				Otherwise, returns the string found.
 *
 * Returns:	0 if successful, -1 if errors, 1 if 'flag' is false and
 *		the next token is '>'.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Skip the rest of the current token.
 *		Get the next token. A token is defined as being one or
 *			more strings between single or double quotes,
 *			or a string bounded by spaces or ended by the
 *			end token '>'.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfStrParam (in_file, in_buf, in_size, in_ptr, flag,
	eat_escape, ignore_quotes, less_test, ret_string)
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	int	   cur_mb_len;
	char     **in_ptr;
	CEBoolean    flag;
	CEBoolean    eat_escape;
	CEBoolean    ignore_quotes;
	CEBoolean    less_test;
	char     **ret_string;
#else
_DtHelpCeGetCcdfStrParam(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	int	   cur_mb_len,
	char     **in_ptr,
	CEBoolean    flag,
	CEBoolean    eat_escape,
	CEBoolean    ignore_quotes,
	CEBoolean    less_test,
	char     **ret_string )
#endif /* _NO_PROTO */
{
    int		 copySize;
    int		 result = 0;
    int		 spnResult;
    int		 stringSize = 0;
    int		 stringMax  = 0;
    char	*stringPtr  = NULL;
    char	*myBufPtr;
    char	*mySpace;
    CEBoolean      done = False;
    CEBoolean      singleQuotes = False;
    CEBoolean      doubleQuotes = False;
    CEBoolean      skipString   = False;
    CEBoolean      testMB       = False;
    
    if (cur_mb_len != 1)
        testMB = True;

    /*
     * do we want to skip the string or read it.
     */
    if (!ret_string)
	skipString = True;

    /*
     * skip to the next parameter or > character. If flag is true,
     * we want another parameter, so error if the > character is found.
     */
    if (_DtHelpCeSkipToNextCcdfToken (in_file, in_buf, in_size, cur_mb_len, in_ptr, flag) != 0)
	return -1; 

    myBufPtr = *in_ptr;

    /*
     * see if there is another parameter
     */
    if (!flag && *myBufPtr == '>')
	return 1;

    /*
     * Initialize the global buffer.
     */
    if (!ignore_quotes && (!testMB || mblen (myBufPtr, cur_mb_len) == 1))
      {
	if (*myBufPtr == '\"')
	  {
	    doubleQuotes = True;
	    myBufPtr++;
	  }
	else if (*myBufPtr == '\'')
	  {
	    singleQuotes = True;
	    myBufPtr++;
	  }
      }

    while (!done)
      {
        spnResult = _DtHelpCeStrcspn (myBufPtr, " <>\'\"\\\n",
						cur_mb_len, &copySize);
	/*
	 * Either skip the string or place it in storage
	 */
	if (skipString)
	    myBufPtr += copySize;
	else if (_DtHelpCeAddStrToBuf (&myBufPtr, &stringPtr, &stringSize,
					&stringMax, copySize, 0) == -1)
	    return -1;

	/*
	 * If spnResult is non-zero, read the next buffer of information
	 */
	if (spnResult)
	  {
	    if (spnResult == -1 && ((int)strlen(myBufPtr)) >= cur_mb_len)
	      {
		errno = CEErrorIllegalInfo;
		return -1;
	      }

	    if (_DtHelpCeGetNxtBuf(in_file,in_buf,&myBufPtr,in_size) == -1)
	        return -1;
	  }

	/*
	 * Otherwise _DtHelpCeStrcspn stopped at a character we want
	 */
	else
	  {
	    switch (*myBufPtr)
	      {
	        case '\\':
		    if (eat_escape || skipString)
		        myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;

		    if (*myBufPtr == '\0' && _DtHelpCeGetNxtBuf(in_file,
					in_buf,&myBufPtr,in_size) == -1)
		        return -1;

		    if (skipString)
			myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;
		    break;

	        case '\n':
		    if (!doubleQuotes && !singleQuotes)
		        done = True;
		    else if (skipString)
			myBufPtr++;
		    else
		      {
			mySpace = " ";
			if (_DtHelpCeAddCharToBuf (&mySpace, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			    return -1;
		      }
		    break;

	        case '<':
		    if (less_test)
			done = True;
		    else if (skipString)
			myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;
		    break;

	        case '>':
	        case ' ':
		    if (!doubleQuotes && !singleQuotes)
		        done = True;
		    else if (skipString)
			myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;
		    break;

	        case '\'':
		    if (!ignore_quotes && singleQuotes)
		        done = True;
		    else if (skipString)
			myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;
		    break;

	        case '\"':
		    if (!ignore_quotes && doubleQuotes)
		        done = True;
		    else if (skipString)
			myBufPtr++;
		    else if (_DtHelpCeAddCharToBuf (&myBufPtr, &stringPtr,
				&stringSize, &stringMax, MIN_GROW) == -1)
			return -1;
		    break;

	      }
	  }
      }

    if (skipString)
      {
	if (stringPtr)
	    free (stringPtr);
      }
    else
        *ret_string = stringPtr;

    *in_ptr = myBufPtr;
    return result;

} /* End _DtHelpCeGetCcdfStrParam */

/******************************************************************************
 * Function: int _DtHelpCeGetCcdfValueParam (FILE *in_file, char *in_buf,
 *					int in_size, char **in_ptr,
 *					CEBoolean flag, int *ret_value)
 * 
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		flag		Specifies whether the routine returns
 *					a -2 if '>' is the next token.
 *		ret_value	Returns the atoi conversion
 *				of the string found.
 *
 * Returns:	0 if successful, -1 if errors, -2 if 'flag' is True and
 *		the next token is '>'.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorFormattingValue
 *		CEErrorReadEmpty
 *
 * Purpose:	Get then next numeric parameter.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfValueParam(in_file, in_buf, in_size, in_ptr, flag, cur_mb_len, ret_value)
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	char     **in_ptr;
	CEBoolean    flag;
	int	   cur_mb_len;
	int	  *ret_value;
#else
_DtHelpCeGetCcdfValueParam(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	char     **in_ptr,
	CEBoolean    flag,
	int	   cur_mb_len,
	int	  *ret_value )
#endif /* _NO_PROTO */
{
    CEBoolean    done = False;
    char *myBufPtr;
    char *stringPtr = NULL;
    int stringSize = 0;
    int stringMax  = 0;
    int result = 0;
    int spnResult = 0;
    int	copySize;

    if (_DtHelpCeSkipToNextCcdfToken (in_file, in_buf, in_size, cur_mb_len, in_ptr, True) != 0)
	return -1; 

    myBufPtr = *in_ptr;

    while (!done)
      {
	spnResult = _DtHelpCeStrcspn (myBufPtr, " >\n", cur_mb_len, &copySize);
	if (_DtHelpCeAddStrToBuf (&myBufPtr, &stringPtr, &stringSize,
					&stringMax, copySize, 0) == -1)
	    return -1;

	/*
	 * _DtHelpCeStrcspn stopped at a character we want
	 */
	if (spnResult == 0)
	    done = True;
	else if (((int)strlen(myBufPtr)) >= cur_mb_len)
	  {
	    errno = CEErrorIllegalInfo;
	    return -1;
	  }
	else if (_DtHelpCeGetNxtBuf(in_file,in_buf,&myBufPtr,in_size) == -1)
	    return -1;
      }

    if (stringPtr != NULL && *stringPtr != '\0')
      {
        *ret_value = atoi(stringPtr);
	free (stringPtr);
      }
    else
      {
	errno = CEErrorFormattingValue;
	result = -1;
      }

    *in_ptr = myBufPtr;

    if (result != -1 && flag && *myBufPtr == '>')
	return -2;

    return result;
}

/******************************************************************************
 * Function: int _DtHelpCeSkipToNextCcdfToken (FILE *read_file, char *read_buf,
 *				int read_size, char **src_ptr, CEBoolean flag)
 * 
 * Parameters:
 *		read_file	Specifies a stream to read from.
 *		read_buf	Specifies the buffer where new information
 *					is placed.
 *		read_size	Specifies the maximum size of 'read_buf'.
 *		src_ptr		Specifies the pointer into 'read_buf' to
 *					start processing.
 *				Returns the new location in 'read_buf' to
 *					continue processing.
 *		flag		Specifies whether the routine returns
 *					a 1 if '>' is the next token.
 * Returns:
 *		-1	If problems encountered finding the next token.
 *		 0	If no problems encountered finding the next token.
 *		 1	If flag is true and the next token is a > character.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorReadEmpty
 *		CEErrorIllegalInfo
 *
 * Purpose:	Move 'src_ptr' to the next occurrance of a > character
 *		or the first character after the next space.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeSkipToNextCcdfToken (read_file, read_buf, read_size, cur_mb_len, src_ptr, flag )
	BufFilePtr read_file;
	char	 *read_buf;
	int	  read_size;
	int	  cur_mb_len;
	char	**src_ptr;
	CEBoolean   flag;
#else
_DtHelpCeSkipToNextCcdfToken (
	BufFilePtr read_file,
	char	 *read_buf,
	int	  read_size,
	int	  cur_mb_len,
	char	**src_ptr,
	CEBoolean   flag )
#endif /* _NO_PROTO */
{
    int       len;
    int       result;
    char     *srcPtr = *src_ptr;

    /*
     * Skip the rest of the alphanumeric string.
     */
    do
      {
	/*
	 * _DtHelpCeStrcspn returns 0 for 'found a character'
	 */
	result = _DtHelpCeStrcspn (srcPtr, " >\n", cur_mb_len, &len);
	srcPtr = srcPtr + len;

	if (result)
	  {
	    if (result == -1 && ((int)strlen(srcPtr)) >= cur_mb_len)
	      {
		errno = CEErrorIllegalInfo;
		return -1;
	      }
	    if (_DtHelpCeGetNxtBuf(read_file, read_buf,&srcPtr,read_size) == -1)
	        return -1;
	  }
      } while (result);

    /*
     * Now skip the blanks and end of lines
     */
    do
      {
	/*
	 * _DtHelpCeStrspn returns 0 for 'found a character'
	 */
	result = _DtHelpCeStrspn (srcPtr, " \n", cur_mb_len, &len);
	srcPtr = srcPtr + len;
	if (result)
	  {
	    if (result == -1 && ((int)strlen(srcPtr)) >= cur_mb_len)
	      {
		errno = CEErrorIllegalInfo;
		return -1;
	      }
	    if (_DtHelpCeGetNxtBuf(read_file,read_buf,&srcPtr,read_size) == -1)
	        return -1;
	  }
      } while (result);

    *src_ptr = srcPtr;

    if (flag && *srcPtr == '>')
      {
	errno = CEErrorFormattingOption;
	return 1;
      }

    return 0;
}

/******************************************************************************
 * Function: int _DtHelpCeGetCcdfParaOptions (
 *			FILE *file         , char  *buffer      ,
 *			int   buf_size     , char **buf_ptr     ,
 *			int  *seg_type     , int   *graphic_type,
 *			int   *first_indent, int   *left_indent ,
 *			int   *right_indent, int   *lines_before,
 *			int   *lines_after , int   *spacing     ,
 *			char **ret_label   , char **ret_name    ,
 *			char **ret_link    , int   *ret_link_type,
 *			char **ret_descript, char **ret_idString )
 * 
 * Parameters:	file		Specifies the stream of the open file.
 *		buffer		Specifies the buffer to read information into.
 *		buf_size	Specifies the maximum size of 'buffer'.
 *		buf_ptr		Specifies the pointer into buffer to use.
 *				Returns the next location in 'buffer' to
 *					process.
 *		seg_type	Specifies the current segment type.
 *				Returns the new segment type.
 *		graphic_type	Specifies the current graphic type.
 *				Returns the new graphic type.
 *		first_indent	Specifies the current first indentation.
 *				Returns the new first indentation.
 *		left_indent	Specifies the current left indentation.
 *				Returns the new left indentation.
 *		right_indent	Specifies the current right indentation.
 *				Returns the new right indentation.
 *		lines_before	Specifies the current number of lines to
 *					place before the paragraph.
 *				Returns the new number of lines to place
 *					before the paragraph.
 *		lines_after	Specifies the current number of lines to
 *					place after the paragraph.
 *				Returns the new number of lines to place
 *					after the paragraph.
 *		spacing		Specifies the current number of spaces to
 *					place between a graphic and its
 *					text.
 *				Returns the new number of spaces to place
 *					between a graphic and its text.
 *		ret_label	Returns the label for this paragraph
 *					if non-null.
 *		ret_name	Returns the name for the justified graphic
 *					found in this paragraph if non-null.
 *		ret_link	Returns the hypertext link specification
 *					associated with the graphic in
 *					this paragraph if non-null.
 *				Note: The caller needs to check to see if
 *					graphic information returned is valid.
 *		ret_link_type	Returns the hypertext link type associated
 *					with the graphic if specified.
 *				Note: The caller needs to check to see if
 *					graphic information returned is valid.
 *		ret_descript	Returns the description found for this
 *					paragraph if non-null.
 *		ret_idString	Returns the location id string found in this
 *					paragraph if non-null.
 *
 * Memory owed by caller:
 *		ret_label
 *		ret_name
 *		ret_link
 *		ret_descript
 *		ret_idString
 *
 * Returns:	0 if successful, -1 for errors.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorFormattingValue
 *		CEErrorReadEmpty
 *		DtParagraphValue
 *		DtParagraphOption
 *
 * Purpose:	Parse the specification found in a <PARAGRAPH> token.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfParaOptions (file, buffer, buf_size, cur_mb_len, buf_ptr,
	seg_type, graphic_type,
	first_indent, left_indent, right_indent,
	lines_before, lines_after, spacing,
	ret_label, ret_name, ret_link, ret_link_type,
	ret_descript, ret_idString)
    BufFilePtr	  file;
    char         *buffer;
    int           buf_size;
    int           cur_mb_len;
    char        **buf_ptr;
    int		 *seg_type;
    int		 *graphic_type;
    int		 *first_indent;
    int		 *left_indent;
    int		 *right_indent;
    int		 *lines_before;
    int		 *lines_after;
    int		 *spacing;
    char	**ret_label;
    char	**ret_name;
    char	**ret_link;
    int		 *ret_link_type;
    char	**ret_descript;
    char	**ret_idString;
#else
_DtHelpCeGetCcdfParaOptions(
    BufFilePtr	  file,
    char         *buffer,
    int           buf_size,
    int           cur_mb_len,
    char        **buf_ptr,
    int		 *seg_type,
    int		 *graphic_type,
    int		 *first_indent,
    int		 *left_indent,
    int		 *right_indent,
    int		 *lines_before,
    int		 *lines_after,
    int		 *spacing,
    char	**ret_label,
    char	**ret_name,
    char	**ret_link,
    int		 *ret_link_type,
    char	**ret_descript,
    char	**ret_idString ) 
#endif /* _NO_PROTO */
{
    int		 result = 0;
    int		 optionCount = 0;
    int		 value;
    char	*myBufPtr  = *buf_ptr;
    char	*tmpString = NULL;

    /*
     * initialize string variables if valid addresses
     */
    if (ret_label)
        *ret_label = NULL;
    if (ret_name)
        *ret_name = NULL;
    if (ret_link)
        *ret_link = NULL;
    if (ret_descript)
        *ret_descript = NULL;

    while (result != -1)
      {
	if (_DtHelpCeSkipToNextCcdfToken (file, buffer, buf_size, cur_mb_len, &myBufPtr, False)
				     ==
				     -1)
	    return -1;

	switch (tolower(*myBufPtr))
	  {
	    /*
	     * end of paragraph spec
	     */
	    case '>':
		myBufPtr++;
		*buf_ptr = myBufPtr;
		return optionCount;

	    /*
	     * after value
	     */
	    case 'a':
		result = -1;
		if (_DtHelpCeGetCcdfValueParam (file, buffer, buf_size,
				&myBufPtr, False, cur_mb_len, &value) != -1)
		  {
		    result = 0;
		    *lines_after = 0;
		    if (value > 0)
		        *lines_after = value;
		  }
		break;

	    /*
	     * before value
	     */
	    case 'b':
		result = -1;
		if (_DtHelpCeGetCcdfValueParam (file, buffer, buf_size,
					&myBufPtr, False, cur_mb_len, &value) != -1)
		  {
		    result = 0;
		    *lines_before = 0;
		    if (value > 0)
			*lines_before = value;
		  }
		break;

	    /*
	     * description [string | "string" | 'string']
	     */
	    case 'd':
		result = _DtHelpCeGetCcdfStrParam (file, buffer,
				buf_size, cur_mb_len, &myBufPtr, True, True, False,
							False, ret_descript);
		break;

	    /*
	     * firstindent value
	     */
	    case 'f':
		if (_DtHelpCeGetCcdfValueParam (
			file, buffer, buf_size, &myBufPtr, False, cur_mb_len, &value) == -1)
		    return -1;

		*first_indent = *first_indent + value;
		if (*first_indent < 0)
		    *first_indent = 0;
		break;

	    /*
	     * glink      [string | "string" | 'string']
	     * gpos       value_string
	     * graphic    string
	     * gspace     value
	     * gtypelink  value
	     */
	    case 'g':
		/*
		 * to determine what token this is we must look at the
		 * second character.
		 */
		myBufPtr++;

		/*
		 * do we need to read more?
		 */
		if (*myBufPtr == '\0' &&
		    _DtHelpCeGetNxtBuf(file, buffer, &myBufPtr, buf_size) == -1)
		    return -1;

		/*
		 * check for the next permutation
		 */
		switch (tolower(*myBufPtr))
		  {
		    /*
		     * glink [string | "string" | 'string']
		     */
		    case 'l':
			result = _DtHelpCeGetCcdfStrParam (
						file, buffer, buf_size,
						cur_mb_len,
						&myBufPtr, True, True,
						False, False, ret_link);
			*graphic_type = _CESetHyperFlag(*graphic_type);
			break;

		    /*
		     * gpos value_string
		     */
		    case 'p':
			if (_DtHelpCeGetCcdfStrParam (
				file, buffer, buf_size, cur_mb_len, &myBufPtr,
				True, True, False, False, &tmpString) == -1)
			    return -1;

			if (tolower (*tmpString) == 'r')
			    *graphic_type = _CESetGraphicType(
							*graphic_type,
							CE_RIGHT_JUSTIFIED);

			if (tmpString)
			    free (tmpString);
			break;

		    /*
		     * graphic    string
		     */
		    case 'r':
			result = _DtHelpCeGetCcdfStrParam (
				file, buffer, buf_size,
					cur_mb_len, &myBufPtr, True, True,
						False, False, ret_name);
			break;

		    /*
		     * gspace     value
		     */
		    case 's':
			if (_DtHelpCeGetCcdfValueParam(file, buffer, buf_size,
				&myBufPtr, False, cur_mb_len, &value) == -1 || value < 0)
			    return -1;

			*spacing = value;
			break;

		    /*
		     * gtypelink  value
		     */
		    case 't':
			if (_DtHelpCeGetCcdfValueParam(file, buffer, buf_size,
				&myBufPtr, False, cur_mb_len, ret_link_type) == -1 ||
				*ret_link_type < 0)
			    return -1;

			break;
		  }
		break;

	    /*
	     * id string
	     */
	    case 'i':
		/*
		 * get the id string
		 */
		if (_DtHelpCeGetCcdfStrParam (
			file, buffer, buf_size, cur_mb_len, &myBufPtr,
				True, True, False, False, ret_idString) == -1)
		    return -1;
		break;

	    /*
	     * label [string | "string" | 'string']
	     * leftindent value
	     */
	    case 'l':
		/*
		 * to determine what token this is we must look at the
		 * second and possibly the third characters.
		 */
		myBufPtr++;

		/*
		 * do we need to read more?
		 */
		if (*myBufPtr == '\0' &&
		    _DtHelpCeGetNxtBuf(file, buffer, &myBufPtr, buf_size) == -1)
		    return -1;

		/*
		 * check for the next permutation
		 */
		if (*myBufPtr == 'a')
		  {
		    /*
		     * label
		     */
		    if (_DtHelpCeGetCcdfStrParam (
			file, buffer, buf_size, cur_mb_len, &myBufPtr,
				True, False, False, False, &tmpString) == -1)
			return -1;

		    /*
		     * If we got a label process it.
		     */
		    if (ret_label == NULL || *ret_label)
		      {
		        /*
		         * we've already processed a label!
		         * ignore this one!
		         */
			if (tmpString)
		            free (tmpString);
		      }
		    else
			*ret_label = tmpString;
		  }
		else if (*myBufPtr == 'e')
		  {
		    /*
		     * leftindent
		     */
		    if (_DtHelpCeGetCcdfValueParam (
			file, buffer, buf_size, &myBufPtr, False, cur_mb_len, &value) == -1)
			return -1;

		    *left_indent = *left_indent + value;
		    if (*left_indent < 0)
		        *left_indent = 0;
		  }
		break;

	    /*
	     * nowrap
	     */
	    case 'n':
		*seg_type = _CESetStaticFlag (*seg_type);
		break;

	    /*
	     * rightindent value
	     */
	    case 'r':
		if (_DtHelpCeGetCcdfValueParam (
			file, buffer, buf_size, &myBufPtr, False, cur_mb_len, &value) == -1)
		    return -1;

		*right_indent = *right_indent + value;
		if (*right_indent < 0)
		    *right_indent = 0;
		break;

	    /*
	     * wrap
	     */
	    case 'w':
		*seg_type = _CEClearStaticFlag (*seg_type);
		break;

	    /*
	     * Found an option we don't understand.
	     */
	    default:
		errno = CEErrorParagraphOption;
		return -1;
	  }
	optionCount++;
      }
    return result;

} /* End __CEFormatGetParagraphOptions */

/******************************************************************************
 * Function: int _DtHelpCeGetCcdfTopicCmd (void *dpy, FILE *in_file, char *in_buf,
 *				char **in_ptr, int in_size,
 *				char **ret_charSet)
 *
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		ret_charSet	Returns the characters set if specified.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * 
 * Purpose:	Checks for the <TOPIC> command and returns the charset
 *		if specified.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfTopicCmd (dpy, in_file, in_buf, in_ptr, in_size, cur_mb_len, ret_charSet)
    void     *dpy;
    BufFilePtr in_file;
    char     *in_buf;
    char    **in_ptr;
    int       in_size;
    int       cur_mb_len;
    char    **ret_charSet;
#else
_DtHelpCeGetCcdfTopicCmd (
    void     *dpy,
    BufFilePtr in_file,
    char     *in_buf,
    char    **in_ptr,
    int       in_size,
    int       cur_mb_len,
    char    **ret_charSet )
#endif /* _NO_PROTO */
{
    char    *mycharSet = NULL;

    /*
     * null the return values
     */
    if (ret_charSet)
	*ret_charSet = NULL;

    /*
     * check for <TOPIC charset string>
     */
    if (_DtHelpCeCheckNextCcdfCmd ("to",in_file,in_buf,in_size,cur_mb_len,in_ptr) == 0)
      {
	/*
	 * The <TOPIC> command and it's attributes must be in 1-byte charset.
	 */
        if (_DtHelpCeSkipToNextCcdfToken (in_file,in_buf,in_size,1,in_ptr,False) == -1)
            return -1;

        /*
         * charset string
         */
        if (tolower (**in_ptr) == 'c')
          {
            if (_DtHelpCeGetCcdfStrParam (in_file, in_buf, in_size,
                        1, in_ptr, True, False, False, False, &mycharSet) == -1)
                return -1;

            if (_DtHelpCeGetCcdfEndMark(in_file,in_buf,in_ptr,in_size,1) == -1)
	      {
		if (mycharSet)
		    free (mycharSet);
                return -1;
	      }

          }
        else if (**in_ptr != '>')
          {
            errno = CEErrorTopicSyntax;
            return -1;
          }
        else
          {
            /*
             * skip the end token
             */
            *in_ptr = *in_ptr + 1;
          }
      }
    else
      {
	errno = CEErrorMissingTopicCmd;
	return -1;
      }

    if (ret_charSet)
	*ret_charSet = mycharSet;
    else if (mycharSet)
	free (mycharSet);

    return 0;
}

/******************************************************************************
 * Function: int _DtHelpCeCheckNextCcdfCmd (char *token, FILE *in_file, char in_buf,
 *				int in_size, char **in_ptr)
 * 
 * Parameters:
 *		token		Specifies the syntax to look for.
 *					<token ....> data </>
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *
 * Returns:	0 if successful, -1 if errors, -2 if token not found.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Determine if the next formatting command is
 *			<'token' ...>.
 *
 * Note:	'token' must be a lower case string.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeCheckNextCcdfCmd (token, in_file, in_buf, in_size, cur_mb_len, in_ptr)
	char	  *token;
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	int        cur_mb_len;
	char     **in_ptr;
#else
_DtHelpCeCheckNextCcdfCmd(
	char	  *token,
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	int        cur_mb_len,
	char     **in_ptr )
#endif /* _NO_PROTO */
{
    int      result = 0;
    int      len = strlen (token);
    char    *str;

    /*
     * check for the token
     */
    while (len > 0 && result > -1)
      {
        if (**in_ptr == '\0' &&
		_DtHelpCeGetNxtBuf(in_file, in_buf, in_ptr, in_size) == -1)
            return -1;

        if (cur_mb_len == 1 || mblen (*in_ptr, cur_mb_len) == 1)
          {
            if ((isspace (**in_ptr) || **in_ptr == '\n'))
                *in_ptr = *in_ptr + 1;
            else if (**in_ptr == '<')
              {
                if (((int)strlen(*in_ptr)) < (len + 1) &&
				_DtHelpCeGetNxtBuf (in_file, in_buf,
                                            in_ptr, in_size) == -1)
                    return -1;

		for (str = *in_ptr + 1; len > 0; len--, str++, token++)
		  {
                    if (tolower (*str) != *token)
			return -2;
		  }
                result = 0;
              }
            else
                result = -2;
          }
        else
            result = -2;
      }

    return result;
}

/*****************************************************************************
 * Function: int _DtHelpCeGetCcdfTopicAbbrev (void *dpy, FILE *file, char *buffer,
 *			char **buf_ptr, int buf_size, CEBoolean strip,
 *			char **ret_title, char **ret_charSet,
 *			char **ret_abbrev)
 *
 * Parameters:
 *		dpy		Specifies the display connection as a void.
 *				Do it this way to prevent the terminal
 *				functions from having to know about Xlib.
 *				Terminal/Access Functions should pass NULL
 *				to this routine.
 * 		file		Specifies the stream of the open file.
 *		buffer		The buffer holding the current information.
 *		buf_ptr		Specifies the pointer into 'buffer' to
 *					start processing.
 *				Returns the new location in 'buffer' to
 *					continue processing.
 *		buf_size	The size of buffer.
 *		ret_title 	Returns a null terminated string
 *				containing the title if found.
 *		ret_charSet 	Returns a null terminated string
 *				containing the character set if found.
 *		ret_abbrev 	Returns a null terminated string
 *				containing the abbreviated title if found.
 *
 * Memory own by caller:
 *		ret_name
 *		ret_charSet
 *		ret_abbrev
 *
 * Returns:	0 if successful, -2 if <TOPIC> not found, otherwise -1.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Find the title and abbreviated title of a topic.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfTopicAbbrev (dpy, file, buffer, buf_ptr, buf_size, cur_mb_len,
		ret_title, ret_charSet, ret_abbrev)
	void		 *dpy;
	BufFilePtr	  file;
	char		 *buffer;
	char		**buf_ptr;
	int		  buf_size;
	int		  cur_mb_len;
	char		**ret_title;
	char		**ret_charSet;
	char		**ret_abbrev;
#else
_DtHelpCeGetCcdfTopicAbbrev (
	void		 *dpy,
	BufFilePtr	  file,
	char		 *buffer,
	char		**buf_ptr,
	int		  buf_size,
	int		  cur_mb_len,
	char		**ret_title,
	char		**ret_charSet,
	char		**ret_abbrev )
#endif /* _NO_PROTO */
{
    /*
     * NULL the entries.
     */
    if (ret_title)
        *ret_title   = NULL;
    if (ret_abbrev)
        *ret_abbrev  = NULL;

    /*
     * check for <TOPIC charset string>
     */
    if (_DtHelpCeGetCcdfTopicCmd (dpy,
			  file,
			  buffer,
			  buf_ptr,
			  buf_size,
			  cur_mb_len,
			  ret_charSet) == -1
			||
	/* Must have a <TITLE> token*/
	GetTitleCmd(file, buffer, buf_size, cur_mb_len,buf_ptr,ret_title) == -1
			||
	/* check for the <ABBREV> token */
	_DtHelpCeGetCcdfAbbrevCmd (file, buffer, buf_size, cur_mb_len,buf_ptr, ret_abbrev) == -1)

	return -1;

    return 0;
}

/*****************************************************************************
 * Function: int _DtHelpCeSkipCcdfAbbrev (FILE *file, char *buffer, int buf_size,
 *							char **buf_ptr)
 *
 * Parameters:
 * 		file		Specifies the stream of the open file.
 *		buffer		The buffer holding the current information.
 *		buf_size	The size of buffer.
 *		buf_ptr		Specifies the pointer into 'buffer' to
 *					start processing.
 *				Returns the new location in 'buffer' to
 *					continue processing.
 *
 * Returns:	0 if successful, otherwise -1.
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorMalloc
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Skip the <ABBREV> command
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeSkipCcdfAbbrev (file, buffer, buf_ptr, buf_size, cur_mb_len);
	BufFilePtr	  file;
	char		 *buffer;
	char		**buf_ptr;
	int		  buf_size;
	int		  cur_mb_len;
#else
_DtHelpCeSkipCcdfAbbrev (
	BufFilePtr	  file,
	char		 *buffer,
	char		**buf_ptr,
	int		  buf_size,
	int		  cur_mb_len)
#endif /* _NO_PROTO */
{
    /*
     * check for the <ABBREV> token and skip its data.
     */
    return _DtHelpCeGetCcdfAbbrevCmd(file, buffer, buf_size, cur_mb_len,buf_ptr, NULL);
}

/******************************************************************************
 * Function: int _DtHelpCeGetCcdfAbbrevCmd (FILE *in_file, char *in_buf, int in_size,
 *				char **in_ptr, char **ret_string)
 * 
 * Parameters:
 *		in_file		Specifies a stream to read from.
 *		in_buf		Specifies the buffer where new information
 *					is placed.
 *		in_size		Specifies the maximum size of 'in_buf'.
 *		in_ptr		Specifies the pointer into 'in_buf' to
 *					start processing.
 *				Returns the new location in 'in_buf' to
 *					continue processing.
 *		ret_string	Returns the data found.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *		read (2)	Errors set via a read call.
 *		EINVAL
 *		CEErrorIllegalInfo
 *		CEErrorReadEmpty
 *
 * Purpose:	Determine if the next formatting command is
 *			<ABBREV ...>.
 *		Returns the data found between <ABBREV> and its ending </>.
 *
 * Note:	The only formatting command allowed between <ABBREV> and
 *		its </> is the <NEWLINE> command. And it is stripped.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfAbbrevCmd (in_file, in_buf, in_size, cur_mb_len, in_ptr, ret_string)
	BufFilePtr in_file;
	char      *in_buf;
	int        in_size;
	int        cur_mb_len;
	char     **in_ptr;
	char     **ret_string;
#else
_DtHelpCeGetCcdfAbbrevCmd(
	BufFilePtr in_file,
	char      *in_buf,
	int        in_size,
	int        cur_mb_len,
	char     **in_ptr,
	char     **ret_string )
#endif /* _NO_PROTO */
{
    int       result;
    int       junkSize  = 0;
    int       junkMax  = 0;

    /*
     * null the return string
     */
    if (ret_string)
	*ret_string = NULL;

    /*
     * check for the token
     */
    result = _DtHelpCeCheckNextCcdfCmd ("ab", in_file, in_buf, in_size, cur_mb_len, in_ptr);
    if (result == -2)
	result = 0;
    else if (result == 0 &&
	_DtHelpCeGetCcdfEndMark(in_file,in_buf,in_ptr,in_size,cur_mb_len) != -1)
      {
	result = GetCmdData (in_file, in_buf, in_size, in_ptr, cur_mb_len,
					CE_NEWLINE_CMD, CE_NEWLINE_CMD,
					&junkSize, &junkMax, ret_string);
        if (result == -1 && errno == CMD_NOT_ALLOWED)
	    errno = CEErrorAbbrevSyntax;
      }
    else
	result = -1;

    return result;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetCcdfCmd (int current, char *buffer,
 *					  char **buf_ptr, FILE *my_file,
 *					  int size, int allowed, CEBoolean flag )
 *
 * Parameters:	current		Specifies the current formatting command.
 *		buffer		Specifies the buffer to use for input.
 *		buf_ptr		Specifies the current position into 'buffer'.
 *		my_file		Specifies the file stream to read from.
 *		size		Specifies the maximum size of the input buffer.
 *		allowed		Specifies the commands allowed at this time.
 *		flag		Specifies the flag to use for
 *					_DtHelpCeGetNxtBuf. No longer used.
 *
 * Returns
 *		-1	If errors reading the file or the command is not
 *			allowed
 *		> 0	The formmatting command.
 *
 * errno Values:
 *
 * Purpose:	Determine if the string pointed to by 'ptr' is a formatting
 *		command.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfCmd (current, buffer, buf_ptr, my_file, size, allowed)
    int     current;
    char   *buffer;
    char  **buf_ptr;
    BufFilePtr my_file;
    int     size;
    int     allowed;
#else
_DtHelpCeGetCcdfCmd (
    int     current,
    char   *buffer,
    char  **buf_ptr,
    BufFilePtr my_file,
    int     size,
    int     allowed)
#endif /* _NO_PROTO */
{
    int   i = -1;
    int   j = 1;
    int   different;
    int   my_error = 0;
    char  firstChar;
    char *myPtr;

    /*
     * always allow the end of formatting command
     */
    allowed = allowed | CE_FORMAT_END;

    /*
     * set the pointer to the current position in the input buffer
     */
    myPtr = *buf_ptr;
    if (*myPtr == '\0' &&
		_DtHelpCeGetNxtBuf(my_file, buffer, &myPtr, size) == -1)
	return -1;

    firstChar = tolower (*myPtr);
    do {
	i++;
	different = firstChar - CcdfFormatCmds[i].cmd[0];

	if (!different && CcdfFormatCmds[i].significant > 1)
	  {
	    if (((int)strlen(myPtr)) < CcdfFormatCmds[i].significant &&
		_DtHelpCeGetNxtBuf (my_file, buffer, &myPtr, size) == -1)
	        return -1;

	    j = 1;
	    do {
		different = tolower(myPtr[j]) - CcdfFormatCmds[i].cmd[j];
		j++;
	      } while (!different && j < CcdfFormatCmds[i].significant);
	  }

      } while (different && CcdfFormatCmds[i].type != CE_FORMAT_END);

    /*
     * update the passed in pointer
     */
    *buf_ptr = myPtr;

    /*
     * check to see if the formatting command is valid
     */
    if (different)
	errno = CEErrorFormattingCmd;

    else if (!different && CE_NOT_ALLOW_CMD (allowed, CcdfFormatCmds[i].type))
      {
	my_error = -1;
	switch (current)
	  {
	    case CE_ABBREV_CMD:
			errno = CEErrorAbbrevSyntax;
			break;
	    case CE_FIGURE_CMD:
			errno = CEErrorFigureSyntax;
			break;
	    case CE_FONT_CMD:
			errno = CEErrorFontSyntax;
			break;
	    case CE_GRAPHIC_CMD:
			errno = CEErrorGraphicSyntax;
			break;
	    case CE_ID_CMD:
			errno = CEErrorIdSyntax;
			break;
	    case CE_LABEL_CMD:
			errno = CEErrorLabelSyntax;
			break;
	    case CE_LINK_CMD:
			errno = CEErrorLinkSyntax;
			break;
	    case CE_NEWLINE_CMD:
			errno = CEErrorNewLineSyntax;
			break;
	    case CE_OCTAL_CMD:
			errno = CEErrorOctalSyntax;
			break;
	    case CE_PARAGRAPH_CMD:
			errno = CEErrorParagraphSyntax;
			break;
	    case CE_TITLE_CMD:
			errno = CEErrorTitleSyntax;
			break;
	    case CE_TOPIC_CMD:
			errno = CEErrorTopicSyntax;
			break;
	  }
      }
    if (different || my_error)
	return -1;

    return CcdfFormatCmds[i].type;
}

/******************************************************************************
 * Function:	int _DtHelpCeGetCcdfFontType (char *code)
 *
 * Parameters:	code	Specifies the character set to initialize
 *			data and flags. The caller must ensure that
 *			there is enough of the code string available
 *			to make the determination.
 *
 * Return Value: int	Returns the font resource to change.
 *			-1 if unknown font change type.
 *
 * Purpose:	Determines if the code is a font change specification.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfFontType (code)
    char      *code;
#else
_DtHelpCeGetCcdfFontType (char      *code)
#endif /* _NO_PROTO */
{
    char  my2Char;
    char  myChar = tolower (*code);

    code++;
    my2Char = tolower (*code);
    switch (myChar)
      {
        /*
         * <angle string>
         */
        case 'a':
		if (my2Char == 'n')
		    return _CEFONT_ANGLE;
		break;
        /*
         * <charset string>
         */
        case 'c': return _CEFONT_CHAR_SET;

        /*
         * <type string>
         */
        case 't':
		if (my2Char == 'y')
		    return _CEFONT_TYPE;
		break;

        /*
         * <weight string>
         */
        case 'w': return _CEFONT_WEIGHT;

        /*
         * <size string>
         * <spacing string>
         */
        case 's':
            if (my2Char == 'i')
		return _CEFONT_SIZE;

            if (my2Char == 'p')
		return _CEFONT_SPACING;
      }
    return -1;
}
