#ifdef VERBOSE_REV_INFO
static char rcsid[] = "$XConsortium: WmPanelP.c /main/cde1_maint/1 1995/07/15 01:55:24 drk $";
#endif /* VERBOSE_REV_INFO */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmPanelP.c
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains private Front Panel functions for parsing,
 **  and for Item and List pseudo classes.
 **
 *********************************************************************
 **
 ** (c) Copyright 1992 HEWLETT-PACKARD COMPANY
 ** ALL RIGHTS RESERVED
 **
 **********************************************************************
 **********************************************************************
 **
 **
 **********************************************************************
 ******************************<+>*************************************/


#include "WmPanelP.h"

#include "WmParse.h"
#include "WmParseP.h"

#include <Dt/ControlP.h>

/*
 *  WmFpString Function Definitions
 */

/*	String To Boolean Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToBooleanToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToBooleanToken (WmFpString pchItem)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, "true"))
    return WM_FP_TOK_true;
else if (! strcmp ((char *) pchItem, "false"))
    return WM_FP_TOK_false;
else
    return WM_FP_TOK_none;
}

/*	String To Item Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToItemToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToItemToken (WmFpString pchItem)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
if (! strcmp ((char *) pchItem, DTWM_FP_PANEL))
    return WM_FP_TOK_panel;
else if (! strcmp ((char *) pchItem, DTWM_FP_BOX))
    return WM_FP_TOK_box;
else if (! strcmp ((char *) pchItem, DTWM_FP_SWITCH))
    return WM_FP_TOK_switch;
else if (! strcmp ((char *) pchItem, DTWM_FP_CONTROL))
    return WM_FP_TOK_control;
else if (! strcmp ((char *) pchItem, DTWM_FP_ANIMATION))
    return WM_FP_TOK_animation;
else
    return WM_FP_TOK_none;
}

/*	String to Panel Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToPanelToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToPanelToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
if (! strcmp ((char *) pchItem, DTWM_FP_BOX))
    return WM_FP_TOK_box;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_STRING))
    return WM_FP_TOK_helpString;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_TOPIC))
    return WM_FP_TOK_helpTopic;
else if (! strcmp ((char *) pchItem, DTWM_FPA_TYPE))
    return WM_FP_TOK_type;
else if (! strcmp ((char *) pchItem, DTWM_FPA_SUBPANEL_DIRECTION))
    return WM_FP_TOK_subpanelDirection;
else if (! strcmp ((char *) pchItem, DTWM_FPA_ORIENTATION))
    return WM_FP_TOK_orientation;
else
    return WM_FP_TOK_none;
}

/*	String to Box Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToBoxToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToBoxToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
if (! strcmp ((char *) pchItem, DTWM_FP_CONTROL))
    return WM_FP_TOK_control;
else if (! strcmp ((char *) pchItem, DTWM_FP_BOX))
    return WM_FP_TOK_box;
else if (! strcmp ((char *) pchItem, DTWM_FP_SWITCH))
    return WM_FP_TOK_switch;
else if (! strcmp ((char *) pchItem, DTWM_FPA_TYPE))
    return WM_FP_TOK_type;
else if (! strcmp ((char *) pchItem, DTWM_FPA_TITLE))
    return WM_FP_TOK_title;
else if (! strcmp ((char *) pchItem, DTWM_FPA_BACKGROUND_TILE))
    return WM_FP_TOK_backgroundTile;
else if (! strcmp ((char *) pchItem, DTWM_FPA_LAYOUT_POLICY))
    return WM_FP_TOK_layoutPolicy;
else if (! strcmp ((char *) pchItem, DTWM_FPA_ORIENTATION))
    return WM_FP_TOK_orientation;
else if (! strcmp ((char *) pchItem, DTWM_FPA_NUMBER_OF_ROWS))
    return WM_FP_TOK_numberOfRows;
else if (! strcmp ((char *) pchItem, DTWM_FPA_NUMBER_OF_COLUMNS))
    return WM_FP_TOK_numberOfColumns;
else if (! strcmp ((char *) pchItem, DTWM_FPA_COLOR_SET))
    return WM_FP_TOK_colorSet;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_STRING))
    return WM_FP_TOK_helpString;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_TOPIC))
    return WM_FP_TOK_helpTopic;
else
    return WM_FP_TOK_none;
}

/*	String to Layout Policy Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToLayoutPolicyToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToLayoutPolicyToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, DTWM_FPT_STANDARD))
    return WM_FP_TOK_standard;
else if (! strcmp ((char *) pchItem, DTWM_FPT_AS_NEEDED))
    return WM_FP_TOK_asNeeded;
else
    return WM_FP_TOK_none;
}

/*	String to Orientation Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToOrientationToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToOrientationToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, DTWM_FPT_HORIZONTAL))
    return WM_FP_TOK_horizontal;
else if (! strcmp ((char *) pchItem, DTWM_FPT_VERTICAL))
    return WM_FP_TOK_vertical;
else
    return WM_FP_TOK_none;
}

/*	String to Switch Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToSwitchToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToSwitchToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
if (! strcmp ((char *) pchItem, DTWM_FPA_ORIENTATION))
    return WM_FP_TOK_orientation;
else if (! strcmp ((char *) pchItem, DTWM_FPA_NUMBER_OF_ROWS))
    return WM_FP_TOK_numberOfRows;
else if (! strcmp ((char *) pchItem, DTWM_FPA_NUMBER_OF_COLUMNS))
    return WM_FP_TOK_numberOfColumns;
else if (! strcmp ((char *) pchItem, DTWM_FP_CONTROL))
    return WM_FP_TOK_control;
else if (! strcmp ((char *) pchItem, DTWM_FPT_SUBPANEL))
    return WM_FP_TOK_subpanel;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_STRING))
    return WM_FP_TOK_helpString;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_TOPIC))
    return WM_FP_TOK_helpTopic;
else if (! strcmp ((char *) pchItem, DTWM_FPA_LAYOUT_POLICY))
    return WM_FP_TOK_layoutPolicy;
else
    return WM_FP_TOK_none;
}

/*	String to Control Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToControlToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToControlToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
if (! strcmp ((char *) pchItem, DTWM_FPA_IMAGE))
    return WM_FP_TOK_image;
else if (! strcmp ((char *) pchItem, DTWM_FPA_LABEL))
    return WM_FP_TOK_label;
else if (! strcmp ((char *) pchItem, DTWM_FPT_SUBPANEL))
    return WM_FP_TOK_subpanel;
else if (! strcmp ((char *) pchItem, DTWM_FPA_PUSH_RECALL))
    return WM_FP_TOK_pushRecall;
else if (! strcmp ((char *) pchItem, DTWM_FPA_PUSH_ACTION))
    return WM_FP_TOK_pushAction;
else if (! strcmp ((char *) pchItem, DTWM_FPA_CLIENT_GEOMETRY))
    return WM_FP_TOK_clientGeometry;
else if (! strcmp ((char *) pchItem, DTWM_FPA_PUSH_ANIMATION))
    return WM_FP_TOK_pushAnimation;
else if (! strcmp ((char *) pchItem, DTWM_FPA_ALTERNATE_IMAGE))
    return WM_FP_TOK_alternateImage;
else if (! strcmp ((char *) pchItem, DTWM_FPA_DROP_ACTION))
    return WM_FP_TOK_dropAction;
else if (! strcmp ((char *) pchItem, DTWM_FPA_DROP_ANIMATION))
    return WM_FP_TOK_dropAnimation;
else if (! strcmp ((char *) pchItem, DTWM_FPA_CLIENT_NAME))
    return WM_FP_TOK_clientName;
else if (! strcmp ((char *) pchItem, DTWM_FPA_TYPE))
    return WM_FP_TOK_type;
else if (! strcmp ((char *) pchItem, DTWM_FPT_MONITOR_FILE))
    return WM_FP_TOK_monitorFile;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_STRING))
    return WM_FP_TOK_helpString;
else if (! strcmp ((char *) pchItem, DTWM_FPA_HELP_TOPIC))
    return WM_FP_TOK_helpTopic;
else if (! strcmp ((char *) pchItem, DTWM_FPA_TITLE))
    return WM_FP_TOK_title;
else
    return WM_FP_TOK_none;
}

/*	String to Box Type Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToBoxTypeToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToBoxTypeToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, DTWM_FPT_PRIMARY))
    return WM_FP_TOK_primary;
else if (! strcmp ((char *) pchItem, DTWM_FPT_SECONDARY))
    return WM_FP_TOK_secondary;
else if (! strcmp ((char *) pchItem, DTWM_FPT_SUBPANEL))
    return WM_FP_TOK_subpanel;
else if (! strcmp ((char *) pchItem, DTWM_FPT_ROW_COLUMN))
    return WM_FP_TOK_rowColumn;
else
    return WM_FP_TOK_none;
}

/*	String to Control Type Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToControlTypeToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToControlTypeToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, DTWM_FPT_CLOCK))
    return WM_FP_TOK_clock;
else if (! strcmp ((char *) pchItem, DTWM_FPT_BUTTON))
    return WM_FP_TOK_button;
else if (! strcmp ((char *) pchItem, DTWM_FPT_BLANK))
    return WM_FP_TOK_blank;
else if (! strcmp ((char *) pchItem, DTWM_FPT_DATE))
    return WM_FP_TOK_date;
else if (! strcmp ((char *) pchItem, DTWM_FPT_BUSY))
    return WM_FP_TOK_busy;
else if (! strcmp ((char *) pchItem, DTWM_FPT_CLIENT))
    return WM_FP_TOK_client;
else if (! strcmp ((char *) pchItem, DTWM_FPT_ICONBOX))
    return WM_FP_TOK_iconBox;
else if (! strcmp ((char *) pchItem, DTWM_FPT_MAIL))
    return WM_FP_TOK_mail;
else if (! strcmp ((char *) pchItem, DTWM_FPT_MONITOR_FILE))
    return WM_FP_TOK_monitorFile;
else
    return WM_FP_TOK_none;
}

/*	String to Subpanel Direction Token
 */
