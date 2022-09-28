/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmImage.c /main/cde1_maint/2 1995/10/03 17:25:20 lehors $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#define MWM_NEED_IIMAGE
#include "WmIBitmap.h"

#ifdef MOTIF_ONE_DOT_ONE
#include <stdio.h>
#include <pwd.h>
#define MATCH_CHAR 'P'		/* Default match character - defined in Xmos.p */
#else
#include <Xm/XmosP.h>
#endif
#ifdef WSM
#include <Xm/IconFile.h>
#include <Dt/GetDispRes.h>
#endif

#define MATCH_XBM 'B'		/* .xbm character: see XmGetPixmap */
#define MATCH_PATH "XBMLANGPATH"

/*
 * include extern functions
 */

#include "WmImage.h"
#include "WmGraphics.h"
#include "WmResource.h"
#include "WmResParse.h"
#include "WmMenu.h"
#include "WmError.h"

#ifdef MOTIF_ONE_DOT_ONE
extern char    *getenv ();
#endif


/******************************<->*************************************
 *
 *  MakeClientIconPixmap (pCD, iconBitmap, iconMask)
 *
 *
 *  Description:
 *  -----------
 *  This function takes a client supplied icon image pixmap and mask 
 *  bitmap and makes it into a colored pixmap suitable for use as an 
 *  icon image.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (icon colors and tiles)
 *
 *  iconBitmap = icon bitmap 
 *  iconMask   = mask bitmap
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = icon pixmap (NULL if error)
 *
 ******************************<->***********************************/

#ifdef _NO_PROTO
Pixmap MakeClientIconPixmap (pCD, iconBitmap, iconMask)

    ClientData *pCD;
    Pixmap iconBitmap;
    Pixmap iconMask;

#else /* _NO_PROTO */
Pixmap MakeClientIconPixmap (
	ClientData *pCD, 
	Pixmap iconBitmap, 
	Pixmap iconMask)
#endif /* _NO_PROTO */
{
    Window root;
    int x;
    int y;
    unsigned int bitmapWidth;
    unsigned int bitmapHeight;
    unsigned int border;
    unsigned int depth;

    
    /*
     * Check out the attributes of the bitmap to insure that it is usable.
     */

    if (!XGetGeometry (DISPLAY, iconBitmap, &root, &x, &y,
	     &bitmapWidth, &bitmapHeight, &border, &depth))
    {
	Warning (((char *)GETMESSAGE(38, 1, "Invalid icon bitmap")));
	return ((Pixmap)NULL);
    }

    if (ROOT_FOR_CLIENT(pCD) != root)
    {
	/*
	 * The bitmap was not made with usable parameters.
	 */
#ifdef WSM
	Warning (((char *)GETMESSAGE(38, 9, "Icon bitmap cannot be used on this screen")));
#else /* WSM */
	Warning ("Invalid root for icon bitmap");
#endif /* WSM */
	return ((Pixmap)NULL);
    }

#ifdef DISALLOW_DEEP_ICONS
    if (depth != 1)
    {
	Warning (((char *)GETMESSAGE(38, 10, "Color icon pixmap not supported")));
	return ((Pixmap)NULL);
    }
#endif

    /*
     * Color the bitmap, center it in a pixmap ....
     */

    return (MakeIconPixmap (pCD, iconBitmap, (Pixmap)iconMask, 
	    bitmapWidth, bitmapHeight, depth));


} /* END OF FUNCTION MakeClientIconPixmap */



