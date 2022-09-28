/* $XConsortium: Format.c /main/cde1_maint/3 1995/10/02 15:08:41 lehors $ */
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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <Xm/Xm.h>


extern	int	errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "DisplayAreaP.h"

/*
 * private core engine
 */
#include "bufioI.h"
#include "AccessI.h"
#include "CESegmentI.h"
#include "CEUtilI.h"
#include "FontI.h"
#include "FontAttrI.h"
#include "AccessP.h"
#include "FormatCCDFI.h"
#include "FormatSDLI.h"
#include "StringFuncsI.h"
#include "XInterfaceI.h"
#include "LocaleXlate.h"

#include "HelpErrorP.h"
#include "FormatI.h"


#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
#define	BUFF_SIZE	1024

static	char	*ScanString = "\n\t";

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	int FormatChunksToXmString ()
 *
 * Parameters:
 *               *ret_list may be NULL when called
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	Take some rich text chunks and turn it into an XmString.
 *
 ******************************************************************************/
static int
#ifdef _NO_PROTO
FormatChunksToXmString(pDAS, free_flag, title_chunks, ret_title, ret_list, ret_mod)
    DtHelpDispAreaStruct *pDAS;
    CEBoolean		  free_flag;
    void		**title_chunks;
    XmString		 *ret_title;
    XmFontList		 *ret_list;
    Boolean		 *ret_mod;
#else
FormatChunksToXmString(
    DtHelpDispAreaStruct *pDAS,
    CEBoolean		  free_flag,
    void		**title_chunks,
    XmString		 *ret_title,
    XmFontList		 *ret_list,
    Boolean		 *ret_mod )
