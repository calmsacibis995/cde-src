/* $XConsortium: WmBackdrop.c /main/cde1_maint/1 1995/07/18 01:54:59 drk $ */
#ifdef WSM 
/* 
 * (c) Copyright 1987,1988,1989,1990,1992,1993,1994 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

#define	BANDWIDTH		16
#define	BOTTOM			 0
#define CHANGE_BACKDROP		(1L << 0)

#include "WmGlobal.h"
#include "WmResource.h"
#include "WmResNames.h"
#include "WmWrkspace.h"
#define DTWM_NEED_BACKBITS
#include "WmIBitmap.h"
#include "WmBackdrop.h"
#include <X11/Core.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <Dt/Message.h>
#include <Dt/DtP.h>
#include <Dt/WsmM.h>
#include <Xm/Xm.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef   NO_MULTIBYTE
#include <stdlib.h>
#endif
#include "WmIPC.h"	/* must be after DtP.h */

/**************************************
	Functions
**/
/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static Pixmap WmXmGetPixmap2() ;

#else

static Pixmap WmXmGetPixmap2(
			Screen 	*screen,
			char	*pchName,
			Pixel	fg,
			Pixel	bg
			) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

/*
 * externals
 */
#include "WmImage.h"
#include "WmResParse.h"
    
/********************************************
	Globals
**/

/* maximum band width in tile units */
#define MAX_BAND_WIDTH	3
#define TOP_BAND_WIDTH	2

static int         bottom = BOTTOM;

static int	   xa_NO_BACKDROP;




/******************************<->*************************************
 *
 *  ChangeBackdrop ( pWS )
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ChangeBackdrop( pWS )
        WmWorkspaceData *pWS ;
#else
ChangeBackdrop(
        WmWorkspaceData *pWS )
#endif /* _NO_PROTO */
{
    int iwin;


    if (pWS->backdrop.window) 
    {
	if (pWS->backdrop.window == pWS->pSD->lastBackdropWin)
	{
	    /* re-expose the window */
	    XClearWindow (DISPLAY, pWS->backdrop.window);
	}
	else
	{
	    /*
	     * The old and new backdrops are different.
	     * Map the new backdrop and unmap the old.
	     */
	    XLowerWindow(DISPLAY, pWS->backdrop.window);

	    XMapWindow(DISPLAY, pWS->backdrop.window);
	}
    }

    if (pWS->pSD->lastBackdropWin &&
	(pWS->backdrop.window != pWS->pSD->lastBackdropWin))
    {
	XUnmapWindow(DISPLAY, pWS->pSD->lastBackdropWin);
    }

    pWS->pSD->lastBackdropWin = pWS->backdrop.window;

}



