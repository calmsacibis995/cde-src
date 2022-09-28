/* $XConsortium: FormatTerm.c /main/cde1_maint/3 1995/10/02 14:10:54 lehors $ */
/************************************<+>*************************************
 ****************************************************************************
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>


extern	int errno;

/*
 * private includes
 */

#include "HelpTermP.h"
#include "HelpP.h"
#include "HelpI.h"
#include "bufioI.h"
#include "AsciiSpcI.h"
#include "Canvas.h"
#include "Access.h"
#include "CESegmentI.h"
#include "AccessI.h"
#include "CanvasI.h"
#include "HelpErrorP.h"
#include "FileUtilsI.h"
#include "StringFuncsI.h"

extern	int	_DtHelpCeParseSdlTopic(
#ifndef	_NO_PROTO
				CanvasHandle     canvas_handle,
				VolumeHandle     volume,
				int              fd,
				int              offset,
				char            *id_string,
				CEBoolean        rich_text,
				TopicHandle     *ret_handle
#endif /* _NO_PROTO */
				);
extern	int	_DtHelpCeFrmtCcdfTopic(
#ifndef	_NO_PROTO
				CanvasHandle	 canvas_handle,
				VolumeHandle	 volume,
				char		*filename,
				int		 offset,
				char		*id_string,
				TopicHandle	*ret_topic
#endif /* _NO_PROTO */
				);

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#ifdef	XTAG
static	VStatus		TermFindGraphic();
#endif
static	void		TermMetrics();
static	Unit		TermStrWidth();
static	void		TermStrDraw();
static	void		TermGetFont();
static	void		TermFontMetrics();
static	VStatus		TermResolveSpc();
static	void		TermDrawSpc();
#else
static	void		TermMetrics(
				ClientData		 client_data,
				CanvasMetrics		*ret_canvas,
				CanvasFontMetrics	*ret_font,
				CanvasLinkMetrics	*ret_link,
				CanvasTraversalMetrics	*ret_traversal);
static	Unit		TermStrWidth(
				ClientData	 client_data,
				char		*string,
				int		 byte_len,
				int		 char_len,
				FontPtr		 font_ptr );
static	void		TermStrDraw(
				ClientData	 client_data,
				Unit		 x,
				Unit		 y,
				const char	*string,
				int		 byte_len,
				int		 char_len,
				FontPtr		 font_ptr,
				Unit		 box_x,
				Unit		 box_y,
				Unit		 box_height,
				int		 link_type,
				Flags		 old_flags,
				Flags		 new_flags );
#ifdef	XTAG
static	VStatus		TermFindGraphic(
				ClientData	 client_data,
				char		*vol_xid,
				char		*topic_xid,
				char		*file_xid,
				char		*format,
				char		*method,
				Unit		*ret_width,
				Unit		*ret_height,
				GraphicPtr	*ret_graphic);
#endif
static	void		TermGetFont(
				ClientData	 client_data,
				char		*lang,
				char		*charset,
				CanvasFontSpec	 font_attr,
				char		**mod_string,
				FontPtr		*ret_font );
static	void		TermFontMetrics(
				ClientData	 client_data,
				FontPtr		 font_ptr,
				Unit		*ret_ascent,
				Unit		*ret_descent,
				Unit		*char_width,
				Unit		*ret_super,
				Unit		*ret_sub);
static	VStatus		TermResolveSpc(
				ClientData	 client_data,
				char		*lang,
				char		*charset,
				CanvasFontSpec	 font_attr,
				char		*spc_symbol,
				SpecialPtr	*ret_handle,
				Unit		*ret_width,
				Unit		*ret_height,
				Unit		*ret_ascent);
static	void		TermDrawSpc(
				ClientData	 client_data,
				SpecialPtr	 spc_handle,
				Unit		 x,
				Unit		 y,
				Unit		 box_x,
				Unit		 box_y,
				Unit		 box_height,
				int		 link_type,
				Flags		 old_flags,
				Flags		 new_flags );
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
#define	GROW_SIZE	10
#define EOS		'\0'

/*
 * My private values
 */
static	VolumeHandle	 MyVolume     = NULL;
static	int		 HyperErr     = 0;
static	wchar_t		 WcSpace      = 0;

/*
 * These values change as the information is processed.
 */
typedef	struct {
	wchar_t **lines;
	size_t   *wc_num;
	int	  max_lines;
	Unit	  max_columns;
	Unit	  max_rows;
} TerminalInfo;

/* default values */
static TerminalInfo	DfltTermInfo   = {NULL, NULL, 0, 1, 100000};
static VirtualInfo	TermVirtInfo =
  {
	TermMetrics,		/* void      (*_CEGetCanvasMetrics)(); */
	TermStrWidth,		/* Unit      (*_CEGetStringWidth)(); */
	TermStrDraw,		/* void      (*_CEDrawString)(); */
	NULL,			/* void      (*_CEDrawLine)(); */
	TermGetFont,		/* void      (*_CEGetFont)(); */
	TermFontMetrics,	/* void      (*_CEGetFontMetrics)(); */
#ifdef	XTAG
	TermFindGraphic,	/* VStatus   (*_CELoadGraphic)(); */
#else
	NULL,			/* VStatus   (*_CELoadGraphic)(); */
#endif
	NULL,			/* void      (*_CEDrawGraphic)(); */
	NULL,			/* void      (*_CEDestroyGraphic)(); */
	TermResolveSpc,		/* VStatus   (*_CEResolveSpecial)(); */
	TermDrawSpc,		/* void      (*_CEDrawSpecial)(); */
	NULL,			/* void      (*_CEDestroySpecial)(); */
	NULL,			/* VStatus   (*_CEBuildSelectionString)(); */
	NULL,			/* VStatus   (*_CEBuildSelectionGraphic)(); */
	NULL,			/* VStatus   (*_CEBuildSelectionSpc)(); */
  };

