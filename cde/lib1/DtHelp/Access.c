/* $XConsortium: Access.c /main/cde1_maint/1 1995/07/17 17:16:12 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Access.c
 **
 **   Project:     Run Time Project File Access
 **
 **   Description: This body of code handles the access routines for the
 **                Display Area.
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
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>


extern	int	errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "CanvasError.h"
#include "Access.h"
#include "bufioI.h"
#include "AccessP.h"
#include "AccessI.h"
#include "AccessCCDFI.h"
#include "AccessSDLP.h"
#include "AccessSDLI.h"

#include "CEUtilI.h"
#include "CCDFUtilI.h"
#include "StringFuncsI.h"

#ifdef NLS16
#endif

/********    Private Defines      ********/
#define	LIST_INCREMENT	10
#define	BUFF_SIZE	256
/********    End Private Defines  ********/

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	int	  GetVolumeKeywords ();
static	int	  VolumeLoad ();
static	int	  VolumeUnload ();
#else
static	int	  GetVolumeKeywords (
			_DtHelpVolume     vol, 
			char	***retKeywords);
static	int	  VolumeLoad (
			CanvasHandle	canvas_handle,
			char        *volFile, 
			_DtHelpVolume  *retVol);
static	int	  VolumeUnload (
			CanvasHandle	canvas_handle,
			_DtHelpVolume vol);
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    Private Macro Declarations        ********/
/********    End Private Macro Declarations    ********/

/******************************************************************************
*
* Private variables used within this file.
*
*******************************************************************************/
static _DtHelpVolume volChain = NULL;	/* Pointer to the head of the chain */
					/* of all the open volumes. */
static const char *Slash  = "/";
static const char *Period = ".";

/******************************************************************************
 *                             Private Functions
 ******************************************************************************/
