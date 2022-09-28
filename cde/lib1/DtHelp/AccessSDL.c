/* $XConsortium: AccessSDL.c /main/cde1_maint/1 1995/07/17 17:19:03 drk $ */
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "Access.h"
#include "bufioI.h"
#include "SDLI.h"
#include "AccessI.h"
#include "AccessP.h"
#include "AccessSDLP.h"
#include "AccessSDLI.h"

#include "CanvasI.h"
#include "CleanUpI.h"
#include "FormatSDLI.h"
#include "StringFuncsI.h"
#include "CEUtilI.h"

#ifdef NLS16
#endif

/********    Private Defines      ********/
/********    End Private Defines  ********/

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static	int	ProcessEntry ();
#else
static	int	ProcessEntry (
			_DtHelpVolume	 vol,
			CESegment	*p_seg,
			char		*parent_key);
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/********    Private Variable Declarations    ********/
static	const	char	*IsoString = "ISO-8859-1";
static	const	CESDLVolume	DefaultSdlVolume =
  {
     NULL,		/* CESegment *sdl_info; */
     NULL,		/* CESegment *toss;     */
     NULL,		/* CESegment *loids;    */
     NULL,		/* CESegment *index;    */
     NULL,		/* CESegment *title;    */
     NULL,		/* CESegment *snb;      */
     0,			/* short      minor_no; */
     False,		/* CEBoolean  title_processed; */
  };

/********    End Private Variable Declarations    ********/

/******************************************************************************
 *                          Private Functions
 ******************************************************************************/
/******************************************************************************
 * Function:	int ProcessSubEntries (
 *
 * Parameters:
 *
 * Return Value:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static int
#ifdef _NO_PROTO
ProcessSubEntries (vol, p_seg, parent_key)
    _DtHelpVolume vol;
    CESegment	*p_seg;
    char	*parent_key;
#else
ProcessSubEntries (
    _DtHelpVolume vol,
    CESegment	*p_seg,
    char	*parent_key)
#endif /* _NO_PROTO */
{
    while (p_seg != NULL)
      {
	if (_CEIsElement(p_seg->seg_type) &&
		_CEElementOfSegment(p_seg)->el_type == SdlElementEntry &&
		ProcessEntry(vol, _CEElementOfSegment(p_seg)->seg_list,
							parent_key) == -1)
	    return -1;

	p_seg = p_seg->next_seg;
      }
    return 0;
}

/******************************************************************************
 * Function:	int AsciiKeyword (
 *
 * Parameters:
 *
 * Return Value:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static char * 
#ifdef _NO_PROTO
AsciiKeyword (p_el, parent_str)
    CEElement	*p_el;
    char	*parent_str;
#else
AsciiKeyword (
    CEElement	*p_el,
    char	*parent_str)
#endif /* _NO_PROTO */
{
    CESegment	*pList   = p_el->seg_list;
    int		 len     = 0;
    char	*goodStr = NULL;

    while (pList != NULL)
      {
	if (_CEIsString(pList->seg_type))
	  {
	    if (parent_str != NULL)
		len += strlen(parent_str);

	    len += strlen(_CEStringOfSegment(pList));

	    if (goodStr == NULL)
	      {
		len++;
		goodStr = (char *) malloc (len);
		if (goodStr != NULL)
		    goodStr[0] = '\0';
	      }
	    else
		goodStr = (char *) realloc (goodStr, len);

	    if (goodStr == NULL)
		return NULL;

	    strcat(goodStr, _CEStringOfSegment(pList));

	    parent_str = NULL;
	  }
	else if (_CEIsElement(pList->seg_type) &&
			_CEElementOfSegment(pList)->el_type != SdlElementEntry)
	  {
	    goodStr = AsciiKeyword(_CEElementOfSegment(pList), goodStr);
	    if (goodStr == NULL)
		return NULL;
	    len = strlen(goodStr) + 1;
	  }

	pList = pList->next_seg;
      }

    return goodStr;
}

/******************************************************************************
 * Function:	int ProcessLocations (
 *
 * Parameters:
 *
 * Return Value:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
ProcessLocations (locs, list)
    char	*locs;
    char	***list;
#else
ProcessLocations (
    char	*locs,
    char	***list)
#endif /* _NO_PROTO */
{
    char  **myList = NULL;
    char   *nextLoc;

    while (locs != NULL && *locs != '\0')
      {
	locs = _DtHelpCeGetNxtToken(locs, &nextLoc);
	if (nextLoc == NULL)
	    return -1;

	if (*nextLoc != '\0')
	  {
	    myList = (char **) _DtHelpCeAddPtrToArray ((void **) myList,
							(void *) nextLoc);
	    if (myList == NULL)
		return -1;
	  }
      }

    *list = myList;
    return 0;
}

