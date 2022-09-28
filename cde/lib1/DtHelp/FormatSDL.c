/* $XConsortium: FormatSDL.c /main/cde1_maint/3 1995/10/08 17:18:33 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	   FormatSDL.c
 **
 **   Project:     CDE Help System
 **
 **   Description: This code formats information in an SDL volume into
 **		   an into internal format.
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
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


extern	int errno;

/*
 * private includes
 */
#include "Canvas.h"
#include "CanvasError.h"
#include "Access.h"
#include "bufioI.h"
#include "AccessI.h"
#include "AccessSDLP.h"
#include "AccessSDLI.h"
#include "CESegmentI.h"
#include "CanvasI.h"
#include "CEUtilI.h"
#include "CleanUpI.h"
#include "FormatSDLI.h"
#include "LinkMgrI.h"
#include "UtilSDLI.h"
#include "SnrefI.h"
#include "StringFuncsI.h"

#ifdef NLS16
#endif

/******************************************************************************
 * Private structures
 ******************************************************************************/
typedef struct _ceFormatStruct {
        CEBoolean        end_flag;
        CEBoolean        last_was_space;
        CEBoolean        last_was_mb;
        CEBoolean        last_was_nl;
        CEBoolean        snref_used;
        CEBoolean        faked_end;
        enum SdlElement  parsed;
        char            *remember;
        int              cur_link;
        int              mb_len;
        int              flags;
	int		 malloc_size;
	int		 alloc_size;
	CESegment	*block_list;
        CESegment       *add_seg;
        CESegment       *last_seg;
        CESegment       *seg_list;
        CESegment       *prev_data;
        CESegment       *toss;
        CESegment       *async_blks;
        CESegment       *snb;
        CESegment       *else_prev;
        CESegment       *then_prev;
        CELinkData       my_links;
        BufFilePtr       my_file;
        CESDLAttrStruct *cur_attr;
} _CEFormatStruct;

static _CEFormatStruct defaultFormatStruct =
    {
	False,				/* end_flag    */
	True,				/* last_was_space */
	False,				/* last_was_mb */
	False,				/* last_was_nl */
	False,				/* snref_used  */
	False,				/* faked_end   */
	SdlElementNone,			/* parsed      */
	NULL,				/* remember    */
	-1,				/* cur_link    */
	 1,				/* mb_len      */
	 0,				/* flags       */
	 1,				/* malloc_size */
	 0,				/* alloc_size  */
	NULL,				/* block_list  */
	NULL,				/* add_seg     */
	NULL,				/* last_seg    */
	NULL,				/* seg_list    */
	NULL,				/* prev_data   */
	NULL,				/* toss        */
	NULL,				/* snb         */
	NULL,				/* else_prev   */
	NULL,				/* then_prev   */
	NULL,				/* async_blks  */
	  {				/* my_links    */
	    0,				    /* max   */
	    NULL,			    /* list  */
	  },
    };

/******************************************************************************
 * Private Function Declarations
 ******************************************************************************/
