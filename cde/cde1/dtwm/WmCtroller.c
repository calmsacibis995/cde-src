#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmCtroller.c,v $ $Revision: 1.9 $ $Date: 94/01/07 13:35:24 $"
#endif
#endif

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmHelp.h"
#include <X11/Core.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Dt/Help.h>
#include <Xm/MessageB.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#ifndef NO_MULTIBYTE
#include <stdlib.h>
#endif /* NO_MULTIBYTE */
#ifdef PANELIST
#include "WmPanelP.h"
#endif /* PANELIST */

/*
 * Function Declarations:
 */

#include "WmCtroller.h"


/*
 * External references
 */
#include "WmIconBox.h"
#include "WmManage.h"
#include "WmResource.h"
#include "WmFunction.h"
#include "WmWrkspace.h"
#include "WmResParse.h"



/*************************************<->*************************************
 *
 *  void
 *  ShowController (pSD)
 *
 *         Remove when unreferenced by WmFunction
 * 
 ******************************<->***********************************/

void 
#ifdef _NO_PROTO
ShowController( pSD )
        WmScreenData *pSD ;
#else
ShowController(
        WmScreenData *pSD )
#endif /* _NO_PROTO */

{
}


/*************************************<->*************************************
 *
 *  Boolean
 *  ValidWsName (title)
 *
 *
 *  Description:
 *  -----------
 *  Tests a title to see if it could be a valid workpace title
 *
 *  Inputs:
 *  ------
 *  title = ptr to character string containing workspace title
 *
 * 
 *  Outputs:
 *  -------
 *  Return = True if valid
 *
 *
 *  Comments:
 *  --------
 *  Disallows the following characters: 
 *    '*'	Conflicts with resource specification
 *    '.'	Conflicts with resource specification
 *    ':'	Conflicts with resource specification
 *    '"' 	Used for quoting names
 *    '\' 	Used for quoting characters (conflict with session mgr)
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
ValidWsName( title )
        unsigned char *title ;
#else
ValidWsName(
        unsigned char *title )
#endif /* _NO_PROTO */
{
    int i;
    int lastNonBlank = -1;
    Boolean valid = True;
#ifndef NO_MULTIBYTE
    int chlen = 999;
#else /* NO_MULTIBYTE */
    int len;
#endif  /* NO_MULTIBYTE */

    if (!title || strlen((char *)title) == 0)
    {
	valid = False;
    }

#ifndef NO_MULTIBYTE
    for (i = 0; (i < strlen((char *)title)) && (chlen>0) && valid; i += chlen)
    {
        chlen = mblen ((char *)&title[i], MB_CUR_MAX);
	/*
	 *  Disallow a leading blank
	 */
	if ((chlen == 1) &&
	     (title[i] == ' '))
	{
	    if (i == 0) 
	    {
		valid = False;
	    }
	}
	else
	{
	    lastNonBlank = i + chlen - 1;
	}

	if ((chlen < 0) ||
	    ((chlen == 1) &&
	     ((title[i] == '*') ||
	      (title[i] == '.') ||
	      (title[i] == '"') ||
	      (title[i] == '\\') ||
	      (title[i] == ':'))))
	{
	    valid = False;
	}
    }
#else /* NO_MULTIBYTE */
    len = strlen((char *)title);
    if ((len == 0) ||  
	(strchr (" \t", (int) title[0])))
    {
	/* don't allow leading white space */
	valid = False;
    }

    for (i = 0; (i < strlen((char *)title)) && valid; i++)
    {
	if (!strchr (" \t", (int) title[i]))
	{
	    lastNonBlank = i;
	}

	if (strchr ("*.:\"\\", (int) title[i]))
	{
	    valid = False;
	}
    }
#endif /* NO_MULTIBYTE */

    /*
     * Kill off trailing white space
     */
    if (lastNonBlank >= 0)
    {
	title[lastNonBlank+1] = (unsigned char) 0;
    }

    return (valid);
}




/*************************************<->*************************************
 *
 *  void
 *  ChangeWorkspaceTitle (pWS, xmstr)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  xmstr = new title (XmString)
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
#ifdef _NO_PROTO
ChangeWorkspaceTitle( pWS, xmstr )
        WmWorkspaceData *pWS ;
        XmString xmstr ;
#else
ChangeWorkspaceTitle(
        WmWorkspaceData *pWS,
        XmString xmstr )
#endif /* _NO_PROTO */
{
    WsClientData *pWsc;
    int i;

    /*
     * This may take a while...
     */
    ShowWaitState (True);

    /* 
     * change title within this workspace data 
     */

    if (pWS->title)
    {
	XmStringFree (pWS->title);
    }

    pWS->title = xmstr;

    /*
     * Save changes to resource database
     */
    SaveWorkspaceResources (pWS, (WM_RES_WORKSPACE_TITLE), False);

    /* 
     * Replace old workspace in info property 
     */
    SetWorkspaceInfoProperty (pWS);
    UpdateWorkspaceInfoProperty (pWS->pSD); /* to be backward compatible */

    /*
     * Ok, we're back...
     */
    ShowWaitState (False);
}

#endif
/****************************   eof    ***************************/