/******************************************************************************
 * Function:	int ProcessEntry (_DtHelpVolume vol)
 *
 * Parameters:	vol	Specifies the volume whose keywords need to be
 *			loaded from disk.  Once loaded, they can be 
 *			accessed through the fields of the volume structure.
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
 *
 * Purpose:	Load the keywords associated with a volume.
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
ProcessEntry (vol, p_seg, parent_key)
    _DtHelpVolume	 vol;
    CESegment		*p_seg;
    char		*parent_key;
#else
ProcessEntry (
    _DtHelpVolume	 vol,
    CESegment		*p_seg,
    char		*parent_key)
#endif /* _NO_PROTO */
{
    char	**topics;
    char	 *nextKey = NULL;

    /* Now parse the string into the appropriate arrays.  The string has the 
       following syntax:

	<!ELEMENT entry     - - ((%simple; | #PCDATA)*, entry*)       >
	<!ATTLIST entry         id      ID     #IMPLIED
				main    IDREFS #IMPLIED
				locs    IDREFS #IMPLIED
				syns    IDREFS #IMPLIED
				sort    CDATA  #IMPLIED               >
     */

#define	ENTRY_STRINGS \
		(_CEElementOfSegment(p_seg)->attributes.strings.entry_strs)

    while (p_seg != NULL)
      {
	nextKey = AsciiKeyword(_CEElementOfSegment(p_seg), parent_key);

	if (nextKey == NULL)
	    return -1;

	/* We have the next keyword.  Hang onto it and add it to the list
	   once we get the array of topics.  We don't add it yet because if
	   there are no topics we want to throw it away.  (Silently ignoring
	   keywords which specify no topics is an undocumented feature.) */

	/* Now get the list of topics. */
	topics = NULL;
	if (ProcessLocations(ENTRY_STRINGS.main, &topics) == -1 ||
			ProcessLocations(ENTRY_STRINGS.locs, &topics) == -1)
	  {
	    free(nextKey);
	    return -1;
	  }

	if (topics != NULL)
	  {
	    vol->keywords = (char **) _DtHelpCeAddPtrToArray (
						(void **) vol->keywords,
						(void *) nextKey);
	    vol->keywordTopics = (char ***) _DtHelpCeAddPtrToArray (
						(void **) vol->keywordTopics,
						(void *) topics);
	    /*
	     * If we just malloc'ed ourselves out of existance...
	     * stop here.
	     */
	    if (vol->keywords == 0 || vol->keywordTopics == 0)
	      {
		if (vol->keywords != NULL)
		  {
		    free(nextKey);
		    _DtHelpCeFreeStringArray (vol->keywords);
		    _DtHelpCeFreeStringArray (topics);
		    vol->keywords = NULL;
		  }
		if (vol->keywordTopics)
		  {
		    char ***topicList;

		    for (topicList = vol->keywordTopics; topicList; topicList++)
			_DtHelpCeFreeStringArray (*topicList);
		    free (vol->keywordTopics);
		    vol->keywordTopics = NULL;
		  }
		return -1;
	      }
	  }

	if (_CEElementOfSegment(p_seg)->seg_list != NULL &&
	    ProcessSubEntries(vol, _CEElementOfSegment(p_seg)->seg_list,
								nextKey) == -1)
	    return -1;

	if (topics == NULL)
	    free (nextKey);

	p_seg = p_seg->next_seg;
      }

    return (0);
}

/******************************************************************************
 * Function:	int MapPath (CESegment *cur_id, int level, char ***ret_ids)
 *
 * Parameters:
 *
 * Return Value:	0 if successful, -1 if failure.
 *
 * Memory:	The memory returned in ret_ids is owned by the caller.
 *
 * Purpose:	To come up with a path from the top of the volume to the
 *		target id.
 *
 ******************************************************************************/
static int 
#ifdef _NO_PROTO
MapPath (cur_id, target_el, stop_lev, lev_cnt, hidden_no, ret_ids)
    CESegment	  **cur_id;
    CEElement	   *target_el;
    int		    stop_lev;
    int		    lev_cnt;
    int		    hidden_no;
    char	***ret_ids;