static TerminalInfo	MyInfo   = {NULL, NULL, 0, 1, 100000};
static CanvasHandle	MyCanvas = NULL;

#ifdef  XTAG
int     missingGraphics = 0;  /* Counter used by cdewalk */
#endif  /* XTAG */

/******************************************************************************
 *
 * Private functions
 *
 ******************************************************************************/
/******************************************************************************
 * Canvas functions
 ******************************************************************************/
#ifdef	XTAG
/*****************************************************************************
 * Function:	VStatus TermFindGraphic ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static VStatus
#ifdef _NO_PROTO
TermFindGraphic (client_data, vol_xid, topic_xid, file_xid, format, method,
		ret_width, ret_height, ret_graphic)
    ClientData	 client_data;
    char	*vol_xid;
    char	*topic_xid;
    char	*file_xid;
    char	*format;
    char	*method;
    Unit	*ret_width;
    Unit	*ret_height;
    GraphicPtr	*ret_graphic;
#else
TermFindGraphic (
    ClientData	 client_data,
    char	*vol_xid,
    char	*topic_xid,
    char	*file_xid,
    char	*format,
    char	*method,
    Unit	*ret_width,
    Unit	*ret_height,
    GraphicPtr	*ret_graphic )
#endif /* _NO_PROTO */
{
    char		 *ptr;
    char		 *fileName = file_xid;

    if (fileName != NULL && *fileName != '/')
      {
	fileName = (char *) malloc (strlen(vol_xid) + strlen (file_xid) + 2);
	if (fileName == NULL)
	    return -1;

	strcpy(fileName, vol_xid);

	if (_DtHelpCeStrrchr(fileName, "/", MB_CUR_MAX, &ptr) != -1)
	    *ptr = '\0';

	strcat(fileName, "/");
	strcat(fileName, file_xid);
      }

    if (access(fileName, R_OK) != 0)
      {
	missingGraphics++;
	fprintf (stdout, "\tGRAPHICS LINK            ***UNRESOLVED***\n");
	fprintf (stdout, "\tUnable to find graphic file: %s\n\n", fileName);
      }

    if (fileName != file_xid)
	free (fileName);

    *ret_width   = 0;
    *ret_height  = 0;
    *ret_graphic = 0;

    return 0;
}
#endif /* XTAG */

/*****************************************************************************
 * Function:	void TermStrDraw ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
TermStrDraw (client_data, x, y, string, byte_len, char_len, font_ptr,
		box_x, box_y, box_height, link_type, old_flags, new_flags)
    ClientData	 client_data;
    Unit	 x;
    Unit	 y;
    const char	*string;
    int		 byte_len;
    int		 char_len;
    FontPtr	 font_ptr;
    Unit	 box_x;
    Unit	 box_y;
    Unit	 box_height;
    int		 link_type;
    Flags	 old_flags;
    Flags	 new_flags;
#else
TermStrDraw (
    ClientData	 client_data,
    Unit	 x,
    Unit	 y,
    const char	*string,
    int		 byte_len,
    int		 char_len,
    FontPtr	 font_ptr,
    Unit	 box_x,
    Unit	 box_y,
    Unit	 box_height,
    int		 link_type,
    Flags	 old_flags,
    Flags	 new_flags )
#endif /* _NO_PROTO */
{
    TerminalInfo   *pTerm  = (TerminalInfo *) client_data;
    wchar_t	   *wcStr  = pTerm->lines[y];
    size_t	    wcLen  = byte_len;
    size_t	    length = 1;

    length = x + wcLen;
    if (wcStr == NULL)
      {
	pTerm->lines[y] = (wchar_t *) malloc (sizeof(wchar_t) * (length + 1));
	if (pTerm->lines[y] != NULL)
	  {
	    register i;

	    wcStr = pTerm->lines[y];
	    for (i = 0; i < x; i++)
		wcStr[i] = WcSpace;

	    /*
	     * this will leave a hole that will be plugged by the next 
	     * mbstowc call.
	     */
	    wcStr[length] = 0;
	    pTerm->wc_num[y] = length;
	  }
      }
    else
      {
        if (length > pTerm->wc_num[y])
          {
	    pTerm->lines[y] = (wchar_t *) realloc (wcStr,
					(sizeof(wchar_t) * (length + 1)));
	    if (pTerm->lines[y] != NULL)
	      {
	        register i;
	        wcStr = pTerm->lines[y];
	        for (i = pTerm->wc_num[y]; i < x; i++)
		    wcStr[i] = WcSpace;

	        wcStr[length] = 0;
	        pTerm->wc_num[y] = length;
	      }
          }
      }

    mbstowcs(&wcStr[x], string, wcLen);
}

/*****************************************************************************
 * Function:	void TermMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	CanvasFontMetrics	defFontMetrics = { 1, 0, 1 };
static	CanvasLinkMetrics	defLinkMetrics = { 0, 0, 0, 0 };

static	void
#ifdef _NO_PROTO
TermMetrics (client_data, ret_canvas, ret_font, ret_link, ret_traversal)
    ClientData			 client_data;
    CanvasMetrics		*ret_canvas;
    CanvasFontMetrics		*ret_font;
    CanvasLinkMetrics		*ret_link;
    CanvasTraversalMetrics	*ret_traversal;
#else
TermMetrics (
    ClientData			 client_data,
    CanvasMetrics		*ret_canvas,
    CanvasFontMetrics		*ret_font,
    CanvasLinkMetrics		*ret_link,
    CanvasTraversalMetrics	*ret_traversal)
#endif /* _NO_PROTO */
{
    TerminalInfo *pTerm = (TerminalInfo *) client_data;

    if (ret_canvas)
      {
        ret_canvas->width          = pTerm->max_columns;
        ret_canvas->height         = 50;
	ret_canvas->leading        = 0;
	ret_canvas->top_margin     = 0;
	ret_canvas->line_thickness = 0;
      }
    if (ret_font)
	*ret_font = defFontMetrics;

    if (ret_link)
	*ret_link = defLinkMetrics;

    if (ret_traversal)
	*ret_traversal = defLinkMetrics;
}

