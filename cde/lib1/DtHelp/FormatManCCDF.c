/* $XConsortium: FormatManCCDF.c /main/cde1_maint/2 1995/08/26 22:41:14 barstow $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	Format.c
 **
 **   Project:     Text Graphic Display Library
 **
 **   Description: This body of formats the input into a form understood by
 **		a Display Area.
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
#include <limits.h>
#include <nl_types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

/*
 * private includes
 */
#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "AccessP.h"
#include "AccessI.h"
#include "CESegmentI.h"
#include "FontAttrI.h"
#include "CanvasI.h"
#include "FormatCCDFI.h"
#include "FormatManCCDFI.h"
#include "CEUtilI.h"
#include "LocaleXlate.h"

#ifdef NLS16
#endif

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
enum	State	{
	Char,
	Bold,
	BoldDone,
	Italic,
	BoldItalic,
	BoldItalicDone};

#define	STDOUT_FD (1)	/* File descriptor for std. output. */
#define	STDERR_FD (1)	/* File descriptor for std. error. */

#define	PIPE_OUT  (0)	/* The output end of a pipe. */
#define	PIPE_IN   (1)	/* The input end of a pipe. */

static	char	*ScanString  = "<\n\\ ";
static	char	*EndToken    = "</>";
static	char	*BoldToken   = "<weight bold>";
static	char	*ItalicToken = "<angle italic>";

static	int	EndTokenSize    = 3;
static	int	BoldTokenSize   = 13;
static	int	ItalicTokenSize = 14;

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	int	FormatManPage();
#else
static	int	FormatManPage(
			CanvasHandle	  canvas_handle,
			VarHandle	  my_vars,
			BufFilePtr	  in_file,
			char		 *in_buf,
			int		  in_size,
			CEFontSpec	*font_attr,
			char		**out_buf,
			int		 *out_size,
			int		 *out_max );
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#define	FileStream(f)	((FILE *)(f)->hidden)

/******************************************************************************
*
* Private Functions
*
*****************************************************************************/
/*********************************************************************
* Procedure: GetNextState (char c, wchar_t wide_c, int length, char *str,
*			wchar_t ret_wc, int ret_wc_len, char ret_str_ptr,
                        Boolean ret_flag)
 *
 * Parameters:	wide_c		Specifies the wide char value of the
 *				previous character.
 *		length		Specifies the length in bytes of the
 *				previous character.
 *		str		Specifies the pointer to the string of
 *				bytes to be processed next.
 *		ret_wc		Returns the wide character that is to be
 *				italicized iff the state returned is Italic.
 *		ret_wc_len	Returns the length in bytes of ret_wc.
 *		ret_str_ptr	Returns the pointer to the next character
 *				after the bold or italic character.
 *              ret_flag        Returns whether we have encountered a
 *                              + followed by a _, to convert to +/-
 *
 * Returns:	Bold, Italic, Char
 *
 * Purpose:	To determine if the sequence of characters represent
 *		a bold or italic character.
 *
 *********************************************************************/
static	enum State
#ifdef _NO_PROTO
GetNextState (c, wide_c, len, str, ret_c, ret_wc, ret_wc_len, ret_str_ptrr, ret_flag)
    char	 c;
    wchar_t	 wide_c;
    int		 len;
    char	*str;
    char	*ret_c;
    wchar_t	*ret_wc;
    int		*ret_wc_len;
    char	**ret_str_ptr;
    Boolean     *ret_flag; 
#else
GetNextState (
    char	 c,
    wchar_t	 wide_c,
    int		 len,
    char	*str,
    char	*ret_c,
    wchar_t	*ret_wc,
    int		*ret_wc_len,
    char	**ret_str_ptr,
    Boolean     *ret_flag )