#ifdef _NO_PROTO
static	int	AllocateBlock();
static	int	AllocateSpace();
static	int	Cdata();
static	int	CheckForSnb();
static	int	CheckType();
static	int	CleanUpBlock();
static	int	IfInfo();
static	int	FakeEnd();
static	int	InitLast();
static	int	MaintainLink();
static	int	MarkFound();
static	int	OnlyOne();
static	int	OnlyOneEach();
static	int	OneToN();
static	int	ParseSDL();
static	int	ProcessEnterAttr();
static	int	ProcessExitAttr();
static	int	RegisterLink();
static	int	ResolveIf();
static	int	SetType();
static	int	ZeroOrOne();
static	int	ZeroToN();
#else
static	int	AllocateBlock(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	AllocateSpace(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	Cdata(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	CheckForSnb(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	CheckType(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	CleanUpBlock(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	IfInfo(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	FakeEnd(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	InitLast(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	MaintainLink(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	MarkFound(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	OnlyOne(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	OnlyOneEach(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	OneToN(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	ParseSDL(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			enum SdlElement	 cur_element,
			enum SdlElement	 sig_element,
			SDLMask		*cur_except,
			SDLMask		*process_mask);
static	int	ProcessEnterAttr(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*cur_except,
			SDLMask		*process_mask);
static	int	ProcessExitAttr(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*cur_except,
			SDLMask		*process_mask);
static	int	RegisterLink(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	ResolveIf(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			CESegment	*prev_data);
static	int	SetType(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	ZeroOrOne(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
static	int	ZeroToN(
			CECanvasStruct	*canvas,
			_CEFormatStruct	*my_struct,
			SDLMask		*element_types,
			enum SdlElement	 sig_element,
			SDLMask		*exceptions,
			SDLMask		*process_mask);
#endif /* _NO_PROTO */

/********    End Public Function Declarations    ********/

/******************************************************************************
 * Private Defines
 *****************************************************************************/
#define	GROW_SIZE		5
#define	MAX_ATTRIBUTE_LENGTH	30

/******************************************************************************
 * Private Macros
 *****************************************************************************/
#define	IdString(x)	((x)->attributes.id_vals.id)
#define	RIdString(x)	((x)->attributes.id_vals.rid)
#define	SSIString(x)	((x)->attributes.ssi_vals.ssi)
#define	RSSIString(x)	((x)->attributes.ssi_vals.rssi)
#define	LevelAttr(x)	((x)->attributes.lev.level)

#define	MySaveString(seg_list,my_struct,string,cur_link,multi_len,nl_flag) \
	_DtHelpCeSaveString(seg_list,				\
				&(my_struct->last_seg),		\
				&(my_struct->prev_data),	\
				string,				\
				&(my_struct->cur_attr->font_specs), \
				cur_link,			\
				multi_len,			\
				my_struct->flags, nl_flag)

/******************************************************************************
 * Private Strings
 *****************************************************************************/
static const	char	AllStr[]          = "all";
static const	char	AnchorStr[]       = "<anchor";
static const	char	AnimateStr[]      = "<animate";
static const	char	ASyncStr[]        = "async";
static const	char	AudioStr[]        = "<audio";
static const	char	BlockStr[]        = "<block";
static const	char	ButtonStr[]       = "button";
static const	char	CallbackStr[]     = "<callback";
static const	char	CenterJustifyStr[]= "center-justify";
static const	char	CenterOrientStr[] = "center-orient";
static const	char	CParaStr[]        = "<cp";
static const	char	CrossdocStr[]     = "<crossdoc";
static const	char	DynamicStr[]      = "dynamic";
static const	char	GraphicStr[]      = "<graphic";
static const	char	FormStr[]         = "<form";
static const	char	HeadStr[]         = "<head";
static const	char	IsoStr[]          = "ISO-8859-1";
static const	char	LeftJustifyStr[]  = "left-justify";
static const	char	ManpageStr[]      = "<man-page";
static const	char	NameStr[]         = "name";
static const	char	NegativeOneStr[]  = "-1";
static const	char	NoBorderStr[]     = "no-border";
static const	char	OneStr[]          = "1";
static const	char	ProductStr[]      = "product";
static const	char	ParaStr[]         = "<p";
static const	char	ParentStr[]       = "parent";
static const	char	RightJustifyStr[] = "right-justify";
static const	char	ScriptStr[]       = "<script";
static const	char	SnrefStr[]        = "<snref";
static const	char	SubHeadStr[]      = "<subhead";
static const	char	SwitchStr[]       = "<switch";
static const	char	SyscmdStr[]       = "<sys-cmd";
static const	char	TenStr[]          = "10";
static const	char	TenThousandStr[]  = "10000";
static const	char	TextStr[]         = "<text";
static const	char	TextfileStr[]     = "<textfile";
static const	char	TopVJustStr[]     = "top-vjust";
static const	char	TopVOrientStr[]   = "top-vorient";
static const	char	UdefKeyStr[]      = "udefkey";
static const	char	VideoStr[]        = "<video";
static const	char	VirpageStr[]      = "<virpage";
static const	char	NoWrapStr[]       = "nowrap";
static const	char	ZeroStr[]         = "0";

static const	SDLMask	AllMaskSet[SDL_MASK_LEN] = CESetAllBits;

/******************************************************************************
 * Entity Defines
 *****************************************************************************/
/*-----------------------------------------------------------------------------
<!ENTITY % system-notations "(graphic  | text     | audio    | video    |
			      animate  | crossdoc | man-page | textfile |
			      sys-cmd  | callback | script   | switch)" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_SYSTEM_NOTATIONS \
		CEInitializeMaskTwelve(SdlElementGraphic,   \
					SdlElementText,     \
					SdlElementAudio,    \
					SdlElementVideo,    \
					SdlElementAnimate,  \
					SdlElementCrossDoc, \
					SdlElementManPage,  \
					SdlElementTextFile, \
					SdlElementSysCmd,   \
					SdlElementCallback, \
					SdlElementScript,   \
					SdlElementSwitch)

/*-----------------------------------------------------------------------------
<!ENTITY % generated-elements "(loids, toss?, lophrases?, index?, rel-docs?,
				rel-file?, notes?)" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_GENERATED_ELEMENTS \
	{ CEInitializeMask(SdlElementLoids)    , OnlyOne   }, \
	{ CEInitializeMask(SdlElementToss)     , ZeroOrOne }, \
	{ CEInitializeMask(SdlElementLoPhrases), ZeroOrOne }, \
	{ CEInitializeMask(SdlElementIndex)    , ZeroOrOne }, \
	{ CEInitializeMask(SdlElementRelDocs)  , ZeroOrOne }, \
	{ CEInitializeMask(SdlElementRelFile)  , ZeroOrOne }, \
	{ CEInitializeMask(SdlElementNotes)    , ZeroOrOne },

/*-----------------------------------------------------------------------------
<!ENTITY % key-class    "acro       | book      | emph     | jargon     |
			 l10n       | name      | quote    | product    |
			 termdef    | term      | mach-in  | mach-out   |
			 mach-cont  | pub-lit   | udefkey" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_CLASSK \
	{ "acro"     , SdlClassAcro     }, \
	{ "book"     , SdlClassBook     }, \
	{ "emph"     , SdlClassEmph     }, \
	{ "jargon"   , SdlClassJargon   }, \
	{ "l10n"     , SdlClassL10n     }, \
	{ NameStr    , SdlClassName     }, \
	{ "quote"    , SdlClassQuote    }, \
	{ ProductStr , SdlClassProduct  }, \
	{ "termdef"  , SdlClassTermdef  }, \
	{ "term"     , SdlClassTerm     }, \
	{ "mach-in"  , SdlClassMachIn   }, \
	{ "mach-out" , SdlClassMachOut  }, \
	{ "mach-cont", SdlClassMachCont }, \
	{ "pub-lit"  , SdlClassPubLit   }, \
	{ UdefKeyStr , SdlClassUdefkey  }  
/*-----------------------------------------------------------------------------
<!ENTITY % head-class    "label     | head      | caption  | annotation |
			  phead     | udefhead" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_CLASSH \
	{ "label"     , SdlClassLabel      }, \
	{ (HeadStr+1) , SdlClassHead       }, \
	{ "caption"   , SdlClassCaption    }, \
	{ "annotation", SdlClassAnnotation }, \
	{ "phead"     , SdlClassPHead      }, \
	{ "udefhead"  , SdlClassUdefhead   }

/*-----------------------------------------------------------------------------
<!ENTITY % format-class  "table     | cell      | list     | item       |
			  text      | udeffrmt" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_CLASSF \
	{ "table"    , SdlClassTable    }, \
	{ "cell"     , SdlClassCell     }, \
	{ "list"     , SdlClassList     }, \
	{ "item"     , SdlClassItem     }, \
	{ (TextStr+1), SdlClassText     }, \
	{ "udeffrmt" , SdlClassUdeffrmt }

/*-----------------------------------------------------------------------------
<!ENTITY % graphic-class "figure    | in-line   | button   | icon       |
			  udefgrph" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_CLASSG \
	{ "figure"    , SdlClassFigure    }, \
	{ "in-line"   , SdlClassInLine    }, \
	{ ButtonStr   , SdlClassButton    }, \
	{ "icon"      , SdlClassIcon      }, \
	{ "udefgraph" , SdlClassUdefgraph }

/*-----------------------------------------------------------------------------
<!ENTITY % phrase-class  "( super   | sub )" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_CLASSP \
	{ "super"   , SdlClassSuper }, \
	{ "sub"     , SdlClassSub   }

/*-----------------------------------------------------------------------------
<!ENTITY % font-styles
	   '-- height of font in points; main body is 10 point --
	    pointsz   NUMBER             #IMPLIED

	    -- width of font in points; defaults to point size --
	    -- similar to point size but refers to relative    --
	    -- width rather than height (e.g., an 8 point font --
	    -- based on a square would be 8 pt, 8 set)         --
	    set-width NUMBER             #IMPLIED

	    -- one of 42 CDE colors or "RGB:rrrr/gggg/bbbb"    --
	    color     CDATA              #IMPLIED

	    -- XLFD typeface name for use on X Window System   --
	    -- e.g., "-adobe-helvetica-bold-i-narrow-*-"       --
	    -- One each of normal, italic, bold and bold       --
	    -- italic must be specified.                       --
	    -- These should only be specified if the author is --
	    -- sure of exactly what font is desired.  In the   --
	    -- usual case, only the logical typeface spec.     --
	    -- defined below will be given.                    --
	    xlfd      CDATA              #IMPLIED
	    xlfdi     CDATA              #IMPLIED
	    xlfdb     CDATA              #IMPLIED
	    xlfdib    CDATA              #IMPLIED

	    -- MS-Windows typeface name (maximum of 32 chars)  --
	    -- One each of normal, italic, bold and bold       --
	    -- italic must be specified.                       --
	    -- As for the XLFD specification above, these      --
	    -- should only be provided if an author knows the  --
	    -- exact font desired.                             --
	    typenam   CDATA              #IMPLIED
	    typenami  CDATA              #IMPLIED
	    typenamb  CDATA              #IMPLIED
	    typenamib CDATA              #IMPLIED

	    -- Logical typeface spec (allows mapping into one  --
	    -- of the 13 PostScript typefaces).  This spec. is --
	    -- used if an exact match XLFD or Windows type-    --
	    -- face name attribute is missing or cannot be     --
	    -- found.  In the usual case, these will be        --
	    -- specified in preference to specifying an exact  --
	    -- font.                                           --

	    -- processor should default style to "sans-serif"  --
	    style     ( serif      |
			sans-serif |
			symbol     )     #IMPLIED

	    -- processor should deflt spacing to "monospace"   --
	    spacing   ( monospace  |
			propspace  )     #IMPLIED

	    -- processor should default weight to "medium"     --
	    weight    ( medium     |
			bold       )     #IMPLIED

	    -- processor should default slant to "roman"       --
	    slant     ( roman      |
			italic     |
			rev-italic )     #IMPLIED

	    -- processor should default special to "none"      --
	    special   ( underline  |
			strikeout  |
			none       )     #IMPLIED'             >

-----------------------------------------------------------------------------*/
#define	font_stylesAttrList \
	{ SDL_ATTR_POINTSZ  , SdlAttrValueImplied , TenStr       }, \
	{ SDL_ATTR_SETWIDTH , SdlAttrValueImplied , TenStr       }, \
	{ SDL_ATTR_COLOR    , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_XLFD     , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_XLFDI    , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_XLFDB    , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_XLFDIB   , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_TYPENAM  , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_TYPENAMI , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_TYPENAMB , SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_TYPENAMIB, SdlAttrValueImplied , NULL         }, \
	{ SDL_ATTR_STYLE    , SdlAttrValueImplied , "sans-serif" }, \
	{ SDL_ATTR_SPACING  , SdlAttrValueImplied , "monospace"  }, \
	{ SDL_ATTR_WEIGHT   , SdlAttrValueImplied , "medium"     }, \
	{ SDL_ATTR_SLANT    , SdlAttrValueImplied , "roman"      }, \
	{ SDL_ATTR_SPECIAL  , SdlAttrValueImplied , "none"       }

/*-----------------------------------------------------------------------------
<!ENTITY % format-styles
	   '-- left and right margins are additive and measured    --
	    -- in character widths defaulting to "0"               --
	    l-margin  NUMBER             #IMPLIED
	    r-margin  NUMBER             #IMPLIED

	    -- top and bottom margins merely take the maximum and  --
	    -- are measured in lines defaulting to "0"             --
	    t-margin  NUMBER             #IMPLIED
	    b-margin  NUMBER             #IMPLIED

	    -- border specifies the decoration type                --
	    -- processor should default border to "no-border"      --
	    border   ( no-border         |
		       full-border       |
		       horizontal-border |
		       vertical-border   |
		       top-border        |
		       bottom-border     |
		       left-border       |
		       right-border      ) #IMPLIED

	    -- applies to the text in the element, not the element --
	    -- itself.                                             --
	    -- processor should default vjust to "top-vjust"       --
	    vjust    ( top-vjust    |
		       bottom-vjust |
		       center-vjust )    #IMPLIED' >
-----------------------------------------------------------------------------*/
#define formt_stylesAttrList \
	{ SDL_ATTR_LMARGIN  , SdlAttrValueImplied , ZeroStr        }, \
	{ SDL_ATTR_RMARGIN  , SdlAttrValueImplied , ZeroStr        }, \
	{ SDL_ATTR_TMARGIN  , SdlAttrValueImplied , ZeroStr        }, \
	{ SDL_ATTR_BMARGIN  , SdlAttrValueImplied , ZeroStr        }, \
	{ SDL_ATTR_BORDER   , SdlAttrValueImplied , NoBorderStr    }, \
	{ SDL_ATTR_VJUST    , SdlAttrValueImplied , TopVJustStr    }

/*-----------------------------------------------------------------------------
<!ENTITY % linkage   "anchor | link" >
<!ENTITY % reference "snref"         >
<!ENTITY % simple    "key | sphrase | rev | if | spc | %reference;" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_SIMPLE   \
	CEInitializeMaskSeven( \
			SdlElementKey   , SdlElementSphrase, SdlElementRev  , \
			SdlElementIf    , SdlElementSpc    , SdlElementSnRef, \
			SdlElementCdata)

/*-----------------------------------------------------------------------------
<!ENTITY % atomic    "( %simple; | %linkage; )" >
-----------------------------------------------------------------------------*/
#define	SDL_ENTITY_ATOMIC \
	CEInitializeMaskNine( \
			SdlElementKey   , SdlElementSphrase, SdlElementRev   , \
			SdlElementIf    , SdlElementSpc    , SdlElementSnRef , \
			SdlElementAnchor, SdlElementLink   , SdlElementCdata)
/*-----------------------------------------------------------------------------
<!NOTATION tcl SYSTEM "embedded tcl interpreter">
<!NOTATION sh  SYSTEM "/bin/sh">
<!NOTATION csh SYSTEM "/bin/csh">
<!NOTATION ksh SYSTEM "/bin/ksh">
-----------------------------------------------------------------------------*/
typedef	struct	_ceInterpData {
	enum SdlOption	 type;
	char		*cmd;
} _CEInterpData;

static _CEInterpData	InterpData[] =
    {
	{ SdlInterpKsh,	"ksh" },
	{ SdlInterpCsh,	"csh" },
	{ SdlInterpSh ,	"sh"  },
	{ SdlInterpTcl,	"tcl" },
	{ SdlOptionBad, NULL  },
    };

/******************************************************************************
 * Private Variables
 *****************************************************************************/
static OptionList hclass[]  = { SDL_ENTITY_CLASSH, {NULL, SdlOptionBad}};
static OptionList fclass[]  = { SDL_ENTITY_CLASSF, {NULL, SdlOptionBad}};
static OptionList kclass[]  = { SDL_ENTITY_CLASSK, {NULL, SdlOptionBad}};
static OptionList pclass[]  = { SDL_ENTITY_CLASSP, {NULL, SdlOptionBad}};
static OptionList gclass[]  = { SDL_ENTITY_CLASSG, {NULL, SdlOptionBad}};
static OptionList fgclass[] =
	{ SDL_ENTITY_CLASSF, SDL_ENTITY_CLASSG, {NULL, SdlOptionBad}};

static OptionList apps[] =
    {
	{ AllStr    , SdlAppAll      },
	{ "help"    , SdlAppHelp     },
	{ "tutorial", SdlAppTutorial },
	{ "ref"     , SdlAppRef      },
	{ "sys"     , SdlAppSys      },
	{  NULL     , SdlOptionBad   }
    };
static OptionList timing[] =
    {
	{ (ASyncStr+1), SdlTimingSync  },
	{ ASyncStr    , SdlTimingAsync },
	{  NULL       , SdlOptionBad   },
    };

static OptionList frmtTypes[] =
    {
	{ "literal" , SdlTypeLiteral },
	{ "lined"   , SdlTypeLined   },
	{ DynamicStr, SdlTypeDynamic },
	{  NULL     , SdlOptionBad   }
    };

static OptionList idTypes[] =
    {
	{ (VirpageStr+1) , SdlIdVirpage   },
	{ (BlockStr+1)   , SdlIdBlock     },
	{ (FormStr+1)    , SdlIdForm      },
	{ (ParaStr+1)    , SdlIdPara      },
	{ (CParaStr+1)   , SdlIdCPara     },
	{ (HeadStr+1)    , SdlIdHead      },
	{ (SubHeadStr+1) , SdlIdSubHead   },
	{ (AnchorStr+1)  , SdlIdAnchor    },
	{ (SwitchStr+1)  , SdlIdSwitch    },
	{ (SnrefStr+1)   , SdlIdSnRef     },
	{ (GraphicStr+1) , SdlIdGraphic   },
	{ (TextStr+1)    , SdlIdText      },
	{ (AudioStr+1)   , SdlIdAudio     },
	{ (VideoStr+1)   , SdlIdVideo     },
	{ (AnimateStr+1) , SdlIdAnimate   },
	{ (CrossdocStr+1), SdlIdCrossDoc  },
	{ (ManpageStr+1) , SdlIdManPage   },
	{ (TextfileStr+1), SdlIdTextFile  },
	{ (SyscmdStr+1)  , SdlIdSysCmd    },
	{ (ScriptStr+1)  , SdlIdScript    },
	{ (CallbackStr+1), SdlIdCallback  },
	{ NULL           , SdlOptionBad   },
    };

static OptionList bool_types[] =
    {
	{ "YES", SdlBoolYes   },
	{ "NO" , SdlBoolNo    },
	{ NULL , SdlOptionBad },
    };

static OptionList borders[] =
    {
	{ NoBorderStr        , SdlBorderNone   },
	{ "full-border"      , SdlBorderFull   },
	{ "horizontal-border", SdlBorderHorz   },
	{ "vertical-border"  , SdlBorderVert   },
	{ "top-border"       , SdlBorderTop    },
	{ "bottom-border"    , SdlBorderBottom },
	{ "left-border"      , SdlBorderLeft   },
	{ "right-border"     , SdlBorderRight  },
	{ NULL               , SdlOptionBad    },
    };

static OptionList justify1[] =
    {
	{ LeftJustifyStr   , SdlJustifyLeft   },
	{ RightJustifyStr  , SdlJustifyRight  },
	{ CenterJustifyStr , SdlJustifyCenter },
	{ "numeric-justify", SdlJustifyNum    },
	{ NULL             , SdlOptionBad    },
    };

static OptionList justify2[] =
    {
	{ LeftJustifyStr   , SdlJustifyLeft   },
	{ RightJustifyStr  , SdlJustifyRight  },
	{ CenterJustifyStr , SdlJustifyCenter },
	{ NULL             , SdlOptionBad    },
    };

static OptionList vjust[] =
    {
	{ TopVJustStr   , SdlJustifyTop    },
	{ "bottom-vjust", SdlJustifyBottom },
	{ "center-vjust", SdlJustifyCenter },
	{ NULL          , SdlOptionBad     },
    };

static OptionList orient[] =
    {
	{ "left-orient"        , SdlJustifyLeft       },
	{ CenterOrientStr      , SdlJustifyCenter     },
	{ "right-orient"       , SdlJustifyRight      },
	{ "left-margin-orient" , SdlJustifyLeftMargin },
	{ "right-margin-orient", SdlJustifyRightMargin},
	{ "left-corner-orient" , SdlJustifyLeftCorner },
	{ "right-corner-orient", SdlJustifyRightCorner},
	{ NULL                 , SdlOptionBad         },
    };

static OptionList vorient[] =
    {
	{ TopVOrientStr   , SdlJustifyTop    },
	{ "bottom-vorient", SdlJustifyBottom },
	{ "center-vorient", SdlJustifyCenter },
	{ NULL            , SdlOptionBad     },
    };

static OptionList placement[] =
    {
	{ "object"     , SdlPlaceObject },
	{ ParentStr    , SdlPlaceParent },
	{  NULL        , SdlOptionBad   },
    };

static OptionList stacking[] =
    {
	{ "horiz"  , SdlStackHoriz },
	{ "vert"   , SdlStackVert  },
	{  NULL    , SdlOptionBad  },
    };

static OptionList flow[] =
    {
	{ (NoWrapStr+2), SdlWrap      },
	{  NoWrapStr   , SdlNoWrap    },
	{  "join"      , SdlWrapJoin  },
	{  NULL        , SdlOptionBad },
    };

static OptionList window[] =
    {
	{ "current", SdlWindowCurrent },
	{ "new"    , SdlWindowNew     },
	{ "popup"  , SdlWindowPopup   },
	{ NULL    , SdlOptionBad },
    };

static OptionList traversal[] =
    {
	{ "return"  , SdlTraversalReturn   },
	{ "noreturn", SdlTraversalNoReturn },
	{ NULL      , SdlOptionBad         },
    };

static OptionList interpreters[] =
    {
	{ "ksh"     , SdlInterpKsh },
	{ "tcl"     , SdlInterpTcl },
	{ "csh"     , SdlInterpCsh },
	{ "sh"      , SdlInterpSh  },
	{ NULL      , SdlOptionBad },
    };

static	SDLAttributeOptions SDLOptionsList[] =
    {
	{SDL_ATTR_CLASSH   , hclass     },
	{SDL_ATTR_CLASSF   , fclass     },
	{SDL_ATTR_CLASSK   , kclass     },
	{SDL_ATTR_CLASSFG  , fgclass    },
	{SDL_ATTR_CLASSP   , pclass     },
	{SDL_ATTR_CLASSG   , gclass     },
	{SDL_ATTR_HDR      , bool_types },
	{SDL_ATTR_APP      , apps       },
	{SDL_ATTR_TIMING   , timing     },
	{SDL_ATTR_TYPEFRMT , frmtTypes  },
	{SDL_ATTR_BORDER   , borders    },
	{SDL_ATTR_JUSTIFY1 , justify1   },
	{SDL_ATTR_JUSTIFY2 , justify2   },
	{SDL_ATTR_VJUST    , vjust      },
	{SDL_ATTR_ORIENT   , orient     },
	{SDL_ATTR_VORIENT  , vorient    },
	{SDL_ATTR_PLACEMENT, placement  },
	{SDL_ATTR_STACK    , stacking   },
	{SDL_ATTR_FLOW     , flow       },
	{SDL_ATTR_WINDOW   , window     },
	{SDL_ATTR_TRAVERSAL, traversal  },
	{SDL_ATTR_TYPEID   , idTypes    },
	{SDL_ATTR_INTERP   , interpreters },
	{ULONG_MAX         , NULL       }
    };

static FontSpecOption styles[] =
    {
	{ "serif"     , CEFontStyleSerif    },
	{ "sans-serif", CEFontStyleSanSerif },
	{ "symbol"    , CEFontStyleSymbol   },
	{ NULL        , CEFontSpecBad   },
    };

static FontSpecOption spacing[] =
    {
	{ "monospace" , CEFontSpacingMono   },
	{ "propspace" , CEFontSpacingProp   },
	{ NULL        , CEFontSpecBad   },
    };

static FontSpecOption weights[] =
    {
	{ "medium"    , CEFontWeightMedium },
	{ "bold"      , CEFontWeightBold   },
	{ NULL        , CEFontSpecBad  },
    };

static FontSpecOption slants[] =
    {
	{ "roman"     , CEFontSlantRoman    },
	{ "italic"    , CEFontSlantItalic   },
	{ "rev-italic", CEFontSlantRevItalic},
	{ NULL        , CEFontSpecBad   },
    };

static FontSpecOption special[] =
    {
	{ "none"      , CEFontSpecialNone      },
	{ "underline" , CEFontSpecialUnderLine },
	{ "strikeout" , CEFontSpecialStrikeOut },
	{ NULL        , CEFontSpecBad      },
    };

static	SDLFontSpecList SDLFontList[] =
    {
	{SDL_ATTR_STYLE    , styles     },
	{SDL_ATTR_SPACING  , spacing    },
	{SDL_ATTR_WEIGHT   , weights    },
	{SDL_ATTR_SLANT    , slants     },
	{SDL_ATTR_SPECIAL  , special    },
	{ULONG_MAX         , NULL       }
    };

/*
 * Make sure the attribute names are in lower case.
 * The read routine to find an attribute automatically changes it to lower
 * case.
 */
static	SDLAttribute  SDLAttributeList[] =
  {
    { "abbrev"   , SdlAttrDataTypeCdata , SDL_ATTR_ABBREV   ,
			CEOffset(CESDLAttrPtr,strings.el_strs.abbrev)       },
    { "app"      , SdlAttrDataTypeEnum  , SDL_ATTR_APP    ,
					CEOffset(CESDLAttrPtr,app)          },
    { "author"   , SdlAttrDataTypeCdata , SDL_ATTR_AUTHOR   ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.author)      },
    { "b-margin" , SdlAttrDataTypeNumber, SDL_ATTR_BMARGIN,
				CEOffset(CESDLAttrPtr,frmt_specs.bmargin)    },
    { "border"   , SdlAttrDataTypeEnum  , SDL_ATTR_BORDER,
				CEOffset(CESDLAttrPtr,frmt_specs.border)     },
    { "branches" , SdlAttrDataTypeCdata , SDL_ATTR_BRANCHES ,
			CEOffset(CESDLAttrPtr,strings.el_strs.branches)      },
    { ButtonStr  , SdlAttrDataTypeId    , SDL_ATTR_BUTTON,
					CEOffset(CESDLAttrPtr,button)        },
    { "cells"    , SdlAttrDataTypeId    , SDL_ATTR_CELLS,
					CEOffset(CESDLAttrPtr,id_vals.cells) },
    { "charset"  , SdlAttrDataTypeCdata , SDL_ATTR_CHARSET,
					CEOffset(CESDLAttrPtr,charset)       },
    { "class"    , SdlAttrDataTypeEnum  , SDL_ATTR_CLASS    ,
					CEOffset(CESDLAttrPtr,clan)         },
    { "colj"     , SdlAttrDataTypeCdata , SDL_ATTR_COLJ   ,
				CEOffset(CESDLAttrPtr,strings.el_strs.colj)  },
    { "color"    , SdlAttrDataTypeCdata , SDL_ATTR_COLOR  ,
				CEOffset(CESDLAttrPtr,font_specs.color)      },
    { "colw"     , SdlAttrDataTypeCdata , SDL_ATTR_COLW   ,
				CEOffset(CESDLAttrPtr,strings.el_strs.colw)  },
    { "command"  , SdlAttrDataTypeCdata , SDL_ATTR_COMMAND ,
			CEOffset(CESDLAttrPtr,strings.not_strs.cmd)          },
    { "count"    , SdlAttrDataTypeNumber, SDL_ATTR_COUNT   ,
				CEOffset(CESDLAttrPtr,num_vals.count)        },
    { "data"     , SdlAttrDataTypeCdata , SDL_ATTR_DATA   ,
			CEOffset(CESDLAttrPtr,strings.not_strs.data)         },
    { "descript" , SdlAttrDataTypeCdata , SDL_ATTR_DESCRIPT,
					CEOffset(CESDLAttrPtr,descript)      },
    { "doc-id"   , SdlAttrDataTypeCdata , SDL_ATTR_DOCID  ,
					CEOffset(CESDLAttrPtr,doc_id)        },
    { "enter"    , SdlAttrDataTypeCdata , SDL_ATTR_ENTER  ,
				CEOffset(CESDLAttrPtr,strings.el_strs.enter) },
    { "exit"     , SdlAttrDataTypeCdata , SDL_ATTR_EXIT   ,
				CEOffset(CESDLAttrPtr,strings.el_strs.exit)  },
    { "first-page",SdlAttrDataTypeId    , SDL_ATTR_FRST_PG,
				CEOffset(CESDLAttrPtr,id_vals.frst_pg)       },
    { "flow"     , SdlAttrDataTypeEnum  , SDL_ATTR_FLOW   ,
					CEOffset(CESDLAttrPtr,flow)          },
    { "f-margin" , SdlAttrDataTypeNumber, SDL_ATTR_FMARGIN,
				CEOffset(CESDLAttrPtr,frmt_specs.fmargin)    },
    { "format"   , SdlAttrDataTypeCdata , SDL_ATTR_FORMAT ,
			CEOffset(CESDLAttrPtr,strings.not_strs.format)       },
    { "hdr"      , SdlAttrDataTypeEnum  , SDL_ATTR_HDR    ,
					CEOffset(CESDLAttrPtr,hdr)           },
    { "headw"    , SdlAttrDataTypeCdata , SDL_ATTR_HEADWDTH,
					CEOffset(CESDLAttrPtr,headw)         },
    { "id"       , SdlAttrDataTypeId    , SDL_ATTR_ID     ,
					CEOffset(CESDLAttrPtr,id_vals.id)    },
    { "interp"   , SdlAttrDataTypeEnum  , SDL_ATTR_INTERP ,
					CEOffset(CESDLAttrPtr,interp)        },
    { "justify"  , SdlAttrDataTypeEnum  , SDL_ATTR_JUSTIFY,
				CEOffset(CESDLAttrPtr,frmt_specs.justify)    },
    { "l-margin" , SdlAttrDataTypeNumber, SDL_ATTR_LMARGIN,
				CEOffset(CESDLAttrPtr,frmt_specs.lmargin)    },
    { "language" , SdlAttrDataTypeCdata , SDL_ATTR_LANGUAGE   ,
					CEOffset(CESDLAttrPtr,language)      },
    { "length"   , SdlAttrDataTypeNumber, SDL_ATTR_LENGTH ,
				CEOffset(CESDLAttrPtr,num_vals.length)       },
    { "level"    , SdlAttrDataTypeNumber, SDL_ATTR_LEVEL  ,
					CEOffset(CESDLAttrPtr,lev.level)     },
    { "license"  , SdlAttrDataTypeCdata , SDL_ATTR_LICENSE  ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.license)      },
    { "linkinfo" , SdlAttrDataTypeCdata , SDL_ATTR_LINKINFO,
					CEOffset(CESDLAttrPtr,linkinfo)      },
    { "locs"     , SdlAttrDataTypeCdata , SDL_ATTR_LOCS   ,
			CEOffset(CESDLAttrPtr,strings.entry_strs.locs)       },
    { "main"     , SdlAttrDataTypeCdata , SDL_ATTR_MAIN   ,
			CEOffset(CESDLAttrPtr,strings.entry_strs.main)       },
    { "method"   , SdlAttrDataTypeCdata , SDL_ATTR_METHOD ,
			CEOffset(CESDLAttrPtr,strings.not_strs.method)       },
    { NameStr    , SdlAttrDataTypeCdata , SDL_ATTR_NAME   ,
					CEOffset(CESDLAttrPtr,id_vals.name)  },
    { "ncols"    , SdlAttrDataTypeNumber, SDL_ATTR_NCOLS  ,
				CEOffset(CESDLAttrPtr,num_vals.ncols)        },
    { "offset"   , SdlAttrDataTypeNumber, SDL_ATTR_OFFSET ,
				CEOffset(CESDLAttrPtr,num_vals.offset)       },
    {(TopVOrientStr+5), SdlAttrDataTypeEnum  , SDL_ATTR_ORIENT ,
					CEOffset(CESDLAttrPtr,orient)        },
    { "placement", SdlAttrDataTypeEnum  , SDL_ATTR_PLACEMENT,
					CEOffset(CESDLAttrPtr,placement)     },
    { "phrase"   , SdlAttrDataTypeCdata , SDL_ATTR_PHRASE ,
					CEOffset(CESDLAttrPtr,phrase)        },
    { "pointsz"  , SdlAttrDataTypeNumber, SDL_ATTR_POINTSZ ,
				CEOffset(CESDLAttrPtr,font_specs.pointsz)    },
    { "pub-id"   , SdlAttrDataTypeCdata , SDL_ATTR_PUBID  ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.pub_id)       },
    { "prodpn"   , SdlAttrDataTypeCdata , SDL_ATTR_PRODPN   ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.prodpn)       },
    { ProductStr , SdlAttrDataTypeCdata , SDL_ATTR_PRODUCT  ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.product)      },
    { "prodver"  , SdlAttrDataTypeCdata , SDL_ATTR_PRODVER  ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.prodver)      },
    { "r-margin" , SdlAttrDataTypeNumber, SDL_ATTR_RMARGIN,
				CEOffset(CESDLAttrPtr,frmt_specs.rmargin)    },
    { "rid"      , SdlAttrDataTypeId    , SDL_ATTR_RID    ,
					CEOffset(CESDLAttrPtr,id_vals.rid)   },
    { "rlevel"   , SdlAttrDataTypeNumber, SDL_ATTR_RLEVEL ,
					CEOffset(CESDLAttrPtr,lev.rlevel)    },
    { "rssi"     , SdlAttrDataTypeCdata , SDL_ATTR_RSSI   ,
					CEOffset(CESDLAttrPtr,ssi_vals.rssi) },
    { "sdldtd"   , SdlAttrDataTypeCdata , SDL_ATTR_SDLDTD   ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.sdldtd)       },
    { "set-width", SdlAttrDataTypeNumber, SDL_ATTR_SETWIDTH,
				CEOffset(CESDLAttrPtr,font_specs.set_width)  },
    { "slant"    , SdlAttrDataTypeFont  , SDL_ATTR_SLANT,
				CEOffset(CESDLAttrPtr,font_specs.slant)      },
    { "sort"     , SdlAttrDataTypeCdata , SDL_ATTR_SORT   ,
			CEOffset(CESDLAttrPtr,strings.entry_strs.sort)       },
    { "spacing"  , SdlAttrDataTypeFont  , SDL_ATTR_SPACING,
				CEOffset(CESDLAttrPtr,font_specs.spacing)    },
    { "special"  , SdlAttrDataTypeFont  , SDL_ATTR_SPECIAL,
				CEOffset(CESDLAttrPtr,font_specs.special)    },
    { "srcdtd"   , SdlAttrDataTypeCdata , SDL_ATTR_SRCDTD   ,
			CEOffset(CESDLAttrPtr,strings.doc_strs.srcdtd)       },
    { "srch-wt"  , SdlAttrDataTypeNumber, SDL_ATTR_SRCHWT ,
					CEOffset(CESDLAttrPtr,srch_wt)       },
    { "ssi"      , SdlAttrDataTypeCdata , SDL_ATTR_SSI    ,
			CEOffset(CESDLAttrPtr,ssi_vals.ssi)},
    { "stack"    , SdlAttrDataTypeEnum  , SDL_ATTR_STACK,
					CEOffset(CESDLAttrPtr,stacking)      },
    { "style"    , SdlAttrDataTypeFont  , SDL_ATTR_STYLE,
				CEOffset(CESDLAttrPtr,font_specs.style)      },
    { "syns"     , SdlAttrDataTypeCdata , SDL_ATTR_SYNS   ,
			CEOffset(CESDLAttrPtr,strings.entry_strs.syns)       },
    { "t-margin" , SdlAttrDataTypeNumber, SDL_ATTR_TMARGIN,
				CEOffset(CESDLAttrPtr,frmt_specs.tmargin)    },
    { (TextStr+1), SdlAttrDataTypeCdata , SDL_ATTR_TEXT     ,
			CEOffset(CESDLAttrPtr,strings.el_strs.text)          },
    { "timestmp" , SdlAttrDataTypeCdata , SDL_ATTR_TIMESTAMP,
			CEOffset(CESDLAttrPtr,strings.doc_strs.timestamp)    },
    { "timing"   , SdlAttrDataTypeEnum  , SDL_ATTR_TIMING ,
					CEOffset(CESDLAttrPtr,timing)        },
    { "type"     , SdlAttrDataTypeEnum  , SDL_ATTR_TYPE   ,
					CEOffset(CESDLAttrPtr,type)          },
    { "traversal", SdlAttrDataTypeEnum  , SDL_ATTR_TRAVERSAL,
					CEOffset(CESDLAttrPtr,traversal)     },
    { "typenam"  , SdlAttrDataTypeCdata , SDL_ATTR_TYPENAM,
				CEOffset(CESDLAttrPtr,font_specs.typenam)    },
    { "typenamb" , SdlAttrDataTypeCdata , SDL_ATTR_TYPENAMB,
				CEOffset(CESDLAttrPtr,font_specs.typenamb)   },
    { "typenami" , SdlAttrDataTypeCdata , SDL_ATTR_TYPENAMI,
				CEOffset(CESDLAttrPtr,font_specs.typenami)   },
    { "typenamib", SdlAttrDataTypeCdata , SDL_ATTR_TYPENAMIB,
				CEOffset(CESDLAttrPtr,font_specs.typenamib)  },
    { "version"  , SdlAttrDataTypeCdata , SDL_ATTR_VERSION,
					CEOffset(CESDLAttrPtr,version)       },
    {(TopVJustStr+4), SdlAttrDataTypeEnum  , SDL_ATTR_VJUST,
				CEOffset(CESDLAttrPtr,frmt_specs.vjust)      },
    {(TopVOrientStr+4), SdlAttrDataTypeEnum  , SDL_ATTR_VORIENT ,
					CEOffset(CESDLAttrPtr,vorient)       },
    { "weight"   , SdlAttrDataTypeFont  , SDL_ATTR_WEIGHT,
				CEOffset(CESDLAttrPtr,font_specs.weight)     },
    { "window"   , SdlAttrDataTypeEnum  , SDL_ATTR_WINDOW,
					CEOffset(CESDLAttrPtr,window)        },
    { "xid"      , SdlAttrDataTypeCdata , SDL_ATTR_XID    ,
			CEOffset(CESDLAttrPtr,strings.not_strs.xid)       },
    { "xlfd"     , SdlAttrDataTypeCdata , SDL_ATTR_XLFD   ,
				CEOffset(CESDLAttrPtr,font_specs.xlfd)       },
    { "xlfdb"    , SdlAttrDataTypeCdata , SDL_ATTR_XLFDB  ,
				CEOffset(CESDLAttrPtr,font_specs.xlfdb)      },
    { "xlfdi"    , SdlAttrDataTypeCdata , SDL_ATTR_XLFDI  ,
				CEOffset(CESDLAttrPtr,font_specs.xlfdi)      },
    { "xlfdib"   , SdlAttrDataTypeCdata , SDL_ATTR_XLFDIB ,
				CEOffset(CESDLAttrPtr,font_specs.xlfdib)     },
    { NULL       , SdlAttrDataTypeInvalid, -1,  NULL                         },
  };

/******************************************************************************
 * Element Content
 *****************************************************************************/
/*-----------------------------------------------------------------------------
<!-- Document Hierarchy  _____________________________________________-->

<!-- The root element is a pageless document, sdldoc.
   -   A sdldoc contains one or more virtual pages.
   -   A Virtual page is the smallest display unit.
   -   A Block is a unit of a given style.
   -   A Paragraph is a unit of character formatting.
  -->

<!ELEMENT sdldoc     - - (vstruct, head*, snb?, virpage+)>
<!ATTLIST sdldoc         pub-id     CDATA    #REQUIRED
                         doc-id     CDATA    #REQUIRED
                         timestmp   CDATA    #REQUIRED
                         first-page IDREF    #IMPLIED
                         product    CDATA    #IMPLIED
                         prodpn     CDATA    #IMPLIED
                         prodver    CDATA    #IMPLIED
                         license    CDATA    #IMPLIED
                         language   CDATA    #IMPLIED
			 charset    CDATA    #IMPLIED
                         author     CDATA    #IMPLIED
                         version    CDATA    #IMPLIED
			 sdldtd     CDATA    #REQUIRED
                         srcdtd     CDATA    #IMPLIED >

-----------------------------------------------------------------------------*/
static SDLContent SdlDocContent[] =
    {
/*
	{ CEInitializeMask(SdlElementVStruct), OnlyOne   },
 * Do not want to parse the entire document.
 * Want to only read enought to work with.
 */
	{ CEInitializeMask(SdlElementSdlDoc) , FakeEnd   },
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };
static SDLElementAttrList  SdlDocAttrList[] =
    {
	{ SDL_ATTR_PUBID    , SdlAttrValueRequired, NULL   },
	{ SDL_ATTR_DOCID    , SdlAttrValueRequired, NULL   },
	{ SDL_ATTR_TIMESTAMP, SdlAttrValueRequired, NULL   },
	{ SDL_ATTR_PRODUCT  , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_PRODPN   , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_PRODVER  , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_LICENSE  , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_LANGUAGE , SdlAttrValueImplied , "C"    },
	{ SDL_ATTR_CHARSET  , SdlAttrValueImplied , IsoStr },
	{ SDL_ATTR_AUTHOR   , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_VERSION  , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_FRST_PG  , SdlAttrValueImplied , NULL   },
	{ SDL_ATTR_SDLDTD   , SdlAttrValueRequired, NULL   },
	{ SDL_ATTR_SRCDTD   , SdlAttrValueImplied , NULL   },
	{ -1                , SdlAttrValueBad     , NULL   }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT vstruct   - - (%generated-elements;) >
<!ATTLIST vstruct       version   CDATA              #IMPLIED
			doc-id    CDATA              #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLContent VStructContent[] =
    {
	SDL_ENTITY_GENERATED_ELEMENTS
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };
static SDLElementAttrList  VStructAttrList[] =
    {
	{ SDL_ATTR_DOCID   , SdlAttrValueImplied , NULL },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied , NULL },
	{ -1               , SdlAttrValueBad     , NULL }
    };

/*-----------------------------------------------------------------------------

<!ELEMENT virpage   - - (head*, snb?, (block | form)*) >
<!ATTLIST virpage       id        ID                 #REQUIRED
			level     NUMBER             #REQUIRED
			version   CDATA              #IMPLIED
			language  CDATA              #IMPLIED
			charset   CDATA              #IMPLIED
			doc-id    CDATA              #REQUIRED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent VirpageContent[] =
    {
        { CEInitializeMask(SdlElementHead)     , ZeroToN   },
        { CEInitializeMask(SdlElementSnb)      , ZeroOrOne },
        { CEInitializeMaskTwo(SdlElementBlock, \
				SdlElementForm), ZeroToN   },
        { CEInitializeMask(SdlElementNone)     , NULL      },
    };
static SDLContent VirpageHeadContent[] =
    {
        { CEInitializeMask(SdlElementHead)     , ZeroToN     },
        { CEInitializeMask(SdlElementVirpage)  , CheckForSnb },
	{ CEInitializeMask(SdlElementTitle)    , FakeEnd     },
        { CEInitializeMask(SdlElementNone)     , NULL        },
    };

static SDLContent HeadAndSnb[] =
    {
        { CEInitializeMask(SdlElementHead)     , ZeroToN     },
        { CEInitializeMask(SdlElementHead)     , CheckForSnb },
        { CEInitializeMask(SdlElementNone)     , NULL        },
    };

static SDLElementAttrList  VirpageAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired  , NULL   },
	{ SDL_ATTR_LEVEL   , SdlAttrValueRequired  , NULL   },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied   , NULL   },
	{ SDL_ATTR_LANGUAGE, SdlAttrValueImplied   , NULL   },
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied   , NULL   },
	{ SDL_ATTR_DOCID   , SdlAttrValueRequired  , NULL   },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL   },
	{ -1               , SdlAttrValueBad       , NULL   }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT snb       - - (head?, (%system-notations;)+) >
<!ATTLIST snb           version   CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent SnbContent[] =
    {
	{ CEInitializeMask(SdlElementHead)  , ZeroOrOne },
	{ SDL_ENTITY_SYSTEM_NOTATIONS       , OneToN    },
	{ CEInitializeMask(SdlElementNone)  , NULL      },
    };
static SDLElementAttrList  SnbAttrList[] =
    {
	{ SDL_ATTR_VERSION, SdlAttrValueImplied , NULL },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT block     - - (head*, (p | cp)*) >
<!ATTLIST block         id        ID                 #IMPLIED

			-- processor should default level to "1"      --
			level     NUMBER             #IMPLIED
			version   CDATA              #IMPLIED
			class     (%format-class;)   #IMPLIED
			language  CDATA              #IMPLIED

			-- processor shld deflt charset to "ISO-8859-1" --
			charset   CDATA              #IMPLIED

			length    NUMBER             #IMPLIED

			-- processor should default app to "all"      --
			app      ( all      |
				   help     |
				   tutorial |
				   ref      |
				   sys      )        #IMPLIED

			-- processor should default timing to "sync"  --
			timing   ( sync | async )    #IMPLIED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent BlockContent[] =
    {
	{ CEInitializeMask(SdlElementHead)                    , ZeroToN   },
	{ CEInitializeMaskTwo(SdlElementPara, SdlElementCPara), ZeroToN   },
	{ CEInitializeMask(SdlElementNone)                    , NULL      },
    };
static SDLElementAttrList  BlockAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueImpliedDef , NULL           },
	{ SDL_ATTR_LEVEL   , SdlAttrValueImpliedDef , OneStr         },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied    , NULL           },
	{ SDL_ATTR_CLASSF  , SdlAttrValueImplied    , (TextStr+1)    },
	{ SDL_ATTR_LANGUAGE, SdlAttrValueImplied    , NULL           },
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied    , NULL           },
	{ SDL_ATTR_LENGTH  , SdlAttrValueImplied    , NegativeOneStr },
	{ SDL_ATTR_APP     , SdlAttrValueImplied    , AllStr         },
	{ SDL_ATTR_TIMING  , SdlAttrValueImplied    , (ASyncStr+1)   },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef , NULL           },
	{ -1               , SdlAttrValueBad        , NULL           }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT form      - - (head*, ((fstyle, fdata) | (fdata, fstyle))) >
<!ATTLIST form          id        ID                 #IMPLIED

			-- processor shld deflt level to nest of form --
			level     NUMBER             #IMPLIED
			version   CDATA              #IMPLIED
			class     (%format-class;)   #IMPLIED
			language  CDATA              #IMPLIED

			-- processor shld deflt charset to "ISO-8859-1" --
			charset   CDATA              #IMPLIED
			length    NUMBER             #IMPLIED

			-- processor should default app to "all"      --
			app      ( all      |
				   help     |
				   tutorial |
				   ref      |
				   sys      )        #IMPLIED
			ssi       CDATA              #IMPLIED >

-----------------------------------------------------------------------------*/
static SDLContent FormContent[] =
    {
	{ CEInitializeMask(SdlElementHead)  , ZeroToN   },
	{ CEInitializeMaskTwo(SdlElementFstyle, SdlElementFdata),
						OnlyOneEach   },
	{ CEInitializeMask(SdlElementNone)  , NULL      },
    };
static SDLElementAttrList  FormAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_LEVEL   , SdlAttrValueImplied   , OneStr         },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_CLASSF  , SdlAttrValueImplied   , (TextStr+1)    },
	{ SDL_ATTR_LANGUAGE, SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_LENGTH  , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_APP     , SdlAttrValueImplied   , AllStr         },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL           },
	{ -1               ,SdlAttrValueBad        , NULL           }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT fstyle    - - (frowvec+) >
<!ATTLIST fstyle        -- The number of columns in this form, ncols, --
                        -- should be a number greater than zero.      --
			-- Unless overridden by a "colw" (column      --
			-- width) specification in a formstyle, the   --
			-- available space is divided evenly among    --
			-- the columns.  Unless overriden by a "colj" --
			-- specification in a formstyle, the columns  --
			-- are all left justified.                    --
                        -- processor should default ncols to 1        --
                        ncols     NUMBER             #IMPLIED >

-----------------------------------------------------------------------------*/
static SDLContent FstyleContent[] =
    {
	{ CEInitializeMask(SdlElementFrowvec), OneToN  },
	{ CEInitializeMask(SdlElementNone)   , NULL    },
    };
static SDLElementAttrList  FstyleAttrList[] =
    {
	{ SDL_ATTR_NCOLS  , SdlAttrValueImpliedDef , OneStr },
	{ -1              , SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT frowvec   - O EMPTY >
<!ATTLIST frowvec       -- processor should default hdr to "NO"       --
			hdr       (YES | NO)         #IMPLIED

			-- Ids of cell contents.  One id per cell.    --
                        -- Each id must refer to either a block or a  --
                        -- form.                                      --
			cells     IDREFS             #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  FrowvecAttrList[] =
    {
	{ SDL_ATTR_HDR    , SdlAttrValueImplied , "NO"      },
	{ SDL_ATTR_CELLS  , SdlAttrValueRequired, NULL      },
	{ -1              , SdlAttrValueBad     , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT fdata     - - ((block | form)*) >
-----------------------------------------------------------------------------*/
static SDLContent FdataContent[] =
    {
	{ CEInitializeMaskTwo(SdlElementBlock, SdlElementForm), ZeroToN },
	{ CEInitializeMask(SdlElementNone)                    , NULL    },
    };

/*-----------------------------------------------------------------------------
<!-- Containers ______________________________________________________-->

<!ELEMENT p         - - (head*, (%atomic | #PCDATA)+) >
<!ATTLIST p             id        ID                 #IMPLIED
			version   CDATA              #IMPLIED

			-- processor should default type to "dynamic" --
			type      (literal |
				   lined   |
				   dynamic )         #IMPLIED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent ParaContent[] =
    {
/*
 * This is optional and followed by potential PCDATA, therefore include
 * the SdlElementCdata & SdlPcDataFollows flag.
 */
	{ CEInitializeMaskThree(SdlElementHead,SdlPcDataFollows,SdlElementCdata), ZeroToN   },
	{ CEInitializeMask(SdlElementPara), InitLast  },
/*
 * PCDATA is described as 0 to n characters,
 * therefore, while the content says 'one to n' of (%atomic | #PCDATA)
 * we need to specify ZeroToN to work for PCDATA
	{ SDL_ENTITY_ATOMIC               , OneToN    },
 */
	{ SDL_ENTITY_ATOMIC               , ZeroToN   },
	{ CEInitializeMask(SdlElementNone), NULL      },
    };
static SDLElementAttrList  ParaAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueImpliedDef, NULL      },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied   , NULL      },
	{ SDL_ATTR_TYPEFRMT, SdlAttrValueImplied   , DynamicStr},
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL      },
	{ -1               , SdlAttrValueBad       , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT cp        - - CDATA >
<!ATTLIST cp            id        ID                 #IMPLIED
			version   CDATA              #IMPLIED

			-- processor should default type to "dynamic" --
			type      (literal |
				   lined   |
				   dynamic )         #IMPLIED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent CParaContent[] =
    {
	{ CEInitializeMask(SdlElementCPara), InitLast  },
	{ CEInitializeMask(SdlElementCdata), ZeroOrOne },
	{ CEInitializeMask(SdlElementNone) , NULL  },
    };
/* uses the same attributes as <p> */

/*-----------------------------------------------------------------------------
<!-- Heads may have multiple sub-heads -->
<!ELEMENT head      - - ((%atomic; | #PCDATA)*, subhead*) >
<!ELEMENT subhead   - - ((%atomic | #PCDATA)*) >
<!ATTLIST (head | subhead)
			id        ID                 #IMPLIED
			version   CDATA              #IMPLIED

			-- processor should default class to "head"   --
			class     (%head-class;)     #IMPLIED
			language  CDATA              #IMPLIED

			-- processor shld deflt charset to "ISO-8859-1" --
			charset   CDATA              #IMPLIED

                        -- processor should default type to "dynamic" --
                        type      (literal |
                                   lined   |
                                   dynamic )         #IMPLIED
                        abbrev    CDATA              #IMPLIED

			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent HeadContent[] =
    {
	{ CEInitializeMask(SdlElementHead)   , InitLast  },
	{ SDL_ENTITY_ATOMIC                  , ZeroToN },
	{ CEInitializeMask(SdlElementSubHead), ZeroToN },
	{ CEInitializeMask(SdlElementNone)   , NULL    },
    };
static SDLContent SubHeadContent[] =
    {
	{ CEInitializeMask(SdlElementSubHead), InitLast  },
	{ SDL_ENTITY_ATOMIC                  , ZeroToN   },
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };
static SDLElementAttrList  HeadAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueImpliedDef, NULL       },
	{ SDL_ATTR_VERSION , SdlAttrValueImplied   , NULL       },
	{ SDL_ATTR_CLASSH  , SdlAttrValueImplied   , (HeadStr+1)},
	{ SDL_ATTR_LANGUAGE, SdlAttrValueImplied   , NULL       },
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied   , NULL       },
	{ SDL_ATTR_TYPEFRMT, SdlAttrValueImplied   , DynamicStr },
	{ SDL_ATTR_ABBREV  , SdlAttrValueImplied   , NULL       },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL       },
	{ -1               , SdlAttrValueBad       , NULL       }
    };

/*-----------------------------------------------------------------------------
<!-- Atomic Link Elements ____________________________________________-->

<!ELEMENT anchor    - O EMPTY     -- Just marks the spot to jump to   -->
<!ATTLIST anchor        id        ID                 #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  AnchorAttrList[] =
    {
	{ SDL_ATTR_ID       , SdlAttrValueRequired, NULL      },
	{ -1                , SdlAttrValueBad     , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT link      - - ((%atomic; | #PCDATA)+) -(link)>
<!ATTLIST link          -- rid is to id in this document or to a link --
			-- type element such as crossdoc in the snb   --
                        -- of the current virpage                     --
			rid       IDREF              #REQUIRED

			-- button should be a graphic in the snb      --
			button    IDREF              #IMPLIED

                        linkinfo  CDATA              #IMPLIED
                        descript  CDATA              #IMPLIED

			-- processor shld default window to "current" --
			window    (current  |
				   new      |
				   popup    )        #IMPLIED

			-- procssr shld dflt traversal to "noreturn"  --
			traversal (return   |
				   noreturn )        #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent LinkContent[] =
    {
	{ CEInitializeMask(SdlElementLink) , RegisterLink },
/*
 * PCDATA is described as 0 to n characters,
 * therefore, while the content says 'one to n' of (%atomic | #PCDATA)
 * we need to specify ZeroToN to work for PCDATA
	{ SDL_ENTITY_ATOMIC                , OneToN       },
 */
	{ SDL_ENTITY_ATOMIC                , ZeroToN      },
	{ CEInitializeMask(SdlElementNone) , NULL         },
    };
static SDLElementAttrList  LinkAttrList[] =
    {
	{ SDL_ATTR_RID      , SdlAttrValueRequired  , NULL      },
	{ SDL_ATTR_BUTTON   , SdlAttrValueImpliedDef, NULL      },
	{ SDL_ATTR_WINDOW   , SdlAttrValueImplied   , "current" },
	{ SDL_ATTR_TRAVERSAL, SdlAttrValueImplied   , "noreturn"},
	{ SDL_ATTR_DESCRIPT , SdlAttrValueImpliedDef, NULL      },
	{ SDL_ATTR_LINKINFO , SdlAttrValueImpliedDef, NULL      },
	{ -1                , SdlAttrValueBad       , NULL      }
    };

/*-----------------------------------------------------------------------------
<!-- reference to an element or alternates in the system notation block -->
<!ELEMENT snref     - - (refitem+, alttext?) >
<!ATTLIST snref         id       ID          #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent SnrefContent[] =
    {
	{ CEInitializeMask(SdlElementSnRef)  , MarkFound  },
	{ CEInitializeMask(SdlElementRefItem), OneToN     },
	{ CEInitializeMask(SdlElementAltText), ZeroOrOne  },
	{ CEInitializeMask(SdlElementNone)   , NULL       },
    };
static SDLElementAttrList  SnrefAttrList[] =
    {
	{ SDL_ATTR_ID, SdlAttrValueImpliedDef, NULL      },
	{ -1         , SdlAttrValueBad       , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- Each refitem is tried in turn until one can be  successfully
   - formatted.  The button is used to request display of the refitem
   - on systems where display of the item would be slow or expensive
   - in some other way, i.e., the button is displayed and the refitem
   - is only displayed on activiation of the button.
  -->
<!ELEMENT refitem   - - (head*) >
<!ATTLIST refitem       -- rid should point to a representational    --
                        -- element in the system notation block      --
                        rid      IDREF               #REQUIRED
                        class    (%graphic-class; |
                                  %format-class;  )  #REQUIRED

                        -- button should be a graphic in the snb      --
                        button    IDREF              #IMPLIED
                        ssi       CDATA              #IMPLIED   >
-----------------------------------------------------------------------------*/
static SDLContent RefItemContent[] =
    {
	{ CEInitializeMask(SdlElementHead)    , ZeroOrOne    },
	{ CEInitializeMask(SdlElementRefItem) , MaintainLink },
	{ CEInitializeMask(SdlElementNone)    , NULL         },
    };
static SDLElementAttrList  RefItemAttrList[] =
    {
	{ SDL_ATTR_RID    , SdlAttrValueRequired  , NULL      },
	{ SDL_ATTR_CLASSFG, SdlAttrValueRequired  , NULL      },
	{ SDL_ATTR_BUTTON , SdlAttrValueImplied   , NULL      },
	{ SDL_ATTR_SSI    , SdlAttrValueImpliedDef, NULL      },
	{ -1              , SdlAttrValueBad       , NULL }
    };
/*-----------------------------------------------------------------------------
<!-- simple text to use if all else fails -->
<!ELEMENT alttext   - - CDATA >
-----------------------------------------------------------------------------*/
static SDLContent AltTextContent[] =
    {
	{ CEInitializeMask(SdlElementCdata), ZeroOrOne },
	{ CEInitializeMask(SdlElementNone) , NULL    },
    };

/*-----------------------------------------------------------------------------
<!-- Atomic Text Elements ____________________________________________-->

<!-- empty rev implies delete -->
<!ELEMENT rev       - - ((%atomic; | #PCDATA)*) -(rev) >
-----------------------------------------------------------------------------*/
static SDLContent RevContent[] =
    {
	{ SDL_ENTITY_ATOMIC               , ZeroToN },
	{ CEInitializeMask(SdlElementNone), NULL    },
    };

/*-----------------------------------------------------------------------------
<!-- font changes -->
<!ELEMENT key       - - ((%atomic; | #PCDATA)*) -(link) >
<!ATTLIST key           -- processor shld deflt charset to "ISO-8859-1" --
			charset   CDATA              #IMPLIED
			class     (%key-class;)      #REQUIRED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent KeyContent[] =
    {
	{ CEInitializeMask(SdlElementKey) , ProcessEnterAttr },
	{ SDL_ENTITY_ATOMIC               , ZeroToN          },
	{ CEInitializeMask(SdlElementKey) , ProcessExitAttr  },
	{ CEInitializeMask(SdlElementNone), NULL             },
    };
static SDLElementAttrList  KeyAttrList[] =
    {
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied   , NULL      },
	{ SDL_ATTR_CLASSK  , SdlAttrValueRequired  , NULL      },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL      },
	{ -1               , SdlAttrValueBad       , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- super or subscripted phrase -->
<!ELEMENT sphrase   - - (spc | #PCDATA)* >
<!ATTLIST sphrase       class     %phrase-class      #REQUIRED
			ssi       CDATA              #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent SphraseContent[] =
    {
	{ CEInitializeMask(SdlElementSphrase), CheckType },
	{ CEInitializeMaskTwo(SdlElementSpc, SdlElementCdata), ZeroToN },
	{ CEInitializeMask(SdlElementSphrase), SetType   },
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };
static SDLElementAttrList  SphraseAttrList[] =
    {
	{ SDL_ATTR_CLASSP  , SdlAttrValueRequired  , NULL      },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL      },
	{ -1               , SdlAttrValueBad       , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- conditional inclusion of text -->
<!ELEMENT if        - - (cond, then, else?) >
-----------------------------------------------------------------------------*/
static SDLContent IfContent[] =
    {
	{ CEInitializeMask(SdlElementIf)  , IfInfo    },
	{ CEInitializeMask(SdlElementCond), OnlyOne   },
	{ CEInitializeMask(SdlElementThen), OnlyOne   },
	{ CEInitializeMask(SdlElementElse), ZeroOrOne },
	{ CEInitializeMask(SdlElementNone), NULL      },
    };

/*-----------------------------------------------------------------------------
<!-- call "interp" passing CDATA -->
<!ELEMENT cond      - - CDATA >
<!ATTLIST cond          -- processor should default interp to "ksh"   --
                        interp    NOTATION
                                  (tcl |
                                   sh  |
                                   ksh |
                                   csh )             #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent CondContent[] =
    {
	{ CEInitializeMask(SdlElementCond) , SetType     },
	{ CEInitializeMask(SdlElementCdata), ZeroOrOne   },
	{ CEInitializeMask(SdlElementNone) , NULL        },
    };
static SDLElementAttrList  CondAttrList[] =
    {
	{ SDL_ATTR_INTERP  , SdlAttrValueImplied    , "ksh"     },
	{ -1               , SdlAttrValueBad        , NULL      }
    };

/*-----------------------------------------------------------------------------
<!-- include this text if "cond" returns non-zero -->
<!ELEMENT then      - - ((%atomic; | #PCDATA)*) >

<!-- include this text if "cond" returns zero -->
<!ELEMENT else      - - ((%atomic; | #PCDATA)*) >
-----------------------------------------------------------------------------*/
static SDLContent ThenElseContent[] =
    {
	{ CEInitializeMask(SdlElementIf)  , IfInfo    },
	{ SDL_ENTITY_ATOMIC               , ZeroToN   },
	{ CEInitializeMask(SdlElementNone), NULL      },
    };

/*-----------------------------------------------------------------------------
<!-- special characters: the "name" attribute must be one of the special
   - character names, e.g., [bull  ] (bullet), defined in the public
   - character entities such as ISOpub, "ISO 8879:1986//ENTITIES
   - Publishing//EN";  these entities are defined in sections D.4.2.1
   - through D.4.5.6 of The SGML Handbook (Goldfarb).
   -->
<!ELEMENT spc       - O EMPTY >
<!ATTLIST spc           name      CDATA              #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLContent SpcContent[] =
    {
	{ CEInitializeMask(SdlElementSpc) , AllocateSpace },
	{ CEInitializeMask(SdlElementNone), NULL       },
    };
static SDLElementAttrList  SpcAttrList[] =
    {
	{ SDL_ATTR_NAME, SdlAttrValueRequired, NULL        },
	{ -1               ,SdlAttrValueBad        , NULL }
    };
/*-----------------------------------------------------------------------------
<!-- Notation Elements _______________________________________________-->
<!-- The first group are representaional - referenced via <snref>     -->

<!-- Only one graphic element is declared.  The "class" attribute in
   - the refitem of the referring snref is used to distinguish between
   - figure, in-line or button.
  -->
<!ELEMENT graphic   - O EMPTY    >
<!ATTLIST graphic       id       ID                  #REQUIRED
                        format   CDATA               #IMPLIED
                        method   IDREF               #IMPLIED
                        xid      CDATA               #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLContent GraphicContent[] =
    {
	{ CEInitializeMask(SdlElementGraphic), AllocateSpace },
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };
static SDLElementAttrList  NotationAttrList[] =
    {
	{ SDL_ATTR_ID    , SdlAttrValueRequired, NULL        },
	{ SDL_ATTR_FORMAT, SdlAttrValueImplied , NULL        },
	{ SDL_ATTR_METHOD, SdlAttrValueImplied , NULL        },
	{ SDL_ATTR_XID   , SdlAttrValueRequired, NULL        },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- alternate rich text for use in a list of refitems in snref -->
<!ELEMENT text      - - ((p | cp)*) >
<!ATTLIST text          id       ID          #REQUIRED
			language CDATA       #IMPLIED

			-- processor shld dflt charset to "ISO-8859-1"  --
			charset  CDATA       #IMPLIED   >
-----------------------------------------------------------------------------*/
static SDLContent TextContent[] =
    {
	{ CEInitializeMaskTwo(SdlElementPara, SdlElementCPara), ZeroToN   },
	{ CEInitializeMask(SdlElementNone)                    , NULL      },
    };
static SDLElementAttrList  TextAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_LANGUAGE, SdlAttrValueImplied , NULL      },
	{ SDL_ATTR_CHARSET , SdlAttrValueImplied , NULL      },
	{ -1               , SdlAttrValueBad     , NULL      }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT audio     - O EMPTY >
<!ATTLIST audio         id        ID                 #REQUIRED
			format    CDATA              #IMPLIED
			method    IDREF              #IMPLIED
			xid       CDATA              #REQUIRED >

<!ELEMENT video     - O EMPTY >
<!ATTLIST video         id        ID                 #REQUIRED
			format    CDATA              #IMPLIED
			method    IDREF              #IMPLIED
			xid       CDATA              #REQUIRED >

<!ELEMENT animate   - O EMPTY >
<!ATTLIST animate       id        ID                 #REQUIRED
			format    CDATA              #IMPLIED
			method    IDREF              #IMPLIED
			xid       CDATA              #REQUIRED >

/----
/- The audio, video, and animate use the NotationAttrList
/- structure defined under graphic.
/---
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
<!-- Execute the content of this element using the specified "interp"
   - whenever the script element is referenced via an snref.  If the
   - script returns a value, that value effectively becomes the CDATA
   - content of the script element and is interpolated into the document
   - at the point of the snref.
  -->
<!ELEMENT script    - - CDATA >
<!ATTLIST script        id        ID                 #REQUIRED
			-- processor should default interp to "ksh"   --
			interp    NOTATION
				  (tcl |
				   sh  |
				   ksh |
				   csh )             #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLContent ScriptContent[] =
    {
	{ CEInitializeMask(SdlElementScript)    , SetType   },
	{ CEInitializeMask(SdlElementCdata)     , ZeroOrOne },
	{ CEInitializeMask(SdlElementNone)      , NULL      },
    };
static SDLElementAttrList  ScriptAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_INTERP  , SdlAttrValueImplied , "ksh"     },
	{ -1               ,SdlAttrValueBad        , NULL }
    };


/*-----------------------------------------------------------------------------
<!-- The second group in the snb is linkage - referenced via <link>   -->
<!ELEMENT crossdoc  - O EMPTY >
<!ATTLIST crossdoc      id        ID                 #REQUIRED

			-- cross document link - doc & id   --
			xid       CDATA              #REQUIRED >

<!ELEMENT man-page  - O EMPTY >
<!ATTLIST man-page      id        ID                 #REQUIRED
			xid       CDATA              #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  IdAndXidAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_XID     , SdlAttrValueRequired, NULL      },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT textfile  - O EMPTY >
<!ATTLIST textfile      id        ID                 #REQUIRED
			xid       CDATA              #REQUIRED
			offset    CDATA              #IMPLIED
			format    CDATA              #IMPLIED  >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  TextFileAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_XID     , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_OFFSET  , SdlAttrValueImplied , ZeroStr   },
	{ SDL_ATTR_FORMAT  , SdlAttrValueImplied , NULL      },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT sys-cmd   - O EMPTY >
<!ATTLIST sys-cmd       id        ID                 #REQUIRED
			command   CDATA              #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  SysCmdAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_COMMAND , SdlAttrValueRequired, NULL      },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT callback  - O EMPTY >
<!ATTLIST callback     id         ID                 #REQUIRED
		       data       CDATA              #IMPLIED  >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  CallbackAttrList[] =
    {
	{ SDL_ATTR_ID      , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_DATA    , SdlAttrValueImplied , NULL      },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- The switch element is always hyperlinked to.  The interpreter,
   - interp, is called passing the CDATA content and returning a number
   - 0 to n.  The return value of the interpreter is used to index into
   - the list of branches and the hyperlink is continued to that ID.  A
   - return value less than zero or greater than the number of IDs minus
   - 1 causes the hyperlink to continue to branch 0.
  -->
<!ELEMENT switch    - - CDATA >
<!ATTLIST switch        id       ID          #REQUIRED
                        -- processor should default interp to "ksh"   --
                        interp   NOTATION
                                 (tcl |
                                  sh  |
                                  ksh |
                                  csh )      #IMPLIED
                        branches IDREFS      #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLContent SwitchContent[] =
    {
	{ CEInitializeMask(SdlElementSwitch), SetType   },
	{ CEInitializeMask(SdlElementCdata) , ZeroOrOne },
	{ CEInitializeMask(SdlElementNone)  , NULL         },
    };
static SDLElementAttrList SwitchAttrList[] =
    {
	{ SDL_ATTR_ID       , SdlAttrValueRequired, NULL      },
	{ SDL_ATTR_INTERP   , SdlAttrValueImplied , "ksh"     },
	{ SDL_ATTR_BRANCHES , SdlAttrValueRequired, NULL      },
	{ -1                , SdlAttrValueBad     , NULL      }
    };

/*-----------------------------------------------------------------------------
<!-- Generated Elements ______________________________________________-->

<!ELEMENT rel-docs  - O EMPTY >
<!ELEMENT rel-file  - O EMPTY >
<!ELEMENT notes     - O EMPTY >

<!-- The list of identifiers, loids, element is a list of ids in this
     document in the order they are defined.  The "count" attribute of
     loids is the number of ids it contains.  The higher level DTD to
     SDL translator may precompute "count" to enable the processor to
     preallocate space for the ids.  If "count" is not present, the
     processor must compute the number itself from the document.  The
     "type" attribute of id is the name of the element to which the
     id belongs.  The "rid" (referenced identifier) attribute is the
     identifier being listed.  The "rssi" (referenced source semantic
     identifier) is the "ssi" of the element to which the identifier
     belongs.  Similarly, the "rlevel" (referenced level) attribute
     is the "level" of the element to which the identifier belongs.
     Finally, the "offset" attribute is the byte offset in the document
     to the start of the virtual page containing the identifier. -->
<!ELEMENT loids     - - (id*) >
<!ATTLIST loids         count     NUMBER             #IMPLIED   >
-----------------------------------------------------------------------------*/
static SDLContent LoidsContent[] = {
	{ CEInitializeMask(SdlElementLoids), AllocateBlock },
	{ CEInitializeMask(SdlElementId)   , ZeroToN       },
	{ CEInitializeMask(SdlElementLoids), CleanUpBlock  },
	{ CEInitializeMask(SdlElementNone) , NULL          },
    };

static SDLElementAttrList  LoidsAttrList[] =
    {
	{ SDL_ATTR_COUNT, SdlAttrValueImplied, NULL },
	{ -1            , SdlAttrValueBad    , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT id        - O EMPTY >
<!ATTLIST id            type     (virpage  |
                                  block    |
                                  form     |
                                  p        |
                                  cp       |
                                  head     |
                                  subhead  |
                                  anchor   |
                                  switch   |
                                  snref    |
                                  graphic  |
                                  text     |
                                  audio    |
                                  video    |
                                  animate  |
                                  crossdoc |
                                  man-page |
                                  textfile |
                                  sys-cmd  |
                                  script   |
                                  callback )         #REQUIRED
                        rid       IDREF              #REQUIRED
                        rssi      CDATA              #IMPLIED
                        rlevel    NUMBER             #IMPLIED
                        offset    NUMBER             #REQUIRED  >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  IdAttrList[] =
    {
	{ SDL_ATTR_TYPEID  , SdlAttrValueRequired, NULL },
	{ SDL_ATTR_RID     , SdlAttrValueRequired, NULL },
	{ SDL_ATTR_RLEVEL  , SdlAttrValueImplied , NegativeOneStr },
	{ SDL_ATTR_RSSI    , SdlAttrValueImpliedDef, NULL },
	{ SDL_ATTR_OFFSET  , SdlAttrValueRequired, NULL },
	{ -1               , SdlAttrValueBad     , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- An index consists of zero or more entries.  Each entry contains
     the indexed (rich) text.  The "count" attribute of index is the
     number of entries (recursively) it contains.  The higher level
     DTD to SDL translator may precompute "count" to enable the
     processor to preallocate space for the entries.  If "count" is
     not present, the processor must compute the number itself from
     the document.  The "locs" and "main" attributes of an entry are
     lists of ids where the entry may be found.  Those ids found on
     the "main" list may be highlighted or emphasized in some way to
     indicate a greater importance than the ids found on the "locs"
     list - a definition, for example.  Otherwise, ids found on the
     "locs" list and the "main" list behave identically.  The "syns"
     attribute of an entry is another list of ids that refer to other
     entry elements and correspond to a "See also" or synonym type
     reference in an index.  The "sort" attribute is an optional sort
     key to be used if the indexed entry is to be sorted other than
     by its content.  The index should be pre-sorted although the
     optional sort keys are preserved in case multiple indexes need
     to be merged at some later date.  An entry element may also
     contain other entries to allow a hierarchical index to be
     generated. -->
<!ELEMENT index     - - (entry*)                              >
<!ATTLIST index         head      CDATA  #IMPLIED
			count     NUMBER #IMPLIED              >
-----------------------------------------------------------------------------*/
static SDLContent IndexContent[] = {
	{ CEInitializeMask(SdlElementEntry), ZeroToN },
	{ CEInitializeMask(SdlElementNone) , NULL    },
    };

static SDLElementAttrList  IndexAttrList[] =
    {
	{ SDL_ATTR_COUNT, SdlAttrValueImplied, NULL },
	{ -1            , SdlAttrValueBad    , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT entry     - - ((%simple; | #PCDATA)*, entry*)       >
<!ATTLIST entry         id      ID     #IMPLIED
                        main    IDREFS #IMPLIED
                        locs    IDREFS #IMPLIED
                        syns    IDREFS #IMPLIED
                        sort    CDATA  #IMPLIED               >
-----------------------------------------------------------------------------*/
static SDLContent EntryContent[] = {
	{ SDL_ENTITY_SIMPLE                , ZeroToN },
	{ CEInitializeMask(SdlElementEntry), ZeroToN },
	{ CEInitializeMask(SdlElementNone) , NULL    },
    };

static SDLElementAttrList  EntryAttrList[] =
    {
	{ SDL_ATTR_ID      ,SdlAttrValueImpliedDef , NULL },
	{ SDL_ATTR_MAIN    ,SdlAttrValueImpliedDef , NULL },
	{ SDL_ATTR_LOCS    ,SdlAttrValueImpliedDef , NULL },
	{ SDL_ATTR_SYNS    ,SdlAttrValueImpliedDef , NULL },
	{ SDL_ATTR_SORT    ,SdlAttrValueImpliedDef , NULL },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- The lophrases (list of phrases) element is generated by the
     higher level DTD to SDL translator.  It is a list of phrases used
     from the toss element content in this document.  The phrases are
     used to map the users knowledge domain into the constructs used
     in SDL.  This information can be used to to pass the phrases
     available for structured/semantic searches to a browser or viewer
     to allow that browser or viewer to offer that information to the
     user.  The "count" attribute of lophrases is the number of phrases
     it contains.  The higher level DTD to SDL translator may precompute
     "count" to enable the processor to preallocate space for the
     phrases.  If "count" is not present, the processor must compute
     the number itself from the document.  -->
<!ELEMENT lophrases - - (phrase+) >
<!ATTLIST lophrases     count     NUMBER             #IMPLIED  >
-----------------------------------------------------------------------------*/
static SDLContent LoPhraseContent[] =
    {
	{ CEInitializeMask(SdlElementPhrase)                  , OneToN },
	{ CEInitializeMask(SdlElementNone)                    , NULL   },
    };

static SDLElementAttrList  LoPhraseAttrList[] =
    {
	{ SDL_ATTR_COUNT, SdlAttrValueImplied, NULL },
	{ -1            , SdlAttrValueBad    , NULL }
    };
/*-----------------------------------------------------------------------------
<!ELEMENT phrase    - O EMPTY     >
<!ATTLIST phrase        text      CDATA              #REQUIRED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  PhraseAttrList[] =
    {
	{ SDL_ATTR_TEXT    , SdlAttrValueRequired, NULL },
	{ -1               ,SdlAttrValueBad        , NULL }
    };

/*-----------------------------------------------------------------------------
<!-- The following element, toss (table of semantics and styles), is
     generated by the higher level DTD to SDL translator.  The "count"
     attribute of toss is the number of styles it contains.  The
     translator may precompute "count" to enable the processor to
     preallocate space for the styles.  If "count" is not present, the
     processor must compute the number itself from the document.  The
     first three attributes of each sub-element are used for lookup in
     the toss.  When formatting an element, the toss is searched for a

     With the exception of the XLFD and Windows typeface name, an
     unspecified attribute implies inheritance.

     The "phrase" attribute is an English (or natural language) phrase
     describing the intended use (semantics) of an element of this
     style.  -->
<!ELEMENT toss      - - (keystyle*,
			headstyle*,
			formstyle*,
			frmtstyle*,
			grphstyle*) >
<!ATTLIST toss          count     NUMBER             #IMPLIED   >
-----------------------------------------------------------------------------*/
static SDLContent TossContent[] = {
	{ CEInitializeMask(SdlElementToss)     , AllocateBlock },
	{ CEInitializeMask(SdlElementKeyStyle) , ZeroToN       },
	{ CEInitializeMask(SdlElementHeadStyle), ZeroToN       },
	{ CEInitializeMask(SdlElementFormStyle), ZeroToN       },
	{ CEInitializeMask(SdlElementFrmtStyle), ZeroToN       },
	{ CEInitializeMask(SdlElementGrphStyle), ZeroToN       },
	{ CEInitializeMask(SdlElementToss)     , CleanUpBlock  },
	{ CEInitializeMask(SdlElementNone)     , NULL          },
    };

static SDLElementAttrList  TossAttrList[] =
    {
	{ SDL_ATTR_COUNT, SdlAttrValueImplied, NULL },
	{ -1            , SdlAttrValueBad    , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT keystyle  - O EMPTY    >
<!ATTLIST keystyle      class     (%key-class;)      #REQUIRED
                        ssi       CDATA              #IMPLIED

                        -- the level of the element being described   --
                        rlevel    NUMBER             #IMPLIED

                        phrase    CDATA              #IMPLIED

                        -- likelihood that this element contains an   --
                        -- actual hit when doing a full text search   --
                        srch-wt   NUMBER             #IMPLIED

                        -- strings to emit on entry and exit from key --
                        enter     CDATA              #IMPLIED
                        exit      CDATA              #IMPLIED

                        %font-styles; >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  KeyStyleAttrList[] =
    {
	{ SDL_ATTR_CLASSK  , SdlAttrValueRequired  , NULL },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL },
	{ SDL_ATTR_RLEVEL  , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_PHRASE  , SdlAttrValueImpliedDef, NULL },
	{ SDL_ATTR_SRCHWT  , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_ENTER   , SdlAttrValueImpliedDef, NULL },
	{ SDL_ATTR_EXIT    , SdlAttrValueImpliedDef, NULL },
	font_stylesAttrList,
	{ -1               , SdlAttrValueBad       , NULL }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT headstyle - O EMPTY    >
<!ATTLIST headstyle     class     (%head-class;)     #REQUIRED
                        ssi       CDATA              #IMPLIED

                        -- the level of the element being described   --
                        rlevel    NUMBER             #IMPLIED

                        phrase    CDATA              #IMPLIED

                        -- likelihood that this element contains an   --
                        -- actual hit when doing a full text search   --
                        srch-wt   NUMBER             #IMPLIED

			-- horizontal orientation of the head with    --
                        -- respect to its associated document,        --
			-- vstruct, virpage, snb, block, form, or p;  --
			-- or of the subhead with respect to its      --
			-- head.                                      --
                        -- procsr shld dflt orient to "center-orient" --
                        orient    (left-orient         |
                                   center-orient       |
                                   right-orient        |
                                   left-margin-orient  |
                                   right-margin-orient |
                                   left-corner-orient  |
                                   right-corner-orient )
                                                     #IMPLIED

			-- vertical orientation of the head or        --
                        -- subhead with respect to its parent.        --
                        -- procsor shld dflt vorient to "top-vorient" --
                        vorient   (top-vorient    |
                                   bottom-vorient |
                                   center-vorient )  #IMPLIED

                        -- This attribute applies to head elements    --
                        -- only, since subheads cannot contain more   --
                        -- subheads.  The attribute determines        --
                        -- whether the vorient attribute applies to   --
                        -- the head only or to the entire head object --
                        -- including its subheads.                    --
                        -- processor shld deflt placement to "parent" --
                        placement (object | parent)  #IMPLIED

                        -- Head width is the percent of the           --
                        -- available space for this element that      --
			-- should be given to its head or the percent --
			-- of the head that should be given to a      --
                        -- subhead.  It is expressed as a fixed point --
                        -- number 1 to 10000 with an implied decimal  --
                        -- point two places to the left of the right  --
                        -- side.                                      --
                        -- processor should default headw to "10000"  --
                        headw      CDATA             #IMPLIED

			-- where to put this head or subhead if it    --
                        -- collides with one already placed.          --
                        -- Horizontal stacking means place this one   --
                        -- under the other.  Vertical stacking means  --
                        -- place this one to the right of the other.  --
                        -- processor should default stack to "vert"   --
                        stack     (horiz | vert)     #IMPLIED

                        -- does the body wrap around the head text?   --
			-- "join" implies starting the content of the --
			-- surrounding element immediatly after this  --
			-- head (i.e., on the same line as the bottom --
			-- of this head).
			-- processor should default flow to "nowrap"  --
                        flow       (wrap   |
				    nowrap |
				    join)           #IMPLIED

                        %font-styles;
                        %format-styles;

		        -- applies to the text in the element, not   --
		        -- the element itself.                       --
		        -- prcsr shld dflt justify to "left-justify" --
		        justify  ( left-justify    |
				   right-justify   |
			           center-justify  |
			           numeric-justify ) #IMPLIED >

-----------------------------------------------------------------------------*/
static SDLElementAttrList  HeadStyleAttrList[] =
    {
	{ SDL_ATTR_CLASSH   , SdlAttrValueRequired  , NULL           },
	{ SDL_ATTR_SSI      , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_RLEVEL   , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_PHRASE   , SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_SRCHWT   , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_ORIENT   , SdlAttrValueImplied   , CenterOrientStr},
	{ SDL_ATTR_VORIENT  , SdlAttrValueImplied   , TopVOrientStr  },
	{ SDL_ATTR_HEADWDTH , SdlAttrValueImplied   , TenThousandStr },
	{ SDL_ATTR_PLACEMENT, SdlAttrValueImpliedDef, ParentStr      },
	{ SDL_ATTR_STACK    , SdlAttrValueImpliedDef, "vert"         },
	{ SDL_ATTR_FLOW     , SdlAttrValueImplied   , NoWrapStr      },
	font_stylesAttrList ,
	formt_stylesAttrList,
	{ SDL_ATTR_JUSTIFY1 , SdlAttrValueImplied   , LeftJustifyStr }, 
	{ -1                , SdlAttrValueBad       , NULL           }
    };

/*-----------------------------------------------------------------------------
<!ELEMENT formstyle - O EMPTY    >
<!ATTLIST formstyle     class     (%format-class;)   #REQUIRED
			ssi       CDATA              #IMPLIED

			-- the level of the element being described   --
			rlevel    NUMBER             #IMPLIED

			phrase    CDATA              #IMPLIED

			-- likelihood that this element contains an   --
			-- actual hit when doing a full text search   --
			srch-wt   NUMBER             #IMPLIED

			-- The widths of the columns in this form,    --
			-- colw, is a space separated list of comma   --
			-- separated integral triples.  If only two   --
			-- comma separated numbers in a triple are    --
			-- given, the second is replicated to make    --
			-- three.  If only one number is given, the   --
			-- other two are assumed to be 0. The first   --
			-- number of a triple is the optimally        --
			-- desired width.  The second number is how   --
			-- much the column is willing to grow         --
			-- relative to the other columns.  The third  --
			-- number is how much the column is willing   --
			-- to shrink relative to the other columns.   --
			-- The numbers are summed and the total is    --
			-- taken as 100 percent of available space.   --
			-- That space is then divided among the       --
			-- columns.  The process of summing and       --
			-- dividing is repeated until all the         --
			-- desired/shrink/grow constraints are met or --
			-- it is known to be impossible to meet them. --
			-- If meeting the constraints is impossible,  --
			-- the column is handled similarly to a       --
			-- graphic that will not fit, e.g., a scroll- --
			-- bar may be added to allow the form to be   --
			-- larger than the size of the viewing area.  --
			-- There should be as many triples as given   --
			-- in the value "ncols" in the "fstyle" sub-  --
			-- element of the form element to which this  --
			-- "formstyle" is being applied.  Extra       --
			-- triples are ignored.  If less than "ncols" --
			-- triples are provided, the last triple is   --
			-- replicated.                                --
			colw      CDATA        	      #IMPLIED

			-- Column justification, taken from one of    --
			-- l | r | c | d (left, right, centered and   --
			-- decimal), separated by spaces, for each    --
			-- column of the form.  Extras are ignored.   --
			-- If there are fewer than the number of      --
			-- columns specified by the "ncols" attribute --
			-- of the "fstyle" subelement of the "form"   --
			-- to which this "formstyle" is being         --
			-- applied, the last value is replicated.     --
			-- The value defaults to "l" (left justified) --
			colj      CDATA	              #IMPLIED

                        %font-styles;
                        %format-styles; >

-----------------------------------------------------------------------------*/
static SDLElementAttrList  FormStyleAttrList[] =
    {
	{ SDL_ATTR_CLASSF  , SdlAttrValueRequired  , NULL           },
	{ SDL_ATTR_SSI     , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_RLEVEL  , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_PHRASE  , SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_SRCHWT  , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_COLW    , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_COLJ    , SdlAttrValueImpliedDef, NULL           },
	font_stylesAttrList,
	formt_stylesAttrList,
	{ -1               , SdlAttrValueBad       , NULL }
    };
/*-----------------------------------------------------------------------------
<!ELEMENT frmtstyle - O EMPTY    >
<!ATTLIST frmtstyle     class     (%format-class;)   #REQUIRED
                        ssi       CDATA              #IMPLIED

                        -- the level of the element being described   --
                        rlevel    NUMBER             #IMPLIED

                        phrase    CDATA              #IMPLIED

			-- first line lmargin may be negative but     --
			-- it's a number used to indicate extension   --
			-- or indentation at start                    --
			-- processor should default f-margin to "0"   --
			f-margin  CDATA              #IMPLIED

                        -- likelihood that this element contains an   --
                        -- actual hit when doing a full text search   --
                        srch-wt   NUMBER             #IMPLIED

                        %font-styles;
                        %format-styles;

		        -- applies to the text in the element, not   --
		        -- the element itself.                       --
		        -- prcsr shld dflt justify to "left-justify" --
		        justify  ( left-justify    |
				   right-justify   |
			           center-justify  |
			           numeric-justify ) #IMPLIED >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  FrmtStyleAttrList[] =
    {
	{ SDL_ATTR_CLASSF   , SdlAttrValueRequired  , NULL           },
	{ SDL_ATTR_SSI      , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_RLEVEL   , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_PHRASE   , SdlAttrValueImplied   , NULL           },
	{ SDL_ATTR_SRCHWT   , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_FMARGIN  , SdlAttrValueImplied   , ZeroStr        },
	font_stylesAttrList ,
	formt_stylesAttrList,
	{ SDL_ATTR_JUSTIFY1 , SdlAttrValueImplied   , LeftJustifyStr }, 
	{ -1                , SdlAttrValueBad       , NULL           }
    };
/*----------------------------------------------------------------------------- <!ELEMENT grphstyle - O EMPTY    >
<!ATTLIST grphstyle     class     (%graphic-class;)  #REQUIRED
			ssi       CDATA              #IMPLIED

			-- the level of the element being described   --
			rlevel    NUMBER             #IMPLIED

			phrase    CDATA              #IMPLIED

			%format-styles;

		        -- applies to the text in the element, not   --
		        -- the element itself.                       --
		        -- prcsr shld dflt justify to "left-justify" --
		        justify  ( left-justify    |
				   right-justify   |
			           center-justify ) #IMPLIED >

] >
-----------------------------------------------------------------------------*/
static SDLElementAttrList  GrphStyleAttrList[] =
    {
	{ SDL_ATTR_CLASSG   , SdlAttrValueRequired  , NULL           },
	{ SDL_ATTR_SSI      , SdlAttrValueImpliedDef, NULL           },
	{ SDL_ATTR_RLEVEL   , SdlAttrValueImplied   , NegativeOneStr },
	{ SDL_ATTR_PHRASE   , SdlAttrValueImplied   , NULL           },
	formt_stylesAttrList,
	{ SDL_ATTR_JUSTIFY2 , SdlAttrValueImplied   , LeftJustifyStr }, 
	{ -1                , SdlAttrValueBad       , NULL           }
    };

static SDLContent CdataContent[] = {
	{ CEInitializeMask(SdlElementCdata), Cdata },
	{ CEInitializeMask(SdlElementNone) , NULL  },
    };

static SDLContent SDLDocumentContent[] = {
	{ CEInitializeMask(SdlElementSgml)   , ZeroOrOne },
	{ CEInitializeMask(SdlElementDocType), ZeroOrOne },
	{ CEInitializeMask(SdlElementSdlDoc) , OnlyOne   },
	{ CEInitializeMask(SdlElementNone)   , NULL      },
    };

/*
 * entries in this structure:
 *    all 'element strings'      must start with '<'
 *    all 'element end strings' must start with '</'
 *
 * If they don't, _DtHelpCeReturnSdlElement will fail
 */
static	SDLElement  SDLElements[] =
  {
    {"<alttext"  , "</alttext" , 3, 3,
			SdlElementAltText     ,
			CEInitializeMask(SdlElementNone),
			NULL                  , AltTextContent  ,
			False, False                 , False                  },

    {AnchorStr   , NULL        , 4, 0,
			SdlElementAnchor      ,
			CEInitializeMask(SdlElementNone),
			AnchorAttrList         , NULL            ,
			False, False           , True                   },

    {AnimateStr  , NULL        , 4, 0,
			SdlElementAnimate     ,
			CEInitializeMask(SdlElementNone),
			NotationAttrList       , NULL            ,
			False, False           , True                   },

    {AudioStr    , NULL        , 3, 0,
			SdlElementAudio       ,
			CEInitializeMask(SdlElementNone),
			NotationAttrList       , NULL            ,
			False, False           , True                   },

    {BlockStr    , "</block"   , 2, 3,
			SdlElementBlock       ,
			CEInitializeMask(SdlElementNone),
			BlockAttrList          , BlockContent    ,
			False, True            , True                   },

    {CallbackStr , NULL        , 3, 0,
			SdlElementCallback    ,
			CEInitializeMask(SdlElementNone),
			CallbackAttrList       , CParaContent    ,
			False, True            , True                   },

    {"<cond"     , "</cond"    , 3, 4,
			SdlElementCond        ,
			CEInitializeMask(SdlElementNone),
			CondAttrList           , CondContent     ,
			False, False           , True                   },

    {CParaStr    , "</cp"      , 3, 3,
			SdlElementCPara       ,
			CEInitializeMask(SdlElementNone),
			ParaAttrList           , CParaContent    ,
			True , True            , True                   },

    {CrossdocStr , NULL        , 3, 0,
			SdlElementCrossDoc    ,
			CEInitializeMask(SdlElementNone),
			IdAndXidAttrList       , NULL            ,
			False, False           , True                   },

    {"<else"     , "</else"    , 3, 4,
			SdlElementElse        ,
			CEInitializeMask(SdlElementNone),
			NULL                   , ThenElseContent ,
			False, False           , False                  },

    {"<entry"    , "</entry"   , 3, 4,
			SdlElementEntry       ,
			CEInitializeMask(SdlElementNone),
			EntryAttrList          , EntryContent    ,
			False, False           , True                   },

    {"<fdata"    , "</fdata"   , 3, 4,
			SdlElementFdata       ,
			CEInitializeMask(SdlElementNone),
			NULL                  , FdataContent    ,
			False, False          , False                  },

/*
 * when one element is a short version of another, put the longer element
 * before the short element so that tests will check it first.
 */
    {"<formstyle", NULL        , 6, 0,
			SdlElementFormStyle   ,
			CEInitializeMask(SdlElementNone),
			FormStyleAttrList     , NULL            ,
			False, False          , True                   },

    {FormStr     , "</form"    , 5, 4,
			SdlElementForm        ,
			CEInitializeMask(SdlElementNone),
			FormAttrList          , FormContent     ,
			False, False          , True                   },

    {"<frmtstyle", NULL        , 4, 0,
			SdlElementFrmtStyle   ,
			CEInitializeMask(SdlElementNone),
			FrmtStyleAttrList     , NULL            ,
			False, False          , True                   },

    {"<frowvec"  , NULL        , 4, 0,
			SdlElementFrowvec     ,
			CEInitializeMask(SdlElementNone),
			FrowvecAttrList       , NULL            ,
			False, False          , True                   },

    {"<fstyle"   , "</fstyle"  , 3, 4,
			SdlElementFstyle      ,
			CEInitializeMask(SdlElementNone),
			FstyleAttrList        , FstyleContent   ,
			False, False          , True                   },

    { GraphicStr , NULL        , 4, 0,
			SdlElementGraphic     ,
			CEInitializeMask(SdlElementNone),
			NotationAttrList      , GraphicContent  ,
			False, False          , True                   },

    {"<grphstyle", NULL        , 4, 0,
			SdlElementGrphStyle   ,
			CEInitializeMask(SdlElementNone),
			GrphStyleAttrList     , NULL            ,
			False, False          , True                   },

/*
 * when one element is a short version of another, put the longer element
 * before the short element so that tests will check it first.
 */
    {"<headstyle", NULL        , 6, 0,
			SdlElementHeadStyle   ,
			CEInitializeMask(SdlElementNone),
			HeadStyleAttrList     , NULL            ,
			False, False          , True                   },

    { HeadStr    , "</head"    , 5, 6,
			SdlElementHead        ,
			CEInitializeMask(SdlElementNone),
			HeadAttrList          , HeadContent     ,
			False, True           , True                   },

    {"<id"       , NULL        , 3, 0,
			SdlElementId          ,
			CEInitializeMask(SdlElementNone),
			IdAttrList            , NULL            ,
			False, False          , True                   },

    {"<if"       , "</if"      , 3, 4,
			SdlElementIf          ,
			CEInitializeMask(SdlElementNone),
			NULL                  , IfContent       ,
			False, False          , False                  },

    {"<index"    , "</index"    , 3, 4,
			SdlElementIndex       ,
			CEInitializeMask(SdlElementNone),
			IndexAttrList         , IndexContent    ,
			False, False          , True                   },

    {"<keystyle" , NULL        , 6, 0,
			SdlElementKeyStyle    ,
			CEInitializeMask(SdlElementNone),
			KeyStyleAttrList      , NULL            ,
			False, False          , True                   },

    {"<key"      , "</key"     , 4, 3,
			SdlElementKey         ,
			CEInitializeMask(SdlElementLink),
			KeyAttrList           , KeyContent      ,
			True , False          , True                   },

    {"<link"     , "</link"    , 3, 4,
			SdlElementLink        ,
			CEInitializeMask(SdlElementLink),
			LinkAttrList          , LinkContent,
			False, False          , True                   },

    {"<loids"    , "</loids"   , 4, 5,
			SdlElementLoids       ,
			CEInitializeMask(SdlElementNone),
			LoidsAttrList         , LoidsContent,
			False, False          , True                   },

    {"<lophrases", "</lophrases", 4, 5,
			SdlElementLoPhrases   ,
			CEInitializeMask(SdlElementNone),
			LoPhraseAttrList      , LoPhraseContent,
			False, False          , True                   },

    {ManpageStr  , NULL        , 2, 0,
			SdlElementManPage     ,
			CEInitializeMask(SdlElementNone),
			IdAndXidAttrList      , NULL        ,
			False, False          , True                   },

    {"<notes"    , NULL        , 2, 0,
			SdlElementNotes       ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , False                  },

    {"<phrase"   , NULL        , 3, 0,
			SdlElementPhrase      ,
			CEInitializeMask(SdlElementNone),
			PhraseAttrList        , NULL       ,
			False, False          , True                   },

    {ParaStr     , "</p"       , 2, 3,
			SdlElementPara        ,
			CEInitializeMask(SdlElementNone),
			ParaAttrList          , ParaContent,
			True , True           , True                   },

    {"<refitem"  , "</refitem" , 4, 5,
			SdlElementRefItem     ,
			CEInitializeMask(SdlElementNone),
			RefItemAttrList       , RefItemContent,
			False, False          , True                   },

    {"<rel-docs" , NULL        , 6, 0,
			SdlElementRelDocs     ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , False                  },

    {"<rel-file" , NULL        , 6, 0,
			SdlElementRelFile     ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , False                  },

    {"<rev"      , "</rev"     , 4, 5,
			SdlElementRev         ,
			CEInitializeMask(SdlElementRev),
			NULL                  , RevContent    ,
			True , False          , False                  },

    {ScriptStr   , "</script"  , 3, 4,
			SdlElementScript      ,
			CEInitializeMask(SdlElementNone),
			ScriptAttrList        , ScriptContent ,
			False, False          , True                   },

    {"<sdldoc"   , "</sdldoc"  , 3, 4,
			SdlElementSdlDoc      ,
			CEInitializeMask(SdlElementNone),
			SdlDocAttrList        , SdlDocContent ,
			False, False          , True                   },

    {"<snb"      , "</snb"     , 4, 5,
			SdlElementSnb         ,
			CEInitializeMask(SdlElementNone),
			SnbAttrList           , SnbContent    ,
			False, False          , True                   },

    {SnrefStr    , "</snref"   , 4, 5,
			SdlElementSnRef       ,
			CEInitializeMask(SdlElementNone),
			SnrefAttrList         , SnrefContent  ,
			False, False          , True                   },

    {"<spc"      , NULL        , 4, 0,
			SdlElementSpc         ,
			CEInitializeMask(SdlElementNone),
			SpcAttrList           , SpcContent    ,
			False, False          , True                   },

    {"<sphrase"  , "</sphrase>", 4, 4,
			SdlElementSphrase     ,
			CEInitializeMask(SdlElementNone),
			SphraseAttrList       , SphraseContent ,
			False, False          , True                   },

    { SubHeadStr , "</subhead" , 3, 4,
			SdlElementSubHead     ,
			CEInitializeMask(SdlElementNone),
			HeadAttrList          , SubHeadContent,
			False, True           , True                   },

    { SwitchStr  , "</switch"  , 3, 4,
			SdlElementSwitch      ,
			CEInitializeMask(SdlElementNone),
			SwitchAttrList        , SwitchContent ,
			False, False          , True                   },

    {SyscmdStr   , NULL        , 3, 0,
			SdlElementSysCmd      ,
			CEInitializeMask(SdlElementNone),
			SysCmdAttrList        , NULL          ,
			False, False          , True                   },

    {TextfileStr , NULL        , 6, 0,
			SdlElementTextFile    ,
			CEInitializeMask(SdlElementNone),
			TextFileAttrList      , NULL          ,
			False, False          , True                   },

    {TextStr     , "</text"    , 4, 4,
			SdlElementText        ,
			CEInitializeMask(SdlElementNone),
			TextAttrList          , TextContent   ,
			False, False          , True                   },

    {"<then"     , "</then"    , 3, 4,
			SdlElementThen        ,
			CEInitializeMask(SdlElementNone),
			NULL                  , ThenElseContent,
			False, False          , False                  },

    {"<toss"     , "</toss"    , 3, 4,
			SdlElementToss        ,
			CEInitializeMask(SdlElementNone),
			TossAttrList          , TossContent   ,
			False, False          , True                   },

    {VideoStr    , NULL        , 4, 0,
			SdlElementVideo       ,
			CEInitializeMask(SdlElementNone),
			NotationAttrList      , NULL          ,
			False, False          , True                   },

    {VirpageStr  , "</virpage" , 4, 4,
			SdlElementVirpage     ,
			CEInitializeMask(SdlElementNone),
			VirpageAttrList       , VirpageContent,
			False, True           , True                   },

    {VirpageStr  , "</virpage" , 4, 4,
			SdlElementTitle     ,
			CEInitializeMask(SdlElementNone),
			VirpageAttrList       , VirpageHeadContent,
			False, True           , True                   },

    {"<vstruct"  , "</vstruct" , 3, 4,
			SdlElementVStruct     ,
			CEInitializeMask(SdlElementNone),
			VStructAttrList       , VStructContent,
			False, False          , True                   },

    {"<!--"      , NULL        , 3, 0,
			SdlElementComment     ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , True                   },

    {"<!doctype" , NULL        , 3, 0,
			SdlElementDocType     ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , True                   },

    {"<!sgml"    , NULL        , 3, 0,
			SdlElementSgml        ,
			CEInitializeMask(SdlElementNone),
			NULL                  , NULL          ,
			False, False          , True                   },

    { NULL       , NULL        , 0, 0,
			SdlElementCdata       ,
			CEInitializeMask(SdlElementNone),
			NULL                  , CdataContent  ,
			False, False          , False                  },
  };

static	int     MaxSDLElements = sizeof (SDLElements) / sizeof (SDLElement);

static const CESDLAttrStruct	DefaultAttributes =
    {			/* CESDLAttrStruct */
	 0,			/* long         enum_values;       */
	 0,			/* long         num_values;        */
	 0,			/* long         str1_values;       */
	 0,			/* long         str2_values;       */
	SdlClassText,		/* enum SdlOption	clan;      */
	SdlBoolNo,		/* enum SdlOption	hdr;       */
	SdlAppAll,		/* enum SdlOption	app;       */
	SdlTimingSync,		/* enum SdlOption	timing;    */
	SdlTypeDynamic,		/* enum SdlOption	type;      */
	SdlJustifyCenter,	/* enum SdlOption	orient;    */
	SdlJustifyTop,		/* enum SdlOption	vorient;   */
	SdlPlaceParent,		/* enum SdlOption	placement; */
	SdlStackVert,		/* enum SdlOption	stacking;  */
	SdlNoWrap,		/* enum SdlOption	flow;      */
	SdlWindowCurrent,	/* enum SdlOption	window;    */
	SdlTraversalNoReturn,	/* enum SdlOption	traversal; */
	SdlInterpKsh,        	/* enum SdlOption	interp;    */
	-1,			/* SDLLevel	level/rlevel;      */
			/* SDLNumVals Union */
	-1,			/* SDLNumber	srch_wt; */
	 0,			/* SDLNumber	length/ncols/offset/count;*/
			/* SDLIdVals Union */
	NULL,			/* SDLId   id/name/rid/cells/frst_pg;*/
			/* End SDLIdVals Union */
			/* SDLssiVals Union */
	NULL,			/* SDLCdata ssi/rssi */
			/* End SDLssiVals Union */
	NULL,			/* SDLCdata	version; */
	"C",			/* SDLCdata	language; */
	(char *)IsoStr,		/* SDLCdata	charset /product  /product  */
	NULL,			/* SDLCdata	doc_id; */
			/* SDLAttrStrings Union */
				/*		el_strs / doc_strs/not_strs */
	NULL,			/* SDLCdata	colw    /product  /product  */
	NULL,			/* SDLCdata	colj    /license  /license  */
	NULL,			/* SDLCdata	text    /timestamp/format   */
	NULL,			/* SDLCdata	branches/prodpn   /method   */
	NULL,			/* SDLCdata	abbrev  /prodver  /xid      */
	NULL,			/* SDLCdata	enter   /author   /command  */
	NULL,			/* SDLCdata	exit    /sdldtd   /data     */
	NULL,			/* SDLCdata	        /srcdtd   /         */
	NULL,			/* SDLCdata	        /pub_id   /         */
			/* End SDLAttrStrings Union */
	NULL,			/* SDLCdata	phrase; */
	(char *)TenThousandStr,	/* SDLCdata	headw; */
	NULL,			/* SDLCdata	button; */
	NULL,			/* SDLCdata	descript; */
	NULL,			/* SDLCdata	linkinfo; */
	{			/* CanvasFontSpec	font_specs; */
	    10,			    /* int	 pointsz;   */
	    10,			    /* int	 set_width  */
	    NULL,		    /* char	*color;     */
	    NULL,		    /* char	*xlfd;      */
	    NULL,		    /* char	*xlfdb;     */
	    NULL,		    /* char	*xlfdi;     */
	    NULL,		    /* char	*xlfdib;    */
	    NULL,		    /* char	*typenam;   */
	    NULL,		    /* char	*typenamb;  */
	    NULL,		    /* char	*typenami;  */
	    NULL,		    /* char	*typenamib; */
	    CEFontStyleSanSerif,    /* char	*style;     */
	    CEFontSpacingMono,	    /* char	*spacing;   */
	    CEFontWeightMedium,	    /* char	*weight;    */
	    CEFontSlantRoman,	    /* char	*slant;     */
	    CEFontSpecialNone,	    /* char	*special;   */
	    NULL,		    /* void	*expand;    */
	},
	{			/* CEFrmtSpecs	frmt_specs; */
	    SdlBorderNone,	    /* enum SdlOption	border;  */
	    SdlJustifyLeft,	    /* enum SdlOption	justify; */
	    SdlJustifyTop,	    /* enum SdlOption	vjust;   */
	     0,			    /* SDLNumber	fmargin; */
	     0,			    /* SDLNumber	lmargin; */
	     0,			    /* SDLNumber	rmargin; */
	     0,			    /* SDLNumber	tmargin; */
	     0,			    /* SDLNumber	bmargin; */
	},
	{			/* CEFrmtInfo	frmt_info; */
	     0,			    /* Unit		y_units; */
	     0,			    /* Unit		x_units; */
	     0,			    /* int		beg_txt; */
	     0,			    /* int		end_txt; */
	    -1,			    /* int		beg_gr;  */
	    -1,			    /* int		end_gr;  */
	},
    };

static CESDLAttrStruct	DefAttrStruct;


/******************************************************************************
 * Private Macros
 *****************************************************************************/
#define	MyFree(x)	if ((char *)(x) != NULL) free(x)
#define	SaveRestoreMask(x,y) \
		{ register int myI;\
		  for (myI = 0; myI < SDL_MASK_LEN; myI++) \
		      x[myI] = y[myI]; \
		}
#define	MergeMasks(x,y) \
		{ register int myI;\
		  for (myI = 0; myI < SDL_MASK_LEN; myI++) \
		      x[myI] |= y[myI]; \
		}
#define	RemoveMasks(x,y) \
		{ register int myI;\
		  for (myI = 0; myI < SDL_MASK_LEN; myI++) \
		      x[myI] &= (~(y[myI])); \
		}
#define	MaskToValue(x,y) \
		{ register int myI;\
		  register SDLMask myMask;\
		  for (myI = 0, y = 0; myI < SDL_MASK_LEN; myI++) \
		      if (x[myI] == 0) \
		          y += SDL_MASK_SIZE; \
		      else \
			{ \
			  myMask = x[myI]; \
			  while (myMask > 1) \
			    { \
				myMask = myMask >> 1; y++; \
			    } \
			  myI = SDL_MASK_LEN; \
			} \
		}
#define	ClearAttrFlag(pa, attr) \
		{ \
		  int  mask = attr & VALUE_MASK; \
		  int  flag = ~(attr & (~VALUE_MASK)); \
 \
		  if (mask == ENUM_VALUE) \
		      (pa).enum_values = (pa).enum_values & flag; \
		  else if (mask == NUMBER_VALUE) \
		      (pa).num_values = (pa).num_values & flag; \
		  else if (mask == STRING1_VALUE) \
		      (pa).str1_values = (pa).str1_values & flag; \
		  else if (mask == STRING2_VALUE) \
		      (pa).str2_values = (pa).str2_values & flag; \
		}

/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    static int ProcessString(string, int idx);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessString(my_struct, last_flag, nl_flag, mod_str, cpy_str, process_flag,
		string,  byte_len, idx);
    _CEFormatStruct	 *my_struct;
    CEBoolean		  last_flag;
    CEBoolean		  nl_flag;
    CEBoolean		  mod_str;
    CEBoolean		  cpy_str;
    CEBoolean		  process_flag;
    char		 *string;
    int			  byte_len;
    int			 *idx;
#else
ProcessString(
    _CEFormatStruct	 *my_struct,
    CEBoolean		  last_flag,
    CEBoolean		  nl_flag,
    CEBoolean		  mod_str,
    CEBoolean		  cpy_str,
    CEBoolean		  process_flag,
    char		 *string,
    int			  byte_len,
    int			 *idx)
#endif /* _NO_PROTO */
{
    if (mod_str == True)
        string[*idx] = '\0';

    if (process_flag == True && MySaveString(&(my_struct->seg_list),
				    my_struct, string, my_struct->cur_link,
                                    byte_len, nl_flag) != 0)
      {
        MyFree(string);
        return -1;
      }

    my_struct->last_was_space = last_flag;

    if (cpy_str == True)
      {
        strcpy (string, &string[*idx+1]);
        *idx = -1;
      }
    return 0;
}

/******************************************************************************
 * Function:    static int ProcessNonBreakChar(string, int idx);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static  int
#ifdef _NO_PROTO
ProcessNonBreakChar(my_struct, process_flag, break_str, string,  byte_len, idx);
    _CEFormatStruct      *my_struct;
    CEBoolean             process_flag;
    char                 *break_str;
    char                 *string;
    int                   byte_len;
    int                  *idx;
#else
ProcessNonBreakChar(
    _CEFormatStruct      *my_struct,
    CEBoolean             process_flag,
    char                 *break_str,
    char                 *string,
    int                   byte_len,
    int                  *idx)
#endif /* _NO_PROTO */
{
    if (ProcessString(my_struct, False, False, True, False, process_flag,
						string, byte_len, idx) != 0)
        return -1;

    my_struct->flags = _CESetNonBreakFlag(my_struct->flags);
    if (ProcessString(my_struct, False, False, False, False, process_flag,
						break_str, byte_len, idx) != 0)
        return -1;

    my_struct->flags = _CEClearBreakFlag(my_struct->flags);
    strcpy (string, &string[*idx+1]);
    *idx = -1;
    return 0;
}

/******************************************************************************
 * Function:    static int MoveString(string, int idx);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
MoveString(string,  max_len, idx);
    char	**string;
    int		 *max_len;
    int		 *idx;
#else
MoveString(
    char	**string,
    int		 *max_len,
    int		 *idx)
#endif /* _NO_PROTO */
{
    int    i     = *idx;
    int    myLen = *max_len;
    char  *src   = *string;

    i--;
    if (i == -1)
      {
        if (strlen(src) == myLen)
          {
	    src = (char *) realloc (src, myLen + 2);
	    if (src == NULL)
	        return -1;

	    myLen++;
            *string  = src;
            *max_len = myLen;
          }
        for (i = myLen; i > 0; i--)
	    src[i] = src[i-1];
      }

    *idx = i;
    return 0;
}

/******************************************************************************
 * Function:    int SetUp (VolumeHandle volume,
 *				CESDLVolume **sdl_vol,
 *				_CEFormatStruct *frmt_struct,
 *				CESDLAttrStruct *attr_struct,
 *				CESegment *toss,
 *				CEBoolean lang
 *
 * Parameters:
 *		volume		Specifies the volume handle. If non-NULL,
 *				create and return the sdl volume pointer
 *				in 'sdl_vol'.
 *		sdl_vol		If not NULL, returns the sdl volume pointer.
 *		frmt_struct	Specifies the formatting structure to use.
 *		attr_struct	Specifies the attributes structure to use.
 *		toss		Specifies the toss to use. If NULL and
 *				sdl_vol asked for and flag == True, the
 *				sdl_vol->toss will be used.
 *		lang		Specifies if the language/charset should
 *				be set using information from the volume.
 *		flag		Specifies if to get/use the sdl_vol->toss
 *				if toss is NULL.
 *
 * Returns:	0 if no failures, -1 if errors.
 *
 * Purpose:     Set up the formatting structure to use.
 *
 *****************************************************************************/
static	int
#ifdef _NO_PROTO
SetUp (volume, sdl_vol, frmt_struct, attr_struct, toss, fd, lang, flag)
    VolumeHandle	  volume;
    CESDLVolume		**sdl_vol;
    _CEFormatStruct	 *frmt_struct;
    CESDLAttrStruct	 *attr_struct;
    CESegment		 *toss;
    int			  fd;
    CEBoolean		  lang;
    CEBoolean		  flag;
#else
SetUp (
    VolumeHandle	  volume,
    CESDLVolume		**sdl_vol,
    _CEFormatStruct	 *frmt_struct,
    CESDLAttrStruct	 *attr_struct,
    CESegment		 *toss,
    int			  fd,
    CEBoolean		  lang,
    CEBoolean		  flag)
#endif /* _NO_PROTO */
{
    *attr_struct           = DefaultAttributes;
    *frmt_struct           = defaultFormatStruct;

    if (volume != NULL && sdl_vol != NULL)
      {
        *sdl_vol = _DtHelpCeGetSdlVolumePtr(volume);
	if (*sdl_vol == NULL)
	    return -1;
      }

    if (volume != NULL)
      {
	if (toss == NULL && flag == True)
	    toss = _DtHelpCeGetSdlVolToss(volume, fd);

        if (lang == True)
          {
	    attr_struct->language = _DtHelpCeGetSdlVolLanguage(volume);
	    attr_struct->charset  = (char *) _DtHelpCeGetSdlVolCharSet(volume);
          }
      }

    /*
     * determine mb_len should be based on lang/charset.
     */
    frmt_struct->mb_len   = _DtHelpCeGetMbLen(attr_struct->language,
						attr_struct->charset);
    frmt_struct->toss     = toss;
    frmt_struct->cur_attr = attr_struct;

    return 0;
}

/******************************************************************************
 * Function:    void AddToAsyncList (CESegment *seg_list)
 *
 * Parameters:
 *
 * Returns      Nothing
 *
 * errno Values:
 *
 * Purpose:     Add a segment block to the async list.
 *
 *****************************************************************************/
static  void
#ifdef _NO_PROTO
AddToAsyncList (my_struct, block_seg)
    _CEFormatStruct	*my_struct;
    CESegment		*block_seg;
#else
AddToAsyncList (
    _CEFormatStruct	*my_struct,
    CESegment		*block_seg)
#endif /* _NO_PROTO */
{

    if (my_struct->async_blks != NULL)
      {
        CESegment  *pSeg = my_struct->async_blks;

	while (pSeg->next_seg != NULL)
	    pSeg = pSeg->next_seg;
	
        pSeg->next_seg = block_seg;
      }
    else
	my_struct->async_blks = block_seg;

}

/******************************************************************************
 * Function:    int CheckOptionList (int attr_value_type, const char *attr,
 *					CEBoolean check_flag,
 *					enum SdlOption cur_num,
 *					enum SdlOption *num);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     If found, returns the enum value.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CheckOptionList(attr_value_type, attr, check, cur_num, num)
    int			 attr_value_type;
    const char		*attr;
    CEBoolean		 check;
    enum SdlOption	 cur_num;
    enum SdlOption	*num;
#else
CheckOptionList(
    int			 attr_value_type,
    const char		*attr,
    CEBoolean		 check,
    enum SdlOption	 cur_num,
    enum SdlOption	*num)
#endif /* _NO_PROTO */
{
    int    i = 0;
    int    lowerChar;
    int    result = -1;
    const OptionList *option;
    CEBoolean	found  = False;
    CEBoolean	cmpFnd = False;

    while (SDLOptionsList[i].attr_value != ULONG_MAX &&
			SDLOptionsList[i].attr_value != attr_value_type)
	i++;

    if (SDLOptionsList[i].attr_value != ULONG_MAX)
      {
	option    = SDLOptionsList[i].options;
	lowerChar = tolower((int)(*attr));

	while (option != NULL && option->string != NULL &&
					(check == True || cmpFnd == False))
	  {
	    /*
	     * check to see if the current option is in the list
	     * of valid options for this attribute.
	     */
	    if (check && cur_num == option->option_value)
	      {
		/*
		 * okay, it's been found, don't check anymore.
		 */
		found = True;
		check = False;
	      }

	    /*
	     * otherwise, find out if this value is a valid option
	     * for the attribute.
	     */
	    else if (cmpFnd == False &&
			lowerChar == tolower((int)(*(option->string))) &&
				_DtHelpCeStrCaseCmp(option->string, attr) == 0)
	      {
		cmpFnd = True;
		*num   = option->option_value;
	      }
	    option++;
	  }
      }

    /*
     * if found is true, means check was originally set to true and
     * we found the current value in the option list for this attribute.
     */
    if (found == True)
	*num = cur_num;

    /*
     * if we found that the current value was valid or that the string
     * was a valid option in the list for the attribute, return no error.
     */
    if (found == True || cmpFnd == True)
	result = 0;

    return result;
}

/******************************************************************************
 * Function:    int CheckFontList (int attr_value_type, char *attr,
 *					enum CEFontSpec *num);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     If found, returns the enum value.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CheckFontList(attr_value_type, attr, num)
    int			 attr_value_type;
    char		*attr;
    enum CEFontSpecValue *num;
#else
CheckFontList(
    int			 attr_value_type,
    char		*attr,
    enum CEFontSpecValue *num)
#endif /* _NO_PROTO */
{
    int    i = 0;
    const FontSpecOption *option;

    while (SDLFontList[i].attr_value != ULONG_MAX &&
			SDLFontList[i].attr_value != attr_value_type)
	i++;

    if (SDLFontList[i].attr_value != ULONG_MAX)
      {
	option = SDLFontList[i].options;

	while (option != NULL && option->string != NULL)
	  {
	    if (_DtHelpCeStrCaseCmp(option->string, attr) == 0)
	      {
		*num = option->option_value;
		return 0;
	      }
	    option++;
	  }
      }
    return -1;
}

/******************************************************************************
 * Function:    int AllocateElement (CEElement **el_ptr);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Allocates an memory
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
AllocateElement(my_struct, el_value, el_ptr)
    _CEFormatStruct	*my_struct;
    enum SdlElement	 el_value;
    CEElement	**el_ptr;
#else
AllocateElement(
    _CEFormatStruct	*my_struct,
    enum SdlElement	 el_value,
    CEElement	**el_ptr)
#endif /* _NO_PROTO */
{
    CEElement *newEl;

    if (_DtHelpCeAllocElement(el_value, my_struct->cur_attr, &newEl) != 0)
        return -1;

    /*
     * now set current attributes to this attributes
     */
    if (el_value != SdlElementIf && el_value != SdlElementCond &&
		el_value != SdlElementThen && el_value != SdlElementElse)
        my_struct->cur_attr = &(newEl->attributes);

    *el_ptr = newEl;
    return 0;
}

/******************************************************************************
 * Function:    int LookAhead (_CEFormatStruct my_struct, int token_value,
 *				int remove_flag);
 *
 * Parameters:
 *		my_struct	Specifies the parsing structure.
 *		token_value	Specifies the token to match
 *		remove_flag	Specifies whether to clear the look ahead
 *				value. If True, will set the look ahead
 *				parsed value to invalid iff token_value
 *				equals the parsed look ahead value.
 *
 * Returns:      0 if token_value matched parsed value,
 *		 1 if parsed value is invalid.
 *		-1 if parsed value is valid but token_value did not match.
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
TestLookAhead(my_struct, token_value, end_flag, remove_flag)
    _CEFormatStruct	*my_struct;
    enum SdlElement	 token_value;
    CEBoolean		 end_flag;
    int			 remove_flag;
#else
TestLookAhead(
    _CEFormatStruct	*my_struct,
    enum SdlElement	 token_value,
    CEBoolean		 end_flag,
    int			 remove_flag)
#endif /* _NO_PROTO */
{
    if (my_struct->parsed == SdlElementNone)
	return 1;

    if (my_struct->parsed == token_value && my_struct->end_flag == end_flag)
      {
	if (remove_flag == True)
	  {
	    my_struct->parsed = SdlElementNone;
	    if (my_struct->remember != NULL)
	      {
		free (my_struct->remember);
		my_struct->remember = NULL;
	      }
	  }
	return 0;
      }

    return -1;
}

/******************************************************************************
 * Function:    int MatchSDLElement (_CEFormatStruct my_struct, sdl_string,
 *					int sdl_element, sig_chars)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the specific element.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
MatchSDLElement(my_struct, sdl_string, sdl_element, sig_chars, end_flag)
    _CEFormatStruct	*my_struct;
    const char		*sdl_string;
    enum SdlElement	 sdl_element;
    int			 sig_chars;
    CEBoolean		 end_flag;
#else
MatchSDLElement(
    _CEFormatStruct	*my_struct,
    const char		*sdl_string,
    enum SdlElement	 sdl_element,
    int			 sig_chars,
    CEBoolean		 end_flag)
#endif /* _NO_PROTO */
{
    int    i;

    i = TestLookAhead(my_struct, sdl_element, end_flag, True);

    if ( i != 1)
	return i;

    return (_DtHelpCeMatchSdlElement(my_struct->my_file,sdl_string,sig_chars));
}

/******************************************************************************
 * Function:    void SetAttributeFlag (
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
SetAttributeFlag(cur_attr, attr_define)
    CESDLAttrStruct	*cur_attr;
    unsigned long	 attr_define;
#else
SetAttributeFlag(
    CESDLAttrStruct	*cur_attr,
    unsigned long	 attr_define)
#endif /* _NO_PROTO */
{
    unsigned long  flag = attr_define & ~(VALUE_MASK);

    switch (attr_define & VALUE_MASK)
      {
	case ENUM_VALUE:
		cur_attr->enum_values |= flag;
		break;
	case STRING1_VALUE:
		cur_attr->str1_values |= flag;
		break;
	case STRING2_VALUE:
		cur_attr->str2_values |= flag;
		break;
	case NUMBER_VALUE:
		cur_attr->num_values |= flag;
		break;

      }
}

/******************************************************************************
 * Function:    void InitializeAttributes(CESDLAttrStruct *as,
 *						SDLElementAttrList *attr_list)
 *
 * Parameters:
 *
 * Returns:     Attribute Processed if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page beginning.
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
InitializeAttributes(as, attr_list)
    CESDLAttrStruct	*as;
    const SDLElementAttrList	*attr_list;
#else
InitializeAttributes(
    CESDLAttrStruct	*as,
    const SDLElementAttrList	*attr_list)
#endif /* _NO_PROTO */
{
   char      **strPtr1;
   void      *varOffset1;
   SDLNumber *numPtr1;
   enum SdlOption   *enumPtr1;
   enum SdlOption    defNum = SdlOptionBad;
   const SDLAttribute *pAttr;

   while (attr_list->sdl_attr_define != -1)
     {
	if (attr_list->def_string != NULL ||
				attr_list->sdl_value == SdlAttrValueImpliedDef)
	  {
	    pAttr = SDLAttributeList;
	    while (pAttr->data_type != SdlAttrDataTypeInvalid &&
		    !(ATTRS_EQUAL(pAttr->data_type,
			pAttr->sdl_attr_define, attr_list->sdl_attr_define)))
		pAttr++;

	    if (pAttr->data_type != SdlAttrDataTypeInvalid &&
			_CEIsAttrSet(*as, pAttr->sdl_attr_define) == False)
	      {
	        varOffset1 = ((char *) as) + pAttr->field_ptr;

	        if (pAttr->data_type == SdlAttrDataTypeNumber)
	          {
		    numPtr1  = (SDLNumber *) varOffset1;
	            *numPtr1 = ((SDLNumber)atoi(attr_list->def_string));
	          }
	        else if (pAttr->data_type == SdlAttrDataTypeEnum)
	          {
		    enumPtr1  = (enum SdlOption *) varOffset1;
		    (void) CheckOptionList (attr_list->sdl_attr_define,
					attr_list->def_string,
		((attr_list->sdl_attr_define & SDL_ATTR_CLASS) ? True : False),
					as->clan,
					&defNum);
		    if ((attr_list->sdl_attr_define & SDL_ATTR_CLASS) &&
				as->clan != defNum)
			SetAttributeFlag(as, attr_list->sdl_attr_define);
		    *enumPtr1 = defNum;
	          }
	        else if (_CEIsString1(pAttr->sdl_attr_define)
					||
			 _CEIsString2(pAttr->sdl_attr_define))
	          {
		    strPtr1  = (char **) varOffset1;
	            *strPtr1 = (char *) attr_list->def_string;
	          }
	      }
	  }
       attr_list++;
     }
}

/******************************************************************************
 * Function:    int ProcessSDLAttribute(_CEFormatStruct my_struct,
 *						SDLAttrStruct *cur_attr,
 *						SDLElementAttrList *attr_list,
 *						char     *attr_name,
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
static	int
#ifdef _NO_PROTO
ProcessSDLAttribute(my_struct, cur_attr, attr_list, attr_name)
    _CEFormatStruct	*my_struct;
    CESDLAttrStruct	*cur_attr;
    const SDLElementAttrList	*attr_list;
    char		*attr_name;
#else
ProcessSDLAttribute(
    _CEFormatStruct	*my_struct,
    CESDLAttrStruct	*cur_attr,
    const SDLElementAttrList	*attr_list,
    char		*attr_name)
#endif /* _NO_PROTO */
{
    int          result;
    char	*attrValue;
    char	**strPtr;
    char	 numBuf[DTD_NAMELEN+1];
    void	*varOffset1;
    SDLNumber   *numPtr1;
    enum SdlOption     *enumPtr1;
    enum SdlOption	 attrNum;
    enum CEFontSpecValue  fontNum;
    enum CEFontSpecValue *fontPtr1;
    const SDLAttribute *pAttr = SDLAttributeList;

    /*
     * find the attribute in the list of SDL attributes
     * assumes that 'attr_name' has been lower cased, since all the
     * attribute names in SDLAttributeList are already lower case.
     */
    while (pAttr->data_type != SdlAttrDataTypeInvalid &&
	      (*(pAttr->name) != *attr_name || strcmp(pAttr->name, attr_name)))
	pAttr++;

    if (pAttr->data_type != SdlAttrDataTypeInvalid)
      {

        /*
         * Check to see if this element contains an attribute with
         * the found value.
         */
	while (attr_list->sdl_attr_define != -1 &&
			!ATTRS_EQUAL(pAttr->data_type, pAttr->sdl_attr_define,
						attr_list->sdl_attr_define))
	    attr_list++;

        if (attr_list->sdl_attr_define == -1)
	    return -1;
      }

    /*
     * check it against the ones that are allowed for this element
     */
    if (pAttr->data_type != SdlAttrDataTypeInvalid)
      {
	switch (pAttr->data_type)
	  {
	    case SdlAttrDataTypeId:
		    result = _DtHelpCeGetSdlId(my_struct->my_file, &attrValue);
		    break;

	    case SdlAttrDataTypeNumber:
		    result = _DtHelpCeGetSdlNumber(my_struct->my_file, numBuf);
		    break;

	    case SdlAttrDataTypeCdata:
		    result = _DtHelpCeGetSdlAttributeCdata(my_struct->my_file,
						    False, &attrValue);
		    break;

	    case SdlAttrDataTypeFont:
		    result = _DtHelpCeGetSdlAttributeCdata(my_struct->my_file,
						    False, &attrValue);
		    if (result != -1)
			result = CheckFontList (attr_list->sdl_attr_define,
						     attrValue, &fontNum);
		    break;

	    case SdlAttrDataTypeEnum:
		    result = _DtHelpCeGetSdlAttributeCdata(my_struct->my_file,
						    False, &attrValue);
		    if (result != -1)
			result = CheckOptionList (attr_list->sdl_attr_define,
						    attrValue, False,
						    SdlOptionBad, &attrNum);
		    break;

	    default:
		    printf ("hit unknown in processing attribute\n");
		    result = -1;
		    break;
	  }

	if (result == 0)
	  {
	    varOffset1 = ((char *) cur_attr) + pAttr->field_ptr;
	    if (pAttr->data_type == SdlAttrDataTypeNumber)
	      {
		numPtr1  = (SDLNumber *) varOffset1;
		*numPtr1 = ((SDLNumber) atoi(numBuf));
	      }
	    else if (pAttr->data_type == SdlAttrDataTypeEnum)
	      {
		enumPtr1  = (enum SdlOption *) varOffset1;
		*enumPtr1 = attrNum;

		free(attrValue);
	      }
	    else if (pAttr->data_type == SdlAttrDataTypeFont)
	      {
		fontPtr1  = (enum CEFontSpecValue *) varOffset1;
		*fontPtr1 = fontNum;

		free(attrValue);
	      }
	    else
	      {
		strPtr = (char **) varOffset1;
		*strPtr = attrValue;
	      }
	    SetAttributeFlag(cur_attr, pAttr->sdl_attr_define);
	  }
	return result;
      }

    return -1;
}

/******************************************************************************
 * Function:    int VerifyAttrList (
 *						SDLElementAttrList *attr_list)
 *
 * Parameters:
 *
 * Returns:     Attribute Processed if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page beginning.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
VerifyAttrList(cur_attr, attr_list)
    CESDLAttrStruct	*cur_attr;
    const SDLElementAttrList	*attr_list;
#else
VerifyAttrList(
    CESDLAttrStruct	*cur_attr,
    const SDLElementAttrList	*attr_list)
#endif /* _NO_PROTO */
{
    long  value;
    while (attr_list->sdl_attr_define != -1)
      {
	if (attr_list->sdl_value == SdlAttrValueRequired)
	  {
	    switch (attr_list->sdl_attr_define & VALUE_MASK)
	      {
		case ENUM_VALUE:
				/*
				 * mask off the minor number
				 */
				value = cur_attr->enum_values & ~(MINOR_MASK);
				break;
		case STRING1_VALUE:
				value = cur_attr->str1_values;
				break;
		case STRING2_VALUE:
				value = cur_attr->str2_values;
				break;
		case NUMBER_VALUE:
				value = cur_attr->num_values;
				break;
	      }
	    if (!(attr_list->sdl_attr_define & value))
		return -1;
	  }
	attr_list++;
      }

    return 0;
}

/******************************************************************************
 * Function:    void MergeTossInfo (
 *				CEElement *cur_element,
 *				_CEFormatStruct my_struct);
 *
 * Parameters:
 *
 * Returns:     nothing
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page beginning.
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
MergeTossInfo(cur_element, my_struct)
    CEElement	*cur_element;
    _CEFormatStruct	*my_struct;
#else
MergeTossInfo(
    CEElement	*cur_element,
    _CEFormatStruct	*my_struct)
#endif /* _NO_PROTO */
{
    CEElement *sElement;

    if (my_struct->toss == NULL)
	return;

/*
    sElement = MatchSemanticStyle (my_struct->toss,
*/
    sElement = _DtHelpCeMatchSemanticStyle (my_struct->toss,
				cur_element->attributes.clan,
				my_struct->cur_attr->lev.level,
				SSIString(cur_element));
    _DtHelpCeMergeSdlAttribInfo(sElement, cur_element);

    return;
}


/*------------------------- Element Processing ------------------------------*/
/******************************************************************************
 * Function:    int ParseElementStart (
 *				_CEFormatStruct my_struct,
 *				int element, CEBoolean process_flag);
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
static	int
#ifdef _NO_PROTO
ParseElementStart(my_struct, element, process_flag)
    _CEFormatStruct	*my_struct;
    SDLElement		 element;
    CEBoolean		 process_flag;
#else
ParseElementStart(
    _CEFormatStruct	*my_struct,
    SDLElement		 element,
    CEBoolean		 process_flag)
#endif /* _NO_PROTO */
{
    CESegment  *segPtr;
    CEElement  *elPtr;

    if (MatchSDLElement (my_struct, element.element_str, element.sdl_element,
					element.str_sig_chars, False) != 0)
	return -1;
	
    if (element.sdl_element == SdlElementCdata || process_flag == False)
	return 0;

    if (_DtHelpCeAllocSegment(my_struct->malloc_size, &(my_struct->alloc_size),
				&(my_struct->block_list), &segPtr) != 0 ||
		AllocateElement(my_struct, element.sdl_element, &elPtr) != 0)
	return -1;

    elPtr->format_break = element.format_break;

    segPtr->seg_type   = CESetTypeToElement(0);
    if (my_struct->cur_link != -1)
      {
	segPtr->seg_type = _CESetHyperFlag(segPtr->seg_type);
        segPtr->link_index = my_struct->cur_link;
      }
    segPtr->seg_handle.el_handle = elPtr;

    my_struct->add_seg    = segPtr;
    if (element.format_break == True)
	my_struct->prev_data = NULL;

    if (element.sdl_element == SdlElementSnb)
	my_struct->snb = segPtr;

    return 0;

} /* End ParseElementStart */

/******************************************************************************
 * Function:    int ParseElementAttr (
 *					_CEFormatStruct my_struct,
 *					SDLElementAttrList *attribs)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ParseElementAttr(my_struct, cur_element, attribs, flag, process_flag)
    _CEFormatStruct	*my_struct;
    enum SdlElement	 cur_element;
    const SDLElementAttrList	*attribs;
    CEBoolean		 flag;
    CEBoolean		 process_flag;
#else
ParseElementAttr(
    _CEFormatStruct	*my_struct,
    enum SdlElement	 cur_element,
    const SDLElementAttrList	*attribs,
    CEBoolean		 flag,
    CEBoolean		 process_flag)
#endif /* _NO_PROTO */
{
    int          result = 0;
    char         attribName[MAX_ATTRIBUTE_LENGTH + 2];
    CEElement	*pElement;

    if (attribs == NULL || process_flag == False)
      {
	/*
	 * No attributes - is there cdata in here?
	 */
	if (flag || attribs != NULL)
	    result = _DtHelpCeSkipCdata (my_struct->my_file,
			(cur_element == SdlElementDocType ? True : False));

	/*
	 * get the element's end.
	 */
	if (result != -1)
	    result = _DtHelpCeGetSdlAttribute(my_struct->my_file,
						MAX_ATTRIBUTE_LENGTH,
						attribName);

	/*
	 * _DtHelpCeGetSdlAttribute should return 1, meaning it found
	 * the >
	 */
	if (result != 1)
	    return -1;
	return 0;
      }

    pElement = _CEElementOfSegment(my_struct->add_seg);
    InitializeAttributes(&(pElement->attributes), attribs);

    /*
     * force the default of subheading orientation to heads as below the head.
     */
    if (pElement->el_type == SdlElementSubHead)
        pElement->attributes.vorient = SdlJustifyBottom;

    do {
	result = _DtHelpCeGetSdlAttribute(my_struct->my_file,
						MAX_ATTRIBUTE_LENGTH,
						attribName);
	if (result == 0)
	    result = ProcessSDLAttribute(my_struct,
					&(pElement->attributes),
					attribs,
					attribName);
    } while (result == 0);

    if (result != -1)
	result = VerifyAttrList(&(pElement->attributes), attribs);

    if (result != -1)
	if (cur_element == SdlElementVirpage
				||
	    (pElement->attributes.enum_values & SDL_ATTR_CLASS)
				||
	    (pElement->attributes.str1_values & SDL_ATTR_SSI)
				||
	    (pElement->attributes.num_values & SDL_ATTR_LEVEL))
	    MergeTossInfo(pElement, my_struct);

    if (result != -1 &&
	(_CEIsStringAttrSet(pElement->attributes, SDL_ATTR_LANGUAGE)
		|| _CEIsStringAttrSet(pElement->attributes, SDL_ATTR_CHARSET)))
	my_struct->mb_len = _DtHelpCeGetMbLen(pElement->attributes.language,
						pElement->attributes.charset);
    if (result == 1)
	result = 0;

    return result;

} /* End ParseElementAttr */

/******************************************************************************
 * Function:    int ParseElementEnd (CECanvasStruct canvas,
 *					_CEFormatStruct my_struct,
 *					int el_type);
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
static	int
#ifdef _NO_PROTO
ParseElementEnd(canvas, my_struct, element, process_flag, end_flag)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLElement		 element;
    CEBoolean		 process_flag;
    CEBoolean		 end_flag;
#else
ParseElementEnd(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLElement		 element,
    CEBoolean		 process_flag,
    CEBoolean		 end_flag)
#endif /* _NO_PROTO */
{
    if (MatchSDLElement (my_struct, element.element_end_str,
		element.sdl_element, element.end_sig_chars, True) != 0 ||
		(end_flag == False &&
		    _DtHelpCeFindSkipSdlElementEnd(my_struct->my_file) != 0))
	return -1;

    if (process_flag == True)
      {
	if (my_struct->add_seg != NULL)
	  {
            CEElement  *pElement = my_struct->add_seg->seg_handle.el_handle;

            pElement->seg_list = my_struct->seg_list;

	    /*
	     * check to see if the element has any data.
	     * If not, can it (the element) be eleminated and free up memory?
	     */
	    if (pElement->seg_list == NULL && element.elim_flag == True
		&& pElement->el_type != SdlElementKey
		&& pElement->attributes.frmt_specs.tmargin == 0
		&& pElement->attributes.frmt_specs.bmargin == 0)
	      {
	        _DtHelpCeFreeSdlStructs(canvas, &(my_struct->add_seg), NULL);
		if (pElement->el_type == SdlElementLink)
		    _DtHelpCeRmLinkFromList(my_struct->my_links,
							my_struct->cur_link);
	        my_struct->add_seg = NULL;
	      }
	    else if (pElement->el_type == SdlElementBlock &&
				pElement->attributes.timing == SdlTimingAsync)
	      {
	        pElement->el_type = SdlElementBlockAsync;
	        AddToAsyncList(my_struct, my_struct->add_seg);
	        my_struct->add_seg = NULL;
	      }
          }
      }
    else
	my_struct->add_seg = my_struct->seg_list;

    if (element.format_break == True)
	my_struct->prev_data = NULL;

    return 0;

} /* End ParseElementEnd */

/******************************************************************************
 *
 * SDL Element Content Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    int FindAndFix (CECanvasStruct canvas,
 *					CESegment *toss,
 *					CESegment *snb,
 *					CESegment *seg_list)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
FindAndFix(canvas, toss, snb, seg_list)
    CECanvasStruct	*canvas;
    CESegment		*toss;
    CESegment		*snb;
    CESegment		*seg_list;
#else
FindAndFix(
    CECanvasStruct	*canvas,
    CESegment		*toss,
    CESegment		*snb,
    CESegment		*seg_list)
#endif /* _NO_PROTO */
{
    CEElement *snbEl;
    CESegment *lstDisp = NULL;

    /*
     * If the head has an snref in it, get the snb and resolve them
     */
    if (snb != NULL)
      {
	snbEl = snb->seg_handle.el_handle;
	while (seg_list != NULL)
	  {
	    if (_CEIsElement(seg_list->seg_type) &&
		     seg_list->seg_handle.el_handle->el_type == SdlElementSnRef)
	        _DtHelpCeResolveSdlSnref(canvas,toss,snbEl,seg_list,lstDisp);
	    else
		lstDisp = seg_list;

	    seg_list = seg_list->next_seg;
	  }
      }

    return 0;
}

/******************************************************************************
 * Function:    int CheckForSnb (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CheckForSnb(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
CheckForSnb(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int    result = 0;

    /*
     * If the head has an snref in it, get the snb and resolve them
     */
    if (my_struct->snref_used == True)
      {
	CESegment	*oldSegList = my_struct->seg_list;
	CESegment	*oldAddSeg  = my_struct->add_seg;
	CESegment	*oldPrevSeg = my_struct->prev_data;
	CESegment	*oldLastSeg = my_struct->last_seg;

	my_struct->seg_list  = NULL;
	my_struct->add_seg   = NULL;
	my_struct->prev_data = NULL;
	my_struct->last_seg  = NULL;

	result = ParseSDL(canvas, my_struct, SdlElementSnb, SdlElementNone,
						exceptions, process_mask);
	my_struct->seg_list  = oldSegList;
	my_struct->add_seg   = oldAddSeg;
	my_struct->prev_data = oldPrevSeg;
	my_struct->last_seg  = oldLastSeg;

	/*
	 * if no problems getting the snb, go through the items and
	 * resolve the snrefs.
	 */
	if (result != -1)
	    result = FindAndFix(canvas, my_struct->toss,
					my_struct->seg_list, my_struct->snb);
      }

    return result;

} /* End CheckForSnb */

/******************************************************************************
 * Function:    int CheckType (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CheckType(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
CheckType(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    enum SdlOption	 newOpt;
    CEElement		*pElement = _CEElementOfSegment(my_struct->add_seg);

    /*
     * cannot honor newlines in super or sub scripts.
     */
    newOpt = my_struct->cur_attr->type;
    if (SdlTypeLiteral == newOpt)
	newOpt = SdlTypeUnlinedLiteral;
    else if (SdlTypeLined == newOpt)
	newOpt = SdlTypeDynamic;

    pElement->attributes.type = newOpt;
    my_struct->cur_attr->type = newOpt;

    my_struct->flags = CE_SUPER_FLAG;
    if (SdlClassSub == pElement->attributes.clan)
        my_struct->flags = CE_SUB_FLAG;

    /*
     * strip hypertext links
     */
    my_struct->cur_link = -1;

    return 0;

} /* End CheckType */

/******************************************************************************
 * Function:    int SetType (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
SetType(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
SetType(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEElement		*pElement = _CEElementOfSegment(my_struct->add_seg);

    if (pElement->el_type == SdlElementSphrase)
      {
        pElement->attributes.type = SdlTypeLiteral;
        my_struct->cur_attr->type = SdlTypeLiteral;
        my_struct->flags          = 0;
      }
    else
        my_struct->cur_attr->type = SdlTypeCdata;

    return 0;

} /* End SetType */

/******************************************************************************
 * Function:    int ElseInfo (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
IfInfo(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
IfInfo(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEElement		*pElement = _CEElementOfSegment(my_struct->add_seg);

    if (pElement->el_type == SdlElementIf)
      {
	my_struct->then_prev = NULL;
	my_struct->else_prev = NULL;
      }
    else if (pElement->el_type == SdlElementThen)
        my_struct->then_prev = my_struct->prev_data;
    else /* if (pElement->el_type == SdlElementElse) */
        my_struct->else_prev = my_struct->prev_data;

    return 0;

} /* End IfInfo */

/******************************************************************************
 * Function:    int MarkFound (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
MarkFound(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
MarkFound(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    my_struct->snref_used = True;

    /*
     * is there a newline hanging around that needs turning into a space?
     */
    if (CESearchSDLMask(process_mask, SdlElementCdata)
	&& my_struct->last_was_space == False && my_struct->last_was_nl == True)
      {
	CESegment *pSeg = my_struct->prev_data;

	/*
	 * tack the space onto the end of previous string if possible.
	 */
	if (pSeg != NULL && _CEIsString(pSeg->seg_type) &&
		_CEGetCharLen (pSeg->seg_type) == 1 &&
			_CEIsNewLine(pSeg->seg_type) == False)
	  {
	    int len = strlen(_CEStringOfSegment(pSeg));

	    _CEStringOfSegment(pSeg) = (char *) realloc(
						_CEStringOfSegment(pSeg),len+2);
	    if (_CEStringOfSegment(pSeg) == NULL)
		return -1;

	    _CEStringOfSegment(pSeg)[len++] = ' ';
	    _CEStringOfSegment(pSeg)[len  ] = '\0';
	  }
	/* FIX to BUG 4007476: The third argument to MySaveString()
	 * was changed from " " to "\0" because helpviewer was
	 * dumping core when trying to change the value of a constant.
	 */
	else if (MySaveString(&(my_struct->seg_list), my_struct, "\0",
				my_struct->cur_link, 1, False ) != 0)
	    return -1;

	my_struct->last_was_space = True;
	my_struct->last_was_nl    = False;
      }

    return 0;

} /* End MarkFound */

/******************************************************************************
 * Function:    int MaintainLink (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
MaintainLink(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
MaintainLink(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    if (my_struct->prev_data != NULL)
        my_struct->prev_data->next_disp = my_struct->add_seg;

    my_struct->last_was_space = False;
    my_struct->last_was_mb    = False;
    my_struct->last_was_nl    = False;
    my_struct->prev_data = my_struct->add_seg;
    return 0;

} /* End MaintainLink */

/******************************************************************************
 * Function:    int AllocateBlock (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
AllocateBlock(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
AllocateBlock(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    /*
     * allocate a block of information all at once for later segments.
     * This, hopefully, will save processing time when later freed.
     *
     * Simply change the malloc_size in the formatting struct.
     */
    CEElement   *pElement;

    if (my_struct->add_seg != NULL)
      {
        pElement = _CEElementOfSegment(my_struct->add_seg);
        if (_CEIsAttrSet(pElement->attributes, SDL_ATTR_COUNT))
	    my_struct->malloc_size = pElement->attributes.num_vals.count;
        else
          {
            my_struct->malloc_size = 500;
            if (pElement->el_type == SdlElementToss)
                my_struct->malloc_size = 160;
          }
      }

    return 0;

} /* End AllocateBlock */

/******************************************************************************
 * Function:    int CleanUpBlock (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CleanUpBlock(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
CleanUpBlock(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    /*
     * Go back to piece meal allocation.
     */
    my_struct->malloc_size = 1;
    return 0;

} /* End CleanUpBlock */

/******************************************************************************
 * Function:    int AllocateSpace (CECanvasStruct canvas,
 *                                              _CEFormatStruct my_struct,
 *                                              int element_types,
 *                                              int exceptions);
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static  int
#ifdef _NO_PROTO
AllocateSpace(canvas, my_struct, sig_element, element_types, exceptions, process_mask)
    CECanvasStruct      *canvas;
    _CEFormatStruct     *my_struct;
    SDLMask             *element_types;
    enum SdlElement	 sig_element;
    SDLMask             *exceptions;
    SDLMask             *process_mask;
#else
AllocateSpace(
    CECanvasStruct      *canvas,
    _CEFormatStruct     *my_struct,
    SDLMask             *element_types,
    enum SdlElement	 sig_element,
    SDLMask             *exceptions,
    SDLMask             *process_mask)
#endif /* _NO_PROTO */
{
    CESegment   *pSeg;
    CEElement   *pElement = my_struct->add_seg->seg_handle.el_handle;

    /*
     * allocate the segment...but don't attach anything to it yet.
     * This saves space for a latter resolution of say a graphic or
     * special character.
     */
    if (_DtHelpCeAllocSegment(my_struct->malloc_size, &(my_struct->alloc_size),
				&(my_struct->block_list), &pSeg) != 0)
        return -1;

    if (CESearchSDLMask(element_types, SdlElementSpc) == True)
      {
	pSeg->seg_type = _CESetSpecial(pSeg->seg_type) | my_struct->flags; 
        pSeg->seg_handle.file_handle = pElement->attributes.id_vals.name;
      }
    else if (CESearchSDLMask(element_types, SdlElementGraphic) == True)
        pSeg->seg_type = _CESetGraphic(pSeg->seg_type);

    if (my_struct->cur_link != -1)
      {
        pSeg->seg_type = _CESetHyperFlag(pSeg->seg_type);
        pSeg->link_index = my_struct->cur_link;
      }

    if (my_struct->prev_data != NULL)
        my_struct->prev_data->next_disp = pSeg;

    my_struct->prev_data = pSeg;
    pElement->seg_list   = pSeg;

    return 0;

} /* End AllocateSpace */

/******************************************************************************
 * Function:    int RegisterLink (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
RegisterLink(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
RegisterLink(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int		 winType = CEWindowHint_CurrentWindow;
    CEElement	*pEl     = my_struct->add_seg->seg_handle.el_handle;
    SDLId	 rid     = RIdString(pEl);

    if (CESearchSDLMask(process_mask, SdlElementLink) == False)
	return 0;

    switch (pEl->attributes.window)
      {
	case SdlWindowNew: winType = CEWindowHint_NewWindow;
			break;
	case SdlWindowPopup: winType = CEWindowHint_PopupWindow;
			break;
      }

    /*
     * the link manager assumes it owns the string, so make a copy.
     */
    rid = strdup(rid);
    if (rid != NULL)
      {
        my_struct->cur_link = _DtHelpCeAddLinkToList(&(my_struct->my_links),
					rid,
					CELinkType_SameVolume, winType, NULL);
	my_struct->add_seg->link_index = my_struct->cur_link;
      }

    if (rid == NULL || my_struct->cur_link < 0)
	return -1;

    return 0;

} /* End RegisterLink */

/******************************************************************************
 * Function:    int ResolveIf (_CEFormatStruct my_struct)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ResolveIf(canvas, my_struct, prev_data)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    CESegment		*prev_data;
#else
ResolveIf(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    CESegment		*prev_data)
#endif /* _NO_PROTO */
{

    CESegment	*ifSeg       = my_struct->add_seg;

    CESegment	*condSeg     = ifSeg->seg_handle.el_handle->seg_list;
    CEElement	*condEl	     = condSeg->seg_handle.el_handle;
    CESegment	*condDataSeg = condEl->seg_list;

    CESegment	*thenSeg     = condSeg->next_seg;
    CESegment	*elseSeg     = thenSeg->next_seg;

    CESegment	*resolveSeg  = NULL;
    CESegment	*nextDisp    = NULL;
    CEElement	*el	     = NULL;
    char	*ifData      = NULL;

    if (_DtHelpCeRunInterp(canvas->virt_functions._CEFilterExecCmd,
				canvas->client_data,
				condEl->attributes.interp,
				_CEStringOfSegment(condDataSeg), &ifData) == 0)
      {
	/*
	 * Get the pointer to the next displayable item in the 'then'
	 * list. This usually will point into the 'then' list,
	 * but may point into the 'else' list.
	 */
	if (my_struct->then_prev != NULL)
	    nextDisp = my_struct->then_prev->next_disp;

	/*
	 * use the 'then' data
	 */
        if (atoi (ifData) != 0)
	  {
	    el = thenSeg->seg_handle.el_handle;

	    /*
	     * check to make sure that the next_disp is NOT into the
	     * 'else' list (because it's about to become a dangling
	     * next_disp if it is!).
	     */
	    if (elseSeg != NULL)
	      {
		/*
		 * if the next displayable segment is in the 'else'
		 * list, null out the next displayable segement since
		 * there isn't anything in the 'then' list.
		 */
	        if (my_struct->then_prev == my_struct->else_prev)
		    nextDisp = NULL;
		else
		    /*
		     * terminate the displayable segment list
		     * before the 'else' list.
		     */
		    my_struct->else_prev->next_disp = NULL;
	      }
	  }
        else if (elseSeg != NULL)
	  {
	    /*
	     * use the 'else' data.
	     */
	    el = elseSeg->seg_handle.el_handle;

	    /*
	     * Get the next displayable item in the 'else' list.
	     */
	    if (my_struct->else_prev != NULL)
	        nextDisp = my_struct->else_prev->next_disp;
	  }

        if (el != NULL)
          {
	    resolveSeg   = el->seg_list;
	    el->seg_list = NULL;
          }

	free(ifData);
      }

    /*
     * set the true next displayable pointer.
     */
    if (prev_data != NULL)
	prev_data->next_disp = nextDisp;

    /*
     * set the previous displayable segment to the last displayable
     * segment in the 'if' clause; in case more displayable segments
     * follow.
     */
    my_struct->prev_data = nextDisp;

    _DtHelpCeFreeSdlStructs(canvas,&(my_struct->add_seg),&(my_struct->my_links));
    my_struct->add_seg = resolveSeg;

    return 0;

} /* End ResolveIf */

/******************************************************************************
 * Function:    int ZeroOrOne (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ZeroOrOne(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
ZeroOrOne(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int    result = 0;

    if (my_struct->parsed == SdlElementNone)
        result = _DtHelpCeReturnSdlElement(my_struct->my_file, SDLElements,
			CESearchSDLMask(element_types, SdlElementCdata),
				&(my_struct->parsed), &(my_struct->remember),
				&(my_struct->end_flag));

    if (result == 0 && my_struct->end_flag == False &&
		CESearchSDLMask(element_types, my_struct->parsed) == True)
      {
	/*
	 * check to make sure this is *not* Cdata or if it is that the
	 * PcDataFollows flag is *not* set.
	 */
	if (my_struct->parsed != SdlElementCdata ||
		CESearchSDLMask(element_types, SdlPcDataFollows) == False)
            result = ParseSDL(canvas, my_struct, my_struct->parsed, sig_element,
						exceptions, process_mask);
      }
    else if (result == 1) /* eof on compressed entry/file */
	result = 0;

    return result;
} /* End ZeroOrOne */

/******************************************************************************
 * Function:    int ZeroToN (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ZeroToN(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
ZeroToN(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEBoolean done   = False;
    int       result = 0;

    while (result == 0 && !done)
      {
        if (my_struct->parsed == SdlElementNone)
            result = _DtHelpCeReturnSdlElement(my_struct->my_file, SDLElements,
			CESearchSDLMask(element_types, SdlElementCdata),
				&(my_struct->parsed), &(my_struct->remember),
				&(my_struct->end_flag));

	if (result == 0 && my_struct->end_flag == False &&
		CESearchSDLMask(element_types, my_struct->parsed) == True)
          {
	    /*
	     * check to make sure this is *not* Cdata or if it is that the
	     * PcDataFollows flag is *not* set.
	     */
	    if (my_struct->parsed != SdlElementCdata ||
		      CESearchSDLMask(element_types, SdlPcDataFollows) == False)
                result = ParseSDL(canvas, my_struct, my_struct->parsed,
					sig_element, exceptions, process_mask);
	    else
		done = True;
	  }
	else
	  {
	    if (result == 1) /* eof on compressed entry/file */
	        result = 0;
	    done   = True;
	  }
      }

    return result;

} /* End ZeroToN */

/******************************************************************************
 * Function:    int OneToN (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
OneToN(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
OneToN(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int    found  = False;
    int    result = 0;
    int    done   = False;

    while (result == 0 && !done)
      {
        if (my_struct->parsed == SdlElementNone)
            result = _DtHelpCeReturnSdlElement(my_struct->my_file, SDLElements,
			CESearchSDLMask(element_types, SdlElementCdata),
				&(my_struct->parsed), &(my_struct->remember),
				&(my_struct->end_flag));

	if (result == 0 && my_struct->end_flag == False &&
		CESearchSDLMask(element_types, my_struct->parsed) == True)
	  {
	    found = True;
            result = ParseSDL(canvas, my_struct, my_struct->parsed, sig_element,
						exceptions, process_mask);
	  }
	else
	    done = True;
      }

    if (!found)
	result = -1;

    return result;

} /* End OneToN */

/******************************************************************************
 * Function:    int OnlyOne (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
OnlyOne(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
OnlyOne(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEBoolean found  = False;
    int       result = 0;

    if (my_struct->parsed == SdlElementNone)
        result = _DtHelpCeReturnSdlElement(my_struct->my_file, SDLElements,
			CESearchSDLMask(element_types, SdlElementCdata),
				&(my_struct->parsed), &(my_struct->remember),
				&(my_struct->end_flag));

    if (result == 0 && my_struct->end_flag == False &&
		CESearchSDLMask(element_types, my_struct->parsed) == True)
      {
	found = True;
        result = ParseSDL(canvas, my_struct, my_struct->parsed, sig_element,
						exceptions, process_mask);
      }

    if (!found)
	result = -1;

    return result;

} /* End OnlyOne */

/******************************************************************************
 * Function:    int OnlyOneEach (CECanvasStruct canvas,
 *						_CEFormatStruct my_struct,
 *						int element_types,
 *						int exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
OnlyOneEach(canvas, my_struct, element_types, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*element_types;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
OnlyOneEach(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*element_types,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int       result = 0;
    enum SdlElement myEl;
    SDLMask	myMask[SDL_MASK_LEN];

    SaveRestoreMask(myMask, element_types);

    while (result == 0 && CECheckMask(myMask))
      {
        if (my_struct->parsed == SdlElementNone)
            result = _DtHelpCeReturnSdlElement(my_struct->my_file, SDLElements,
			CESearchSDLMask(myMask, SdlElementCdata),
				&(my_struct->parsed), &(my_struct->remember),
				&(my_struct->end_flag));

        if (result == 0 && my_struct->end_flag == False &&
		CESearchSDLMask(element_types, my_struct->parsed) == True)
          {
	    myEl   = my_struct->parsed;
            result = ParseSDL(canvas, my_struct, my_struct->parsed, sig_element,
						exceptions, process_mask);

	    CEStripElementFromMask(myMask, myEl);
          }
	else
	    result = -1;
      }

    if (CECheckMask(myMask))
	result = -1;

    return result;

} /* End OnlyOneEach */

/******************************************************************************
 * Function:    int Cdata (CECanvasStruct canvas, _CEFormatStruct my_struct,
 *					int cur_element, exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
Cdata(canvas, my_struct, cur_element, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
Cdata(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int		 i;
    int		 reason;
    int		 myLen    = 0;
    int		 curLen   = my_struct->mb_len;
    int		 multiLen = my_struct->mb_len;
    int		 saveLen  = my_struct->mb_len;
    char	*string   = NULL;
    char	 spaceStr[] = " ";
    char	 dashStr[]  = "-";
    enum SdlOption type     = my_struct->cur_attr->type;
    CESegment	*pSeg       = NULL;
    CEBoolean	 nlToSpace   = True;
    CEBoolean	 processFlag = CESearchSDLMask(process_mask, SdlElementCdata);

    if (canvas != NULL && canvas->nl_to_space == 0)
	nlToSpace = False;

    if (type == SdlTypeCdata)
      {
	/*
	 * the element requires straight cdata for processing - i.e.
	 * the data is going to be passed off to an interperter.
	 * Therefore it doesn't want it broken up into different
	 * byte length segments.
	 *
	 * Therefore, force the string save to put all the data into
	 * one string.
	 */
	saveLen = 1;
      }

    if (my_struct->remember != NULL)
      {
        i   = 0;
        string  = my_struct->remember;
	myLen   = strlen(string);
        while (string[i] != '\0')
          {
            if (multiLen != 1)
                curLen = mblen (&string[i], multiLen);

	    if (curLen == 1)
	      {
		if (my_struct->last_was_nl == True)
		  {
		    if (MoveString(&string, &myLen, &i) == -1)
			return -1;

		    string[i++] = ' ';
		    my_struct->last_was_space = True;
		  }

		my_struct->last_was_nl = False;
		my_struct->last_was_mb = False;

                if (string[i] == '\t')
                    string[i] = ' ';
    
                if (string[i] == '&')
                  {
                    strcpy (&string[i], &string[i+1]);
                    if (string[i] == '\0')
                      {
                        string[i] = BufFileGet(my_struct->my_file);
                        if (string[i] == BUFFILEEOF)
                            return -1;
                        string[i+1] = '\0';
                      }
    
		    /*
		     * is this an SGML numeric character reference
		     * entity?  if so, it should have the format
		     * '&#d[d[d]][;]' where 'ddd' represent characters
		     * of '0' to '9'.  The semi-colon is required iff
		     * the next character is a numeric character of '0'
		     * to '9'.  Otherwise it is optional.
		     */
		    if (string[i] == '#')
		      {
			int j;
			int value;

#define	ESC_STRING_LEN	4
			i++;

			/*
			 * Is there enough to room to process three digits
			 * and a possible semi-colon?
			 */
			if (myLen - i < ESC_STRING_LEN)
			  {
			    /*
			     * lengthen the string so that it can contain
			     * the information
			     */
			    myLen  += ESC_STRING_LEN;
			    string  = (char *) realloc(string,
						sizeof(char) * (myLen + 1));
			    if (string == NULL)
				return -1;
			  }

			/*
			 * now make sure that the entire numeric entity
			 * exists in the string.
			 */
			j = i;
			while ('0' <= string[i] && string[i] <= '9')
			    i++;

			/*
			 * run into the end of string before running
			 * into a delimiter? Fill out the escaped
			 * numeric character.
			 */
			if (string[i] == '\0')
			  {
			    do
			      {
			        string[i] = BufFileGet(my_struct->my_file);
				if (string[i] == BUFFILEEOF)
				    return -1;
				i++;
			      } while (i < myLen && '0' <= string[i-1]
							&& string[i-1] <= '9');
			    /*
			     * end the string and back up to the last
			     * character
			     */
			    string[i] = '\0';
			    i--;
			  }

			/*
			 * the fourth character is a numeric, error
			 */
			if ('0' <= string[i] && string[i] <= '9')
			    return -1;

			if (string[i] == ';')
			    i++;

			value = atoi(&string[j]);
			if (value > 255)
			    return -1;

			/*
			 * smash over the pound sign with the 'real' value
			 * and copy the rest of the string to after it.
			 */
			string[j-1] = (char) value;
                        strcpy (&string[j], &string[i]);
			i = j;
		      }

                    if (string[i] == '\n')
                      {
                        if (ProcessString(my_struct, True, True, True, True,
					processFlag, string, saveLen, &i) != 0)
			    return -1;
                      }
		    else if (string[i] == ' ')
		      {
			if (ProcessNonBreakChar(my_struct, processFlag,
					spaceStr, string, saveLen, &i) != 0)
			    return -1;
		      }
		    else if (string[i] == '-')
		      {
			if (ProcessNonBreakChar(my_struct, processFlag,
					dashStr, string, saveLen, &i) != 0)
			    return -1;
		      }
		    else
		        my_struct->last_was_space = False;
                    i++;
                  }
                else if (string[i] == '\n')
                  {
		    /*
		     * want to keep the newlines
		     */
		    if (type == SdlTypeCdata)
			i++;
                    else if (type == SdlTypeDynamic ||
						type == SdlTypeUnlinedLiteral)
                      {
                        if (my_struct->last_was_space == False)
			    my_struct->last_was_nl = True;

                        strcpy (&string[i], &string[i+1]);
                      }
                    else
                      {
                        string[i] = '\0';
                        if (processFlag == True &&
                                    MySaveString(&(my_struct->seg_list),
					my_struct, string, my_struct->cur_link,
                                        saveLen, True) != 0)
                          {
                            MyFree(string);
                            return -1;
                          }
    
                        strcpy (string, &string[i+1]);
                        i = 0;
                      }
                  }
                else if (string[i] == ' ')
                  {
                    if (type != SdlTypeLiteral && type != SdlTypeUnlinedLiteral
					&& my_struct->last_was_space == True)
                        strcpy (&string[i], &string[i+1]);
                    else
                        i++;
                    my_struct->last_was_space = True;
                  }
                else
                  {
                    my_struct->last_was_space = False;
                    i++;
                  }
              }
	    else if (curLen > 0)
	      {
		if (my_struct->last_was_nl == True)
		  {
		    if (nlToSpace == True || my_struct->last_was_mb == False)
		      {
		        if (MoveString(&string, &myLen, &i) == -1)
			    return -1;

		        string[i++] = ' ';
		      }
		    else /* the last was a multibyte character, tighten up */
		      {
			i--;
			strcpy (&string[i], &string[i+1]);
		      }
		  }

		my_struct->last_was_space = False;
		my_struct->last_was_nl    = False;
		my_struct->last_was_mb    = True;
		i += curLen;
	      }
	    else if (curLen == 0)
		return -1;
	    else /* if (curLen < 0) */
	      {
		/*
		 * must finish up the character
		 */
		int  len = i + 1;

		while (curLen < 0 && len - i < multiLen)
		  {
		    if (myLen <= len)
		      {
		        string = (char *) realloc(string, myLen + multiLen + 1);
		        if (string == NULL)
			    return -1;
			myLen += multiLen;
		      }

                    string[len] = BufFileGet(my_struct->my_file);
                    if (string[len++] == BUFFILEEOF)
                            return -1;

                    string[len] = '\0';
                    curLen      = mblen (&string[i], multiLen);
		  }

		if (curLen < 0)
		    return -1;
	      }
          }

        /* ----------------------------------------------------------------- */
        /*  Start from scratch in case we have an abbreviaion tag and value  */
        /* ----------------------------------------------------------------- */

	if (processFlag == False ||
            my_struct->cur_attr->strings.el_strs.abbrev != NULL)
	  {
	    free(string);
	    string = NULL;
	    myLen  = 0;
	  }

	my_struct->remember = NULL;
      }
    do {
        my_struct->parsed = SdlElementNone;
        reason = _DtHelpCeGetSdlCdata(my_struct->my_file,
                                      my_struct->cur_attr->strings.el_strs.abbrev,
                                      type, multiLen,
                                      nlToSpace,
                                      &my_struct->last_was_space,
                                      &my_struct->last_was_nl,
                                      &my_struct->last_was_mb,
                                      (processFlag == True ? &string : ((char**)NULL)),
                                      myLen);
        if (reason < 0)
	    return -1;

	if (string != NULL && *string != '\0')
	  {
	    if (MySaveString(&(my_struct->seg_list), my_struct,
			string, my_struct->cur_link, saveLen,
			(reason ? True : False)) != 0)
              {
	        MyFree(string);
	        return -1;
              }

	    string[0] = '\0';
	  }
	else if (reason == 1) /* stopped because of newline */
	  {
	    pSeg = my_struct->prev_data;

	    if (pSeg == NULL || _CEIsNewLine(pSeg->seg_type))
	      {
		if (_DtHelpCeAllocSegment(my_struct->malloc_size,
				&(my_struct->alloc_size),
				&(my_struct->block_list), &pSeg) != 0)
		    return -1;

		pSeg->seg_type   = _CESetNoop(0);
		pSeg->seg_handle.str_handle = NULL;

		if (my_struct->prev_data != NULL)
		    my_struct->prev_data->next_disp = pSeg;

		my_struct->prev_data = pSeg;
		_DtHelpCeAddSegToList(pSeg, &(my_struct->seg_list),
						&(my_struct->last_seg));
	      }

	    pSeg->seg_type = _CESetNewLineFlag(pSeg->seg_type);

	    my_struct->last_was_space = True;
	    my_struct->last_was_nl    = False;
	  }

      } while (reason > 0);

    MyFree(string);
    return 0;

} /* End Cdata */

/******************************************************************************
 * Function:    int ProcessEnterAttr (CECanvasStruct canvas,
 *					_CEFormatStruct my_struct,
 *					int cur_element, exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessEnterAttr(canvas, my_struct, cur_element, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
ProcessEnterAttr(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEElement  *pElement = _CEElementOfSegment(my_struct->add_seg);

    /*
     * save the enter string as part of this element's segment list
     */
    if (pElement->attributes.strings.el_strs.enter != NULL &&
	((int)strlen(pElement->attributes.strings.el_strs.enter)) > 0 &&
	MySaveString(&(my_struct->seg_list), my_struct,
			pElement->attributes.strings.el_strs.enter,
			my_struct->cur_link, my_struct->mb_len, False) != 0)
	    return -1;

    return 0;

} /* End ProcessEnterAttr */