/******************************<->*************************************
 *
 *  ProcessBackdropResources (pWS, callFlags)
 *
 *  Description:
 *  -----------
 *  Processes a backdrop for a particular workspace
 *
 *  Inputs:
 *  ------
 *  pWS  = pointer to screen data (backdrop data in particular)
 *  callFlags =  processing flags
 *           CHANGE_BACKDROP - the pixmap has already been created.
 * 
 *  Outputs:
 *  -------
 *  pWS = modifies the backdrop data that's part of this structure
 *
 *  Comments:
 *  ---------
 *  This routine interprets the backdrop.image field and converts
 *  it from a string to the appropriate bitmap/pixmap images.
 *  It also creates windows necessary for the backdrop.
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
ProcessBackdropResources( pWS, callFlags )
        WmWorkspaceData *pWS ;
        unsigned long callFlags ;
#else
ProcessBackdropResources(
        WmWorkspaceData *pWS,
        unsigned long callFlags )
#endif /* _NO_PROTO */
{
    XSetWindowAttributes xswa;
    unsigned int xswamask;
    unsigned char *pchImageName = NULL;
    unsigned char *pchL = NULL;
    unsigned char *pch, *pLine;
    Pixmap tmpPix;
    int x, y;
    unsigned int w, h, bw, depth;
    Window root;
    unsigned long oldFlags;
    static unsigned char *none_string = NULL;
    static unsigned char *no_backdrop_string = NULL;
    Boolean bNone = False;
#ifndef NO_MULTIBYTE
    unsigned int chlen;
#endif /* NO_MULTIBYTE */
    
    if (callFlags & CHANGE_BACKDROP)
    {
	oldFlags = pWS->backdrop.flags;
    }

    if (!no_backdrop_string && 
	(no_backdrop_string = (unsigned char *)
		       XtNewString (DTWM_REQP_BACKDROP_NONE)))
    {
	ToLower(no_backdrop_string);
	xa_NO_BACKDROP = XmInternAtom (DISPLAY, no_backdrop_string, False);

	/* for compatiblity with DT 2.01 */
	none_string = (unsigned char *) XtNewString ("none");
    }
    if (!no_backdrop_string)
    {
	Warning(((char *)GETMESSAGE(6, 4, "Insufficient memory for backdrop window.")));
	return;
    }

    pWS->backdrop.flags = BACKDROP_NONE;	/* by default */

    /*
     *  see if we're using a bitmap 
     */
    if (pWS->backdrop.image)
    {
	/*
	 * Strip off leading '@', if any
	 */
	pch = (unsigned char *) pWS->backdrop.image;
#ifndef NO_MULTIBYTE
 	chlen = mblen ((char *)pch, MB_CUR_MAX);
	if (chlen == 1 && *pch++ == '@')
	{
	    chlen = mblen ((char *)pch, MB_CUR_MAX);
	    if (chlen >= 1)
	    {
		int j;
		int il = 1+strlen ((char *)pch);
		unsigned char *pchD = (unsigned char *)pWS->backdrop.image;

		while (il)
		{
		    *pchD++ = *pch++;
		    il--;
		}
	    }
	}
#else /* NO_MULTIBYTE */
	if (*pch++ == '@' && *pch)
	{
		int il = 1+strlen ((char *)pch);
		unsigned char *pchD = (unsigned char *)pWS->backdrop.image;

		while (il)
		{
		    *pchD++ = *pch++;
		    il--;
		}
	}
#endif /* NO_MULTIBYTE */

	/*
	 * Use a copy of the string because our parsing routines
	 * destroy the thing being parsed. 
	 */

	if ((pLine = pchImageName = (unsigned char *) 
			    strdup (pWS->backdrop.image)) &&
	    (pch = GetString(&pLine)))
	{
	    pchL = (unsigned char *) strdup ((char *)pch);

            if (*pchL) 
		ToLower(pchL);
	
	    if (!(strcmp ((char *)pchL, (char *)no_backdrop_string)) ||
	        !(strcmp ((char *)pchL, (char *)none_string)))
	    {
		/*
		 * No backdrop (root window shows through)
		 */
		pWS->backdrop.window = NULL;
		pWS->backdrop.nameAtom = xa_NO_BACKDROP;
		bNone = True;
	    }

	    if (pch && !bNone)
	    {
		/*
		 * Bitmap backdrop 
		 * Load in the bitmap, create a pixmap of
		 * the right depth, and make the backdrop
		 * window if necessary.
		 */
		if ((callFlags & CHANGE_BACKDROP))
		{
		    /*
		     * We're changing the backdrop, so the
		     * imagePixmap actually contains a depth 1
		     * pixmap. Convert it into a pixmap of the
		     * proper depth.
		     */
		    tmpPix = pWS->backdrop.imagePixmap;
		    pWS->backdrop.imagePixmap = 
			WmXmGetPixmap2 (XtScreen(pWS->workspaceTopLevelW),
				 (char *)pch,
				 pWS->backdrop.foreground,
				 pWS->backdrop.background);
		}
		else 
		{
		    pWS->backdrop.imagePixmap =
			WmXmGetPixmap2 (XtScreen(pWS->workspaceTopLevelW),
				 (char *)pch,
				 pWS->backdrop.foreground,
				 pWS->backdrop.background);
		}

		if ((callFlags & CHANGE_BACKDROP) &&
		    (pWS->backdrop.window))
		{
		    if (pWS->backdrop.imagePixmap !=
			    XmUNSPECIFIED_PIXMAP)
		    {
			XSetWindowBackgroundPixmap (DISPLAY,
			    pWS->backdrop.window,
			    pWS->backdrop.imagePixmap);
		    }
		    else
		    {
			/*
			 * Failed to find bitmap
			 * set background to "background"
			 */
			XSetWindowBackground (DISPLAY, 
			    pWS->backdrop.window, 
			    pWS->backdrop.background);
		    }
		}
		else
		{
		    if (pWS->backdrop.imagePixmap !=
			    XmUNSPECIFIED_PIXMAP)
		    {
			xswa.override_redirect = True;    
			xswa.background_pixmap = 
			    pWS->backdrop.imagePixmap; 
			xswamask = CWOverrideRedirect | CWBackPixmap;
		    }
		    else
		    {
			xswa.override_redirect = True;    
			xswa.background_pixel = 
			    pWS->backdrop.background; 
			xswamask = CWOverrideRedirect | CWBackPixel;
		    }

		    if ((wmGD.keyboardFocusPolicy == 
					KEYBOARD_FOCUS_POINTER) ||
			(wmGD.colormapFocusPolicy == 
					CMAP_FOCUS_POINTER))
		    {
			/*
			 * Listen for enter/levae events if we
			 * have a pointer tracking focus policy
			 */
			xswamask |= CWEventMask;
			xswa.event_mask = EnterWindowMask | 
						LeaveWindowMask;
		    }

		    xswa.backing_store = NotUseful;
		    xswa.save_under = False;
		    xswamask |= (CWBackingStore | CWSaveUnder);

		    pWS->backdrop.window = XCreateWindow(DISPLAY, 
		       pWS->pSD->rootWindow, 
		       0, 0, 
		       DisplayWidth(DISPLAY, pWS->pSD->screen), 
		       DisplayHeight(DISPLAY, pWS->pSD->screen),
		       0, 
		       XDefaultDepth(DISPLAY,pWS->pSD->screen), 
		       CopyFromParent,
		       CopyFromParent, 
		       xswamask, 
		       &xswa);

		}

		if (pch &&
		    (pWS->backdrop.imagePixmap != XmUNSPECIFIED_PIXMAP) &&
		    (pWS->backdrop.window))
		{
		    /* 
		     * Succeeded in setting up a bitmap backdrop.
		     */
		    pWS->backdrop.flags |= BACKDROP_BITMAP;

		    pWS->backdrop.nameAtom = XmInternAtom (DISPLAY, 
					    pWS->backdrop.image, False);
		}
		else
		{
		    char msg[MAXPATH+1];

		    sprintf ((char *)msg, 
		        ((char *)GETMESSAGE(6, 3, "Unable to get image %s for workspace %s.")),
			pWS->backdrop.image, pWS->name);

		    Warning(msg);
		}
		pch = NULL;
	    }
	    if (pchImageName)
	    {
		free (pchImageName);	/* temporary string */
	    }
	    if (pchL)
	    {
		free (pchL);	/* temporary string */
	    }
	}
    }
}