#else
MapPath (
    CESegment	  **cur_id,
    CEElement	   *target_el,
    int		    stop_lev,
    int		    lev_cnt,
    int		    hidden_no,
    char	***ret_ids)
#endif /* _NO_PROTO */
{
    CEElement	*pEl;
    CESegment	*mySeg  = *cur_id;
    int          count  = -1;
    int          myLev;

    while (mySeg != NULL)
      {
	/*
	 * Does this match the target id?
	 * And, is the element a child of the current path?
	 */
	pEl   = _CEElementOfSegment(mySeg);
	myLev = _DtHelpCeSdlRlevel(pEl);
	if (target_el == pEl && (myLev == -1 || myLev > stop_lev))
	  {
	    /*
	     * matched the target id.
	     * allocate memory and return.
	     */
	    count = 0;
	    if (pEl->attributes.type == SdlIdVirpage)
	      {
		count++;
		lev_cnt++;
	      }

	    *ret_ids = (char **) malloc((size_t)(sizeof(char *) * (lev_cnt + 1)));
	    if ((*ret_ids) == NULL)
		return -1;

	    (*ret_ids)[lev_cnt] = NULL;

	    if (pEl->attributes.type == SdlIdVirpage)
		(*ret_ids)[lev_cnt - 1] = strdup (_DtHelpCeSdlRidString(pEl));

	    return count;
	  }
	else if (myLev != -1 && myLev != hidden_no
				&& pEl->attributes.type == SdlIdVirpage)
	  {
	    /*
	     * If we've hit a virpage that is a sibling or an aunt
	     * set the search pointer to this segment (since this
	     * is where we want to start searching again) and return
	     * a negative on the successful search.
	     */
	    if (myLev <= stop_lev)
	      {
		*cur_id = mySeg;
		return -1;
	      }

	    /*
	     * this virpage is a child of mine, so look at it's children
	     * for the target id.
	     */
	    mySeg = mySeg->next_seg;
	    count = MapPath(&mySeg, target_el, myLev, lev_cnt + 1, hidden_no,
				ret_ids);

	    /*
	     * successful response on finding the target id in the virpage's
	     * children. Duplicate the virpage's id string and return to
	     * my parent.
	     */
	    if (count != -1)
	      {
		(*ret_ids)[lev_cnt] = strdup(_DtHelpCeSdlRidString(pEl));
		count++;
		return count;
	      }
 	  }
	else /* did not match the target id and is not a virpage
	      * or is a hidden virpage */
	    mySeg = mySeg->next_seg;
      }

    *cur_id = mySeg;
    return -1;
}

/******************************************************************************
 *                          Semi-Private Functions
 ******************************************************************************/
/*******************************************************************************
 * Function:    CESDLVolume *_DtHelpCeGetSdlVolumePtr (VolumeHandle vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
CESDLVolume *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolumePtr (volume)
     VolumeHandle	 volume;
#else
_DtHelpCeGetSdlVolumePtr (
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    _DtHelpVolume vol = (_DtHelpVolume) volume;

    if (vol != NULL)
	return ((CESDLVolume *) vol->vols.sdl_vol);
    return NULL;
}

/******************************************************************************
 *                          Semi-Public Functions
 ******************************************************************************/