/******************************************************************************
 * Function:    int ProcessExitAttr (CECanvasStruct canvas,
 *					_CEFormatStruct my_struct,
 *					int cur_element, exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ProcessExitAttr(canvas, my_struct, cur_element, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
ProcessExitAttr(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    CEElement  *pElement = _CEElementOfSegment(my_struct->add_seg);

    /*
     * save the exit string as part of this element's segment list
     */
    if (pElement->attributes.strings.el_strs.exit != NULL &&
	((int)strlen(pElement->attributes.strings.el_strs.exit)) > 0 &&
	MySaveString(&(my_struct->seg_list), my_struct,
			pElement->attributes.strings.el_strs.exit,
			my_struct->cur_link, my_struct->mb_len, False) != 0)
	    return -1;

    return 0;

} /* End ProcessExitAttr */

/******************************************************************************
 * Function:    int FakeEnd (CECanvasStruct canvas, _CEFormatStruct my_struct,
 *					int cur_element, exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
FakeEnd(canvas, my_struct, cur_element, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
FakeEnd(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{

    my_struct->end_flag  = True;
    my_struct->faked_end = True;
    MaskToValue(cur_element, my_struct->parsed);

    return 0;

} /* End FakeEnd */

/******************************************************************************
 * Function:    int InitLast (CECanvasStruct canvas, _CEFormatStruct my_struct,
 *					int cur_element, exceptions);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Looks for the virtual page attributes.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
InitLast(canvas, my_struct, cur_element, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    SDLMask		*cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
InitLast(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    SDLMask		*cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{

    my_struct->last_was_space = True;
    my_struct->last_was_mb    = False;
    my_struct->last_was_nl    = False;
    return 0;

} /* End InitLast */