/*************************************<->*************************************
 *
 *  MakeNamedIconPixmap (pCD, iconName)
 *
 *
 *  Description:
 *  -----------
 *  This function makes an icon pixmap for a particular client given the
 *  name of a bitmap file.
 *
 *
 *  Inputs:
 *  ------
 *  pCD      = (nonNULL) pointer to client data
 *  iconName = pointer to the icon name (bitmap file path name or NULL)
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = icon pixmap or NULL
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Pixmap MakeNamedIconPixmap (pCD, iconName)

    ClientData *pCD;
    String      iconName;

#else /* _NO_PROTO */
Pixmap MakeNamedIconPixmap (ClientData *pCD, String iconName)
#endif /* _NO_PROTO */
{
    int          bitmapIndex;
#ifdef WSM
    Pixmap	pixmap, pixmap_r, mask;
    Window	root;
    int		x, y;
    unsigned int	width, height, border_width, depth;
    String	sIconFileName = (String)NULL;
    int		iconSizeDesired;
#endif /* WSM */

    /*
     * Get the bitmap cache entry (will read data from file if necessary).
     * If unable to find the iconName file return NULL.
     */

#ifdef WSM
    if ((bitmapIndex = GetBitmapIndex (PSD_FOR_CLIENT(pCD), iconName,
			False)) < 0)
#else /* WSM */
    if ((bitmapIndex = GetBitmapIndex (PSD_FOR_CLIENT(pCD), iconName)) < 0)
#endif /* WSM */
    {
#ifdef WSM
       if ((PSD_FOR_CLIENT(pCD)->displayResolutionType == LOW_RES_DISPLAY) ||
           (PSD_FOR_CLIENT(pCD)->displayResolutionType == VGA_RES_DISPLAY))
       {
          iconSizeDesired = XmMEDIUM_ICON_SIZE;
       }
       else
       {
          iconSizeDesired = XmLARGE_ICON_SIZE;
       }

       pixmap = XmUNSPECIFIED_PIXMAP;

       sIconFileName = XmGetIconFileName( 
			XtScreen(PSD_FOR_CLIENT(pCD)->screenTopLevelW1),
			NULL,
			iconName,
			NULL,
			iconSizeDesired);

       if (sIconFileName != NULL)
       {
	   pixmap = XmGetPixmap (
			XtScreen (PSD_FOR_CLIENT(pCD)->screenTopLevelW1), 
			sIconFileName,
			pCD->iconImageForeground, 
			pCD->iconImageBackground);
       }

       if (pixmap == XmUNSPECIFIED_PIXMAP)
       {
	   pixmap = XmGetPixmap (
			XtScreen (PSD_FOR_CLIENT(pCD)->screenTopLevelW1), 
			iconName,
			pCD->iconImageForeground, 
			pCD->iconImageBackground);
       }

       if (pixmap == XmUNSPECIFIED_PIXMAP)
       {
            MWarning (((char *)GETMESSAGE(38, 7, "Unable to read bitmap file %s\n")), iconName);
       }
       else
       {
	   mask = _DtGetMask (XtScreen(PSD_FOR_CLIENT(pCD)->screenTopLevelW1), 
			(sIconFileName == NULL) ? iconName : sIconFileName);
	   if (mask == XmUNSPECIFIED_PIXMAP) 
	   {
	       mask = (Pixmap) NULL;
	   }

	   /*
	    * We need to synchronize DISPLAY1 here because the pixmap and mask 
	    * were created on that display connection, and we must ensure that 
	    * the server has processed those requests before we attempt to use 
	    * the pixmap and mask on DISPLAY.  We thus assume that the 
	    * subsequent XGetGeometry() call is successful.
	    */
	   XSync(DISPLAY1, False);	

	   (void) XGetGeometry (DISPLAY, pixmap, &root, &x, &y, &width,
		       &height, &border_width, &depth);

	   pixmap_r = MakeIconPixmap (pCD, pixmap, mask,
				     width, height, depth);

	   XmDestroyPixmap (XtScreen (PSD_FOR_CLIENT(pCD)->screenTopLevelW1), 
				pixmap);
	   if (mask)
	       XmDestroyPixmap (
		   XtScreen (PSD_FOR_CLIENT(pCD)->screenTopLevelW1), mask);

           if (sIconFileName != NULL)
             XtFree (sIconFileName);
	   return (pixmap_r);
       }

       if (sIconFileName != NULL)
	   XtFree (sIconFileName);
#endif /* WSM */
	return ((Pixmap)NULL);
    }

    /*
     * Color the bitmap, center it in a pixmap ....
     */

    return (MakeCachedIconPixmap (pCD, bitmapIndex, (Pixmap)NULL));

} /* END OF FUNCTION MakeNamedIconPixmap */