/*******************************************************************************
 * Function:    void _DtHelpCeInitSdlVolume (_DtHelpVolume vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeInitSdlVolume (volume)
     VolumeHandle	 volume;
#else
_DtHelpCeInitSdlVolume (
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESDLVolume	*sdlVol = _DtHelpCeGetSdlVolumePtr(volume);

    if (sdlVol != NULL)
	*sdlVol = DefaultSdlVolume;

}

/*******************************************************************************
 * Function:    void _DtHelpCeOpenSdlVolume (_DtHelpVolume vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeOpenSdlVolume (canvas, volume)
     CanvasHandle	 canvas;
     VolumeHandle	 volume;
#else
_DtHelpCeOpenSdlVolume (
     CanvasHandle	 canvas,
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESDLVolume	*sdlVol;
    _DtHelpVolume  vol = (_DtHelpVolume) volume;

    sdlVol  = (CESDLVolume *) malloc (sizeof(CESDLVolume));
    if (sdlVol != NULL)
      {
	vol->vols.sdl_vol = (SdlVolumeHandle) sdlVol;
	_DtHelpCeInitSdlVolume(volume);
	if (_DtHelpCeFrmtSdlVolumeInfo(canvas,
					vol->volFile,
					vol, &(vol->check_time)) == 0)
	  {
	    vol->sdl_flag = True;
	    return 0;
	  }

	vol->vols.sdl_vol = NULL;
	free(sdlVol);
      }

    return -1;
}

/*******************************************************************************
 * Function:    void _DtHelpCeCleanSdlVolume (_DtHelpVolume vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeCleanSdlVolume (canvas, volume)
     CanvasHandle	 canvas;
     VolumeHandle	 volume;
#else
_DtHelpCeCleanSdlVolume (
     CanvasHandle	 canvas,
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESDLVolume	*sdlVol = _DtHelpCeGetSdlVolumePtr(volume);

    if (sdlVol != NULL)
      {
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->sdl_info),NULL);
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->toss),NULL);
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->loids),NULL);
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->index),NULL);
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->title),NULL);
	_DtHelpCeFreeSdlStructs((CECanvasStruct *)canvas,&(sdlVol->snb),NULL);
      }
}

/*******************************************************************************
 * Function:    int _DtHelpCeRereadSdlVolume (_DtHelpVolume vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeRereadSdlVolume (canvas, volume)
     CanvasHandle	 canvas;
     VolumeHandle	 volume;
#else
_DtHelpCeRereadSdlVolume (
     CanvasHandle	 canvas,
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    _DtHelpCeCleanSdlVolume(canvas, volume);
    _DtHelpCeInitSdlVolume(volume);
    if (_DtHelpCeFrmtSdlVolumeInfo(canvas, _DtHelpCeGetVolumeName(volume),
					volume, NULL) == 0)
	    return 0;

    return -1;
}

/*******************************************************************************
 * Function:    void _DtHelpCeCloseSdlVolume (_DtHelpVolume vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        0 if successful, -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     When the volume is no longer needed, it should be unloaded
 *              with this call.  Unloading it frees the memory (which means
 *              any handles on the volume become invalid.)
 *
 ******************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCeCloseSdlVolume (canvas, volume)
     CanvasHandle	 canvas;
     VolumeHandle	 volume;
#else
_DtHelpCeCloseSdlVolume (
     CanvasHandle	 canvas,
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESDLVolume	*sdlVol = _DtHelpCeGetSdlVolumePtr(volume);

    if (sdlVol != NULL)
      {
	_DtHelpCeCleanSdlVolume(canvas, volume);
	free(sdlVol);
      }
}

/*****************************************************************************
 * Function: Boolean _DtHelpCeGetSdlHomeTopicId (_DtHelpVolume vol,
 *					char *target_id,
 *					char *ret_name,	int *ret_offset)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		target_id	Specifies target location ID
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
 *			CEErrorLocIdNotFound
 *					Specifies that 'locId' was not
 *					found.
 *
 * Purpose:	Find which topic contains a specified locationID.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetSdlHomeTopicId (volume)
	VolumeHandle	 volume;
#else
_DtHelpCeGetSdlHomeTopicId (
	VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESegment	 *idSegs;
    CEElement	 *pEl;
    CESDLVolume  *sdlVol  =  _DtHelpCeGetSdlVolumePtr(volume);

    pEl = _DtHelpCeFindSdlElement(sdlVol->sdl_info, SdlElementSdlDoc, True);
    if (pEl != NULL)
      {
	/*
	 * Was the first page topic declared in the header?
	 */
	if (_CEIsAttrSet(pEl->attributes, SDL_ATTR_FRST_PG))
	    return _DtHelpCeSdlFirstPgAttr(pEl);

	/*
	 * have to search the list of ids for the home topic.  This is a
	 * bit of a kludge since we are looking for a specific string in
	 * the rssi.  But this is for backwards compatibility since the
	 * Snapshot release of the help system were released with out
	 * the first-page attribute and relied on _hometopic.
	 */
        if (_DtHelpCeGetSdlVolIds(volume, -1, &idSegs) != 0)
	    return NULL;

        while (idSegs != NULL)
          {
	    pEl = idSegs->seg_handle.el_handle;
	    if (pEl->attributes.type == SdlIdVirpage &&
		_DtHelpCeStrCaseCmp(_DtHelpCeSdlRssiString(pEl),"_hometopic")
					== 0)
	        return _DtHelpCeSdlRidString(pEl);

	    idSegs = idSegs->next_seg;
          }
      }

    return NULL;
}

