/* $XConsortium: XInterface.c /main/cde1_maint/2 1995/10/02 15:09:58 lehors $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	   XInterface.c
 **
 **   Project:     Cde Help System
 **
 **   Description: This is the UI dependant layer to the Help System.
 **		   It recieves requests from the Core Engine for
 **		   information or to do rendering.  It does the
 **		   appropriate work based on a Motif and X Lib
 **		   interface.
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
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <X11/Xlib.h>

extern	int	errno;

/*
 * private includes
 */
#include "AsciiSpcI.h"
#include "Canvas.h"
#include "DisplayAreaP.h"
#include "HelpErrorP.h"
#include "XInterfaceI.h"
#include "FontI.h"
#include "GraphicsI.h"
#include "StringFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*****************************************************************************
 *		Private Defines
 *****************************************************************************/
#define	GROW_SIZE	3

/*****************************************************************************
 *		Private Variables
 *****************************************************************************/
static	const char	IsoString[]    = "ISO-8859-1";
static	const char	SymbolString[] = "DT-SYMBOL-1";
static	const char	DefaultStr[]   = "default_spc";

typedef	struct {
	const char	*symbol;
	const char	 spc_string[4];
	const char	*spc_value;
} SpecialSymbolTable;


static	SpecialSymbolTable SpcTable[] =
{ 
 {"[alpha ]", {'\141','\0'}, SymbolString}, /*ISOgrk3, Greek, small alpha    */ 
 {"[aleph ]", {'\300','\0'}, SymbolString}, /*ISOtech, aleph, Hebrews        */ 
 {"[and   ]", {'\331','\0'}, SymbolString}, /*ISOtech, wedge, logical and    */ 
 {"[ap    ]", {'\273','\0'}, SymbolString}, /*ISOtech, approximately equal   */ 
 {"[beta  ]", {'\142','\0'}, SymbolString}, /*ISOgrk3, Greek, small beta     */ 
 {"[bull  ]", {'\267','\0'}, SymbolString}, /*ISOpub , round bullet,filled   */ 
 {"[cap   ]", {'\307','\0'}, SymbolString}, /*ISOtech, intersection          */ 
 {"[cent  ]", {'\242','\0'}, IsoString   }, /*ISOnum , cent sign             */ 
 {"[chi   ]", {'\143','\0'}, SymbolString}, /*ISOgrk3, Greek, small chi      */ 
 {"[clubs ]", {'\247','\0'}, SymbolString}, /*ISOpub , clubs suit symbol     */ 
 {"[copy  ]", {'\323','\0'}, SymbolString}, /*ISOnum , copyright symbol      */ 
 {"[cong  ]", {'\100','\0'}, SymbolString}, /*ISOtech, congruent with        */ 
 {"[cup   ]", {'\310','\0'}, SymbolString}, /*ISOtech, union or logical sum  */ 
 {"[curren]", {'\244','\0'}, IsoString   }, /*ISOnum , general currency sign */ 
 {"[darr  ]", {'\257','\0'}, SymbolString}, /*ISOnum , downward arrow        */ 
 {"[dArr  ]", {'\337','\0'}, SymbolString}, /*ISOamsa, down double arrow     */ 
 {"[deg   ]", {'\260','\0'}, SymbolString}, /*ISOnum , degree sign           */ 
 {"[delta ]", {'\144','\0'}, SymbolString}, /*ISOgrk3, Greek, small delta    */ 
 {"[diams ]", {'\250','\0'}, SymbolString}, /*ISOpub , diamond suit symbol   */ 
 {"[divide]", {'\270','\0'}, SymbolString}, /*ISOnum , divide sign           */ 
 {"[epsiv ]", {'\145','\0'}, SymbolString}, /*ISOgrk3, Greek,sm epsilon, var */ 
 {"[equiv ]", {'\272','\0'}, SymbolString}, /*ISOtech, identical with        */ 
 {"[eta   ]", {'\150','\0'}, SymbolString}, /*ISOgrk3, Greek, small eta      */ 
 {"[exist ]", {'\044','\0'}, SymbolString}, /*ISOtech, at least one exists   */ 
 {"[forall]", {'\042','\0'}, SymbolString}, /*ISOtech, for all               */ 
 {"[frac12]", {'\275','\0'}, IsoString   }, /*ISOnum , fraction one-half     */ 
 {"[frac14]", {'\274','\0'}, IsoString   }, /*ISOnum , fraction one-quarter  */ 
 {"[frac34]", {'\276','\0'}, IsoString   }, /*ISOnum , fraction three-quarter*/ 
 {"[gamma ]", {'\147','\0'}, SymbolString}, /*ISOgrk3, Greek, small gamma    */ 
 {"[ge    ]", {'\263','\0'}, SymbolString}, /*ISOtech, greater-than-or-equal */ 
 {"[half  ]", {'\275','\0'}, IsoString   }, /*ISOnum , fraction one-half     */ 
 {"[harr  ]", {'\253','\0'}, SymbolString}, /*ISOamsa, left & right arrow    */ 
 {"[hArr  ]", {'\333','\0'}, SymbolString}, /*ISOamsa, l & r double arrow    */ 
 {"[hearts]", {'\251','\0'}, SymbolString}, /*ISOpub , hearts suit symbol    */ 
 {"[hellip]", {'\274','\0'}, SymbolString}, /*ISOpub , ellipsis(horizontal)  */ 
 {"[iexcl ]", {'\241','\0'}, IsoString   }, /*ISOnum , inverted ! mark       */ 
 {"[image ]", {'\301','\0'}, SymbolString}, /*ISOamso,imaginary number symbol*/ 
 {"[infin ]", {'\245','\0'}, SymbolString}, /*ISOtech, infinity              */ 
 {"[iota  ]", {'\151','\0'}, SymbolString}, /*ISOgrk3, Greek, small iota     */ 
 {"[iquest]", {'\277','\0'}, IsoString   }, /*ISOnum , inverted ? mark       */ 
 {"[isin  ]", {'\316','\0'}, SymbolString}, /*ISOtech, set membership        */ 
 {"[kappa ]", {'\153','\0'}, SymbolString}, /*ISOgrk3, Greek, small kappa    */ 
 {"[lambda]", {'\154','\0'}, SymbolString}, /*ISOgrk3, Greek, small lambda   */ 
 {"[larr  ]", {'\254','\0'}, SymbolString}, /*ISOnum , leftward arrow        */ 
 {"[le    ]", {'\243','\0'}, SymbolString}, /*ISOtech, less-than-or-equal    */ 
 {"[lsquo ]", {  '`' ,'\0'}, IsoString   }, /*ISOnum , left single quote     */ 
 {"[mdash ]", {'\276','\0'}, SymbolString}, /*ISOpub , em dash(long dash)    */ 
 {"[mu    ]", {'\155','\0'}, SymbolString}, /*ISOgrk3, Greek, small mu       */ 
 {"[ndash ]", {  '-' ,'\0'}, IsoString   }, /*ISOpub , en dash(short dash)   */ 
 {"[ne    ]", {'\271','\0'}, SymbolString}, /*ISOtech, not equal             */ 
 {"[not   ]", {'\330','\0'}, SymbolString}, /*ISOnum , not                   */ 
 {"[notin ]", {'\317','\0'}, SymbolString}, /*ISOtech, negated set membership*/ 
 {"[nu    ]", {'\156','\0'}, SymbolString}, /*ISOgrk3, Greek, small nu       */ 
 {"[omega ]", {'\167','\0'}, SymbolString}, /*ISOgrk3, Greek, small omega    */ 
 {"[oplus ]", {'\305','\0'}, SymbolString}, /*ISOamsb, plus sign in circle   */ 
 {"[or    ]", {'\332','\0'}, SymbolString}, /*ISOtech, vee, logical or       */ 
 {"[osol  ]", {'\306','\0'}, SymbolString}, /*ISOamsb, slash in circle       */ 
 {"[otimes]", {'\304','\0'}, SymbolString}, /*ISOamsb,multiply sign in circle*/ 
 {"[over  ]", {'\140','\0'}, SymbolString}, /*made up, over symbol           */ 
 {"[para  ]", {'\266','\0'}, IsoString   }, /*ISOnum , paragraph sign        */ 
 {"[part  ]", {'\266','\0'}, SymbolString}, /*ISOtech, partial differential  */ 
 {"[perp  ]", {'\136','\0'}, SymbolString}, /*ISOtech, perpendicular         */ 
 {"[phis  ]", {'\146','\0'}, SymbolString}, /*ISOgrk3, Greek, small phi      */ 
 {"[pi    ]", {'\160','\0'}, SymbolString}, /*ISOgrk3, Greek, small pi       */ 
 {"[piv   ]", {'\166','\0'}, SymbolString}, /*ISOgrk3, Greek, small pi, var  */ 
 {"[plusmn]", {'\261','\0'}, SymbolString}, /*ISOnum , plus or minus sign    */ 
 {"[pound ]", {'\243','\0'}, IsoString   }, /*ISOnum , pound sign            */ 
 {"[prime ]", {'\242','\0'}, SymbolString}, /*ISOtech, prime or minute       */ 
 {"[prop  ]", {'\265','\0'}, SymbolString}, /*ISOtech, proportional to       */ 
 {"[psi   ]", {'\171','\0'}, SymbolString}, /*ISOgrk3, Greek, small psi      */ 
 {"[rarr  ]", {'\256','\0'}, SymbolString}, /*ISOnum , rightward arrow       */ 
 {"[rArr  ]", {'\336','\0'}, SymbolString}, /*ISOtech, right double arrow    */ 
 {"[real  ]", {'\302','\0'}, SymbolString}, /*ISOamso, real number symbol    */ 
 {"[reg   ]", {'\322','\0'}, SymbolString}, /*ISOnum,circledR,registered sign*/ 
 {"[rho   ]", {'\162','\0'}, SymbolString}, /*ISOgrk3, Greek, small rho      */ 
 {"[sect  ]", {'\247','\0'}, IsoString   }, /*ISOnum , section sign          */ 
 {"[sigma ]", {'\163','\0'}, SymbolString}, /*ISOgrk3, Greek, small sigma    */ 
 {"[spades]", {'\252','\0'}, SymbolString}, /*ISOpub , spades suit symbol    */ 
 {"[sub   ]", {'\314','\0'}, SymbolString}, /*ISOtech, subset/is implied by  */ 
 {"[sube  ]", {'\315','\0'}, SymbolString}, /*ISOtech, subset, equals        */ 
 {"[sup   ]", {'\311','\0'}, SymbolString}, /*ISOtech, superset or implies   */ 
 {"[supe  ]", {'\312','\0'}, SymbolString}, /*ISOtech, superset, equals      */ 
 {"[tau   ]", {'\164','\0'}, SymbolString}, /*ISOgrk3, Greek, small tau      */ 
 {"[there4]", {'\134','\0'}, SymbolString}, /*ISOtech, therefore             */ 
 {"[thetas]", {'\161','\0'}, SymbolString}, /*ISOgrk3, Greek, small theta    */ 
 {"[thetav]", {'\112','\0'}, SymbolString}, /*ISOgrk3, Greek, small theta,var*/ 
 {"[times ]", {'\264','\0'}, SymbolString}, /*ISOnum , multipy sign          */ 
 {"[trade ]", {'\324','\0'}, SymbolString}, /*ISOnum , trade mark sign       */ 
 {"[uarr  ]", {'\255','\0'}, SymbolString}, /*ISOnum , upward arrow          */ 
 {"[uArr  ]", {'\335','\0'}, SymbolString}, /*ISOamsa, up double arrow       */ 
 {"[upsi  ]", {'\165','\0'}, SymbolString}, /*ISOgrk3, Greek, small upsilon  */ 
 {"[vellip]", {':','\n',':','\0'},
			     SymbolString}, /*ISOpub , vertical ellipsis     */ 
 {"[weierp]", {'\303','\0'}, SymbolString}, /*ISOamso, Weierstrass p         */ 
 {"[xi    ]", {'\170','\0'}, SymbolString}, /*ISOgrk3, Greek, small xi       */ 
 {"[yen   ]", {'\245','\0'}, IsoString   }, /*ISOnum , yen sign              */ 
 {"[zeta  ]", {'\172','\0'}, SymbolString}, /*ISOgrk3, Greek, small zeta     */ 
 {"[Delta ]", {'\104','\0'}, SymbolString}, /*ISOgrk3, Greek, large delta    */ 
 {"[Gamma ]", {'\107','\0'}, SymbolString}, /*ISOgrk3, Greek, large gamma    */ 
 {"[Lambda]", {'\114','\0'}, SymbolString}, /*ISOgrk3, Greek, large lambda   */ 
 {"[Omega ]", {'\127','\0'}, SymbolString}, /*ISOgrk3, Greek, large omega    */ 
 {"[Pi    ]", {'\120','\0'}, SymbolString}, /*ISOgrk3, Greek, large pi       */ 
 {"[Prime ]", {'\262','\0'}, SymbolString}, /*ISOtech, double prime/second   */ 
 {"[Phi   ]", {'\106','\0'}, SymbolString}, /*ISOgrk3, Greek, large phi      */ 
 {"[Psi   ]", {'\131','\0'}, SymbolString}, /*ISOgrk3, Greek, large psi      */ 
 {"[Sigma ]", {'\123','\0'}, SymbolString}, /*ISOgrk3, Greek, large sigma    */ 
 {"[Theta ]", {'\121','\0'}, SymbolString}, /*ISOgrk3, Greek, large theta    */ 
 {"[Upsi  ]", {'\125','\0'}, SymbolString}, /*ISOgrk3, Greek, large upsilon  */ 
 {"[Xi    ]", {'\130','\0'}, SymbolString}, /*ISOgrk3, Greek, large xi       */ 
 { NULL     , {' '   ,'\0'}, IsoString   }, /* default character to use      */
};