/******************************<->*************************************
 *
 *  static Pixmap WmXmGetPixmap2 
 *
 *  Description:
 *  -----------
 *  Tries twice to get a pixmap from a file name
 *
 *  Inputs:
 *  ------
 *  screen	- ptr to screen
 *  pchName	- image file name
 *  fg		- foreground color
 *  bg		- background color
 * 
 *  Outputs:
 *  -------
 *  Return 	-  pixmap if found, XmUNSPECIFIED_PIXMAP if not
 *
 *  Comments:
 *  ---------
 *  This routine performs some backward compatibility checks.
 *
 *  Do a two stage lookup for backdrop files. If a full path
 *  is specified, but XmGetPixmap fails, the get the basename
 *  of the file and try again. 
 *
 *************************************<->***********************************/
static Pixmap 
#ifdef _NO_PROTO
WmXmGetPixmap2 ( screen, pchName, fg, bg )
	Screen 		*screen;
	char		*pchName;
	Pixel		fg;
	Pixel		bg;
#else
WmXmGetPixmap2 (
        Screen		*screen,
	char		*pchName,
	Pixel		fg,
	Pixel		bg)
#endif /* _NO_PROTO */
{
    Pixmap pixReturn;
    char *pch;

    if (pchName && *pchName) 
    {
	pixReturn = XmGetPixmap (screen, pchName, fg, bg);

	if (pixReturn == XmUNSPECIFIED_PIXMAP)
	{
	    /* 
	     * Use our bitmap lookup paths by using only the 
	     * basename of the file path.
	     */
	    pch = strrchr (pchName, '/');
	    if (pch && 
		(pch < (pchName + strlen(pchName) - 1)))
	    {
		pch++;
		pixReturn = XmGetPixmap (screen, pch, fg, bg);
	    }
	}
    }
    else
    {
	pixReturn = XmUNSPECIFIED_PIXMAP;
    }

    return (pixReturn);
}



/******************************<->*************************************
 *
 *  FullBitmapFilePath (pch)
 *
 *  Description:
 *  -----------
 *  Takes a bitmap file name turns it into a full path name.
 *
 *  Inputs:
 *  ------
 *  pch = ptr to bitmap file name
 * 
 *  Outputs:
 *  -------
 *  Return = ptr to a string containing full path name
 *           or NULL on failure
 *
 *  Comments:
 *  ---------
 *
 *************************************<->***********************************/
