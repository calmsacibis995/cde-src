/* $XConsortium: AsciiSpc.c /main/cde1_maint/1 1995/07/17 17:20:35 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	 AsciiSpc.c
 **
 **   Project:   Cde Help Library
 **
 **   Description: This file resolves an <spc> spec into an ascii 
 **		   representation of the special character.
 **
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
#include <string.h>

/*
 * private includes
 */
#include "AsciiSpcI.h"

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
typedef	struct {
	const char	*symbol;
	const char	*spc_string;
} SpecialSymbolTable;


static	SpecialSymbolTable SpcTable[] =
{ 
	{"[alpha ]", " "         }, /*ISOgrk3, Greek, small alpha    */ 
	{"[aleph ]", " "         }, /*ISOtech, aleph, Hebrews        */ 
	{"[and   ]", "^"         }, /*ISOtech, wedge, logical and    */ 
	{"[ap    ]", " "         }, /*ISOtech, approximately equal   */ 
	{"[beta  ]", " "         }, /*ISOgrk3, Greek, small beta     */ 
	{"[bull  ]", "*"         }, /*ISOpub , round bullet,filled   */ 
	{"[cap   ]", " "         }, /*ISOtech, intersection          */ 
	{"[cent  ]", " "         }, /*ISOnum , cent sign             */ 
	{"[chi   ]", " "         }, /*ISOgrk3, Greek, small chi      */ 
	{"[clubs ]", " "         }, /*ISOpub , clubs suit symbol     */ 
	{"[copy  ]", " "         }, /*ISOnum , copyright symbol      */ 
	{"[cong  ]", " "         }, /*ISOtech, congruent with        */ 
	{"[cup   ]", " "         }, /*ISOtech, union or logical sum  */ 
	{"[curren]", " "         }, /*ISOnum , general currency sign */ 
	{"[darr  ]", " "         }, /*ISOnum , downward arrow        */ 
	{"[dArr  ]", " "         }, /*ISOamsa, down double arrow     */ 
	{"[deg   ]", " "         }, /*ISOnum , degree sign           */ 
	{"[delta ]", " "         }, /*ISOgrk3, Greek, small delta    */ 
	{"[diams ]", " "         }, /*ISOpub , diamond suit symbol   */ 
	{"[divide]", "/"         }, /*ISOnum , divide sign           */ 
	{"[epsiv ]", " "         }, /*ISOgrk3, Greek,sm epsilon, var */ 
	{"[equiv ]", " "         }, /*ISOtech, identical with        */ 
	{"[eta   ]", " "         }, /*ISOgrk3, Greek, small eta      */ 
	{"[exist ]", " "         }, /*ISOtech, at least one exists   */ 
	{"[forall]", " "         }, /*ISOtech, for all               */ 
	{"[frac12]", "1/2"       }, /*ISOnum , fraction one-half     */ 
	{"[frac14]", "1/4"       }, /*ISOnum , fraction one-quarter  */ 
	{"[frac34]", "3/4"       }, /*ISOnum , fraction three-quarter*/ 
	{"[gamma ]", " "         }, /*ISOgrk3, Greek, small gamma    */ 
	{"[ge    ]", ">="        }, /*ISOtech, greater-than-or-equal */ 
	{"[half  ]", "1/2"       }, /*ISOnum , fraction one-half     */ 
	{"[harr  ]", "<-->"      }, /*ISOamsa, left & right arrow    */ 
	{"[hArr  ]", "<<==>>"    }, /*ISOamsa, l & r double arrow    */ 
	{"[hearts]", " "         }, /*ISOpub , hearts suit symbol    */ 
	{"[hellip]", "..."       }, /*ISOpub , ellipsis(horizontal)  */ 
	{"[iexcl ]", " "         }, /*ISOnum , inverted ! mark       */ 
	{"[image ]", " "         }, /*ISOamso,imaginary number symbol*/ 
	{"[infin ]", " "         }, /*ISOtech, infinity              */ 
	{"[iota  ]", " "         }, /*ISOgrk3, Greek, small iota     */ 
	{"[iquest]", " "         }, /*ISOnum , inverted ? mark       */ 
	{"[isin  ]", " "         }, /*ISOtech, set membership        */ 
	{"[kappa ]", " "         }, /*ISOgrk3, Greek, small kappa    */ 
	{"[lambda]", " "         }, /*ISOgrk3, Greek, small lambda   */ 
	{"[larr  ]", "<--"       }, /*ISOnum , leftward arrow        */ 
	{"[le    ]", "=<"        }, /*ISOtech, less-than-or-equal    */ 
	{"[lsquo ]", "`"         }, /*ISOnum , left single quote     */ 
	{"[mdash ]", "--"        }, /*ISOpub , em dash(long dash)    */ 
	{"[mu    ]", " "         }, /*ISOgrk3, Greek, small mu       */ 
	{"[ndash ]", "-"         }, /*ISOpub , en dash(short dash)   */ 
	{"[ne    ]", "!="        }, /*ISOtech, not equal             */ 
	{"[not   ]", " "         }, /*ISOnum , not                   */ 
	{"[notin ]", " "         }, /*ISOtech, negated set membership*/ 
	{"[nu    ]", " "         }, /*ISOgrk3, Greek, small nu       */ 
	{"[omega ]", " "         }, /*ISOgrk3, Greek, small omega    */ 
	{"[oplus ]", " "         }, /*ISOamsb, plus sign in circle   */ 
	{"[or    ]", "V"         }, /*ISOtech, vee, logical or       */ 
	{"[osol  ]", " "         }, /*ISOamsb, slash in circle       */ 
	{"[otimes]", " "         }, /*ISOamsb,multiply sign in circle*/ 
	{"[over  ]", " "         }, /*made up, over symbol           */ 
	{"[para  ]", " "         }, /*ISOnum , paragraph sign        */ 
	{"[part  ]", " "         }, /*ISOtech, partial differential  */ 
	{"[perp  ]", " "         }, /*ISOtech, perpendicular         */ 
	{"[phis  ]", " "         }, /*ISOgrk3, Greek, small phi      */ 
	{"[pi    ]", " "         }, /*ISOgrk3, Greek, small pi       */ 
	{"[piv   ]", " "         }, /*ISOgrk3, Greek, small pi, var  */ 
	{"[plusmn]", "+/-"       }, /*ISOnum , plus or minus sign    */ 
	{"[pound ]", "#"         }, /*ISOnum , pound sign            */ 
	{"[prime ]", "\'"        }, /*ISOtech, prime or minute       */ 
	{"[prop  ]", " "         }, /*ISOtech, proportional to       */ 
	{"[psi   ]", " "         }, /*ISOgrk3, Greek, small psi      */ 
	{"[rarr  ]", "-->"       }, /*ISOnum , rightward arrow       */ 
	{"[rArr  ]", "==>>"      }, /*ISOtech, right double arrow    */ 
	{"[real  ]", " "         }, /*ISOamso, real number symbol    */ 
	{"[reg   ]", " "         }, /*ISOnum,circledR,registered sign*/ 
	{"[rho   ]", " "         }, /*ISOgrk3, Greek, small rho      */ 
	{"[sect  ]", " "         }, /*ISOnum , section sign          */ 
	{"[sigma ]", " "         }, /*ISOgrk3, Greek, small sigma    */ 
	{"[spades]", " "         }, /*ISOpub , spades suit symbol    */ 
	{"[sub   ]", " "         }, /*ISOtech, subset/is implied by  */ 
	{"[sube  ]", " "         }, /*ISOtech, subset, equals        */ 
	{"[sup   ]", " "         }, /*ISOtech, superset or implies   */ 
	{"[supe  ]", " "         }, /*ISOtech, superset, equals      */ 
	{"[tau   ]", " "         }, /*ISOgrk3, Greek, small tau      */ 
	{"[there4]", " "         }, /*ISOtech, therefore             */ 
	{"[thetas]", " "         }, /*ISOgrk3, Greek, small theta    */ 
	{"[thetav]", " "         }, /*ISOgrk3, Greek, small theta,var*/ 
	{"[times ]", "x"         }, /*ISOnum , multipy sign          */ 
	{"[trade ]", " "         }, /*ISOnum , trade mark sign       */ 
	{"[uarr  ]", " "         }, /*ISOnum , upward arrow          */ 
	{"[uArr  ]", " "         }, /*ISOamsa, up double arrow       */ 
	{"[upsi  ]", " "         }, /*ISOgrk3, Greek, small upsilon  */ 
	{"[vellip]", ".\n.\n.\n" }, /*ISOpub , vertical ellipsis     */ 
	{"[weierp]", " "         }, /*ISOamso, Weierstrass p         */ 
	{"[xi    ]", " "         }, /*ISOgrk3, Greek, small xi       */ 
	{"[yen   ]", " "         }, /*ISOnum , yen sign              */ 
	{"[zeta  ]", " "         }, /*ISOgrk3, Greek, small zeta     */ 
	{"[Delta ]", " "         }, /*ISOgrk3, Greek, large delta    */ 
	{"[Gamma ]", " "         }, /*ISOgrk3, Greek, large gamma    */ 
	{"[Lambda]", " "         }, /*ISOgrk3, Greek, large lambda   */ 
	{"[Omega ]", " "         }, /*ISOgrk3, Greek, large omega    */ 
	{"[Pi    ]", " "         }, /*ISOgrk3, Greek, large pi       */ 
	{"[Prime ]", "\""        }, /*ISOtech, double prime/second   */ 
	{"[Phi   ]", " "         }, /*ISOgrk3, Greek, large phi      */ 
	{"[Psi   ]", " "         }, /*ISOgrk3, Greek, large psi      */ 
	{"[Sigma ]", " "         }, /*ISOgrk3, Greek, large sigma    */ 
	{"[Theta ]", " "         }, /*ISOgrk3, Greek, large theta    */ 
	{"[Upsi  ]", " "         }, /*ISOgrk3, Greek, large upsilon  */ 
	{"[Xi    ]", " "         }, /*ISOgrk3, Greek, large xi       */ 
	{ NULL     , " "         }, /* default character to use      */
      };

/******************************************************************************
 *
 * Semi Public functions
 *
 ******************************************************************************/
/*****************************************************************************
 * Function:	const char *_DtHelpCeResolveSpcToAscii ();
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
_DtHelpCeResolveSpcToAscii (spc_symbol)
    const char	*spc_symbol;
#else
_DtHelpCeResolveSpcToAscii (
    const char	*spc_symbol)
#endif /* _NO_PROTO */
{
    register int          i = 0;

    while (SpcTable[i].symbol != NULL &&
				strcmp(SpcTable[i].symbol, spc_symbol) != 0)
	i++;

    if (SpcTable[i].symbol != NULL)
	return (SpcTable[i].spc_string);

    return NULL;
}