#endif /* _NO_PROTO */
{
    register int i = len;
    
    /* check to see if we have the pattern of a bold + followed by a bold _
       purpose is to eventually convert this to +/-
       This is a hack because the nroff output produced for the sequence
       of a + with an _ UNDER it, as in indicating statistical significance,
       looks like a bold +, backspace, and then a bold _.
       However, the nroff output indicates <c>bs_ as an italic, so
       this sequence has to be treated as a special case. */
       
    if (*str == '+')
      {
	if (str[7] == '\010' && str[8] == '_' &&
	    str[9] == '\010' && str[10] == '_' &&
	    str[11] == '\010' && str[12] == '_' &&
	    str[13] == '\010' && str[14] == '_' )
	  {
	    *ret_flag = TRUE;
	  } 
      }

    /* We need to eat the bold '_' -- continuation of aforementioned hack */
    if (*ret_flag)
      {
	if (c == '+' &&
	    str[0] == '\010' && str[1] == '_' &&
	    str[2] == '\010' && str[3] == '_' &&
	    str[4] == '\010' && str[5] == '_' &&
	    str[6] == '\010' && str[7] == '_' )
	  {
	    *ret_flag = FALSE;
	    *ret_str_ptr = str + 8;
	    return Bold;
	  }
      }
    
    /*
     * check for multi-byte italic which is '_ _bsbs<c>'
     */
    if (len == 1 && c == '_' && mblen (str, MB_CUR_MAX) == 1 && *str == '_')
      {
	str++;
	i++;
      }

    /*
     * check for backspaces equal to the number of bytes making up wide_c.
     */
    while (i)
      {
	if (mblen (str, MB_CUR_MAX) != 1 || *str != '\010')
	    return Char;
	i--;
	str++;
      }

    /*
     * convert the next character
     */
    *ret_wc_len = mbtowc(ret_wc, str, MB_CUR_MAX);

    /*
     * check for validity
     */
    if ((*ret_wc_len) < 1)
	return Char;

    /*
     * check to make sure the next character is equal to the first character
     * if so, the sequence was <c>bs<c> indicating a bold character.
     */
    if (*ret_wc_len == len && *ret_wc == wide_c)
      {
	/*
	 * Do not set the return string ptr prior to this return.
	 * If the user specifies the same variable for both the str and
	 * ret_str_ptr, then we will overwrite it improperly if the
	 * sequence is not an italic or bold.
	 */
        *ret_str_ptr = str + *ret_wc_len;
	return Bold;
      }

    /*
     * Check to see if the sequence is _bs<c> for italics.
     * If so, take the next character for the ret_c (even if it
     * is a multi-byte character). ret_wc, and ret_wc_len are
     * already set from the mbtowc call. Move the ptr to after
     * the processed character.
     */
    if (len == 1 && c == '_')
      {
	*ret_c       = *str;
        *ret_str_ptr = str + *ret_wc_len;
	return Italic;
      }

    /*
     * Check to see if the sequence is <c>bs_ for italics.
     * If so, return the given c for the ret_c (even if it
     * is a multi-byte character). Set ret_wc and ret_wc_len
     * to the passed in wide character and wide character len.
     * Move the ptr to after the underscore.
     */
    if (*ret_wc_len == 1 && *str == '_')
      {
        *ret_c       = c;
        *ret_str_ptr = str + *ret_wc_len;
	*ret_wc      = wide_c;
	*ret_wc_len  = len;
	return Italic;
      }
    
    /*
     * failed all the tests.
     * It could be a character followed by backspaces
     * followed by another character (which would cause over-
     * striking), but we don't know how to deal with this.
     */
    return Char;
}