/*****************************************************************************
 *		Semi-Private Variables
 *****************************************************************************/
/*****************************************************************************
 *		Private Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	Unit GetTocIndicator(pDAS);
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static	CanvasFontSpec	DefFontAttrs = {
		10, 10, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		CEFontStyleSerif,
		CEFontSpacingProp,
		CEFontWeightMedium,
		CEFontSlantRoman,
		CEFontSpecialNone };

static void
#ifdef _NO_PROTO
GetTocIndicator (pDAS)
    DtHelpDispAreaStruct	*pDAS;
#else
GetTocIndicator (
    DtHelpDispAreaStruct	*pDAS)
#endif /* _NO_PROTO */
{
    Unit	ascent;

    pDAS->toc_width  = 0;
    pDAS->toc_height = 0;
    _DtHelpDAResolveSpc (pDAS, "C", IsoString, DefFontAttrs, "[rArr  ]",
			&(pDAS->toc_indicator), &(pDAS->toc_width),
			&(pDAS->toc_height), &ascent);
}

static void
#ifdef _NO_PROTO
ClearTocMarker (pDAS, flag)
    DtHelpDispAreaStruct	*pDAS;
    Flags			 flag;
#else
ClearTocMarker (
    DtHelpDispAreaStruct	*pDAS,
    Flags			 flag)