/*************************************<->*************************************
 *
 *  Pixmap
 *  MakeCachedIconPixmap (pCD, bitmapIndex, mask)
 *
 *
 *  Description:
 *  -----------
 *  Convert the cached bitmap and mask into an icon pixmap.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- (nonNULL) pointer to client data (icon colors and tiles)
 *  bitmapIndex	- bitmap cache index of image to be converted
 *  mask	- bitmap mask, 1 for bits of "bitmap" to be kept
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN 	- icon pixmap or NULL
 *
 *
 *  Comments:
 *  --------
 *  o "mask" is not used.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Pixmap MakeCachedIconPixmap (pCD, bitmapIndex, mask)

    ClientData *pCD;
    int         bitmapIndex;
    Pixmap      mask;

#else /* _NO_PROTO */
Pixmap MakeCachedIconPixmap (ClientData *pCD, int bitmapIndex, Pixmap mask)
#endif /* _NO_PROTO */
{
    BitmapCache  *bitmapc;
    PixmapCache  *pixmapc;
    Pixmap        pixmap = (Pixmap)NULL;
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);

    if (bitmapIndex < 0)
    {
	return ((Pixmap)NULL);
    }
    bitmapc = &(pSD->bitmapCache[bitmapIndex]);

    /*
     * Search for an icon pixmap matching the client icon colors.
     */

    pixmapc = bitmapc->pixmapCache;
    while (pixmapc)
    {
        if ((pixmapc->pixmapType == ICON_PIXMAP) &&
	    (pixmapc->foreground == pCD->iconImageForeground) &&
	    (pixmapc->background == pCD->iconImageBackground))
        {
	    pixmap = pixmapc->pixmap;
	    break;
        }
	pixmapc = pixmapc->next;
    }

    /* 
     * If a matching pixmap was not found in the pixmap cache for this bitmap
     *   then create the icon pixmap with the appropriate colors.
     * If have sufficient memory, save the pixmap info in the pixmapCache.
     */

    if (!pixmap &&
        (pixmap = MakeIconPixmap (pCD, bitmapc->bitmap, mask,
				     bitmapc->width, bitmapc->height, 1)) &&
	(pixmapc = (PixmapCache *) XtMalloc (sizeof (PixmapCache))))
    {

        pixmapc->pixmapType = ICON_PIXMAP;
	pixmapc->foreground = pCD->iconImageForeground;
	pixmapc->background = pCD->iconImageBackground;
	pixmapc->pixmap = pixmap;
	pixmapc->next = bitmapc->pixmapCache;
	bitmapc->pixmapCache = pixmapc;
    }

    return (pixmap);

} /* END OF FUNCTION MakeCachedIconPixmap */