/******************************************************************************
 *
 *  Formatting a man page is done based on the current input and the
 *  current state of the processing. 
 *
 * Note:
 *	Bold multi-byte characters have one backspace per byte. Therefore,
 *		a two-byte character would be be followed by two backspaces
 *		and the same two-bytes to indicate that the character is to
 *		be a bold character.
 *	Italic characters can either be preceeded by a backspace/underline
 *		or followed by a backspace/underline. I.E. _bsAB<bs_
 *		would be an italic capitol A, a regular capital B and
 *		an italic less than sign.
 *	All backslashes and less than signs are output with a backslash
 *		preceeding them.
 *
 * The following characters have special meaning in the state table:
 *	<c>	indicates a character.
 *	<?>	indicates the next character in the sequence.
 *	<_>	indicates an underscore character.
 *	bs	indicates a backspace character.
 *	nl	indicates a new-line character.
 *
 *  Cur State		Cur Sequence		Action		New State
 * ------------------------------------------------------------------------
 *  Char		<c>bs<c><?>		Bold -> buf	Bold
 *						<c>  -> buf
 *						lastChar = <c>
 *						incr to <?>
 *
 *			<_>bs<c><?>		Italic -> buf	Italic
 *						<c>  -> buf
 *						incr to <?>
 *
 *			<c>bs<_><?>		Italic -> buf	Italic
 *						<c>  -> buf
 *						incr to <?>
 *
 *			nl<?>			dump buf	Char
 *						incr to <?>
 *
 *  			<c><?>			<c>  -> buf	Char
 *						incr to <b>
 *
 *  Bold		bs<c><?>
 *			<c> = lastChar		skip to <?>	Bold
 *
 *			nl<?>			<\> -> buf	Char
 *						dump buf
 *						incr to <?>
 *
 *			<c><?>			<\> -> buf	BoldDone
 *
 *  BoldDone		<c>bs<c><?>		erase <\>	Bold
 *						<c> -> buf
 *						lastChar = <c>
 *						incr to <?>
 *
 *			<_>bs<c><?>		Italic -> buf	Italic
 *						<c>  -> buf
 *						incr to <?>
 *
 *			<c>bs<_><?>		Italic -> buf	Italic
 *						<c>  -> buf
 *						incr to <?>
 *
 *			nl<?>			dump buf	Char
 *						incr to <?>
 *
 *			<c><?>			<c> -> buf	Char
 *						incr to <?>
 *
 *  Italic		<c>bs<c><?>		<\> -> buf	Char
 *
 *  			<_>bs<c><?>		<c> -> buf	Italic
 *						incr to <?>
 *
 *			<c>bs<_><?>		<c>  -> buf	Italic
 *						incr to <?>
 *
 *			bs<c><?>		erase lastChar	BoldItalic
 *						Bold -> buf
 *						<c>  -> buf
 *						lastChar = <c>
 *						incr to <?>
 *
 *			nl<?>			<\> -> buf	Char
 *						dump buf
 *						incr to <?>
 *
 *  			<c><?>			</> -> buf 	Char
 *
 *  BoldItalic		bs<c><?>		skip to <?>	BoldItalic
 *
 *			<c><?>			<\> -> buf	BoldItalicDone
 *
 *  BoldItalicDone	_bs<c>bs<c><?>	erase <\>	ItalicBold
 *					<c> -> buf
 *					lastChar = <c>
 *					inc to <?>
 *
 *			_bs<c><?>	<c> -> buf	Italic
 *					lastChar = <c>
 *					inc to <?>
 *					
 *			<c><?>		<\> ->buf	Char
 *					<c> -> buf
 *					lastChar = <c>
 *					inc to <?>
 *
 ******************************************************************************/
/******************************************************************************
 * Function: RemoveToken
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
RemoveToken(size, out_buf, out_size)
    int		  size;
    char	**out_buf;
    int		 *out_size;
#else
RemoveToken(
    int		  size,
    char	**out_buf,
    int		 *out_size)
#endif /* _NO_PROTO */
{
    *out_size = *out_size - size;

    if (size == 1 &&
		((*out_buf)[*out_size] == '\\' || (*out_buf)[*out_size] == '<'))
	*out_size = *out_size - 1;

    (*out_buf)[*out_size] = '\0';
}

/*********************************************************************
 * Function: WriteToken
 *********************************************************************/
static	int
#ifdef _NO_PROTO
WriteToken(token, size, out_buf, out_size, out_max)
    char	 *token;
    int		  size;
    char	**out_buf;
    int		 *out_size;
    int		 *out_max;
#else
WriteToken(
    char	 *token,
    int		  size,
    char	**out_buf,
    int		 *out_size,
    int		 *out_max )
#endif /* _NO_PROTO */
{
    char	*ptr = token;

    return (_DtHelpCeAddStrToBuf(&ptr, out_buf, out_size, out_max, size, 128));
}

/*********************************************************************
 * Function: WriteOutChar
 *********************************************************************/
static	int
#ifdef _NO_PROTO
WriteOutChar(last_len, last_wc, c, out_buf, out_size, out_max, flag)
    int		  last_len;
    wchar_t	  last_wc;
    char	  c;
    char	**out_buf;
    int		 *out_size;
    int		 *out_max;
    Boolean       flag;
#else
WriteOutChar(
    int		  last_len,
    wchar_t	  last_wc,
    char	  c,
    char	**out_buf,
    int		 *out_size,
    int		 *out_max,
    Boolean       flag )