/******************************************************************************
 *
 * Main Parsing Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    int ParseSDL (CECanvasStruct canvas, _CEFormatStruct my_struct,
 *				int cur_element, int cur_execpt);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Parses a set of rules.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ParseSDL(canvas, my_struct, cur_element, sig_element, cur_except, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    enum SdlElement	 cur_element;
    enum SdlElement	 sig_element;
    SDLMask		*cur_except;
    SDLMask		*process_mask;
#else
ParseSDL(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    enum SdlElement	 cur_element,
    enum SdlElement	 sig_element,
    SDLMask		*cur_except,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int    i = 0;
    int    result  = 0;
    int    oldLink = my_struct->cur_link;
    CESegment       *oldCurSeg   = my_struct->last_seg;
    CESegment       *oldSeglist  = my_struct->seg_list;
    CESegment       *oldAddSeg   = my_struct->add_seg;
    CESegment       *ifPrevData  = my_struct->prev_data;
    CESDLAttrStruct *saveAttribs = my_struct->cur_attr;
    const SDLContent	*content;
    SDLMask		 oldMask[SDL_MASK_LEN];
    SDLMask		 oldExcept[SDL_MASK_LEN];
    CEBoolean		 processFlag =
				CESearchSDLMask(process_mask, SdlElementCdata);

    /*
     * While this element is allowed normally in the content of
     * the parent element, it it currently allowed?
     */
    if (CESearchSDLMask(cur_except, cur_element) == True)
	return -1;

    /*
     * special processing for CDATA elements
     */
    if (cur_element != SdlElementCdata)
      {
        my_struct->seg_list = NULL;
        my_struct->last_seg = NULL;
        my_struct->add_seg  = NULL;
      }

    /*
     * look in the master list for this element
     */
    while (i < MaxSDLElements && SDLElements[i].sdl_element != cur_element)
	i++;

    /*
     * Didn't find this element in the master list.
     */
    if (i >= MaxSDLElements)
	return -1;

    /*
     * merge this element's exceptions with parent's list.
     * save the old process mask.
     */
    SaveRestoreMask(oldExcept, cur_except);
    MergeMasks(cur_except, SDLElements[i].exceptions);
    SaveRestoreMask(oldMask, process_mask);

    /*
     * the content for this element is...
     */
    content = SDLElements[i].content;

    /*
     * check to see if we want to process this element
     * If sig_element is set, parse the element and its content.
     */
    if (sig_element != SdlElementNone)
      {
	processFlag = False;
	if (sig_element == cur_element)
	  {
	    processFlag  = True;
	    sig_element  = SdlElementNone;
	    SaveRestoreMask(process_mask, AllMaskSet);
	  }
      }

    /*
     * If not CDATA, then have to get the element start string, attributes
     * and ending markup tag.
     */
    if (cur_element != SdlElementCdata)
      {
        if (ParseElementStart(my_struct,SDLElements[i],processFlag) != 0
				||
		ParseElementAttr(my_struct,
					cur_element,
					SDLElements[i].attrib_list,
					SDLElements[i].cdata_flag,
					processFlag) != 0)
	    result = -1;
      }

    /*
     * now parse the element's content
     */
    my_struct->faked_end = False;
    while (result == 0 && content != NULL &&
			CESearchSDLMask(content->mask, SdlElementNone) == False)
      {
	result = (*(content->model))(canvas, my_struct, content->mask,
					sig_element, cur_except, process_mask);
	content++;
      }

    /*
     * If not CDATA, get the element end markup and adjust some pointers
     */
    SaveRestoreMask(process_mask, oldMask);
    SaveRestoreMask(cur_except  , oldExcept);
    if (cur_element != SdlElementCdata)
      {
        if (result == 0 && SDLElements[i].element_end_str != NULL)
	    result = ParseElementEnd(canvas, my_struct, SDLElements[i],
							processFlag,
							my_struct->faked_end);

	if (result == 0 && cur_element == SdlElementIf)
	    result = ResolveIf (canvas, my_struct, ifPrevData);

        my_struct->last_seg = oldCurSeg;
        my_struct->seg_list = oldSeglist;

        if (result == 0 && my_struct->add_seg != NULL)
	    _DtHelpCeAddSegToList(my_struct->add_seg, &(my_struct->seg_list),
						&(my_struct->last_seg));

        my_struct->add_seg   = oldAddSeg;
	my_struct->cur_attr  = saveAttribs;
	my_struct->cur_link  = oldLink;
      }

    return result;

} /* End ParseSDL */

