/**---------------------------------------------------------------------
***	
***    (c)Copyright 1991 Hewlett-Packard Co.
***    
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***
***-------------------------------------------------------------------*/

/*
    Image Library Utility (ILU) package - a package to (hopefully) simplify, 
    and serve as an example of, the use of the Image Library (IL).
*/

#include "ilu.h"
#include "ilpipelem.h"

#include <X11/Xutil.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

    /*  Private definition of what is in an iluControl. */
typedef struct {
    ilContext           context;
    ilPipe              pipe, maskPipe;
    Display            *dstDisplay;
    Window              dstWindow;
    GC                  dstGC;
    Visual             *dstVisual;
    Colormap            dstColormap;
    ilInternalImage     maskImage;
    Pixmap              maskBitmap;
    GC                  maskGC;
    ilXWC               maskXWC;
    ilXWC               XWC;
    ilFile              dstFile;
    unsigned long       scaleThreshold, convertThreshold;
    ilShortBool         clearWindow;
    ilInternalImage     readImage;
    ilInternalImage     writeImage;
    } iluControlRec, *iluControlPtr;


    /* ======================= MODULARIZED FUNCTIONS ===================== */

    /*  Functions in this section are "modularized" in that they use no "ilu"
        types or global variables, so that they can be copied into other code
        and used there.
    */


    /*  ---------------------- iluGetWindowInfo ------------------------- */
    /*  Given a window id, return it's visual and colormap, or if error:
        null if depth == 1, else default visual / colormap.
    */
static void iluGetWindowInfo (
    Display        *display,
    Window          window,
    Visual        **pVisual,                /* RETURNED */
    Colormap       *pColormap               /* RETURNED */
    )
{
    XWindowAttributes atts;

    if (!XGetWindowAttributes (display, window, &atts)) {
        if (atts.depth == 1) {
            *pVisual = (Visual *)NULL;
            *pColormap = (Colormap)0;
            }
        else {
            *pVisual = DefaultVisual (display, DefaultScreen (display));
            *pColormap = DefaultColormap (display, DefaultScreen (display));
            }
        }
    else {
        *pVisual = atts.visual;
        *pColormap = atts.colormap;
        }
}

    /*  ------------------------ iluGamma ------------------------------- */
    /*  Add an ilMap() filter to do gamma correction, only if the image
        is a continuous-tone image.
    */
static ilError iluGamma (
    ilPipe          pipe,
    double          gamma
    )
{
    ilImageDes      des, *pDes;
    int             nSamples, sample, mapValue, i;
    ilImageInfo     imageInfo, *pImageInfo;
    ilPtr           pByte;
    ilClientImage   gammaImage;

        /*  Query pipe; exit if not gray or rgb image.
            Set des/format for image: basically same as image type.
            If a palette image convert it to RGB.  We could gamma the actual
            palette but that is more work; requires a pipe element to be added.
        */
    ilQueryPipe (pipe, (long *)NULL, (long *)NULL, &des);
    switch (des.type) {
      case IL_GRAY:
        nSamples = 1;
        imageInfo.pDes = IL_DES_GRAY;
        imageInfo.pFormat = IL_FORMAT_BYTE;
        break;
      case IL_PALETTE:
      case IL_RGB:
        nSamples = 3;
        imageInfo.pDes = IL_DES_RGB;
        imageInfo.pFormat = IL_FORMAT_3BYTE_PIXEL;  /* RGB bytes together */
        break;
      case IL_YCBCR:
        nSamples = 3;
        imageInfo.pDes = IL_DES_YCBCR;
        imageInfo.pFormat = IL_FORMAT_3BYTE_PIXEL;  /* YCbCr bytes together */
        break;
      default:
        return ILU_OK;              /* no error, add no gamma filter */
        }

        /*  Do an ilConvert() to the above des.  This is done to make
            sure the # of levels of the image is 256.  If this were not done,
            and for example a 4 bit gray scale image were gamma'd, then the
            below ilMap() would convert the 4 bit image to 8 bit, but would
            leave the number of levels as 16, instead of converting to 256.
            (ilMap() converts the format only, not any descriptor qualities.)
                Note that we could also just make the code below use 
            des.nLevelsPerSample[] - 1, instead of 255, but might have to 
            handle different # of levels for each component in the case of RGB.
        */
    if (!ilConvert (pipe, imageInfo.pDes, (ilImageFormat *)NULL, 0, NULL))
        return pipe->context->error;

        /*  Create a 256 by 1 client image, point pByte to first pixel.  */
    imageInfo.width = 256;
    imageInfo.height = 1;
    imageInfo.clientPixels = FALSE;
    gammaImage = ilCreateClientImage (pipe->context, &imageInfo, 0);
    if (!gammaImage)
        return pipe->context->error;

    ilQueryClientImage (gammaImage, &pImageInfo, 0);
    pByte = pImageInfo->plane[0].pPixels;

        /*  Load the client image with pixels based on the gamma factor.
            The client image will be used as a lookup table.  A gamma of 1.0
            will result in no change to the image; i.e. the value of pixel "i"
            will = i.  Gammas > 1.0 will lighten the image - the value of pixel
            "i" will be > i.  In every case pixel 0 = 0 and 255 = 255.
        */
    for (i = 0; i < 256; i++) {
        if (i == 0) 
            mapValue = 0;
        else if (i == 255)
            mapValue = 255;
        else {
            mapValue = 255.0 * exp (log ((double)i / 255.0) / gamma) + 0.5;
            if (mapValue < 0)
                mapValue = 0;
            else if (mapValue > 255)
                mapValue = 255;
            }

            /*  Now apply the same gamma remap for all samples (components), _except_
                for YCbCr, where we only want to remap Y (plane 0).  The Cb and Cr
                components remain the same, i.e. we want the value for entry i = i.
            */
        if (des.type == IL_YCBCR) {
            *pByte++ = mapValue;    /* Y */
            *pByte++ = i;           /* Cb */
            *pByte++ = i;           /* Cr */
            }
        else for (sample = 0; sample < nSamples; sample++)
            *pByte++ = mapValue;
        }

        /*  Add an ilMap() filter to do the requested gamma and destroy gammaImage.
            This is ok to do because the ilMap() image is reference counted and
            will not really be destroyed until "pipe" is.
        */
    ilMap (pipe, gammaImage);
    ilDestroyObject (gammaImage);
    return pipe->context->error;
}


    /*  ------------------------ iluScale ------------------------------- */
    /*  Add a scale filter to the given pipe to scale the image to "width"
        by "height".  "method" is one of:
            IL_SCALE_BITONAL_TO_GRAY - used only if pipe image is bitonal
        and image is being scaled down (made smaller): otherwise same as
        IL_SCALE_SAMPLE.  The bitonal pipe image is scaled down to result
        in a 256-level gray image.  This option is generally used to display
        high-resolution (e.g. FAX) bitonal images, and results in better
        readability.  It should be used only if the target display can do
        gray display; use ilChangeXWC (XWC, IL_XWC_ALLOC_GRAYS) to determine
        if gray display supported given a XWC.
            IL_SCALE_SAMPLE - if image is being scaled up (enlarged), currently
        the same as IL_SCALE_SIMPLE.  If scaled down, pixels are formed by
        averaging source pixels.  If the pipe image is an IL_PALETTE image it
        is converted to IL_RGB before scaling, since palette pixels cannot
        be averaged.  If this option is used and the pipe image is bitonal,
        then "pBiThreshold" must point to a persistent value (malloc'd or
        static - not a local variable) that is used as the threshold. Logically
        the image is scaled to gray as in IL_SCALE_BITONAL_TO_GRAY, but then
        the resulting gray pixel (0..255) is compared to the threshold value.
        If greater, a white pixel results; if less, black results.  The logical
        value for the threshold would be 128, but in fact a value of around 190
        works better for scanned text documents.  The higher the threshold, 
        the "darker" the resulting image.
            IL_SCALE_SIMPLE - sometimes called "nearest neighbor" or 
        "decimation": each dst pixel is copied from a src pixel.  This is
        the fastest but poorest quality.  The image type is not changed, e.g.
        IL_PALETTE images remain IL_PALETTE.
    */