#endif /* _NO_PROTO */
{
    char  *ptr;
    char   temp[10];
    
    if (last_len == 1 && c == '<')
	ptr = "\\<";
    else if (last_len == 1 && c == '\\')
	ptr = "\\\\";
    else
      {
	(void) wctomb (temp, last_wc);
	temp[last_len] = '\0';
	ptr            = temp;
      }

    /*  here is the conversion for the bold '+' followed by bold '_'
        part of a hack to treat this special character sequence */
    if (flag && c == '+')
      ptr = "+/-";

    return (_DtHelpCeAddStrToBuf(&ptr, out_buf, out_size, out_max,
							strlen(ptr), 128));
}

/*********************************************************************
 * Function: FormatManPage
 *
 *    FormatManPage is the top entry point for formating man pages
 *	into a form understood by a display area.
 *
 *********************************************************************/
static	int
#ifdef _NO_PROTO
FormatManPage(canvas_handle, my_vars, in_file, in_buf, in_size, font_attr, out_buf, out_size, out_max)
	CanvasHandle	  canvas_handle;
	VarHandle	  my_vars;
	BufFilePtr	  in_file;
	char		 *in_buf;
	int		  in_size;
	CEFontSpec	*font_attr;
	char		**out_buf;
	int		 *out_size;
	int		 *out_max;
#else
FormatManPage(
	CanvasHandle	  canvas_handle,
	VarHandle	  my_vars,
	BufFilePtr	  in_file,
	char		 *in_buf,
	int		  in_size,
	CEFontSpec	*font_attr,
	char		**out_buf,
	int		 *out_size,
	int		 *out_max )