/******************************************************************************
 * Function:    int ProcessContent (
 *				CECanvasStruct *canvas,
 *				_CEFormatStruct *my_struct,
 *				const SDLContent *content,
 *				SDLMask *exceptions,
 *				SDLMask *process_mask);
 *
 * Parameters:
 *		canvas		Specifies the canvas to use.
 *		my_struct	Specifies specific informationg for this parse.
 *		content		Specifies the content model to parse.
 *		exceptions	Specifies the current elements excepted
 *				from being in the current content.
 *		process_mask	Specifies which elements to save in memory.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
static int
#ifdef _NO_PROTO
ProcessContent(canvas, my_struct, content, sig_element, exceptions, process_mask)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    const SDLContent	*content;
    enum SdlElement	 sig_element;
    SDLMask		*exceptions;
    SDLMask		*process_mask;
#else
ProcessContent(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    const SDLContent	*content,
    enum SdlElement	 sig_element,
    SDLMask		*exceptions,
    SDLMask		*process_mask)
#endif /* _NO_PROTO */
{
    int   result = 0;

    while (result == 0 && content != NULL &&
			CESearchSDLMask(content->mask, SdlElementNone) == False)
      {
	result = (*(content->model))(canvas, my_struct, content->mask,
					sig_element, exceptions, process_mask);
	content++;
      }

    return result;
}
#ifdef	NOTDONE
/******************************************************************************
 * Function:    int SearchElement (CESegment p_seg, 
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Parses a set of rules.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
SearchElement(p_seg, sdl_vol)
    CESegment		*p_seg;
    CESDLVolume		*sdl_vol;
#else
SearchElement(
    CESegment		*p_seg,
    CESDLVolume		*sdl_vol)
#endif /* _NO_PROTO */
{
    CEElement  *p_el = p_seg->seg_handle.el_handle;

    if (p_el->el_type == SdlElementToss)
	sdl_vol->toss = p_el->seg_list;
    else if (p_el->el_type == SdlElementLoids)
	sdl_vol->loids = p_el->seg_list;
    else
      {
	for (p_seg = p_el->seg_list; p_seg != NULL &&
	      (sdl_vol->loids == NULL || sdl_vol->toss == NULL);
				p_seg = p_seg->next_seg)
	    if (_CEGetPrimaryType(p_seg->seg_type) == CE_ELEMENT)
	        SearchElement(p_seg, sdl_vol);
      }

    return 0;

} /* End SearchElement */
#endif