#endif /* _NO_PROTO */
{
    int			 result = 0;
    int			 i;
    int			 j;
    int			 quarkCount;
    int			 chunkType;
    int			 myIdx;
    FontPtr		 fontPtr;
    XmStringCharSet	 charSet;
    const char		*strChunk;
    char		 buffer[16];
    CEFontSpec		 fontSpecs;
    XmFontContext	 fontContext;
    XmString		 partTitle;
    XmString		 newTitle;
    XrmQuark		 charSetQuark;
    XrmName		 myCharSetQuarks[20];
    XrmName		 xrmName[_CEFontAttrNumber];
    Boolean		 myMore;

    /*
     * Initialize the pointers.
     */
    *ret_title = NULL;
    *ret_mod   = False;

    if (title_chunks == NULL)
	return -1;

    /*
     * initialize the font context
     */
    _DtHelpCeCopyDefFontAttrList(&fontSpecs);
    if ( NULL != *ret_list )
      {
        if (XmFontListInitFontContext (&fontContext, *ret_list) == False)
            result = -1;
        else 
          {
            XFontStruct *myFontStruct;
            /*
             * quarkize all the character sets found.
             */
            quarkCount = 0;
            do
              {
                myMore = XmFontListGetNextFont (fontContext, &charSet,
                                                                &myFontStruct);
                if (myMore)
                  {
                    myCharSetQuarks[quarkCount++] = 
                                                XrmStringToQuark (charSet);
                    XtFree (charSet);
                  }
              } while (myMore);
    
            XmFontListFreeFontContext (fontContext);
          }
      } /* if NULL != *ret_list */
    else
      { /* if NULL == *ret_list */
         quarkCount = 0;
         myCharSetQuarks[0] = 0;
      }

    /*
     * Build the XrmString based on the segments.
     * The format of the returned information is
     *		'DT_HELP_CE_CHARSET  locale  string'
     *		'DT_HELP_CE_FONT_PTR fontptr string'
     *		'DT_HELP_CE_SPC      spc'
     *		'DT_HELP_CE_STRING   string' - uses last specified
     *                                         charset/font_ptr.
     *
     * The order and manner in which the title_chunks are processed
     * is known and depended upon in several locations.
     * Do not change this without changing the other locations.
     * See the _DtHelpFormatxxx() routines and the ones that
     * create the title_chunk arrays in FormatSDL.c and FormatCCDF.c
     */
    myIdx = __DtHelpDefaultFontIndexGet(pDAS);
    _DtHelpCopyDefaultList(xrmName);
    for (i = 0; result == 0 && title_chunks[i] != DT_HELP_CE_END; i++)
      {
        /*
         * create a string for the char set and a quark for it.
         */
	chunkType = (int) title_chunks[i++];

        /*
	 * i now points to the first value after the type
	 */
	if (chunkType & DT_HELP_CE_CHARSET)
	  {
	    char *charSet;
	    char *lang = (char *) title_chunks[i];

	    /*
	     * test to see if the locale is in a lang.codeset form
	     */
	    if (_DtHelpCeStrchr(lang, ".", 1, &charSet) == 0)
	      {
		*charSet = '\0';
		charSet++;
	      }
	    else
	      {
		charSet = lang;
		lang    = NULL;
	      }

	    /*
	     * resolve/load the font for the default fonts
	     */
	    _DtHelpDAResolveFont(pDAS, lang, charSet,
					fontSpecs.font_specs, NULL, &fontPtr);
	    myIdx = (int) fontPtr;
	    if (lang != NULL)
	      {
		charSet--;
		*charSet = '.';
	      }

	    if (free_flag)
	        free(title_chunks[i]);

	    /*
	     * move the i to point to the string.
	     */
	    i++;
	  }
	else if (chunkType & DT_HELP_CE_FONT_PTR)
	  {
	    /*
	     * get the default font for the language and code set.
	     */
	    (void) __DtHelpFontCharSetQuarkGet(pDAS, (int)title_chunks[i],
					&xrmName[_DT_HELP_FONT_CHAR_SET]);
	    (void) __DtHelpFontLangQuarkGet(pDAS, (int)title_chunks[i],
					&xrmName[_DT_HELP_FONT_LANG_TER]);
	    (void) __DtHelpFontIndexGet(pDAS, xrmName, &myIdx);

	    /*
	     * move the i to point to the string.
	     */
	    i++;
	  }

        /*
	 * the i point spc or string.
	 */
	if (chunkType & DT_HELP_CE_SPC)
	  {
	    j        = (int) title_chunks[i];
	    strChunk = _DtHelpDAGetSpcString(pDAS->spc_chars[j].spc_idx);
	    strChunk = strdup(strChunk);
	    fontPtr  = pDAS->spc_chars[j].font_ptr;

	    /*
	     * get the default font for the language and code set.
	     */
	    (void) __DtHelpFontCharSetQuarkGet(pDAS, (int)fontPtr,
					&xrmName[_DT_HELP_FONT_CHAR_SET]);
	    (void) __DtHelpFontLangQuarkGet(pDAS, (int)fontPtr,
					&xrmName[_DT_HELP_FONT_LANG_TER]);
	    (void) __DtHelpFontIndexGet(pDAS, xrmName, &myIdx);
	  }
	else /* if (chunkType & _DT_HELP_CE_STRING) */
	    strChunk = (char *) title_chunks[i];

	sprintf(buffer, "%d", myIdx);
	charSetQuark = XrmStringToQuark(buffer);

        j = 0;
        while (j < quarkCount && myCharSetQuarks[j] != charSetQuark)
            j++;

        /*
         * If we didn't find a matching character set,
         * add it to the list.
         */
        if (j >= quarkCount)
          {
	    if (myIdx < 0)
	      {
		XFontSet fontSet = __DtHelpFontSetGet(pDAS->font_info, myIdx);
                XmFontListEntry fontEntry;

		fontEntry = XmFontListEntryCreate (buffer,
						XmFONT_IS_FONTSET, fontSet);
		*ret_list = XmFontListAppendEntry (*ret_list, fontEntry);
		XmFontListEntryFree (&fontEntry);
	      }
	    else
	      {
		XFontStruct *fontStruct =
				__DtHelpFontStructGet(pDAS->font_info, myIdx);
                XmFontListEntry fontEntry;

		fontEntry = XmFontListEntryCreate (buffer,
						XmFONT_IS_FONT, fontStruct);
		*ret_list = XmFontListAppendEntry (*ret_list, fontEntry);
		XmFontListEntryFree (&fontEntry);
	      }

           *ret_mod = True;
            if (*ret_list == NULL)
                result = -1;

            myCharSetQuarks[quarkCount++] = charSetQuark;
          }

        /*
         * add this segment to the XmString.
         */
        if (result == 0)
          {
            if (*ret_title == NULL)
                *ret_title = XmStringCreateLtoR ((char *) strChunk, buffer);
            else
              {
                partTitle = XmStringCreateLtoR ((char *) strChunk, buffer);

                newTitle = XmStringConcat (*ret_title, partTitle);

                XmStringFree (*ret_title);
                XmStringFree (partTitle);
                *ret_title = newTitle;
              }

            /*
             * if a newline was specified,
             * replace it with a blank.
             */

            if (*ret_title != NULL && (chunkType & DT_HELP_CE_NEWLINE))
              {
                partTitle = XmStringCreateLtoR (" ", buffer);
                newTitle = XmStringConcat (*ret_title, partTitle);
                XmStringFree (*ret_title);
                XmStringFree (partTitle);
                *ret_title = newTitle;
              }

            if (*ret_title == NULL)
                result = -1;
          }

	/* 
	 * deallocate the memory for title_chunks
	 */
	if (free_flag && !(chunkType & DT_HELP_CE_SPC))
	    free(title_chunks[i]);
      }
    /*
     * deallocate the memory.
     */
    if (free_flag) free(title_chunks);
    return result;
}