/******************************************************************************
 * Function:	CheckVolList (_DtHelpVolume vol, _DtHelpVolume *ret_prev)
 *
 * Parameters:	vol		Specifies the volume to search for.
 *		ret_prev	Returns the volume whose nextVol element
 *				points to 'vol' if non NULL.
 *
 * Returns:	0 if successful, -1 if failure.
 *
 * errno Values:
 *
 * Purpose:	To check for the existance of a volume.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CheckVolList (vol, ret_prev)
    _DtHelpVolume	 vol;
    _DtHelpVolume	*ret_prev;
#else
CheckVolList (
    _DtHelpVolume	 vol,
    _DtHelpVolume	*ret_prev )
#endif /* _NO_PROTO */
{
    _DtHelpVolume	myVol = volChain;
    _DtHelpVolume	prevVol = NULL;

    while (myVol != NULL && myVol != vol)
      {
	prevVol = myVol;
	myVol = myVol->nextVol;
      }

    if (ret_prev)
	*ret_prev = prevVol;

    if (myVol != vol)
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int VolumeLoad (char *volFile, _DtHelpVolume *retVol);
 *
 * Parameters:	volFile		Specifies the name of the Help Volume file
 *				to load.
 *
 *		retVol		Returns the handle to the loaded volume.
 *
 * Return Value:	Returns 0 if successful,
 *			-1 if an error occurred.
 *
 * errno Values:	CEErrorMalloc
 *			CEErrorIllegalDatabaseFile
 *
 * Purpose:	This function must be called to load a Help Volume file
 *		before any of the information in the volume can be
 *		accessed. 
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
VolumeLoad (canvas_handle, volFile, retVol)
    CanvasHandle canvas_handle;
    char	*volFile;
    _DtHelpVolume  *retVol;

#else
VolumeLoad (
    CanvasHandle canvas_handle,
    char        *volFile, 
    _DtHelpVolume  *retVol)
#endif /* _NO_PROTO */
{
    /* Allocate the volume structure and initialize it. */
    *retVol = (_DtHelpVolume) malloc (sizeof (struct _DtHelpVolumeRec));
    if (*retVol)
      {
        (*retVol)->sdl_flag      = False;
        (*retVol)->volFile       = volFile;
        (*retVol)->keywords      = NULL;
        (*retVol)->keywordTopics = NULL;
        (*retVol)->openCount     = 1;
        (*retVol)->nextVol       = NULL;
        (*retVol)->vols.ccdf_vol = NULL;

	if (_DtHelpCeOpenSdlVolume (canvas_handle, (*retVol)) == 0)
	    return 0;
	else if (_DtHelpCeOpenCcdfVolume(*retVol) == 0)
	    return 0;

	/*
	 * Set the global error
	 */
	errno = CEErrorIllegalDatabaseFile;

	/*
	 * error on loading the database.
	 */
	free ((char *) ((*retVol)->volFile));
	free ((char *) (*retVol));
	*retVol = NULL;
      }
    else
	errno = CEErrorMalloc;

    return -1;

} /* End VolumeLoad */

/*******************************************************************************
 * Function:	int VolumeUnload (_DtHelpVolume vol);
 *
 * Parameters:	vol	Specifies the loaded volume.
 *
 * Return Value:	0 if successful, -1 if a failure occurs
 *
 * errno Values:	None
 *
 * Purpose:	When the volume is no longer needed, it should be unloaded
 *		with this call.  Unloading it frees the memory (which means
 *		any handles on the volume become invalid.)
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
VolumeUnload (canvas, vol)
     CanvasHandle  canvas;
     _DtHelpVolume vol;
#else
VolumeUnload (
     CanvasHandle  canvas,
     _DtHelpVolume vol)
#endif /* _NO_PROTO */
{
    char	***topicList;
    
    if (vol != NULL)
      {

	if (vol->sdl_flag == True)
	    _DtHelpCeCloseSdlVolume(canvas, (VolumeHandle) vol);
	else
	    _DtHelpCeCloseCcdfVolume(vol);

	if (vol->volFile != NULL)
	    free (vol->volFile);

	if (vol->keywords != NULL)
	    _DtHelpCeFreeStringArray (vol->keywords);

	if (vol->keywordTopics != NULL)
	  {
	    for (topicList = vol->keywordTopics;
					*topicList != NULL; topicList++)
		_DtHelpCeFreeStringArray (*topicList);

	    free (vol->keywordTopics);
          }

	free (vol);
      }

    return (0);
}

/*******************************************************************************
 * Function:	int RereadVolume (_DtHelpVolume vol);
 *
 * Parameters:	vol	Specifies the loaded volume.
 *
 * Return Value:	0 if successful, -1 if a failure occurs
 *
 * errno Values:	None
 *
 * Purpose:	When the volume is no longer needed, it should be unloaded
 *		with this call.  Unloading it frees the memory (which means
 *		any handles on the volume become invalid.)
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
RereadVolume (canvas, vol)
     CanvasHandle  canvas;
     _DtHelpVolume vol;
#else
RereadVolume (
     CanvasHandle  canvas,
     _DtHelpVolume vol)
#endif /* _NO_PROTO */
{
    int            result;
    char	***topicList;
    
    if (vol->keywords != NULL)
      {
        _DtHelpCeFreeStringArray (vol->keywords);
        vol->keywords = NULL;
      }

    if (vol->keywordTopics != NULL)
      {
        for (topicList = vol->keywordTopics; *topicList != NULL; topicList++)
	    _DtHelpCeFreeStringArray (*topicList);

        free (vol->keywordTopics);
	vol->keywordTopics = NULL;
      }

    if (vol->sdl_flag == False)
        result = _DtHelpCeRereadCcdfVolume(vol);
    else
        result = _DtHelpCeRereadSdlVolume(canvas, vol);

    return (result);
}

/******************************************************************************
 * Function:	static int GetKeywordTopics (_DtHelpVolume vol, char *keyword,
 *				       char ***topics);
 *
 * Parameters:	vol		Specifies the loaded volume
 *		keyword		Specifies the keyword whose location is desired.
 *		topics		Returns a NULL-terminated string array of the
 *				list of topics which contain the keyword.
 *				This array is NOT owned by the caller and
 *				should only be read or copied.
 *
 * Return Value:	0 if successful, -1 if a failure occurs
 *
 * errno Values:	CEErrorNoKeywordList
 *					Specifies that the volume does not
 *					have a keyword list.
 *			CEErrorIllegalKeyword
 *					Specifies that 'keyword' was not
 *					found.
 *			CEErrorMalloc
 *			CEErrorIllegalDatabaseFile
 *					Specifies that the keyword file is
 *					invalid or corrupt.
 *			CEErrorMissingKeywordsRes
 *					Specifies that the keyword file does
 *					not contain the 'Keywords/keywords'
 *					resource or the resource is NULL
 *
 *
 * Purpose:	Find which topic contains a specified locationId.
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
GetKeywordTopics (vol, keyword, retTopics)
    _DtHelpVolume    vol;
    char	  *keyword;
    char	***retTopics;
#else
GetKeywordTopics (
   _DtHelpVolume     vol, 
   char		  *keyword, 
   char		***retTopics)
#endif /* _NO_PROTO */
{
    char	**keywords;
    char	**nextKey;
    int index;

    *retTopics = NULL;

    /* Get the list of keywords. */
    if (GetVolumeKeywords (vol, &keywords) != 0)
	return -1;

    if (keywords == NULL || vol->keywordTopics == NULL)
      {
	errno = CEErrorNoKeywordList;
	return -1;
      }

    /* Search the list of keywords for the current one. */
    nextKey = keywords;
    while (*nextKey != NULL && strcmp (*nextKey, keyword))
	nextKey++;

    if (*nextKey == NULL)
      {
	errno = CEErrorIllegalKeyword;
	return -1;
      }

    index = nextKey - keywords;
    *retTopics = *(vol->keywordTopics + index);

    return (0);
}

/******************************************************************************
 * Function:	static int GetVolumeKeywords(_DtHelpVolume vol,char ***keywords);
 *
 * Parameters:	vol		Specifies the volume.
 *		keywords	Returns a NULL-terminated string array
 *				containing the sorted list of keywords in the
 *				volume.  This array is NOT owned by the caller
 *				and should only be read or copied.
 *
 * Return Value:	0 if successful, -1 if a failure occurs
 *
 * errno Values:	CEErrorMalloc
 *			CEErrorIllegalDatabaseFile
 *					Specifies that the keyword file is
 *					invalid or corrupt.
 *			CEErrorMissingKeywordsRes
 *					Specifies that the keyword file does
 *					not contain the 'Keywords/keywords'
 *					resource or the resource is NULL
 *
 * Purpose:	Get the list of keywords defined in a volume.
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
GetVolumeKeywords (vol, retKeywords)
     _DtHelpVolume     vol;
     char	***retKeywords;
#else
GetVolumeKeywords (
    _DtHelpVolume     vol, 
    char	***retKeywords)
#endif /* _NO_PROTO */
{
    int   result;
    _DtHelpCeLockInfo lockInfo;

    /* Keywords aren't loaded until they are needed, so see if they have
       been loaded yet. */
    if (vol->keywords == NULL)
      {
        /*
         * What type of volume is it?
         */
	if (_DtHelpCeLockVolume(NULL, vol, &lockInfo) != 0)
	    return -1;

        if (vol->sdl_flag == False)
            result = _DtHelpCeGetCcdfKeywordList(vol);
	else
            result = _DtHelpCeGetSdlKeywordList(vol);

	_DtHelpCeUnlockVolume(lockInfo);

	if (result != 0)
	    return -1;
      }

    /* All of the keyword processing is done when they are loaded. */
     *retKeywords = vol->keywords;

    if (*retKeywords == NULL)
	return (-1);

    return (0);
}

/*****************************************************************************
 * Function: GetTopicTitleAndAbbrev (
 *
 * Parameters:
 *
 * Memory own by caller:
 *              ret_name
 *              ret_abrrev
 *
 * Returns:     0 if successful, -2 if didn't find the id,
 *		-3 if couldn't format the topic,
 *              otherwise -1.
 *
 * Purpose:     Find the title and abbreviated title of a topic.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
GetTopicTitleAndAbbrev (canvas, volume, id, ret_name, ret_abbrev)
        CanvasHandle     canvas;
        _DtHelpVolume    volume;
        char             *id;
        char            **ret_name;
        char            **ret_abbrev;
#else
GetTopicTitleAndAbbrev (
        CanvasHandle     canvas,
        _DtHelpVolume    volume,
        char             *id,
        char            **ret_name,
        char            **ret_abbrev )
#endif /* _NO_PROTO */
{
    int         result = 0;
    int         offset;
    char        buffer[BUFF_SIZE];
    char       *bufPtr;
    char       *filename = NULL;
    BufFilePtr  file;
    _DtHelpCeLockInfo lockInfo;

    if (_DtHelpCeLockVolume(canvas, volume, &lockInfo) != 0)
	return -1;

    if (_DtHelpCeFindId(volume, id, lockInfo.fd, &filename, &offset) != True)
        result = -2;

    /*
     * What type of volume is it?
     */
    if (result == 0)
      {
        if (volume->sdl_flag == False)
          {
            result = _DtHelpCeFileOpenAndSeek(filename,offset,-1,&file,NULL);
            if (result == 0)
              {
		result = -1;
                if (_DtHelpCeReadBuf (file, buffer, BUFF_SIZE) != -1)
                  {
		    result = 0;
                    bufPtr = buffer;
                    if (_DtHelpCeGetCcdfTopicAbbrev (NULL, file,
                                buffer, &bufPtr, BUFF_SIZE, MB_CUR_MAX,
                                        ret_name, NULL, ret_abbrev) != 0)
		        result = -3;
                  }
                _DtHelpCeBufFileClose(file, True);
              }
          }
        else if (_DtHelpCeFrmtSDLTitleToAscii(canvas, volume, offset,
                                                ret_name, ret_abbrev) != 0)
            result = -3;
      }

    if (filename != NULL)
        free(filename);

    _DtHelpCeUnlockVolume(lockInfo);

    return result;
}

/*****************************************************************************
 * Function: static int FileOpenRtnFd (char *name, int *ret_fd)
 *
 * Parameters:	name		Specifies the file to open.
 *		ret_fd		Returns the fd of the opened file.
 *
 * Returns:	 0 if required uncompress.
 *		   file descriptor to remove the file from the system.
 *		 1 if no uncompression required.
 *		-1 if a failure occurs
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorFileSeek
 *					Specifies the seek offset was invalid.
 *
 * Purpose:	Find out if a file is compressed and uncompress it if it is.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
FileOpenRtnFd (name, ret_fd)
    char	*name;
    int		*ret_fd;
#else
FileOpenRtnFd (
    char	*name,
    int		*ret_fd )
#endif /* _NO_PROTO */
{
    char *inFile = NULL;
    char  tmpName[MAXPATHLEN + 1];
    int   result = 1;

    /*
     * check to see if the file exists in uncompressed form
     */
    *ret_fd = open(name, O_RDONLY);
    if (*ret_fd == -1)
      {
	/*
	 * get a temporary name
	 */
	(void) tmpnam (tmpName);

	/*
	 * malloc memory for the dot Z file name.
	 */
	inFile = (char *) malloc (strlen (name) + 3);
	if (inFile != NULL)
	  {
	    /*
	     * make the dot Z file
	     */
	    strcpy (inFile, name);
	    strcat (inFile, ".Z");

	    /*
	     * do the uncompress
	     */
	    result = _DtHelpCeUncompressFile (inFile, tmpName);
	    free (inFile);

	    if (result != 0)
	      {
		errno = ENOENT;
		return -1;
	      }

	    /*
	     * now open the uncompressed file
	     */
	    *ret_fd = open(tmpName, O_RDONLY);
	    if (*ret_fd == -1)
		result = -1;
	    else
		unlink(tmpName);
	  }
	else
	  {
	    errno = CEErrorMalloc;
	    return -1;
	  }
      }

    return result;

} /* End FileOpenRtnFd */

/******************************************************************************
 *                          Semi-Public Functions
 ******************************************************************************/
/*****************************************************************************
 * Function: char *_DtHelpCeExpandPathname (char *spec, char *filename, char *type,
 *		char *suffix, char *lang, _DtSubstitutionRec *subs, int num)
 *
 * Parameters:
 *		spec		Specifies a string with substitution
 *					characters.
 *				containing the character set if found.
 *		filename	Specifies the string to substitute for %N.
 *		type		Specifies the string to substitute for %T.
 *		suffix		Specifies the string to substitute for %S.
 *		lang		Specifies the string to substitute for %L.
 *		subs		Specifies the application own specific
 *					substitutions.
 *		num		Specifies the number of substitution pairs
 *					in 'subs'.
 *
 * Memory own by caller:
 *		returned pointer
 *
 * Returns:	The expanded filename if successful. NULL if errors.
 *
 * Purpose:	Expand a string with %<char> substitution values.
 *		Default substitutions are:
 *			%N	replaced with 'filename'.
 *			%T	replaced with 'type'
 *			%S	replaced with 'suffix'
 *			%L	replaced with 'lang'
 *
 *			%l	replaced with the language sub part of 'lang'.
 *			%t	replaced with the territory sub part of 'lang'.
 *			%c	replaced with the code set sub part of 'lang'.
 *		Other substitutions can be done via the 'subs' parameter.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeExpandPathname (spec, filename, type, suffix, lang, subs, num)
    char  *spec;
    char  *filename;
    char  *type;
    char  *suffix;
    char  *lang;
    _DtSubstitutionRec *subs;
    int    num;
#else
_DtHelpCeExpandPathname (
    char  *spec,
    char  *filename,
    char  *type,
    char  *suffix,
    char  *lang,
    _DtSubstitutionRec *subs,
    int    num )
#endif /* _NO_PROTO */
{
    int    i;
    int    len = 1;
    char  *ptr;
    char  *subString;
    char  *partLang;
    char  *partTer;
    char  *partCodeSet;
    char   pathName [MAXPATHLEN + 5];
    CEBoolean   previousSlash = False;
#define	MY_NUM 7
    _DtSubstitutionRec mySubs [MY_NUM];

    if (spec == NULL || *spec == NULL)
      {
	errno = EINVAL;
	return NULL;
      }

    /*
     * fill in the language sub parts
     */
    if (_DtHelpCeGetLangSubParts (lang, &partLang, &partTer, &partCodeSet))
	return NULL;

    mySubs[0].match = 'N';
    mySubs[0].substitution = filename;
    mySubs[1].match = 'T';
    mySubs[1].substitution = type;
    mySubs[2].match = 'S';
    mySubs[2].substitution = suffix;
    mySubs[3].match = 'L';
    mySubs[3].substitution = lang;

    mySubs[4].match = 'l';
    mySubs[4].substitution = partLang;
    mySubs[5].match = 't';
    mySubs[5].substitution = partTer;
    mySubs[6].match = 's';
    mySubs[6].substitution = partCodeSet;

    ptr = pathName;
    while (*spec)
      {
	len = 1;
	if (MB_CUR_MAX != 1)
	    len = mblen (spec, MB_CUR_MAX);

	if (len == 1 && *spec == '/')
	  {
	    if (previousSlash)
		spec++;
	    else
	      {
		previousSlash = True;
		*ptr++ = *spec++;
	      }
	  }
	else if (len == 1 && *spec == '%')
	  {
	    spec++;
	    switch (*spec)
	      {
		case '\0':
			*ptr++ = '%';
			break;

		case '/':
			if (!previousSlash)
			  {
			     previousSlash = True;
			     *ptr++ = *spec;
			  }
			spec++;
			break;

		default:
			i = 0;
			while (i < MY_NUM && mySubs && mySubs[i].match != *spec)
			    i++;

			if (i < MY_NUM)
			  {
			    if (mySubs[i].substitution != NULL)
			      {
			        subString = mySubs[i].substitution;
				if (((int)(ptr - pathName + strlen(subString)))
						> MAXPATHLEN)
				  {
				    errno = CEErrorExceedMaxSize;
				    return NULL;
				  }
			        while (subString && *subString)
				    *ptr++ = *subString++;
			      }
			  }
			else
			  {
			    i = 0;
			    while (i < num && subs && subs[i].match != *spec)
				i++;
			    /*
			     * If the substitution character is not found
			     * include the character onto the final string.
			     */
			    if (i >= num)
			        *ptr++ = *spec;
			    else if (subs[i].substitution != NULL)
			      {
			        subString = subs[i].substitution;
				if (((int)(ptr - pathName + strlen(subString)))
						> MAXPATHLEN)
				  {
				    errno = CEErrorExceedMaxSize;
				    return NULL;
				  }
			        while (subString && *subString)
				    *ptr++ = *subString++;
			      }
			  }
			spec++;
			previousSlash = False;
			break;
	      }
	  }
	else
	  {
	    previousSlash = False;
	    do 
	      {
	        *ptr++ = *spec++;
		len--;
	      } while (len > 0);
	  }

	if (ptr - pathName > MAXPATHLEN)
	  {
	    errno = CEErrorExceedMaxSize;
	    return NULL;
	  }
      }

    if (partLang)
	free (partLang);
    if (partTer)
	free (partTer);
    if (partCodeSet)
	free (partCodeSet);

    *ptr = '\0';
    ptr = strdup (pathName);
    if (ptr == NULL)
	errno = CEErrorMalloc;

    return ptr;
}

/*****************************************************************************
 * Function: char *_DtHelpCeGetLangSubParts (char *lang, char **subLang,
 *					char **subTer, char **subCodeSet)
 *
 * Parameters:
 *		lang		Specifies the language string.
 *		subLang		Returns the language sub part of 'lang'
 *					or NULL.
 *		subTer		Returns the territory sub part of 'lang'
 *					or NULL.
 *		subCodeSet	Returns the code set sub part of 'lang'
 *					or NULL.
 *
 * Memory own by caller:
 *		subLang
 *		subTer
 *		subCodeSet
 *
 * errno Values:
 *		EINVAL
 *		CEErrorMalloc
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * Purpose:	Break a %l_%t.%c language specification into its sub parts.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetLangSubParts (lang, subLang, subTer, subCodeSet)
    char   *lang;
    char  **subLang;
    char  **subTer;
    char  **subCodeSet;
#else
_DtHelpCeGetLangSubParts (
    char   *lang,
    char  **subLang,
    char  **subTer,
    char  **subCodeSet )
#endif /* _NO_PROTO */
{
    int   len;
    char *ptr;
    char *sLang = NULL;
    char *sTer  = NULL;
    char *sCode = NULL;

    if (subLang == NULL || subTer == NULL || subCodeSet == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    if (lang != NULL && *lang != '\0')
      {
	/*
	 * look for lang_ter
	 */
	_DtHelpCeStrchr (lang, "_", MB_CUR_MAX, &ptr);
	if (ptr)
	  {
	    /*
	     * do we want this string?
	     */
	    if (subLang != NULL)
	      {
		len = ptr - lang;
		sLang = (char *) malloc (len + 1);
		if (sLang != NULL)
		  {
		    strncpy (sLang, lang, len);
		    sLang[len] = '\0';
		  }
		else
		  {
		    errno = CEErrorMalloc;
		    return -1;
		  }
	      }
	    /*
	     * just mark that the lang part was found
	     */
	    else
		sLang = lang;
    
	    lang = ptr + 1;
	  }
    
	/*
	 * look for lang.codeset
	 */
	_DtHelpCeStrchr (lang, Period, MB_CUR_MAX, &ptr);
	if (ptr)
	  {
	    len = ptr - lang;
    
	    /*
	     * if it was in the form lang_ter.codeset, sLang will non-null
	     */
	    if (sLang != NULL)
	      {
		/*
		 * do we want to save the territory?
		 */
		if (subTer != NULL)
		  {
		    sTer = (char *) malloc (len + 1);
		    if (sTer != NULL)
		      {
			strncpy (sTer, lang, len);
			sTer[len] = '\0';
		      }
		    else
		      {
			errno = CEErrorMalloc;
			return -1;
		      }
		  }
		/*
		 * don't wan to save, but make sure we mark the territory
		 * as being found (non-null).
		 */
		else
		    sTer = lang;
	      }
	    /*
	     * the lang was in the form lang.codeset.
	     * now check to see if want to save the lang portion.
	     */
	    else if (subLang != NULL)
	      {
		sLang = (char *) malloc (len + 1);
		if (sLang != NULL)
		  {
		    strncpy (sLang, lang, len);
		    sLang[len] = '\0';
		  }
		else
		  {
		    errno = CEErrorMalloc;
		    return -1;
		  }
	      }
	    /*
	     * didn't want to save the lang portion, but mark as found.
	     */
	    else
		sLang = lang;
	  }
    
       /*
	* currently pointing at the dot?
	*/
       if (ptr && *ptr == '.')
	 {
	    /*
	     * yes save the code set
	     */
	    ptr++;
	    if (subCodeSet != NULL)
	      {
		sCode = strdup (ptr);
		if (sCode == NULL)
		  {
		    errno = CEErrorMalloc;
		    return -1;
		  }
	      }
	    /*
	     * don't save the code set, but make sure we mark as found
	     */
	    else
		sCode = ptr;
	 }
       /*
	* didn't find a code set, so save the current info.
	* If we haven't already processed a lang portion, save as the
	* lang.
	*/
       else if (sLang == NULL)
	 {
	    if (subLang != NULL)
	      {
		sLang = strdup (lang);
		if (sLang == NULL)
		  {
		    errno = CEErrorMalloc;
		    return -1;
		  }
	      }
	    else
		sLang = lang;
	 }
       /*
	* otherwise this is the territory of the language. Save if desired
	*/
       else if (subTer != NULL)
	 {
	    sTer = strdup (lang);
	    if (sTer == NULL)
	      {
		errno = CEErrorMalloc;
		return -1;
	      }
	  }
      }

    if (subLang)
	*subLang = sLang;
    if (subTer)
	*subTer = sTer;
    if (subCodeSet)
	*subCodeSet = sCode;

    return 0;
}

/*****************************************************************************
 * Function: int _DtHelpCeGetUncompressedFileName (char *name, char **ret_name)
 *
 * Parameters:	name		Specifies the file to open.
 *		ret_name 	Returns the name of the uncompressed file.
 *				This memory must be freed by the caller.
 *
 * Returns:	 0 if required uncompress. ret_name will contain the
 *		   name of the uncompressed file. The caller is required
 *		   to free the memory.
 *		 1 if no uncompression required. ret_name points to name.
 *		-1 if a failure occurs
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorFileSeek
 *					Specifies the seek offset was invalid.
 *
 * Purpose:	Find out if a file is compressed and uncompress it if it is.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetUncompressedFileName (name, ret_name)
	char		 *name;
	char		**ret_name;
#else
_DtHelpCeGetUncompressedFileName (
	char		 *name,
	char		**ret_name )
#endif /* _NO_PROTO */
{
    char *inFile = NULL;
    char  tmpName[MAXPATHLEN + 1];
    int   result = 1;

    /*
     * check to see if the file exists in uncompressed form
     */
    *ret_name = name;
    if (access (name, F_OK) == -1)
      {
	/*
	 * get a temporary name
	 */
	(void) tmpnam (tmpName);

	/*
	 * malloc memory for the dot Z file name.
	 */
	inFile = (char *) malloc (strlen (name) + 3);
	if (inFile != NULL)
	  {
	    /*
	     * make the dot Z file
	     */
	    strcpy (inFile, name);
	    strcat (inFile, ".Z");

	    /*
	     * do the uncompress
	     */
	    result = _DtHelpCeUncompressFile (inFile, tmpName);
	    free (inFile);

	    if (result != 0)
	      {
		errno = ENOENT;
		return -1;
	      }

	    *ret_name = strdup (tmpName);
	    if (*ret_name == NULL)
	      {
		errno = CEErrorMalloc;
		return -1;
	      }
	  }
	else
	  {
	    errno = CEErrorMalloc;
	    return -1;
	  }
      }

    return result;
}

/******************************************************************************
 * Function:	int _DtHelpCeCompressPathname (char *basePath)
 *
 * Parameters:	basePath	Specifies the path for the file possibily
 *				containing /./, //, and /../.
 *
 * Return Value:	0 for success, -1 for failure.
 *			The number of bytes in basePath will be less than or
 *			equal to the number of bytes in basePath when passed
 *			in.
 *
 * errno Values:	EINVAL
 *
 * Purpose:	This function compresses directory changes found
 *		in a file name path.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeCompressPathname (basePath)
    char	*basePath;
#else
_DtHelpCeCompressPathname ( char	*basePath )
#endif /* _NO_PROTO */
{
    int    len;
    int    result;
    short  char1;
    short  char2;
    short  char3;
    char  *ptr     = basePath;
    char  *prevPtr = NULL;

    if (basePath == NULL || *basePath != '/')
      {
	errno = EINVAL;
	return -1;
      }

    do
      {
	/*
	 * for multi-byte environments, check how far single bytes extend.
	 */
	char1 = False;
	char2 = False;
	char3 = False;
	if (MB_CUR_MAX == 1 || mblen (&ptr[1], MB_CUR_MAX) == 1)
	  {
	    char1 = True;
	    if (MB_CUR_MAX == 1 || mblen (&ptr[2], MB_CUR_MAX) == 1)
	      {
		char2 = True;
		if (MB_CUR_MAX == 1 || mblen (&ptr[3], MB_CUR_MAX) == 1)
		    char3 = True;
	      }
	  }

	/*
	 * check for //
	 */
	if (char1 == True && ptr[1] == '/')
	    strcpy (ptr, (ptr + 1));

	/*
	 * check for /./
	 */
	else if (char2 == True && ptr[1] == '.' && ptr[2] == '/')
	    strcpy (ptr, (ptr + 2));

	/*
	 * check for /../
	 */
	else if (char3 == True && strncmp (&ptr[1], "../", 3) == 0)
	  {
	    /*
	     * if at the top of the path, just ignore the extra
	     * directory change.
	     */
	    if (prevPtr == NULL)
	        strcpy (ptr, (ptr + 3));
	    else
	      {
		/*
		 * compress the /../
		 */
		strcpy (prevPtr, (ptr + 3));

		/*
		 * reset the current pointer
		 */
		ptr    = prevPtr;

		/*
		 * find the previous slash
		 */
		*ptr   = '\0';
		result = _DtHelpCeStrrchr(basePath,Slash,MB_CUR_MAX,&prevPtr);
		if (result == -1)
		    return -1;

		/*
		 * if there is no previous slash, set the pointer to
		 * indicate that we're at the top of the path (NULL).
		 */
		if (result != 0)
		    prevPtr = NULL;

		/*
		 * restore the slash (or null byte)
		 */
		*ptr = '/';
	      }
	  }
	else
	  {
	    /*
	     * remember this slash for /../ directory changes
	     */
	    prevPtr = ptr;

	    /*
	     * skip this slash, and find the next one.
	     */
	    ptr++;
	    result = _DtHelpCeStrcspn (ptr, "/", MB_CUR_MAX, &len);

	    /*
	     * if we run into invalid data, error
	     */
	    if (result == -1)
		return -1;

	    ptr += len;
	  }

      } while (*ptr != '\0');

    return (0);
}

/******************************************************************************
 * Function:    char *_DtHelpCeTracePathName (char *path)
 *
 * Parameters:
 *		 path	Specifies the a path to trace and compress
 *
 * Return Value:	The new string if successful, NULL otherwise.
 *			The new string is owned by the caller and contains
 *			an absolute pathname.
 *
 * errno Values:        EINVAL                  Illegal parameter specified.
 *                      getcwd(2)               errno set via a getcwd call.
 *                      readlink(2)             errno set via a readlink call.
 *                      DtErrorMalloc
 *                      DtErrorExceedMaxSize   The new path will exceed
 *                                              max_size.
 *                      DtErrorIllegalPath     The compression will required
 *                                              the path to change to a parent
 *                                              directory beyond the beginning
 *                                              of basePath.
 *
 * Purpose:     This function is called to trace the path of a file.
 *		It can contain symbolic links, //, /./, and /../.
 *
 ******************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeTracePathName (path)
    char        *path;
#else
_DtHelpCeTracePathName (char *path)
#endif /* _NO_PROTO */
{
    int     result;
    int     len;
    char    c;
    char   *ptr;
    char   *prev;
    char    newPath  [2 * MAXPATHLEN + 2];
    char    linkPath [MAXPATHLEN + 2];
    char    tempPath [MAXPATHLEN + 2];

    if (path == NULL || *path == '\0')
      {
	errno = EINVAL;
	return NULL;
      }

    /*
     * initialize the new path
     */
    newPath[0] = '\0';

    /*
     * if the path passed in does not start with a slash,
     * get the current working directory and append the path to it.
     */
    if ((MB_CUR_MAX == 1 || mblen(path, MB_CUR_MAX) == 1) && *path != '/')
      {
	if (getcwd (newPath, MAXPATHLEN) == NULL)
	    return NULL;

	strcat (newPath, "/");
      }

    /*
     * put the path in the working path buffer (or append it to
     * the current working directory).
     */
    strcat (newPath, path);

    /*
     * Compress out the slashes and directory changes.
     */
    if (_DtHelpCeCompressPathname (newPath) != 0)
	return NULL;

    ptr = newPath;
    do
      {
	/*
	 * point to the first character of the next directory
	 */
	prev = ptr + 1;

	/*
	 * get the next slash after that
	 */
	result = _DtHelpCeStrcspn (prev, "/", MB_CUR_MAX, &len);
	if (result == -1)
	    return NULL;

	/*
	 * Found either a slash or a null byte.
	 * place the string terminator at this point
	 */
	ptr  = prev + len;
	c    = *ptr;
	*ptr = '\0';

	/*
	 * find out if this path is a symbolic link
	 */
	result = readlink (newPath, linkPath, MAXPATHLEN);

	/*
	 * replace the slash (or null byte).
	 */
	*ptr = c;

	/*
	 * check for the result of the readlink call
	 */
	if (result == -1)
	  {
	    /*
	     * if this was NOT a symbolic link, errno should be EINVAL
	     */
	    if (errno != EINVAL)
		return NULL;
	  }
	else
	  {
	    /*
	     * put the null byte on the end of the symbolic link string.
	     */
	    linkPath [result] = '\0';

	    /*
	     * Save the rest of the string that we haven't processed
	     * for tacking on after the new link path has been
	     * dropped into the path.
	     */
	    strcpy (tempPath, ptr);

	    /*
	     * is it an absolute path? Simply replace the path
	     * being search with the new link path.
	     */
	    if (*linkPath == '/')
		strcpy (newPath, linkPath);
	    else
	      {
		/*
		 * this is a relative link.
		 * prev is looking at the first character of this directory.
		 * replace with the link.
		 */
		strcpy (prev, linkPath);
	      }

	    /*
	     * now tack on the rest of the name
	     */
	    strcat (newPath, tempPath);

	    /*
	     * compress out the directory changes.
	     */
	    if (_DtHelpCeCompressPathname (newPath) != 0)
		return NULL;

	    /*
	     * start again from the top, until we have a clean path
	     */
	    ptr = newPath;
	  }

      } while (*ptr != '\0');

    return (strdup (newPath));
}

/******************************************************************************
 * Function:    char *_DtHelpCeTraceFilenamePath (char *file_path)
 *
 * Parameters:
 *		 file_path	Specifies the a path to trace and compress
 *
 * Return Value:	The new string if successful, NULL otherwise.
 *			The new string is owned by the caller and
 *			contains an absolute filename path.
 *
 * errno Values:        EINVAL                  Illegal parameter specified.
 *                      getcwd(2)               errno set via a getcwd call.
 *                      readlink(2)             errno set via a readlink call.
 *                      DtErrorMalloc
 *                      DtErrorExceedMaxSize   The new path will exceed
 *                                              max_size.
 *                      DtErrorIllegalPath     The compression will required
 *                                              the path to change to a parent
 *                                              directory beyond the beginning
 *                                              of basePath.
 *
 * Purpose:     This function is called to trace a filename path.
 *		It can contain symbolic links, //, /./, and /../.
 *
 ******************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeTraceFilenamePath (file_path)
    char        *file_path;
#else
_DtHelpCeTraceFilenamePath (char *file_path)
#endif /* _NO_PROTO */
{
    int     result;
    int     done = False;
    char   *newPath;
    char   *oldName;
    char   *namePlace;
    char    workName [MAXPATHLEN + 2];
    char    newName  [MAXPATHLEN + 2];
    char    linkName [MAXPATHLEN + 2];

    if (file_path == NULL || *file_path == '\0')
      {
	errno = EINVAL;
	return NULL;
      }

    workName[0] = '\0';
    if ((MB_CUR_MAX == 1 || mblen(file_path, MB_CUR_MAX) == 1)
							&& *file_path != '/')
      {
	if (getcwd(workName, MAXPATHLEN) == NULL)
	   return NULL;

	strcat(workName, "/");
      }

    strcat (workName, file_path);

    do
      {
        /*
         * find and save the old filename
         */
        result = _DtHelpCeStrrchr(workName, Slash, MB_CUR_MAX, &oldName);
        if (result == -1)
	    return NULL;

	/*
	 * terminate the path
	 */
	*oldName = '\0';

	/*
	 * trace the path, resolving the symbolic links
	 * and directory changes. If /filename given,
	 * skip the path tracing.
	 */
	newName[0] = '\0';
	if (workName[0] != '\0')
	  {
	    newPath = _DtHelpCeTracePathName(workName);
	    if (newPath == NULL)
	        return NULL;

	    /*
	     * copy the new path and free the allocated copy
	     */
	    strcpy (newName, newPath);
	    free (newPath);
	  }

	/*
	 * replace the slash
	 */
	*oldName = '/';

	/*
	 * now append the slash and filename (pointed to by oldName)
	 * onto the end of the new path.
	 */
	namePlace = newName + strlen (newName);
	strcpy (namePlace, oldName);

	/*
	 * See if the absolute path/filename is a symbolic link.
	 */
	result = readlink (newName, linkName, MAXPATHLEN);
	if (result == -1)
	  {
	    if (errno != EINVAL)
		return NULL;

	    done = True;
	  }
	else
	  {
	    /*
	     * put the null byte on the end of the symbolic
	     * link string.
	     */
	    linkName [result] = '\0';
	    if (*linkName == '/')
		strcpy (newName, linkName);
	    else
	      {
		/*
		 * overwrite the filename with the link
		 * but don't overwrite the slash.
		 */
		strcpy ((namePlace + 1), linkName);
	      }

	    /*
	     * make a copy of the new name to work on
	     */
	    strcpy (workName, newName);
	  }
      } while (!done);

    return (strdup (newName));
}


/******************************************************************************
 *                    Core Engine Semi-Public Functions
 ******************************************************************************/
/*****************************************************************************
 * Function: char *_DtHelpCeGetVolumeName (VolumeHandle volume)
 *
 * Parameters:
 *
 * Returns:	ptr to the name of the volume, NULL otherwise.
 *
 * Purpose:	Get the fully qualified volume name.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetVolumeName (volume_handle)
    VolumeHandle  volume_handle;
#else
_DtHelpCeGetVolumeName (
    VolumeHandle  volume_handle)
#endif /* _NO_PROTO */
{
    return (((_DtHelpVolume)volume_handle)->volFile);

} /* End __DtHelpCeGetVolumeName */

/*****************************************************************************
 * Function: int _DtHelpCeFileOpenAndSeek (char *name, int offset, int fildes,
 *							BufFilePtr *ret_file)
 *
 * Parameters:	name		Specifies the file to open.
 *		offset		Specifies location within the file to seek to.
 *
 * Returns:	0 if successful, -1 if a failure occurs
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorFileSeek
 *					Specifies the seek offset was invalid.
 *
 * Purpose:	Open a file and seek to a specific place.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFileOpenAndSeek (name, offset, fd, ret_file, ret_time)
	char		*name;
	int		 offset;
	int		 fd;
	BufFilePtr	*ret_file;
	time_t		*ret_time;
#else
_DtHelpCeFileOpenAndSeek (
	char		*name,
	int		 offset,
	int		 fd,
	BufFilePtr	*ret_file,
	time_t		*ret_time)
#endif /* _NO_PROTO */
{
    unsigned char fileMagic[4];
    int           bytesRead;
    int           result = 0;
    int           tmpFd;
    struct stat   buf;

    /*
     * Get the file descriptor of the uncompressed file
     */
    tmpFd = fd;
    if (fd == -1)
      {
        result = FileOpenRtnFd (name, &tmpFd);
        if (result == -1)
	    return -1;
      }

    if (ret_time != NULL)
      {
	(void) fstat(tmpFd, &buf);
	*ret_time = buf.st_mtime;
      }

    /*
     * make sure we don't go past the end of the file
     */
    result = lseek (tmpFd, (off_t) 0, SEEK_END);
    if (result != -1)
      {
        if (result > offset)
            result = lseek (tmpFd, (off_t) offset, SEEK_SET);
        else
          {
            result = -1;
            errno = CEErrorFileSeek;
          }
      }

    if (result == -1)
      {
	if (fd == -1)
	    close (tmpFd);
	return -1;
      }

    bytesRead = read(tmpFd, fileMagic, 4);
    if (bytesRead != 4)
      { /* something's wrong in reading the file */
	if (fd == -1)
	    close (tmpFd);
	return -1;
      }

    if (!*fileMagic)
      { /* started with compressed file magic number */

	CECompressInfoPtr myInfo;
	BufFilePtr	  inputRaw;

	/*
	 * allocate the private information
	 */
	myInfo = (CECompressInfoPtr) malloc(sizeof(CECompressInfo));
	if (myInfo == NULL)
	  {
	    if (fd == -1)
	        close (tmpFd);
	    errno = CEErrorMalloc;
	    return -1;
	  }

	/*
	 * set the information
	 * set the size to the maximum number of bytes we
	 * want to read.
	 */
	myInfo->fd   = tmpFd;
	myInfo->size =
	  (((fileMagic[1] * 256) + fileMagic[2]) * 256) + fileMagic[3];

	/*
	 * start with raw functionality
	 */
	inputRaw = _DtHelpCeBufFileRdRawZ(myInfo);
	if (inputRaw == NULL)
	  {
	    if (fd == -1)
	        close (tmpFd);
	    return -1;
	  }

	*ret_file = _DtHelpCeBufFilePushZ(inputRaw);
	if (*ret_file == NULL)
	  {
	    _DtHelpCeBufFileClose(inputRaw, (fd == -1 ? True : False));
	    return -1;
	  }
      }
    else
      {
	/*
	 * not a compressed file, back up the four bytes we read
	 */
	result = lseek (tmpFd, (off_t) offset, SEEK_SET);
	if (result == -1)
	  {
	    if (fd == -1)
	        close (tmpFd);
	    return -1;
	  }

	/*
	 * read with raw functionality
	 */
	*ret_file = _DtHelpCeBufFileRdWithFd(tmpFd);
	if (*ret_file == NULL)
	  {
	    if (fd == -1)
	        close (tmpFd);
	    return -1;
	  }
      }

    return 0;

} /* End of _DtHelpCeFileOpenAndSeek */
/******************************************************************************
 *                     Core Engine Public Functions
 ******************************************************************************/
/******************************************************************************
 * Function:	int _DtHelpCeOpenVolume (char *volFile, _DtHelpVolume *retVol);
 *
 * Parameters:	volFile		Specifies the name of the Help Volume file
 *				to load.
 *
 *		retVol		Returns the handle to the loaded volume.
 *				If a volume is opened several times, the
 *				same handle will be returned each time.
 *
 * Return Value:		0 if successful, -1 if a failure occurred.
 *
 * errno Values:	EINVAL			Illegal parameter specified.
 *			getcwd(2)		errno set via a getcwd call.
 *			readlink(2)		errno set via a readlink call.
 * 			CEErrorMalloc
 *			CEErrorExceedMaxSize	The new path will exceed
 *						max_size.
 *			CEErrorIllegalPath	The compression will required
 *						the path to change to a parent
 *						directory beyond the beginning
 *						of basePath.
 *			CEErrorIllegalDatabaseFile
 *						Specifies that 'volFile' is
 *						an illegal database file.
 *
 *
 * Purpose:	This function must be called to open a Help Volume file
 *		before any of the information in the volume can be
 *		accessed. 
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeOpenVolume (canvas_handle, volFile, retVol)
    CanvasHandle canvas_handle;
    char	*volFile;
    VolumeHandle  *retVol;
#else
_DtHelpCeOpenVolume (
    CanvasHandle canvas_handle,
    char	*volFile, 
    VolumeHandle  *retVol)
#endif /* _NO_PROTO */
{
    int     result  = 0;
    _DtHelpVolume vol, prevVol;

    if (volFile == NULL || retVol == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * follow all the symbolic links and get the absolute path and filename.
     */
    volFile = _DtHelpCeTraceFilenamePath(volFile);
    if (volFile == NULL)
	return -1;

    /* Search the volume chain to see if it is already open. */
    prevVol = NULL;
    vol = volChain; 
    while (vol != NULL && strcmp (vol->volFile, volFile))
      {
	prevVol = vol;
	vol     = vol->nextVol;
      }

    if (vol)
      {
	vol->openCount++;
        free(volFile);
      }
    else /* if (vol == NULL) */
      {
    	/* If it isn't open, open it and insert it in the chain. */
	result = VolumeLoad (canvas_handle, volFile, &vol);
	if (result == 0)
	  {
	    if (prevVol == NULL)
	        volChain = vol;
	    else
	        prevVol->nextVol = vol;
	  }
      }

    /* Return the volume handle and a status indicating success/failure. */
    *retVol = (VolumeHandle) vol;
    return result;
}

/******************************************************************************
 * Function:	int _DtHelpCeUpVolumeOpenCnt (VolumeHandle vol);
 *
 * Parameters:	vol	Specifies the loaded volume.
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * errno Values:	EINVAL		'vol' was NULL, no volumes open or
 *					'vol' does not exist.
 *
 * Purpose:	When the volume is no longer needed, it should be
 *		closed with this call.  If the volume has been opened
 *		several times, closing it will just decrement the 
 *		reference count.  When it has been closed as many times
 *		as it was opened, the memory it is using will be freed
 *		and any handles to the volume will be invalid.
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeUpVolumeOpenCnt (volume)
     VolumeHandle	volume;
#else
_DtHelpCeUpVolumeOpenCnt (
     VolumeHandle	volume)
#endif /* _NO_PROTO */
{
    _DtHelpVolume prevVol;
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    if (vol == NULL || volChain == NULL)
      {
	errno = EINVAL;
	return (-1);
      }

    /*
     * check to see if this volume is in our chain
     */
    if (vol != volChain)
      {
	if (CheckVolList (vol, &prevVol) == -1)
          {
	    errno = EINVAL;
	    return (-1);
          }
      }

    /*
     * increment it's usage count.
     */
    vol->openCount++;

    return (0);
}

/******************************************************************************
 * Function:	int _DtHelpCeCloseVolume (VolumeHandle vol);
 *
 * Parameters:	vol	Specifies the loaded volume.
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * errno Values:	EINVAL		'vol' was NULL, no volumes open or
 *					'vol' does not exist.
 *
 * Purpose:	When the volume is no longer needed, it should be
 *		closed with this call.  If the volume has been opened
 *		several times, closing it will just decrement the 
 *		reference count.  When it has been closed as many times
 *		as it was opened, the memory it is using will be freed
 *		and any handles to the volume will be invalid.
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeCloseVolume (canvas, volume)
     CanvasHandle	canvas;
     VolumeHandle	volume;
#else
_DtHelpCeCloseVolume (
     CanvasHandle	canvas,
     VolumeHandle	volume)
#endif /* _NO_PROTO */
{
    _DtHelpVolume prevVol;
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    if (vol == NULL || volChain == NULL)
      {
	errno = EINVAL;
	return (-1);
      }

    /*
     * check to see if this volume is in our chain
     */
    if (vol != volChain)
      {
	if (CheckVolList (vol, &prevVol) == -1)
          {
	    errno = EINVAL;
	    return (-1);
          }
      }

    /*
     * decrement it's usage count.
     */
    vol->openCount--;
    if (vol->openCount == 0)
      {
	/* The volume is no longer needed.  Unlink it from the chain
	   and free it. */

	if (vol == volChain) 
	    volChain = volChain->nextVol;

	else
	    prevVol->nextVol = vol->nextVol;

	VolumeUnload (canvas, vol);
      }

    return (0);
}

/*****************************************************************************
 * Function: CEBoolean _DtHelpCeGetTopTopicId (_DtHelpVolume vol,
 *					char **ret_idString)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		ret_idString 	Returns the location ID of the
 *				the top level topic.
 *
 * Memory own by caller:
 *		ret_idString
 *
 * Returns:	True for success, False if a failure occurs.
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorMissingTopTopicRes
 *					Specifies that the 'TopTopic/topTopic'
 *					resource is missing from the database.
 * 			CEErrorMalloc
 *
 * Purpose:	Get the information to access the top level topic.
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeGetTopTopicId (volume, ret_idString)
	VolumeHandle	volume;
        char		**ret_idString;
#else
_DtHelpCeGetTopTopicId (
	VolumeHandle	volume,
        char		**ret_idString )
#endif /* _NO_PROTO */
{
    CEBoolean   found = False;
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    if (vol == NULL || ret_idString == NULL || CheckVolList(vol, NULL) == -1)
	errno = EINVAL;
    else
      {
	/*
	 * What type of volume is it?
	 */
	if (vol->sdl_flag == False)
	    (void) _DtHelpCeGetCcdfTopTopic(vol, ret_idString);
	else
	    *ret_idString = _DtHelpCeGetSdlHomeTopicId((VolumeHandle) vol);

	if (*ret_idString != NULL)
	    *ret_idString = strdup(*ret_idString);

	if (*ret_idString != NULL)
	    found = True;
      }

    return found;

}  /* End _DtHelpCeGetTopTopicId */

/*****************************************************************************
 * Function: CEBoolean _DtHelpCeFindId (_DtHelpVolume vol, char *target_id,
 *					int fd,
 *					char *ret_name,	int *ret_offset)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		target_id	Specifies target location ID
 *		fd		Specifies the locked file descriptor.
 *		ret_name 	Returns a null terminated string
 *				containing a fully qualified path to
 *				the file that contains 'target_id'.
 *		ret_offset 	Returns the offset into 'ret_name'
 *				to the topic that contains 'target_id'.
 *
 * Memory own by caller:
 *		ret_name
 *
 * Returns:	True if successful, False if a failure occurs
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorMalloc
 *			CEErrorMissingFilenameRes
 *					Specifies that the 'Filename/filename'
 *					resource for 'topic' does not exist.
 *			CEErrorMissingFileposRes
 *					If the resource is not in the
 *					database or if the resource NULL.
 *			CEErrorLocIdNotFound
 *					Specifies that 'locId' was not
 *					found.
 *
 * Purpose:	Find which topic contains a specified locationID.
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeFindId (volume, target_id, fd, ret_name, ret_offset)
	VolumeHandle	volume;
	char		*target_id;
	int		 fd;
	char		**ret_name;
	int		*ret_offset;
#else
_DtHelpCeFindId (
	VolumeHandle	volume,
	char		*target_id,
	int		 fd,
	char		**ret_name,
	int		*ret_offset )
#endif /* _NO_PROTO */
{
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || target_id == NULL || ret_name == NULL ||
			ret_offset == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return False;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return (_DtHelpCeFindCcdfId(vol, target_id, ret_name, ret_offset));

    return (_DtHelpCeFindSdlId(vol, target_id, fd, ret_name, ret_offset));

}  /* End _DtHelpCeFindId */

/*****************************************************************************
 * Function: int _DtHelpCeGetKeywordList (_DtHelpVolume vol, char ***ret_keywords)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		ret_keywords	Returns a NULL-terminated string array
 *				containing the sorted list of keywords in the
 *				volume.  This array is NOT owned by the caller
 *				and should only be read or copied.
 *
 * Returns:	The count of keywords associated with the volume if successful.
 *		-1 if a failure occurs;
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorMalloc
 *			CEErrorIllegalDatabaseFile
 *					Specifies that the keyword file is
 *					invalid or corrupt.
 *			CEErrorMissingKeywordsRes
 *					Specifies that the keyword file does
 *					not contain the 'Keywords/keywords'
 *					resource or the resource is NULL
 *
 * Purpose:	Get the list of keywords contained in a volume.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetKeywordList (volume, ret_keywords)
	VolumeHandle	volume;
	char		***ret_keywords;
#else
_DtHelpCeGetKeywordList (
	VolumeHandle	volume,
	char		***ret_keywords )
#endif /* _NO_PROTO */
{
    int       nameCount = -1;
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    if (vol == NULL || ret_keywords == NULL || CheckVolList (vol, NULL) == -1)
	errno = EINVAL;
    else if (GetVolumeKeywords (vol, ret_keywords) == 0)
      {
	nameCount = 0;
	while (*ret_keywords && (*ret_keywords)[nameCount])
	    nameCount++;
      }

    return nameCount;

}  /* End _DtHelpCeGetKeywordList */

/*****************************************************************************
 * Function: int _DtHelpCeFindKeyword (_DtHelpVolume vol, char *target, char ***ret_ids)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		target		The target keyword.
 *		ret_ids 	Returns a null terminated list of location
 *				ids associated with the target keyword.
 *
 * Returns:	The count of ids associated with the keyword if successful.
 *		-1 if a failure occurs;
 *
 * errno Values:	EINVAL		Specifies an invalid parameter was
 *					used.
 *			CEErrorNoKeywordList
 *					Specifies that the volume does not
 *					have a keyword list.
 *			CEErrorIllegalKeyword
 *					Specifies that 'target' was not
 *					found.
 *			CEErrorMalloc
 *			CEErrorIllegalDatabaseFile
 *					Specifies that the keyword file is
 *					invalid or corrupt.
 *			CEErrorMissingKeywordsRes
 *					Specifies that the keyword file does
 *					not contain the 'Keywords/keywords'
 *					resource or the resource is NULL
 *
 * Purpose:	Get the list of location ids associated with a keyword
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFindKeyword (volume, target, ret_ids)
	VolumeHandle	volume;
	char		*target;
	char		***ret_ids;
#else
_DtHelpCeFindKeyword (
	VolumeHandle	volume,
	char		*target,
	char		***ret_ids )
#endif /* _NO_PROTO */
{
    int       nameCount = -1;
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    if (vol == NULL || target == NULL || ret_ids == NULL ||
					CheckVolList (vol, NULL) == -1)
	errno = EINVAL;
    else if (GetKeywordTopics (vol, target, ret_ids) == 0)
      {
	nameCount = 0;
	while (*ret_ids && (*ret_ids)[nameCount])
	    nameCount++;
      }

    return nameCount;

}  /* End _DtHelpCeFindKeyword */

/******************************************************************************
 * Function:	int _DtHelpCeGetAsciiVolumeAbstract (_DtHelpVolume vol, char **abstract);
 *
 * Parameters:	vol	Specifies the loaded volume.
 *		abstract Returns the abstract of the volume.  This string
 *			 is owned by the caller and should be freed.
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * errno Values:	EINVAL		'vol' or 'retAbs' is NULL.
 *			CEErrorMalloc
 *			CEErrorMissingAbstractRes
 *					'vol' does not contain a 'Abstract'
 *					or 'abstract' resource or the resource
 *					is NULL.
 *
 * Purpose:	Get the abstract of a volume.
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeGetAsciiVolumeAbstract (canvas, volume, retAbs)
    CanvasHandle	  canvas;
    VolumeHandle	  volume;
    char		**retAbs;
#else
_DtHelpCeGetAsciiVolumeAbstract (
    CanvasHandle	  canvas,
    VolumeHandle	  volume,
    char		**retAbs)
#endif /* _NO_PROTO */
{
    _DtHelpVolume vol = (_DtHelpVolume)volume;
    if (vol == NULL || retAbs == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
	return (_DtHelpCeGetCcdfVolumeAbstract(vol, retAbs));

    *retAbs = _DtHelpCeGetSdlVolumeAsciiAbstract(canvas, volume);
    if (*retAbs == NULL)
	return (-1);

    return (0);
}

/*****************************************************************************
 * Function: int _DtHelpCeGetTopicTitle (CanvasHandle canvas_handle,
 *                                      VolumeHandle volume,
 *                                      char *id, char **ret_title)
 *
 * Parameters:  volume          Specifies the volume containing the id.
 *              id              Specifies the id for the topic desired.
 *              ret_title       Returns a null terminated string containing
 *                              the title.
 *
 * Memory own by caller:
 *              ret_title
 *
 * Returns:     0 if successful, -2 if didn't find the id,
 *              otherwise -1.
 *
 * errno Values:        EINVAL          Specifies an invalid parameter was
 *                                      used.
 *
 * Purpose:     Get the title of a topic.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetTopicTitle (canvas, volume, id, ret_title)
    CanvasHandle          canvas;
    VolumeHandle          volume;
    char                 *id;
    char                **ret_title;
#else
_DtHelpCeGetTopicTitle (
    CanvasHandle          canvas,
    VolumeHandle          volume,
    char                 *id,
    char                **ret_title)
#endif /* _NO_PROTO */
{
    int            result;
    char          *abbrevTitle;
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    if (canvas == NULL || volume == NULL || id == NULL ||
			CheckVolList (vol, NULL) == -1 || ret_title == NULL)
      {
        errno = EINVAL;
        return -1;
      }

    /*
     * Try to get the title via the <TOPIC> and <ABBREV> tags.
     */
    result = GetTopicTitleAndAbbrev(canvas, vol, id, ret_title, &abbrevTitle);
    if (result == 0)
      {
        /*
         * If we have a abbreviated title, return it instead.
         */
        if (abbrevTitle)
          {
            if (*ret_title)
                free ((char *) *ret_title);
            *ret_title = abbrevTitle;
          }
      }

    return result;

}  /* End _DtHelpCeGetTopicTitle */

/*****************************************************************************
 * Function: int _DtHelpCeMapTargetToId (_DtHelpVolume vol,
 *					char *target_id,
 *					char *ret_id)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		target_id	Specifies target location ID
 *		ret_id 		Returns the id containing the target_id.
 *				This memory *IS NOT* owned by the caller.
 *
 * Returns:	0 if successful, -1 if a failure occurs
 *
 * Purpose:	Find which topic contains a specified locationID.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeMapTargetToId (volume, target_id, ret_id)
	VolumeHandle	volume;
	char		*target_id;
	char		**ret_id;
#else
_DtHelpCeMapTargetToId (
	VolumeHandle	volume,
	char		*target_id,
	char		**ret_id)
#endif /* _NO_PROTO */
{
    _DtHelpVolume vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || target_id == NULL || ret_id == NULL ||
					CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return (_DtHelpCeMapCcdfTargetToId(vol, target_id, ret_id));

    return (_DtHelpCeMapIdToSdlTopicId(vol, target_id, ret_id));

}  /* End _DtHelpCeMapTargetToId */

/*****************************************************************************
 * Function: char * _DtHelpCeGetVolumeLocale (_DtHelpVolume vol)
 *
 * Parameters:	vol		Specifies the loaded volume
 *
 * Returns:	The pointer to the locale string if successful. Otherwise
 *		NULL.
 *
 * Purpose:	Get the locale of the specified volume.
 *		Returns the locale in a unix specific format
 *		- locale[_ter][.charset] - This memory is owned by
 *		the caller.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetVolumeLocale (volume)
	VolumeHandle	volume;
#else
_DtHelpCeGetVolumeLocale (
	VolumeHandle	volume)
#endif /* _NO_PROTO */
{
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return NULL;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return (_DtHelpCeGetCcdfVolLocale(vol));

    return (_DtHelpCeGetSdlVolumeLocale(vol));

}  /* End _DtHelpCeGetVolumeLocale */

/*****************************************************************************
 * Function: int _DtHelpCeGetDocStamp (VolumeHandle volume, char **ret_doc,
 *						char **ret_time)
 *
 * Parameters:	volume		Specifies the loaded volume
 *		ret_doc		Returns the doc id.
 *		ret_time	Returns the time stamp.
 *
 * Memory:	Caller owns the string in ret_doc and ret_time.
 *
 * Returns:	0 if successful, -2 if the volume does not contain
 *		one or the other, -1 if any other failure.
 *
 * Purpose:	Get doc id and time stamp of a volume.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetDocStamp (volume, ret_doc, ret_time)
    VolumeHandle	  volume;
    char		**ret_doc;
    char		**ret_time;
#else
_DtHelpCeGetDocStamp (
    VolumeHandle	volume,
    char		**ret_doc,
    char		**ret_time)
#endif /* _NO_PROTO */
{
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return NULL;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return (_DtHelpCeGetCcdfDocStamp (vol, ret_doc, ret_time));

    return (_DtHelpCeGetSdlDocStamp(vol, ret_doc, ret_time));

}  /* End _DtHelpCeGetDocStamp */

/*****************************************************************************
 * Function: char * _DtHelpCeGetTopicChilren (VolumeHandle vol)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		topic_id	Speicifes the topic id of which the
 *				children are to be found.
 *		retTopics	Returns the null terminated array of
 *				ids. This memory is owned by the caller
 *				and must be freed.
 *
 * Returns:	> 0 if successful, -1 if failures.
 *
 * Purpose:	Get the children of a topic.
 *
 *****************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeGetTopicChildren (volume, topic_id, retTopics)
    VolumeHandle   volume,
    char	  *topic_id;
    char	***retTopics;
#else
_DtHelpCeGetTopicChildren (
    VolumeHandle   volume,
    char	  *topic_id,
    char	***retTopics)
#endif /* _NO_PROTO */
{
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return(_DtHelpCeGetCcdfTopicChildren(volume, topic_id, retTopics));

    return (_DtHelpCeGetSdlTopicChildren(volume, topic_id, retTopics));

}  /* End _DtHelpCeGetTopicChildren */

/******************************************************************************
 * Function:	int _DtHelpVolumeTitle (_DtHelpVolume vol, char **retTitle);
 *
 * Parameters:	vol		Specifies the loaded volume.
 *		retTitle	Returns the title of the volume. This string is
 *				owned by the caller and must be freed.
 *
 * Return Value: 0 if successful, -1 if a failure occurs
 *
 * errno Values:	CEErrorMalloc
 *			EINVAL		'vol' or 'retTitle' is NULL.
 *			CEErrorMissingTitleRes
 *					'vol' does not contain the resource
 *					'Title' or 'title' or the resource
 *					is zero length.
 *
 * Purpose:	Get the title of a volume.
 *
 ******************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeGetVolumeTitle (canvas, volume, retTitle)
    CanvasHandle	  canvas;
    VolumeHandle	  volume;
    char		**ret_title;
#else
_DtHelpCeGetVolumeTitle (
    CanvasHandle	  canvas,
    VolumeHandle	  volume,
    char		**ret_title)
#endif /* _NO_PROTO */
{
    _DtHelpVolume vol = (_DtHelpVolume)volume;
    if (vol == NULL || ret_title == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
        return (_DtHelpCeGetCcdfVolumeTitle(vol, ret_title));

    return (_DtHelpCeFrmtSDLVolTitleToAscii(canvas, volume, ret_title));

}

/*****************************************************************************
 * Function: CEBoolean _DtHelpCeGetVolumeFlag (VolumeHandle vol)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		topic_id	Speicifes the topic id of which the
 *				children are to be found.
 *		retTopics	Returns the null terminated array of
 *				ids. This memory is owned by the caller
 *				and must be freed.
 *
 * Returns:	> 0 if successful, -1 if failures.
 *
 * Purpose:	Get the children of a topic.
 *
 *****************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeGetVolumeFlag (volume)
    VolumeHandle   volume,
#else
_DtHelpCeGetVolumeFlag (
    VolumeHandle   volume)
#endif /* _NO_PROTO */
{
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * What type of volume is it?
     */
    return((int) vol->sdl_flag);

}  /* End _DtHelpCeGetVolumeFlag */

/*****************************************************************************
 * Function: int _DtHelpCeLockVolume (VolumeHandle vol)
 *
 * Parameters:	vol		Specifies the loaded volume
 *
 * Returns:	> 0 if successful, -1 if failures.
 *
 * Purpose:	Lock the volume so that it can't get change out from
 *		under the caller.
 *
 *****************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeLockVolume (canvas_handle, volume, lock_info)
    CanvasHandle	 canvas_handle;
    VolumeHandle	 volume;
    _DtHelpCeLockInfo	*lock_info;
#else
_DtHelpCeLockVolume (
    CanvasHandle	 canvas_handle,
    VolumeHandle	 volume,
    _DtHelpCeLockInfo	*lock_info)
#endif /* _NO_PROTO */
{
    struct stat    buf;
    _DtHelpVolume  vol = (_DtHelpVolume)volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
	return -1;

    /*
     * lock it by opening it.
     */
    lock_info->fd = open(vol->volFile, O_RDONLY);
    if (lock_info->fd == -1)
	return -1;

    (void) fstat(lock_info->fd, &buf);
    if (buf.st_mtime != vol->check_time)
      {
	if (RereadVolume(canvas_handle, vol) != 0)
	  {
	    close(lock_info->fd);
	    return -1;
	  }

	vol->check_time = buf.st_mtime;
      }

    /*
     * Synthetic open
     */
    vol->openCount++;
    lock_info->volume = volume;
    return 0;

}  /* End _DtHelpCeLockVolume */

/*****************************************************************************
 * Function: int _DtHelpCeUnlockVolume (VolumeHandle vol)
 *
 * Parameters:	vol		Specifies the loaded volume
 *
 * Returns:	> 0 if successful, -1 if failures.
 *
 * Purpose:	Unlock the volume.
 *
 *****************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeUnlockVolume (lock_info)
    _DtHelpCeLockInfo	lock_info;
#else
_DtHelpCeUnlockVolume (
    _DtHelpCeLockInfo	lock_info)
#endif /* _NO_PROTO */
{
    _DtHelpVolume  vol = (_DtHelpVolume)(lock_info.volume);

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
	return -1;

    /*
     * check to see if it needs to be unlocked.
     */
    if (lock_info.fd != -1)
	close(lock_info.fd);

    /*
     * Synthetic close
     */
    vol->openCount--;
    return 0;

}  /* End _DtHelpCeUnlockVolume */

/*****************************************************************************
 * Function: int _DtHelpCeIsTopTopic (VolumeHandle volume, const char *id)
 *
 * Parameters:	vol		Specifies the loaded volume
 *              id              Specifies a location id.
 *
 * Returns:	= 0 if successful, != 0 if failures.
 *
 * Purpose:	Tests to see if the location id is in the top topic of
 *		the volume.
 *
 *****************************************************************************/
int 
#ifdef _NO_PROTO
_DtHelpCeIsTopTopic (volume, id)
    VolumeHandle	 volume;
    const char		*id;
#else
_DtHelpCeIsTopTopic (
    VolumeHandle	 volume,
    const char		*id)
#endif /* _NO_PROTO */
{
    int		   result  = -1;
    char          *topicId = NULL;
    char          *topId   = NULL;
    _DtHelpVolume  vol = (_DtHelpVolume) volume;

    /*
     * check the parameters
     */
    if (vol == NULL || CheckVolList (vol, NULL) == -1)
	return -1;

    /*
     * What type of volume is it?
     */
    if (vol->sdl_flag == False)
      {
	if (_DtHelpCeMapCcdfTargetToId(vol, id, &topicId) == 0 &&
				_DtHelpCeGetCcdfTopTopic(vol, &topId) == 0)
            result = _DtHelpCeStrCaseCmp(topId, topicId);
      }
    else if (_DtHelpCeMapIdToSdlTopicId(vol, id, &topicId) == 0)
      {
	topId = _DtHelpCeGetSdlHomeTopicId(volume);
	if (topId != NULL)
	    result = _DtHelpCeStrCaseCmp(topId, topicId);
      }

    return result;

}  /* End _DtHelpCeIsTopTopic */
