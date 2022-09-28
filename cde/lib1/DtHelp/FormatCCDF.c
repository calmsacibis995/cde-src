/* $XConsortium: FormatCCDF.c /main/cde1_maint/1 1995/07/17 17:30:26 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	FormatCCDF.c
 **
 **   Project:    Unix Desktop Help
 **
 **   Description: This code formats an file in CCDF (Cache Creek
 **		   Distribution Format) into internal format.
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
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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
#include "AccessI.h"
#include "AccessP.h"
#include "AccessCCDFI.h"
#include "LinkMgrI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "FontAttrI.h"
#include "FormatCCDFI.h"
#include "CCDFUtilI.h"
#include "CEUtilI.h"
#include "StringFuncsI.h"
#include "LocaleXlate.h"

#ifdef NLS16
#endif

/******************************************************************************
 * Private variables and defines.
 *****************************************************************************/
#define	BUFF_SIZE	1024
#define	GROW_SIZE	10
#define	INFO_GROW	256

#define	CCDF_LINK_JUMP_REUSE	0
#define	CCDF_LINK_JUMP_NEW	1
#define	CCDF_LINK_DEFINITION	2
#define	CCDF_LINK_EXECUTE	3
#define	CCDF_LINK_MAN_PAGE	4
#define	CCDF_LINK_APP_DEFINE	5

/******************************************************************************
 * Private structures
 ******************************************************************************/
typedef	struct _controlStruct {
	CEParagraph	values;
	int		my_index;
	int		first_child;
	int		last_child;
	struct _controlStruct *my_parent;
} ControlStruct;

typedef	struct _formatVariables {

	CEBoolean		 found_id;
	CEBoolean		 last_was_space;
	CEBoolean		 last_was_mb;
	CEBoolean		 last_was_nl;

	char		*rd_buf;
	char		*rd_ptr;
	char		*fmt_buf;
	char		*my_path;
	char		*id_string;
	char		*topic_char_set;
	char		*topic_lang;

	int		 rd_size;
	int		 rd_flag;

	int		 topic_mb_max;
	int		 cur_mb_max;

	int		 para_cnt;
	int		 para_max;

	int		 seg_cnt;
	int		 seg_max;

	int		 id_para;
	int		 id_seg;

	int		 fmt_buf_max;
	int		 fmt_size;

	BufFilePtr	 my_file;
	CESegment	*seg_list;
	CEParagraph	*para_list;
	CELinkData	 my_links;
} FormatVariables;

/******************************************************************************
 * Private Function Declarations
 ******************************************************************************/
#ifdef _NO_PROTO
static	CEParagraph	*AllocateParagraphStruct();
static	CESegment	*AllocateSegments();
static	int		 AppendCharToInfo ();
static	int		 AppendOctalToInfo ();
static	int		 AppendSpaceToInfo ();
static	int		 AppendToInfo ();
static	int		 ChangeFont ();
#ifdef	oldway
static	int		 ChangeStringFont ();
#endif
static	int		 CheckIdString();
static	int		 CheckSaveSegment ();
static	int		 CreateSaveGraphic ();
static	int		 FindBreak ();
static	int		 Format ();
static	int		 GetNextBuffer ();
static	int		 GetStringParameter ();
static	int		 GetValueParameter ();
static	int		 Initialize ();
static	int		 InitializeParagraph ();
static	int		 Parse ();
static	int		 ProcessFigureCmd();
static	int		 ProcessHypertext();
static	int		 ProcessInLine();
static	int		 ProcessLabelCmd();
static	int		 ProcessParagraph ();
static	int		 SaveNewLine();
static	int		 SaveSegment ();
static	int		 SkipToNextToken ();
static	int		 TerminateSegList ();
static  int              SegmentSave();
#else
static	CEParagraph	*AllocateParagraphStruct(
				CEParagraph    *p_ps,
				int              size );
static	CESegment	*AllocateSegments(
				CESegment      *p_seg,
				int              original_size,
				int              new_size );
static	int		 AppendCharToInfo(
				FormatVariables  *cur_vars,
				char		**src );
static	int		 AppendOctalToInfo(
				FormatVariables *cur_vars,
				char		*src );
static	int		 AppendSpaceToInfo ( 
				FormatVariables  *cur_vars,
				char		**src,
				int		  type );
static	int		 AppendToInfo (
				FormatVariables  *cur_vars,
				char		**src,
				const char	 *scan_string );
static	int		 ChangeFont(
				CECanvasStruct	*canvas,
				int		  whichOne,
				int		  segType,
				CEFontSpec		 *font_attr,
				int		  linkspec,
				char		 *description,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				int		  flags,
				int		  fnt_flg);
static	int		 CheckIdString(
				CECanvasStruct	*canvas,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				int		  segType,
				CEFontSpec		 *font_attr,
				int		  linkspec,
				char		 *description,
				int		  flags,
				int		  fnt_flg);
static	int		 CheckSaveSegment (
				CECanvasStruct	*canvas,
				int		 type,
				CEFontSpec		 *font_attr,
				int 		 link_spec,
				FormatVariables	 *cur_vars );
static	int		 CreateSaveGraphic (
				FormatVariables *cur_vars,
				int		 type,
				char		*file_name,
				int		 link_spec );
static	int		 FindBreak (
				char	*ptr,
				int	 size );
static	int		 Format(
				CECanvasStruct	*canvas,
				FormatVariables *cur_vars,
				char		*id_string,
				TopicHandle	*ret_handle);
static	int		 GetNextBuffer (
				FormatVariables *cur_vars);
static	int		 GetStringParameter(
				FormatVariables  *cur_vars,
				CEBoolean		  flag,
				CEBoolean		  eat_escape,
				CEBoolean		  ignore_quotes,
				CEBoolean		  less_test,
				char		**ret_string );
static	int		 GetValueParameter(
				FormatVariables *cur_vars,
				CEBoolean		 flag,
				int		*ret_value );
static	int		 Initialize(
				FormatVariables *cur_vars,
				char		*findString,
				char		*rd_buf,
				int		 rd_size,
				int		 grow_size);
static	int		 InitializeParagraph(
				FormatVariables	*cur_vars,
				int		 grow_size );
static	int		 Parse(
				CECanvasStruct	*canvas,
				int		  cur_cmd,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				int		  segType,
				CEFontSpec	 *font_attr,
				int		  linkspec,
				char		 *description,
				const char	 *scan_string,
				int		  allowed,
				int		  fnt_flag);
static	int		 ProcessFigureCmd(
				CECanvasStruct	*canvas,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				CEFontSpec	 *font_attr,
				int		  fnt_flag);
static	int		 ProcessHypertext(
				CECanvasStruct	*canvas,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				int		  segType,
				CEFontSpec		 *font_attr,
				int		  flags,
				int		  fnt_flag);
static	int		 ProcessInLine(
				FormatVariables	*cur_vars,
				int		 seg_type,
				int		 link_spec );
static	int		 ProcessLabelCmd(
				CECanvasStruct	*canvas,
				ControlStruct	 *parent,
				FormatVariables	 *cur_vars,
				int		  seg_type,
				CEFontSpec		 *font_attr,
				int		  link_spec,
				char		 *description,
				int		  flags,
				int		  fnt_flag);
static	int		 ProcessParagraph (
				CECanvasStruct	*canvas,
				ControlStruct	*parent,
				FormatVariables	*cur_vars,
				int		  fnt_flg);
static	int		 SaveNewLine(
				CECanvasStruct	*canvas,
				FormatVariables	 *cur_vars,
				int		  cur_cmd,
				CEFontSpec		 *font_attr,
				int		  link_spec );
static	int		 SaveSegment (
				CECanvasStruct	*canvas,
				FormatVariables	 *cur_vars,
				int		  type,
				CEFontSpec		 *font_attr,
				int		  link_spec);
static	int		 SkipToNextToken (
				FormatVariables	*cur_vars,
				CEBoolean		 flag );
static	int		 TerminateSegList (
				FormatVariables	*cur_vars);
static  int              SegmentSave(
				CECanvasStruct	*canvas,
                                int              type,
				CEFontSpec		 *font_attr,
                                char            *string,
                                int              str_size,
                                int              link_spec,
                                CESegment      *p_seg );
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

/******************************************************************************
 * Private variables
 *****************************************************************************/
static	const char	*Specials = "<\n\\ ";
static	const char	*SpaceString = "        ";
static	const char	*CString = "C";
static	const char	*Period  = ".";
static	const char	*Slash   = "/";

static const FormatVariables DefVars = {
	False,		/* CEBoolean	 found_id;		*/
	True,		/* CEBoolean	 last_was_space;	*/
	False,		/* CEBoolean	 last_was_mb;		*/
	False,		/* CEBoolean	 last_was_nl;		*/

	NULL,		/* char		*rd_buf;		*/
	NULL,		/* char		*rd_ptr;		*/
	NULL,		/* char		*fmt_buf;		*/
	NULL,		/* char		*my_path;		*/
	NULL,		/* char		*id_string;		*/
	NULL,		/* char		*topic_char_set;	*/
	NULL,		/* char		*topic_lang;		*/

	0,		/* int		 rd_size;		*/
	0,		/* int		 rd_flag;		*/

	1,		/* int		 topic_mb_max;		*/
	1,		/* int		 cur_mb_max;		*/

	0,		/* int		 para_cnt;		*/
	0,		/* int		 para_max;		*/

	0,		/* int		 seg_cnt;		*/
	0,		/* int		 seg_max;		*/

	-1,		/* int		 id_para;		*/
	-1,		/* int		 id_seg;		*/

	0,		/* int		 fmt_buf_max;		*/
	0,		/* int		 fmt_size;		*/

	NULL,		/* BufFilePtr	 my_file;		*/
	NULL,		/* CESegment	*seg_list;		*/
	NULL,		/* CEParagraph	*para_list;		*/
	{0, NULL},	/* CELinkData	 my_links;		*/
};

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	void GetMbLen (
 *
 * Parameters:
 *              cur_vars        Specifies the current setting of
 *				formatting variables.
 *              font_str        Specifies the CCDF string for a
 *				CHARACTERSET change.  The string is
 *				modified by this routine.
 *              lang_ptr        Returns the language and territory
 *				value.  The caller owns this memory.
 *              set_ptr         Returns the code set of the language.
 *				The caller owns this memory.
 *
 * Returns:	Nothing
 *
 * Purpose:	1) To decompose the font_string into a language and code set
 *		   portions.
 *		2) To set the mb_cur_len variable in cur_vars.
 *
 *****************************************************************************/
static	void
#ifdef _NO_PROTO
GetMbLen (cur_vars, font_str, lang_ptr, set_ptr, font_str)
    FormatVariables	 *cur_vars;
    char		 *font_str;
    char		**lang_ptr;
    char		**set_ptr;
#else
GetMbLen (
    FormatVariables	 *cur_vars,
    char		 *font_str,
    char		**lang_ptr,
    char		**set_ptr)
#endif /* _NO_PROTO */
{
    int   i;
    char *ptr;
    char *langPart = NULL;
    char *codePart = NULL;

    /*
     * strip spaces
     */
    while (*font_str == ' ')
	font_str++;

    for (i = strlen(font_str) - 1; i > -1 && font_str[i] == ' '; i--)
	font_str[i] = '\0';

    /*
     * check for locale.codeset
     */
    if (_DtHelpCeStrchr(font_str, ".", 1, &ptr) == 0)
      {
	/*
	 * make font_str two seperate strings by replacing
	 * the dot with a end of line.
	 */
	*ptr++ = '\0';
	langPart = strdup(font_str);
	codePart = strdup(ptr);
      }

    /*
     * old CCDF volume
     */
    else
      {
	_DtHelpCeXlateOpToStdLocale(DtLCX_OPER_CCDF, font_str,
						NULL, &langPart, &codePart);
	/*
	 * if the language is not specified for the code set,
	 * get the environment's.
	 */
	if (strcmp(langPart, "?") == 0)
	  {
	    free(langPart);

            langPart = getenv("LANG");
            if (langPart == NULL || *langPart == '\0')
	        langPart = (char *) CString;
	    else if (_DtHelpCeStrrchr(langPart, "_", MB_CUR_MAX, &ptr) == 0)
		*ptr = '\0';
	    else if (_DtHelpCeStrrchr(langPart, Period, MB_CUR_MAX, &ptr) == 0)
		*ptr = '\0';

	    langPart = strdup(langPart);
	  }
      }

    cur_vars->cur_mb_max = _DtHelpCeGetMbLen(langPart, codePart);

    /*
     * now set the return variables
     */
    if (lang_ptr != NULL)
        *lang_ptr = langPart;
    else
	free(langPart);

    if (set_ptr != NULL)
        *set_ptr = codePart;
    else
	free(codePart);
}

/******************************************************************************
 * Function:	int ReturnLinkWinHint (int ccdf_type)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ReturnLinkWinHint (ccdf_type)
    int   ccdf_type;
#else
ReturnLinkWinHint (
    int   ccdf_type)
#endif /* _NO_PROTO */
{
    int  winHint = CEWindowHint_CurrentWindow;

    if (ccdf_type == CCDF_LINK_JUMP_NEW)
	winHint = CEWindowHint_NewWindow;

    else if (ccdf_type == CCDF_LINK_DEFINITION)
	winHint = CEWindowHint_PopupWindow;

    return winHint;
}

/******************************************************************************
 * Function:	int ReturnLinkType (int ccdf_type, char *spec)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ReturnLinkType (ccdf_type, spec)
    int   ccdf_type;
    char *spec;
#else
ReturnLinkType (
    int   ccdf_type,
    char *spec)
#endif /* _NO_PROTO */
{
    int  trueType = CELinkType_SameVolume;

    switch(ccdf_type)
      {
	case CCDF_LINK_JUMP_NEW:
	case CCDF_LINK_JUMP_REUSE:
        case CCDF_LINK_DEFINITION:
		if (strchr(spec, ' ') != NULL)
		    trueType = CELinkType_CrossLink;
		break;

	case CCDF_LINK_EXECUTE:
		trueType = CELinkType_Execute;
		break;

	case CCDF_LINK_MAN_PAGE:
		trueType = CELinkType_ManPage;
		break;
	
        case CCDF_LINK_APP_DEFINE:
		trueType = CELinkType_AppDefine;
		break;

      }

    return trueType;
}

/******************************************************************************
 * Function:	int FindBreak (char *ptr, int size)
 *
 * Parameters:
 *		ptr	Specifies the string to check.
 *		size	Specifies the character length.
 *
 * Returns	number of bytes that have a charater length of 'size'.
 *
 * errno Values:
 *
 * Purpose:	Find a length of 'ptr' comprised of characters of 'size'
 *		length.
 *
 *****************************************************************************/
static	int
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
 * Function:	int CheckSegList (FormatVariables *cur_vars)
 *
 * Parameters:
 *
 * Returns:	0 for success, -1 for errors.
 *
 * errno Values:
 *		CEErrorReported
 *
 * Purpose:	Check the global variable 'SegCount' against 'SegMax' and
 *		realloc/malloc more memory for the global variable 'SegList'
 *		if necessary. If memory is realloc'ed/malloc'ed, the entry
 *		'para_entry->seg_list' is set to 'SegList'.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
CheckSegList (cur_vars)
    FormatVariables	*cur_vars;
#else
CheckSegList (
    FormatVariables	*cur_vars )
#endif /* _NO_PROTO */
{
    int    result = 0;

    if (cur_vars->seg_cnt >= cur_vars->seg_max)
      {
	cur_vars->seg_list = AllocateSegments (cur_vars->seg_list,
				cur_vars->seg_max,
				(cur_vars->seg_max + GROW_SIZE));
	if (cur_vars->seg_list)
	  {
	    CEParagraph	*para_entry =
				&(cur_vars->para_list[cur_vars->para_cnt]);

	    cur_vars->seg_max   += GROW_SIZE;
	    para_entry->seg_list = cur_vars->seg_list;
	  }
	else
	    result = -1;
      }
    return result;
}

/******************************************************************************
 * Function: static CESegment *AllocateSegments (CESegment p_seg,
 *					int original_size, int new_size)
 *
 * Parameters:
 *		p_seg		Specifies a pointer to the old list
 *				of CESegments.
 *		original_size	Specifies the current size of 'p_seg'.
 *		new_size	Specifies the new size of 'p_seg'.
 *
 * Returns:	A pointer to the new list of structures.
 *		A NULL indicates an error.
 *
 * errno Values:
 *		CEErrorReported
 *
 * Purpose:	Allocates CESegment structures and makes the primary type
 *		CE_NOOP
 *
 *****************************************************************************/