#endif /* _NO_PROTO */
{
    int clearHeight;
    int clearWidth;
    int clearY      = pDAS->toc_y + pDAS->decorThickness
					- pDAS->firstVisible;
    int clearX      = pDAS->toc_x + pDAS->decorThickness - pDAS->virtualX;

    clearHeight = pDAS->dispHeight - pDAS->decorThickness - clearY;
    clearWidth  = pDAS->dispWidth  - pDAS->decorThickness - clearX;

    if (clearHeight > pDAS->toc_height)
	clearHeight = pDAS->toc_height;

    if (clearWidth > pDAS->toc_width)
	clearWidth = pDAS->toc_width;

    if (clearY < pDAS->decorThickness)
      {
	clearHeight = clearHeight - pDAS->decorThickness + clearY;
	clearY      = pDAS->decorThickness;
      }
    if (clearX < pDAS->decorThickness)
      {
	clearWidth = clearWidth - pDAS->decorThickness + clearX;
	clearX     = pDAS->decorThickness;
      }

    if (clearWidth > 0 && clearHeight > 0)
      {
	if (CEIsSelected(flag))
	    XFillRectangle (XtDisplay(pDAS->dispWid), XtWindow(pDAS->dispWid),
				pDAS->invertGC,
				clearX, clearY, clearWidth, clearHeight);
	else
            XClearArea(XtDisplay(pDAS->dispWid), XtWindow(pDAS->dispWid),
			clearX, clearY, clearWidth, clearHeight, False);
      }
}

/*****************************************************************************
 * Function:	void DrawTocIndicator ();
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
DrawTocIndicator (pDAS, dst_x, base_y, top_y, height, flag)
    DtHelpDispAreaStruct *pDAS;
    Unit	dst_x;
    Unit	base_y;
    Unit	top_y;
    Unit	height;
    Flags	flag;
#else
DrawTocIndicator (
    DtHelpDispAreaStruct *pDAS,
    Unit	dst_x,
    Unit	base_y,
    Unit	top_y,
    Unit	height,
    Flags	flag)
#endif /* _NO_PROTO */
{

    if (CEIsTocTopicBegin(flag))
      {
	/*
	 * create absolute x,y
	 */
	pDAS->toc_y  = top_y - pDAS->decorThickness + pDAS->firstVisible;
	pDAS->toc_x  = 0;

	if (CEIsTocTopicOn(flag))
	  {
	    flag &= ~(CETocTopicFlag);
	    flag &= ~(CETraversalFlag);
	    _DtHelpDADrawSpc (((ClientData) pDAS), pDAS->toc_indicator,
			0, base_y - pDAS->decorThickness + pDAS->firstVisible,
			0, top_y  - pDAS->decorThickness + pDAS->firstVisible,
			height, 0, flag, flag);
	  }
	else
	    ClearTocMarker(pDAS, flag);
      }
}

/*****************************************************************************
 * Function:	void DrawTraversal ();
 *
 * Parameters:
 *		pDAS	Specifies the display area.
 *		dst_x	Specifies the inside boundary of the segment.
 *			This is where the segment starts, so the
 *			traversal box must end at dst_x - 1.
 *		dst_y	Specifies the upper pixel that the traversal
 *			will cover. 
 *		width	Specifies the inner width of the traversal box.
 *		height  Specifies the outer height of the traversal box
 *			including the leading. But since the leading is
 *			also used for the bottom traversal line, its
 *			okay.
 *		flag	Specifies whether to turn the traversal box on
 *			or off.
 *
 * Returns: Nothing.
 *
 * Purpose: Draw the traversal box.
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
DrawTraversal (pDAS, dst_x, dst_y, width, height, flag)
    DtHelpDispAreaStruct	*pDAS;
    Unit	dst_x;
    Unit	dst_y;
    Unit	width;
    Unit	height;
    Flags	flag;
#else
DrawTraversal (
    DtHelpDispAreaStruct	*pDAS,
    Unit	dst_x,
    Unit	dst_y,
    Unit	width,
    Unit	height,
    Flags	flag)
#endif /* _NO_PROTO */
{
    int     copyY1;
    int     copyY2;

    int     lineX1;
    int     lineY1;
    int     lineX2;
    int     lineY2;

    int     onLeftTop;
    int     onRightBot;
    Display	*dpy = XtDisplay(pDAS->dispWid);
    Window	 win = XtWindow(pDAS->dispWid);

    if (pDAS->neededFlags & _DT_HELP_FOCUS_FLAG)
      {
/*
 * TODO - IBM and SUN may do this different.
 */
	/*
	 * how many pixels will be above or to the left of the
	 * destination position.
	 */
        onLeftTop = pDAS->lineThickness / 2;

	/*
	 * how many pixels will be on and inside the destination.
	 */
	onRightBot = pDAS->lineThickness - onLeftTop;

	/*
	 * calculate the y position of the traversal lines
	 * for the top and bottom lines.
	 */
	lineY1 = dst_y + onLeftTop;
	lineY2 = dst_y + height - onRightBot;

	/*
	 * get the extreme y positions.
	 */
	copyY1 = dst_y;
	copyY2 = dst_y + height;

	/*
	 * adjust the source and destination positions.
	 */
	lineX1 = dst_x;
	if (CEIsTraversalBegin (flag))
	    lineX1 = lineX1 - onRightBot;

	lineX2 = dst_x + width;
	if (CEIsTraversalEnd (flag))
	    lineX2 = lineX2 + onLeftTop;

	if (CEHasTraversal(flag))
	    XSetForeground (dpy, pDAS->normalGC, pDAS->traversalColor);
	else
	    XSetForeground (dpy, pDAS->normalGC, pDAS->backgroundColor);

	/*
	 * draw the top and bottom lines
	 */
	if (lineX1 - onLeftTop < ((int) pDAS->dispWidth) &&
						lineX2 + onRightBot - 1 > 0)
	  {
	    /*
	     * draw the top line
	     */
	    if (copyY1 < ((int) pDAS->dispHeight) &&
		      copyY1 + ((int)pDAS->lineThickness) >
						((int)pDAS->decorThickness))
	        XDrawLine(dpy,win,pDAS->normalGC,lineX1,lineY1,lineX2,lineY1);

	    /*
	     * draw the bottom line
	     */
	    if (copyY2 - ((int)pDAS->lineThickness) < ((int)pDAS->dispHeight)
				&& copyY2 > ((int)pDAS->decorThickness))
	        XDrawLine(dpy,win,pDAS->normalGC,lineX1,lineY2,lineX2,lineY2);
	  }

	/*
	 * draw the left end
	 */
	if (CEIsTraversalBegin(flag)
		&& lineX1 - onLeftTop < ((int)pDAS->dispWidth)
		&& lineX1 + onRightBot - 1 > 0
		&& copyY1 < ((int) pDAS->dispHeight)
		&& copyY2 > ((int)pDAS->decorThickness))
	    XDrawLine(dpy, win, pDAS->normalGC, lineX1, copyY1, lineX1, copyY2);

	/*
	 * draw the right end
	 */
	if (CEIsTraversalEnd(flag)
		&& lineX2 - onLeftTop < ((int) pDAS->dispWidth)
		&& lineX2 + onRightBot - 1 > 0
		&& copyY1 < ((int) pDAS->dispHeight)
		&& copyY2 > ((int) pDAS->decorThickness))
	    XDrawLine(dpy, win, pDAS->normalGC, lineX2, copyY1, lineX2, copyY2);

	XSetForeground (dpy, pDAS->normalGC, pDAS->foregroundColor);
	XSetFillStyle(dpy, pDAS->normalGC, FillSolid);
      }
}