static iluError iluScale (
    ilPipe          pipe,
    long            width,
    long            height,
    int             method,
    unsigned long  *pBiThreshold
    )
{
    unsigned long  *pOptData;
    long            pipeWidth, pipeHeight;
    ilImageDes      des;

        /*  For bitonal images, scaling down only (a scale up in either
            dimension currently causes the IL to use simple scaling):
                if scale to gray use IL_SCALE_BITONAL_TO_GRAY, else:
                    if sample scaling, point pOptData to a threshold.  A scale
                    to a 0..255 gray value is done for each pixel, then if
                    the gray pixel is >= threshold, white is output, else black.
            For non-bitonal, use simple or sample depending on request.
            Note that when a IL_SCALE_SAMPLE is done on a palette image,
            the image is first converted to RGB so sampling can be done.
        */
    pOptData = (unsigned long *)NULL;

    ilQueryPipe (pipe, &pipeWidth, &pipeHeight, &des);
    if (method == IL_SCALE_BITONAL_TO_GRAY)
        if ((des.type != IL_BITONAL) 
         || (pipeWidth < width) 
         || (pipeHeight < height))
            method = IL_SCALE_SAMPLE;

    if ((des.type == IL_BITONAL) && (method == IL_SCALE_SAMPLE))
        pOptData = pBiThreshold;

    ilScale (pipe, width, height, method, pOptData);
    return pipe->context->error;
}



    /*  -------------------- iluCreateWindow ------------------------------- */
    /*  Create and return the X visual, colormap and window for image display.
        The window is created at location "windowX/Y"; usually ignored unless
        the window is created without window manager knowing ("noWM" == TRUE).
        "width" and "height" is the size of the window to create, e.g. the
        size of the current pipe imgae.
        If there is an available True/DirectColor 24 bit visual it is used
        unless "useDefaultVisual" is true.  If "usePrivateColormap", a colormap
        is created even if DefaultVisual being used.
    */
static iluError iluCreateWindow (
    Display        *display,
    int             windowX,
    int             windowY,
    long            width,
    long            height,
    ilBool          useDefaultVisual,
    ilBool          usePrivateColormap,
    ilBool          noWM,
    Visual        **pDstVisual,
    Colormap       *pDstColormap,
    Window         *pDstWindow
    )
{
    XSetWindowAttributes atts;
    XVisualInfo         vinfo;
    unsigned long       white;
    int                 screen;
    Window              root;
    ilBool              createColormap;
    int                 depth;

    screen = DefaultScreen (display);
    root = DefaultRootWindow (display);

        /*  If enabled try to find a 24 bit visual that the IL will use.
            Opening a window in the 24 bit True/DirectColor visual is
            surprisingly difficult!  Must create a colormap, then specify it 
            and the border and back pixels when creating the window.  If we 
            don't do this (e.g. use XCreateSimpleWindow) we'll get a BadMatch 
            X error, because X will try to use the colormap and border/back 
            pixels of the parent (root) window - which is probably 8 bit; 
            hence the BadMatch.
                The check of vinfo.red/green/blue_mask is to make sure the IL
            will handle this 24 bit visual as such.  If the packing of RGB
            into a long for XPutImage() is not <unused><R><G><B>, each a byte,
            then the IL does not handle the visual as 24 bits, but instead
            converts the image to bitonal and displays it as such.  This is the
            same thing the IL would do with any visual it does not handle,
            e.g. a 4 bit PseudoColor visual.
                Of course, in the future the IL may handle such visuals ...
        */
    if (!useDefaultVisual
     && (XMatchVisualInfo (display, screen, 24, TrueColor, &vinfo)
         || XMatchVisualInfo (display, screen, 24, DirectColor, &vinfo)) 
     && (vinfo.red_mask == 0xff0000)
     && (vinfo.green_mask == 0xff00)
     && (vinfo.blue_mask == 0xff) ) {
        *pDstVisual = vinfo.visual;
        depth = 24;
        createColormap = TRUE;
        }
    else {
        *pDstVisual = DefaultVisual (display, screen);
        depth = 0;          /* meaning copy from parent, the root window */
        createColormap = usePrivateColormap;
        }

        /*  Create a colormap if 24 bit visual or private requested */
    if (createColormap) {
        *pDstColormap = XCreateColormap (display, root, *pDstVisual, 
                                           AllocNone);
        if (!*pDstColormap)
            return ILU_ERROR_WRITE_WINDOW;
        }
    else *pDstColormap = DefaultColormap (display, screen);

        /*  Create the window in visual with new or default colormap */
    atts.colormap = *pDstColormap;
    atts.background_pixel = atts.border_pixel = 0;      /* black? */
    *pDstWindow = XCreateWindow (display, root,
           windowX, windowY, width, height, 0, depth, 
           InputOutput, *pDstVisual, 
           CWColormap | CWBorderPixel | CWBackPixel, &atts);
    if (!*pDstWindow)
        return ILU_ERROR_WRITE_WINDOW;

        /*  Turn on backing store on the new window */
    atts.backing_store = Always;
    XChangeWindowAttributes (display, *pDstWindow, CWBackingStore, &atts);

        /*  Disable window manager if requested */
    if (noWM) {
        atts.override_redirect = TRUE;
        XChangeWindowAttributes (display, *pDstWindow, CWOverrideRedirect, 
                                 &atts);
        }

        /*  Map the window - should wait for an input event here, but
            currently this package does no X input handling.  Hopefully backing
            store will be honored and no output lost.
        */
    XMapRaised (display, *pDstWindow);
    XFlush (display);

    return IL_OK;
}


    /*  -------------------- iluAllocInit/Execute ------------------------- */
    /*  Pipe element Init() and Execute() functions, added to pipe by 
        iluAllocColors() to set the X colormap values to colors in the palette.
    */

    /*  Private for pipe element added by iluAllocColors() */