/*****************************************************************************
 * Function: Boolean _DtHelpCeFindSdlId (_DtHelpVolume vol, char *target_id,
 *					char *ret_name,	int *ret_offset)
 *
 * Parameters:	vol		Specifies the loaded volume
 *		target_id	Specifies target location ID
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
 *			CEErrorLocIdNotFound
 *					Specifies that 'locId' was not
 *					found.
 *
 * Purpose:	Find which topic contains a specified locationID.
 *
 *****************************************************************************/
CEBoolean
#ifdef _NO_PROTO
_DtHelpCeFindSdlId (volume, target_id, fd, ret_name, ret_offset)
	VolumeHandle	 volume;
	char		*target_id;
	in		 fd;
	char		**ret_name;
	int		*ret_offset;
#else
_DtHelpCeFindSdlId (
	VolumeHandle	 volume,
	char		*target_id,
	int		 fd,
	char		**ret_name,
	int		*ret_offset )
#endif /* _NO_PROTO */
{
    _DtHelpVolume   vol    = (_DtHelpVolume) volume;
    CEElement	*pEl;

    pEl = _DtHelpCeMapSdlIdToElement(volume, target_id, fd);

    if (pEl != NULL)
      {
	if (ret_name != NULL)
	    *ret_name   = strdup(vol->volFile);
	*ret_offset = pEl->attributes.num_vals.offset;
	return True;
      }

    return False;
}

/*****************************************************************************
 * Function: int _DtHelpCeGetSdlKeywordList (
 *
 * Parameters:
 *
 * Returns:	0 if successful, -1 if not.
 *
 * errno Values:
 *
 * Purpose:	Get the KeywordList for an SDL volume.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlKeywordList (volume)
	VolumeHandle	   volume;
#else
_DtHelpCeGetSdlKeywordList (
	VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CESDLVolume	*sdlVol =  _DtHelpCeGetSdlVolumePtr(volume);

    if (_DtHelpCeGetSdlVolIndex(volume) != 0 || sdlVol->index == NULL
		|| _CEElementOfSegment(sdlVol->index)->seg_list == NULL)
	return -1;

    return(ProcessEntry(((_DtHelpVolume) volume),
			_CEElementOfSegment(sdlVol->index)->seg_list, NULL));
}

/*****************************************************************************
 * Function: int _DtHelpCeGetSdlVolumeAsciiAbstract(volume);
 *
 * Parameters:
 *
 * Returns:	0 if successful, -1 if not.
 *
 * errno Values:
 *
 * Purpose:	Get the KeywordList for an SDL volume.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolumeAsciiAbstract(canvas, volume)
    CanvasHandle	canvas;
    VolumeHandle	volume;
#else
_DtHelpCeGetSdlVolumeAsciiAbstract(
    CanvasHandle	canvas,
    VolumeHandle	volume)
#endif /* _NO_PROTO */
{
    return(_DtHelpCeFrmtSdlVolumeAbstractToAscii(canvas, volume));
}

/*****************************************************************************
 * Function: int _DtHelpCeGetSdlIdPath(volume, target_id, ret_ids);
 *
 * Parameters:
 *
 * Returns:	> 0 if successful, -1 if not.
 *
 * Memory:	The memory returned is owned by the caller.
 *
 * Purpose:	Get the list of location ids between the top and the
 *		target_id.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlIdPath(volume, target_id, ret_ids)
    VolumeHandle	   volume;
    char		  *target_id;
    char		***ret_ids;
#else
_DtHelpCeGetSdlIdPath(
    VolumeHandle	   volume,
    char		  *target_id,
    char		***ret_ids)
#endif /* _NO_PROTO */
{
    CESegment      *idSegs;
    CEElement	   *targetEl;
    int		    hiddenNo = -1;

    targetEl = _DtHelpCeMapSdlIdToElement(volume, target_id, -1);

    if (targetEl == NULL)
	return -1;

    *ret_ids = NULL;

    if (_DtHelpCeGetSdlVolIds(volume, -1, &idSegs) != 0)
	return NULL;

    if (_DtHelpCeGetSdlMinorNumber(volume) == SDL_DTD_1_1)
	hiddenNo = 0;

    return (MapPath(&idSegs, targetEl, -1, 0, hiddenNo, ret_ids));
}