/******************************************************************************
 *
 * Semi-Public Functions
 *
 *****************************************************************************/
/*****************************************************************************
 * Function:	int _DtHelpFormatAsciiFile (char *filename,
 *					CEParagraph **ret_para, int *ret_num)
 *
 * Parameters:
 *		filename	Specifies the ascii file to read.
 *		ret_para	Returns a pointer to a list of CEParagraph
 *				structures.
 *		ret_num		Returns the number of structures in 'ret_para'.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *		EINVAL
 *
 * Purpose:	_DtHelpFormatAsciiFile formats Ascii Files into a list of
 *		CEParagraph structures.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatAsciiFile(pDAS, filename, ret_handle )
	DtHelpDispAreaStruct *pDAS;
	char		 *filename;
	TopicHandle	 *ret_handle;
#else
_DtHelpFormatAsciiFile(
	DtHelpDispAreaStruct *pDAS,
	char		 *filename,
	TopicHandle	 *ret_handle)
#endif /* _NO_PROTO */
{
    int	       myFile;
    int	       result = -1;
    CEFontSpec fontAttrs;
    char       buffer [BUFF_SIZE];
    BufFilePtr rawInput;
    VarHandle	   varHandle;

    /*
     * check the parameters
     */
    if (filename == NULL || ret_handle == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * Initialize the pointers, buffers and counters
     */
    *ret_handle  = NULL;

    /*
     * open the file.
     */
    myFile = open (filename, O_RDONLY);
    if (myFile != -1)
      {
	/*
	 * set the information
	 */
	rawInput = _DtHelpCeBufFileRdWithFd(myFile);
	if (rawInput == 0)
	  {
	    close (myFile);
 	    errno = CEErrorMalloc;
	    return -1;
	  }

        result = _DtHelpCeReadBuf (rawInput, buffer, BUFF_SIZE);

	if (result != -1)
	  {
	    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
	    fontAttrs.font_specs.spacing = CEFontSpacingMono;
	    _DtHelpCeXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				setlocale(LC_CTYPE,NULL), NULL,
				&(fontAttrs.lang), &(fontAttrs.char_set));

	    varHandle = __DtHelpCeSetUpVars(fontAttrs.lang, fontAttrs.char_set);
	    if (varHandle == NULL)
	      {
	        free(fontAttrs.lang);
	        free(fontAttrs.char_set);
		return -1;
	      }

	    result = __DtHelpCeProcessString (pDAS->canvas, varHandle, rawInput,
				ScanString, buffer, BUFF_SIZE,
				_CESetStaticFlag (CE_STRING), &fontAttrs);

	    if (result != -1)
		result = __DtHelpCeGetParagraphList (varHandle, ret_handle);

	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(varHandle);
	  }
	_DtHelpCeBufFileClose(rawInput, True);
      }

    return result;

}  /* End _DtHelpFormatAsciiFile */