/******************************************************************************
 * Function:    int SearchForElement (CESegment p_seg, 
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Parses a set of rules.
 *
 ******************************************************************************/
static	CEElement *
#ifdef _NO_PROTO
SearchForElement(p_seg, el_type)
    CESegment		*p_seg;
    enum SdlElement	 el_type;
#else
SearchForElement(
    CESegment		*p_seg,
    enum SdlElement	 el_type)
#endif /* _NO_PROTO */
{
    CEElement  *p_el = NULL;

    if (_CEIsElement(p_seg->seg_type))
      {
	p_el = _CEElementOfSegment(p_seg);
        if (p_el->el_type != el_type)
          {
	    for (p_seg = p_el->seg_list, p_el = NULL;
			p_el == NULL && p_seg != NULL; p_seg = p_seg->next_seg)
	        p_el = SearchForElement(p_seg, el_type);
          }
      }

    return p_el;

} /* End SearchForElement */

/******************************************************************************
 * Function:    int FormatSDLTitle (
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Parses a set of rules.
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
FormatSDLTitle(canvas, filename, offset, fd, my_struct)
    CECanvasStruct	*canvas;
    char		*filename;
    int			 offset;
    int			 fd;
    _CEFormatStruct	*my_struct;
#else
FormatSDLTitle(
    CECanvasStruct	*canvas,
    char		*filename,
    int			 offset,
    int			 fd,
    _CEFormatStruct	*my_struct)
#endif /* _NO_PROTO */
{
    int		result = 0;
    SDLMask	processMask[SDL_MASK_LEN] = CEInitializeMaskEleven( \
				SdlElementTitle  , SdlElementHead   , \
				SdlElementKey    , SdlElementSphrase, \
				SdlElementRev    , SdlElementIf     , \
				SdlElementSpc    , SdlElementAnchor , \
				SdlElementLink   , SdlElementSnRef  , \
				SdlElementCdata);

    SDLMask	startExcept[SDL_MASK_LEN] = CEInitializeMask(SdlElementNone);

    result = _DtHelpCeFileOpenAndSeek(filename, offset, fd,
					&(my_struct->my_file), NULL);
    if (result != -1)
      {
        result = ParseSDL (canvas, my_struct, SdlElementTitle, SdlElementNone,
				    startExcept, processMask);
        _DtHelpCeBufFileClose (my_struct->my_file, (fd == -1 ? True : False));
      }

    return result;

} /* End FormatSDLTitle */