/*****************************************************************************
 * Function: CESegment *_DtHelpCeMapSdlIdToSegment(volume, target_id);
 *
 * Parameters:
 *
 * Returns:	> 0 if successful, -1 if not.
 *
 * errno Values:
 *
 * Purpose:	Get the list of location ids between the top and the
 *		target_id.
 *
 *****************************************************************************/
CESegment *
#ifdef _NO_PROTO
_DtHelpCeMapSdlIdToSegment(volume, target_id, fd)
    VolumeHandle	   volume;
    const char		  *target_id;
    int			   fd;
#else
_DtHelpCeMapSdlIdToSegment(
    VolumeHandle	   volume,
    const char		  *target_id,
    int			   fd)
#endif /* _NO_PROTO */
{
    short	    minorNo;
    CESegment      *idSegs;
    CEElement	   *pEl;
    char	   *idString;
    char	    resStr[128] = "SDL-RESERVED-";
    CEBoolean       underScore = False;

    minorNo = _DtHelpCeGetSdlMinorNumber(volume);

    if (*target_id == '_')
      {
        if (minorNo == SDL_DTD_1_1)
          {
	    target_id++;
	    strcat(resStr, target_id);
	    target_id = resStr;
          }
	else
	    underScore = True;
      }

    if (_DtHelpCeGetSdlVolIds(volume, fd, &idSegs) != 0)
	return NULL;

    while (idSegs != NULL)
      {
	pEl = _CEElementOfSegment(idSegs);

	if (underScore == True)
	    idString = _DtHelpCeSdlRssiString(pEl);
	else
	    idString = _DtHelpCeSdlRidString(pEl);

	if (idString != NULL && _DtHelpCeStrCaseCmp(idString, target_id) == 0)
	    return idSegs;

	idSegs = idSegs->next_seg;
      }

    return NULL;
}

/*****************************************************************************
 * Function: CEElement *_DtHelpCeMapSdlIdToElement(volume, target_id);
 *
 * Parameters:
 *
 * Returns:	> 0 if successful, -1 if not.
 *
 * errno Values:
 *
 * Purpose:	Get the list of location ids between the top and the
 *		target_id.
 *
 *****************************************************************************/
CEElement *
#ifdef _NO_PROTO
_DtHelpCeMapSdlIdToElement(volume, target_id, fd)
    VolumeHandle	   volume;
    char		  *target_id;
    int			   fd;
#else
_DtHelpCeMapSdlIdToElement(
    VolumeHandle	   volume,
    char		  *target_id,
    int			   fd)
#endif /* _NO_PROTO */
{
    CESegment      *idSeg;

    idSeg = _DtHelpCeMapSdlIdToSegment(volume, target_id, fd);
    if (idSeg != NULL)
	return _CEElementOfSegment(idSeg);
    return NULL;
}

/*****************************************************************************
 * Function: int _DtHelpCeMapIdToSdlTopicId(volume, target_id);
 *
 * Parameters:
 *
 * Returns:	> 0 if successful, -1 if not.
 *
 * errno Values:
 *
 * Purpose:	Get the id of the virpage containing the target_id.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeMapIdToSdlTopicId(volume, target_id, ret_id)
    VolumeHandle	   volume;
    const char		  *target_id;
    char		 **ret_id;
#else
_DtHelpCeMapIdToSdlTopicId(
    VolumeHandle	   volume,
    const char		  *target_id,
    char		 **ret_id)
#endif /* _NO_PROTO */
{
    CESegment	*idList;
    CESegment	*idSeg;
    CEElement	*idEl;
    CEBoolean    found = False;

    if (_DtHelpCeGetSdlVolIds(volume, -1, &idList) == 0)
      {
        idSeg = _DtHelpCeMapSdlIdToSegment(volume, target_id, -1);
        if (idSeg != NULL)
          {
	    while (found == False && idList != NULL)
	      {
		idEl = _CEElementOfSegment(idList);
	        if (idEl->attributes.type == SdlIdVirpage)
	            *ret_id = _DtHelpCeSdlIdString(idEl);

		if (idList == idSeg)
		    found = True;
		else
		    idList = idList->next_seg;
	      }
	  }
	if (found == True)
	    return 0;
      }

    return -1;
}