/*****************************************************************************
 * Function:	void MyDrawString (Display *dpy, Drawable d, GC gc,
 *				int font_index,
 *				int x, int y, char *string, int length,
 *				int char_size, Boolean image_flag )
 *
 * Parameters:
 *		dpy		Specifies the connection to the X Server.
 *		d		Specifies the drawable.
 *		gc		Specifies the graphic's context.
 *		font_index	Specifies the font structure or set to use.
 *				If the value is positive, it indicates a
 *				font structure. If negative, it indicates
 *				a font set.
 *		x		Specifies the x coordinate.
 *		y		Specifies the y coordinate.
 *		string		Specifies the string to render.
 *		length		Specifies the number of bytes in 'string'
 *				to render.
 *		char_size	Specifies the number of bytes required to
 *				render one character.
 *		image_flag	Specifies whether to use the XDrawImageString
 *				function or not.
 *
 * Return Value:	Nothing
 *
 * Purpose:	Renders an text string to the specified drawable.
 *
 *****************************************************************************/
static void
#ifdef _NO_PROTO
MyDrawString (dpy, d, gc, font_index, x, y, string, length, char_size,
		font_info, image_flag)
    Display	*dpy;
    Drawable	 d;
    GC		 gc;
    int		 font_index;
    int		 x;
    int		 y;
    const char	*string;
    int		 length;
    int		 char_size;
    DtHelpDAFontInfo	font_info;
    Boolean	 image_flag;
#else
MyDrawString (
    Display	*dpy,
    Drawable	 d,
    GC		 gc,
    int		 font_index,
    int		 x,
    int		 y,
    const char	*string,
    int		 length,
    int		 char_size,
    DtHelpDAFontInfo	font_info,
    Boolean	 image_flag )
#endif /* _NO_PROTO */
{

/*
 * If the 'font_index' is a negative number, it indicates a font set was
 * opened for this string. Use the X11R5 I18N routines to render the
 * glyphs. Otherwise use the standard X11 drawing routines.
 */

    if (font_index < 0)
      {
	XFontSet  mySet;

	mySet = __DtHelpFontSetGet(font_info, font_index);

#ifdef	notdone
	if (image_flag)
	    XmbDrawImageString (dpy, d, mySet, gc, x, y, string, length);
	else
#endif
	    XmbDrawString (dpy, d, mySet, gc, x, y, string, length);
      }
    else
      {
	XFontStruct  *myFont;

	myFont = __DtHelpFontStructGet(font_info, font_index);
	XSetFont (dpy, gc, myFont->fid);

#ifdef	notdone
        if (image_flag)
	    XDrawImageString (dpy, d, gc, x, y, string, length);
        else
#endif
	    XDrawString (dpy, d, gc, x, y, string, length);
      }
}

/*****************************************************************************
 * Function:	void ResolveFont ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
ResolveFont (client_data, lang, charset, font_attr, ret_idx)
    ClientData		 client_data;
    char		*lang;
    const char		*charset;
    CanvasFontSpec	 font_attr;
    int			*ret_idx;
#else
ResolveFont (
    ClientData		 client_data,
    char		*lang,
    const char		*charset,
    CanvasFontSpec	 font_attr,
    int			*ret_idx )
#endif /* _NO_PROTO */
{
    int    result = -1;
    char  *xlfdSpec = font_attr.xlfd;
    char   buffer[10];
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
    XrmName	xrmList[_DtHelpFontQuarkNumber];

    _DtHelpCopyDefaultList(xrmList);

    if (font_attr.spacing != CEFontSpacingProp)
        xrmList[_DT_HELP_FONT_SPACING] = XrmStringToQuark("m");

    sprintf(buffer, "%d", font_attr.pointsz);
    xrmList[_DT_HELP_FONT_SIZE]    = XrmStringToQuark(buffer);

    if (font_attr.slant != CEFontSlantRoman)
      {
        xrmList[_DT_HELP_FONT_ANGLE]  = XrmStringToQuark("italic");
	if (font_attr.xlfdi != NULL)
	    xlfdSpec = font_attr.xlfdi;
      }

    if (font_attr.weight == CEFontWeightBold)
      {
        xrmList[_DT_HELP_FONT_WEIGHT] = XrmStringToQuark("bold");
	if (xrmList[_DT_HELP_FONT_ANGLE] == XrmStringToQuark("italic"))
	  {
	    if (font_attr.xlfdib != NULL)
	        xlfdSpec = font_attr.xlfdib;
	  }
	else if (font_attr.xlfdb != NULL)
	    xlfdSpec = font_attr.xlfdb;
      }

    if (font_attr.style == CEFontStyleSerif)
        xrmList[_DT_HELP_FONT_TYPE] = XrmStringToQuark("serif");
    else if (font_attr.style == CEFontStyleSymbol)
        xrmList[_DT_HELP_FONT_TYPE] = XrmStringToQuark("symbol");

    xrmList[_DT_HELP_FONT_LANG_TER] = XrmStringToQuark ("C");
    if (lang != NULL)
	xrmList[_DT_HELP_FONT_LANG_TER] = XrmStringToQuark(lang);

    if (charset != NULL)
        xrmList[_DT_HELP_FONT_CHAR_SET] = XrmStringToQuark(charset);

    if (NULL == xlfdSpec ||
		_DtHelpGetExactFontIndex(pDAS,lang,charset,xlfdSpec,ret_idx)!=0)
        result = __DtHelpFontIndexGet (pDAS, xrmList, ret_idx);

    return result;
}

/*****************************************************************************
 *		Semi-Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	char * _DtHelpDAGetSpcString ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
const char *
#ifdef _NO_PROTO
_DtHelpDAGetSpcString (idx)
    int		idx;
#else
_DtHelpDAGetSpcString (
    int		idx)
#endif /* _NO_PROTO */
{
    return (SpcTable[idx].spc_string);
}

/*****************************************************************************
 *		Public Functions
 *****************************************************************************/