/*************************************<->*************************************
 *
 *  MakeIconPixmap (pCD, bitmap, mask, width, height, depth)
 *
 *
 *  Description:
 *  -----------
 *  Convert the bitmap and mask into an icon pixmap.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data (icon colors and tiles)
 *  pWS		- pointer to workspace data
 *  bitmap	- bitmap image to be converted
 *  mask	- bitmap mask, 1 for bits of "bitmap" to be kept
 *  width	- pixel width of bitmap
 *  height	- pixel height of bitmap
 *  depth	- depth of bitmap (pixmap, really)
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN 	- icon pixmap or NULL
 *
 *
 *  Comments:
 *  --------
 *  o "mask" is not used.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
Pixmap MakeIconPixmap (pCD, bitmap, mask, width, height, depth)

    ClientData  *pCD;
    Pixmap       bitmap;
    Pixmap       mask;
    unsigned int width;
    unsigned int height;
    unsigned int depth;

#else /* _NO_PROTO */
Pixmap MakeIconPixmap (ClientData *pCD, Pixmap bitmap, Pixmap mask, unsigned int width, unsigned int height, unsigned int depth)
#endif /* _NO_PROTO */
{
    Pixmap       iconPixmap;
    GC           imageGC, topGC, botGC;
    XGCValues    gcv;
#ifdef WSM
    unsigned long gc_mask;
    PixelSet     *pPS = NULL;
#endif /* WSM */
    unsigned int imageWidth;
    unsigned int imageHeight;
    int          dest_x, dest_y;
#ifndef NO_CLIP_CENTER
    int          src_x, src_y;
#endif /* NO_CLIP_CENTER */
    Pixel        fg;
    Pixel        bg;
    static RList *top_rects = NULL;
    static RList *bot_rects = NULL;
    WmScreenData	*pSD;

    if ((top_rects == NULL) && 
	(top_rects = AllocateRList 
	((unsigned)2 * ICON_INTERNAL_SHADOW_WIDTH)) == NULL)
    {
	/* Out of memory! */
	Warning (((char *)GETMESSAGE(38, 3, "Insufficient memory to bevel icon image")));
	return ((Pixmap)NULL);
    }

    if ((bot_rects == NULL) &&
	(bot_rects = AllocateRList 
	((unsigned)2 * ICON_INTERNAL_SHADOW_WIDTH)) == NULL)
    {
	/* Out of memory! */
	Warning (((char *)GETMESSAGE(38, 4, "Insufficient memory to bevel icon image")));
	return ((Pixmap)NULL);
    }

    if (pCD)
    {
	pSD = pCD->pSD;
    }
    else 
    {
	pSD = wmGD.pActiveSD;
    }

    /* don't make icon pixmap if bitmap is too small */

    if ((width < pSD->iconImageMinimum.width) ||
	(height < pSD->iconImageMinimum.height))
    {
	/* bitmap is too small */
	return ((Pixmap)NULL);
    }
#ifndef NO_CLIP_CENTER
    
    /* copy the center of the icon if too big */
    if (width > pSD->iconImageMaximum.width)
    {
	src_x = (width - pSD->iconImageMaximum.width)/2;
    }
    else
    {
	src_x = 0;
    }
    if (height > pSD->iconImageMaximum.height)
    {
	src_y = (height - pSD->iconImageMaximum.height)/2;
    }
    else
    {
	src_y = 0;
    }
#endif /* NO_CLIP_CENTER */

    /*
     * SLAB frameStyle adds a single pixel of background color around
     * the image to set it off from the beveling.
     */
    imageWidth = pSD->iconImageMaximum.width + 
		  2 * ICON_INTERNAL_SHADOW_WIDTH +
		  ((wmGD.frameStyle == WmSLAB) ? 2 : 0);
    imageHeight = pSD->iconImageMaximum.height +
		  2 * ICON_INTERNAL_SHADOW_WIDTH +
		  ((wmGD.frameStyle == WmSLAB) ? 2 : 0);

    /* create a pixmap (to be returned) */

    iconPixmap = XCreatePixmap (DISPLAY, pSD->rootWindow, 
		     imageWidth, imageHeight,
		     DefaultDepth(DISPLAY, pSD->screen));

    /*
     * If a client is not specified use icon component colors, otherwise
     * use the client-specific icon colors.
     */

    if (pCD)
    {
	bg = pCD->iconImageBackground;
	fg = pCD->iconImageForeground;
    }
    else
    {
	bg = pSD->iconAppearance.background;
	fg = pSD->iconAppearance.foreground;
    }

    /* create a GC to use */
#ifdef WSM
    gc_mask = GCForeground | GCBackground | GCGraphicsExposures;
    if (mask)
    {
	if (pSD->pPrimaryPixelSet != NULL)
	{
	    pPS = pSD->pPrimaryPixelSet;
	    gcv.background = pPS->bg;
	    /* set fg to bg color to clear it first */
	    gcv.foreground = pPS->bg;
	}
	else
	{
	    gcv.background = ICON_APPEARANCE(pCD).background;
	    /* set fg to bg color to clear it first */
	    gcv.foreground = ICON_APPEARANCE(pCD).background;
	}
    } 
    else 
    {
	gcv.foreground = bg;	/* clear it first! */
	gcv.background = bg;
    }
    gcv.graphics_exposures = False;

    imageGC = XCreateGC (DISPLAY, iconPixmap, gc_mask, &gcv);
#else /* WSM */
    gcv.foreground = bg;	/* clear it first! */
    gcv.background = bg;
    gcv.graphics_exposures = False;

    imageGC = XCreateGC (DISPLAY, iconPixmap, (GCForeground|GCBackground),
		  &gcv);
#endif /* WSM */

    /*
     * Format the image. 
     */

    /* fill in background */

    XFillRectangle(DISPLAY, iconPixmap, imageGC, 0, 0, 
		   imageWidth, imageHeight);

    /* center the image */

    if (width > pSD->iconImageMaximum.width)
    {
	width = pSD->iconImageMaximum.width;
    }
    if (height > pSD->iconImageMaximum.height)
    {
	height = pSD->iconImageMaximum.height;
    }
    /* center the image */

    dest_x = (imageWidth - width) / 2;
    dest_y = (imageHeight - height) / 2;

#ifdef WSM
    if (mask)
    {
	if (pPS != NULL)
	{
	    gcv.foreground = pPS->fg;
	}
	else
	{
	    gcv.foreground = ICON_APPEARANCE(pCD).foreground;
	}
    }
    else
    {
	gcv.foreground = fg;
    }
    gc_mask = GCForeground;
    if (mask)
    {
	gcv.clip_mask = mask;
#ifndef NO_CLIP_CENTER
	gcv.clip_x_origin = dest_x - src_x;
	gcv.clip_y_origin = dest_y - src_y;
#else /* NO_CLIP_CENTER */
	gcv.clip_x_origin = dest_x;
	gcv.clip_y_origin = dest_y;
#endif /* NO_CLIP_CENTER */
	gc_mask |= GCClipXOrigin | GCClipYOrigin | GCClipMask;
    } 

    XChangeGC (DISPLAY, imageGC, gc_mask, &gcv);
#else /* WSM */
    /* set the foreground */
    XSetForeground (DISPLAY, imageGC, fg);
#endif /* WSM */

    /* copy the bitmap to the pixmap */
#ifndef DISALLOW_DEEP_ICONS
    if ((depth > 1) &&
        (depth == DefaultDepth(DISPLAY, pSD->screen)))
    {
#ifndef NO_CLIP_CENTER
        XCopyArea (DISPLAY, bitmap, iconPixmap, imageGC, src_x, src_y,
                width, height, dest_x, dest_y);
#else /* NO_CLIP_CENTER */
        XCopyArea (DISPLAY, bitmap, iconPixmap, imageGC, 0, 0,
                width, height, dest_x, dest_y);
#endif /* NO_CLIP_CENTER */
    }
    else
#endif /* DISALLOW_DEEP_ICONS */
#ifndef NO_CLIP_CENTER
    XCopyPlane (DISPLAY, bitmap, iconPixmap, imageGC, src_x, src_y, width, 
		height, dest_x, dest_y, 1L);
#else /* NO_CLIP_CENTER */
    XCopyPlane (DISPLAY, bitmap, iconPixmap, imageGC, 0, 0, width, height, 
		dest_x, dest_y, 1L);
#endif /* NO_CLIP_CENTER */

    /* free resources */
    XFreeGC (DISPLAY, imageGC);

    if (pCD)
    {
	/*
	 * Shadowing
	 */

#ifdef WSM
        if (mask && (pPS != NULL))
	{
	topGC = GetHighlightGC (pSD, pPS->ts, pPS->bg,
				  pCD->iconImageTopShadowPixmap);

	botGC = GetHighlightGC (pSD, pPS->bs, pPS->bg,
				  pCD->iconImageBottomShadowPixmap);
	}
	else
	{
#endif /* WSM */
	topGC = GetHighlightGC (pSD, pCD->iconImageTopShadowColor, 
				  pCD->iconImageBackground,
				  pCD->iconImageTopShadowPixmap);

	botGC = GetHighlightGC (pSD, pCD->iconImageBottomShadowColor, 
				  pCD->iconImageBackground,
				  pCD->iconImageBottomShadowPixmap);
#ifdef WSM
	}
#endif /* WSM */

       /*
        *  CR5208 - Better fix than from OSF!
        *           Zero out the rectangle count in our
        *           static structures so that BevelRectangle
        *           won't extend the RList causing a memory leak.
        *           Old fix allocated and freed rectangle structure
        *           each time through.
        */
	top_rects->used = 0;	/* reset count */
	bot_rects->used = 0;

	BevelRectangle (top_rects, 
			bot_rects, 
			0, 0,
			imageWidth, imageHeight,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH);

	XFillRectangles (DISPLAY, iconPixmap, topGC, top_rects->prect, 
							top_rects->used);
	XFillRectangles (DISPLAY, iconPixmap, botGC, bot_rects->prect,
							bot_rects->used);
    }
    return (iconPixmap);

} /* END OF FUNCTION MakeIconPixmap */