/*****************************************************************************
 * Function: char * _DtHelpCeGetSdlVolCharSet(volume);
 *
 * Parameters:
 *
 * Returns:	the pointer to the locale string. Null otherwise.
 *
 * errno Values:
 *
 * Purpose:	Get the locale of the volume.
 *
 *****************************************************************************/
const char *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolCharSet(volume)
    VolumeHandle	   volume;
#else
_DtHelpCeGetSdlVolCharSet(
    VolumeHandle	   volume)
#endif /* _NO_PROTO */
{
    const char	   *charSet = IsoString;
    CESDLVolume    *sdlVol  =  _DtHelpCeGetSdlVolumePtr(volume);
    CEElement      *docEl;

    docEl = _DtHelpCeFindSdlElement(sdlVol->sdl_info, SdlElementSdlDoc, True);
    if (docEl != NULL)
	charSet = _DtHelpCeSdlCharSetAttr(docEl);

    return charSet;
}

/*****************************************************************************
 * Function: char * _DtHelpCeGetSdlVolLanguage(volume);
 *
 * Parameters:
 *
 * Returns:	the pointer to the language used in the volume.
 *
 * errno Values:
 *
 * Purpose:	Get the locale of the volume.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolLanguage(volume)
    VolumeHandle	   volume;
#else
_DtHelpCeGetSdlVolLanguage(
    VolumeHandle	   volume)
#endif /* _NO_PROTO */
{
    char	   *language = "C";
    CESDLVolume    *sdlVol  =  _DtHelpCeGetSdlVolumePtr(volume);
    CEElement      *docEl;

    docEl = _DtHelpCeFindSdlElement(sdlVol->sdl_info, SdlElementSdlDoc, True);
    if (docEl != NULL)
	language = _DtHelpCeSdlLangAttr(docEl);

    return language;
}

/*****************************************************************************
 * Function: char * _DtHelpCeGetSdlVolumeLocale(volume);
 *
 * Parameters:
 *
 * Returns:	the pointer to the locale string. Null otherwise.
 *
 * errno Values:
 *
 * Purpose:	Get the locale of the volume.
 *
 *****************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolumeLocale(volume)
    VolumeHandle	   volume;
#else
_DtHelpCeGetSdlVolumeLocale(
    VolumeHandle	   volume)
#endif /* _NO_PROTO */
{
    int		    langLen;
    char	   *locale;
    char	   *lang;
    const char	   *charSet;

    lang    = _DtHelpCeGetSdlVolLanguage(volume);
    charSet = _DtHelpCeGetSdlVolCharSet(volume);

    langLen = strlen(lang);
    locale  = (char *) malloc (langLen + strlen(charSet) + 2);
    if (locale != NULL)
      {
	strcpy(locale, lang);
	if (langLen != 0 && *charSet != '\0')
	  {
	    locale[langLen++] = '.';
	    strcpy(&(locale[langLen]), charSet);
	  }
      }

    return locale;
}

/*****************************************************************************
 * Function: CEElement *_DtHelpCeFindSdlElement(
 *
 * Parameters:
 *
 * Returns:	pointer to the element, Null otherwise.
 *
 * errno Values:
 *
 * Purpose:	Find the specified element.
 *
 *****************************************************************************/
CEElement *
#ifdef _NO_PROTO
_DtHelpCeFindSdlElement(seg_list, target, depth)
    CESegment		*seg_list;
    enum SdlElement	 target;
    CEBoolean		 depth;
#else
_DtHelpCeFindSdlElement(
    CESegment		*seg_list,
    enum SdlElement	 target,
    CEBoolean		 depth)
#endif /* _NO_PROTO */
{
    CEElement	*pEl = NULL;

    while (seg_list != NULL)
      {
	if (_CEIsElement(seg_list->seg_type) &&
			_CEElementOfSegment(seg_list) != NULL)
	  {
	    pEl = _CEElementOfSegment(seg_list);
	    if (pEl->el_type == target)
		return pEl;
	    else if (depth == True)
	      {
		pEl = _DtHelpCeFindSdlElement(pEl->seg_list, target, depth);
		if (pEl != NULL)
		    return pEl;
	      }
	  }
	seg_list = seg_list->next_seg;
      }

    return NULL;
}