/*****************************************************************************
 * Function:	VStatus _DtHelpDAGetCvsMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDAGetCvsMetrics (client_data, ret_canvas, ret_font, ret_link, ret_traversal)
    ClientData			 client_data;
    CanvasMetrics		*ret_canvas;
    CanvasFontMetrics		*ret_font;
    CanvasLinkMetrics		*ret_link;
    CanvasTraversalMetrics	*ret_traversal;
#else
_DtHelpDAGetCvsMetrics (
    ClientData			 client_data,
    CanvasMetrics		*ret_canvas,
    CanvasFontMetrics		*ret_font,
    CanvasLinkMetrics		*ret_link,
    CanvasTraversalMetrics	*ret_traversal)
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (ret_canvas)
      {
        ret_canvas->width          = pDAS->dispUseWidth;
        ret_canvas->height         = pDAS->dispUseHeight;
	ret_canvas->leading        = pDAS->leading;
	ret_canvas->top_margin     = pDAS->marginHeight;
	ret_canvas->side_margin    = pDAS->marginWidth;
	ret_canvas->line_thickness = pDAS->lineThickness;
      }
    if (ret_font)
      {
	ret_font->ascent        = pDAS->fontAscent;
	ret_font->descent       = pDAS->lineHeight - pDAS->fontAscent
							- pDAS->leading;
	ret_font->average_width = (int)(pDAS->charWidth / 10 +
					((pDAS->charWidth % 10) ? 1 : 0));
      }
    if (ret_link)
      {
	ret_link->space_before = 0;
	ret_link->space_after  = 0;
	ret_link->space_above  = 0;
	ret_link->space_below  = 0;
      }
    if (ret_traversal)
      {
	ret_traversal->space_before = pDAS->lineThickness;
	if (pDAS->traversal & _DT_HELP_SHADOW_TRAVERSAL)
	  {
	    if (pDAS->traversal & _DT_HELP_NOT_INITIALIZED)
	      {
	        GetTocIndicator(pDAS);
	        pDAS->traversal &= (~(_DT_HELP_NOT_INITIALIZED));
	      }
	    ret_traversal->space_before += pDAS->toc_width;
	  }

	ret_traversal->space_after  = pDAS->lineThickness;
	ret_traversal->space_above  = pDAS->lineThickness;
	ret_traversal->space_below  = 0;
      }
}

/*****************************************************************************
 * Function:	void _DtHelpDAResolveFont ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDAResolveFont (client_data, lang, charset, font_attr, mod_string, ret_font)
    ClientData		 client_data;
    char		*lang;
    const char		*charset;
    CanvasFontSpec	 font_attr;
    char		**mod_string;
    FontPtr		*ret_font;
#else
_DtHelpDAResolveFont (
    ClientData		 client_data,
    char		*lang,
    const char		*charset,
    CanvasFontSpec	 font_attr,
    char		**mod_string,
    FontPtr		*ret_font )
#endif /* _NO_PROTO */
{
    /*
     * note - if the mod_string comes in with a NULL pointer,
     * it came from our own call rather than the Core Engine.
     */
    int idx;

    (void) ResolveFont(client_data, lang, charset, font_attr, &idx);
    *ret_font = (FontPtr) idx;
}

/*****************************************************************************
 * Function:	VStatus _DtHelpDAResolveSpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpDAResolveSpc (client_data, lang, charset, font_attr, spc_symbol,
		   ret_handle, ret_width, ret_height, ret_ascent)
    ClientData		 client_data;
    char		*lang;
    const char		*charset;
    CanvasFontSpec	 font_attr;
    const char		*spc_symbol;
    SpecialPtr		*ret_handle;
    Unit		*ret_width;
    Unit		*ret_height;
    Unit		*ret_ascent;
#else
_DtHelpDAResolveSpc (
    ClientData		 client_data,
    char		*lang,
    const char		*charset,
    CanvasFontSpec	 font_attr,
    const char		*spc_symbol,
    SpecialPtr		*ret_handle,
    Unit		*ret_width,
    Unit		*ret_height,
    Unit		*ret_ascent)
#endif /* _NO_PROTO */
{
    register int          i = 0;
    int			  result = -1;
    int			  fontIdx;
    int			  spcTbIdx = 0;
    Unit		  ascent;
    Unit		  descent;
    const char		 *newSet;
    const char		 *spcStr;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    while (SpcTable[spcTbIdx].symbol != NULL &&
			strcmp(SpcTable[spcTbIdx].symbol, spc_symbol) != 0)
	spcTbIdx++;

    newSet = SpcTable[spcTbIdx].spc_value;
    spcStr = SpcTable[spcTbIdx].spc_string;

    result = ResolveFont(client_data, lang, newSet, font_attr, &fontIdx);

    if (result == 0)
      {
	int spcLstIdx = 0;
	Unit maxWidth;
	while (spcLstIdx < pDAS->cur_spc &&
				pDAS->spc_chars[spcLstIdx].spc_idx != -1)
	    spcLstIdx++;

	if (spcLstIdx == pDAS->cur_spc)
	  {
	    if (pDAS->cur_spc >= pDAS->max_spc)
	      {
		pDAS->max_spc += GROW_SIZE;
		if (pDAS->cur_spc > 0)
		    pDAS->spc_chars = (DtHelpSpecialChars *) realloc(
				(void *)pDAS->spc_chars,
				(sizeof(DtHelpSpecialChars)*pDAS->max_spc));
		else
		    pDAS->spc_chars = (DtHelpSpecialChars *) malloc(
				(sizeof(DtHelpSpecialChars)*pDAS->max_spc));

		if (pDAS->spc_chars == NULL)
		    return -1;

		for (i = pDAS->cur_spc; i < pDAS->max_spc; i++)
		    pDAS->spc_chars[i].spc_idx = -1;
	      }
	    pDAS->cur_spc++;
	  }

	pDAS->spc_chars[spcLstIdx].font_ptr = (FontPtr)fontIdx;
	pDAS->spc_chars[spcLstIdx].spc_idx  = spcTbIdx;
	*ret_handle = ((SpecialPtr)(spcLstIdx));

	*ret_height = 0;
	*ret_ascent = 0;
	*ret_width  = 0;
	do
	  {
	    i = 0;
	    while(spcStr[i] != '\0' && spcStr[i] != '\n')
		i++;

	    _DtHelpDAGetFontMetrics(client_data, (FontPtr)fontIdx,
					&ascent, &descent, NULL, NULL, NULL);
	    *ret_height = *ret_height + ascent + descent;
	    if (*ret_ascent == 0)
		*ret_ascent = ascent;

	    maxWidth  = _DtHelpDAGetStrWidth (client_data, spcStr, i,
							1, (FontPtr)fontIdx);
	    if (*ret_width < maxWidth)
		*ret_width = maxWidth;

	    if (spcStr[i] == '\n')
		i++;

	    spcStr += i;
	  } while (*spcStr != '\0');
	result = 0;
      }
    else if (spc_symbol != DefaultStr)
	result = _DtHelpDAResolveSpc (client_data, lang, charset,
					font_attr, DefaultStr,
					ret_handle, ret_width,
					ret_height, ret_ascent);
    return result;
}