static CESegment *
#ifdef _NO_PROTO
AllocateSegments(p_seg, original_size, new_size )
	CESegment	*p_seg;
	int		 original_size;
	int		 new_size;
#else
AllocateSegments (
	CESegment	*p_seg,
	int		 original_size,
	int		 new_size )
#endif /* _NO_PROTO */
{
    if (p_seg)
	p_seg = (CESegment *) realloc ((void *) p_seg,
					(sizeof(CESegment) * new_size));
    else
	p_seg = (CESegment *) malloc ((sizeof(CESegment) * new_size));

    if (p_seg == NULL)
	errno = CEErrorMalloc;
    else
      {
        /*
         * want to 'empty' the type.
         */
        while (original_size < new_size)
	    p_seg[original_size++].seg_type = CE_NOOP;
      }

    return p_seg;
}

/******************************************************************************
 * Function:	static CEParagraph *AllocateParagraphStruct (
						CEParagraph *p_ps, int size)
 *
 * Parameters:
 *		p_ps	Specifies the list of CEParagraph structures to
 *			malloc/realloc.
 *		size	Spcifies the number of elements to allocate for p_ps.
 *
 * Returns:	A pointer to the list of structures, otherwise NULL.
 *
 * errno Values:
 *		CEErrorMalloc
 *
 * Purpose:	Allocates a list of CEParagraph structures.
 *
 *****************************************************************************/
static CEParagraph *
#ifdef _NO_PROTO
AllocateParagraphStruct( p_ps,  size )
    CEParagraph	*p_ps;
    int			 size;
#else
AllocateParagraphStruct (
    CEParagraph	*p_ps,
    int			 size )
#endif /* _NO_PROTO */
{
    if (p_ps)
	p_ps = (CEParagraph *) realloc ((void *) p_ps,
					    (sizeof(CEParagraph) * size));
    else
	p_ps = (CEParagraph *) malloc (sizeof(CEParagraph) * size);

    if (p_ps == NULL)
	errno = CEErrorMalloc;

    return p_ps;
}

/******************************************************************************
 * Function: int SegmentSave (int type,
 *			char **font_attr, char *string, int linkspec,
 *			CESegment *p_seg)
 * 
 * Parameters:
 *		type		Specifies the segment type.
 *		font_attr	Specifies the resource list of quarks for the
 *					font to associate with the segment.
 *		string		Specifies the string segment.
 *		link_spec	Specifies the link specification for the
 *					segment.
 *		description	Not Used.
 *		p_seg		Specifies the CESegment structure to
 *					initialize with the data.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		CEErrorReported
 *
 * Purpose:	Creates a new segment with the given data.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
SegmentSave (canvas, type, font_attr, string, linkspec, p_seg )
	CECanvasStruct	*canvas;
	int		 type;
	CEFontSpec		*font_attr;
	char		*string;
	int		 str_size;
	int		 linkspec;
	CESegment	*p_seg;
#else
SegmentSave (
	CECanvasStruct	*canvas,
	int		 type,
	CEFontSpec	*font_attr,
	char		*string,
	int		 str_size,
	int		 linkspec,
	CESegment	*p_seg )
#endif /* _NO_PROTO */
{
    int		  result = 0;
    CEString	 *ce_string;

    p_seg->seg_type   = _CEClearInitialized(type);
    p_seg->link_index = linkspec;

    /*
     * We use this routine for many things.
     * If we want an newline in here, we may not have any data.
     */
    if (str_size)
      {
	ce_string = (CEString *) malloc (sizeof (CEString));
	if (ce_string != NULL)
	  {
	    p_seg->seg_handle.str_handle = ce_string;
	    ce_string->string            = strdup (string);
	    ce_string->font_handle       = NULL;

	    if (canvas->virt_functions._CEGetFont != NULL)
	        (*(canvas->virt_functions._CEGetFont))(
			    canvas->client_data,
			    font_attr->lang,
			    font_attr->char_set,
			    font_attr->font_specs,
			    &(ce_string->string),
			    &(ce_string->font_handle));
	    p_seg->seg_type = _CESetInitialized(type);
	  }
	else
	  {
	    errno = CEErrorMalloc;
	    result = -1;
	  }
      }
    else
	p_seg->seg_handle.str_handle = NULL;

    return result;
}

/******************************************************************************
 * Function: int SaveSegment (FormatVariables, int type, char **font_attr,
 *				int link_spec)
 * 
 * Parameters:
 *		type		Specifies the segment type.
 *		font_attr	Specifies the resource list of quarks for the
 *					font to associate with the segment.
 *		link_spec	Specifies the link specification for the
 *					segment.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Saves a segment into the global 'SegList'. This is
 *		a wrapper around SegmentSave that adds specific
 *		information relative to this module.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
SaveSegment (canvas, cur_vars, type, font_attr, link_spec)
    CECanvasStruct	*canvas;
    FormatVariables	 *cur_vars;
    int			  type;
	CEFontSpec		*font_attr;
    int			  link_spec;
#else
SaveSegment (
    CECanvasStruct	*canvas,
    FormatVariables	 *cur_vars,
    int			  type,
    CEFontSpec		*font_attr,
    int			  link_spec)
#endif /* _NO_PROTO */
{
    int     len;
    int     size;
    CEBoolean newLine = False;
    char    tmpChar;
    char   *ptr;

    if (cur_vars->cur_mb_max != 1 && cur_vars->fmt_size > 1)
      {
	if (_CEIsNewLine (type))
	  {
	    newLine = True;
	    type = _CEClearNewline (type);
	  }

	ptr = cur_vars->fmt_buf;
	while (cur_vars->fmt_size)
	  {
	    /*
	     * what type of character is this?
	     */
	    len = mblen (ptr, cur_vars->cur_mb_max);

	    /*
	     * How many of the following characters are of the same size?
	     */
	    size = FindBreak (ptr, len);

	    /*
	     * save off the character that doesn't match
	     */
	    tmpChar = ptr[size];

	    /*
	     * put in the null byte for the string.
	     * and set the size.
	     */
	    ptr[size] = '\0';
	    type = _CESetCharLen (type, len);

	    /*
	     * save the segment.
	     */
	    if (CheckSegList(cur_vars) == -1 ||
		SegmentSave (canvas, type, font_attr, ptr, size, link_spec,
				&(cur_vars->seg_list[cur_vars->seg_cnt])) == -1)
		return -1;

	    /*
	     * Bump the segment list count.
	     */
	    cur_vars->seg_cnt++;

	    /*
	     * replace the character that didn't match.
	     * and bump the pointer to it.
	     */
	    ptr[size] = tmpChar;
	    ptr      += size;
	    cur_vars->fmt_size -= size;
	  }

	if (newLine)
	    cur_vars->seg_list[cur_vars->seg_cnt - 1].seg_type =
		_CESetNewLineFlag (
			cur_vars->seg_list[cur_vars->seg_cnt - 1].seg_type);
      }
    else
      {
	if (cur_vars->fmt_size == 0)
	    type = _CESetNoop(type);

	if (CheckSegList (cur_vars) == -1 ||
			SegmentSave (canvas, type, font_attr,
				cur_vars->fmt_buf, cur_vars->fmt_size,
				link_spec,
				&(cur_vars->seg_list[cur_vars->seg_cnt])) == -1)
	    return -1;
        cur_vars->seg_cnt++;
      }

    cur_vars->fmt_size = 0;
    if (cur_vars->fmt_buf)
      cur_vars->fmt_buf[0] = '\0';

    return 0;
}

/******************************************************************************
 * Function: int CheckSaveSegment (int type, char **font_attr, int link_spec,
 *					FormatVariables cur_vars)
 * 
 * Parameters:
 *		type		Specifies the segment type.
 *		font_attr	Specifies the resource list of quarks for the
 *					font to associate with the segment.
 *		link_spec	Specifies the link specification for the
 *					segment.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Checks 'cur_vars->fmt_size' for a non-zero value. If it is,
 *		calls SaveSegment.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
CheckSaveSegment (canvas, type, font_attr, link_spec, cur_vars)
    CECanvasStruct	*canvas;
    int			  type;
	CEFontSpec		*font_attr;
    int			  link_spec;
    FormatVariables	 *cur_vars;
#else
CheckSaveSegment (
    CECanvasStruct	*canvas,
    int			  type,
	CEFontSpec		*font_attr,
    int			  link_spec,
    FormatVariables	 *cur_vars)
#endif /* _NO_PROTO */
{
    int   result = 0;

    if (cur_vars->fmt_size)
        result = SaveSegment (canvas, cur_vars, type, font_attr, link_spec);

    return result;
}

/******************************************************************************
 * Function: int TerminateSegList (FormatVariables	*cur_vars)
 * 
 * Parameters:
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *		CEErrorReported
 *
 * Purpose:	Puts the terminator flag on the last valid structure
 *		in 'SegList'
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
TerminateSegList (cur_vars)
    FormatVariables	*cur_vars;
#else
TerminateSegList (
    FormatVariables	*cur_vars)
#endif /* _NO_PROTO */
{
    int	count = 0;
    CESegment *pSeg;

    /*
     * terminate the last segment.
     */
    if (cur_vars->seg_cnt)
	count = cur_vars->seg_cnt - 1;
    else if (cur_vars->seg_list == NULL)
      {
        cur_vars->seg_list = AllocateSegments (cur_vars->seg_list, 0, 1);
	if (cur_vars->seg_list)
	  {
	    CEParagraph *para_entry =
				&(cur_vars->para_list[cur_vars->para_cnt]);

	    cur_vars->seg_max = 1;
	    para_entry->seg_list = cur_vars->seg_list;
	    cur_vars->seg_list->seg_type = _CESetNoop(0);
	  }
	else
	    return -1;
      }

    pSeg = cur_vars->seg_list;
    for (count = 0; count < cur_vars->seg_cnt - 1; count++)
      {
	pSeg[count].next_seg  = &(pSeg[count + 1]);
	pSeg[count].next_disp = &(pSeg[count + 1]);
      }

    pSeg[count].next_seg  = NULL;
    pSeg[count].next_disp = NULL;

    return 0;
}

/******************************************************************************
 * Function: int InitializeParagraph (FormatVariables *cur_vars, int grow_size)
 *
 * Parameters:
 *		grow_size
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Initialize the next entry in 'para_list' with default values.
 *		Reset SegList to NULL.
 *		Reset SegCount to 0.
 *		Reset SegMax to 0
 *		Reset cur_vars->last_was_space to True.
 *
 *****************************************************************************/
static	CEParagraph	DefaultParagraph = { 0, 0, 0, 2, 0, 1, NULL };

static	int
#ifdef _NO_PROTO
InitializeParagraph( cur_vars, grow_size )
    FormatVariables	*cur_vars;
    int			 grow_size;
#else
InitializeParagraph(
    FormatVariables	*cur_vars,
    int			 grow_size )