/*****************************************************************************
 * Function:	int _DtHelpFormatAsciiString (char *input_string,
 *					CEParagraph **ret_para, int *ret_num)
 *
 * Parameters:
 *		input_string	Specifies the ascii string to format.
 *		ret_para	Returns a pointer to a list of CEParagraph
 *				structures.
 *		ret_num		Returns the number of structures in 'ret_para'.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpFormatAsciiString formats a string as if it were a
 *		static string - watching for newlines and using them
 *		to force a break in the line.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatAsciiString(pDAS, input_string, ret_handle )
	DtHelpDispAreaStruct *pDAS;
	char		 *input_string;
	TopicHandle	 *ret_handle;
#else
_DtHelpFormatAsciiString(
	DtHelpDispAreaStruct *pDAS,
	char		 *input_string,
	TopicHandle	 *ret_handle)
#endif /* _NO_PROTO */
{
    int      result;
    CEFontSpec fontAttrs;
    VarHandle varHandle;

    /*
     * check the parameters.
     */
    if (input_string == NULL || ret_handle == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * Get the initialized variables
     */
    *ret_handle  = NULL;

    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    _DtHelpCeXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,setlocale(LC_CTYPE,NULL),
				NULL, &(fontAttrs.lang), &(fontAttrs.char_set));

    varHandle = __DtHelpCeSetUpVars(fontAttrs.lang, fontAttrs.char_set);
    if (varHandle == NULL)
      {
        free(fontAttrs.lang);
        free(fontAttrs.char_set);
	return -1;
      }

    result = __DtHelpCeProcessString (pDAS->canvas, varHandle, NULL,
				ScanString, input_string,
				strlen(input_string),
				_CESetStaticFlag (CE_STRING), &fontAttrs);

    if (result != -1)
	result = __DtHelpCeGetParagraphList (varHandle, ret_handle);

    free(fontAttrs.lang);
    free(fontAttrs.char_set);
    free(varHandle);
    return result;

}  /* End _DtHelpFormatAsciiString */

/*****************************************************************************
 * Function:	int _DtHelpFormatAsciiStringDynamic (char *input_string,
 *					CEParagraph **ret_para, int *ret_num)
 *
 * Parameters:
 *		input_string	Specifies the ascii string to format.
 *		ret_para	Returns a pointer to a list of CEParagraph
 *				structures.
 *		ret_num		Returns the number of structures in 'ret_para'.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpFormatAsciiStringDynamic formats a string as if it were
 *		a dynamic string - it uses newline characters to terminate
 *		the current paragraph, not a line.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatAsciiStringDynamic(pDAS, input_string, ret_handle)
	DtHelpDispAreaStruct *pDAS;
	char		 *input_string ;
	TopicHandle	 *ret_handle;
#else
_DtHelpFormatAsciiStringDynamic(
	DtHelpDispAreaStruct *pDAS,
	char		 *input_string,
	TopicHandle	 *ret_handle)
#endif /* _NO_PROTO */
{
    int      result = 0;
    CEFontSpec fontAttrs;
    VarHandle varHandle;

    /*
     * check the parameters.
     */
    if (input_string == NULL || ret_handle == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * Fake the flag and give the string as the input buffer.
     */
    *ret_handle  = NULL;

    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    _DtHelpCeXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,setlocale(LC_CTYPE,NULL),
				NULL, &(fontAttrs.lang), &(fontAttrs.char_set));

    varHandle = __DtHelpCeSetUpVars(fontAttrs.lang, fontAttrs.char_set);
    if (varHandle == NULL)
      {
        free(fontAttrs.lang);
        free(fontAttrs.char_set);
	return -1;
      }

    result = __DtHelpCeProcessString (pDAS->canvas, varHandle, NULL,
				ScanString, input_string,
				strlen(input_string),
				_CESetEolFlag (CE_STRING), &fontAttrs);

    if (result != -1)
	result = __DtHelpCeGetParagraphList (varHandle, ret_handle);

    free(fontAttrs.lang);
    free(fontAttrs.char_set);
    free(varHandle);
    return result;

}  /* End _DtHelpFormatAsciiString */