/*****************************************************************************
 * Function:	void _DtHelpDAGetFontMetrics ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDAGetFontMetrics (client_data, font_ptr,
		ret_ascent, ret_descent, char_width, ret_super, ret_sub)
    ClientData	 client_data;
    FontPtr	 font_ptr;
    Unit	*ret_ascent;
    Unit	*ret_descent;
    Unit	*char_width;
    Unit	*ret_super;
    Unit	*ret_sub;
#else
_DtHelpDAGetFontMetrics (
    ClientData	 client_data,
    FontPtr	 font_ptr,
    Unit	*ret_ascent,
    Unit	*ret_descent,
    Unit	*char_width,
    Unit	*ret_super,
    Unit	*ret_sub)
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    __DtHelpFontMetrics (pDAS->font_info, (int) font_ptr,
		ret_ascent, ret_descent, char_width, ret_super, ret_sub);
}

/*****************************************************************************
 * Function:	Unit _DtHelpDAGetStrWidth ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
Unit
#ifdef _NO_PROTO
_DtHelpDAGetStrWidth (client_data, string, byte_len, char_len, font_ptr)
    ClientData	 client_data;
    const char	*string;
    int		 byte_len;
    int		 char_len;
    FontPtr	 font_ptr;
#else
_DtHelpDAGetStrWidth (
    ClientData	 client_data,
    const char	*string,
    int		 byte_len,
    int		 char_len,
    FontPtr	 font_ptr )
#endif /* _NO_PROTO */
{
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
/*
 * If the 'font_index' is a negative number, it indicates a font set was
 * opened for this string. Use the X11R5 I18N routine to figure out
 * its length. Otherwise use the standard X11 text width call.
 */

    int   length;
    int   font_index = (int) font_ptr;

    if (font_index < 0)
	length = XmbTextEscapement(
			__DtHelpFontSetGet(pDAS->font_info, font_index),
							string, byte_len);
    else
        length = XTextWidth(__DtHelpFontStructGet(pDAS->font_info, font_index),
							string, byte_len);

   return ((Unit) length);
}

/*****************************************************************************
 * Function:	void _DtHelpDADrawString ();
 *
 * Parameters:
 *		client_data	Specifies the client specific data
 *				associated with this canvas.
 *		x		Specifies the x unit at which the
 *				string is to be rendered.
 *		y		Specifes the y unit at which the baseline
 *				of the string is to be rendered.
 *		string		Specifies the string to render.
 *		byte_len	Specifies the number of bytes of the
 *				string to render.
 *		char_len	Specifies the number of bytes that
 *				comprise one character.
 *		font_ptr	Specifies the font associated with this
 *				string.
 *		box_x,box_y	Specifies the upper left hand corner of
 *				the bounding box for this string. This
 *				includes any traversal and/or link offsets
 *				relevent for the line this segment occupies.
 *		box_height	Specifies the maximum height of the bounding
 *				box for this string. This includes any
 *				traversal and/or offsets relevent to this
 *				line.
 *		link_type	Specifies the link type if this string is
 *				part of a link.
 *		old_flags	Specifies .............
 *		new_flags	Specifies .............
 *
 * Returns: Nothing
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADrawString (client_data, x, y, string, byte_len, char_len, font_ptr,
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
_DtHelpDADrawString (
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
    Boolean	 image_flag = False;
    int		 strWidth = 0;
    Display	*dpy;
    Window	 win;
    GC		 drawGC;
    GC		 fillGC;
    XGCValues	 gcValues;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    y = y + pDAS->decorThickness - pDAS->firstVisible;
    x = x + pDAS->decorThickness - pDAS->virtualX;

    box_x = box_x + pDAS->decorThickness - pDAS->virtualX;
    box_y = box_y + pDAS->decorThickness - pDAS->firstVisible;

    if (byte_len)
        strWidth = _DtHelpDAGetStrWidth(client_data, string,
						byte_len, char_len, font_ptr);

    dpy = XtDisplay(pDAS->dispWid);
    win = XtWindow(pDAS->dispWid);

    drawGC = pDAS->normalGC;
    fillGC = pDAS->invertGC;
    if (CEIsSelected(new_flags))
      {
	drawGC = pDAS->invertGC;
	fillGC = pDAS->normalGC;
      }

    if (CEHasTraversal(old_flags))
	DrawTraversal (pDAS, x, box_y, strWidth, box_height,
					(old_flags & (~CETraversalFlag)));

    if (CEIsSelected(new_flags) || CEIsSelected(old_flags))
	image_flag = True;

    if (image_flag)
      {
	int width = strWidth + (x - box_x);

	if (CEHasTraversal(new_flags) && CEIsTraversalEnd(new_flags))
	    width += pDAS->lineThickness;

	XFillRectangle (dpy, win, fillGC, (int) box_x, (int) box_y,
					width, box_height);
      }

    if (CEIsTocTopicOn(old_flags) || CEIsTocTopicOn(new_flags))
	DrawTocIndicator (pDAS, x, y, box_y, box_height, new_flags);

    if (strWidth)
      {
        MyDrawString (dpy, win, drawGC,
			(int) font_ptr,
			(int) x, (int) y, string, byte_len,
			char_len,
			pDAS->font_info,
			image_flag);

        if (CEIsHyperLink(new_flags))
          {
	    y   = y + pDAS->underLine;

	    if (new_flags & CELinkPopUp)
	      {
	        gcValues.line_style = LineOnOffDash;
	        XChangeGC (dpy, drawGC, GCLineStyle, &gcValues);
	      }

            XDrawLine (dpy, win, drawGC, (int) x, (int) y,
						(int) (x + strWidth), (int) y);

	    if (new_flags & CELinkPopUp)
	      {
	        gcValues.line_style = LineSolid;
	        XChangeGC (dpy, drawGC, GCLineStyle, &gcValues);
	      }
          }
      }

    if (CEHasTraversal(new_flags))
	DrawTraversal (pDAS, x, box_y, strWidth, box_height, new_flags);

}

/*****************************************************************************
 * Function:	void _DtHelpDADrawSpc ();
 *
 * Parameters:
 *		client_data	Specifies the client specific data
 *				associated with this canvas.
 *		spc_handle	Specifies the handle of the special
 *				character to render.
 *		x		Specifies the x unit at which the
 *				spc is to be rendered.
 *		y		Specifes the y unit at which the baseline
 *				of the spc is to be rendered.
 *		box_x,box_y	Specifies the upper left hand corner of
 *				the bounding box for this string. This
 *				includes any traversal and/or link offsets
 *				relevent for the line this segment occupies.
 *		box_height	Specifies the maximum height of the bounding
 *				box for this string. This includes any
 *				traversal and/or offsets relevent to this
 *				line.
 *		link_type	Specifies the link type if this spc is
 *				part of a link.
 *		old_flags	Specifies .............
 *		new_flags	Specifies .............
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADrawSpc (client_data, spc_handle, x, y, box_x, box_y, box_height,
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
_DtHelpDADrawSpc (
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
    int			  i;
    int			  spcLstIdx = (int) spc_handle;
    Unit		  ascent;
    Unit		  descent;
    const char		 *spcStr;
    DtHelpDispAreaStruct *pDAS      = (DtHelpDispAreaStruct *) client_data;

    if (pDAS->max_spc == 0)
	return;

    spcStr    = SpcTable[pDAS->spc_chars[spcLstIdx].spc_idx].spc_string;
    _DtHelpDAGetFontMetrics(client_data, pDAS->spc_chars[spcLstIdx].font_ptr,
					&ascent, &descent, NULL, NULL, NULL);

    do
      {
	i = 0;
	while (spcStr[i] != '\0' && spcStr[i] != '\n')
	    i++;

        _DtHelpDADrawString (client_data, x, y,
			spcStr, i, 1,
			pDAS->spc_chars[spcLstIdx].font_ptr,
			box_x, box_y, box_height, link_type, old_flags,
			new_flags);

	y  = y + ascent + descent;
	spcStr += i;
	if (*spcStr == '\n')
	    spcStr++;

      } while (*spcStr != '\0');
}

/*****************************************************************************
 * Function:	void _DtHelpDADrawLine ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADrawLine (client_data, x1, y1, x2, y2)
    ClientData	 client_data;
    Unit	 x1;
    Unit	 y1;
    Unit	 x2;
    Unit	 y2;
#else
_DtHelpDADrawLine (
    ClientData	 client_data,
    Unit	 x1,
    Unit	 y1,
    Unit	 x2,
    Unit	 y2)
#endif /* _NO_PROTO */
{
    int		 offset;
    Display	*dpy;
    Window	 win;
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    offset = pDAS->lineThickness / 2;
    dpy    = XtDisplay(pDAS->dispWid);
    win    = XtWindow(pDAS->dispWid);

    /*
     * drawing a horizontal line?
     */
    if (x1 == x2)
      {
	x1 += offset;
	x2  = x1;
      }
    else
      {
	y1 += offset;
	y2  = y1;
      }

    y1 = y1 + pDAS->decorThickness - pDAS->firstVisible;
    x1 = x1 + pDAS->decorThickness - pDAS->virtualX;

    y2 = y2 + pDAS->decorThickness - pDAS->firstVisible;
    x2 = x2 + pDAS->decorThickness - pDAS->virtualX;

    XDrawLine (dpy, win, pDAS->normalGC, x1, y1, x2, y2);
}