#endif /* _NO_PROTO */
{
    if (cur_vars->para_cnt >= cur_vars->para_max)
      {
        cur_vars->para_max += grow_size;
	cur_vars->para_list = AllocateParagraphStruct (cur_vars->para_list,
						cur_vars->para_max );
      }
    if (cur_vars->para_list)
      {
        cur_vars->para_list[cur_vars->para_cnt] = DefaultParagraph;
        cur_vars->seg_list   = NULL;
        cur_vars->seg_cnt = 0;
        cur_vars->seg_max   = 0;

        cur_vars->last_was_space = True;
      }
    else
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int Initialize (FormatVariables *cur_vars,
 *				char *id_string, char *rd_buf, int rd_size,
 *					int grow_size)
 * 
 * Parameters:
 *		rd_buffer	Specifies the buffer all reads use.
 *		id_string	Specifies the location ID to search for.
 *		grow_size	Specifies the minimum number 'para_list'
 *				grows by.
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Reset the paragraph and segment variables back to zero/NULL.
 *		Set the global variable 'cur_vars->rd_ptr' to 'rd_buffer'.
 *		Reset the 'FindSeg' flag and remember the location id.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
Initialize (cur_vars, id_string, rd_buf, rd_size, grow_size)
    FormatVariables	*cur_vars;
    char		*id_string;
    char		*rd_buf;
    int			 rd_size;
    int			 grow_size;
#else
Initialize(
    FormatVariables	*cur_vars,
    char		*id_string,
    char		*rd_buf,
    int			 rd_size,
    int			 grow_size)
#endif /* _NO_PROTO */
{
    cur_vars->rd_size  = rd_size;
    cur_vars->rd_buf   = rd_buf;
    cur_vars->rd_ptr   = cur_vars->rd_buf;

    if (InitializeParagraph (cur_vars, grow_size ) == -1)
	return -1;

    if (id_string && *id_string != '\0')
	cur_vars->id_string = id_string;

    cur_vars->fmt_size = 0;
    if (cur_vars->fmt_buf)
	cur_vars->fmt_buf[0] = '\0';

    return 0;
}

/******************************************************************************
 * Function:	int AppendToInfo (FormatVariables *cur_vars,
 *					char **src, const char *scan_string)
 *
 * Parameters:
 *		src		Specifies the source string to read.
 *				Returns pointing at a special character,
 *					an invalid character or the
 *					end of string.
 *		scan_string	Specifies the special characters to
 *				look for in 'src'.
 *
 * Returns:	 0	if stopped on a special character.
 *		 1	if found the end of string.
 *		 2	if found an invalid character.
 *		-1	if errors.
 *
 * errno Values:
 * 
 * Purpose:	Appends onto 'cur_vars->fmt_buf' the number of characters
 *		found in 'src' that does not match any character in
 *		'scan_string'.
 *
 *		Sets 'cur_vars->last_was_space' to false (assumes that one
 *		of the special characters is a space).
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
AppendToInfo (cur_vars, src, scan_string)
    FormatVariables	 *cur_vars;
    char		**src;
    const char		 *scan_string;
#else
AppendToInfo (
    FormatVariables	 *cur_vars,
    char		**src,
    const char		 *scan_string )
#endif /* _NO_PROTO */
{
    int    spnResult;
    int    size;

    cur_vars->last_was_space = False;
    spnResult = _DtHelpCeStrcspn (*src, scan_string, cur_vars->cur_mb_max, &size);
    if (_DtHelpCeAddStrToBuf(src,&(cur_vars->fmt_buf),&(cur_vars->fmt_size),
				&(cur_vars->fmt_buf_max),size,INFO_GROW) == -1)
	return -1;

    if (spnResult == -1)
	spnResult = 2;

    return spnResult;
}

/******************************************************************************
 * Function:	int AppendOctalToInfo (FormatVariables *cur_vars, char *src)
 * 
 * Parameters:
 *		src		Specifies the source string to read.
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *		CEErrorFormattingValue
 *		CEErrorMalloc
 * 
 * Purpose:	Convert the octal representation pointed to by 'src' and
 *		change it into a character byte. The routine only allows
 *		a number between the values 1-255.
 *
 *		Append the byte onto the global variable 'Info'.
 *		Set 'cur_vars->last_was_space' to false.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
AppendOctalToInfo (cur_vars,  src )
    FormatVariables	*cur_vars;
    char		*src;
#else
AppendOctalToInfo(
    FormatVariables	*cur_vars,
    char		*src)
#endif /* _NO_PROTO */
{
    /*
     * must have 0xXX
     */
    if (((int)strlen(src)) < 4 && cur_vars->my_file != NULL)
      {
	if (GetNextBuffer (cur_vars) == -1)
	    return -1;

	src = cur_vars->rd_ptr;
      }

    if (_DtHelpCeAddOctalToBuf(src,&(cur_vars->fmt_buf),&(cur_vars->fmt_size),
				&(cur_vars->fmt_buf_max),INFO_GROW) == -1)
	return -1;

    cur_vars->last_was_space = False;

    return 0;
}

/******************************************************************************
 * Function:	int AppendCharToInfo (FormatVariables *cur_vars, char **src)
 * 
 * Parameters:
 *		src		Specifies the source string to read.
 *				Returns pointing at the next character
 *					the string.
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Appends the character pointed to by 'src' onto the
 *		global buffer 'Info', updating the pointers associated
 *		with 'Info' accordingly.
 *
 *		Sets 'cur_vars->last_was_space' to False;
 * 
 *****************************************************************************/ static	int
#ifdef _NO_PROTO
AppendCharToInfo (cur_vars, src )
    FormatVariables	 *cur_vars;
    char		**src;
#else
AppendCharToInfo(
    FormatVariables	*cur_vars,
    char		**src)
#endif /* _NO_PROTO */
{
    cur_vars->last_was_space = False;
    return (_DtHelpCeAddCharToBuf (src, &(cur_vars->fmt_buf),
				&(cur_vars->fmt_size),
				&(cur_vars->fmt_buf_max), INFO_GROW));
}

/******************************************************************************
 * Function:	int AppendSpaceToInfo (char **src, int type)
 *
 * Parameters:
 *		src		Specifies the source string to read.
 *				Returns pointing at the next character
 *					the string.
 *		type		Specifies the type of the segment being
 *					processed.
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Appends a space onto the global buffer 'Info', if
 *		the variable 'cur_vars->last_was_space' is false or the
 *		static string flag is set in 'type'.
 *
 *		Sets 'cur_vars->last_was_space' to true.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
AppendSpaceToInfo (cur_vars, src, type )
    FormatVariables	 *cur_vars;
    char		**src;
    int			  type;
#else
AppendSpaceToInfo ( 
    FormatVariables	 *cur_vars,
    char		**src,
    int			  type)
#endif /* _NO_PROTO */
{
    int   result = 0;
    char *space = (char *) SpaceString;

    if (!cur_vars->last_was_space || _CEIsStaticString (type)) {

	result = _DtHelpCeAddCharToBuf (&space, &(cur_vars->fmt_buf),
					&(cur_vars->fmt_size),
					&(cur_vars->fmt_buf_max), INFO_GROW);
        cur_vars->last_was_space = True;
    }

    if (src != NULL)
        *src = *src + 1;
    return result;
}

/******************************************************************************
 * Function:	int FindEndMarker (FormatVariables *cur_vars)
 * 
 * Returns:	 0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpFindEndMarker.
 *		Find the '>' token.
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
FindEndMarker (cur_vars)
    FormatVariables	*cur_vars;
#else
FindEndMarker(
    FormatVariables	*cur_vars)
#endif /* _NO_PROTO */
{
    return (_DtHelpCeGetCcdfEndMark (cur_vars->my_file, cur_vars->rd_buf,
		&(cur_vars->rd_ptr),cur_vars->rd_size,1));
}

/******************************************************************************
 * Function:	int GetNextBuffer (FormatVariables *cur_vars)
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpGetNextBuffer.
 *		Read the next buffer's worth of information.
 * 
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GetNextBuffer (cur_vars)
    FormatVariables	*cur_vars;
#else
GetNextBuffer (
    FormatVariables *cur_vars)
#endif /* _NO_PROTO */
{
    cur_vars->rd_flag = _DtHelpCeGetNxtBuf (cur_vars->my_file,
					cur_vars->rd_buf,
					&(cur_vars->rd_ptr),
					cur_vars->rd_size);
    return (cur_vars->rd_flag);
}

/******************************************************************************
 * Function:	int SaveNewLine (FormatVariables *cur_vars, int cur_type,
 *					char **font_attr,
 *					int link_spec)
 *
 * Parameters:
 *		cur_cmd		Specifies the type of segment being processed.
 *		font_attr	Specifies the list of font quarks associated
 *				with the current segment.
 *		link_spec	Specifies the hypertext link specification
 *				associated with the current segment.
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Sets the newline flag on a segment and saves it by calling
 *			'SaveSegment'.
 *		If the current segment is non-null, save it with the
 *			newline flag set.
 *		Otherwise if there are no segments saved, create one with the
 *			a type of CE_NOOP.
 *		Otherwise there are other segments, but the current segment is
 *			empty. Look back at the previous segment.
 *			If it doesn't have the newline set on it,
 *			its newline flag is enabled.
 *		Otherwise the previous segment had the newline set, so
 *			create another segment just like it with a 
 *			null length and the newline flag set (if the
 *			previous segment is a graphic, create it
 *			with a type of CE_NOOP).
 * 
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
SaveNewLine (canvas, cur_vars, cur_type, font_attr, link_spec)
    CECanvasStruct	*canvas;
    FormatVariables	 *cur_vars;
    int			  cur_type;
	CEFontSpec		*font_attr;
    int			  link_spec;
#else
SaveNewLine(
    CECanvasStruct	*canvas,
    FormatVariables	 *cur_vars,
    int			  cur_type,
	CEFontSpec		*font_attr,
    int			  link_spec )
#endif /* _NO_PROTO */
{
    int         result = 0;
    CESegment   *pSeg;

    /*
     * If there is information in the buffer, save it with the newline flag
     */
    if (cur_vars->fmt_size)
	result = SaveSegment (canvas, cur_vars, _CESetNewLineFlag (cur_type),
					font_attr, link_spec);
    /*
     * check to see if there is any segments. If not create a new one
     * with the type NOOP.
     */
    else if (!cur_vars->seg_cnt)
	result = SaveSegment (canvas, cur_vars, _CESetNewLineFlag (CE_NOOP),
					font_attr, link_spec);
    /*
     * There was not any information in the buffer and we have one or
     * more segments. Try placing the flag on the previous segment.
     */
    else 
      {
	pSeg = &(cur_vars->seg_list[cur_vars->seg_cnt - 1]);
	/*
	 * Does the previous segment already have a newline flag?
	 * If so, create a new NOOP segment with the newline set.
	 */
	if (_CEIsNewLine (pSeg->seg_type))
	    result = SaveSegment (canvas, cur_vars, _CESetNewLineFlag (CE_NOOP),
					font_attr, link_spec);
	else
	    pSeg->seg_type = _CESetNewLineFlag (pSeg->seg_type);
      }

    cur_vars->last_was_space = True;
    return result;
}

/******************************************************************************
 * Function:	int CreateSaveGraphic (FormatVariables cur_vars,
 *				int type,
 *					char *file_name, int link_spec )
 * 
 * Parameters:
 *		type		Specifies the type of graphic segment
 *				being processed.
 *		file_name	Specifies the file name of the graphic.
 *		link_spec	Specifies the hypertext link specification
 *				associated with the graphic.
 *
 * Returns:	 0	if successful, -1 if errors.
 *
 * errno Values:
 *		CEErrorMalloc
 *
 * Purpose:	Save a graphic segment.
 *		If 'file_name' is not an absolute path, resolve it to
 *		a full path by using the path to the volume.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
CreateSaveGraphic (cur_vars, type, file_name, link_spec )
    FormatVariables	*cur_vars;
    int			 type;
    char		*file_name;
    int			 link_spec;
#else
CreateSaveGraphic (
    FormatVariables	*cur_vars,
    int			 type,
    char		*file_name,
    int			 link_spec)
#endif /* _NO_PROTO */
{
    char	     *fullName;

    if (CheckSegList (cur_vars) == -1)
	return -1;

    if (*file_name == '/')
	fullName = strdup (file_name);
    else
      {
	fullName = (char *) malloc (strlen (cur_vars->my_path) +
						strlen (file_name) + 2);
	if (fullName == NULL)
	  {
	    errno = CEErrorMalloc;
	    return -1;
	  }
	strcpy (fullName, cur_vars->my_path);
	strcat (fullName, "/");
	strcat (fullName, file_name);
      }

    cur_vars->seg_list[cur_vars->seg_cnt].seg_type   = _CEClearInitialized(type);
    cur_vars->seg_list[cur_vars->seg_cnt].seg_handle.file_handle = fullName;
    cur_vars->seg_list[cur_vars->seg_cnt].link_index = link_spec;

    cur_vars->seg_cnt++;

    return 0;
}

/******************************************************************************
 * Function:	int ChangeFont (int whichOne, int segType,
 *				char **font_attr, int linkspec,
 *				char *description,
 *				ControlStruct *parent,
 *				FormatVariables *cur_vars, int flags)
 * 
 * Parameters:
 *		whichOne	Specifies the index into 'font_attr' to
 *				change.
 *		segType		Specifies the type of segment currently
 *				being processed.
 *		font_attr	Specifies the list of font quarks associated
 *				with the current segment.
 *		linkspec	Specifies the hypertext link specification
 *				associated with the segment.
 *		description	Not Used.
 *		flags		Specifies the routines flags.
 *
 * Returns:	The value returned from 'routine' if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Saves any segment with the current font type and
 *		process the next segment using the new font type.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ChangeFont (canvas, whichOne, segType, font_attr, linkspec, description, parent,
		cur_vars, flags, fnt_flg )
    CECanvasStruct	*canvas;
    int			 whichOne;
    int			 segType;
    CEFontSpec		*font_attr;
    int			 linkspec;
    char		*description;
    ControlStruct	*parent;
    FormatVariables	*cur_vars;
    int			 flags;
    int			 fnt_flg;
#else
ChangeFont(
    CECanvasStruct	*canvas,
    int			 whichOne,
    int			 segType,
    CEFontSpec		*font_attr,
    int			 linkspec,
    char		*description,
    ControlStruct	*parent,
    FormatVariables	*cur_vars,
    int			 flags,
    int			 fnt_flg)
#endif /* _NO_PROTO */
{
    int    result;
    int    oldMb_Len = cur_vars->cur_mb_max;
    char  *fontString;
    CEFontSpec oldFontStruct;

    if (CheckSaveSegment (canvas, segType, font_attr, linkspec, cur_vars) == -1)
	return -1;

    oldFontStruct = *font_attr;

    if (GetStringParameter (cur_vars, True, True, False, False, &fontString) == -1)
	return -1;

    /*
     * Is this font change allowed to go through?
     */
    if (!(fnt_flg & (1 << whichOne)))
      {
        switch (whichOne)
          {
	    case _CEFONT_SPACING:
			font_attr->font_specs.spacing = CEFontSpacingProp;
			if (fontString[0] == 'm')
			    font_attr->font_specs.spacing = CEFontSpacingMono;
			break;
	    case _CEFONT_SIZE:
			font_attr->font_specs.pointsz = atoi(fontString);
			break;
	    case _CEFONT_ANGLE:
			font_attr->font_specs.slant = CEFontSlantRoman;
			if (fontString[0] == 'i')
			    font_attr->font_specs.slant = CEFontSlantItalic;
			break;
	    case _CEFONT_WEIGHT:
			font_attr->font_specs.weight = CEFontWeightMedium;
			if (fontString[0] == 'b')
			    font_attr->font_specs.weight = CEFontWeightBold;
			break;
	    case _CEFONT_TYPE:
			font_attr->font_specs.style = CEFontStyleSanSerif;
			if (*fontString == 's')
			  {
			    if (fontString[1] == 'e')
			        font_attr->font_specs.style = CEFontStyleSerif;
			    else if (fontString[1] == 'y')
			        font_attr->font_specs.style = CEFontStyleSymbol;
			  }
			break;
	    case _CEFONT_CHAR_SET:
			/*
			 * Change to the correct mb_len.
			 */
			GetMbLen(cur_vars, fontString, &(font_attr->lang),
					&(font_attr->char_set));
			break;
          }
      }

    result = FindEndMarker (cur_vars);
    if (!result)
	result = Parse (canvas, CE_FONT_CMD, parent, cur_vars, segType,
				font_attr, linkspec, description,
				Specials, flags, fnt_flg);
    /*
     * free the lanugage and code sets strings.
     */
    if (!(fnt_flg & (1 << whichOne)) && whichOne == _CEFONT_CHAR_SET)
      {
	free(font_attr->lang);
	free(font_attr->char_set);
      }

    /*
     * reset the old quark
     * if necessary, reset the MB_LEN
     */
    *font_attr = oldFontStruct;
    cur_vars->cur_mb_max = oldMb_Len;

    /*
     * free the memory
     */
    free (fontString);

    if (result == -1)
	return -1;

    return 0;

} /* ChangeFont */

/******************************************************************************
 * Function:	int SkipToNextToken (FormatVariables *cur_vars, CEBoolean flag)
 * 
 * Parameters:
 *		flag	Specifies whether the routine returns a -1
 *				if '>' is the next token.
 *
 * Returns:	The value from __DtHelpSkipToNextToken:
 *			-1  If problems encountered finding the next token.
 *			 0  If no problems encountered finding the next token.
 *			 1  If flag is true and the next token is a > character.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpSkipToNextToken.
 *		Skip the current string and any spaces or newline
 *		characters after it.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
SkipToNextToken (cur_vars, flag )
    FormatVariables	 *cur_vars;
#else
SkipToNextToken (
    FormatVariables	 *cur_vars,
    CEBoolean		  flag)
#endif /* _NO_PROTO */
{
    return (_DtHelpCeSkipToNextCcdfToken (cur_vars->my_file, cur_vars->rd_buf,
		cur_vars->rd_size, 1, &(cur_vars->rd_ptr), flag));
}

/******************************************************************************
 * Function:	int GetStringParameter (FormatVariables *cur_vars,
 *				CEBoolean flag, CEBoolean eat_escape,
 *				CEBoolean ignore_quotes, CEBoolean less_test,
 *				char **ret_string)
 * 
 * Parameters:
 *		flag            Specifies whether the routine returns
 *                                      a -1 if '>' is the next token.
 *              eat_secape      Specifies whether the backslash is not
 *                                      placed in the returned string.
 *                                      True - it is skipped.
 *                                      False - it is saved in 'ret_string'.
 *              ignore_quotes   Specifies whether quotes are to be included
 *                                      in the returned string.
 *              less_test       Specifies whether the routine should
 *                                      stop when it finds a '<' character.
 *              ret_string      Returns the string found.
 *				If NULL, throws the information away.
 *
 * Returns:	The value from __DtHelpFormatGetStringParameter:
 *			-1  If problems encountered.
 *			 0  If no problems encountered getting the string.
 *			 1  If flag is false and the no string was found.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpFormatGetStringParameter.
 *		Skip the current string and any spaces or newline
 *		characters after it. Get the next quoted/unquoted
 *		string after that.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GetStringParameter(cur_vars, flag, eat_escape, ignore_quotes, less_test,
		ret_string)
    FormatVariables	 *cur_vars;
    CEBoolean		  flag;
    CEBoolean		  eat_escape;
    CEBoolean		  ignore_quotes;
    CEBoolean		  less_test;
    char		**ret_string;
#else
GetStringParameter(
    FormatVariables	 *cur_vars,
    CEBoolean		  flag,
    CEBoolean		  eat_escape,
    CEBoolean		  ignore_quotes,
    CEBoolean		  less_test,
    char		**ret_string)
#endif /* _NO_PROTO */
{
    return (_DtHelpCeGetCcdfStrParam (cur_vars->my_file,
		cur_vars->rd_buf, cur_vars->rd_size, cur_vars->cur_mb_max,
		&(cur_vars->rd_ptr),
		flag, eat_escape, ignore_quotes, less_test, ret_string));
}

/******************************************************************************
 * Function:	int GetValueParameter (FormatVariables *cur_vars,
 *				CEBoolean flag, int *ret_value)
 * 
 * Parameters:
 *              flag            Specifies whether the routine returns
 *                                      a -2 if '>' is the next token.
 *              ret_value       Returns the atoi conversion
 *                              of the string found.
 *
 * Returns:	The value from __DtHelpFormatGetValueParameter:
 *			-1  If problems encountered.
 *			 0  If no problems encountered getting the string.
 *			-2  If flag is true and the next token is a >
 *				character.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpFormatGetValueParameter.
 *		Skip the current string and any spaces or newline
 *		characters after it. Process the next string as
 *		a numeric value.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GetValueParameter (cur_vars, flag, ret_value )
    FormatVariables	*cur_vars,
    CEBoolean		 flag;
    int			*ret_value;
#else
GetValueParameter(
    FormatVariables	*cur_vars,
    CEBoolean		 flag,
    int			*ret_value )
#endif /* _NO_PROTO */
{
    return (_DtHelpCeGetCcdfValueParam (cur_vars->my_file, cur_vars->rd_buf,
			cur_vars->rd_size,
			&(cur_vars->rd_ptr), flag, cur_vars->cur_mb_max,
			ret_value));
}

/******************************************************************************
 * Function:	int GetParagraphParameters (FormatVariables *cur_vars,
 *				int seg_type, int graphic_type,
 *				char **label, char **file_name,
 *				char **link_string, int *link_type,
 *				char **description)
 *
 * Parameters:
 *              seg_type	Specifies the default type for the segment.
 *				Returns the new type for the segment.
 *		graphic_type	Specifies the default type for a graphic
 *					if a justified graphic is
 *					encountered in the paragraph options.
 *				Returns the new type for a graphic
 *					if a justified graphic was 
 *					encountered in the paragraph options.
 *		label		Returns the label if one is specified
 *					in the paragraph options.
 *		file_name	Returns the file name of a graphic if
 *					one is specified in the paragraph
 *					options.
 *		link_string	Returns the hypertext specification if one
 *					is specified in the paragraph options.
 *		link_type	Returns the hypertext link type if one is
 *					specified.
 *		description	Returns the hypertext description if one
 *					is specified.
 * 
 * Returns:	0 if successfult, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Wrapper around __DtHelpParagraphGetOptions.
 *		Process the options found in <PARAGRAPH> syntax.
 *		Test to see if the id specified in the <PARAGRAPH>
 *		is the one we are looking at.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
GetParagraphParameters (cur_vars, seg_type, graphic_type, label, file_name, link_string, link_type, description)
    FormatVariables	 *cur_vars;
    int			 *seg_type;
    int			 *graphic_type;
    char		**label;
    char		**file_name;
    char		**link_string;
    int 		 *link_type;
    char		**description;
#else
GetParagraphParameters(
    FormatVariables	 *cur_vars,
    int			 *seg_type,
    int			 *graphic_type,
    char		**label,
    char		**file_name,
    char		**link_string,
    int 		 *link_type,
    char		**description ) 
#endif /* _NO_PROTO */
{
    int		 result = 0;
    char	*tmpString = NULL;
    CEParagraph	*para_entry = &(cur_vars->para_list[cur_vars->para_cnt]);

    result = _DtHelpCeGetCcdfParaOptions (cur_vars->my_file, cur_vars->rd_buf,
			cur_vars->rd_size, cur_vars->cur_mb_max,
			&(cur_vars->rd_ptr),
			seg_type, graphic_type,
			&(para_entry->first_indent),
			&(para_entry->left_margin),
			&(para_entry->right_margin),
			&(para_entry->lines_before),
			&(para_entry->lines_after),
			&(para_entry->spacing),
			label, file_name, link_string, link_type,
			description, &tmpString);
    /*
     * test to see if this location id is the one we are looking for
     */
    if (result != -1 && cur_vars->id_string != NULL && tmpString != NULL)
      {
	 if (_DtHelpCeStrCaseCmp (tmpString, cur_vars->id_string) == 0)
	  {
	    cur_vars->found_id = True;
	    cur_vars->id_para   = cur_vars->para_cnt;
	    cur_vars->id_seg    = 0;
	  }
      }

    /*
     * free the string
     */
    if (tmpString != NULL)
        free (tmpString);

    return result;

} /* End GetParagraphParameters */

/******************************************************************************
 * Function:	int ProcessParagraph (ControlStruct *parent)
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:	Process the <PARAGRAPH> specification.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessParagraph (canvas, parent, cur_vars, fnt_flg)
    CECanvasStruct	*canvas;
    ControlStruct	*parent;
    FormatVariables	*cur_vars;
    int			 fnt_flg;
#else
ProcessParagraph(
    CECanvasStruct	*canvas,
    ControlStruct	*parent,
    FormatVariables	*cur_vars,
    int			 fnt_flg)
#endif /* _NO_PROTO */
{
    int		 result = 0;
    int		 allowed = ~(CE_TOPIC_CMD | CE_TITLE_CMD | CE_ABBREV_CMD);
    int		 segType;
    int		 graphicType;
    int		 linkType;
    int		 oldMyIndex;
    int		 linkIndex    = -1;
    int          oldSegCount  = cur_vars->seg_cnt;
    int          oldSegMax    = cur_vars->seg_max;
    int		 oldMbLenMax  = cur_vars->cur_mb_max;
    char	*description  = NULL;
    char	*label        = NULL;
    char	*linkSpec     = NULL;
    char	*fileName     = NULL;
    CEBoolean	 labelBody    = False;
    CEBoolean	 graphicBody  = False;
    CEFontSpec	 fontAttrs;
    CESegment  *oldSegList = cur_vars->seg_list;
    ControlStruct currParagraph;
    ControlStruct *realParent = parent;
    ControlStruct *realMe;

    /*
     * remember the old font list.
     * initialize the font quark list
     * and use the char set specified for this topic.
     */
    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    fontAttrs.char_set   = cur_vars->topic_char_set;
    cur_vars->cur_mb_max = cur_vars->topic_mb_max;
    fontAttrs.lang       = cur_vars->topic_lang;

    /*
     * terminate the last segment.
     */
    oldMyIndex = realParent->my_index;
    if (cur_vars->seg_list)
      {
        if (TerminateSegList (cur_vars) == -1)
	    return -1;

	realParent->my_index = cur_vars->para_cnt;
        cur_vars->para_cnt++;
      }

    if (InitializeParagraph (cur_vars, GROW_SIZE ) == -1)
	return -1;

    graphicType = _CESetGraphicInfo (0, CE_LEFT_JUSTIFIED);
    segType     = _CESetString(0);
    linkType    = CCDF_LINK_JUMP_REUSE;

    /*
     * To begin, the new paragraph will inheirit its parent's indents.
     * Though first indent is funny in that it inherits its parent's
     * left indent.
     */
    cur_vars->para_list[cur_vars->para_cnt] = realParent->values;
    cur_vars->para_list[cur_vars->para_cnt].first_indent =
			cur_vars->para_list[cur_vars->para_cnt].left_margin;
    cur_vars->para_list[cur_vars->para_cnt].lines_before = 0;
    cur_vars->para_list[cur_vars->para_cnt].lines_after  = 1;

    result = GetParagraphParameters (cur_vars, &segType, &graphicType, &label,
				&fileName, &linkSpec, &linkType, &description);

    currParagraph.values = cur_vars->para_list[cur_vars->para_cnt];
    currParagraph.my_index     = -1;
    currParagraph.first_child  = -1;
    currParagraph.last_child   = -1;
    currParagraph.my_parent    = realParent;
    realMe = &currParagraph;

    if (result != -1)
      {
	if (_CEIsHyperLink (graphicType))
	  {
	    if (linkSpec && strlen (linkSpec))
	      {
		linkIndex = _DtHelpCeAddLinkToList (&(cur_vars->my_links),
					linkSpec,
					ReturnLinkType(linkType, linkSpec),
					ReturnLinkWinHint(linkType),
					description);
		if (linkIndex == -1)
		    result = -1;
	      }
	    else
	      {
		graphicType = _CEClearHyperFlag (graphicType);
		free (linkSpec);
		free (description);
	      }
	  }
      }

    /*
     * there were no parameters to the paragraph tag and
     * the previous paragraph contains only a label without a newline.
     */
    if (result == 0 && oldSegList &&
	_CEIsLabel (oldSegList[(oldSegCount - 1)].seg_type) &&
	_CEIsNotNewLine (oldSegList[(oldSegCount - 1)].seg_type))
      {
	/*
	 * Attach myself to the previous paragraph.
	 */
	cur_vars->para_cnt--;
	cur_vars->seg_list = oldSegList;
	cur_vars->seg_cnt  = oldSegCount;
	cur_vars->seg_max  = oldSegMax;

	labelBody = True;
      }
    else if (result == 0 && oldSegList && oldSegCount == 1 &&
	_CEIsGraphic (oldSegList[0].seg_type) &&
	(_CEGetGraphicType (oldSegList[0].seg_type) == CE_LEFT_JUSTIFIED ||
	 _CEGetGraphicType (oldSegList[0].seg_type) == CE_RIGHT_JUSTIFIED) &&
	_CEIsNotNewLine (oldSegList[(oldSegCount - 1)].seg_type))
      {
	/*
	 * Attach myself to the previous paragraph.
	 */
	cur_vars->para_cnt--;
	cur_vars->seg_list = oldSegList;
	cur_vars->seg_cnt  = oldSegCount;
	cur_vars->seg_max  = oldSegMax;

	graphicBody = True;
      }
    else if (result != -1)
      {
	/*
	 *	<P before 2>			blank line
	 *	  <P before 1>   ----->   	blank line
	 *		text			text
	 *	  </P>
	 *	</P>
	 */
	if (realParent->my_index == -1 && realParent->first_child == -1 &&
	    realParent->values.lines_before > cur_vars->para_list[cur_vars->para_cnt].lines_before)
	    cur_vars->para_list[cur_vars->para_cnt].lines_before = parent->values.lines_before;

	if (label)
	  {
	    FormatVariables   oldVars = *cur_vars;

	    cur_vars->rd_buf  = label;
	    cur_vars->rd_flag = 0;
	    cur_vars->rd_ptr  = label;
	    cur_vars->my_file = NULL;

	    result = Parse (canvas, CE_LABEL_CMD, realMe, cur_vars,
					_CESetLabel(segType),
					&fontAttrs, -1, NULL,
					Specials,
					(CE_FONT_CMD | CE_OCTAL_CMD),
					fnt_flg);

	    if (result != -1)
	        result = CheckSaveSegment (canvas, segType, &fontAttrs, -1, cur_vars);

	    cur_vars->rd_buf  = oldVars.rd_buf;
	    cur_vars->rd_flag = oldVars.rd_flag;
	    cur_vars->rd_ptr  = oldVars.rd_ptr;
	    cur_vars->my_file = oldVars.my_file;
	    cur_vars->last_was_space = True;
	    allowed &= (~(CE_LABEL_CMD));
	  }
      }

    if (result != -1)
      {
	if (labelBody || graphicBody)
	  {
	    /*
	     * since this paragraph is being attached to a label or graphic
	     * we need to change the current paragraph control structure
	     * to be what was my parent's, and my parent is really was
	     * was my grandparent.
	     */
	    if (realParent->my_index == cur_vars->para_cnt)
		realParent->my_index = oldMyIndex;

	    realMe     = parent;
	    realParent = parent->my_parent;
	  }

	if (fileName)
	    result = CreateSaveGraphic (cur_vars, graphicType,
							fileName, linkIndex );

	if (result != -1)
	    result = Parse (canvas, CE_PARAGRAPH_CMD, realMe, cur_vars, segType,
					&fontAttrs, -1, NULL,
					Specials, allowed,
					fnt_flg);

	if (result != -1)
	  {
	    if (cur_vars->seg_list)
	      {
	        result = TerminateSegList (cur_vars);

		if (result != -1)
		  {
	            /*
	             * clean up the last segment.
	             * If it was the body of a label paragraph, but the child
	             * paragraph was empty, force a newline.
	             */
	            if (labelBody || graphicBody)
		      {
			CESegment *pSeg =
				&cur_vars->seg_list[cur_vars->seg_cnt - 1];

	                if (cur_vars->seg_cnt == oldSegCount &&
					_CEIsNotNewLine (pSeg->seg_type))
		            pSeg->seg_type = _CESetNewLineFlag(pSeg->seg_type);
		       }
		    /*
		     * check parent's indexing.
		     */
		    if (realParent->first_child == -1)
		        realParent->first_child = cur_vars->para_cnt;
		    
		    /*
		     * make sure we honor the child's lines before value
		     */
		    if (realParent->my_index != -1)
			cur_vars->para_list[realParent->my_index].lines_after = 0;

		    realParent->last_child = cur_vars->para_cnt;

	            cur_vars->para_cnt++;
	          }
	      }
	    else if (realMe->last_child != -1)
	      {
		/*
		 * If we have:
		 *	<P after 1>
		 *	    <P after 0>
		 *		data
		 *	    </P>
		 *	</P>
		 *
		 * we won't get one line after the paragraph (like we should)
		 * because there is no data after the close of the child
		 * paragraph.
		 */
		realParent->last_child = realMe->last_child;
		if (realMe->values.lines_after >
				cur_vars->para_list[realMe->last_child].lines_after)
		    cur_vars->para_list[realMe->last_child].lines_after =
						realMe->values.lines_after;
	      }
	  }
      }

    /*
     * initialize the next paragraph
     */
    if (result != -1)
        result = InitializeParagraph (cur_vars, GROW_SIZE );

    /*
     * Restore the parent paragraph information.
     */
    cur_vars->para_list[cur_vars->para_cnt] = parent->values;

    /*
     * restore the font MB_CUR_MAX
     */
    cur_vars->cur_mb_max = oldMbLenMax;

    /*
     * free all the strings
     */
    if (label != NULL)
        free (label);
    if (fileName != NULL)
        free (fileName);

    /*
     * do not free the linkSpec and description, they are owned by
     * the Hyper List functions.
     */

    if (result == -1)
	return -1;

    return 0;

} /* End ProcessParagraph */

/******************************************************************************
 * Function:	int ProcessHypertext (ControlStruct *parent,
 *					FormatVariables *cur_vars, int segType,
 *					char **font_attr, int flags )
 *
 * Parameters:
 *		segType		Specifies the type of segment currently
 *				being processed.
 *		font_attr		Specifies the list of font quarks to
 *				associate with the string.
 *		flags		Specifies the formatting commands allowed.
 * 
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Process the <LINK> specification.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessHypertext (canvas, parent, cur_vars, segType, font_attr, flags, fnt_flg)
    CECanvasStruct	*canvas;
    ControlStruct	 *parent;
    FormatVariables	 *cur_vars;
    int			  segType;
    CEFontSpec		*font_attr;
    int			  flags;
    int			  fnt_flg;
#else
ProcessHypertext(
    CECanvasStruct	*canvas,
    ControlStruct	 *parent,
    FormatVariables	 *cur_vars,
    int			  segType,
    CEFontSpec		*font_attr,
    int			  flags,
    int			  fnt_flg)
#endif /* _NO_PROTO */
{
    int    result = 0;
    int    hyperType;
    int    hyperIndex;
    char  *description = NULL;
    char  *hyperlink = NULL;

    /*
     * is a <LINK> command allowed here?
     */
    if (CE_NOT_ALLOW_CMD(flags, CE_LINK_CMD))
	return -1;

    /*
     * get the hypertext type
     */
    if (GetValueParameter (cur_vars, True, &hyperType) < 0)
	return -1;

    if (hyperType < 0)
      {
	errno = CEErrorHyperType;
	return -1;
      }

    /*
     * get the hypertext link spec.
     */
    result = GetStringParameter (cur_vars, True, True, False, False, &hyperlink);
    if (!result)
      {
        /*
         * See if we have the optional description string
         */
        result = GetStringParameter (cur_vars, False, True, False, False, &description);
        if (result == 1)
	    description = NULL;

	result = _DtHelpCeAddLinkToList (&(cur_vars->my_links), hyperlink,
					ReturnLinkType(hyperType, hyperlink),
					ReturnLinkWinHint(hyperType),
					description);
      }

    /*
     * if no problems encountered, start parsing with this
     * hypertext link.
     */
    if (result != -1)
      {
	hyperIndex = result;
        result = FindEndMarker (cur_vars);
        if (!result)
	  {
	    /*
	     * set the hypertext flag and type.
	     */
	    segType = _CESetHyperFlag(segType);

	    result = Parse (canvas, CE_LINK_CMD, parent, cur_vars, segType, font_attr,
						hyperIndex, NULL,
						Specials, flags,
						fnt_flg);
	  }
      }

    /*
     * Don't free the hypertext string or description.
     * It is now owned by the Link List.
     */

    if (result == -1)
	return -1;

    return 0;

} /* End ProcessHypertext */

/******************************************************************************
 * Function:	int ProcessLabelCmd (ControlStruct *parent,
 *				FormatVariables *cur_vars, int seg_type,
 *				char **font_attr, int link_spec,
 *				char *description, int flags )
 *
 * Parameters:
 *		font_attr	Specifies the list of font quarks to
 *				associate with the string.
 *		link_spec	Specifies the hypertext link to associate
 *				with the string.
 *		description	Specifies the description of the hypertext
 *				link.
 *		flags		Specifies the formatting commands allowed.
 * 
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Process the <LABEL> specification.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessLabelCmd (canvas, parent, cur_vars, seg_type, font_attr, link_spec,
		description, flags, fnt_flg )
    CECanvasStruct	*canvas;
    ControlStruct	 *parent;
    FormatVariables	 *cur_vars;
    int			  seg_type;
	CEFontSpec		*font_attr;
    int			  link_spec;
    char		 *description;
    int			  flags;
    int			  fnt_flg;
#else
ProcessLabelCmd(
    CECanvasStruct	*canvas,
    ControlStruct	  *parent,
    FormatVariables	  *cur_vars,
    int			  seg_type,
    CEFontSpec		*font_attr,
    int			  link_spec,
    char		 *description,
    int			  flags,
    int			  fnt_flg)
#endif /* _NO_PROTO */
{
    /*
     * is a <LABEL> command allowed?
     * it has to be the first item in a paragraph.
     */
    if (cur_vars->seg_list != NULL || CE_NOT_ALLOW_CMD(flags, CE_LABEL_CMD))
	return -1;

    /*
     * can't have another <LABEL> command inside this one
     */
    flags &= (~(CE_LABEL_CMD));

    /*
     * Find the end marker
     */
    if (FindEndMarker (cur_vars) != 0)
	return -1;

    seg_type = _CESetLabel (seg_type);
    if (Parse (canvas, CE_LABEL_CMD, parent, cur_vars, seg_type, font_attr,
					link_spec, description,
					Specials, flags,
					fnt_flg) == -1)
	return -1;

    /*
     * Indicate that preceding space on the next text should be ignored
     */
    cur_vars->last_was_space = True;

    return 0;

} /* End ProcessLabelCmd */

/******************************************************************************
 * Function:	int ProcessFigureCmd (ControlStruct *parent,
 *					FormatVariables *cur_vars,
 *					char **font_attr)
 *
 * Parameters:
 *		font_attr		Specifies the list of font quarks to
 *				associate with the string.
 * 
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Process the <FIGURE> specification.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessFigureCmd ( canvas, parent, cur_vars, font_attr )
    CECanvasStruct	*canvas,
    ControlStruct	 *parent;
    FormatVariables	 *cur_vars;
    CEFontSpec		*font_attr;
    int			 fnt_flg;
#else
ProcessFigureCmd(
    CECanvasStruct	*canvas,
    ControlStruct	 *parent,
    FormatVariables	 *cur_vars,
    CEFontSpec		*font_attr,
    int			 fnt_flg)
#endif /* _NO_PROTO */
{
    CEBoolean	done         = False;
    int		result       = 0;
    int		capType;
    int		linkType     = CCDF_LINK_JUMP_REUSE;
    int		linkIndex    = -1;
    int		segType;
    char	*description = NULL;
    char	*id_string   = NULL;
    char	*filename    = NULL;
    char	*linkspec    = NULL;

    /*
     * clean out the segType flags we don't want and
     * set them to what we want.
     */
    segType = _CESetGraphicPosition(
			_CESetGraphicInfo (0, CE_ALONE_CENTERED),
			CE_AL_CENTER);

    /*
     * initialize a new type for any caption that might be specified.
     */
    capType = _CESetCaptionAtBottom(
			_CESetCaptionInfo(0, CE_CAPTION_CENTER));

    while (!done && result != -1)
      {
	if (SkipToNextToken (cur_vars, False) == -1)
	  {
	    result = -1;
	    continue;
	  }

	switch (tolower(*(cur_vars->rd_ptr)))
	  {
	    /*
	     * end of figure spec
	     */
	    case '>':
		/*
		 * move past the end of token marker
		 * and skip the leading blanks in the caption.
		 */
		(cur_vars->rd_ptr)++;
		while (result != -1 && !done)
		  {
		    if (*(cur_vars->rd_ptr) == '\0')
			result = GetNextBuffer (cur_vars);
		    else if ((cur_vars->cur_mb_max == 1 ||
			mblen(cur_vars->rd_ptr, cur_vars->cur_mb_max) == 1)
						&& *(cur_vars->rd_ptr) == ' ')
			(cur_vars->rd_ptr)++;
		    else
			done = True;
		  }
		break;

	    /*
	     * ccenter
	     * center
	     * cbottom
	     * cleft
	     * cright
	     * ctop
	     */
	    case 'c':
		/*
		 * Go to the next character.
		 */
		(cur_vars->rd_ptr)++;

		/*
		 * Do we need to read more information?
		 */
		if (*(cur_vars->rd_ptr) == '\0' && GetNextBuffer (cur_vars) == -1)
		    result = -1;

		if (result != -1)
		  {
		    /*
		     * look at the next charager to determine the token.
		     */
		    switch (tolower(*(cur_vars->rd_ptr)))
		      {
		        case 'c':
			    capType = _CESetCaptionPosition (capType,
							CE_CAPTION_CENTER);
			    break;
		        case 'e':
			    segType = _CESetGraphicPosition (segType,
								CE_AL_CENTER);
			    break;
		        case 'b':
			    capType = _CESetCaptionAtBottom (capType);
			    break;
		        case 'l':
			    capType = _CESetCaptionPosition (capType,
							CE_CAPTION_LEFT);
			    break;
		        case 'r':
			    capType = _CESetCaptionPosition (capType,
							CE_CAPTION_RIGHT);
			    break;
		        case 't':
			    capType = _CESetCaptionAtTop (capType);
			    break;
		      }
		  }
		break;

	    /*
	     * description [string | "string" | 'string']
	     */
	    case 'd':
		result = GetStringParameter (cur_vars, True, True, False,
							False, &description);
		break;

	    /*
	     * file
	     */
	    case 'f':
		result = GetStringParameter (cur_vars, True, True, False,
							False, &filename);
		break;

	    /*
	     * id string
	     */
	    case 'i':
		/*
		 * get the id string
		 */
		result = GetStringParameter (cur_vars, True, True, False, False,
							&id_string);
		if (result != -1 && cur_vars->id_string != NULL &&
							id_string != NULL)
		  {
		    /*
		     * test to see if it is the one we are looking for
		     */
		    if (_DtHelpCeStrCaseCmp (id_string, cur_vars->id_string) == 0)
		      {
		        cur_vars->found_id = True;
		        cur_vars->id_para   = cur_vars->para_cnt;
		        cur_vars->id_seg    = cur_vars->seg_cnt;
		      }
		  }

		/*
		 * free the string
		 */
		free (id_string);

		break;

	    /*
	     * left
	     * link [string | "string" | 'string']
	     */
	    case 'l':
		/*
		 * Go to the next character.
		 */
		(cur_vars->rd_ptr)++;

		/*
		 * Do we need to read more information?
		 */
		if (*(cur_vars->rd_ptr) == '\0' && GetNextBuffer (cur_vars) == -1)
		    result = -1;

		if (result != -1)
		  {
		    /*
		     * look at the next charager to determine the token.
		     */
		    switch (tolower(*(cur_vars->rd_ptr)))
		      {
		        case 'e':
			    segType = _CESetGraphicPosition (segType,
								 CE_AL_LEFT);
			    break;
		        case 'i':
			    segType = _CESetHyperFlag (segType);
			    result = GetStringParameter (cur_vars, True, True,
						False, False, &linkspec);
			    break;
		        default:
			    result = -1;
		      }
		  }
		break;

	    /*
	     * right
	     */
	    case 'r':
		segType = _CESetGraphicPosition (segType, CE_AL_RIGHT);
		break;

	    /*
	     * typelink value
	     */
	    case 't':
		if (GetValueParameter (cur_vars, True, &linkType) == -1)
		    result = -1;
		else if (linkType < 0)
		  {
		    errno = CEErrorHyperType;
		    result = -1;
		  }
		break;

	    default:
		errno  = CEErrorFigureSyntax;
		result = -1;
	  }
      }

    if (result != -1 && (filename == NULL || strlen(filename) == 0))
      {
	errno  = CEErrorFigureSyntax;
	result = -1;
      }

    if (result != -1)
      {
        if (_CEIsHyperLink(segType) && !linkspec)
	  {
	    segType = _CEClearHyperFlag (segType);
	    free (description);
	    description = NULL;
	  }

	if (_CEIsHyperLink(segType))
	  {
	    linkIndex = _DtHelpCeAddLinkToList (&(cur_vars->my_links),
					linkspec,
					ReturnLinkType(linkType, linkspec),
					ReturnLinkWinHint(linkType),
					description);
	    if (linkIndex == -1)
		result = -1;
	  }

      }

    if (result != -1)
      {
	if (_CEIsCaptionAtTop (capType))
	    result = Parse (canvas, CE_FIGURE_CMD, parent, cur_vars,
	   			 capType, font_attr, -1, NULL,
				Specials,
				(CE_NEWLINE_CMD | CE_LINK_CMD |
				 CE_FONT_CMD    | CE_ID_CMD),
				 fnt_flg);

	if (result != -1)
	  {
	    result = CreateSaveGraphic (cur_vars, segType, filename, linkIndex);

	    if (result != -1 && _CEIsCaptionNotAtTop (capType))
	        result = Parse (canvas, CE_FIGURE_CMD, parent, cur_vars,
				capType, font_attr, -1, NULL,
				Specials,
				(CE_NEWLINE_CMD | CE_LINK_CMD |
				 CE_FONT_CMD    | CE_ID_CMD),
				 fnt_flg);
	  }
      }

    free (filename);

    /*
     * don't free the link string or description,
     * the link list owns them now.
     */

    return result;

} /* End ProcessFigureCmd */

/******************************************************************************
 * Function:	int ProcessInLine (FormatVariables cur_vars,
 *					int seg_type, int link_spec )
 *
 * Parameters:
 *		seg_type	Specifes the type of segment currently
 *				being processed.
 *		link_spec	Specifies the hypertext link associated
 *				with this segment.
 * 
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Process a <GRAPHIC> specification.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessInLine (cur_vars, seg_type, link_spec)
    FormatVariables	*cur_vars;
    int			 seg_type;
    int			 link_spec;
#else
ProcessInLine(
    FormatVariables	*cur_vars,
    int			seg_type,
    int			link_spec )
#endif /* _NO_PROTO */
{
    CEBoolean	done = False;
    int		result = 0;
    int		segType;
    char	*idString = NULL;
    char	*filename = NULL;

    segType = _CESetGraphicInfo (seg_type, CE_IN_LINE);
    while (!done && result != -1)
      {
	if (SkipToNextToken (cur_vars, False) == -1)
	    return -1;

	switch (tolower(*(cur_vars->rd_ptr)))
	  {
	    /*
	     * end of in line spec
	     */
	    case '>':
		/*
		 * move past the end of token marker
		 */
		(cur_vars->rd_ptr)++;
		done = True;
		break;

	    /*
	     * file
	     */
	    case 'f':
		result = GetStringParameter (cur_vars, True, True, False,
							False, &filename);
		break;

	    /*
	     * id string
	     */
	    case 'i':
		/*
		 * get the id string
		 */
		result = GetStringParameter (cur_vars, True, True, False,
							False, &idString);
		if (result != -1 && cur_vars->id_string != NULL &&
							idString != NULL)
		  {
		    /*
		     * test to see if it is the one we are looking for
		     */
		    if (_DtHelpCeStrCaseCmp (idString, cur_vars->id_string) == 0)
		      {
		        cur_vars->found_id = True;
		        cur_vars->id_para   = cur_vars->para_cnt;
		        cur_vars->id_seg    = cur_vars->seg_cnt;
		      }
		  }

		/*
		 * free the string
		 */
		free (idString);

		break;

	    default:
		errno  = CEErrorFigureSyntax;
		result = -1;
	  }
      }

    if (result != -1 && (filename == NULL || strlen(filename) == 0))
      {
	errno  = CEErrorFigureSyntax;
	result = -1;
      }

    if (result != -1)
      {
	result = CreateSaveGraphic (cur_vars, segType, filename, link_spec);
	cur_vars->last_was_space = False;
      }

    free (filename);

    return result;

} /* End ProcessInLine */

/******************************************************************************
 * Function:	int CheckIdString (ControlStruct *parent,
 *				FormatVariables *cur_vars, int segType,
 *				char **font_attr, int linkspec,
 *				char *description, int flags)
 * 
 * Parameters:
 *		segType		Specifies the type of segment currently
 *				being processed.
 *		font_attr		Specifies the list of font quarks to
 *				associate with the string.
 *		linkspec	Specifies the hypertext link associated
 *				with the segment.
 *		description	Specifies the hypertext link description
 *				associated with the segment.
 *		flags		Specifies the formatting commands allowed.
 * 
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Check to see if this the id we are looking for.
 * 
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
CheckIdString(canvas, parent, cur_vars, segType, font_attr, linkspec,
		description, flags, fnt_flg)
    CECanvasStruct	*canvas;
    ControlStruct	 *parent;
    FormatVariables	 *cur_vars;
    int			  segType;
	CEFontSpec		*font_attr;
    int			  linkspec;
    char		 *description;
    int			  flags;
    int			  fnt_flg;
#else
CheckIdString(
    CECanvasStruct	*canvas,
    ControlStruct	 *parent,
    FormatVariables	 *cur_vars,
    int			  segType,
    CEFontSpec		*font_attr,
    int			  linkspec,
    char		 *description,
    int			  flags,
    int			  fnt_flg)
#endif /* _NO_PROTO */
{
    int    done;
    char  *ptr = NULL;

    /*
     * is it legal to have a <ID> command at this point?
     */
    if (CE_NOT_ALLOW_CMD (flags, CE_ID_CMD))
	return -1;

    /*
     * get the string
     */
    done = GetStringParameter (cur_vars, True, True, False, False, &ptr);
    if (done != -1)
      {
	if (cur_vars->id_string != NULL && ptr != NULL)
	  {
	    if (_DtHelpCeStrCaseCmp (ptr, cur_vars->id_string) == 0)
	      {
	        /*
	         * clear out any information 
	         * in the buffer
	         */
	        done = CheckSaveSegment (canvas, segType, font_attr, linkspec, cur_vars);
		if (done != -1)
		  {
	            /*
	             * Set the flags.
	             */
	            cur_vars->found_id = True;
	            cur_vars->id_para   = cur_vars->para_cnt;
	            cur_vars->id_seg    = cur_vars->seg_cnt;
	          }
	      }
	  }
	if (done != -1)
	    done = FindEndMarker (cur_vars);

	if (done != -1)
	    done = Parse (canvas, CE_ID_CMD, parent, cur_vars, segType, font_attr,
						linkspec, description,
						Specials, flags,
						fnt_flg);
      }
    if (ptr)
	free (ptr);

    if (done == -1)
	return -1;

    return 0;

} /* End CheckIdString */

/******************************************************************************
 * Function:	int	Parse (int cur_cmd, ControlStruct *parent,
 *				FormatVariables *cur_vars, int segType,
 *				char **font_attr,
 *				int  linkspec, char *description, int allowed)
 * 
 * Parameters:
 *		cur_cmd		Specifies the current formatting command
 *				being processed.
 *		parent		Specifies the parent paragraph.
 *		segType		Specifies the type of segment currently
 *				being processed.
 *		font_attr		Specifies the list of font quarks to
 *				associate with the string.
 *		linkspec	Specifies the hypertext link associated
 *				with the segment.
 *		description	Not Used.
 *		allowed		Specifies the formatting commands allowed.
 * 
 * Returns:
 *		1 if ran into a </> specification.
 *		-1 if errors.
 *
 * errno Values:
 *
 * Purpose:	Parse the data.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
Parse(canvas, cur_cmd, parent, cur_vars, segType, font_attr, linkspec,
	description, scan_string, allowed, fnt_flg)
    CECanvasStruct	*canvas;
    int			  cur_cmd;
    ControlStruct	 *parent;
    FormatVariables	 *cur_vars;
    int			  segType;
	CEFontSpec		*font_attr;
    int			  linkspec;
    char		 *description;
    const char		 *scan_string;
    int			  allowed;
    int			  fnt_flg;
#else
Parse(
    CECanvasStruct	*canvas,
    int			  cur_cmd,
    ControlStruct	 *parent,
    FormatVariables	 *cur_vars,
    int			  segType,
    CEFontSpec		*font_attr,
    int			  linkspec,
    char		 *description,
    const char		 *scan_string,
    int			  allowed,
    int			  fnt_flg)
#endif /* _NO_PROTO */
{
    int      charSize = 1;
    int	     leftOver = 0;
    int	     done = False;
    int	     fontType = False;
    int	     cmdType  = False;
    char    *ptr;

    while (!done)
      {
	if (cur_vars->cur_mb_max != 1)
	    charSize = mblen(cur_vars->rd_ptr, cur_vars->cur_mb_max);

	if (charSize == 1)
	  {
	    /*
	     * check to see if a newline was the previous character.
	     * If so, it may need to be replaced with a space.
	     */
	    if (cur_vars->last_was_nl == True &&
			AppendSpaceToInfo(cur_vars, NULL, segType) == -1)
		return -1;

	    cur_vars->last_was_nl = False;
	    cur_vars->last_was_mb = False;

	    switch (*(cur_vars->rd_ptr))
	      {
		case '<':
		    /*
		     * Go to the next character.
		     */
		    (cur_vars->rd_ptr)++;

		    /*
		     * determine the cmd
		     */
		    cmdType = _DtHelpCeGetCcdfCmd (cur_cmd, cur_vars->rd_buf,
							&(cur_vars->rd_ptr),
							cur_vars->my_file,
							cur_vars->rd_size,
							allowed);
		    switch (cmdType)
		      {
			/*
			 * <figure>
			 */
			case CE_FIGURE_CMD:
			    	if (CheckSaveSegment(canvas, segType, font_attr,
					linkspec, cur_vars) == -1
						||
					ProcessFigureCmd(canvas,parent,cur_vars,
						font_attr,fnt_flg) == -1)
				    return -1;
				break;

			/*
			 * <angle>
			 * <characterset>
			 * <size>
			 * <spacing>
			 * <type>
			 * <weight>
			 */
			case CE_FONT_CMD:
				fontType = _DtHelpCeGetCcdfFontType (cur_vars->rd_ptr);
				done     = ChangeFont(canvas, fontType, segType,
							font_attr , linkspec,
							description, parent,
							cur_vars, allowed,
							fnt_flg);

				break;

			/*
			 * </>
			 */
			case CE_FORMAT_END:
				if (CheckSaveSegment(canvas, segType, font_attr,
						linkspec, cur_vars) == -1 ||
					FindEndMarker (cur_vars) == -1)
				    return -1;

				return 1;

			/*
			 * <graphic>
			 */
			case CE_GRAPHIC_CMD:
				/*
				 * clear out any information
				 * in the buffer and then
				 * process the figure.
				 */
				if (CheckSaveSegment (canvas, segType, font_attr,
						linkspec, cur_vars) == -1
							||
					ProcessInLine (cur_vars, segType,
								linkspec) == -1)
				    done = -1;
				break;

			/*
			 * <id>
			 */
			case CE_ID_CMD:
				done = CheckIdString (canvas, parent, cur_vars,
						      segType, font_attr,
						      linkspec, description,
						      allowed, fnt_flg);
				break;

			/*
			 * <label>
			 */
			case CE_LABEL_CMD:
				if (CheckSaveSegment (canvas, segType, font_attr,
						linkspec, cur_vars) == -1
					||
				    ProcessLabelCmd (canvas, parent, cur_vars, segType,
						font_attr, linkspec,
						description, allowed, fnt_flg) == -1)
				    done = -1;
				break;

			/*
			 * <link>
			 */
			case CE_LINK_CMD:
				if (CheckSaveSegment (canvas, segType, font_attr,
						linkspec, cur_vars) == -1
					||
				     ProcessHypertext(canvas, parent, cur_vars, segType,
						  font_attr, allowed, fnt_flg) == -1)
				    done = -1;
				break;

			/*
			 * <newline>
			 */
			case CE_NEWLINE_CMD:
				if (SaveNewLine (canvas, cur_vars, segType, font_attr,
						linkspec) == -1
						||
					FindEndMarker (cur_vars) == -1)
				    done = -1;
				break;

			case CE_OCTAL_CMD:
				if (AppendOctalToInfo(cur_vars,
							cur_vars->rd_ptr) == -1
					|| FindEndMarker (cur_vars) == -1)
				    done = -1;
				break;

			/*
			 * <paragraph>
			 */
			case CE_PARAGRAPH_CMD:
				if (CheckSaveSegment (canvas, segType, font_attr,
						linkspec, cur_vars) == -1
						||
				      ProcessParagraph(canvas,parent,cur_vars,fnt_flg) == -1)
				    done = -1;
				break;

			/*
			 * unknown command.
			 */
			default:
				return -1;
		      }
		    break;

		case '\n':
			/*
			 * Go to the next character
			 */
			(cur_vars->rd_ptr)++;

			/*
			 * If processing a static segment, we want to
			 * keep the author defined end-of-lines.
			 *
			 * Otherwise, we throw them away.
			 */
			if (_CEIsStaticString (segType) ||
						_CEEndOnNewLine(segType))
			  {
			    done = SaveSegment (canvas, cur_vars,
					_CESetNewLineFlag(segType),
					font_attr, linkspec);

			    if (_CEEndOnNewLine(segType))
				return 0;
			  }
			else if (cur_vars->last_was_space == False)
			    cur_vars->last_was_nl = True;

			break;

		case '\t':
			/*
			 * go past this character.
			 */
			(cur_vars->rd_ptr)++;

			/*
			 * append 1-8 characters on the end of the buffer.
			 */
			_DtHelpCeCountChars(cur_vars->fmt_buf, MB_CUR_MAX,
								&leftOver);
			leftOver = leftOver % 8;
			ptr = ((char *) SpaceString) + leftOver;
			done = _DtHelpCeAddStrToBuf (&ptr,
						&(cur_vars->fmt_buf),
						&(cur_vars->fmt_size),
						&(cur_vars->fmt_buf_max),
						(8 - leftOver),
						INFO_GROW);
			break;

		case '\\':
			/*
			 * The author has escaped a character.
			 * Increment to the escaped character.
			 */
			(cur_vars->rd_ptr)++;

			/*
			 * Do we need to read more information?
			 */
			if (*(cur_vars->rd_ptr) == '\0' &&
						GetNextBuffer (cur_vars) == -1)
			    return -1;
			/*
			 * If we didn't read more information or
			 * was successful on the read, save the
			 * escaped character.
			 */
			done = AppendCharToInfo (cur_vars, &(cur_vars->rd_ptr));
			break;

		case ' ':
			/*
			 * Put a space in the segment.
			 */
			done = AppendSpaceToInfo (cur_vars,
						&(cur_vars->rd_ptr), segType);
			break;

		default:
			/*
			 * put the information in the buffer
			 */
			if (AppendToInfo (cur_vars, &(cur_vars->rd_ptr),
							scan_string) == -1)
			    return -1;
	      }

	  }
	else if (charSize > 1)
	  {
	    if ((canvas->nl_to_space == 1 ||
			(cur_vars->last_was_mb == False
					&& cur_vars->last_was_nl == True))
	         && AppendSpaceToInfo(cur_vars, NULL, segType) == -1)
		done = -1;
		
	    cur_vars->last_was_nl = False;
	    cur_vars->last_was_mb = True;
	    if (AppendToInfo (cur_vars, &(cur_vars->rd_ptr), scan_string) == -1)
		return -1;
	  }
	else if (charSize < 0)
	  {
	    /*
	     * we have either invalid characters or part of a multi-byte
	     * character. Read the next buffer for more info.
	     */
	     leftOver = strlen (cur_vars->rd_ptr);
	     if (leftOver < ((int) MB_CUR_MAX))
	       {
		if (GetNextBuffer (cur_vars) == -1)
		    done = -1;
	       }
	     else
	       {
		 /*
		  * In trouble brothers and sisters. We have garbage in the
		  * buffer - BAIL OUT!
		  */
		 done = -1;
	       }
	  }
	if (!done && *(cur_vars->rd_ptr) == '\0')
	  {
	    /*
	     * We're at the end of the buffer, can we read more?
	     */
	    if (cur_vars->rd_flag > 0 && GetNextBuffer (cur_vars) == -1)
		return -1;

	    if (cur_vars->rd_flag == 0)
		done = True;
          }
      }

    /*
     * DO NOT MODIFY 'done' HERE
     * If you do, the caller is unable to test for return value == 1!!!!
     */
    if (done == -1 ||
	CheckSaveSegment (canvas, segType, font_attr, linkspec, cur_vars) == -1)
	return -1;

    return done;
}

/*****************************************************************************
 * Function:	int Format (FormatVariables cur_vars, char *id_string,
 *				CEParagraph **ret_para,
 *					int *ret_num, CESegment **ret_seg)
 *
 * Parameters:
 *		cur_vars	Specifies the current values for formatting.
 *		id_string	Specifies the id to look for or NULL.
 *		ret_para	Returns a pointer to a list of CEParagraph
 *				structures.
 *		ret_num		Specifies the number of structures in
 *				'ret_para'.
 *		ret_seg		Specifies the segment containing 'id_string'
 *				or NULL.
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:	Format is the top entry point for formating Help Files with
 *		formatting information into a form understood by a display area.
 *		It will keep calling Parse, until the entire topic is read
 *		or an error  occurs.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
Format(canvas, cur_vars, id_string, ret_topic)
	CECanvasStruct	*canvas;
	FormatVariables *cur_vars;
	char		*id_string;
	TopicHandle	*ret_topic;
#else
Format(
	CECanvasStruct	*canvas,
	FormatVariables *cur_vars,
	char		 *id_string,
	TopicHandle	*ret_topic )
#endif /* _NO_PROTO */
{
    char    *charSet = NULL;
    int      result  = 0;
    int      done    = 0;
    CEFontSpec    fontAttrs;
    CETopicStruct *topicHandle;
    ControlStruct parent;
    char    *ptr;
    char     readBuf[BUFF_SIZE];

    /*
     * malloc space for the topic handle.
     */
    topicHandle = (CETopicStruct *) malloc (sizeof(CETopicStruct));
    if (topicHandle == NULL)
      {
	errno = CEErrorMalloc;
	return -1;
      }

    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    if (Initialize (cur_vars, id_string, readBuf, BUFF_SIZE, GROW_SIZE ) == -1)
	return -1;

    cur_vars->rd_flag = _DtHelpCeReadBuf (cur_vars->my_file, cur_vars->rd_buf,
						cur_vars->rd_size);

    if (cur_vars->rd_flag == -1)
	return -1;

    /*
     * initialize the default parent paragraph.
     */
    parent.values = DefaultParagraph;
    parent.my_index = -1;
    parent.first_child  = -1;
    parent.last_child   = -1;
    parent.my_parent    = NULL;

    /*
     * The first command in the buffer MUST be the <TOPIC> command.
     * It is in a one byte charset.
     */
    if (_DtHelpCeGetCcdfTopicCmd (((void *) NULL), cur_vars->my_file,
			cur_vars->rd_buf, &(cur_vars->rd_ptr),
			cur_vars->rd_size, 1, &charSet) == -1)
	return -1;

    /*
     * change the character set to the returned character set
     * Assume that the charset is 'iso8859' if not specified.
     */
    cur_vars->cur_mb_max = 1;
    if (NULL != charSet)
	GetMbLen(cur_vars, charSet, &(fontAttrs.lang), &(fontAttrs.char_set));

    /*
     * Remember the topic code set and MB_CUR_MAX.
     */
    cur_vars->topic_char_set = fontAttrs.char_set;
    cur_vars->topic_mb_max   = cur_vars->cur_mb_max;
    cur_vars->topic_lang     = fontAttrs.lang;

    /*
     * Check for the <TITLE> command.
     */
    if (_DtHelpCeCheckNextCcdfCmd ("ti", cur_vars->my_file, cur_vars->rd_buf,
			cur_vars->rd_size, 1, &(cur_vars->rd_ptr)) != 0)
      {
	if (NULL != charSet)
	  {
	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(charSet);
	  }
	return -1;
      }

    /*
     * Skip the <TITLE> command
     */
    if (FindEndMarker (cur_vars) != 0)
      {
	if (NULL != charSet)
	  {
	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(charSet);
	  }
	return -1;
      }

    /*
     * Parse will return
     *    1 when a </> is found,
     *    0 if the data runs out before we finish parsing,
     *   -1 if errors.
     */
    result = Parse (canvas, CE_TITLE_CMD, &parent, cur_vars, CE_STRING,
			&fontAttrs, -1, NULL, Specials,
			~(CE_TOPIC_CMD | CE_TITLE_CMD | CE_ABBREV_CMD |
			CE_PARAGRAPH_CMD | CE_FIGURE_CMD | CE_LABEL_CMD), 0);

    /*
     * A return of 1 is required from Parse
     */
    if (result == 1)
      {
	result = _DtHelpCeSkipCcdfAbbrev (cur_vars->my_file, cur_vars->rd_buf, &(cur_vars->rd_ptr), cur_vars->rd_size, cur_vars->cur_mb_max);
	if (result != -1 && cur_vars->seg_list)
	  {
	    parent.first_child = cur_vars->para_cnt;
	    parent.last_child  = cur_vars->para_cnt;

	    result = TerminateSegList (cur_vars);
	    if (result != -1)
	      {
	        cur_vars->para_cnt++;
	        result = InitializeParagraph (cur_vars, GROW_SIZE );
              }
          }
      }
    else
	result = -1;

    /*
     * process the rest of the information
     */
    while (!result && !done)
      {
	done = Parse (canvas, CE_PARAGRAPH_CMD, &parent, cur_vars, CE_STRING,
			&fontAttrs, -1, NULL, Specials,
			~(CE_TOPIC_CMD | CE_TITLE_CMD | CE_ABBREV_CMD), 0);

	if (done != -1)
	  {
	    /*
	     * clean up the last segment.
	     */
	    if (cur_vars->seg_list)
	      {
	        result = TerminateSegList (cur_vars);
		if (result != -1)
		    cur_vars->para_cnt++;
	      }
	    if (result != -1)
	        result = InitializeParagraph (cur_vars, GROW_SIZE );
	  }
	else
	    result = -1;
      }

    /*
     * If we had errors, deallocate the memory.
     */
    if (result == -1)
      {
	_DtHelpCeFreeParagraphs(NULL, cur_vars->my_links,
					&(cur_vars->para_list),
					cur_vars->para_cnt);
	cur_vars->found_id = False;
      }
    else
      {
	/*
	 * Tighten up the paragraph structures if too many allocated.
	 */
	if (cur_vars->para_cnt)
	  {
	    if (cur_vars->para_cnt < cur_vars->para_max)
		cur_vars->para_list = (CEParagraph *) realloc(
				(void *) cur_vars->para_list,
				(sizeof (CEParagraph) * cur_vars->para_cnt));
	    if (cur_vars->para_list == NULL)
	      {
		cur_vars->para_cnt = 0;
		errno = CEErrorMalloc;
		result = -1;
	      }
	  }
	else
	  {
	    free ((char *) cur_vars->para_list);
	    cur_vars->para_list = NULL;
	  }
      }

    /*
     * return the values, even if they are null and zero.
     */
    topicHandle->para_num  = cur_vars->para_cnt;
    topicHandle->para_list = cur_vars->para_list;
    topicHandle->id_seg    = NULL;
    topicHandle->link_data = cur_vars->my_links;

    *ret_topic = (TopicHandle) topicHandle;


    if (cur_vars->found_id && cur_vars->id_para != -1)
	topicHandle->id_seg =
	   &(cur_vars->para_list[cur_vars->id_para].seg_list[cur_vars->id_seg]);

    /*
     * free memory
     */
    if (NULL != charSet)
      {
	free(fontAttrs.lang);
	free(fontAttrs.char_set);
	free(charSet);
      }

    if (result == -1)
	return -1;

    return 0;
}

/*****************************************************************************
 * Function:	int FormatCCDFTitle (FormatVariables cur_vars
 *
 * Parameters:
 *		cur_vars	Specifies the current values for formatting.
 *		id_string	Specifies the id to look for or NULL.
 *		ret_para	Returns a pointer to a list of CEParagraph
 *				structures.
 *		ret_num		Specifies the number of structures in
 *				'ret_para'.
 *		ret_seg		Specifies the segment containing 'id_string'
 *				or NULL.
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:	Format is the top entry point for formating Help Files with
 *		formatting information into a form understood by a display area.
 *		It will keep calling Parse, until the entire topic is read
 *		or an error  occurs.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
FormatCCDFTitle(canvas, cur_vars, fontAttrs, filename, offset, level, fnt_flg,
			want_abbrev)
    CECanvasStruct	*canvas;
    FormatVariables	*cur_vars;
    CEFontSpec		 fontAttrs;
    char		*filename;
    int			 offset;
    int			 level;
    int			 fnt_flg;
    CEBoolean		 want_abbrev;
#else
FormatCCDFTitle(
    CECanvasStruct	*canvas,
    FormatVariables	*cur_vars,
    CEFontSpec		 fontAttrs,
    char		*filename,
    int			 offset,
    int			 level,
    int			 fnt_flg,
    CEBoolean		 want_abbrev)
#endif /* _NO_PROTO */
{
    char    *charSet   = NULL;
    int      result    = 0;
    int      segCnt;
    int      origCnt   = cur_vars->seg_cnt;
    char    *ptr;

    cur_vars->rd_buf[0] = '\0';
    cur_vars->rd_ptr    = cur_vars->rd_buf;

    if (_DtHelpCeFileOpenAndSeek(filename,offset,-1,&(cur_vars->my_file),NULL)!=0)
	return -1;

    cur_vars->rd_flag = _DtHelpCeReadBuf (cur_vars->my_file, cur_vars->rd_buf,
						cur_vars->rd_size);

    if (cur_vars->rd_flag == -1)
      {
	_DtHelpCeBufFileClose (cur_vars->my_file, True);
	return -1;
      }

    /*
     * The first command in the buffer MUST be the <TOPIC> command.
     * It is in a one byte charset.
     */
    if (_DtHelpCeGetCcdfTopicCmd (((void *) NULL), cur_vars->my_file,
			cur_vars->rd_buf, &(cur_vars->rd_ptr),
			cur_vars->rd_size, 1, &charSet) == -1)
      {
	_DtHelpCeBufFileClose (cur_vars->my_file, True);
	return -1;
      }

    /*
     * change the character set to the returned character set
     * Assume that the charset is 'iso8859' if not specified.
     */
    cur_vars->cur_mb_max = 1;
    if (NULL != charSet)
	GetMbLen(cur_vars, charSet, &(fontAttrs.lang), &(fontAttrs.char_set));

    /*
     * Remember the topic code set and MB_CUR_MAX.
     */
    cur_vars->topic_char_set = fontAttrs.char_set;
    cur_vars->topic_mb_max   = cur_vars->cur_mb_max;
    cur_vars->topic_lang     = fontAttrs.lang;

    /*
     * Check for the <TITLE> command.
     */
    if (_DtHelpCeCheckNextCcdfCmd ("ti", cur_vars->my_file, cur_vars->rd_buf,
			cur_vars->rd_size, 1, &(cur_vars->rd_ptr)) != 0)
      {
	if (NULL != charSet)
	  {
	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(charSet);
	  }
	_DtHelpCeBufFileClose (cur_vars->my_file, True);
	return -1;
      }

    /*
     * Skip the <TITLE> command
     */
    if (FindEndMarker (cur_vars) != 0)
      {
	if (NULL != charSet)
	  {
	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(charSet);
	  }
	_DtHelpCeBufFileClose (cur_vars->my_file, True);
	return -1;
      }

    /*
     * Parse will return
     *    1 when a </> is found,
     *    0 if the data runs out before we finish parsing,
     *   -1 if errors.
     */
    level = level * 4;
    if (level > 0 && result != -1)
      {
	level++;
	if (cur_vars->fmt_buf_max < level)
	  {
	    if (cur_vars->fmt_buf == NULL)
	        cur_vars->fmt_buf = (char *) malloc(sizeof(char) * level);
	    else
		cur_vars->fmt_buf = (char *) realloc((void *)cur_vars->fmt_buf,
						(sizeof(char) * level));
	    if (cur_vars->fmt_buf == NULL)
	      {
		if (NULL != charSet)
		  {
		    free(fontAttrs.lang);
		    free(fontAttrs.char_set);
		    free(charSet);
		  }
		_DtHelpCeBufFileClose (cur_vars->my_file, True);
		return -1;
	      }
	    cur_vars->fmt_buf_max = level;
	  }

	level--;
	cur_vars->fmt_size       = level;
	cur_vars->fmt_buf[level] = '\0';
	do {
	    level--;
	    cur_vars->fmt_buf[level] = ' ';
	  } while (level > 0);
      }

    if (result == -1 || (level > 0 &&
	CheckSaveSegment(canvas, _CESetStaticFlag(_CESetString(0)),
						&fontAttrs,-1,cur_vars) == -1))
      {
	if (NULL != charSet)
	  {
	    free(fontAttrs.lang);
	    free(fontAttrs.char_set);
	    free(charSet);
	  }
	_DtHelpCeBufFileClose (cur_vars->my_file, True);
	return -1;
      }

    segCnt = cur_vars->seg_cnt;
    if (result != -1)
        result = Parse (canvas, CE_TITLE_CMD, NULL, cur_vars,
			_CESetStaticFlag(CE_STRING),
			&fontAttrs, -1, NULL, Specials,
			~(CE_TOPIC_CMD | CE_TITLE_CMD | CE_ABBREV_CMD |
			CE_PARAGRAPH_CMD | CE_FIGURE_CMD | CE_LABEL_CMD),
			fnt_flg);

    /*
     * A return of 1 is required from Parse
     */
    if (result == 1)
      {
	CEBoolean foundTitle = True;

	/*
	 * if the title is null or we want the abbrev,
	 * process the abbreviation.
	 */
	if (cur_vars->seg_cnt == segCnt)
	    foundTitle = False;
	else
	    TerminateSegList(cur_vars);

	if (foundTitle == False || want_abbrev)
	  {

	    /*
	     * reset the buffer to get the abbreviation
	     */
	    if (cur_vars->fmt_buf != NULL)
	      {
		free(cur_vars->fmt_buf);
		cur_vars->fmt_buf     = NULL;
		cur_vars->fmt_size    = 0;
		cur_vars->fmt_buf_max = 0;
	      }
	    result = _DtHelpCeGetCcdfAbbrevCmd (cur_vars->my_file,
				cur_vars->rd_buf,
				cur_vars->rd_size,
				cur_vars->cur_mb_max,
				&(cur_vars->rd_ptr),
				&(cur_vars->fmt_buf));

	    /*
	     * If no errors getting the abbreviation, save it
	     */
	    if (result != -1)
	      {
		/*
		 * save the abbreviation
		 */
	        if (cur_vars->fmt_buf != NULL)
		  {
		    /*
		     * If there was a title, free it
		     * Don't have to worry about graphics,
		     * for CCDF, they have not been allocated yet.
		     */
		    if (foundTitle == True)
		        _DtHelpCeFreeSegList(canvas, cur_vars->my_links,
						cur_vars->seg_cnt - segCnt,
						&(cur_vars->seg_list[segCnt]));
		    cur_vars->seg_cnt = segCnt;

		    /*
		     * save the abbreviation
		     */
		    cur_vars->fmt_size     = strlen (cur_vars->fmt_buf);
		    cur_vars->fmt_buf_max  = cur_vars->fmt_size + 1;
		    if (cur_vars->fmt_size > 0)
		        result = SaveSegment (canvas, cur_vars,
					_CESetStaticFlag(_CESetString(0)),
					&fontAttrs, -1);
		    if (result != -1)
			TerminateSegList(cur_vars);
	          }
		/*
		 * eliminate the preceeding spaces if no title or abbrev
		 * was found.
		 */
		else if (foundTitle == False && origCnt < segCnt)
		  {
		    _DtHelpCeFreeSegList(canvas, cur_vars->my_links,
						segCnt - origCnt,
						&(cur_vars->seg_list[origCnt]));
		    cur_vars->seg_cnt = origCnt;
		    segCnt            = origCnt;
		  }
	      }
	  }
	else
	    result = _DtHelpCeSkipCcdfAbbrev (cur_vars->my_file, cur_vars->rd_buf,
				&(cur_vars->rd_ptr), cur_vars->rd_size,
				cur_vars->cur_mb_max);

	if (result != -1 && cur_vars->seg_cnt > segCnt)
	  {
	    segCnt = cur_vars->seg_cnt - 1;
	    cur_vars->seg_list[segCnt].seg_type =
			_CESetNewLineFlag(cur_vars->seg_list[segCnt].seg_type);
          }
	if (cur_vars->fmt_buf != NULL)
	    cur_vars->fmt_buf[0] = '\0';
	cur_vars->fmt_size     = 0;
      }
    else
	result = -1;

    _DtHelpCeBufFileClose (cur_vars->my_file, True);

    /*
     * free memory
     */
    if (NULL != charSet)
      {
	free(fontAttrs.lang);
	free(fontAttrs.char_set);
	free(charSet);
      }

    if (result == -1)
	return -1;

    return 0;
}

/******************************************************************************
 * Function:	int FormatEntryInToc (
 *
 * Parameters:
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static int
#ifdef _NO_PROTO
FormatEntryInToc(canvas_handle, volume, loc_id, cur_vars, font_attrs, level)
    CanvasHandle	 canvas_handle;
    _DtHelpVolume	  volume;
    char		 *loc_id;
    FormatVariables	 *cur_vars;
    CEFontSpec		  font_attrs;
    int			  level;
#else
FormatEntryInToc(
    CanvasHandle	 canvas_handle,
    _DtHelpVolume	  volume,
    char		 *loc_id,
    FormatVariables	 *cur_vars,
    CEFontSpec		  font_attrs,
    int			  level)
#endif /* _NO_PROTO */
{
    int		  result    = -1;
    int		  tocLen    = 0;
    int		  offset;
    int		  num;
    int		  segCnt;
    char	 *strPtr;
    CESegment	 *pSeg;

    if (InitializeParagraph (cur_vars, GROW_SIZE ) == -1)
	return -1;

    cur_vars->para_list[cur_vars->para_cnt].lines_before = 0;
    cur_vars->para_list[cur_vars->para_cnt].lines_after  = 0;
    cur_vars->para_list[cur_vars->para_cnt].left_margin  = 2 * level;
    cur_vars->para_list[cur_vars->para_cnt].first_indent = 2 * level;

    if (_DtHelpCeFindId(volume, loc_id, -1, &strPtr, &offset) == True)
      {
	result = FormatCCDFTitle(canvas_handle, cur_vars,
				    font_attrs, strPtr,
				    offset, /*2 * level*/ 0,
  ((1<<_CEFONT_SIZE)|(1<<_CEFONT_WEIGHT)|(1<<_CEFONT_ANGLE)|(1<<_CEFONT_TYPE)),
				    False);

	free(strPtr);
    
	strPtr = strdup(loc_id);
	num    = _DtHelpCeAddLinkToList(&(cur_vars->my_links), strPtr,
					    CELinkType_SameVolume,
					    CEWindowHint_CurrentWindow, NULL);
	if (num != -1)
	  {
	    pSeg = cur_vars->para_list[cur_vars->para_cnt].seg_list;
	    if (result != -1 && cur_vars->seg_cnt > 0)
	      {
		for(segCnt = 0; segCnt < cur_vars->seg_cnt; segCnt++)
		  {
		    /*
		     * If this segment has a hypertext link, remove it.
		     */
		    if (pSeg[segCnt].link_index != -1)
			_DtHelpCeRmLinkFromList(cur_vars->my_links,
						pSeg[segCnt].link_index);
    
		    if (segCnt + 1 < cur_vars->seg_cnt)
			pSeg[segCnt].seg_type =
					_CEClearNewline(pSeg[segCnt].seg_type);
    
		    pSeg[segCnt].link_index = num;
		    pSeg[segCnt].seg_type = _CESetGhostLink(
				_CEClearHyperFlag(pSeg[segCnt].seg_type));
		  }
	      }
	    else
	      {
		tocLen = strlen (loc_id) + 9;
		if (tocLen > cur_vars->fmt_buf_max)
		  {
		    if (NULL == cur_vars->fmt_buf)
			cur_vars->fmt_buf = (char *) malloc (tocLen);
		    else
			cur_vars->fmt_buf = (char *) realloc(
					    cur_vars->fmt_buf,
					    tocLen);
		    cur_vars->fmt_buf_max = tocLen;
		  }
    
		cur_vars->fmt_size = tocLen - 1;
		strcpy (cur_vars->fmt_buf, "...(");
		strcat (cur_vars->fmt_buf, loc_id);
		strcat (cur_vars->fmt_buf, ")...");
    
		result = SaveSegment(canvas_handle, cur_vars,
			    _CESetStaticFlag(
				    _CESetGhostLink(
					    _CESetNewLineFlag(
						    _CESetString(0)))),
			    &font_attrs, num);
	      }
	  }
      }

    if (cur_vars->seg_list)
      {
	TerminateSegList(cur_vars);
	cur_vars->para_cnt++;
	cur_vars->seg_list = NULL;
      }

   return result;

}  /* End FormatEntryInToc */

/******************************************************************************
 * Function:	int FormatExpandToc (
 *				Canvas_handle canvas,
 *				_DtHelpVolume volume,
 *
 * Purpose:
 ******************************************************************************/
static int
#ifdef _NO_PROTO
FormatExpandedToc(canvas_handle, volume, cur_id, path_lst, cur_vars,
			font_attrs, level)
    CanvasHandle	 canvas_handle;
    _DtHelpVolume	  volume;
    char		  *cur_id;
    char		 **path_lst;
    FormatVariables	 *cur_vars;
    CEFontSpec		  font_attrs;
    int			  level;
#else
FormatExpandedToc(
    CanvasHandle	 canvas_handle,
    _DtHelpVolume	  volume,
    char		  *cur_id,
    char		 **path_lst,
    FormatVariables	 *cur_vars,
    CEFontSpec		  font_attrs,
    int			  level)
#endif /* _NO_PROTO */
{
    int    result = 0;

    if (*path_lst != NULL)
      {
	font_attrs.font_specs.weight = CEFontWeightMedium;
	if (_DtHelpCeStrCaseCmp(*path_lst, cur_id) == 0)
	    font_attrs.font_specs.weight = CEFontWeightBold;

        result = FormatEntryInToc(canvas_handle, volume, *path_lst, cur_vars,
				font_attrs, level);

        if (result != -1)
          {
	    char **children;
	    char **childLst;
	    int    count = _DtHelpCeGetCcdfTopicChildren(volume,
							*path_lst, &children);

	    /*
	     * if there are children, format them
	     */
	    level++;
	    path_lst++;
	    childLst = children;
	    font_attrs.font_specs.weight = CEFontWeightMedium;
	    while (result != -1 && count > 0)
	      {
		/*
		 * found the next item in the list
		 */
		if (*path_lst != NULL &&
				_DtHelpCeStrCaseCmp(*path_lst, *childLst) == 0)
		    result = FormatExpandedToc(canvas_handle, volume,
					cur_id, path_lst,
					cur_vars, font_attrs, level);
		else
		    result = FormatEntryInToc(canvas_handle, volume, *childLst,
					cur_vars, font_attrs, level);

		childLst++;
		count--;
	      }

	    if (children != NULL)
		_DtHelpCeFreeStringArray(children);
          }
      }

    return result;
}

/******************************************************************************
 *
 * Semi-Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	VarHandle *__DtHelpCeSetUpVars (char *rd_buf, grow_size)
 *
 * Parameters:
 *
 * Returns:	NULL if unsuccessful.
 *
 * errno Values:
 *
 * Purpose:	
 *
 ******************************************************************************/
VarHandle
#ifdef _NO_PROTO
__DtHelpCeSetUpVars(lang, code_set)
    char  *lang;
    char  *code_set;
#else
__DtHelpCeSetUpVars(char *lang, char *code_set)
#endif /* _NO_PROTO */
{
    FormatVariables    *newVars;

    newVars = (FormatVariables *) malloc (sizeof(FormatVariables));
    *newVars = DefVars;

    if (InitializeParagraph (newVars, 1) == -1)
	return NULL;

    newVars->topic_mb_max = _DtHelpCeGetMbLen(lang, code_set);
    newVars->cur_mb_max   = newVars->topic_mb_max;

    return ((VarHandle) newVars);

}  /* End __DtHelpCeSetUpVars */

/******************************************************************************
 * Function:	int __DtHelpCeProcessString (char *in_string, int seg_type,
 *						char **font_attr)
 *
 * Parameters:
 *
 * Returns:	0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:	
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpCeProcessString(canvas_handle, var_handle, my_file, scan_string, in_string, in_size, seg_type, font_attr)
    CanvasHandle  canvas_handle;
    VarHandle	  var_handle;
    BufFilePtr	  my_file,
    char	 *scan_string;
    char	 *in_string;
    int		  in_size;
    int		  seg_type;
    CEFontSpec	*font_attr;
#else
__DtHelpCeProcessString(
    CanvasHandle  canvas_handle,
    VarHandle	  var_handle,
    BufFilePtr	  my_file,
    char	 *scan_string,
    char	 *in_string,
    int		  in_size,
    int		  seg_type,
    CEFontSpec	*font_attr )
#endif /* _NO_PROTO */
{
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    FormatVariables   *myVars = (FormatVariables *) var_handle;
    int    done = 0;

    myVars->rd_buf  = in_string;
    myVars->rd_size = in_size;
    myVars->rd_ptr  = in_string;
    myVars->my_file = my_file;

    if (my_file == NULL)
	myVars->rd_flag = 0;
    else
	myVars->rd_flag = strlen(in_string);

    while (!done)
      {
        done = Parse (canvas, CE_FONT_CMD, NULL, myVars, seg_type, font_attr,
			-1, NULL, scan_string, (CE_FONT_CMD | CE_OCTAL_CMD), 0);

	if (done != -1 && _CEEndOnNewLine(seg_type) && myVars->seg_list != NULL)
	  {
	    if (TerminateSegList (myVars) == -1)
		return -1;
	
	    (myVars->para_cnt)++;
	    done = InitializeParagraph (myVars, GROW_SIZE );
	  }
      }

    if (done == -1)
	_DtHelpCeFreeParagraphs(NULL, myVars->my_links, &(myVars->para_list),
					myVars->para_cnt);

    return done;

}  /* End __DtHelpCeProcessString */

/*****************************************************************************
 * Function:    int __DtHelpCeGetParagraphList (
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors.
 *
 * errno Values:
 *
 * Purpose:     _DtHelpFormatGetParagraphList places a terminator on the
 *              last segment in the current paragraph and returns the
 *              values of the static variables for ParaList and ParaCount.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpCeGetParagraphList (var_handle, ret_handle)
    VarHandle	 var_handle;
    TopicHandle	*ret_handle;
#else
__DtHelpCeGetParagraphList (
    VarHandle	 var_handle,
    TopicHandle	*ret_handle)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    FormatVariables	*myVars = (FormatVariables *) var_handle;
    CETopicStruct	*topicHandle;

    /*
     * check the parameters.
     */
    if (ret_handle == NULL || myVars == NULL)
      {
        errno = EINVAL;
        return -1;
      }

    if (myVars->seg_list)
      {
        result = TerminateSegList (myVars);
	if (result != -1)
	    (myVars->para_cnt)++;
      }

    if (result != -1)
      {
        topicHandle = (CETopicStruct *) malloc (sizeof(CETopicStruct));
        if (topicHandle != NULL)
          {
            topicHandle->element_list = NULL;
            topicHandle->para_num     = myVars->para_cnt;
            topicHandle->para_list    = myVars->para_list;
            topicHandle->id_seg       = NULL;
            topicHandle->link_data    = myVars->my_links;

            *ret_handle = (TopicHandle) topicHandle;
          }
        else
          {
            errno = CEErrorMalloc;
	    result = -1;
          }
      }
    else
	_DtHelpCeFreeParagraphs(NULL, myVars->my_links, &(myVars->para_list),
					myVars->para_cnt);

    if (myVars->fmt_buf != NULL)
	free(myVars->fmt_buf);

    return result;
}