/*****************************************************************************
 * Function:	void TermGetFont ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
TermGetFont (client_data, lang, charset, font_attr, mod_string, ret_font)
    ClientData		 client_data;
    char		*lang;
    char		*charset;
    CanvasFontSpec	 font_attr;
    char		**mod_string;
    FontPtr		*ret_font;
#else
TermGetFont (
    ClientData		 client_data,
    char		*lang,
    char		*charset,
    CanvasFontSpec	 font_attr,
    char		**mod_string,
    FontPtr		*ret_font )
#endif /* _NO_PROTO */
{
    *ret_font = (FontPtr) 0;
}

/*****************************************************************************
 * Function:	VStatus TermResolveSpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	VStatus
#ifdef _NO_PROTO
TermResolveSpc (client_data, lang, charset, font_attr, spc_symbol,
		   ret_handle, ret_width, ret_height, ret_ascent)
    ClientData		 client_data;
    char		*lang;
    char		*charset;
    CanvasFontSpec	 font_attr;
    char		*spc_symbol;
    SpecialPtr		*ret_handle;
    Unit		*ret_width;
    Unit		*ret_height;
    Unit		*ret_ascent;
#else
TermResolveSpc (
    ClientData		 client_data,
    char		*lang,
    char		*charset,
    CanvasFontSpec	 font_attr,
    char		*spc_symbol,
    SpecialPtr		*ret_handle,
    Unit		*ret_width,
    Unit		*ret_height,
    Unit		*ret_ascent)
#endif /* _NO_PROTO */
{
    const char*	spcStr;

    spcStr = _DtHelpCeResolveSpcToAscii (spc_symbol);

    if (spcStr != NULL)
      {
	*ret_handle = ((SpecialPtr)(spcStr));
	*ret_height = 1;
	*ret_ascent = 0;
	*ret_width  = strlen(spcStr);

	return 0;
      }

    return -1;
}

/*****************************************************************************
 * Function:	void TermFontMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
TermFontMetrics (client_data, font_ptr, ret_ascent, ret_descent, char_width)
    ClientData	 client_data;
    FontPtr	 font_ptr;
    Unit	*ret_ascent;
    Unit	*ret_descent;
    Unit	*char_width;
    Unit	*ret_super;
    Unit	*ret_sub;
#else
TermFontMetrics (
    ClientData	 client_data,
    FontPtr	 font_ptr,
    Unit	*ret_ascent,
    Unit	*ret_descent,
    Unit	*char_width,
    Unit	*ret_super,
    Unit	*ret_sub)
#endif /* _NO_PROTO */
{
    if (ret_ascent)
	*ret_ascent = 0;
    if (ret_descent)
	*ret_descent = 0;
    if (char_width)
	*char_width = 0;
    if (ret_super)
	*ret_super = 0;
    if (ret_sub)
	*ret_sub = 0;

    return;
}

/*****************************************************************************
 * Function:	Unit TermStrWidth ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	Unit
#ifdef _NO_PROTO
TermStrWidth (client_data, string, byte_len, char_len, font_ptr)
    ClientData	 client_data;
    char	*string;
    int		 byte_len;
    int		 char_len;
    FontPtr	 font_ptr;
#else
TermStrWidth (
    ClientData	 client_data,
    char	*string,
    int		 byte_len,
    int		 char_len,
    FontPtr	 font_ptr )
#endif /* _NO_PROTO */
{
   return ((Unit)(byte_len/char_len));
}

/*****************************************************************************
 * Function:	void TermDrawSpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
TermDrawSpc (client_data, spc_handle, x, y, box_x, box_y, box_height,
		link_type, old_flags, new_flags)
    ClientData	 client_data;
    SpecialPtr	 spc_handle;
    Unit	 x;
    Unit	 y;
    Unit	 box_x;
    Unit	 box_y;
    Unit	 box_height;
    int		 link_type;
    Flags	 old_flags;
    Flags	 new_flags;
#else
TermDrawSpc (
    ClientData	 client_data,
    SpecialPtr	 spc_handle,
    Unit	 x,
    Unit	 y,
    Unit	 box_x,
    Unit	 box_y,
    Unit	 box_height,
    int		 link_type,
    Flags	 old_flags,
    Flags	 new_flags )
#endif /* _NO_PROTO */
{
    const char *spcStr = (const char*) spc_handle;

    TermStrDraw (client_data, x, y, spcStr, strlen(spcStr), 1, 0,
			box_x, box_y, box_height, link_type, old_flags,
			new_flags);
}
/******************************************************************************
 * End Canvas functions
 * Begin other private functions
 ******************************************************************************/

/******************************************************************************
 * Function: DtHelpHyperLines *AddHyperToArray (DtHelpHyperLines *array_ptr,
 *				int value, char *link_spec, char *title)
 *
 * Parameters:
 *		array_ptr	Specifies a NULL terminated list of
 *				DtHelpHyperLines or NULL.
 *		value		Specifies the link type.
 *		link_spec	Specifies the link specification.
 *		title		Specifies the title of the link.
 *
 * Returns:	A ptr to the new DtHelpHyperLines if successful,
 *		NULL if errors.
 *
 * errno Values:
 *		DtErrorMalloc
 *
 * Purpose:	Add a hypertext link to an array of DtHelpHyperLines.
 * 
 *****************************************************************************/
static	DtHelpHyperLines *
#ifdef _NO_PROTO
AddHyperToArray(array_ptr, value, win_hint, link_spec, title)
	DtHelpHyperLines *array_ptr;
	int		 value;
	int		 win_hint;
	char		*link_spec;
	char		*title;