/*****************************************************************************
 * Function:	VStatus GraphicLoad ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpDALoadGraphic (client_data, vol_xid, topic_xid, file_xid, format, method,
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
_DtHelpDALoadGraphic (
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
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
    DtHelpGraphicStruct	 *pGS;
    Widget		  wid;
    Window		  win;
    char		 *ptr;
    char		 *extptr;
    char		 *fileName = file_xid;

    pGS = (DtHelpGraphicStruct *) malloc (sizeof(DtHelpGraphicStruct));
    if (pGS == NULL)
      {
        errno = DtErrorMalloc;
        return -1;
      }

    wid = pDAS->dispWid;
    while (!XtIsRealized(wid) && XtParent(wid) != NULL)
	wid = XtParent(wid);

    win = XtWindow(wid);
    if (!XtIsRealized(wid))
	win = XDefaultRootWindow(XtDisplay(pDAS->dispWid));

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

    /*
     * Find out if this is a X Pixmap graphic and set flag if it is.
     * This will be used later when/if colors need to be freed.
     */
    if (fileName != NULL && _DtHelpCeStrrchr(fileName, ".", MB_CUR_MAX, &extptr) != -1)
    	if (strcmp (extptr, ".xpm") == 0 || strcmp (extptr, ".pm") == 0)
		pGS->used = -1;

    pGS->pix = _DtHelpProcessGraphic (XtDisplay(pDAS->dispWid), win,
				XScreenNumberOfScreen(XtScreen(pDAS->dispWid)),
				pDAS->depth,
				pDAS->pixmapGC,
				&(pDAS->def_pix),
				&(pDAS->def_pix_width),
				&(pDAS->def_pix_height),
				&(pDAS->tiff_xwc),
				pDAS->colormap,
				pDAS->visual,
				pDAS->foregroundColor,
				pDAS->backgroundColor, fileName,
				&(pGS->width), &(pGS->height), &(pGS->mask),
				&(pGS->pixels), &(pGS->num_pixels));

    if (fileName != file_xid)
	free (fileName);

    if (pGS->pix == NULL)
      {
	free(pGS);
	return -1;
      }

    *ret_width   = pGS->width;
    *ret_height  = pGS->height;
    *ret_graphic = (GraphicPtr) pGS;

    return 0;
}

/*****************************************************************************
 * Function:	void _DtHelpDADrawGraphic ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADrawGraphic (client_data, graphic_ptr, x, y, old_flags, new_flags)
    ClientData	 client_data;
    GraphicPtr	 graphic_ptr;
    Unit	 x;
    Unit	 y;
    Flags	 old_flags;
    Flags	 new_flags;
#else
_DtHelpDADrawGraphic (
    ClientData	 client_data,
    GraphicPtr	 graphic_ptr,
    Unit	 x,
    Unit	 y,
    Flags	 old_flags,
    Flags	 new_flags )
#endif /* _NO_PROTO */
{
    int    width;
    int    height;
    int    old_x;
    int    old_y;
    int    srcX;
    int    srcY;

    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
    DtHelpGraphicStruct	 *pGS  = (DtHelpGraphicStruct *)     graphic_ptr;
    Display		 *dpy  = XtDisplay(pDAS->dispWid);
    Window		  win  = XtWindow(pDAS->dispWid);
    XGCValues	 gcValues;

    y = y + pDAS->decorThickness - pDAS->firstVisible;
    x = x + pDAS->decorThickness - pDAS->virtualX;


    old_x = x;
    old_y = y;

    /*
     * add the line thickness of the traversal to the height.
     */
    height = pGS->height + 2 * pDAS->lineThickness;
    if (CEHasTraversal(old_flags))
	DrawTraversal (pDAS, old_x, old_y - pDAS->lineThickness,
			pGS->width, height,
			(old_flags & (~CETraversalFlag)));

    srcX   = 0;
    srcY   = 0;
    width  = pGS->width;
    height = 0;

    /*
     * Check and adjust the width of the graphic if it is
     * to be displayed
     */
    if (x < 0)
      {
	srcX  = srcX - x;
	width = width + x;
	x     = 0;
      }

    if (width > 0)
      {
	height = pGS->height;

	if (y < pDAS->decorThickness)
	  {
	    srcY    = pDAS->decorThickness - y;
	    height -= srcY;
	    y       = pDAS->decorThickness;
	  }
	if (y + height > ((int)(pDAS->dispHeight - pDAS->decorThickness)))
	    height = pDAS->dispHeight - pDAS->decorThickness - y;
      }

    /*
     * If we found something to write out,
     * height and width will/should be greater than zero.
     */
    if (height > 0 && width > 0)
	XCopyArea (dpy, pGS->pix, win, pDAS->normalGC,
					srcX, srcY, width, height, x, y);


    if (CEHasTraversal(new_flags))
	DrawTraversal (pDAS, old_x, old_y - pDAS->lineThickness, pGS->width,
			pGS->height + 2 * pDAS->lineThickness, new_flags);
    else if (CEIsHyperLink(new_flags))
      {
	int x,y;

	XSetFillStyle(dpy, pDAS->normalGC, FillTiled);

	if (new_flags & CELinkPopUp)
	  {
	    gcValues.line_style = LineOnOffDash;
	    XChangeGC (dpy, pDAS->normalGC, GCLineStyle, &gcValues);
	  }

	x   = old_x + 2;
	y   = old_y - pDAS->lineThickness + pDAS->lineThickness / 2;

	XDrawLine(dpy, win, pDAS->normalGC, x, y, old_x + pGS->width - 2, y);

	y   = old_y + pGS->height + pDAS->lineThickness / 2;

	XDrawLine(dpy, win, pDAS->normalGC, x, y, old_x + pGS->width - 2, y);

	y   = old_y + 2;
	x   = old_x - pDAS->lineThickness + pDAS->lineThickness / 2 - 1;

	XDrawLine(dpy, win, pDAS->normalGC, x, y, x, old_y + pGS->height - 2);

	x   = old_x + pGS->width + pDAS->lineThickness / 2 + 1;

	XDrawLine(dpy, win, pDAS->normalGC, x, y, x, old_y + pGS->height - 2);

	if (new_flags & CELinkPopUp)
	  {
	    gcValues.line_style = LineSolid;
	    XChangeGC (dpy, pDAS->normalGC, GCLineStyle, &gcValues);
	  }
	XSetFillStyle(dpy, pDAS->normalGC, FillSolid);
      }
}