/******************************************************************************
 * Function:    void SetGhostLink (
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
SetGhostLink(link_data, segments, link_idx)
    CELinkData	*link_data;
    CESegment   *segments;
    int		 link_idx;
#else
SetGhostLink(
    CELinkData	*link_data,
    CESegment   *segments,
    int		 link_idx)
#endif /* _NO_PROTO */
{
    CEElement *pEl;

    while (segments != NULL)
      {
	if (_CEIsNotElement(segments->seg_type))
	  {
	    if (_CEIsHyperLink(segments->seg_type))
	      {
	        _DtHelpCeRmLinkFromList(*link_data, segments->link_index);
	        segments->seg_type = _CEClearHyperFlag(segments->seg_type);
	      }

	    segments->seg_type   = _CESetGhostLink(segments->seg_type);
	    segments->link_index = link_idx;
	  }
	else /* if (_CEIsElement(segments->seg_type)) */
	  {
	    pEl = segments->seg_handle.el_handle;
	    SetGhostLink (link_data, pEl->seg_list, link_idx);
	  }
	segments = segments->next_seg;
      }

} /* End SetGhostLink */

/******************************************************************************
 * Function:    void CompressAndIntegrateSnb ()
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
CompressAndIntegrateSnb(snb_top, new_snb)
    CESegment   **snb_top;
    CESegment    *new_snb;
#else
CompressAndIntegrateSnb(
    CESegment   **snb_top,
    CESegment    *new_snb)
#endif /* _NO_PROTO */
{
    CEElement *oldSnbEl;
    CEElement *newSnbEl = NULL;

    CESegment *newSegList;
    CESegment *prevSeg = NULL;
    CESegment *nextSeg;

    if (new_snb != NULL)
      {
	/*
	 * get the new snb's element handle
	 */
	newSnbEl   = new_snb->seg_handle.el_handle;

	/*
	 * get the segment list of the new snb.
	 */
	newSegList = newSnbEl->seg_list;

	/*
	 * go through the list and eliminate all the items that
	 * aren't being used.
	 */
	while (newSegList != NULL)
	  {
	    /*
	     * remember the next item
	     */
	    nextSeg = newSegList->next_seg;

	    /*
	     * Can I get rid of this element?
	     */
	    if (_CEIsElement(newSegList->seg_type) &&
		    _CEElementOfSegment(newSegList)->el_type == SdlElementHead)
	      {
		/*
		 * shorten up the chain
		 */
		if (prevSeg != NULL)
		    prevSeg->next_seg = nextSeg;
		else
		    newSnbEl->seg_list = nextSeg;

		_DtHelpCeFreeSdlStructs(NULL, &(newSegList), NULL);
	      }
	    else
		prevSeg = newSegList;

	    /*
	     * go to the next item
	     */
	    newSegList = nextSeg;
	  }

	/*
	 * if all the items in the snb have NOT been removed,
	 * append them onto the old snb.
	 */
	if (newSnbEl->seg_list != NULL)
	  {
	    /*
	     * does an snb exist?
	     */
            if (*snb_top == NULL)
	        *snb_top = new_snb;
            else
              {
		/*
		 * get the current list of snb's
		 */
	        newSegList = newSnbEl->seg_list;

		/*
		 * get the old snb's element.
		 */
	        oldSnbEl   = (*snb_top)->seg_handle.el_handle;

		/*
		 * if the old snb didn't have a list, make this one it.
		 */
	        if (oldSnbEl->seg_list == NULL)
	            oldSnbEl->seg_list = newSegList;
	        else
	          {
		    /*
		     * walk through the list and get the last one.
		     */
	            nextSeg = oldSnbEl->seg_list;
	            while (nextSeg->next_seg != NULL)
		        nextSeg = nextSeg->next_seg;

		    /*
		     * tag the new list onto the old one
		     */
	            nextSeg->next_seg = newSegList;
	          }

		/*
		 * free what's left of the new snb strutures
		 */
	        newSnbEl->seg_list = NULL;
		_DtHelpCeFreeSdlStructs(NULL, &(new_snb), NULL);
              }
          }
	/*
	 * empty - remove it
	 */
	else
	    _DtHelpCeFreeSdlStructs(NULL, &(new_snb), NULL);
      }
}

/******************************************************************************
 * Function:    void CreateAsciiString ()
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
CreateAsciiString(canvas, p_seg, snb, nl_flag, stop_mask, ret_string)
    CECanvasStruct	 *canvas;
    CESegment		 *p_seg;
    CEElement		 *snb;
    CEBoolean		 *nl_flag;
    SDLMask		 *stop_mask;
    char		**ret_string;
#else
CreateAsciiString(
    CECanvasStruct	 *canvas,
    CESegment		 *p_seg,
    CEElement		 *snb,
    CEBoolean		 *nl_flag,
    SDLMask		 *stop_mask,
    char		**ret_string)
#endif /* _NO_PROTO */
{
    int		 len;
    CEElement	*pEl;

    while (p_seg != NULL)
      {
	len = 1;
	if (_CEIsString(p_seg->seg_type))
	  {
	    if (*nl_flag == True)
	      {
		if (*ret_string != NULL &&
				    (*ret_string)[strlen(*ret_string)-1] != ' ')
		  len++;
		else
		  *nl_flag = False;
	      }

	    len += strlen(_CEStringOfSegment(p_seg));

	    if (*ret_string == NULL)
	      {
		*ret_string  = (char *) malloc (len);
		**ret_string = '\0';
	      }
	    else
	      {
		len += strlen(*ret_string);
		*ret_string = (char *) realloc (*ret_string, len);
	      }

	    if (*nl_flag == True)
		strcat(*ret_string, " ");

	    strcat (*ret_string, _CEStringOfSegment(p_seg));

	    *nl_flag = False;
	    if (p_seg->next_disp == NULL)
		*nl_flag = True;
	  }
	else if (_CEIsElement(p_seg->seg_type))
	  {
	    pEl = _CEElementOfSegment(p_seg);

	    if (pEl->el_type == SdlElementSnRef)
	      {
	        _DtHelpCeResolveSdlSnref(canvas, NULL, snb, p_seg, NULL);
	        CreateAsciiString(canvas,pEl->seg_list,snb, nl_flag,
							stop_mask, ret_string);
	      }
	    else if (CESearchSDLMask(stop_mask, pEl->el_type) != True)
	        CreateAsciiString(canvas, pEl->seg_list, snb, nl_flag,
							stop_mask, ret_string);
	  }

	p_seg = p_seg->next_seg;
      }
}

/******************************************************************************
 * Function:    void CreateAsciiAbbrev ()
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
CreateAsciiAbbrev(p_el, ret_abbrev)
    CEElement		 *p_el;
    char		**ret_abbrev;
#else
CreateAsciiAbbrev(
    CEElement		 *p_el,
    char		**ret_abbrev)
#endif /* _NO_PROTO */
{
  *ret_abbrev = NULL;
  if (_CEIsAttrSet(p_el->attributes, SDL_ATTR_ABBREV)
			&& p_el->attributes.strings.el_strs.abbrev != NULL
			&& strlen (p_el->attributes.strings.el_strs.abbrev))
	*ret_abbrev = strdup(p_el->attributes.strings.el_strs.abbrev);
}

/******************************************************************************
 * Function:    void CleanUpToc ()
 *
 * Parameters:
 *		my_struct	Specifies current formatting information.
 *		p_seg		Specifies the current segment list to modify.
 *		level		Specifes the parent element's level.
 *		lnk_indx	Specifies the link index to use for the
 *				ghost link.
 *
 * Returns:     nothing
 *
 * Purpose:     Modifies the formatting information for conform to what
 *		it should be for a TOC and sets the ghost link.
 *
 ******************************************************************************/
static	void
#ifdef _NO_PROTO
CleanUpToc(p_seg, target)
    CESegment		*p_seg;
    CEBoolean		 target;
#else
CleanUpToc(
    CESegment		*p_seg,
    CEBoolean		 target)
#endif /* _NO_PROTO */
{
    CEElement	*pNewEl;

    while (p_seg != NULL)
      {
	if (_CEIsNewLine(p_seg->seg_type))
	    p_seg->seg_type = _CEClearNewline(p_seg->seg_type);

	if (_CEIsElement(p_seg->seg_type))
	  {
	    pNewEl = _CEElementOfSegment(p_seg);

	    if (_CEIsAttrSet(pNewEl->attributes, SDL_ATTR_HEADWDTH))
	      {
		free(pNewEl->attributes.headw);
		ClearAttrFlag(pNewEl->attributes, SDL_ATTR_HEADWDTH);
	      }

	    pNewEl->attributes.headw              = (char *)TenThousandStr;
	    pNewEl->attributes.orient             = SdlJustifyLeftMargin;
	    pNewEl->attributes.flow               = SdlNoWrap;
	    pNewEl->attributes.type               = SdlTypeLiteral;
	    pNewEl->attributes.font_specs.pointsz = 10;
	    pNewEl->attributes.font_specs.weight  = CEFontWeightMedium;
	    pNewEl->attributes.frmt_specs.justify = SdlJustifyLeft;
	    pNewEl->attributes.frmt_specs.vjust   = SdlJustifyTop;
	    pNewEl->attributes.frmt_specs.lmargin = 0;
	    pNewEl->attributes.frmt_specs.tmargin = 0;
	    pNewEl->attributes.frmt_specs.bmargin = 0;
	    pNewEl->attributes.frmt_specs.rmargin = 0;
	    pNewEl->attributes.frmt_specs.fmargin = 0;

	    if (target == True)
	        pNewEl->attributes.font_specs.weight = CEFontWeightBold;

	    CleanUpToc(pNewEl->seg_list, target);
	  }
	p_seg = p_seg->next_seg;
      }
}

/******************************************************************************
 * Function:    int AddEntryToToc ()
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
AddEntryToToc(canvas, my_struct, volume, fd, lang, char_set, toss, minor_no,
			id_el, target,
			ret_snb, seg_list, prev_list)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    VolumeHandle	 volume;
    int			 fd;
    char		*lang;
    const char		*char_set;
    CESegment		*toss;
    short		 minor_no;
    CEElement		*id_el;
    CEBoolean		 target;
    CESegment		**ret_snb;
    CESegment		**seg_list;
    CESegment		**prev_list;
#else
AddEntryToToc(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    VolumeHandle	 volume,
    int			 fd,
    char		*lang,
    const char		*char_set,
    CESegment		*toss,
    short		 minor_no,
    CEElement		*id_el,
    CEBoolean		 target,
    CESegment		**ret_snb,
    CESegment		**seg_list,
    CESegment		**prev_list)
#endif /* _NO_PROTO */
{
    int		result;
    CELinkData	saveLinks;

    /*
     * initialize the structure
     */
    saveLinks = my_struct->my_links;

    if (SetUp (NULL, NULL, my_struct, &DefAttrStruct, toss, fd, False, False) != 0)
	return -1;

    DefAttrStruct.language = lang;
    DefAttrStruct.charset  = (char *) char_set;
    my_struct->my_links    = saveLinks;

    result = FormatSDLTitle(canvas, _DtHelpCeGetVolumeName(volume),
			id_el->attributes.num_vals.offset, fd, my_struct);
    /*
     * if no errors, continue
     */
    if (result != -1)
      {
	CEElement *headEl  = NULL;
	CESegment *nextSeg = my_struct->seg_list;

	/*
	 * find the actual head element
	 */
	if (nextSeg != NULL)
	    headEl = SearchForElement(my_struct->seg_list, SdlElementHead);

	/*
	 * If there isn't a head element, use the id.
	 */
	if (headEl == NULL)
	  {
	    CESegment *addSeg = NULL;
	    CEElement *virEl  = NULL;

	    if (MySaveString(&(addSeg), my_struct, RIdString(id_el),
					-1, my_struct->mb_len, False) != 0)
	      {
		_DtHelpCeFreeSdlStructs(canvas, &(my_struct->seg_list),
						&(my_struct->my_links));
		return -1;
	      }

	    /*
	     * if the list is not empty, look for the virpage list.
	     */
	    if (nextSeg != NULL)
		virEl = SearchForElement(nextSeg, SdlElementTitle);

	    /*
	     * if there is a virpage, attach the segment to it.
	     */
	    if (virEl != NULL)
	      {
		nextSeg = virEl->seg_list;
		if (nextSeg == NULL)
		    virEl->seg_list = addSeg;
		else
		  {
		    while (nextSeg->next_seg != NULL)
			nextSeg = nextSeg->next_seg;
		    nextSeg->next_seg = addSeg;
		  }
	      }
	    else
		my_struct->seg_list = addSeg;

	    nextSeg = my_struct->seg_list;
	  }

	/*
	 * there was a empty head, use the abbreviation or the id.
	 */
	else if (headEl != NULL && headEl->seg_list == NULL)
	  {
	    char *myPtr = RIdString(id_el);

	    if (_CEIsAttrSet(headEl->attributes, SDL_ATTR_ABBREV)
		&& headEl->attributes.strings.el_strs.abbrev != NULL
		&& strlen(headEl->attributes.strings.el_strs.abbrev))
		myPtr = headEl->attributes.strings.el_strs.abbrev;

	    if (_DtHelpCeSaveString(&(headEl->seg_list), &(my_struct->last_seg),
				&(my_struct->prev_data), myPtr,
				&(headEl->attributes.font_specs),
				-1,
				my_struct->mb_len, my_struct->flags, False) !=0)
	      {
		_DtHelpCeFreeSdlStructs(canvas, &(my_struct->seg_list),
						&(my_struct->my_links));
		return -1;
	      }
	  }

	if (nextSeg != NULL)
	  {
	    CEElement *tmpEl;
	    int	level;
	    int	linkIndex = _DtHelpCeAddLinkToList(&(my_struct->my_links),
				strdup(RIdString(id_el)),
					CELinkType_SameVolume,
					CEWindowHint_CurrentWindow, NULL);

	    CleanUpToc(nextSeg, target);
	    SetGhostLink(&(my_struct->my_links), nextSeg, linkIndex);

	    while (nextSeg != NULL)
	      {
		if (_CEIsElement(nextSeg->seg_type))
		  {
		    tmpEl = _CEElementOfSegment(nextSeg);
		    level  = LevelAttr(id_el);
		    if (minor_no == SDL_DTD_1_1)
			level--;

		    if (level < 0)
			level = 0;
		    tmpEl->attributes.frmt_specs.lmargin = 2 * level;
		  }
		nextSeg = nextSeg->next_seg;
	      }
	  }
      }

    if (result != -1)
      {
        /*
         * now tack this segment onto the end of the list
         */
        if ((*seg_list) == NULL)
	    (*seg_list) = my_struct->seg_list;
        else
	    (*prev_list)->next_seg = my_struct->seg_list;

        *prev_list = my_struct->seg_list;
        while ((*prev_list) != NULL && (*prev_list)->next_seg != NULL)
	    *prev_list = (*prev_list)->next_seg;
      }

    /*
     * if the snb for this topic was read, only keep the ones used.
     */
    if (my_struct->snb != NULL)
      {
	CompressAndIntegrateSnb(ret_snb, my_struct->snb);
	my_struct->snb = NULL;
      }

    return result;
}

/******************************************************************************
 * Function:    int ExpandToc ()
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
ExpandToc(canvas, my_struct, volume, fd, lang, char_set, toss, level, minor_no,
			path_list, id_seg, ret_snb, seg_list, prev_list)
    CECanvasStruct	*canvas;
    _CEFormatStruct	*my_struct;
    VolumeHandle	 volume;
    int			 fd;
    char		*lang;
    const char		*char_set;
    CESegment		*toss;
    int			 level;
    short		 minor_no;
    char		**path_list;
    CESegment		**id_seg;
    CESegment		**ret_snb;
    CESegment		**seg_list;
    CESegment		**prev_list;
#else
ExpandToc(
    CECanvasStruct	*canvas,
    _CEFormatStruct	*my_struct,
    VolumeHandle	 volume,
    int			 fd,
    char		*lang,
    const char		*char_set,
    CESegment		*toss,
    int			 level,
    short		 minor_no,
    char		**path_list,
    CESegment		**id_seg,
    CESegment		**ret_snb,
    CESegment		**seg_list,
    CESegment		**prev_list)
#endif /* _NO_PROTO */
{
    int		 result   = 0;
    int		 segLev;
    int		 tst      = 1;
    CEElement	*pEl;
    CEBoolean	 done     = False;
    CEBoolean	 found    = False;

    /*
     * skip anything that isn't a virpage and of the correct level.
     */
    while ((*id_seg) != NULL &&
	_CEElementOfSegment(*id_seg)->attributes.type != SdlIdVirpage &&
			LevelAttr(_CEElementOfSegment(*id_seg)) != level)
	*id_seg = (*id_seg)->next_seg;

    /*
     * process any virpage that has the correct level
     */
    while ((*id_seg) != NULL && done == False && result == 0)
      {
	pEl    = _CEElementOfSegment(*id_seg);
	segLev = LevelAttr(pEl);

	if (pEl->attributes.type == SdlIdVirpage)
	  {
	    if (segLev == level)
	      {
		/*
		 * If the virpage in the path list has not been found,
		 * test the next virpage. Otherwise skip.
		 */
		if (found == False && *path_list != NULL &&
							IdString(pEl) != NULL)
		    tst   = _DtHelpCeStrCaseCmp(*path_list,IdString(pEl));
			
		/*
		 * the only time tst == 0 is when the next item in the
		 * path_list matches this element. Check to see if the
		 * next item in the path_list is null. If so, that
		 * means this is the location the user has desired.
		 */
	        result = AddEntryToToc(canvas, my_struct, volume, fd,
			lang, char_set, toss, minor_no, pEl,
			(tst == 0 && path_list[1] == NULL ? True : False),
			ret_snb, seg_list, prev_list);

		/*
		 * increment the segment pointer to the next item
		 */
		*id_seg = (*id_seg)->next_seg;

		/*
		 * The only time tst is zero is if the current virpage
		 * matches the next item in the list. Expand it's children.
		 * and set tst to non-zero so that AddEntryToToc does not
		 * special case the siblings following this one.
		 */
		if (tst == 0)
		  {
		    result = ExpandToc(canvas, my_struct, volume, fd, lang,
					char_set, toss,
					segLev + 1,
					minor_no,
					&path_list[1],
					id_seg, ret_snb, seg_list, prev_list);
		    found = True;
		    tst   = 1;
		  }
	      }
	    else if (segLev < level)
		done = True;
	    else
	        *id_seg = (*id_seg)->next_seg;
	  }
	else
	    *id_seg = (*id_seg)->next_seg;
      }

    return result;

} /* ExpandToc */

/******************************************************************************
 * Function:    void CreateTitleChunks ()
 *
 * Parameters:
 *
 * Returns:
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static	int
#ifdef _NO_PROTO
CreateTitleChunks(canvas, toss, p_seg, snb, stop_mask,
			lang, char_set, font_spec,
			ret_cnt, ret_chunks)
    CECanvasStruct	  *canvas;
    CESegment		  *toss;
    CESegment		  *p_seg;
    CEElement		  *snb;
    SDLMask		  *stop_mask;
    char		  *lang,
    char		  *char_set,
    CanvasFontSpec	  *font_specs,
    int			  *ret_cnt;
    void		***ret_chunks;
#else
CreateTitleChunks(
    CECanvasStruct	  *canvas,
    CESegment		  *toss,
    CESegment		  *p_seg,
    CEElement		  *snb,
    SDLMask		  *stop_mask,
    char		  *lang,
    char		  *char_set,
    CanvasFontSpec	  *font_specs,
    int			  *ret_cnt,
    void		***ret_chunks)
#endif /* _NO_PROTO */
{
    int		 type;
    int		 cnt;
    int		 result = 0;
    char	*myLang;
    char	*mySet;
    void	*ptr;
    CEElement	*pEl;
    CESpecial   *mySpc;

    while (result != -1 && p_seg != NULL)
      {
	myLang = lang;
	mySet  = char_set;
	cnt = *ret_cnt;
	if (_CEIsString(p_seg->seg_type))
	  {
	    if (*ret_cnt == 0)
		*ret_cnt = 1;
	    else
		cnt--;

	    *ret_cnt = *ret_cnt + 3;
	    if (*ret_chunks == NULL)
		*ret_chunks = (void **) malloc (sizeof(void *) * (*ret_cnt));
	    else
		*ret_chunks = (void **) realloc (*ret_chunks,
						sizeof(void *) * (*ret_cnt));
	    if (*ret_chunks == NULL)
		return -1;

	    type = DT_HELP_CE_CHARSET | DT_HELP_CE_STRING;
	    if (_CEIsInitialized(p_seg->seg_type))
	      {
		type = DT_HELP_CE_FONT_PTR | DT_HELP_CE_STRING;
		ptr  = _CEFontOfSegment(p_seg);
	      }
	    else
	      {
		int   len = 0;
                char *strPtr;

		len = strlen(char_set) + 1;
		if (lang != NULL)
		    len += (strlen(lang) + 1);

		strPtr = (char *) malloc (len);
		if (strPtr == NULL)
		    return -1;

		*strPtr = '\0';
		if (lang != NULL)
		  {
		    strcpy(strPtr, lang);
		    strcat(strPtr, ".");
		  }
		strcat(strPtr, char_set);
		ptr = (void *) strPtr;
	      }

	    if (p_seg->next_disp == NULL || _CEIsNewLine(p_seg->seg_type))
		type |= DT_HELP_CE_NEWLINE;

	    (*ret_chunks)[cnt++] = (void *) type;
	    (*ret_chunks)[cnt++] = (void *) ptr;
	    (*ret_chunks)[cnt++] = (void *) strdup(_CEStringOfSegment(p_seg));
	    if ((*ret_chunks)[cnt-1] == NULL)
		return -1;

	    (*ret_chunks)[cnt++] = (void *) DT_HELP_CE_END;
	  }
	else if (_CEIsSpecial(p_seg->seg_type))
	  {
	    if (_CEIsInitialized(p_seg->seg_type))
	      {
		if (_CEIsValidSpc(p_seg->seg_type))
		  {
		    mySpc = _CESpcOfSegment(p_seg);
		    p_seg->seg_type = _CEClearInitialized(p_seg->seg_type);
		  }
		else
		    result = -1;
	      }
	    else if (_DtHelpCeResolveSpc(canvas, p_seg->seg_handle.file_handle,
				lang, char_set, font_specs, &mySpc) != 0)
		result = -1;

	    if (result == 0)
	      {
	        if (*ret_cnt == 0)
		    *ret_cnt = 1;
	        else
		    cnt--;

	        *ret_cnt = *ret_cnt + 2;
	        if (*ret_chunks == NULL)
		    *ret_chunks = (void **) malloc (sizeof(void *) * *ret_cnt);
	        else
		    *ret_chunks = (void **) realloc (*ret_chunks,
						sizeof(void *) * *ret_cnt);
	        if (*ret_chunks == NULL)
		    return -1;

	        type = DT_HELP_CE_SPC;
	        if (p_seg->next_disp == NULL || _CEIsNewLine(p_seg->seg_type))
		    type |= DT_HELP_CE_NEWLINE;

	        (*ret_chunks)[cnt++] = (void *) type;
	        (*ret_chunks)[cnt++] = (void *) mySpc->spc_handle;
	        (*ret_chunks)[cnt++] = (void *) DT_HELP_CE_END;

	        free(mySpc);
	      }
	    result = 0;
	  }
	else if (_CEIsElement(p_seg->seg_type))
	  {
	    pEl = _CEElementOfSegment(p_seg);

	    if (_CEIsStringAttrSet(pEl->attributes, SDL_ATTR_LANGUAGE))
		myLang = pEl->attributes.language;
	    if (_CEIsStringAttrSet(pEl->attributes, SDL_ATTR_CHARSET))
		mySet = pEl->attributes.charset;

	    if (pEl->el_type == SdlElementSnRef)
	      {
	        _DtHelpCeResolveSdlSnref(canvas, toss, snb, p_seg, NULL);
	        result = CreateTitleChunks(canvas, toss, pEl->seg_list,
				snb, stop_mask,
				myLang, mySet, &(pEl->attributes.font_specs),
				ret_cnt, ret_chunks);
	      }
	    else if (CESearchSDLMask(stop_mask, pEl->el_type) != True)
	        result = CreateTitleChunks(canvas, toss, pEl->seg_list, snb,
				stop_mask,
				myLang, mySet, &(pEl->attributes.font_specs),
				ret_cnt, ret_chunks);
	  }
	p_seg = p_seg->next_seg;
      }

    return result;
}

/******************************************************************************
 * Function:    int ProcessSegmentsToChunks ()
 *
 * Parameters:
 *
 * Returns:	0 if created a chunk, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 *
 ******************************************************************************/
static int
#ifdef _NO_PROTO
ProcessSegmentsToChunks(canvas, toss, head_el, snb_el,
			stop_mask, lang, char_set,
			ret_chunks)
    CECanvasStruct	  *canvas;
    CESegment		  *toss;
    CEElement		  *head_el;
    CEElement		  *snb_el;
    SDLMask		  *stop_mask;
    char		  *lang;
    char		  *char_set;
    void		***ret_chunks;
#else
ProcessSegmentsToChunks(
    CECanvasStruct	  *canvas,
    CESegment		  *toss,
    CEElement		  *head_el,
    CEElement		  *snb_el,
    SDLMask		  *stop_mask,
    char		  *lang,
    char		  *char_set,
    void		***ret_chunks)
#endif /* _NO_PROTO */
{
    int cnt    = 0;
    int result = 0;

    if (_CEIsAttrSet(head_el->attributes, SDL_ATTR_LANGUAGE))
	lang = head_el->attributes.language;
    if (_CEIsAttrSet(head_el->attributes, SDL_ATTR_CHARSET))
	char_set = head_el->attributes.charset;

    result = CreateTitleChunks(canvas, toss, head_el->seg_list,
					snb_el, stop_mask, lang, char_set,
					&(head_el->attributes.font_specs),
					&cnt, ret_chunks);
    if ((result != 0 || cnt == 0)
    		&& _CEIsAttrSet(head_el->attributes, SDL_ATTR_ABBREV)
		&& head_el->attributes.strings.el_strs.abbrev != NULL
		&& strlen(head_el->attributes.strings.el_strs.abbrev))
      {
	*ret_chunks   = (void **) malloc (sizeof(void *) * 4);
	if (*ret_chunks == NULL)
	    return -1;

	(*ret_chunks)[0] = (void *) DT_HELP_CE_CHARSET;
	(*ret_chunks)[1] = (void *) strdup(char_set);
	(*ret_chunks)[2] = (void *)
			strdup(head_el->attributes.strings.el_strs.abbrev);
	(*ret_chunks)[3] = (void *) DT_HELP_CE_END;
      }

    return result;
}

/******************************************************************************
 * Function:    CEElement *GetSdlDocSnb (
 *				VolumeHandle    volume)
 * Parameters:
 *		volume		Specifies the volume.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:	If the title has been parsed and it used snrefs,
 *		this function will return the snb specified in the
 *		sdldoc element.
 ******************************************************************************/
static CEElement *
#ifdef _NO_PROTO
GetSdlDocSnb(canvas, volume)
    VolumeHandle	 volume;
#else
GetSdlDocSnb(
    VolumeHandle	 volume)
#endif /* _NO_PROTO */
{
    CEElement	*retEl = NULL;
    CESDLVolume	*sdlVol;

    sdlVol = _DtHelpCeGetSdlVolumePtr(volume);
    if (sdlVol != NULL && sdlVol->snb != NULL)
	retEl = _CEElementOfSegment(sdlVol->snb);

    return retEl;
}

/******************************************************************************
 * Function:    int GetSdlDocTitle (
 *				CECanvasStruct *canvas,
 *				VolumeHandle    volume,
 *				CEBoolean	  flag,
 *				CESegment	**ret_seg)
 * Parameters:
 *		canvas		Specifies the canvas to work with.
 *		volume		Specifies the volume.
 *		flag		Specifies if the toss is needed.
 *		ret_seg		Returns sdlVol->title.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:	This will fill in the 'title' and 'snb' elements of
 *		the CESDLVolume structure and return 'title' in 'ret_seg'.
 ******************************************************************************/
static int
#ifdef _NO_PROTO
GetSdlDocTitle(canvas, volume, flag, ret_seg)
    CECanvasStruct	*canvas;
    VolumeHandle	 volume;
    CEBoolean		 flag;
    CESegment		**ret_seg;
#else
GetSdlDocTitle(
    CECanvasStruct	*canvas,
    VolumeHandle	 volume,
    CEBoolean		 flag,
    CESegment		**ret_seg)
#endif /* _NO_PROTO */
{
    int			 result = -1;
    CESDLVolume		*sdlVol;
    _CEFormatStruct	 frmtStruct;
    CESegment		*toss = NULL;
    SDLMask		 skipMask   [SDL_MASK_LEN] = CEClearAllBits;
    SDLMask		 processMask[SDL_MASK_LEN] = CESetAllBits;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);

    sdlVol = _DtHelpCeGetSdlVolumePtr(volume);
    if (sdlVol == NULL)
	return -1;

    if (sdlVol->title_processed == False)
      {
	if (flag == True)
	    toss = _DtHelpCeGetSdlVolToss(volume, -1);

	if (SetUp (volume, NULL, &frmtStruct, &DefAttrStruct, toss, -1, True, False) != 0)
	    return -1;

	if (_DtHelpCeFileOpenAndSeek(_DtHelpCeGetVolumeName(volume), 0, -1,
					&(frmtStruct.my_file), NULL) != -1)
          {
	    if (ProcessContent(canvas, &frmtStruct, SDLDocumentContent,
				SdlElementNone, startExcept, skipMask) != -1
		&& ParseSDL(canvas, &frmtStruct, SdlElementVStruct,
				SdlElementNone, startExcept, skipMask) != -1
	        && ProcessContent(canvas, &frmtStruct, HeadAndSnb,
				SdlElementNone, startExcept, processMask) != -1)
	      {
		sdlVol->title = frmtStruct.seg_list;
		sdlVol->snb   = frmtStruct.snb;
		result = 0;
	      }
	    else
	       _DtHelpCeFreeSdlStructs(canvas, &(frmtStruct.seg_list), NULL);

	    _DtHelpCeBufFileClose (frmtStruct.my_file, True);
          }

        sdlVol->title_processed = True;
      }
    else if (sdlVol->title != NULL)
	result = 0;

    *ret_seg = sdlVol->title;
    return result;
}