/*************************************<->*************************************
 *
 *  Pixmap
 *  MakeCachedLabelPixmap (pSD, menuW, bitmapIndex)
 *
 *
 *  Description:
 *  -----------
 *  Creates and returns a label pixmap.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  menuW = menu widget (for foreground and background colors)
 *  bitmapIndex = bitmap cache index
 *
 * 
 *  Outputs:
 *  -------
 *  Return = label pixmap or NULL.
 *
 *
 *  Comments:
 *  --------
 *  Assumes bitmapIndex is valid.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Pixmap MakeCachedLabelPixmap (pSD, menuW, bitmapIndex)

    WmScreenData  *pSD;
    Widget        menuW;
    int           bitmapIndex;

#else /* _NO_PROTO */
Pixmap MakeCachedLabelPixmap (WmScreenData *pSD, Widget menuW, int bitmapIndex)
#endif /* _NO_PROTO */
{
    BitmapCache  *bitmapc;
    PixmapCache  *pixmapc;
    int           i;
    Arg           args[5];
    Pixel         fg, bg;
    Pixmap        pixmap = (Pixmap)NULL;
    GC            gc;
    XGCValues     gcv;

    if (bitmapIndex < 0)
    {
	return ((Pixmap)NULL);
    }
    bitmapc = &(pSD->bitmapCache[bitmapIndex]);

    /*
     * Get the foreground and background colors from the menu widget.
     * Search for a label pixmap matching those colors.
     */

    i = 0;
    XtSetArg (args[i], XtNforeground, &fg); i++;
    XtSetArg (args[i], XtNbackground, &bg); i++;
    XtGetValues (menuW, (ArgList)args, i);

    pixmapc = bitmapc->pixmapCache;
    while (pixmapc)
    {
        if ((pixmapc->pixmapType == LABEL_PIXMAP) &&
	    (pixmapc->foreground == fg) &&
	    (pixmapc->background == bg))
        {
	    pixmap = pixmapc->pixmap;
	    break;
        }
	pixmapc = pixmapc->next;
    }

    if (!pixmap)
    /* 
     * A matching pixmap was not found in the pixmap cache for this bitmap.
     * Create and save the label pixmap with appropriate colors.
     */
    {
        /* 
         * Create a pixmap of the appropriate size, root, and depth.
         * Only BadAlloc error possible; BadDrawable and BadValue are avoided.
         */

        pixmap = XCreatePixmap (DISPLAY, pSD->rootWindow, 
				bitmapc->width, bitmapc->height,
                                DefaultDepth (DISPLAY, pSD->screen));

        /*
	 * Create a GC and copy the bitmap to the pixmap.
         * Only BadAlloc and BadDrawable errors are possible; others are avoided
         */

        gcv.foreground = bg;
        gcv.background = bg;
	gcv.graphics_exposures = False;
        gc = XCreateGC(DISPLAY, pixmap, (GCForeground|GCBackground), &gcv);
   
        /*
	 * Fill in the background, set the foreground, copy the bitmap to the 
         * pixmap, and free the gc.
         */

        XFillRectangle (DISPLAY, pixmap, gc, 0, 0,
		        bitmapc->width, bitmapc->height);
        XSetForeground (DISPLAY, gc, fg);
        XCopyPlane (DISPLAY, bitmapc->bitmap, pixmap, gc, 0, 0,
                    bitmapc->width, bitmapc->height, 0, 0, 1L);
        XFreeGC (DISPLAY, gc);

        /*
	 * If have sufficient memory, save the pixmap info in the pixmapCache.
	 */

	if ((pixmapc = (PixmapCache *) XtMalloc(sizeof(PixmapCache))) != NULL)
	{
	    pixmapc->pixmapType = LABEL_PIXMAP;
	    pixmapc->foreground = fg;
	    pixmapc->background = bg;
	    pixmapc->pixmap = pixmap;
	    pixmapc->next = bitmapc->pixmapCache;
	    bitmapc->pixmapCache = pixmapc;
	}
    }

    return (pixmap);

} /* END OF FUNCTION MakeCachedLabelPixmap */