typedef struct {
    Display        *display;
    Colormap        colormap;
    ilBool          firstStrip;         /* TRUE iff first strip this pass */
    int             nAlloc;             /* # of colors to set */
    unsigned long   pixels[256];        /* X pixel values for IL colors */
    } iluAllocPrivRec, *iluAllocPrivPtr;

static ilError iluAllocInit (
    iluAllocPrivPtr     pPriv,
    ilImageInfo        *pSrcImage,
    ilImageInfo        *pDstImage
    )
{
    pPriv->firstStrip = TRUE;
    return IL_OK;
}

static ilError iluAllocExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
    iluAllocPrivPtr         pPriv;
    unsigned short         *pPalette;
    int                     i;
    XColor                  colors[256], *pColor;

        /*  If first strip, read the palette and set the X color cells to
            the colors in the palette.
        */
    pPriv = (iluAllocPrivPtr)pData->pPrivate;
    if (pPriv->firstStrip) {
        pPriv->firstStrip = FALSE;
        pPalette = pData->pSrcImage->pPalette;
        for (i = 0, pColor = colors; i < pPriv->nAlloc; i++, pColor++) {
            pColor->pixel = pPriv->pixels[i];
            pColor->red   = pPalette [i + 0];
            pColor->green = pPalette [i + 256];
            pColor->blue  = pPalette [i + 512];
            pColor->flags = DoRed | DoGreen | DoBlue;
            pColor->pad = 0;
            }
        XStoreColors (pPriv->display, pPriv->colormap, colors, pPriv->nAlloc);
        }

        /*  Don't have to copy pixel to next filter because this is a "no dst"
            filter.  However, must set next filter's srcLine to ours.
        */
    *pData->pNextSrcLine = pData->srcLine;
    return IL_OK;
}

    /*  -------------------- iluAllocColors ------------------------------- */
    /*  For GrayScale and PseudoColor visuals only: allocate all available
        writeable colors, then use them to display the pipe image.
    */
static ilBool iluAllocColors (
    ilPipe          pipe,
    Display        *display,
    Visual         *visual,
    Colormap        colormap,
    ilXWC           XWC
    )
{
    ilImageDes      des;
    int             nitems, nAlloc, i;
    XVisualInfo    *pInfo;
    ilImageInfo     imageInfo, *pImageInfo;
    ilClientImage   mapImage;
    ilPtr           pMapPixels, pByte;
    unsigned long   pixels[256], planes;
    XVisualInfo     vinfo;
    iluAllocPrivPtr pPriv;
    ilBool          rawMode;
    XColor          colors[256], *pColor;
    ilConvertToPaletteInfo cvt;

        /*  Find current pipe image type, return if bitonal */
    ilQueryPipe (pipe, (long *)NULL, (long *)NULL, &des);
    if (des.type == IL_BITONAL)
        return TRUE;                /* no colors alloc'd for bitonal */

        /*  Get visual info; only handle depth 8 visuals; return if not */
    vinfo.visualid = visual->visualid;
    pInfo = XGetVisualInfo (display, VisualIDMask, &vinfo, &nitems);
    if (pInfo->depth != 8)
        return TRUE;

        /*  Check class of visual: gray => convert image to gray;
            PseudoColor => support it, else exit, no special support.
        */
    switch (pInfo->class) {
      case GrayScale:
        des = *IL_DES_GRAY;
        if (!ilConvert (pipe, &des, IL_FORMAT_BYTE, 0, NULL))
            return FALSE;
        break;
      case PseudoColor:
        break;
      default:
        return TRUE;
        }

        /*  Allocate a map image to do an ilMap() with after any elements
            added below.  A map image is used as a lookup table, e.g. a pipe
            image pixel with a value of 0 results in whatever is in pixel 0
            in the map image being sent down the pipe.  By loading the map
            image with the value of the X pixel that corresponds to that
            IL pixel the ilMap() will map pipe image pixels to X pixels.
        */
    imageInfo.pDes = IL_DES_GRAY;
    imageInfo.pFormat = IL_FORMAT_BYTE;
    imageInfo.width = 256;
    imageInfo.height = 1;
    imageInfo.clientPixels = FALSE;
    mapImage = ilCreateClientImage (pipe->context, &imageInfo, 0);
    if (!mapImage)
        return FALSE;
    ilQueryClientImage (mapImage, &pImageInfo, 0);
    pMapPixels = pImageInfo->plane[0].pPixels;
    bzero (pMapPixels, 256);

        /*  Allocate as many *writeable* color cells as allowed, up to 256. */
    for (nAlloc = 0; nAlloc < 256; nAlloc++) {
        if (!XAllocColorCells (display, colormap, FALSE, &planes, 0, 
                                &pixels[nAlloc], 1))
            break;
        }
    if (nAlloc < 1)
        return TRUE;                    /* too few colors; give up */

        /*  For gray image and/or visual (image cvtd to gray above): remap 
            256 grays to nAlloc X pixel values; load nAlloc color cells with 
            a gray ramp.
        */
    if (des.type == IL_GRAY) {
        for (i = 0; i < 256; i++)
            *pMapPixels++ = pixels[(i * nAlloc) / 256];
        for (i = 0, pColor = colors; i < nAlloc; i++, pColor++) {
            pColor->pixel = pixels[i];
            pColor->red = pColor->green = pColor->blue = 
                i * 65535 / nAlloc;
            pColor->flags = DoRed | DoGreen | DoBlue;
            pColor->pad = 0;
            }
        XStoreColors (display, colormap, colors, nAlloc);
        }
    else {
            /*  Color image and PseudoColor visual: convert to palette image, 
                choosing nAlloc colors.  The colors won't be known until the 
                pipe is run, so must insert a pipe element that sets the colors 
                after they are known.
                    Note that if image is a palette image, we could allocate
                the colors of the image, but that would be more work and we
                don't know if we will get all of the colors.  This way we will
                always get as many colors as possible and fit to them.
            */
        for (i = 0; i < nAlloc; i++)
            *pMapPixels++ = pixels[i];      /* map to X pixels */
        cvt.method = IL_CHOOSE_COLORS;      /* cvt to nAlloc color palette */
        cvt.levels[0] = nAlloc;
        cvt.dstType = IL_PALETTE;
        cvt.mapImage = (ilClientImage)NULL;
        if (!ilConvert (pipe, IL_DES_PALETTE, IL_FORMAT_BYTE, 
                        IL_CONVERT_TO_PALETTE, &cvt))
            return FALSE;
        pPriv = (iluAllocPrivPtr)ilAddPipeElement (pipe, IL_FILTER, 
            sizeof (iluAllocPrivRec), IL_ADD_PIPE_NO_DST, 
            (ilSrcElementData *)NULL, (ilDstElementData *)NULL, 
            iluAllocInit, IL_NPF, IL_NPF, iluAllocExecute, 0);
        if (!pPriv)
            return FALSE;
        pPriv->display = display;
        pPriv->colormap = colormap;
        pPriv->nAlloc = nAlloc;
        pPriv->display = display;
        for (i = 0; i < nAlloc; i++)
            pPriv->pixels[i] = pixels[i];
        }

        /*  Remap pipe image pixels to X pixels, and destroy mapImage.
            This is ok to do because mapImage is reference counted and
            will not really be destroyed until "pipe" is.
        */
    if (!ilMap (pipe, mapImage))
        return FALSE;
    ilDestroyObject (mapImage);

        /*  Set "raw mode" into XWC, so pixels are written w/o translation */
    rawMode = TRUE;
    return ilChangeXWC (XWC, IL_XWC_SET_RAW_MODE, &rawMode);
}




    /* ================= IL PRODUCERS (READS)  ========================== */

    /*  -------------------- iluReadEFS ------------------------------- */
    /*  Add a producer to read a EFS file.
    */