/******************************************************************************
 * Function:	int _DtHelpCeFrmtCcdfTopic (_DtHelpVolume volume, char *filename,
 *				int offset, char *id_string,
 *				TopicHandle *ret_handle)
 *
 * Parameters:
 *		volume		Specifies the Help Volume the information
 *				is associated with.
 *		filename	Specifies the file containing the Help Topic
 *				desired.
 *		offset		Specifies the offset into 'filename' to
 *				the Help Topic desired.
 *		id_string	Specifies the location id to look for or NULL.
 *		ret_handle	Returns a handle to the topic information
 *				including the number of paragraphs and the
 *				id match segment.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpCeFrmtCcdfTopic formats Help Files with formatting
 *		information into a list of CEParagraph structures.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtCcdfTopic(canvas_handle, volume, filename, offset, id_string, ret_handle)
	CanvasHandle	 canvas_handle;
	_DtHelpVolume	  volume;
	char		 *filename ;
	int		  offset;
	char		 *id_string;
	TopicHandle	 *ret_handle;
#else
_DtHelpCeFrmtCcdfTopic(
	CanvasHandle	 canvas_handle,
	_DtHelpVolume	  volume,
	char		 *filename,
	int		  offset,
	char		 *id_string,
	TopicHandle	 *ret_handle )
#endif /* _NO_PROTO */
{
    int		 result = -1;
    char	*ptr;
    FormatVariables	variables;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;

    /*
     * Check the parameters
     */
    if (volume == NULL || filename == NULL || offset < 0 || ret_handle == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * get the current file path.
     */
    variables = DefVars;
    if (volume)
      {
	variables.my_path = strdup (volume->volFile);
	if (variables.my_path == NULL)
	  {
	    errno = CEErrorMalloc;
	    return -1;
	  }

	result = _DtHelpCeStrrchr (variables.my_path, Slash, MB_CUR_MAX, &ptr);
	if (result == -1)
	    ptr = strrchr (variables.my_path, '/');

	if (ptr)
	    *ptr = '\0';
      }

    /*
     * open the file and seek to the correct place.
     */
    result = _DtHelpCeFileOpenAndSeek (filename, offset,-1,
						&(variables.my_file), NULL);
    if (result != -1)
      {
	/*
	 * Initialize the X variables.
	 */
	result = Format (canvas, &variables, id_string, ret_handle);

	_DtHelpCeBufFileClose (variables.my_file, True);
      }

    if (variables.fmt_buf != NULL)
	free (variables.fmt_buf);

    if (variables.my_path != NULL)
	free (variables.my_path);

    return result;

}  /* End _DtHelpCeFrmtCcdfTopic */

/******************************************************************************
 * Function:	int _DtHelpCeFrmtCcdfPathAndChildren (
 *				Canvas_handle canvas,
 *				_DtHelpVolume volume,
 *				TopicHandle *ret_handle)
 *
 * Parameters:
 *		ret_handle	Returns a handle to the topic information
 *				including the number of paragraphs and the
 *				id match segment.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpCeFormatCcdfToc formats Table of Contents for
 *		a CCDF Help File.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtCcdfPathAndChildren(canvas_handle, volume, loc_id, ret_handle)
    CanvasHandle	 canvas_handle;
    _DtHelpVolume	  volume;
    char		 *loc_id;
    TopicHandle		 *ret_handle;
#else
_DtHelpCeFrmtCcdfPathAndChildren(
    CanvasHandle	 canvas_handle,
    _DtHelpVolume	  volume,
    char		 *loc_id,
    TopicHandle		 *ret_handle )
#endif /* _NO_PROTO */
{
    int		  result    = 0;
    int		  pathCnt;
    char	 *ptr;
    char	**pathHead;
    char	  readBuf[BUFF_SIZE];
    FormatVariables     myVars;
    CEFontSpec    fontAttrs;

    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    fontAttrs.font_specs.pointsz = 10;
    fontAttrs.font_specs.weight  = CEFontWeightMedium;
    fontAttrs.font_specs.style   = CEFontStyleSanSerif;

    /*
     * Check the parameters
     */
    if (volume == NULL || ret_handle == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    readBuf[0] = '\0';
    myVars     = DefVars;
    if (Initialize (&myVars, NULL, readBuf, BUFF_SIZE, GROW_SIZE) == -1)
	return -1;

    /*
     * get the list of topics in the volume.
     */
    pathCnt = _DtHelpCeGetCcdfIdPath(volume, loc_id, &pathHead);
    if (pathCnt == -1)
	return -1;

    /*
     * Check to see if this topic is in the topic list.
     * I.e. if a count of 1 is returned this should match the
     * top topic, otherwise this is an hidden topic.
     */
    if (pathCnt == 1)
      {
	if (_DtHelpCeGetCcdfTopTopic(volume, &ptr) == -1)
	    result = -1;
	else if (_DtHelpCeStrCaseCmp(ptr, *pathHead) != 0)
	    result = 1;
      }

    /*
     * if result is non-zero, we had problems or this is a hidden topic
     */
    if (result == 0)
      {
        fontAttrs.font_specs.slant   = CEFontSlantRoman;

        myVars.my_path = strdup (volume->volFile);
        if (myVars.my_path == NULL)
          {
	    errno = CEErrorMalloc;
	    return -1;
          }

        result = _DtHelpCeStrrchr (myVars.my_path, Slash, MB_CUR_MAX, &ptr);
        if (result == -1)
	    ptr = strrchr (myVars.my_path, '/');

        if (ptr)
	    *ptr = '\0';

        result = FormatExpandedToc(canvas_handle, volume, loc_id, pathHead,
						&myVars, fontAttrs, 0);
	_DtHelpCeFreeStringArray(pathHead);
      }

    if (result != -1)
      {
	myVars.para_list->lines_after = 0;
	result = __DtHelpCeGetParagraphList (&myVars, ret_handle);
      }

    if (myVars.my_path != NULL)
        free(myVars.my_path);

    return result;

}  /* End _DtHelpCeFrmtCcdfPathAndChildren */

/******************************************************************************
 * Function:	int _DtHelpCeGetCcdfTitleChunks (
 *				Canvas_handle canvas,
 *				_DtHelpVolume volume,
 *				TopicHandle *ret_handle)
 *
 * Parameters:
 *		ret_handle	Returns a handle to the topic information
 *				including the number of paragraphs and the
 *				id match segment.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpCeFormatCcdfToc formats Table of Contents for
 *		a CCDF Help File.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfTitleChunks(canvas_handle, volume, loc_id, ret_chunks)
    CanvasHandle	   canvas_handle;
    _DtHelpVolume	   volume;
    char		  *loc_id;
    void		***ret_chunks;
#else
_DtHelpCeGetCcdfTitleChunks(
    CanvasHandle	   canvas_handle,
    _DtHelpVolume	   volume,
    char		  *loc_id,
    void		***ret_chunks)
#endif /* _NO_PROTO */
{
    int		  type   = 0;
    int		  result = 0;
    int		  segCnt = 0;
    int		  cnt    = 0;
    int		  offset;
    char	 *fileName;
    char	 *ptr;
    char	  readBuf[BUFF_SIZE];
    FormatVariables     myVars;
    CEFontSpec    fontAttrs;
    _DtHelpCeLockInfo lockInfo;

    /*
     * Check the parameters
     */
    if (volume == NULL || ret_chunks == NULL)
      {
	errno = EINVAL;
	return -1;
      }

    /*
     * get the topic in the volume.
     */
    if (_DtHelpCeLockVolume(canvas_handle, volume, &lockInfo) != 0)
	return -1;

    if (_DtHelpCeFindId(volume, loc_id, -1, &fileName, &offset) == False)
      {
	_DtHelpCeUnlockVolume(lockInfo);
	return -1;
      }

    _DtHelpCeCopyDefFontAttrList (&fontAttrs);
    myVars = DefVars;
    if (Initialize (&myVars, NULL, readBuf, BUFF_SIZE, 1) == -1)
      {
	_DtHelpCeUnlockVolume(lockInfo);
	return -1;
      }

    /*
     */
    myVars.my_path = strdup (volume->volFile);
    if (myVars.my_path == NULL)
      {
	_DtHelpCeUnlockVolume(lockInfo);
	errno = CEErrorMalloc;
	return -1;
      }

    result = _DtHelpCeStrrchr (myVars.my_path, Slash, MB_CUR_MAX, &ptr);
    if (result == -1)
	ptr = strrchr (myVars.my_path, '/');

    if (ptr)
	*ptr = '\0';

    result = FormatCCDFTitle(canvas_handle, &myVars, fontAttrs, fileName,
				offset, 0, (1 << _CEFONT_SIZE), True);
    free(fileName);

    if (result != -1 && segCnt < myVars.seg_cnt)
      {
	*ret_chunks = (void **) malloc(sizeof(void *)*(myVars.seg_cnt * 3 + 1));
	if (*ret_chunks == NULL)
	    result = -1;
	else
	  {
	    CESegment *pSeg = myVars.seg_list;
	    for(cnt = 0; segCnt < myVars.seg_cnt; segCnt++, pSeg++)
	      {
	        /*
	         * If this segment has a hypertext link, remove it.
	         */
	        if (pSeg->link_index != -1)
		    _DtHelpCeRmLinkFromList(myVars.my_links, pSeg->link_index);

		type = DT_HELP_CE_FONT_PTR | DT_HELP_CE_STRING;
		if (_CEIsNewLine(pSeg->seg_type))
		    type |= DT_HELP_CE_NEWLINE;

		if (_CEIsString(pSeg->seg_type))
		  {
		    (*ret_chunks)[cnt++] = (void *) type;
		    (*ret_chunks)[cnt++] = (void *) _CEFontOfSegment(pSeg);
		    (*ret_chunks)[cnt++] = (void *)
					strdup(_CEStringOfSegment(pSeg));
		  }
	      }
	  }
      }
    else
      {
	char buf[128];
	int idLen = strlen (loc_id) + 9;
	if (idLen > myVars.fmt_buf_max)
	  {
	    if (NULL == myVars.fmt_buf)
		myVars.fmt_buf = (char *) malloc (sizeof(char) * idLen);
	    else
		myVars.fmt_buf = (char *) realloc(myVars.fmt_buf,
							(sizeof(char) * idLen));
	    myVars.fmt_buf_max = idLen;
	  }

	/*
	 * format the location id
	 */
	myVars.fmt_buf[0] = '\0';
	strcat (myVars.fmt_buf, "...(");
	strcat (myVars.fmt_buf, loc_id);
	strcat (myVars.fmt_buf, ")...");
	
	/*
	 * format the language and codeset
	 */
	strcpy(buf, fontAttrs.lang);
	strcat(buf, fontAttrs.char_set);

	/*
	 * creat a chunk table
	 */
	*ret_chunks = (void **) malloc (sizeof (void *) * 4);
	(*ret_chunks)[0] = (void *) DT_HELP_CE_CHARSET;
	(*ret_chunks)[1] = (void *) strdup(buf);
	(*ret_chunks)[2] = (void *) myVars.fmt_buf;
	cnt = 3;
      }

    if (cnt > 0)
        (*ret_chunks)[cnt++] = (void *) DT_HELP_CE_END;

    if (result != -1)
      {
	if (myVars.seg_list != NULL)
	  {
	    _DtHelpCeFreeSegList(canvas_handle, myVars.my_links, myVars.seg_cnt,
							myVars.seg_list);
	    free(myVars.seg_list);
	  }

	if (myVars.para_list != NULL)
	    free(myVars.para_list);
      }

    if (myVars.my_path != NULL)
        free(myVars.my_path);
    if (myVars.fmt_buf != NULL)
        free(myVars.fmt_buf);

    _DtHelpCeUnlockVolume(lockInfo);
    return result;

}  /* End _DtHelpCeGetCcdfTitleChunks */

/******************************************************************************
 * Function:	int _DtHelpCeGetCcdfVolTitleChunks (
 *				CanvasHandle canvas,
 *				_DtHelpVolume volume,
 *				void ***ret_chunks)
 *
 * Parameters:
 *		ret_chunks	Returns a ptr to the title information chunks
 *				The title chunks are stored in allocated memory
 *				that is owned by the caller and should be
 *                              freed when no longer needed.
 *				id match segment.
 *
 * Returns:	0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:	_DtHelpCeFormatCcdfVolTitleChunks formats volume title for
 *		a CCDF Help File.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetCcdfVolTitleChunks(canvas_handle, volume, ret_chunks)
    CanvasHandle	   canvas_handle;
    _DtHelpVolume	   volume;
    void		***ret_chunks;
#else
_DtHelpCeGetCcdfVolTitleChunks(
    CanvasHandle	   canvas_handle,
    _DtHelpVolume	   volume,
    void		***ret_chunks)
#endif /* _NO_PROTO */
{
    char    *charSet;
    char    *titleStr;

    /*
     * get the title of the volume (we own the memory).
     */
    charSet = _DtHelpCeGetCcdfVolLocale(volume);
    if (charSet == NULL)
	charSet = strdup("C.ISO-8859-1");

    titleStr = _DtHelpCeGetCcdfVolTitle(volume);
    if (titleStr != NULL)
      {
        *ret_chunks = (void **) malloc(sizeof(void *) * 4);
        if (*ret_chunks == NULL)
	    return -1;

        (*ret_chunks)[0] = (void *) DT_HELP_CE_CHARSET;
        (*ret_chunks)[1] = (void *) charSet;
        (*ret_chunks)[2] = (void *) strdup(titleStr);
        (*ret_chunks)[3] = (void *) DT_HELP_CE_END;
      }
    else 
      {
        free(charSet);
        if (_DtHelpCeGetCcdfTitleChunks(canvas_handle, volume,
						"_title", ret_chunks) != 0
	   && _DtHelpCeGetCcdfTitleChunks(canvas_handle, volume,
					"_hometopic", ret_chunks) != 0)
 	return -1;
      }

    return 0;

}  /* End _DtHelpCeGetCcdfVolTitleChunks */