#endif /* _NO_PROTO */
{
    int		 italicCnt = 0;
    int		 result = 0;
    int		 cread;
    int		 lastLen;
    int		 checkLen;
    int		 retWCLen;
    wchar_t	 lastWC;
    wchar_t	 retWC;
    char	*rloc = in_buf;
    char	*retStrPtr;
    char	 c;
    char	 retC;
    Boolean      flag = False;
    enum State	 newState;
    enum State	 state = Char;

    cread = strlen (rloc);

    do
      {
	/*
	 * while I can read information process; loop.
	 */
	while (result != -1 && cread > 0)
	  {
	    /**
	     * check for the size of the character
	     **/
	    checkLen = mblen(rloc, MB_CUR_MAX);

	    /*
	     * if we hit a null character before we've run out of characters,
	     * we've got corrupt data.
	     */
	    if (checkLen == 0)
		return -1;

	    if (checkLen > 0)
	      {
		/*
		 * check for end of line
		 */
		if (checkLen == 1 && *rloc == '\n')
		  {
		    cread--;
		    if (state == Bold || state == Italic)
		        result = WriteToken(EndToken, EndTokenSize,
						out_buf, out_size, out_max);

		    if (result != -1)
		        result = _DtHelpCeAddCharToBuf (&rloc, out_buf,
						out_size, out_max, 128);
		    if (result != -1)
		      {
		        result = __DtHelpCeProcessString(canvas_handle,
					my_vars, NULL,
					ScanString,
					*out_buf,
					*out_size,
					_CESetStaticFlag(CE_STRING),
					font_attr);
		        *out_size = 0;
		        (*out_buf)[0] = '\0';
		        state = Char;
		      }
		  }
		else
		  {
		    switch (state)
		      {
			case Char:
			case BoldDone:
			case BoldItalicDone:
				/*
				 * get the character and wide character
				 * representation of the next character.
				 */
				c        = *rloc;
				lastLen  = mbtowc (&lastWC, rloc, MB_CUR_MAX);

				/*
				 * skip past this character.
				 */
				rloc     = rloc  + lastLen;
				cread    = cread - lastLen;

				/*
				 * Check ahead for bold or italic sequences
				 */
				newState = GetNextState (c, lastWC, lastLen,
						rloc,
						&retC, &retWC, &retWCLen,
						&retStrPtr, &flag);

				if (newState == Bold)
				  {
				    if (state == BoldDone)
					RemoveToken(EndTokenSize, out_buf, out_size);
				    else
					result = WriteToken(BoldToken,
							BoldTokenSize,
							out_buf, out_size,
							out_max);

				    /*
				     * skip the backspaces and the extra
				     * copy of the character.
				     */
				    cread = cread - (retStrPtr - rloc);
				    rloc  = retStrPtr;
				  }
				else if (newState == Italic)
				  {
				    if (state != BoldItalicDone)
					result = WriteToken(ItalicToken,
							ItalicTokenSize,
							out_buf, out_size,
							out_max);

				    /*
				     * skip the blanks after the current
				     * character plus the character after
				     * that. The returned wide character
				     * is the character that is to be
				     * italicized.
				     */
				    cread   = cread - (retStrPtr - rloc);
				    rloc    = retStrPtr;
				    c       = retC;
				    lastWC  = retWC;
				    lastLen = retWCLen;
				    italicCnt = 1;

				    if (state == BoldItalicDone &&
					GetNextState (c, lastWC, lastLen, rloc,
						&retC, &retWC, &retWCLen,
						&retStrPtr, &flag) == Bold)
				      {
					RemoveToken(EndTokenSize, out_buf, out_size);
					newState = BoldItalic;
				      }
				  }
				else if (state == BoldItalicDone)
				    result = WriteToken(EndToken, EndTokenSize,
							out_buf, out_size,
							out_max);

				state = newState;


				result = WriteOutChar(lastLen, lastWC, c,
						out_buf, out_size, out_max, flag);
				break;

			case BoldItalic:
			case Bold:
				if (GetNextState (c, lastWC, lastLen, rloc,
						&retC, &retWC, &retWCLen,
						&retStrPtr, &flag) == Bold)
				  {
				  /* skip backspaces and copy characters */
				    cread = cread - (retStrPtr - rloc);
				    rloc  = retStrPtr;
				  }
				else
				  {
				    result = WriteToken(EndToken, EndTokenSize,
						out_buf, out_size, out_max);
				    if (state == BoldItalic)
				        state = BoldItalicDone;
				    else
				        state = BoldDone;
				  }
				break;

			case Italic:
				c = *rloc;
				newState = GetNextState (c, lastWC, lastLen,
						rloc,
						&retC, &retWC, &retWCLen,
						&retStrPtr, &flag);

				if (newState == Italic)
				  {
				    italicCnt++;
				    cread   = cread - (retStrPtr - rloc);
				    rloc    = retStrPtr;
				    c       = retC;
				    lastWC  = retWC;
				    lastLen = retWCLen;

				    result = WriteOutChar(lastLen, lastWC, c,
						out_buf, out_size, out_max, flag);
				  }
				else if (italicCnt == 1 && lastWC == retWC
						&& newState == Bold)
				  {
				    RemoveToken(lastLen, out_buf, out_size);

				    result  = WriteToken(BoldToken,
							BoldTokenSize,
							out_buf, out_size,
							out_max);

				    cread   = cread - (retStrPtr - rloc);
				    rloc    = retStrPtr;
				    c       = retC;
				    lastWC  = retWC;
				    lastLen = retWCLen;

				    result = WriteOutChar(lastLen, lastWC, c,
						out_buf, out_size, out_max, flag);

				    state = BoldItalic;
				  }
				else
				  {
				    result    = WriteToken(EndToken,
							EndTokenSize,
							out_buf, out_size,
							out_max);
				    state     = Char;
				    italicCnt = 0;
				  }

				break;
		      }
		  }

		if (cread < (3 * ((int) MB_CUR_MAX)) &&
						!feof (FileStream(in_file)))
		    cread = 0;
	      }
	    else
	      {
		/**
		 * if it is an invalid character - skip.
		 * But be careful.
		 * If this is the start of a multi-byte character,
		 * I must save it and try again on the next read.
		 **/
		if (cread < ((int) MB_CUR_MAX))
		    cread = 0;
		else
		  {
		    /*
		     * otherwise we've got corrupt data.
		     */
		    return -1;
		  }
	      }
          }
	if (result != -1 && !feof(FileStream(in_file)))
	  {
	    if (_DtHelpCeGetNxtBuf (in_file, in_buf, &rloc, in_size) == -1)
		result = -1;

	    if (result != -1)
	        cread = strlen (rloc);
	  }
      } while (result != -1 && cread > 0);

    return(result);

} /* End FormatManPage */
/******************************************************************************
 *
 * Public Functions
 *
 *****************************************************************************/