#else
AddHyperToArray(
	DtHelpHyperLines *array_ptr,
	int		 value,
	int		 win_hint,
	char		*link_spec,
	char		*title )
#endif /* _NO_PROTO */
{
    DtHelpHyperLines *next;
    int            num = 0;


    if (array_ptr)
      {
	for (next = array_ptr; next->specification != NULL; next++)
		num++;

	if (((num + 1) % GROW_SIZE) == 0)
	    array_ptr = (DtHelpHyperLines *) realloc ((void *) array_ptr,
			(sizeof (DtHelpHyperLines) * (num + 1 + GROW_SIZE)));
      }
    else
	array_ptr = (DtHelpHyperLines *) malloc (
					sizeof (DtHelpHyperLines) * GROW_SIZE);

    if (array_ptr == NULL)
	errno = DtErrorMalloc;
    else
      {
	next = array_ptr + num;

        next->hyper_type    = value;
        next->win_hint      = win_hint;
        next->specification = strdup(link_spec);
        next->title         = title;
        next++;

        next->specification = NULL;
        next->title         = NULL;
      }

    return array_ptr;
}

/******************************************************************************
 * Function:  void DeallocateHyperArray (DtHelpHyperLines *array_ptr)
 *
 * Parameters:
 *		array_ptr	Specifies a NULL terminated array of
 *				DtHelpHyperLines.
 *
 * Returns:	Nothing
 *
 * errno Values:
 *
 * Purpose:	De-allocate an array of DtHelpHyperLines structures.
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
DeallocateHyperArray(array_ptr)
	DtHelpHyperLines	*array_ptr;
#else
DeallocateHyperArray(
	DtHelpHyperLines	*array_ptr )
#endif /* _NO_PROTO */
{
    register DtHelpHyperLines *next = array_ptr;

    if (array_ptr)
      {
        while (next->title && next->specification)
	  {
	    free (next->title);
	    free (next->specification);
	    next++;
	  }
	free (array_ptr);
      }
}

/******************************************************************************
 * Function: int AddHyperToList (
 * 
 * Parameters:
 *
 * Returns:	 0 if successful.
 *		-1 if unrecoverable errors.
 *		-2 if could not resolve the hypertext link.
 *
 * errno Values:
 *		DtErrorMalloc
 *		DtErrorFormattingLabel
 *				A label has illegal syntax.
 *		DtErrorHyperType
 *				Invalid (negative) hypertype.
 *		DtErrorFormattingLink
 *				Invalid <LINK>.
 *		DtErrorHyperSpec
 *				Invalid 'hyper_specification" in the
 *				the <LINK>.
 *		DtErrorFormattingId
 *				Invalid <ID> syntax.
 *
 * Purpose:	Process the result of a hypertext link, filling out
 *		a hypertext structure element with the information.
 *			
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
AddHyperToList (volume, canvas, i, ret_list)
    VolumeHandle volume;
    CanvasHandle canvas,
    int		 i;
    DtHelpHyperLines	**ret_list;
#else
AddHyperToList(
    VolumeHandle volume,
    CanvasHandle canvas,
    int		 i,
    DtHelpHyperLines	**ret_list)
#endif /* _NO_PROTO */
{
    CECanvasStruct	*myCanvas = (CECanvasStruct *)canvas;
    CEHyperTextCallbackStruct hyperInfo;
    VolumeHandle        newVol     = NULL;
    VolumeHandle        useVol     = volume;
    char		*volName   = NULL;
    char		*title     = NULL;
    char		*allocName = NULL;
    char		*spec      = NULL;
    char		*id;
    int                 result    = -3;

    if (_DtHelpCeGetLinkInfo(canvas, myCanvas->link_data, i, &hyperInfo) == 0)
      {
	result = 0;
	if (hyperInfo.description == NULL)
	  {
	    id = hyperInfo.specification;
	    switch (hyperInfo.hyper_type)
	      {
	        case CELinkType_CrossLink:
			spec = strdup (id);
			if (spec != NULL)
			  {
			    volName = spec;
                            id = NULL;
                            _DtHelpCeStrchr (spec," ",MB_CUR_MAX,&id);
			    if (id != NULL)
			      {
				*id = '\0';
				id++;

                                /* find the volume (volName is malloc'd) */
                                allocName = _DtHelpFileLocate(DtHelpVOLUME_TYPE, volName,
                                                 _DtHelpFileSuffixList,False,R_OK);
                                if (allocName == NULL) 
                                     result = -2;

				if (_DtHelpCeOpenVolume(canvas, allocName, &newVol)==0)
				    useVol = newVol;
				else
				    result = -2;
			      }
			    else
				id = spec;
			  }
			else
			    result = -1;
                        /* fall thru */
	        case CELinkType_SameVolume:
			if (result == 0)
			  {
			    result = _DtHelpCeGetTopicTitle(canvas, useVol,
								id, &title);
			    if (result == -2)
				HyperErr = DtErrorLocIdNotFound;
			  }

			if (newVol != NULL)
			    _DtHelpCeCloseVolume(canvas, newVol);
			if (spec != NULL)
			    free(spec);
			if (allocName != NULL)
			    free(allocName);

			break;

	        case CELinkType_Execute:
			title = (char *) malloc(strlen(id) + 11);
			sprintf(title, "Execute \"%s\"", id);
			break;

	        case CELinkType_ManPage:
			title = (char *) malloc(strlen(id) + 13);
			sprintf(title, "Man Page \"%s\"", id);
			break;

	        case CELinkType_AppDefine:
			title = (char *) malloc(strlen(id) + 26);
		        sprintf(title, "Application Link Type \"%s\"", id);
			break;

	        case CELinkType_TextFile:
			title = (char *) malloc(strlen(id) + 12);
		        sprintf(title, "Text File \"%s\"", id);
			break;
	        default:
			title = strdup ("Unkown link type");
			break;

	      }
	  }
	else
	  {
	    title = strdup (hyperInfo.description);
	  }
      }

    if (result == -2)
	*ret_list = AddHyperToArray (*ret_list, -(hyperInfo.hyper_type + 1),
				hyperInfo.win_hint,
				hyperInfo.specification,
				strdup("Invalid Link"));
    else if (result >= 0)
	*ret_list = AddHyperToArray (*ret_list, hyperInfo.hyper_type,
				hyperInfo.win_hint,
				hyperInfo.specification,
				title);

    /*
     * report unable to resolve the hypertext link
     */
    if (result == -2)
	return -2;

    if (result == -1 || *ret_list == NULL)
	return -1;

    return (0);

} /* End AddHyperToList */