static iluError iluReadEFS (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{

        /*  Add an element to read the next page from the EFS file. */
    if (ilEFSReadImage (pC->pipe, pR->read.rw.EFSFile, IL_EFS_READ_MAIN, 0, 0, NULL))
        if (pR->read.rw.doCrop) 
            ilCrop (pC->pipe, &pR->read.rw.cropRect);
    return pC->context->error;
}


    /*  -------------------- iluReadX ------------------------------- */
    /*  Add a producer to read from an X window.
    */
static iluError iluReadX (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    iluError            error;
    Display            *display;
    int                 screen;
    ilBool              copyToPixmap;
    Colormap            colormap;
    Visual             *visual;
    int                 depth;
    Window              window;

    display = pR->read.rw.display;
    screen = DefaultScreen (display);

        /*  Get read window id: root or given window/pixmap id (not create).
            Do a copy to pixmap if not the root window
        */
    switch (pR->read.rw.windowUsage) {
      case ILU_WINDOW_ROOT:
        window = DefaultRootWindow (display);
        copyToPixmap = FALSE;
        break;
      case ILU_WINDOW_ID:
        window = pR->read.rw.windowID;
        copyToPixmap = TRUE;
        break;
      case ILU_WINDOW_CREATE:
      default:
        return ILU_ERROR_READ_WINDOW_USAGE;
        }

        /*  Read (cropRect piece) from the src drawable, using the visual
            and colormap for the source window.  If the drawable is not a
            window (e.g. is a pixmap): visual and colormap will be null;
            if pixmap depth is not 1 assume default visual/colormap.
        */
    iluGetWindowInfo (display, window, &visual, &colormap);
    ilReadXDrawable (pC->pipe, display, window, visual, colormap,
        FALSE, (pR->read.rw.doCrop) ? &pR->read.rw.cropRect : (ilRect *)NULL,
        copyToPixmap, 0);

    return pC->context->error;
}


    /* ================= IL CONSUMERS (WRITES) ========================== */

    /*  -------------------- iluWriteEFS ------------------------------- */
    /*  Add a consumer to write an EFS file.
    */
static iluError iluWriteEFS (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
        /*  Do a crop if output crop requested. */
    if (pR->write.rw.doCrop)
        if (!ilCrop (pC->pipe, &pR->write.rw.cropRect))
            return pC->context->error;

        /*  Add an element to write a page to the EFS file. */
    ilEFSWriteImage (pC->pipe, pR->write.rw.EFSFile, pR->write.xRes, 
                     pR->write.yRes, pC->maskImage, NULL);
    return pC->context->error;
}


    /*  -------------------- iluWriteTIFF ------------------------------- */
    /*  Add a consumer to write a TIFF file. 
    */
static iluError iluWriteTIFF (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilFileImage         fileImage;

        /*  Do a crop if output crop requested. */
    if (pR->write.rw.doCrop)
        if (!ilCrop (pC->pipe, &pR->write.rw.cropRect))
            return pC->context->error;

        /*  Init output file (write a TIFF header to it) if not appending. */
    if (!pR->write.doAppend)
        if (!ilInitFile (pR->context, pR->write.rw.stream, 0, TRUE, 0))
            return pC->context->error;

        /*  Connect to the file and write one image. */
    pC->dstFile = ilConnectFile (pC->context, pR->write.rw.stream, 0, 0);
    if (!pC->dstFile)
        return pC->context->error;

    ilWriteFileImage (pC->pipe, pC->dstFile, IL_WRITE_MAIN, (ilFileImage)NULL,
            pR->write.compression, pR->write.pCompData, pR->write.stripHeight,
            pR->write.xRes, pR->write.yRes, pR->write.page, pR->write.nPages,
            0, (ilFileTag *)NULL, 0);
    return pC->context->error;
}


    /*  -------------------- iluWriteX ------------------------------- */
    /*  Add a consumer to write an X window.
    */
static iluError iluWriteX (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    Display            *display;
    int                 screen;
    iluError            error;
    unsigned long       black, white;
    int                 depth;
    long                width, height;

        /*  Do a crop if output crop requested. */
    if (pR->write.rw.doCrop)
        if (!ilCrop (pC->pipe, &pR->write.rw.cropRect))
            return pC->context->error;

        /*  Get output window: use root, use given id or create a window */
    display = pR->write.rw.display;
    screen = DefaultScreen (display);

    switch (pR->write.rw.windowUsage) {
      case ILU_WINDOW_ROOT:
        pC->dstWindow = DefaultRootWindow (display);
        iluGetWindowInfo (pC->dstDisplay, pC->dstWindow, 
                         &pC->dstVisual, &pC->dstColormap);
        break;
      case ILU_WINDOW_ID:
        pC->dstWindow = pR->write.rw.windowID;
        iluGetWindowInfo (pC->dstDisplay, pC->dstWindow, 
                         &pC->dstVisual, &pC->dstColormap);
        break;
      case ILU_WINDOW_CREATE:
            /*  Query the pipe; create a window the size of the pipe image. */
        ilQueryPipe (pC->pipe, &width, &height, (ilImageDes *)NULL);
        if (error = iluCreateWindow (pR->write.rw.display, 
            pR->write.windowX, pR->write.windowY, width, height,
            pR->write.useDefaultVisual, pR->write.usePrivateColormap,
            pR->write.noWM, &pC->dstVisual, &pC->dstColormap, &pC->dstWindow))
           return error;
        break;
      default:
        return ILU_ERROR_WRITE_WINDOW_USAGE;
        }

        /*  Create a GC compatible with the dst window, and create an XWC. */
    pC->dstGC = XCreateGC (display, pC->dstWindow, 0, (XGCValues *)NULL);
    if (!pC->dstGC) 
        return ILU_ERROR_WRITE_GC;
    if (!(pC->XWC = ilCreateXWC (pC->context, display, pC->dstVisual,
                                 pC->dstColormap, pC->dstGC, 0, 0)))
        return pC->context->error;

        /*  Set fore/background to black/white: 1/0 if bitmap, defaults if default visual,
            else alloc.  NOTE: doing this after creating the XWC is an optimization
            for Direct/TrueColor visuals, since ilCreateXWC() will allocate a smooth ramp
            and the pixel will be so ordered, so that no indirection (ilMap()) will be
            required when displaying 24 bit images, since RGB values will be 0..255.
        */
    if (!pC->dstVisual) {   /* rendering to bitmap: draw with 1s on 0s */
        black = 1;                 
        white = 0;
        }
    else if (pC->dstVisual == DefaultVisual (display, DefaultScreen (display))) {
        black = BlackPixel (display, screen);
        white = WhitePixel (display, screen);
        }
    else {
            /*  Must allocate; first cause the IL to allocate its colors so that
                a smooth ramp and the pixel will be so ordered, so that no indirection 
                (ilMap()) will be required since RGB values will be ramped 0..255.
            */
        XColor      color;
        ilChangeXWC (pC->XWC, IL_XWC_ALLOC_COLORS, NULL);
        color.flags = DoRed | DoGreen | DoBlue;
        color.red = color.green = color.blue = 0;
        if (XAllocColor (pC->dstDisplay, pC->dstColormap, &color))
             black = color.pixel;
        else black = 0;
        color.red = color.green = color.blue = ~0;
        if (XAllocColor (pC->dstDisplay, pC->dstColormap, &color))
             white = color.pixel;
        else white = ~0;
        }
    XSetForeground (display, pC->dstGC, black);
    XSetBackground (display, pC->dstGC, white);

        /*  If a mask bitmap, set it as clip mask into the GC, and set its
            origin to dstX,Y (top-left position where image will be displayed):
            the clip origin relates to display position, not to mask image.
        */
    if (pC->maskBitmap) {
        XSetClipMask (display, pC->dstGC, pC->maskBitmap);
        XSetClipOrigin (display, pC->dstGC, pR->write.dstX, pR->write.dstY);
        }

        /*  Change dither method if requested */
    if (pR->write.ditherMethod >= 0)
        if (!ilChangeXWC (pC->XWC, IL_XWC_SET_DITHER_METHOD, 
                                  (ilPtr)&pR->write.ditherMethod))
            return pC->context->error;

        /*  If enabled, allocate colors specific to the image */
    if (pR->write.useAllColors)
        if (!iluAllocColors (pC->pipe, display, pC->dstVisual, 
                             pC->dstColormap, pC->XWC))
            return pC->context->error;

        /*  Write image to the window at requested offset */
    ilWriteXDrawable (pC->pipe, pC->dstWindow, pC->XWC, (ilRect *)NULL, 
                      pR->write.dstX, pR->write.dstY, 0);
    return pC->context->error;
}

    /*  ---------------------- iluWriteNull ------------------------------ */
    /*  Add a "null" consumer - write to the bit bucket (aka /dev/null).
    */
static ilError iluNullExecute () 
{
    return IL_OK;
}

static iluError iluWriteNull (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilAddPipeElement (pC->pipe, IL_CONSUMER, 0, 0, (ilSrcElementData *)NULL, 
        (ilDstElementData *)NULL, IL_NPF, IL_NPF, IL_NPF, iluNullExecute, 0);
    return pC->context->error;
}


    /*  ---------------------- iluWriteImage ------------------------------ */
    /*  Create an internal image compatible with the pipe image and write to it.
    */
static iluError iluWriteImage (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
long                width, height;
ilImageDes          des;

    ilQueryPipe (pC->pipe, &width, &height, &des);
    pC->writeImage = ilCreateInternalImage (pC->context, width, height, &des, 0);
    if (!pC->writeImage)
        return pC->context->error;
    ilWriteImage (pC->pipe, pC->writeImage);
    return pC->context->error;
}


    /* ======================== IL FILTERS =============================== */

    /*  ------------------------ iluCrop ------------------------------- */
    /*  Add a Crop filter to the pipe.
    */
static iluError iluCrop (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilCrop (pC->pipe, &pR->crop.rect);
    return pC->context->error;
}


    /*  ------------------------ iluConvert ------------------------------- */
    /*  Add a convert filter to the pipe.
    */
static iluError iluConvert (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    void           *pOptData;
    ilImageDes     *pDes, des;

    pOptData = (unsigned long *)NULL;
    switch (pR->convert.type) {
      case IL_BITONAL:
        pDes = IL_DES_BITONAL;
        break;
      case IL_GRAY:
        pDes = IL_DES_GRAY;
        break;
      case IL_RGB:
        pDes = IL_DES_RGB;
        break;
      case IL_PALETTE:
        pDes = IL_DES_PALETTE;
        break;
      case IL_YCBCR:
        des = *IL_DES_YCBCR;
        des.typeInfo.YCbCr.sample[1].subsampleHoriz = 
            des.typeInfo.YCbCr.sample[1].subsampleVert = 
            des.typeInfo.YCbCr.sample[2].subsampleHoriz = 
            des.typeInfo.YCbCr.sample[2].subsampleVert = pR->convert.subsample;
        pDes = &des;
        break;
      default:
        return ILU_ERROR_CONVERT_TYPE;
        }

        /*  Point to option data based on option */
    switch (pR->convert.option) {
      case 0:
        pOptData = NULL;
        break;
      case IL_CONVERT_THRESHOLD:
        pC->convertThreshold = pR->convert.biThreshold;
        pOptData = &pC->convertThreshold;
        break;
      case IL_CONVERT_TO_PALETTE:
        pOptData = &pR->convert.palette;
        break;
        }

    ilConvert (pC->pipe, pDes, (ilImageFormat *)NULL, 
               pR->convert.option, pOptData);
    return pC->context->error;
}


    /*  ------------------------ iluMirror ------------------------------- */
    /*  Add a mirror filter to the pipe.
    */
static iluError iluMirror (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilMirror (pC->pipe, pR->mirror.direction);
    return pC->context->error;
}


    /*  ------------------------ iluRotate90 ------------------------------- */
    /*  Add a rotate90 filter to the pipe.
    */
static iluError iluRotate90 (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilRotate90 (pC->pipe, pR->rotate90.factor);
    return pC->context->error;
}


    /*  ------------------------ iluInvert ------------------------------- */
    /*  Invert the image if bitonal or gray.
    */
static iluError iluInvert (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilImageDes      des;
    unsigned int    option;

        /*  Query the current pipe state.  If not gray or bitonal, ignore.
            Do an ilConvert(), inverting "blackIsZero", using the "soft
            invert" option, to cause the invert.
                Note that if the "soft invert" option was not used, the 
            pixels would be inverted, but blackIsZero would be too.  When
            the image is then displayed, ilWriteXDrawable() would check
            blackIsZero, and would invert the image to get it the way it
            wants it (false for bitonal, true for gray).  The image would
            then be inverted twice, with no apparent inversion.
                Also note that a "hard" invert (invert the pixels without
            changing blackIsZero) can be achieved by doing ilConvert (option=0)
            with blackIsZero flipped, then another ilConvert(option=softinvert)
            with blackIsZero back to its original value.
        */
    ilQueryPipe (pC->pipe, (long *)NULL, (long *)NULL, &des);
    if ((des.type == IL_BITONAL) || (des.type == IL_GRAY)) {
        des.blackIsZero = (des.blackIsZero) ? FALSE : TRUE;
        des.compression = IL_UNCOMPRESSED;      /* ilConvert() will not compress */
        ilConvert (pC->pipe, &des, (ilImageFormat *)NULL, 
                   IL_CONVERT_SOFT_INVERT, NULL);
        }
    return pC->context->error;
}

    /*  ------------------------ iluCompress ------------------------------- */
    /*  Compress the image.
    */
static iluError iluCompress (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
    ilCompress (pC->pipe, pR->compress.compression, pR->compress.pCompData,
                pR->compress.stripHeight, 0);
    return pC->context->error;
}

    /*  ------------------------ iluDecompress ---------------------------- */
    /*  Decompress the image.
    */
static iluError iluDecompress (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
        /*  Call ilGetPipeInfo() to force decompression as a side effect. */
    ilGetPipeInfo (pC->pipe, TRUE, (ilPipeInfo *)NULL, (ilImageDes *)NULL,
                   (ilImageFormat *)NULL);
    return pC->context->error;
}


    /*  ------------------------ iluDoChecksum ---------------------------- */
    /*  Checksum the image.
    */
static iluError iluDoChecksum (
    iluRequestPtr   pR,
    iluControlPtr   pC
    )
{
        /*  Add a pipe element to calculate checksum from pipe image */
    iluChecksumPipe (pC->pipe, FALSE, pR->checksum.pChecksum);
    return pC->context->error;
}



    /* ====================== ILU-SPECIFIC CODE ========================== */
    /*  The rest of the file is somewhat specific to ILU - it less interesting
        than the above code, as an example of using the IL.
    */

    /*  -------------------- iluDestroyControl ------------------------------- */
    /*  Destroy (free) the given control block and all its resources (not the
        associated context).
    */
iluError iluDestroyControl (
    iluControl      control
    )
{
    register iluControlPtr  pC;
    iluError                error;

    pC = (iluControlPtr)control;
    if (pC->pipe)
        ilDestroyObject (pC->pipe);
    if (pC->maskPipe)
        ilDestroyObject (pC->maskPipe);

/*  NOTE!!! next lines commented out, because to free the XWC would 
    dellocate X colors.

    if (pC->XWC)
        ilDestroyObject (pC->XWC);
*/

    if (pC->maskXWC)
        ilDestroyObject (pC->maskXWC);
    if (pC->readImage)
        ilDestroyObject (pC->readImage);
    if (pC->writeImage)
        ilDestroyObject (pC->writeImage);

    if (pC->dstGC)
        XFreeGC (pC->dstDisplay, pC->dstGC);
    if (pC->maskGC)
        XFreeGC (pC->dstDisplay, pC->maskGC);
    if (pC->maskBitmap)
        XFreePixmap (pC->dstDisplay, pC->maskBitmap);

    error = pC->context->error;
    free (pC);
    return error;
}


    /*  --------------------- iluCreateMaskPipe --------------------------- */
    /*  If relevant create pC->maskPipe to handle mask (clip) image.
        Return true if no error, whether maskPipe created or not.
    */
static ilBool iluCreateMaskPipe (
    iluRequestPtr       pR,
    iluControlPtr       pC
    )
{
    Display            *display;
    ilEFSPageInfo       pageInfo;
    ilEFSFileInfo       fileInfo;
    ilBool              createImage;        /* else create X pixmap */
    ilImageInfo         imageInfo;
    long                width, height;

        /*  Determine if mask image present (current EFS page has a mask image)
            and relevant (output file supports masks, or displaying to existing
            window).  The mask is omitted when displaying to a new window, 
            because the new window has no contents to show thru the transparent
            pixels, other than background color - and presumably the main image
            has an acceptable background color in the transparent areas.
        */
    switch (pR->read.rw.type) {
      case ILU_RW_X:
      default:
        return TRUE;
        break;
      case ILU_RW_EFS:
        if (!ilEFSGetPageInfo (pR->read.rw.EFSFile, &pageInfo))
            return FALSE;
        if (!(pageInfo.attributes & IL_EFS_MASKED))
            return TRUE;
        }

    switch (pR->write.rw.type) {
      case ILU_RW_X:
        createImage = FALSE;
        switch (pR->write.rw.windowUsage) {
          case ILU_WINDOW_ROOT:
          case ILU_WINDOW_ID:
            break;
          case ILU_WINDOW_CREATE:
          default:
            return TRUE;
            }
        break;
      case ILU_WRITE_TIFF:
        createImage = TRUE;
        return TRUE;                /* TIFF-specific write disabled for now */
        break;
      case ILU_RW_EFS:
        createImage = TRUE;
        if (!ilEFSGetFileInfo (pR->write.rw.EFSFile, &fileInfo))
            return FALSE;
        if (!(fileInfo.attributes & IL_EFS_MASK_WRITES))
            return TRUE;
        break;
      case ILU_WRITE_NULL:
        return TRUE;
        break;
      default:
        return TRUE;
        }

        /*  If we got this far, a mask is available and relevant.  Create
            a pipe that reads the mask and  does the geometric pipe operations
        */
    pC->maskPipe = ilCreatePipe (pR->context, 0);
    if (!pC->maskPipe)
        return FALSE;

    if (!ilEFSReadImage (pC->maskPipe, pR->read.rw.EFSFile, IL_EFS_READ_MASK, 
                         0, 0, NULL))
        return FALSE;

    if (pR->crop.enabled)
        if (!ilCrop (pC->maskPipe, &pR->crop.rect))
            return FALSE;
    if (pR->scale.enabled)
        if (!ilScale (pC->maskPipe, pR->scale.width, pR->scale.height,
                     IL_SCALE_SIMPLE, NULL))
            return FALSE;
    if (pR->mirror.enabled)
        if (!ilMirror (pC->maskPipe, pR->mirror.direction))
            return FALSE;
    if (pR->rotate90.enabled)
        if (!ilRotate90 (pC->maskPipe, pR->rotate90.factor))
            return FALSE;

        /*  Either write the result to a client image (for writing to a file) 
            or to an X bitmap (for X bit-clipped display).
        */
    ilQueryPipe (pC->maskPipe, &width, &height, (ilImageDes *)NULL);
    if (createImage) {
        imageInfo.pDes = IL_DES_BITONAL;
        imageInfo.pFormat = IL_FORMAT_BIT;
        imageInfo.width = width;
        imageInfo.height = height;
        imageInfo.pPalette = (unsigned short *)NULL;
        imageInfo.pCompData = (ilPtr)NULL;
        imageInfo.clientPalette = imageInfo.clientCompData = 
            imageInfo.clientPixels = FALSE;     /* let IL allocate pixels */
        pC->maskImage = ilCreateClientImage (pC->context, &imageInfo, 0);
        if (!pC->maskImage)
            return FALSE;
        if (!ilWriteImage (pC->maskPipe, pC->maskImage))
            return FALSE;
        }
    else {
            /*  Create bitmap (pixmap of depth 1) and write to it.  Must
                create GC, with fore/background colors of 1/0, then an XWC,
                using null visual/colormap (special for writing to bitmaps),
                then write to the bitmap using the maskXWC.
            */
        display = pR->write.rw.display;
        pC->maskBitmap = XCreatePixmap (display, DefaultRootWindow (display), 
                                        width, height, 1);
        if (!pC->maskBitmap)
            return FALSE;
        pC->maskGC = XCreateGC (display, pC->maskBitmap, 0, (XGCValues *)NULL);
        if (!pC->maskGC) 
            return FALSE;
        XSetForeground (display, pC->maskGC, 1);
        XSetBackground (display, pC->maskGC, 0);
        pC->maskXWC = ilCreateXWC (pC->context, display, (Visual *)NULL, 
                                   (Colormap)0, pC->maskGC, 0, 0);
        if (!pC->maskXWC)
            return FALSE;
        if (!ilWriteXDrawable (pC->maskPipe, pC->maskBitmap, pC->maskXWC, 
                      (ilRect *)NULL, 0, 0, 0))
            return FALSE;
        }

    return TRUE;
}



    /*  ----------------------- iluCreateControl -------------------------- */
    /*  Create a control rec from the given request information (*pRequest)
        and return a ptr to it, to *pControl, or return error.  The request
        is then executed by calling iluExecuteControl () with *pControl.
        "context" is an IL context from IL_CREATE_CONTEXT().
            Note that to some extent the request is executed by this function:
        a window may be created, the TIFF file header may be written, etc.
        iluExecuteControl() must be called to complete the request however.
    */
iluError iluCreateControl (
    iluRequestPtr   pR,
    iluControl     *control            /* RETURNED */
    )
{
    register iluControlPtr  pC;
    iluError                error;
    ilContext               context;

    *control = (iluControl)NULL;
    pC = (iluControlPtr)malloc (sizeof (iluControlRec));
    if (!pC)
        return ILU_ERROR_MALLOC;

        /*  Init control block, null fields in case of error. */
    context = pR->context;
    pC->context = context;
    pC->pipe = pC->maskPipe = (ilPipe)0;
    pC->dstWindow = (Window)0;
    pC->dstDisplay = pR->write.rw.display;
    pC->dstGC = (GC)0;
    pC->dstVisual = (Visual *)NULL;
    pC->dstColormap = (Colormap)0;
    pC->maskImage = (ilObject)NULL;
    pC->maskBitmap = (Pixmap)0;
    pC->maskGC = (GC)0;
    pC->maskXWC = (ilXWC)NULL;
    pC->XWC = (ilXWC)NULL;
    pC->dstFile = (ilFile)NULL;
    pC->clearWindow = pR->write.clearWindow;
    pC->readImage = (ilInternalImage)NULL;
    pC->writeImage = (ilInternalImage)NULL;

        /*  Build maskPipe if enabled, relevant for output and current 
            page has a mask.
        */
    if (!iluCreateMaskPipe (pR, pC)) {
        error = context->error;
        goto ccError;
        }

        /*  Build pipe */
    pC->pipe = ilCreatePipe (context, 0);
    if (!pC->pipe) {
        error = context->error;
        goto ccError;
        }

        /*  Add producer (read) plus crop if requested. */
    switch (pR->read.rw.type) {
      case ILU_RW_X:
        if (error = iluReadX (pR, pC))
            goto ccError;
        break;
      case ILU_RW_EFS:
        if (error = iluReadEFS (pR, pC))
            goto ccError;
        break;
      default:
        error = ILU_ERROR_READ_TYPE;
        goto ccError;
        }

        /*  If useImage, create image from the pipe, execute the pipe,
            empty it and restart it with a read from the image.
                NOTE: an internal image will be stored in strips if the image
            is compressed in the file.  But a client image is always stored as
            a single strip.  Therefore, internal images are handled more 
            efficiently in this case than client images.
        */
    if (pR->general.useImage) {
        long                width, height;
        ilImageDes          des;

        ilQueryPipe (pC->pipe, &width, &height, &des);
        pC->readImage = ilCreateInternalImage (context, width, height, &des, 0);
        if (!pC->readImage
         || !ilWriteImage (pC->pipe, pC->readImage)
         || (ilExecutePipe (pC->pipe, 0) == IL_EXECUTE_ERROR)
         || !ilEmptyPipe (pC->pipe)
         || !ilReadImage (pC->pipe, pC->readImage)) {
            error = context->error;
            goto ccError;
            }
        }

        /*  Add optional filters that are enabled. */
    if (pR->crop.enabled)
        if (error = iluCrop (pR, pC))
            goto ccError;

    if (pR->scale.enabled) {
        pC->scaleThreshold = pR->scale.biThreshold;
        if (error = iluScale (pC->pipe, pR->scale.width, pR->scale.height, 
                               pR->scale.method, &pC->scaleThreshold))
            goto ccError;
        }

    if (pR->convert.enabled)
        if (error = iluConvert (pR, pC))
            goto ccError;

    if (pR->gamma.enabled)
        if (error = iluGamma (pC->pipe, pR->gamma.factor))
            goto ccError;

    if (pR->mirror.enabled)
        if (error = iluMirror (pR, pC))
            goto ccError;

    if (pR->rotate90.enabled)
        if (error = iluRotate90 (pR, pC))
            goto ccError;

    if (pR->invert.enabled)
        if (error = iluInvert (pR, pC))
            goto ccError;

    if (pR->compress.enabled)
        if (error = iluCompress (pR, pC))
            goto ccError;

    if (pR->decompress.enabled)
        if (error = iluDecompress (pR, pC))
            goto ccError;

    if (pR->checksum.enabled)
        if (error = iluDoChecksum (pR, pC))
            goto ccError;

        /*  Add consumer (write) with pre-crop if requested. */
    switch (pR->write.rw.type) {
      case ILU_WRITE_TIFF:
        if (error = iluWriteTIFF (pR, pC))
            goto ccError;
        break;
      case ILU_RW_EFS:
        if (error = iluWriteEFS (pR, pC))
            goto ccError;
        break;
      case ILU_RW_X:
        if (error = iluWriteX (pR, pC))
            goto ccError;
        break;
      case ILU_WRITE_NULL:
        if (error = iluWriteNull (pR, pC))
            goto ccError;
        break;
      case ILU_WRITE_IMAGE:
        if (error = iluWriteImage (pR, pC))
            goto ccError;
        break;
      default:
        error = ILU_ERROR_WRITE_TYPE;
        goto ccError;
        }

        /*  Success: return ptr to control block. */
    *control = (iluControl)pC;
    return ILU_OK;

        /*  Error: destroy control block, return "error" (maybe from IL). */
ccError:
    iluDestroyControl ((iluControl)pC);
    return error;
}



    /*  ----------------------- iluExecuteControl -------------------------- */
    /*  Execute the pipe associated with the given control block.
    */
iluError iluExecuteControl (
    iluControl      control
    )
{
    register iluControlPtr  pC;

    pC = (iluControlPtr)control;

        /*  If there is an output window and option enabled, clear it */
    if (pC->dstWindow && pC->clearWindow) {
        XClearWindow (pC->dstDisplay, pC->dstWindow);
        XFlush (pC->dstDisplay);
        }

        /*  Execute the mask (if present) and "main" pipe(s)  */
    if (pC->maskPipe)
        ilExecutePipe (pC->maskPipe, 0);
    ilExecutePipe (pC->pipe, 0);
    return pC->context->error;
}


    /*  -------------------- iluCreateRequest -------------------------- */
    /*  Allocate and initialize an iluRequestRec and return a pointer to it
        or null if failure.
    */
iluRequestPtr iluCreateRequest (
    ilContext       context
    )
{
    register iluRequestPtr  pR;

    pR = (iluRequestPtr)malloc (sizeof (iluRequestRec));
    if (!pR)
        return (iluRequestPtr)NULL;

        /*  Default all fields in the request rec.
        */
    pR->context = context;

    pR->general.masksEnabled = TRUE;
    pR->general.useImage = FALSE;

    pR->read.rw.type = ILU_RW_EFS;
    pR->read.rw.doCrop = FALSE;
    pR->read.rw.cropRect.x = pR->read.rw.cropRect.y = 0;
    pR->read.rw.cropRect.width = pR->read.rw.cropRect.height = 1000000;
    pR->read.rw.stream = (FILE *)NULL;
    pR->read.rw.EFSFile = (ilEFSFile)NULL;
    pR->read.rw.display = (Display *)NULL;
    pR->read.rw.windowUsage = ILU_WINDOW_ROOT;
    pR->read.rw.windowID = (Window)0;

        /*  Default filters to disabled, default their fields.
        */
    pR->crop.enabled = FALSE;
    pR->crop.rect.x = pR->crop.rect.y = 0;
    pR->crop.rect.width = pR->crop.rect.height = 1000000;

    pR->scale.enabled = FALSE;
    pR->scale.width = pR->scale.height = 0;
    pR->scale.method = IL_SCALE_SAMPLE;
    pR->scale.biThreshold = 128;

    pR->convert.enabled = FALSE;
    pR->convert.type = IL_BITONAL;
    pR->convert.option = 0;
    pR->convert.biThreshold = 128;
    pR->convert.subsample = 1;
    pR->convert.palette.method = IL_DIFFUSION;
    pR->convert.palette.levels[0] = 4;
    pR->convert.palette.levels[1] = 8;
    pR->convert.palette.levels[2] = 4;
    pR->convert.palette.kernelSize = 8;
    pR->convert.palette.dstType = IL_PALETTE;
    pR->convert.palette.mapImage = (ilObject)NULL;

    pR->gamma.enabled = FALSE;
    pR->gamma.factor = 1.0;

    pR->mirror.enabled = FALSE;
    pR->mirror.direction = IL_MIRRORX;

    pR->rotate90.enabled = FALSE;
    pR->rotate90.factor = 0;

    pR->invert.enabled = FALSE;

    pR->compress.enabled = FALSE;
    pR->compress.compression = IL_UNCOMPRESSED;
    pR->compress.pCompData = (ilPtr)NULL;
    pR->compress.stripHeight = 0;

    pR->decompress.enabled = FALSE;

    pR->checksum.enabled = FALSE;

        /*  Default write fields to same as read, then set differences. */
    pR->write.rw = pR->read.rw;
    pR->write.rw.type = ILU_RW_X;
    pR->write.rw.windowUsage = ILU_WINDOW_CREATE;

        /*  Default write-specific options, e.g. TIFF options. */
    pR->write.clearWindow = FALSE;
    pR->write.useDefaultVisual = FALSE;
    pR->write.usePrivateColormap = FALSE;
    pR->write.useAllColors = FALSE;
    pR->write.dstX = pR->write.dstY = 0;
    pR->write.windowX = pR->write.windowY = 0;
    pR->write.noWM = FALSE;
    pR->write.ditherMethod = -1;        /* => dont set; use IL's default */
    pR->write.doAppend = FALSE;
    pR->write.compression = IL_WRITE_CURRENT_COMPRESSION;
    pR->write.pCompData = (ilPtr)NULL;
    pR->write.xRes = pR->write.yRes = 200;
    pR->write.page = pR->write.nPages = -1;     /* default: no page numbers */
    pR->write.stripHeight = 0;

    return pR;
}


    /*  -------------------- iluDestroyRequest -------------------------- */
    /*  Destroy the request rec created by iluCreateRequest().
    */
iluError iluDestroyRequest (
    iluRequestPtr    pR
    )
{
    free (pR);
    return ILU_OK;
}