#ifdef _NO_PROTO
WmFpToken
WmFpStringToSubpanelDirectionToken (pchItem)
#else /* _NO_PROTO */
WmFpToken
WmFpStringToSubpanelDirectionToken (	WmFpString	 pchItem	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpString	 pchItem;
#endif /* _NO_PROTO */
{
_DtWmParseToLower ((unsigned char *) pchItem);
if (! strcmp ((char *) pchItem, DTWM_FPT_NORTH))
    return WM_FP_TOK_north;
else if (! strcmp ((char *) pchItem, DTWM_FPT_SOUTH))
    return WM_FP_TOK_south;
else if (! strcmp ((char *) pchItem, DTWM_FPT_EAST))
    return WM_FP_TOK_east;
else if (! strcmp ((char *) pchItem, DTWM_FPT_WEST))
    return WM_FP_TOK_west;
else
    return WM_FP_TOK_none;
}


/*
 *  WmFpItem Function Definitions
 */

/*	Item Create
 */
#ifdef _NO_PROTO
WmFpItem	WmFpItemCreate (eTokItem, pchName, pchTokens, iTokens)
	WmFpToken		 eTokItem;
	WmFpString	 pchName;
	WmFpStringList	 pchTokens;
	int		 iTokens;
#else /* _NO_PROTO */
WmFpItem	WmFpItemCreate (	WmFpToken		 eTokItem,
			WmFpString	 pchName,
			WmFpStringList	 pchTokens,
			int		 iTokens)
#endif /* _NO_PROTO */
{
WmFpItem	pItem = (WmFpItem) XtMalloc (sizeof (WmFpItemData));

pItem->eTokItem = eTokItem;
pItem->pchName = pchName;
pItem->ppchTokens = pchTokens;
pItem->iTokens = iTokens;

return pItem;
}


/*
 *	Item Destroy
 */
#ifdef _NO_PROTO
void	WmFpItemDestroy (item)
	WmFpItem       item;
#else /* _NO_PROTO */
void	WmFpItemDestroy (	WmFpItem		 item	)
#endif /* _NO_PROTO */
{
XtFree ((char *) item->pchName);
XtFree ((char *) item->ppchTokens);
XtFree ((char *) item);
}


/*
 *  WmFpList Function Definitions
 */

/*	List Create
 */
#ifdef _NO_PROTO
WmFpList	WmFpListCreate ()
#else /* _NO_PROTO */
WmFpList	WmFpListCreate ()
#endif /* _NO_PROTO */
{
WmFpList	pList = (WmFpList) XtMalloc (sizeof (WmFpListData));

pList->ppItems = (WmFpItemList) XtMalloc (sizeof (WmFpItem) * WM_FP_MAX_ITEMS);
pList->iMaxItems = WM_FP_MAX_ITEMS;
pList->iItems = 0;

return pList;
}

/*	List Destroy
 */
#ifdef _NO_PROTO
void	WmFpListDestroy (pList)
	WmFpList   pList;
# else /* _NO_PROTO */
void	WmFpListDestroy (	WmFpList		 pList	)
#endif /* _NO_PROTO */
{
int	i;
for (i = 0; i < pList->iItems; i++)
    {
    if (pList->ppItems[i] != NULL)
	WmFpItemDestroy(pList->ppItems[i]);
    }
XtFree ((char *) pList->ppItems);
XtFree ((char *) pList);
}

/*	List Add Item
 */
#ifdef _NO_PROTO
void	WmFpListAddItem (pList, pItem)
	WmFpList   pList;
	WmFpItem   pItem;
#else /* _NO_PROTO */
void	WmFpListAddItem (	WmFpList		 pList,
			WmFpItem		 pItem	)
#endif /* _NO_PROTO */
{
/*
 *  Allocate more space.
 */
if (pList->iItems == pList->iMaxItems)
    {
    pList->iMaxItems += WM_FP_MAX_ITEMS;
    pList->ppItems = (WmFpItemList) XtRealloc ((char *) pList->ppItems,
					sizeof (WmFpItem) * pList->iMaxItems);
    }
/*
 *  Add Item.
 */
pList->ppItems[pList->iItems++] = pItem;
}

/*	List Get Item Pos
 */
#ifdef _NO_PROTO
int	WmFpListGetItemPos (pList, pchName)
#else /* _NO_PROTO */
int	WmFpListGetItemPos (	WmFpList		 pList,
				WmFpString	 pchName	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpList		 pList;
WmFpString	 pchName;
#endif /* _NO_PROTO */
{
int		 iPos;
int		 i;
Boolean		 bFound;

for (	bFound = False, i = pList->iItems - 1, iPos = 0;
	( (! bFound) && (i >= 0) );
	i--	)
    if ( (pList->ppItems[i] != NULL) &&
	 (! strcmp ((char *) pchName, (char *) pList->ppItems[i]->pchName)) )
	{
	bFound = True;
	iPos = i + 1;
	}

return iPos;
}

/*	List Get Item
 */
#ifdef _NO_PROTO
WmFpItem	WmFpListGetItem (pList, iPos)
#else /* _NO_PROTO */
WmFpItem	WmFpListGetItem (	WmFpList		 pList,
			int		 iPos	)
#endif /* _NO_PROTO */
#ifdef _NO_PROTO
WmFpList		 pList;
int		 iPos;
#endif /* _NO_PROTO */
{
if ( (iPos > 0) && (iPos <= pList->iItems) )
    return pList->ppItems[iPos-1];
else
    return NULL;
}