/******************************************************************************
 * Function: TerminalInfo * GetTermInfo(canvasHandle)
 *
 * Parameters:
 *		canvasHandle	Canvas whose client_data is a TerminalInfo *
 *
 * Returns:	0 for success, -1 for failure.
 *
 ******************************************************************************/
static TerminalInfo * 
#if  _NO_PROTO
GetTermInfo(canvasHandle)
           CanvasHandle canvasHandle;
#else
GetTermInfo(
           CanvasHandle canvasHandle)
#endif  /* _NO_PROTO */
{
   CECanvasStruct * canvas = (CECanvasStruct *) canvasHandle;

   return (TerminalInfo *) canvas->client_data;
}

/******************************************************************************
 *
 * Public functions
 *
 ******************************************************************************/
/******************************************************************************
 * Function: int _DtHelpTermCreateCanvas (int maxColumns,CanvasHandle * ret_canvas)
 *
 * Parameters:
 *		maxColumns	Specifies the column width of the window
 *				for which to format the information.
 *		ret_canvas	handle to the canvas that was created
 *
 * Returns:	0 for success, -1 for failure.
 *
 * errno Values:
 *		EINVAL		'ret_canvas' was NULL or 'maxColumns'
 *				was less than one.
 *              ENOMEM		unable to allocate necessary memory
 *              DtErrorMalloc
 *
 * Purpose:	_DtHelpTermCreateCanvas creates a canvas that use 
 *		text-only content processing routines
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpTermCreateCanvas(maxColumns,ret_canvas)
	int		  maxColumns;
        CanvasHandle *    ret_canvas;
#else
_DtHelpTermCreateCanvas (
	int		  maxColumns,
        CanvasHandle *    ret_canvas)
#endif /* _NO_PROTO */
{
    TerminalInfo * termInfo;

    /*
     * check the parameters
     */
    if (maxColumns < 1 || ret_canvas == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    termInfo = (TerminalInfo *) malloc(sizeof(TerminalInfo));
    if (termInfo == NULL)
    {
	errno = ENOMEM;
	return -1;
    }

     /* init info and create a canvas */
    *termInfo = DfltTermInfo;
    termInfo->max_columns = maxColumns;
    *ret_canvas  = _DtHelpCeCreateCanvas (TermVirtInfo, (ClientData) termInfo);

    if (*ret_canvas == NULL)
	    return -1;

    return 0;
}


/******************************************************************************
 * Function: int _DtHelpTermGetTopicData(canvasHandle,volHandle,
 *                                           locationId,helpList,hyperList)
 *
 * Parameters:
 *              canvasHandle    Canvas used to retrieve the info; MUST
 *				be a Terminal canvas Since it isn't easy
 *				to verify this, we don't try.  So if the
 *				Canvas ISN'T a Terminal Canvas, we'll
 *				crash.
 *              volHandle	Help volume to use
 *		locationId	Specifies the locationId of the desired topic.
 *		helpList	Returns a NULL terminated array of
 *				strings.
 *		hyperList	Returns a NULL terminated array of
 *				DtHelpHyperLines containing the hyperlinks
 *				found in the topic.
 *
 * Returns:	0 for success, -1 for failure.
 *
 * errno Values:
 *		EINVAL		'helpVolume', 'locationId', 'helpList',
 *				or 'hyperList' were NULL. 'maxColumns'
 *				was less than one.
 *              open(2)		errno set via an open call on
 *				the file for 'locationId'.
 *              DtErrorMalloc
 *              DtErrorExceedMaxSize
 *				When following symbolic links, the
 *				new path will exceed the system
 *				maximum file path length.
 *              DtErrorIllegalPath
 *				When following symbolic links, the
 *				new path would change to a parent
 *                              directory beyond the beginning
 *                              of the base path.
 *              DtErrorIllegalDatabaseFile
 *                              Specifies that 'helpVolume' is
 *                              an illegal database file.
 *		DtErrorMissingFilenameRes
 *                              Specifies that the 'Filename/filename'
 *                              resource for the topic does not exist.
 *              DtErrorMissingFileposRes
 *                              Specifies that the 'Filepos/filepos'
 *                              resource for the topic does not exist.
 *              DtErrorLocIdNotFound
 *                              Specifies that 'locationId' was not found.
 *		DtErrorFormattingLabel
 *				A label has illegal syntax.
 *		DtErrorHyperType
 *				Invalid (negative) hypertype.
 *		DtErrorFormattingLink
 *				Invalid <LINK>.
 *		DtErrorHyperSpec
 *				Invalid 'hyper_specification" in the
 *				the <LINK>.
 *		DtErrorFormattingId
 *				Invalid <ID> syntax.
 *		DtErrorFormattingTitle
 *				Invalid <TITLE> syntax.
 *
 * Purpose:	_DtHelpTermGetTopicData retrieves Help Files content with
 *		in a form understood by a terminal
 *
 *****************************************************************************/
int
#if _NO_PROTO
_DtHelpTermGetTopicData(canvasHandle,volHandle,locationId,helpList,hyperList)
    CanvasHandle      canvasHandle;
    VolumeHandle      volHandle;
    char *            locationId;
    char * * *	      helpList;
    DtHelpHyperLines ** hyperList;
#else
_DtHelpTermGetTopicData(
    CanvasHandle      canvasHandle,
    VolumeHandle      volHandle,
    char *            locationId,
    char * * *	      helpList,
    DtHelpHyperLines ** hyperList)
#endif /* _NO_PROTO */
{
    int		 result = -1;
    int		 offset;
    Unit	 maxRows;
    Unit	 maxWidth;
    char * *     strList;
    char*        fileName = NULL;
    _DtHelpCeLockInfo lockInfo;
    TopicHandle	 topic;
    TerminalInfo * termInfo;

    termInfo = GetTermInfo(canvasHandle);

    if (WcSpace == 0)
	mbtowc (&WcSpace, " ", 1);

    /*
     * find the filename and the Id string.
     */
    if (_DtHelpCeLockVolume(canvasHandle, volHandle, &lockInfo) != 0)
      {
	return -1;
      }

    if (_DtHelpCeFindId(volHandle,locationId,lockInfo.fd,&fileName,&offset)==0)
      {
	_DtHelpCeUnlockVolume(lockInfo);
	return -1;
      }

    /*
     * Format the topic.
     */
    result = _DtHelpCeGetVolumeFlag(volHandle);
    if (result == 1)
	result = _DtHelpCeParseSdlTopic(canvasHandle, volHandle, lockInfo.fd,
						offset, NULL, True, &topic);
    else if (result == 0)
	result = _DtHelpCeFrmtCcdfTopic(canvasHandle, volHandle, fileName,
							offset, NULL, &topic);
    /* if successfully formatted topic */
    if (result != -1)
    {
	int	i;
	int	len;
	wchar_t **wcList;
	CECanvasStruct *myCStruct = (CECanvasStruct *)canvasHandle;

        /* build the help text list, if requested */
        if (NULL != helpList)
        {
           _DtHelpCeSetTopic(canvasHandle,topic,-1, &maxWidth, &maxRows, NULL);
   
           termInfo->lines  = (wchar_t **) malloc(sizeof(wchar_t *) * maxRows);
           termInfo->wc_num = (size_t   *) malloc(sizeof(size_t)    * maxRows);
           strList          = (char **)    malloc(sizeof(char *) * (maxRows+1));
   	   if (termInfo->lines == NULL || termInfo->wc_num == NULL ||
							strList == NULL)
           {
   	      free(fileName);
	      if (termInfo->lines != NULL)
		free(termInfo->lines);
	      if (termInfo->wc_num != NULL)
		free(termInfo->wc_num);
	      if (strList != NULL)
		free(strList);
   	      _DtHelpCeUnlockVolume(lockInfo);
   	      return -1;
           }
   
   	   for (i = 0; i < maxRows; i++)
	     {
   	       termInfo->lines[i]  = NULL;
   	       termInfo->wc_num[i] = 0;
	     }
   
   	   _DtHelpCeScrollCanvas(canvasHandle, 0, 0, maxWidth, maxRows);
   
   	   *helpList = strList;
   	   wcList    = termInfo->lines;
   	   for (i = 0; i < maxRows; i++, wcList++, strList++)
   	   {
   	      if (*wcList == NULL)
   	      {
   	  	 *strList  = (char *) malloc (1);
   		 **strList = '\0';
   	      }
	      else
	      {
		len      = (termInfo->wc_num[i] + 1) * MB_CUR_MAX;
		*strList = (char *) malloc (sizeof (char) * len);

		if (*strList != NULL)
		    wcstombs(*strList, *wcList, len);
	      }
   	   }
   
	  *strList = NULL;

	   /*
	    * free the allocated memory
	    */
	   for (i = 0, wcList = termInfo->lines; i < maxRows; i++, wcList++)
	       if (*wcList != NULL)
		   free(*wcList);
	   free(termInfo->lines);
	   free(termInfo->wc_num);

        }  /* if requested help text */

	/*
	 * build the hyperlinks list, if requested
	 */
        if ( NULL != hyperList )
        {
	   *hyperList = NULL;
	   for (i = 0; result != -1 && i < myCStruct->link_data.max; i++)
	       result = AddHyperToList(volHandle, canvasHandle, i, hyperList);
        }
    }  /* if successfully formatted topic */

    _DtHelpCeCleanCanvas(canvasHandle);

    free(fileName);
    _DtHelpCeUnlockVolume(lockInfo);
    return result;

}  /* End _DtHelpTermGetTopicData */

/******************************************************************************
 * Function: int _DtHelpGetTopicDataHandles(ret_canvasHandle,ret_volHandle)
 *
 * Parameters:
 *		ret_canvasHandle	Canvas used to retrieve the info
 *              ret_volHandle	Help volume in use
 *
 * Returns:
 *  0:  canvas handle & volHandle are not NULL
 * -1:  canvas handle & volHandle are NULL
 ******************************************************************************/
int
#if _NO_PROTO
_DtHelpGetTopicDataHandles(ret_canvasHandle,ret_volHandle)
    CanvasHandle *    ret_canvasHandle;
    VolumeHandle *    ret_volHandle;
#else
_DtHelpGetTopicDataHandles(
    CanvasHandle *    ret_canvasHandle,
    VolumeHandle *    ret_volHandle)
#endif
{
   *ret_canvasHandle = MyCanvas;
   *ret_volHandle = MyVolume;
   return ((MyCanvas != NULL && MyVolume != NULL) ? 0 : -1 );
}


/******************************************************************************
 * Function: int _DtHelpGetTopicData (char *helpVolume, char *locationId,
 *		int maxColumns, char ***helpList, DtHelpHyperLines **hyperList)
 *
 * Parameters:
 *		helpVolume	Specifies a file path to the volume.
 *		locationId	Specifies the locationId of the desired topic.
 *		maxColumns	Specifies the column width of the window
 *				for which to format the information.
 *		helpList	Returns a NULL terminated array of
 *				strings.
 *		hyperList	Returns a NULL terminated array of
 *				DtHelpHyperLines containing the hyperlinks
 *				found in the topic.
 *
 * Returns:	0 for success, -1 for failure.
 *
 * errno Values:
 *		EINVAL		'helpVolume', 'locationId', 'helpList',
 *				or 'hyperList' were NULL. 'maxColumns'
 *				was less than one.
 *              getcwd(2) 	errno set via a getcwd call.
 *              readlink(2)     errno set via a readlink call.
 *              open(2)		errno set via an open call on
 *				the file for 'locationId'.
 *              DtErrorMalloc
 *              DtErrorExceedMaxSize
 *				When following symbolic links, the
 *				new path will exceed the system
 *				maximum file path length.
 *              DtErrorIllegalPath
 *				When following symbolic links, the
 *				new path would change to a parent
 *                              directory beyond the beginning
 *                              of the base path.
 *              DtErrorIllegalDatabaseFile
 *                              Specifies that 'helpVolume' is
 *                              an illegal database file.
 *			DtErrorMissingFilenameRes
 *                              Specifies that the 'Filename/filename'
 *                              resource for the topic does not exist.
 *              DtErrorMissingFileposRes
 *                              Specifies that the 'Filepos/filepos'
 *                              resource for the topic does not exist.
 *              DtErrorLocIdNotFound
 *                              Specifies that 'locationId' was not found.
 *		DtErrorFormattingLabel
 *				A label has illegal syntax.
 *		DtErrorHyperType
 *				Invalid (negative) hypertype.
 *		DtErrorFormattingLink
 *				Invalid <LINK>.
 *		DtErrorHyperSpec
 *				Invalid 'hyper_specification" in the
 *				the <LINK>.
 *		DtErrorFormattingId
 *				Invalid <ID> syntax.
 *		DtErrorFormattingTitle
 *				Invalid <TITLE> syntax.
 *
 * Purpose:	_DtHelpGetTopicData formats Help Files with
 *		formatting information
 *		into a form understood by a terminal
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpGetTopicData(helpVolume, locationId, maxColumns, helpList, hyperList )
	char *               helpVolume;
	char *               locationId;
	int                  maxColumns;
	char * * *           helpList;
	DtHelpHyperLines * * hyperList;
#else
_DtHelpGetTopicData (
	char *               helpVolume,
	char *               locationId,
	int                  maxColumns,
	char * * *           helpList,
	DtHelpHyperLines * * hyperList )
#endif /* _NO_PROTO */
{
    int           result = -1;
    VolumeHandle  volume = NULL;
    char *        path = NULL;

    /* find the volume (path is malloc'd) */
    path = _DtHelpFileLocate(DtHelpVOLUME_TYPE, helpVolume,
                                  _DtHelpFileSuffixList,False,R_OK);
    if (path == NULL) { errno = EINVAL; return -1; }          /* RETURN */

    /* open new canvas or reuse old one with new size */
    if (MyCanvas == NULL)
    {
       _DtHelpTermCreateCanvas(maxColumns,&MyCanvas);
       if (MyCanvas == NULL)
       {
          free(path);
          return -1;		/* RETURN: errno=from CreateCanvas */
       }
    }
    else
    {
       MyInfo.max_columns = maxColumns;
       _DtHelpCeResizeCanvas (MyCanvas, NULL, NULL);
    }

    if (_DtHelpCeOpenVolume (MyCanvas, helpVolume, &volume) == -1)
    {
        free(path);
	return -1;		/* RETURN: errno=from OpenVolume */
    }

    /* release any previously opened volume */
    if (MyVolume)
	_DtHelpCeCloseVolume (MyCanvas, MyVolume);

    /* assign the new volume */
    MyVolume = volume;

    /* get the terminal info */
   result = _DtHelpTermGetTopicData(MyCanvas,MyVolume,locationId,helpList,hyperList);

   free(path);
   return result;
}

/*****************************************************************************
 * Function: int _DtHelpProcessLinkData (char * ref_volume,DtHelpHyperLines *hyperLine,
 *					char **ret_helpVolume, char **ret_locationId)
 *
 * Parameters:
 *		hyperLine	Specifies the hypertext line that
 *				the caller wishes to be resolved.
 *		helpVolume	Returns the help volume specified by
 *				'hyperLine'.
 *		locationId	Returns the location Id specified by
 *				'hyperLine'.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		EINVAL		'hyperLines', 'helpVolume', or 'locationId'
 *				is NULL.
 *		DtErrorMalloc
 *		DtErrorHyperType
 *				The hyper type is not _DtJUMP_REUSE,
 *				_DtJUMP_NEW, or _DtDEFINITION.
 *		DtErrorHyperSpec
 *				Invalid hyper specification.
 *		DtErrorIllegalPath
 *				The volume used by the link spec (either
 *				embedded or the ref_volume) could not be
 *				located.
 *
 * Purpose:	_DtHelpProcessLinkData resolves a hypertext specification
 *		into a pathname to a help volume and a location Id within
 *		the help volume.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpProcessLinkData(ref_volume, hyperLine, ret_helpVolume, ret_locationId )
    char *              ref_volume;
    DtHelpHyperLines  * hyperLine;
    char * *            ret_helpVolume;
    char * *            ret_locationId;
#else
_DtHelpProcessLinkData (
    char *              ref_volume,
    DtHelpHyperLines  * hyperLine,
    char * *            ret_helpVolume,
    char * *            ret_locationId )
#endif /* _NO_PROTO */
{
    char * linkSpec   = NULL;
    char * volumeName = NULL;
    char * idToken    = NULL;

    /* check params */
    if (ref_volume == NULL || hyperLine == NULL ||
				ret_helpVolume == NULL || ret_locationId == NULL)
    {
	errno = EINVAL;
	return -1;
    }

    /* only process SameVolume or CrossVolume links */
    switch (hyperLine->hyper_type)
    {
	case CELinkType_SameVolume:
	case CELinkType_CrossLink:
		break;

	default:
		errno = DtErrorHyperType;
		return -1;			/* RETURN */
    }

    /* Initialize the pointers. */
    *ret_helpVolume  = NULL;
    *ret_locationId = NULL;

    /* get working copy of spec */
    linkSpec = strdup (hyperLine->specification);
    if (linkSpec == NULL)
    {
	errno = DtErrorMalloc;
        return -1;
    }

    /* parse the link spec.  Syntax is: "[volume] locationId" */
    if (linkSpec == NULL || *linkSpec == EOS)
    {
        errno = DtErrorHyperSpec;      /* no spec */
    }
    else  /* at least one token exists */
    {
        volumeName = linkSpec;  /* posit that first token is the volume */

        /* look for another possible token */
        idToken = NULL;
        _DtHelpCeStrchr (linkSpec," ",MB_CUR_MAX,&idToken);

        /* second token? */
        if (idToken) /* second token */
        {
            /* separate the tokens and advance idToken to first valid char */
            *idToken = EOS; idToken++;

            /* find the fully qualified volume (volName is malloc'd) */
            volumeName = _DtHelpFileLocate(DtHelpVOLUME_TYPE, volumeName,
                              _DtHelpFileSuffixList,False,R_OK);
            if (volumeName == NULL)
                errno = DtErrorIllegalPath;
        }
        else /* no second token */
        {
	    idToken = volumeName;
            /* find the fully qualified volume (volName is malloc'd) */
            volumeName = _DtHelpFileLocate(DtHelpVOLUME_TYPE, ref_volume,
                              _DtHelpFileSuffixList,False,R_OK);
	    if (volumeName == NULL)
	        errno = DtErrorIllegalPath;
	}

	if (idToken && volumeName)
	{
            *ret_helpVolume = volumeName;  /* already allocd by _DtHelpFileLocate */
            *ret_locationId = strdup (idToken);
	    if (*ret_locationId == NULL)
	        errno = DtErrorMalloc;
        }
    } /* if at least one token exists */

    free (linkSpec);
         
    return ( (*ret_helpVolume && *ret_locationId) ? 0 : -1 );

}  /* End _DtHelpProcessLinkData */

/*****************************************************************************
 * Function: void _DtHelpFreeTopicData (char **helpList,
 *					DtHelpHyperLines *hyperList)
 *
 * Parameters:
 *		helpList	Specifies a pointer to a NULL terminated
 *				array of strings.
 *		hyperList	Specifies a pointer to a NULL terminated
 *				list of DtHelpHyperLines.
 *
 * Returns:	Nothing
 *
 * Purpose:	_DtHelpFreeTopicData frees the data associated with a topic.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpFreeTopicData(helpList, hyperList)
	char	         **helpList;
	DtHelpHyperLines     *hyperList;
#else
_DtHelpFreeTopicData (
	char	         **helpList,
	DtHelpHyperLines     *hyperList)
#endif /* _NO_PROTO */
{
    if (helpList != NULL)
        _DtHelpCeFreeStringArray (helpList);

    if (hyperList != NULL)
        DeallocateHyperArray (hyperList);

}  /* End _DtHelpFreeTopicData */

/******************************************************************************
 * Function: int _DtHelpGetTopicChildren (char *helpVolume, char *topic_id,
 *		char ***ret_children)
 *
 * Parameters:
 *		helpVolume	Specifies a file path to the volume.
 *		topic_id	Specifies the id of the desired topic.
 *		ret_children	Returns a NULL terminated array of
 *				strings. The memory for these strings
 *				*IS OWNED* by the caller and must be freed
 *				after use.
 *
 * Returns:	> 0 for success, -1 for failure.
 *
 * errno Values:
 *
 * Purpose:	_DtHelpGetTopicChildren returns the topic ids of the children
 *		for a given topic id.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpGetTopicChildren(helpVolume, topic_id, ret_children )
	char	         *helpVolume ;
	char		 *topic_id;
	char		***ret_children;
#else
_DtHelpGetTopicChildren (
	char	         *helpVolume,
	char		 *topic_id,
	char		***ret_children)
#endif
{
    int           result = -1;
    VolumeHandle  volume = NULL;
    char *        path;

    /* Initialize the pointer */
    *ret_children  = NULL;

    /* find the volume (path is malloc'd) */
    path = _DtHelpFileLocate(DtHelpVOLUME_TYPE, helpVolume,
                                  _DtHelpFileSuffixList,False,R_OK);
    if (path == NULL) { errno = EINVAL; return -1; }          /* RETURN */

    /* open new canvas or reuse old one with new size */
    if (MyCanvas == NULL)
    {
       _DtHelpTermCreateCanvas(72,&MyCanvas);  /* 72: arbitary value for max columns */
       if (MyCanvas == NULL)
       {
          free(path);
          return -1;            /* RETURN: errno=??? */
       }
    }

    if (_DtHelpCeOpenVolume (MyCanvas, helpVolume, &volume) == -1)
    {
        free(path);
        return -1;              /* RETURN: errno=??? */
    }

    /* release any previously opened volume */
    if (MyVolume)
        _DtHelpCeCloseVolume (MyCanvas, MyVolume);

    /* assign the new volume */
    MyVolume = volume;

    /* Get the children */
   result = _DtHelpCeGetTopicChildren(MyVolume, topic_id, ret_children);

   free(path);
   return result;
}