/*************************************<->*************************************
 *
 *  int
 *  GetBitmapIndex (pSD, name)
 *
 *
 *  Description:
 *  -----------
 *  Retrieve bitmap from cache.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  name = bitmap file name or NULL pointer
 *  bitmapCache[]
 *  bitmapCacheSize
 *  bitmapCacheCount
 *
 * 
 *  Outputs:
 *  -------
 *  bitmapCache[]
 *  bitmapCacheSize
 *  bitmapCacheCount
 *  Return   = bitmap cache index or -1
 *
 *
 *  Comments:
 *  --------
 *  None
 * 
 *************************************<->***********************************/

#define BITMAP_CACHE_INC 5

#ifdef WSM
#ifdef _NO_PROTO
int GetBitmapIndex (pSD, name, bReportError)

    WmScreenData *pSD;
    char         *name;
    Boolean	 bReportError;
#else /* _NO_PROTO */
int GetBitmapIndex (WmScreenData *pSD, char *name, Boolean bReportError)
#endif /* _NO_PROTO */
#else /* WSM */
#ifdef _NO_PROTO
int GetBitmapIndex (pSD, name)

    WmScreenData *pSD;
    char         *name;
#else /* _NO_PROTO */
int GetBitmapIndex (WmScreenData *pSD, char *name)
#endif /* _NO_PROTO */
#endif /* WSM */
{
    char         *path;
    BitmapCache  *bitmapc;
    unsigned int  n;
    int           x, y;

    /*
     * Search a nonempty bitmap cache for a pathname match.
     */
    path = BitmapPathName (name);
    for (n = 0, bitmapc = pSD->bitmapCache;
	 n < pSD->bitmapCacheCount;
	 n++, bitmapc++)
    {
        if ((!path && !bitmapc->path) ||
            (path && bitmapc->path && 
	     !strcmp (path, bitmapc->path)))
        {
	    return (n);
	}
    }

    /*
     * The bitmap path name was not found in bitmapCache.
     * Find the next BitmapCache entry, creating or enlarging bitmapCache if 
     * necessary.
     */
    if (pSD->bitmapCacheSize == 0)
    /* create */
    {
        pSD->bitmapCacheSize = BITMAP_CACHE_INC;
        pSD->bitmapCache =
	    (BitmapCache *) XtMalloc (BITMAP_CACHE_INC * sizeof (BitmapCache));
    }
    else if (pSD->bitmapCacheCount == pSD->bitmapCacheSize)
    /* enlarge */
    {
        pSD->bitmapCacheSize += BITMAP_CACHE_INC;
        pSD->bitmapCache = (BitmapCache *) 
	    XtRealloc ((char*)pSD->bitmapCache, 
		     pSD->bitmapCacheSize * sizeof (BitmapCache));
    }

    if (pSD->bitmapCache == NULL)
    {
        MWarning (((char *)GETMESSAGE(38, 5, "Insufficient memory for bitmap %s\n")), name);
	pSD->bitmapCacheSize = 0;
	pSD->bitmapCacheCount = 0;
	return (-1);
    }

    bitmapc = &(pSD->bitmapCache[pSD->bitmapCacheCount]);

    /*
     * Fill the entry with the bitmap info.
     * A NULL path indicates the builtin icon bitmap.
     * Indicate that no pixmapCache exists yet.
     */

    if (path)
    {
        if ((bitmapc->path = (String)
                 XtMalloc ((unsigned int)(strlen (path) + 1))) == NULL)
        {
            MWarning (((char *)GETMESSAGE(38, 6, "Insufficient memory for bitmap %s\n")), name);
	    return (-1);
        }
        strcpy (bitmapc->path, path);

        if (XReadBitmapFile (DISPLAY, pSD->rootWindow, path, 
			     &bitmapc->width, &bitmapc->height, 
			     &bitmapc->bitmap, &x, &y)
            != BitmapSuccess)
        {
#ifdef WSM
	  if (bReportError)
#endif /* WSM */
            MWarning (((char *)GETMESSAGE(38, 7, "Unable to read bitmap file %s\n")), path);
	    XtFree ((char *)bitmapc->path);
	    return (-1);
        }

        if (bitmapc->width == 0 || bitmapc->height == 0)
        {
#ifdef WSM
	  if (bReportError)
#endif /* WSM */
            MWarning (((char *)GETMESSAGE(38, 8, "Invalid bitmap file %s\n")), path);
	    XtFree ((char *)bitmapc->path);
	    return (-1);
        }
    }
    else
    /* builtin icon bitmap */
    {
        bitmapc->path   = NULL;
        bitmapc->bitmap = pSD->builtinIconPixmap;
        bitmapc->width  = iImage_width;
        bitmapc->height = iImage_height;
    }

    bitmapc->pixmapCache = NULL;

    return (pSD->bitmapCacheCount++);

} /* END OF FUNCTION GetBitmapIndex */