/******************************************************************************
 *
 * Semi-Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    SDLElementAttrList *_DtHelpCeGetSdlElementAttrs (
 *						enum SdlElement element)
 *
 * Parameters:
 *
 * Returns:     ptr or NULL;
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
const SDLElementAttrList *
#ifdef _NO_PROTO
_DtHelpCeGetSdlElementAttrs(element)
    enum SdlElement	element;
#else
_DtHelpCeGetSdlElementAttrs(
    enum SdlElement	element)
#endif /* _NO_PROTO */
{
    register int i = 0;

    if (element == SdlElementBlockAsync)
	element = SdlElementBlock;

    while (i < MaxSDLElements && SDLElements[i].sdl_element != element)
	i++;

    return (SDLElements[i].attrib_list);
}

/******************************************************************************
 * Function:    SDLAttribute *_DtHelpCeGetSdlAttribEntry(int attr_define)
 *
 * Parameters:
 *
 * Returns:     ptr or NULL;
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
const SDLAttribute *
#ifdef _NO_PROTO
_DtHelpCeGetSdlAttribEntry(attr_define)
    int		attr_define;
#else
_DtHelpCeGetSdlAttribEntry(
    int		attr_define)
#endif /* _NO_PROTO */
{
    const SDLAttribute	*bigList;

    bigList = SDLAttributeList;
    while (bigList->sdl_attr_define != -1 &&
	!ATTRS_EQUAL(bigList->data_type,attr_define,bigList->sdl_attr_define))
	bigList++;

    return (bigList);
}

/******************************************************************************
 * Function:    SDLAttribute *_DtHelpCeGetSdlAttributeList(void)
 *
 * Parameters:
 *
 * Returns:     ptr or NULL;
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
const SDLAttribute *
#ifdef _NO_PROTO
_DtHelpCeGetSdlAttributeList()
#else
_DtHelpCeGetSdlAttributeList(void)
#endif /* _NO_PROTO */
{
    return (SDLAttributeList);
}

/******************************************************************************
 *
 * Semi-Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    int _DtHelpCeFrmtSDLPathAndChildren (
 *				VolumeHandle volume, char *filename,
 *                              int offset, char *id_string,
 *                              TopicHandle *ret_handle)
 *
 * Parameters:
 *              volume          Specifies the Help Volume the information
 *                              is associated with.
 *              filename        Specifies the file containing the Help Topic
 *                              desired.
 *              offset          Specifies the offset into 'filename' to
 *                              the Help Topic desired.
 *              id_string       Specifies the location id to look for or NULL.
 *              ret_handle      Returns a handle to the topic information
 *                              including the number of paragraphs and the
 *                              id match segment.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtSdlPathAndChildren(canvas_handle, volume, fd, target_id, ret_handle)
    CanvasHandle	 canvas_handle;
    VolumeHandle	 volume;
    int			 fd;
    char		*target_id;
    TopicHandle		*ret_handle;
#else
_DtHelpCeFrmtSdlPathAndChildren(
    CanvasHandle	 canvas_handle,
    VolumeHandle	 volume,
    int			 fd,
    char		*target_id,
    TopicHandle		*ret_handle)
#endif /* _NO_PROTO */
{
    char		*ptr;
    char		*lang;
    const char		*charSet;
    char		**topicMap;
    int			 result = 0;
    int			 pathCnt;
    short		 minorNo;
    _CEFormatStruct	 frmtStruct;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CETopicStruct	*topicHandle;
    CESegment		*mySegList = NULL;
    CESegment		*myPrevSeg = NULL;
    CESegment		*loids;
    CESegment		*snb    = NULL;
    CEElement		*pEl;

    *ret_handle = NULL;

    /*
     * look for the heading for each virpage
     */
    frmtStruct               = defaultFormatStruct;
    frmtStruct.my_links.max  = 0;
    frmtStruct.my_links.list = NULL;

    /*
     * get the path from the target to the top.
     */
    minorNo = _DtHelpCeGetSdlMinorNumber(volume);
    pathCnt = _DtHelpCeGetSdlIdPath(volume, target_id, &topicMap);
    if (pathCnt == -1)
	return -1;

    /*
     * get the beginning of the path
     */
    loids = _DtHelpCeMapSdlIdToSegment(volume, *topicMap, fd);
    if (loids == NULL)
      {
        _DtHelpCeFreeStringArray(topicMap);
        return -1;
      }
    
    /*
     * if this is SDL DTD 1.0.0, hidden files have an rssi starting
     * with an underscore (except top topic).
     */
    if (pathCnt == 1 && minorNo == SDL_DTD_1_0)
      {
	ptr = _DtHelpCeSdlRssiString(_CEElementOfSegment(loids));
	if (ptr != NULL && *ptr == '_' &&
		_DtHelpCeStrCaseCmp(*topicMap,
				_DtHelpCeGetSdlHomeTopicId(volume)) != 0)
	    result = 1;
      }

    if (result == 0)
      {
        /*
         * format the top topic entry.
         */
        pEl = _CEElementOfSegment(loids);
        if (minorNo == SDL_DTD_1_0 || LevelAttr(pEl) > 0)
          {
            /*
             * Put the top topic in the table of contents.
             */
            lang    = _DtHelpCeGetSdlVolLanguage(volume);
            charSet = _DtHelpCeGetSdlVolCharSet(volume);
            result  = AddEntryToToc(canvas, &frmtStruct, volume, fd,
                                    lang, charSet,
                                    _DtHelpCeGetSdlVolToss(volume, fd),
                                    minorNo,
                                    pEl,
                                    (topicMap[1] == NULL ? True : False),
                                    &snb, &mySegList, &myPrevSeg);
            /*
             * format the children.
             */
            if (result != -1 && loids->next_seg != NULL)
              {
                loids  = loids->next_seg;
                result = ExpandToc(canvas, &frmtStruct, volume, fd,
				    lang, charSet,
                                    _DtHelpCeGetSdlVolToss(volume, fd),
                                    LevelAttr(pEl) + 1,
                                    minorNo,
                                    &topicMap[1],
                                    &loids, &snb, &mySegList, &myPrevSeg);
              }
          }
      }
    else
	result = 0;

    if (result != -1)
      {
	topicHandle = (CETopicStruct *) malloc (sizeof(CETopicStruct));
	if (topicHandle != NULL)
	  {
	    /*
	     * if one or more headers used a snref, attach the snb that
	     * resolves them.
	     */
	    if (snb != NULL)
	      {
		snb->next_seg = mySegList;
		mySegList = snb;
	      }

	    topicHandle->name      = _DtHelpCeGetVolumeName(volume);
	    topicHandle->para_num  = 0;
	    topicHandle->para_list = NULL;
	    topicHandle->id_seg    = NULL;
	    topicHandle->id_str    = NULL;
	    topicHandle->link_data = frmtStruct.my_links;

	    topicHandle->element_list = (void *) mySegList;
	    topicHandle->toss         = _DtHelpCeGetSdlVolToss(volume, fd);

	    *ret_handle = (void *) topicHandle;
	  }
	else
	    result = -1;
      }

    _DtHelpCeFreeStringArray(topicMap);
    return result;

} /* End _DtHelpCeFrmtSDLPathAndChildren */

/******************************************************************************
 * Function:    int _DtHelpCeFrmtSdlVolumeInfo (char *filename,
 *                              TopicHandle *ret_handle)
 *
 * Parameters:
 *              filename        Specifies the file containing the Help Topic
 *                              desired.
 *              ret_handle      Returns a handle to the topic information
 *                              including the number of paragraphs and the
 *                              id match segment.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     _DtHelpCeFrmtSdlVolumeInfo processes the SDL volume looking
 *		for the generated items.
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtSdlVolumeInfo(canvas_handle, filename, volume, ret_time)
    CanvasHandle	 canvas_handle;
    char		*filename;
    VolumeHandle	 volume;
    time_t		*ret_time;
#else
_DtHelpCeFrmtSdlVolumeInfo(
    CanvasHandle	 canvas_handle,
    char		*filename,
    VolumeHandle	 volume,
    time_t		*ret_time)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    char		*numPtr;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CESDLVolume		*sdlVol;
    CEElement		*sdlEl;
    _CEFormatStruct	 frmtStruct;
    SDLMask		 processMask[SDL_MASK_LEN] = CESetAllBits;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);

    if (SetUp (volume, &sdlVol, &frmtStruct, &DefAttrStruct, NULL, -1, True, False) != 0)
	return -1;

    if (result == 0)
      {
	result = _DtHelpCeFileOpenAndSeek(filename, 0, -1,
					&(frmtStruct.my_file), ret_time);
	if (result != -1)
	  {
	    result = ProcessContent(canvas, &frmtStruct, SDLDocumentContent,
				SdlElementNone, startExcept, processMask);

	    _DtHelpCeBufFileClose (frmtStruct.my_file, True);

	    if (frmtStruct.remember != NULL)
		free(frmtStruct.remember);
	  }
    
	if (result != -1)
	  {
	    sdlVol->sdl_info = frmtStruct.seg_list;
	    sdlEl            = _DtHelpCeFindSdlElement(sdlVol->sdl_info,
							SdlElementSdlDoc, True);
	    numPtr = sdlEl->attributes.strings.doc_strs.sdldtd;

	    while (*numPtr < '0' || *numPtr > '9')
	        numPtr++;

	    if (atoi(numPtr) != SDL_DTD_VERSION)
		return -1;

	    while (*numPtr != '.' && *numPtr != '\0')
	        numPtr++;

	    if (*numPtr == '.')
	        numPtr++;

	    sdlVol->minor_no = atoi(numPtr);
	  }
      }

    return result;
}

/******************************************************************************
 * Function:    char *_DtHelpCeGetInterpCmd (enum SdlOption interp_type);
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     Return the command associated with the interpreter type.
 *
 ******************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeGetInterpCmd(interp_type)
    enum SdlOption	interp_type;
#else
_DtHelpCeGetInterpCmd(
    enum SdlOption	interp_type)
#endif /* _NO_PROTO */
{
    const _CEInterpData *interp = InterpData;

    while (interp->type != SdlOptionBad)
      {
	if (interp->type == interp_type)
	    return interp->cmd;

	interp++;
      }

    return NULL;

}

/******************************************************************************
 * Function:    int _DtHelpCeFrmtSDLTitleToAscii (
 *				char *filename,
 *                              int offset,
 *                              char **ret_title, char **ret_abbrev)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtSDLTitleToAscii(canvas_handle, volume, offset,
				ret_title, ret_abbrev)
    CanvasHandle canvas_handle;
    VolumeHandle        volume;
    int		 offset;
    char	*ret_title;
    char	*ret_abbrev;
#else
_DtHelpCeFrmtSDLTitleToAscii(
    CanvasHandle canvas_handle,
    VolumeHandle        volume,
    int		 offset,
    char	**ret_title,
    char	**ret_abbrev)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    _CEFormatStruct	 frmtStruct;
    SDLMask		 stopMask[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementSubHead);

    if (SetUp (volume, NULL, &frmtStruct, &DefAttrStruct, NULL, -1, True, False) != 0)
	return -1;

    *ret_title = NULL;
    if (ret_abbrev != NULL)
	*ret_abbrev = NULL;

    result = FormatSDLTitle(canvas, _DtHelpCeGetVolumeName(volume),
						offset, -1, &frmtStruct);

    if (result != -1 && frmtStruct.seg_list != NULL)
      {
	CESegment  *pHeadSeg;
	CEElement  *pVirEl;
	CEElement  *pSnbEl = NULL;
	CEBoolean   nlFlag = False;

	pVirEl   = _CEElementOfSegment(frmtStruct.seg_list);
	pHeadSeg = pVirEl->seg_list;

	if (pHeadSeg != NULL)
	  {
	    if (frmtStruct.snb != NULL)
	        pSnbEl = _CEElementOfSegment(frmtStruct.snb);

	    CreateAsciiString(canvas, _CEElementOfSegment(pHeadSeg)->seg_list,
				pSnbEl, &nlFlag, stopMask, ret_title);
	    CreateAsciiAbbrev(_CEElementOfSegment(pHeadSeg), ret_abbrev);
	  }
	else
	    result = -1;
      }
    else
	result = -1;

    if (frmtStruct.seg_list != NULL)
	_DtHelpCeFreeSdlStructs(canvas, &(frmtStruct.seg_list),
						&(frmtStruct.my_links));

    return result;

} /* End _DtHelpCeFrmtSDLTitleToAscii */

/******************************************************************************
 * Function:    char *_DtHelpCeFrmtSdlVolumeAbstractToAscii(
 *							VolumeHandle volume)
 *
 * Parameters:
 *              volume          Specifies the Help Volume the information
 *                              is associated with.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     _DtHelpCeFrmtSdlVolumeAbstractToAscii formats Help Files
 *		with formatting information into a CEVirtualPage
 *
 ******************************************************************************/
char *
#ifdef _NO_PROTO
_DtHelpCeFrmtSdlVolumeAbstractToAscii(canvas_handle, volume)
    CanvasHandle	  canvas_handle;
    VolumeHandle        volume;
#else
_DtHelpCeFrmtSdlVolumeAbstractToAscii(
    CanvasHandle	canvas_handle,
    VolumeHandle        volume)
#endif /* _NO_PROTO */
{
    int			 offset;
    char		*abstr = NULL;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CETopicStruct	*topicHandle;
    CESegment		*pSeg;
    CEElement		*pSnb;
    SDLMask		 stopMask[SDL_MASK_LEN] = CEClearAllBits;
    CEBoolean		 nlFlag = False;
    _DtHelpCeLockInfo    lockInfo;

    if (_DtHelpCeLockVolume(canvas_handle, volume, &lockInfo) != 0)
	return NULL;

    if (_DtHelpCeFindSdlId(volume,"_abstract",lockInfo.fd,NULL,&offset) == True)
      {
        if (_DtHelpCeParseSdlTopic(canvas, volume, lockInfo.fd,
					offset, NULL, False,
					(TopicHandle) &topicHandle) != -1)
	  {
	    pSeg = (CESegment *)(topicHandle->element_list);
	    pSnb = SearchForElement(pSeg, SdlElementSnb);

	    CreateAsciiString(canvas, pSeg, pSnb, &nlFlag, stopMask, &abstr);

	    if (topicHandle->element_list != NULL)
		_DtHelpCeFreeSdlStructs(canvas, &pSeg,
						&(topicHandle->link_data));
	    free(topicHandle);
	  }
      }
    _DtHelpCeUnlockVolume(lockInfo);

    return abstr;

} /* End _DtHelpCeFrmtSdlVolumeAbstractToAscii */

/******************************************************************************
 * Function:    int _DtHelpCeFrmtSDLVolTitleToAscii (
 *				char *filename,
 *                              int offset,
 *                              char **ret_title, char **ret_abbrev)
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeFrmtSDLVolTitleToAscii(canvas_handle, volume, ret_title)
    CanvasHandle canvas_handle;
    VolumeHandle volume;
    char	*ret_title;
#else
_DtHelpCeFrmtSDLVolTitleToAscii(
    CanvasHandle canvas_handle,
    VolumeHandle volume,
    char	**ret_title)
#endif /* _NO_PROTO */
{
    char		*abbrev;
    int			 result  = 0;
    CECanvasStruct	*canvas  = (CECanvasStruct *) canvas_handle;
    CEElement		*pEl;
    CESegment		*pHeadSeg;
    _DtHelpCeLockInfo    lockInfo;
    SDLMask		 stopMask[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementSubHead);

    *ret_title = NULL;

    /*
     * get the head element
     */
    if (_DtHelpCeLockVolume(canvas, volume, &lockInfo) != 0)
	return -1;

    result = GetSdlDocTitle(canvas, volume, False, &pHeadSeg);
    _DtHelpCeUnlockVolume(lockInfo);

    if (result == 0)
      {
        if (pHeadSeg != NULL && _CEElementOfSegment(pHeadSeg)->seg_list != NULL)
          {
	    CEBoolean   nlFlag = False;

	    pEl      = _CEElementOfSegment(pHeadSeg);
	    pHeadSeg = pEl->seg_list;

	    CreateAsciiAbbrev(pEl, &abbrev);

	    if (abbrev != NULL && *abbrev != '\0')
		*ret_title = abbrev;
	    else
	      {
	        CreateAsciiString(canvas, pHeadSeg, GetSdlDocSnb(volume),
						&nlFlag, stopMask, ret_title);
	        if (abbrev != NULL)
		    free(abbrev);
	      }
          }
        else if (_DtHelpCeGetTopicTitle(canvas,volume,"_title",ret_title) != 0)
	    result = _DtHelpCeGetTopicTitle(canvas,volume, "_hometopic",
								ret_title);
      }

    return result;

} /* End _DtHelpCeFrmtSDLVolTitleToAscii */

/******************************************************************************
 * Function:    int _DtHelpCeGetSdlTopicTitleChunks (
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlTitleChunks(canvas_handle, volume, loc_id, ret_chunks)
    CanvasHandle	  canvas_handle;
    VolumeHandle	  volume;
    char		  *loc_id;
    void		***ret_chunks;
#else
_DtHelpCeGetSdlTitleChunks(
    CanvasHandle	  canvas_handle,
    VolumeHandle	  volume,
    char		  *loc_id,
    void		***ret_chunks)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    int			 offset;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    _CEFormatStruct	 frmtStruct;
    CESDLVolume		*sdlVol;
    SDLMask		 stopMask[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementSubHead);

    *ret_chunks = NULL;
    if (_DtHelpCeFindSdlId(volume, loc_id, -1, NULL, &offset) != True)
	return -1;

    if (SetUp(volume,&sdlVol,&frmtStruct,&DefAttrStruct,NULL,-1,True,True) != 0)
	return -1;

    result = FormatSDLTitle(canvas, _DtHelpCeGetVolumeName(volume),
					offset, -1, &frmtStruct);

    if (result != -1 && frmtStruct.seg_list != NULL)
      {
	CEElement  *pVirEl;
	CEElement  *headEl;
	CEElement  *pSnbEl = NULL;

	result = -1;
	pVirEl = _DtHelpCeFindSdlElement(frmtStruct.seg_list,
						SdlElementTitle, True);
	if (pVirEl != NULL)
	  {
	    headEl = _DtHelpCeFindSdlElement(pVirEl->seg_list,
						SdlElementHead, False);
	    if (headEl != NULL)
	      {
	        pSnbEl = _DtHelpCeFindSdlElement(pVirEl->seg_list,
						SdlElementSnb, False);

	        result = ProcessSegmentsToChunks(canvas,
					frmtStruct.toss,
					headEl,
					pSnbEl, stopMask,
					pVirEl->attributes.language,
					pVirEl->attributes.charset,
					ret_chunks);
	      }
	  }
      }
    else
	result = -1;

    if (frmtStruct.seg_list != NULL)
	_DtHelpCeFreeSdlStructs(canvas, &(frmtStruct.seg_list),
						&(frmtStruct.my_links));
    if (frmtStruct.snb != NULL)
	_DtHelpCeFreeSdlStructs(canvas, &(frmtStruct.snb), NULL);

    return result;

} /* End _DtHelpCeGetSdlTitleChunks */

/******************************************************************************
 * Function:    int _DtHelpCeGetSdlVolTitleChunks (
 *
 * Parameters:
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolTitleChunks(canvas_handle, volume_handle, ret_chunks)
    CanvasHandle canvas_handle;
    VolumeHandle volume_handle;
    void 	***ret_chunks;
#else
_DtHelpCeGetSdlVolTitleChunks(
    CanvasHandle canvas_handle,
    VolumeHandle volume_handle,
    void 	***ret_chunks)
#endif /* _NO_PROTO */
{
    int			 result  = 0;
    CECanvasStruct	*canvas  = (CECanvasStruct *) canvas_handle;
    CESegment		*pHeadSeg;
    CEElement		*headEl  = NULL;
    CEElement		*docEl;
    CESDLVolume		*sdlVol;
    SDLMask		 stopMask[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementSubHead);

    /*
     * get the sdl volume pointer.
     */
    sdlVol = _DtHelpCeGetSdlVolumePtr(volume_handle);
    if (sdlVol ==  NULL)
	return -1;

    /*
     * find the document attributes
     */
    docEl  = _DtHelpCeFindSdlElement(sdlVol->sdl_info, SdlElementSdlDoc, True);
    if (docEl == NULL)
	return -1;

    /*
     * get the head element
     */
    if (GetSdlDocTitle(canvas, volume_handle, True, &pHeadSeg) == -1)
	return -1;

    if (pHeadSeg != NULL)
        headEl = _CEElementOfSegment(pHeadSeg);

    /*
     * process it
     */
    if (headEl != NULL)
	result = ProcessSegmentsToChunks(canvas,
				_DtHelpCeGetSdlVolToss(volume_handle, -1),
				headEl,
				GetSdlDocSnb(volume_handle), stopMask,
				docEl->attributes.language,
				docEl->attributes.charset,
				ret_chunks);
    if (result != 0)
      {
	result = _DtHelpCeGetSdlTitleChunks(canvas_handle, volume_handle,
						"_title", ret_chunks);
	if (result != 0)
	    result = _DtHelpCeGetSdlTitleChunks(canvas_handle, volume_handle,
						"_hometopic", ret_chunks);
      }

    return result;

} /* End _DtHelpCeGetSdlVolTitleChunks */

/******************************************************************************
 * Function:    int _DtHelpCeGetSdlVolToss (
 *				VolumeHandle volume,
 * Parameters:
 *		volume		Specifies the volume to read/parse.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:	Get the toss from a volume.
 ******************************************************************************/
CESegment *
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolToss(volume, fd)
    VolumeHandle volume;
    int          fd;
#else
_DtHelpCeGetSdlVolToss(
    VolumeHandle volume,
    int          fd)
#endif /* _NO_PROTO */
{
    CESDLVolume		*sdlVol;
    _CEFormatStruct	 frmtStruct;
    SDLMask		 skipMask   [SDL_MASK_LEN] = CEClearAllBits;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);
    /*
     * get the sdl volume pointer.
     */
    sdlVol = _DtHelpCeGetSdlVolumePtr(volume);
    if (sdlVol == NULL)
	return NULL;

    if (sdlVol->toss == NULL
	&& SetUp(volume,NULL,&frmtStruct,&DefAttrStruct,NULL,fd,True,False)==0
	&& _DtHelpCeFileOpenAndSeek(_DtHelpCeGetVolumeName(volume), 0, fd,
					&(frmtStruct.my_file), NULL) != -1)
      {
	if (ProcessContent(NULL, &frmtStruct, SDLDocumentContent,
				SdlElementNone, startExcept, skipMask) != -1
	    && ParseSDL(NULL, &frmtStruct, SdlElementVStruct,
				SdlElementToss, startExcept, skipMask) != -1)
	    sdlVol->toss = frmtStruct.seg_list;

	_DtHelpCeBufFileClose (frmtStruct.my_file, (fd == -1 ? True : False));
      }

    if (sdlVol->toss != NULL)
	return (_CEElementOfSegment(sdlVol->toss)->seg_list);

    return NULL;

} /* End _DtHelpCeGetSdlVolToss */

/******************************************************************************
 * Function:    int _DtHelpCeGetSdlVolIndex (
 *				VolumeHandle volume,
 * Parameters:
 *		volume		Specifies the volume to read/parse.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:	Get the index from a volume.
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolIndex(volume)
    VolumeHandle volume;
#else
_DtHelpCeGetSdlVolIndex(
    VolumeHandle volume)
#endif /* _NO_PROTO */
{
    int			 result = -1;
    CESDLVolume		*sdlVol;
    _CEFormatStruct	 frmtStruct;
    SDLMask		 skipMask   [SDL_MASK_LEN] = CEClearAllBits;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);

    sdlVol = _DtHelpCeGetSdlVolumePtr(volume);
    if (sdlVol == NULL)
	return -1;

    if (sdlVol->index != NULL)
	return 0;

    if (SetUp(volume, NULL, &frmtStruct, &DefAttrStruct, NULL, -1, True, True) != 0)
	return -1;

    if (_DtHelpCeFileOpenAndSeek(_DtHelpCeGetVolumeName(volume), 0, -1,
					&(frmtStruct.my_file), NULL) != -1)
      {
	if (ProcessContent(NULL, &frmtStruct, SDLDocumentContent,
				SdlElementNone, startExcept, skipMask) != -1
	    && ParseSDL(NULL, &frmtStruct, SdlElementVStruct,
				SdlElementIndex, startExcept, skipMask) != -1)
	  {
	    sdlVol->index = frmtStruct.seg_list;
	    result = 0;
	  }

	_DtHelpCeBufFileClose (frmtStruct.my_file, True);
      }

    return result;

} /* End _DtHelpCeGetSdlVolIndex */

/******************************************************************************
 * Function:    int _DtHelpCeGetSdlVolIds (
 *				VolumeHandle volume,
 *				CESegment **ret_ids
 * Parameters:
 *		volume		Specifies the volume to read/parse.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * Purpose:	Get the loids from a volume.
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeGetSdlVolIds(volume, fd, ret_ids)
    VolumeHandle	  volume;
    int			  fd;
    CESegment		**ret_ids;
#else
_DtHelpCeGetSdlVolIds(
    VolumeHandle	  volume,
    int			  fd,
    CESegment		**ret_ids)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    CESDLVolume		*sdlVol;
    _CEFormatStruct	 frmtStruct;
    SDLMask		 skipMask   [SDL_MASK_LEN] = CEClearAllBits;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);

    *ret_ids = NULL;
    sdlVol   = _DtHelpCeGetSdlVolumePtr(volume);
    if (sdlVol == NULL)
	return -1;

    if (sdlVol->loids == NULL)
      {
	result = -1;
        if (SetUp(NULL, NULL, &frmtStruct, &DefAttrStruct, NULL, -1, False, False) != -1
	    && _DtHelpCeFileOpenAndSeek(_DtHelpCeGetVolumeName(volume), 0, fd,
					&(frmtStruct.my_file), NULL) != -1)
          {
	    if (ProcessContent(NULL, &frmtStruct, SDLDocumentContent,
				SdlElementNone, startExcept, skipMask) != -1)
	      {
	        if (ParseSDL(NULL, &frmtStruct, SdlElementVStruct,
				SdlElementLoids, startExcept, skipMask) != -1)
	          {
		    sdlVol->loids = frmtStruct.seg_list;
		    result = 0;
	          }
	      }
	    _DtHelpCeBufFileClose (frmtStruct.my_file, (fd == -1 ? True : False));
          }
      }

    if (sdlVol->loids != NULL)
        *ret_ids = _CEElementOfSegment(sdlVol->loids)->seg_list;

    return result;

} /* End _DtHelpCeGetSdlVolIds */

/******************************************************************************
 * Function:    int _DtHelpCeParseSdlTopic (VolumeHandle volume,
 *                              int offset, char *id_string,
 *                              TopicHandle *ret_handle)
 *
 * Parameters:
 *              volume          Specifies the Help Volume the information
 *                              is associated with.
 *              offset          Specifies the offset into 'filename' to
 *                              the Help Topic desired.
 *              id_string       Specifies the location id to look for or NULL.
 *              ret_handle      Returns a handle to the topic information
 *                              including the number of paragraphs and the
 *                              id match segment.
 *
 * Returns:     0 if successful, -1 if errors
 *
 * errno Values:
 *
 * Purpose:     _DtHelpCeParseSdlTopic formats Help Files with formatting
 *              information into a CEVirtualPage
 *
 ******************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpCeParseSdlTopic(canvas_handle, volume, fd, offset, id_string,
			rich_text, ret_handle)
    CanvasHandle	 canvas_handle;
    VolumeHandle	 volume;
    int			 fd;
    int			 offset;
    char		*id_string;
    CEBoolean		 rich_text;
    TopicHandle		*ret_handle;
#else
_DtHelpCeParseSdlTopic(
    CanvasHandle	 canvas_handle,
    VolumeHandle	 volume,
    int			 fd,
    int			 offset,
    char		*id_string,
    CEBoolean		 rich_text,
    TopicHandle		*ret_handle)
#endif /* _NO_PROTO */
{
    int			 result = 0;
    SDLMask		 startExcept[SDL_MASK_LEN] =
					CEInitializeMask(SdlElementNone);
    SDLMask		 processMask[SDL_MASK_LEN]  = CESetAllBits;
    _CEFormatStruct	 frmtStruct;
    CECanvasStruct	*canvas = (CECanvasStruct *) canvas_handle;
    CETopicStruct	*topicHandle;
    CESDLVolume		*sdlVol;
    CESegment		*pSeg;

    *ret_handle = NULL;


    if (SetUp(volume,&sdlVol,&frmtStruct,&DefAttrStruct,NULL,fd,True,rich_text)!=0)
	return -1;

    result = _DtHelpCeFileOpenAndSeek(_DtHelpCeGetVolumeName(volume),
					offset, fd,
					&(frmtStruct.my_file), NULL);
    if (result != -1)
      {
	result = ParseSDL (canvas, &frmtStruct, SdlElementVirpage,
				SdlElementNone, startExcept, processMask);
	_DtHelpCeBufFileClose (frmtStruct.my_file, (fd == -1 ? True : False));
      }

    if (result != -1)
      {
	topicHandle = (CETopicStruct *) malloc (sizeof(CETopicStruct));
	if (topicHandle != NULL)
	  {
	    topicHandle->name      = _DtHelpCeGetVolumeName(volume);
	    topicHandle->para_num  = 0;
	    topicHandle->para_list = NULL;
	    topicHandle->id_seg    = NULL;
	    topicHandle->id_str    = NULL;
	    if (id_string != NULL)
		topicHandle->id_str = strdup(id_string);

	    topicHandle->link_data = frmtStruct.my_links;

	    pSeg = frmtStruct.async_blks;
	    if (pSeg != NULL)
	      {
		while (pSeg->next_seg != NULL)
		    pSeg = pSeg->next_seg;

		pSeg->next_seg	  = frmtStruct.seg_list;
		frmtStruct.seg_list = frmtStruct.async_blks;
	      }

	    topicHandle->element_list = (void *)frmtStruct.seg_list;
	    topicHandle->toss         = _DtHelpCeGetSdlVolToss(volume, fd);

	    *ret_handle = (void *) topicHandle;
	  }
	else
	    result = -1;
      }

    return result;

} /* End _DtHelpCeParseSdlTopic */