/******************************************************************************
 * Function:	int _DtHelpFormatTopicTitle (Display *dpy, _XvhVolume volume,
 *				char *filename, int offset,
 *				XmString *ret_title, XmFontList *ret_list,
 *				Boolean *ret_mod)
 *
 * Parameters:
 *		dpy		Specifies the connection to the server.
 *		volume		Specifies the Help Volume the information
 *				is associated with.
 *		filename	Specifies the file containing the Help Topic
 *				desired.
 *		offset		Specifies the offset into 'filename' to
 *				the Help Topic desired.
 *		ret_title	Returns XmString containing the title.
 *		ret_list	Specifies the current font list being used.
 *				Returns a new font list if new character
 *				sets are added to it.
 *		ret_mod		Returns True if fonts were added to the
 *				font list.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpFormatTopicTitle formats a topic title and associates
 *		the correct font with it.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatTopicTitle(pDAS, volume_handle, location_id,
			ret_title, ret_list, ret_mod )
    DtHelpDispAreaStruct *pDAS;
    VolumeHandle	  volume_handle;
    char		 *location_id;
    XmString		 *ret_title;
    XmFontList		 *ret_list;
    Boolean		 *ret_mod;
#else
_DtHelpFormatTopicTitle(
    DtHelpDispAreaStruct *pDAS,
    VolumeHandle	  volume_handle,
    char		 *location_id,
    XmString		 *ret_title,
    XmFontList		 *ret_list,
    Boolean		 *ret_mod )
#endif /* _NO_PROTO */
{
    int		 result = -1;
    void	**titleChunks = NULL;
    _DtHelpCeLockInfo  lockInfo;

    /*
     * Check the parameters
     */
    if (ret_title == NULL || ret_list == NULL || ret_mod == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * lock the volume
     */
    if (_DtHelpCeLockVolume(pDAS->canvas, volume_handle, &lockInfo) != 0)
	return -1;

    /*
     * Get the title and charsets associated with the title segments.
     * The format of the returned information is
     *		[type,charset/fontptr,]type,string/spc
     */
    result = _DtHelpCeGetVolumeFlag(volume_handle);
    if (result == 1)
        result = _DtHelpCeGetSdlTitleChunks(pDAS->canvas, volume_handle,
				location_id, &titleChunks);
    else if (result == 0)
        result = _DtHelpCeGetCcdfTitleChunks(pDAS->canvas,
				(_DtHelpVolume) volume_handle,
				location_id, &titleChunks);
    if (result != -1)
        result = FormatChunksToXmString(pDAS, True, titleChunks,
						ret_title, ret_list, ret_mod);
    _DtHelpCeUnlockVolume(lockInfo);
    return result;

}  /* End _DtHelpFormatTopicTitle */

/******************************************************************************
 * Function:	int _DtHelpFormatVolumeTitle (DtHelpDispAreaStruct *pDAS,
 *				_XvhVolume volume,
 *				XmString *ret_title, XmFontList *ret_list,
 *				Boolean *ret_mod)
 *
 * Parameters:
 *		volume		Specifies the Help Volume the information
 *				is associated with.
 *		ret_title	Returns XmString containing the title.
 *		ret_list	Specifies the current font list being used.
 *				Returns a new font list if new character
 *				sets are added to it.
 *		ret_mod		Returns True if fonts were added to the
 *				font list.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpFormatVolumeTitle formats a volume title and associates
 *		the correct font with it.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatVolumeTitle(pDAS, volume_handle, ret_title, ret_list, ret_mod )
    DtHelpDispAreaStruct *pDAS;
    VolumeHandle	  volume_handle;
    XmString		 *ret_title;
    XmFontList		 *ret_list;
    Boolean		 *ret_mod;
#else
_DtHelpFormatVolumeTitle(
    DtHelpDispAreaStruct *pDAS,
    VolumeHandle	  volume_handle,
    XmString		 *ret_title,
    XmFontList		 *ret_list,
    Boolean		 *ret_mod )
#endif /* _NO_PROTO */
{
    int		 result = -1;
    void	**titleChunks = NULL;
    _DtHelpCeLockInfo lockInfo;

    /*
     * Check the parameters
     */
    if (ret_title == NULL || ret_list == NULL || ret_mod == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * lock the volume
     */
    *ret_title = NULL;
    if (_DtHelpCeLockVolume(pDAS->canvas, volume_handle, &lockInfo) != 0)
	return -1;

    /*
     * Get the title and charsets associated with the volume title.
     * The format of the returned information is
     *		[type,charset/fontptr,]type,string/spc
     */
    result = _DtHelpCeGetVolumeFlag(volume_handle);
    if (result == 1)
        result = _DtHelpCeGetSdlVolTitleChunks(pDAS->canvas, volume_handle,
				&titleChunks);
    else if (result == 0)
        result = _DtHelpCeGetCcdfVolTitleChunks(pDAS->canvas,
				(_DtHelpVolume) volume_handle,
				&titleChunks);
    if (result != -1)
        result = FormatChunksToXmString(pDAS, True, titleChunks,
						ret_title, ret_list, ret_mod);
    _DtHelpCeUnlockVolume(lockInfo);
    return result;

}  /* End _DtHelpFormatVolumeTitle */

/******************************************************************************
 * Function:	int _DtHelpFormatIndexEntries (DtHelpDispAreaStruct *pDAS,
 *				_XvhVolume volume,
 *				XmString *ret_title, XmFontList *ret_list,
 *				Boolean *ret_mod)
 *
 * Parameters:
 *		volume		Specifies the Help Volume the information
 *				is associated with.
 *              ret_cnt         number of valid entries in the array
 *		ret_words	Returns NULL-termintaed array of 
 *                                XmStrings containing the words.
 *		ret_list	Specifies the current font list being used.
 *				Returns a new font list if new character
 *				sets are added to it.
 *		ret_mod		Returns True if fonts were added to the
 *				font list.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpFormatIndexEntries formats index entries and associates
 *		the correct font with it.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatIndexEntries(pDAS, volume_handle, ret_cnt,
			ret_words, ret_list, ret_mod )
    DtHelpDispAreaStruct *pDAS;
    VolumeHandle	  volume_handle;
    int			 *ret_cnt;
    XmString		**ret_words;
    XmFontList		 *ret_list;
    Boolean		 *ret_mod;
#else
_DtHelpFormatIndexEntries(
    DtHelpDispAreaStruct *pDAS,
    VolumeHandle	  volume_handle,
    int			 *ret_cnt,
    XmString		**ret_words,
    XmFontList		 *ret_list,
    Boolean		 *ret_mod )
#endif /* _NO_PROTO */
{
    int		  i;
    int		  result = -1;
    void	 *titleChunks[4];
    char	 *charSet;
    char	**keyWords;
    Boolean       myMod = False;

    /*
     * Check the parameters
     */
    if (ret_words == NULL || ret_list == NULL || ret_mod == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    *ret_mod = False;

    charSet = _DtHelpCeGetVolumeLocale(volume_handle);
    if (charSet != NULL)
      {
	*ret_cnt = _DtHelpCeGetKeywordList(volume_handle, &keyWords);
	if (*ret_cnt > 0)
	  {

	    *ret_words = (XmString *) malloc (sizeof(XmString) * (*ret_cnt+1));
	    if (*ret_words == NULL)
		return -1;

            /* the order of the string ptr and charset ptr in the titleChunks
               is determined by the order of the if(..) tests on the
               flags and subsequent processing in FormatChunksToXmString() */
	    titleChunks[0] = (void *) (DT_HELP_CE_STRING | DT_HELP_CE_CHARSET);
	    titleChunks[1] = (void *) charSet;
	    titleChunks[3] = (void *) DT_HELP_CE_END;
	    for (i = 0, result = 0; result == 0 && i < *ret_cnt; i++)
	      {
		titleChunks[2] = (void *) keyWords[i];
		result = FormatChunksToXmString(pDAS, False, titleChunks,
				&(*ret_words)[i], ret_list, &myMod);
		if (myMod == True)
		    *ret_mod = True;
	      }
            (*ret_words)[i] = NULL;
	  }
	free(charSet);
      }

    return result;

}  /* End _DtHelpFormatIndexEntries */

/******************************************************************************
 * Function:	int _DtHelpFormatTopic (
 *				DtHelpDispAreaStruct *pDAS,
 *				VolumeHandle volume,
 *				char *id_string,
 *				CEBoolean look_at_id,
 *				TopicHandle *ret_handle)
 *
 * Parameters:
 *		pDAS		Specifies the display area to use.
 *		volume		Specifies the Help Volume the information
 *				is associated with.
 *		id_string	Specifies the location id for the topic.
 *				This location id can be contained in a
 *				topic.
 *		look_at_id	Specifies whether or not to start the
 *				display of the topic at the location id.
 *		ret_handle	Returns a handle to the topic information
 *				including the number of paragraphs and the
 *				id match segment.
 *
 * Returns:	0	if successful.
 *		-1	if errors.
 *		-2	if the id was not found.
 *		-3	if unable to format the topic.
 *
 * errno Values:
 *
 * Purpose:	_DtHelpParseTopic accesses and parses Help topics.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatTopic(pDAS, volume, id_string, look_at_id, ret_handle)
	DtHelpDispAreaStruct *pDAS;
	VolumeHandle	  volume;
	char		 *id_string;
	CEBoolean	  look_at_id;
	TopicHandle	 *ret_handle;
#else
_DtHelpFormatTopic(
	DtHelpDispAreaStruct *pDAS,
	VolumeHandle	  volume,
	char		 *id_string,
	CEBoolean	  look_at_id,
	TopicHandle	 *ret_handle )
#endif /* _NO_PROTO */
{
    char  *filename = NULL;
    int    offset;
    int    result = -2;
    _DtHelpCeLockInfo   lockInfo;

    if (_DtHelpCeLockVolume(pDAS->canvas, volume, &lockInfo) != 0)
	return -1;

    if (_DtHelpCeFindId(volume,id_string,lockInfo.fd,&filename,&offset)==True)
      {
	if (look_at_id == False)
	    id_string = NULL;

	result = _DtHelpCeGetVolumeFlag(volume);
        if (result == 1)
            result = _DtHelpCeParseSdlTopic(pDAS->canvas, volume,
						lockInfo.fd, offset,
						id_string, True, ret_handle);
        else if (result == 0)
            result = _DtHelpCeFrmtCcdfTopic(pDAS->canvas,
						(_DtHelpVolume) volume,
						filename, offset,
						id_string, ret_handle);
	if (result != 0)
	    result = -3;

	if (filename != NULL)
	    free(filename);
      }

    _DtHelpCeUnlockVolume(lockInfo);

    return result;

}  /* End _DtHelpFormatTopic */