/*************************************<->*************************************
 *
 *  BitmapPathName (string)
 *
 *
 *  Description:
 *  -----------
 *  Constructs a bitmap file pathname from the bitmap file name and the
 *  bitmapDirectory resource value.
 *
 *
 *  Inputs:
 *  ------
 *  string = bitmap file name or NULL
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *  XBMLANGPATH
 *  XAPPLRESDIR
 *
 * 
 *  Outputs:
 *  -------
 *  Return = string containing the bitmap file pathname or NULL.
 *
 *
 *  Comments:
 *  --------
 *  If the bitmap file does not exist, searches using XBMLANGPATH.
 *  Returns NULL path name for a NULL file name.
 * 
 *************************************<->***********************************/

char *BitmapPathName (string)
    char *string;

{
    static char  fileName[MAXPATH+1];
    char *retname;
    SubstitutionRec subs[1];
#ifndef MOTIF_ONE_DOT_ONE
    char *homeDir = _XmOSGetHomeDirName();
#endif

    if (!string || !*string)
    {
	return (NULL);
    }

    /*
     * Interpret "~/.." as relative to the user's home directory.
     * Interpret "/.." as an absolute pathname.
     * If the bitmapDirectory resource is nonNULL, interpret path as relative
     *   to it.
     * Else, or if bitmapDirectory has no such file, use a XBMLANGPATH lookup.
     */

    if ((string[0] == '~') && (string[1] == '/'))
    /* 
     * Handle "~/.." 
     */
    {
#ifdef MOTIF_ONE_DOT_ONE
	GetHomeDirName(fileName);
#else
	strcpy (fileName, homeDir);
#endif
        strncat (fileName, &(string[1]), MAXPATH - strlen (fileName));
	return (fileName);
    }

    if (string[0] == '/')
    {
      return(string);
    }

    if (wmGD.bitmapDirectory && *wmGD.bitmapDirectory)
    /*
     * Relative to nonNULL bitmapDirectory (which may have relative to HOME)
     */
    {
	if ((wmGD.bitmapDirectory[0] == '~') &&
	    (wmGD.bitmapDirectory[1] == '/'))
	{
#ifdef MOTIF_ONE_DOT_ONE
	    GetHomeDirName(fileName);
#else
	    strcpy (fileName, homeDir);
#endif
            strncat (fileName, &wmGD.bitmapDirectory[1],
		     MAXPATH - strlen (fileName));
	} else {
	    strcpy (fileName, wmGD.bitmapDirectory);
	}
        strncat (fileName, "/", MAXPATH - strlen (fileName));
        strncat (fileName, string, MAXPATH - strlen (fileName));

/* Test file for existence. */

	subs[0].substitution = "";
	if ((retname = XtFindFile(fileName, subs, 0,
				  (XtFilePredicate) NULL)) != NULL) {
	  XtFree(retname);
	  return (fileName);
	}
    }

    /* Fall back on a path search */

#ifdef MOTIF_ONE_DOT_ONE
    return (NULL);
#else
    {
	char *search_path;
	Boolean user_path;

	search_path = _XmOSInitPath(string, MATCH_PATH, &user_path);
	subs[0].match = user_path ? MATCH_XBM : MATCH_CHAR;
	subs[0].substitution = string;
	retname = XtResolvePathname(DISPLAY, "bitmaps", NULL, NULL,
				    search_path, subs, XtNumber(subs), 
					(XtFilePredicate)NULL);
	XtFree(search_path);

	if (!retname)
	  return (string);

	strncpy(fileName, retname, MAXPATH);
	XtFree(retname);
	return (fileName);
    }
#endif

} /* END OF FUNCTION BitmapPathName */

#ifdef WSM
/****************************   eof    ***************************/
#endif /* WSM */