/*****************************************************************************
 * Function: int _DtHelpCeGetSdlDocStamp(volume, ret_doc, ret_time);
 *
 * Parameters:
 *
 * Returns:	0 if successful, -2 if the volume does not contain
 *		one or the other, -1 if any other failure.
 *
 * Memory:	The Caller owns the memory returned in ret_doc and ret_time.
 *
 * Purpose:	Get the doc id and time stamp of a volume.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlDocStamp(volume, ret_doc, ret_time)
    VolumeHandle	   volume;
    char		**ret_doc;
    char		**ret_time;
#else
_DtHelpCeGetSdlDocStamp(
    VolumeHandle	   volume,
    char		**ret_doc,
    char		**ret_time)
#endif /* _NO_PROTO */
{
    int		    result    = -1;
    char	   *docId     = NULL;
    char	   *timestamp = NULL;
    CESDLVolume    *sdlVol    =  _DtHelpCeGetSdlVolumePtr(volume);
    CEElement      *docEl;

    docEl = _DtHelpCeFindSdlElement(sdlVol->sdl_info, SdlElementSdlDoc, True);
    if (docEl != NULL)
      {
	result = 0;
	if (_CEIsAttrSet(docEl->attributes, SDL_ATTR_DOCID))
	    docId = strdup(_DtHelpCeSdlDocIdAttr(docEl));
	else
	    result = -2;

	if (_CEIsAttrSet(docEl->attributes, SDL_ATTR_TIMESTAMP))
	    timestamp = strdup(_DtHelpCeSdlTimeStampAttr(docEl));
	else
	    result = -2;
      }

    if (ret_doc != NULL)
	*ret_doc = docId;
    if (ret_time != NULL)
	*ret_time = timestamp;

    if (result == 0 && (docId == NULL || timestamp == NULL))
	return -1;

    return result;
}

/*****************************************************************************
 * Function: int _DtHelpCeGetSdlTopicChildren(
 *
 * Parameters:
 *
 * Returns:	pointer to the element, Null otherwise.
 *
 * errno Values:
 *
 * Purpose:	Find the specified element.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlTopicChildren(volume, target, ret_ids)
    VolumeHandle	 volume;
    char		*target;
    char		***ret_ids;
#else
_DtHelpCeGetSdlTopicChildren(
    VolumeHandle	 volume,
    char		*target,
    char		***ret_ids)
#endif /* _NO_PROTO */
{
    int		 count  = 0;
    int		 segLev;
    CESegment	*idSeg;
    CEElement	*pEl;
    CEBoolean	 done   = False;

    /*
     * Find the target id.
     */
    idSeg = _DtHelpCeMapSdlIdToSegment(volume, target, -1);

    /*
     * save this level and start looking for its children at the next seg.
     */
    *ret_ids = NULL;
    if (idSeg != NULL)
      {
	segLev = _DtHelpCeSdlLevel(_CEElementOfSegment(idSeg)) + 1;
	idSeg  = idSeg->next_seg;
      }
    /*
     * process any virpage that has the correct level
     */
    while (idSeg != NULL && done == False)
      {
	pEl    = _CEElementOfSegment(idSeg);
	if (pEl->attributes.type == SdlIdVirpage)
	  {
	    /*
	     * If greater, we're at the next sibling.
	     */
	    if (segLev > _DtHelpCeSdlLevel(pEl))
		done = True;
	    else if (segLev == _DtHelpCeSdlLevel(pEl))
	      {
		*ret_ids = (char **) _DtHelpCeAddPtrToArray( (void **) *ret_ids,
				(void *)(strdup(_DtHelpCeSdlRidString(pEl))));
		if ((*ret_ids) == NULL)
		    return -1;

		count++;
	      }
	  }
	idSeg = idSeg->next_seg;
      }

    return count;
}

/*******************************************************************************
 * Function:    void _DtHelpCeGetSdlMinorNumber (VolumeHandle vol);
 *
 * Parameters:  vol     Specifies the loaded volume.
 *
 * Return Value:        minorNo number of the volume if successful,
 *                      -1 if a failure occurs
 *
 * errno Values:        None
 *
 * Purpose:     Return the minorNo number of the sdl dtd used for this volume.
 *
 ******************************************************************************/
short
#ifdef _NO_PROTO
_DtHelpCeGetSdlMinorNumber (volume)
     VolumeHandle	 volume;
#else
_DtHelpCeGetSdlMinorNumber (
     VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    short        number = -1;
    CESDLVolume	*sdlVol = _DtHelpCeGetSdlVolumePtr(volume);

    if (sdlVol != NULL)
	number = sdlVol->minor_no;

    return number;
}