/*********************************************************************
 * Function: _DtHelpCeFormatManCcdf
 *
 *    _DtHelpCeFormatManCcdf formats a man page
 *	into a form understood by a display area.
 *
 *********************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFormatManCcdf(canvas_handle, man_spec, ret_topic)
	CanvasHandle	 canvas_handle;
	char		*man_spec;
	TopicHandle	*ret_topic;
#else
_DtHelpCeFormatManCcdf(
	CanvasHandle	 canvas_handle,
	char		*man_spec,
	TopicHandle	*ret_topic)
#endif /* _NO_PROTO */
{
    int      result;
    FILE    *myFile;
    int      writeBufSize = 0;
    int      writeBufMax  = 0;
    char    *ptr;
    char    *writeBuf = NULL;
    char     readBuf[BUFSIZ];
    CEFontSpec    fontAttr;
    VarHandle  myVars;
    CETopicStruct *topicStruct;
    BufFilePtr myBufFile;
    static char manString[]     = "man ";
    static char devNullString[] = " 2>/dev/null";

    /*
     * Initialize the pointers.
     */
    *ret_topic = NULL;

    /*
     * pre-append the man command to man specification
     */
    ptr = (char *) malloc(sizeof(manString) + strlen(man_spec) +
						   sizeof(devNullString) - 1);
    if (!ptr)
	return -1;
    strcpy (ptr, manString);
    strcat (ptr, man_spec);
    strcat (ptr, devNullString);

    myFile = popen(ptr, "r");

    /*
     * free the man command
     */
    free (ptr);

    /*
     * check for problems
     */
    if (!myFile) /* couldn't create man(1) process */
      {
	return -1;
      }

    /*
     * initalize some variables.
     */

    /*
     * make sure we don't try to read compressed.
     */
    myBufFile = _DtHelpCeCreatePipeBufFile(myFile);
    if (myBufFile == NULL)
      {
	(void) pclose(myFile); /* don't check for error, it was popen'd */
	return -1;
      }

    /*
     * get the font quark list - but force to mono-space
     */
    _DtHelpCeCopyDefFontAttrList (&fontAttr);
    fontAttr.font_specs.spacing  = CEFontSpacingMono;
    _DtHelpCeXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,setlocale(LC_CTYPE,NULL),
				NULL, &(fontAttr.lang), &(fontAttr.char_set));

    myVars = __DtHelpCeSetUpVars(fontAttr.lang, fontAttr.char_set);
    if (myVars == NULL)
      {
	free(fontAttr.lang);
	free(fontAttr.char_set);
	_DtHelpCeBufFileClose (myBufFile, True);
	return -1;
      }

    readBuf[0] = '\0';
    ptr        = readBuf;

    result = _DtHelpCeGetNxtBuf (myBufFile, readBuf, &ptr, BUFSIZ);
    if (result > 0)
	 result = FormatManPage (canvas_handle, myVars, myBufFile,
				     readBuf,
				     BUFSIZ,
				     &fontAttr,
				     &writeBuf,
				     &writeBufSize,
				     &writeBufMax );

    if ((result != -1) && writeBufSize)
      {
	result = __DtHelpCeProcessString(canvas_handle, myVars, NULL,
			ScanString, writeBuf, writeBufSize,
			_CESetStaticFlag(CE_STRING), &fontAttr);
      }

    if (writeBuf)
        free (writeBuf);

    /*
     * close the pipe
     */
    _DtHelpCeBufFileClose (myBufFile, True);

    /*
     * clean up the last segment.
     */
    if (result != -1)
	__CEGetParagraphList (myVars, ret_topic);

    topicStruct = (CETopicStruct *) (*ret_topic);

   /*
    * did we have any paragraphs to format?
    */
   if (*ret_topic != NULL && topicStruct->para_num == 0)
      {
	free ((char *) topicStruct);
	*ret_topic = NULL;
	errno = ENOENT; /* we'll just assume no man page existed */
	result = -1;
      }

    free(fontAttr.lang);
    free(fontAttr.char_set);
    free(myVars);
    return result;

}  /* End _DtHelpCeFormatManCcdf */