/*****************************************************************************
 * Function:	void _DtHelpDADestroyGraphic ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADestroyGraphic (client_data, graphic_ptr)
    ClientData	client_data;
    GraphicPtr	graphic_ptr;
#else
_DtHelpDADestroyGraphic (
    ClientData	client_data,
    GraphicPtr	graphic_ptr )
#endif /* _NO_PROTO */
{
    Arg   args[5];
    int	  i;

    Colormap colormap;

    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;
    DtHelpGraphicStruct	 *pGS  = (DtHelpGraphicStruct *)     graphic_ptr;
    Display		 *dpy  = XtDisplay(pDAS->dispWid);

    if (pGS->pix != pDAS->def_pix)
	XFreePixmap(dpy, pGS->pix);

    if (pGS->mask != None)
	XFreePixmap(dpy, pGS->mask);

    if (pGS->num_pixels)
      {
	XtSetArg (args[0], XmNcolormap, &colormap);
	XtGetValues (pDAS->dispWid, args, 1);

	/*
	 * This will only be set to -1 if the original graphic was an X Pixmap,
	 * <filename>.pm or <filename>.xpm.
	 */
	if (pGS->used != -1)
		XFreeColors (dpy, colormap, pGS->pixels, pGS->num_pixels, 0);

	free((void *) pGS->pixels);
      }

    free((char *) pGS);
}

/*****************************************************************************
 * Function:	void _DtHelpDADestroySpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpDADestroySpc (client_data, spc_handle)
    ClientData	client_data;
    SpecialPtr	spc_handle;
#else
_DtHelpDADestroySpc (
    ClientData	client_data,
    SpecialPtr	spc_handle)
#endif /* _NO_PROTO */
{
    int			  spc_index = (int) spc_handle;
    DtHelpDispAreaStruct *pDAS      = (DtHelpDispAreaStruct *) client_data;

    if (pDAS->max_spc == 0)
	return;

    pDAS->spc_chars[spc_index].spc_idx = -1;
}

/*****************************************************************************
 * Function:	SelectionInfo _DtHelpDABuildSelectedString ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpDABuildSelectedString (client_data, prev_info, string, byte_len,
			char_len, font_ptr, space, flags)
    ClientData		 client_data;
    SelectionInfo	*prev_info;
    char		*string;
    int			 byte_len;
    int			 char_len;
    FontPtr		 font_ptr;
    Unit		 space;
    Flags		 flags;
#else
_DtHelpDABuildSelectedString (
    ClientData		 client_data,
    SelectionInfo	*prev_info,
    char		*string,
    int			 byte_len,
    int			 char_len,
    FontPtr		 font_ptr,
    Unit		 space,
    Flags		 flags )
#endif /* _NO_PROTO */
{
    int    i;
    int    j;
    int    totalLen;
    int    spaceNum  = 0;
    int    stringLen = 0;
    long   spaceLong = space * 10;
    char  *myString  = (char *)(*prev_info);
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    if (spaceLong > 0)
	spaceNum = (int) (spaceLong / pDAS->charWidth +
		((spaceLong % pDAS->charWidth) < pDAS->charWidth / 2 ? 0 : 1));

    if (string == NULL)
	byte_len = 0;

    totalLen = byte_len + spaceNum + 1 + (CEIsEndOfLine(flags) ? 1 : 0);

    if (myString == NULL)
	myString = (char *) malloc (sizeof(char) * totalLen);
    else
      {
	stringLen = strlen(myString);
	myString  = (char *) realloc ((void *) myString,
				(sizeof(char) * (stringLen + totalLen)));
      }

    if (myString == NULL)
	return -1;

    for (i = stringLen; spaceNum > 0; spaceNum--)
	myString[i++] = ' ';

    for (j = 0; byte_len > 0 && string[j] != '\0'; byte_len--)
	myString[i++] = string[j++];

    if (flags)
	myString[i++] = '\n';

    myString[i] = '\0';

    *prev_info = (SelectionInfo) myString;

    return 0;
}

/*****************************************************************************
 * Function:	SelectionInfo _DtHelpDABuildSelectedSpc ();
 *
 * Parameters:
 *
 * Returns:
 *
 * Purpose:
 *
 *****************************************************************************/
VStatus
#ifdef _NO_PROTO
_DtHelpDABuildSelectedSpc (client_data, prev_info, spc_handle, space, flags)
    ClientData		 client_data;
    SelectionInfo	*prev_info;
    SpecialPtr		 spc_handle;
    Unit		 space;
    Flags		 flags;
#else
_DtHelpDABuildSelectedSpc (
    ClientData		 client_data,
    SelectionInfo	*prev_info,
    SpecialPtr		 spc_handle,
    Unit		 space,
    Flags		 flags )
#endif /* _NO_PROTO */
{
    int    spcIdx = (int) spc_handle;
    int    i;
    int    totalLen;
    int    spaceNum  = 0;
    int    stringLen = 0;
    long   spaceLong = space * 10;
    const char *spcStr;
    char  *myString  = (char *)(*prev_info);
    DtHelpDispAreaStruct *pDAS = (DtHelpDispAreaStruct *) client_data;

    /*
     * find the ascii version of this spc
     */
    spcIdx = pDAS->spc_chars[spcIdx].spc_idx;
    spcStr = _DtHelpCeResolveSpcToAscii(SpcTable[spcIdx].symbol);

    if (spcStr == NULL)
	return -1;

    /*
     * get the length to realloc
     */
    totalLen = strlen(spcStr);

    if (spaceLong > 0)
	spaceNum = (int)(spaceLong / pDAS->charWidth +
		((spaceLong % pDAS->charWidth) < pDAS->charWidth / 2 ? 0 : 1));

    totalLen = totalLen + spaceNum + 1 + (CEIsEndOfLine(flags) ? 1 : 0);

    if (myString == NULL)
	myString = (char *) malloc (sizeof(char) * totalLen);
    else
      {
	stringLen = strlen(myString);
	myString  = (char *) realloc ((void *) myString,
				(sizeof(char) * (stringLen + totalLen)));
      }

    if (myString == NULL)
	return -1;

    for (i = stringLen; spaceNum > 0; spaceNum--)
	myString[i++] = ' ';

    while (*spcStr != '\0')
	myString[i++] = *spcStr++;

    if (flags)
	myString[i++] = '\n';

    myString[i] = '\0';

    *prev_info = (SelectionInfo) myString;
    return 0;
}