String 
#ifdef _NO_PROTO
FullBitmapFilePath( pch )
        String pch ;
#else
FullBitmapFilePath(
        String pch )
#endif /* _NO_PROTO */
{
    String pchR;
    struct stat buf;

    if (*pch != '/')
    {
	pchR = (String) BitmapPathName (pch);

	if ((stat(pchR, &buf) == -1) &&
	    (*pch != '~'))
	{
	    /* file not there! */
	    pchR = pch;
	}
    }
    else
    {
	pchR = pch;
    }
    return (pchR);
}


/******************************<->*************************************
 *
 *  SetNewBackdrop (pWS, pixmap, aName)
 *
 *  Description:
 *  -----------
 *  Sets a new backdrop for a workspace
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pixmap = pixmap for the backdrop (if any)
 *  aName = atomized name for the backdrop (either file name or "none")
 * 
 *  Outputs:
 *  -------
 *  Return = ptr to a string containing full path name
 *           or NULL on failure
 *
 *  Comments:
 *  ---------
 *
 *************************************<->***********************************/
void 
#ifdef _NO_PROTO
SetNewBackdrop( pWS, pixmap, bitmapFile )
        WmWorkspaceData *pWS ;
        Pixmap pixmap ;
        String bitmapFile ;
#else
SetNewBackdrop(
        WmWorkspaceData *pWS,
        Pixmap pixmap,
        String bitmapFile )
#endif /* _NO_PROTO */
{
    String pchNewBitmap = NULL;

    if (!bitmapFile || !strlen(bitmapFile) || 
	!strcmp(bitmapFile, DTWM_REQP_BACKDROP_NONE))
    {
	pixmap = NULL;
    }

    if (bitmapFile) 
    {
	pchNewBitmap = (String) XtNewString (bitmapFile);
    }

    /*
     * Free up old resources 
     */
    if ((pWS->backdrop.imagePixmap) &&
	(pWS->backdrop.imagePixmap != pixmap))
    {
	if (!XmDestroyPixmap (XtScreen(pWS->workspaceTopLevelW), 
			pWS->backdrop.imagePixmap))
	{
	    /* not in Xm pixmap cache */
	}
	pWS->backdrop.imagePixmap = NULL;
    }

    /* free pWS->backdrop.image */
    if ((pWS->backdrop.flags & BACKDROP_IMAGE_ALLOCED) &&
	(pWS->backdrop.image))
    {
	free (pWS->backdrop.image);
    }
	
    pWS->backdrop.imagePixmap = pixmap;
    pWS->backdrop.image = pchNewBitmap;

    ProcessBackdropResources (pWS, CHANGE_BACKDROP);

    if (pchNewBitmap)
    {
	pWS->backdrop.flags |= BACKDROP_IMAGE_ALLOCED;
    }

    ChangeBackdrop (pWS);
    SaveWorkspaceResources (pWS, WM_RES_BACKDROP_IMAGE);

#ifdef HP_VUE
    UpdateWorkspaceInfoProperty (pWS->pSD); /* to be backward compatible */
#endif /* HP_VUE */
    SetWorkspaceInfoProperty (pWS);

    /*
     * Inform the world of the new workspace title
     */
    dtSendWorkspaceModifyNotification(pWS->pSD, pWS->id, 
						DtWSM_REASON_BACKDROP);
}


/******************************<->*************************************
 *
 *  Boolean IsBackdropWindow (pSD, win)
 *
 *  Description:
 *  -----------
 *  Tests a window to see if it is a backdrop window
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  win = window to test.
 * 
 *  Outputs:
 *  -------
 *  Return = True if win is a backdrop window.
 *           False otherwise.
 *
 *  Comments:
 *  ---------
 *
 *************************************<->***********************************/
Boolean 
#ifdef _NO_PROTO
IsBackdropWindow( pSD, win )
        WmScreenData *pSD ;
        Window win ;
#else
IsBackdropWindow(
        WmScreenData *pSD,
        Window win )
#endif /* _NO_PROTO */
{
    Boolean rval = False;
    int i;

    /*
     * Is it one of the backdrop windows for a workspace?
     */
    for (i=0; (i < pSD->numWorkspaces) && !rval; i++)
    {
	if (pSD->pWS[i].backdrop.window == win)
	{
	    rval = True;
	}
    }

    return (rval);
}

#endif /* WSM */
/*********************        eof     ***************************/
