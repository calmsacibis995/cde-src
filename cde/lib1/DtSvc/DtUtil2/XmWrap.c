/* $XConsortium: XmWrap.c /main/cde1_maint/2 1995/08/26 22:46:36 barstow $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <Xm/XmP.h>
#include <Xm/IconFile.h>

#ifdef _NO_PROTO
extern Pixmap XmeGetMask() ;
#else
extern Pixmap XmeGetMask(Screen *screen, char *image_name);
#endif

/************************************************************************
 *
 *  _DtGetPixmapByDepth
 *      Public wrapper around __DtGetPixmap with parameter order changed.
 *
 ************************************************************************/
Pixmap
#ifdef _NO_PROTO
  _DtGetPixmapByDepth(screen, image_name, foreground, background, depth)
Screen *screen ;
char *image_name ;
Pixel foreground ;
Pixel background ;
int depth;
#else
_DtGetPixmapByDepth(
                     Screen *screen,
                     char *image_name,
                     Pixel foreground,
                     Pixel background,
                     int depth)
#endif /* _NO_PROTO */
{
    return(_XmGetPixmap(screen, image_name, depth,
                          foreground, background));
}

/************************************************************************
*
*  _DtGetPixmap
*       Create a pixmap of screen depth, using the image referenced
*       by the name and the foreground and background colors
*       specified.  Ensure that multiple pixmaps of the same attributes
*       are not created by maintaining a cache of the pixmaps.
*
************************************************************************/
Pixmap
#ifdef _NO_PROTO
_DtGetPixmap( screen, image_name, foreground, background )
        Screen *screen ;
        char *image_name ;
        Pixel foreground ;
        Pixel background ;
#else
_DtGetPixmap(
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background )
#endif /* _NO_PROTO */
{
    return (XmGetPixmap(screen, image_name, foreground, background));
}

/************************************************************************
*
*  _DtGetMask
*
************************************************************************/
Pixmap 
#ifdef _NO_PROTO
_DtGetMask( screen, image_name )
        Screen *screen ;
        char *image_name ;
#else
_DtGetMask(
        Screen *screen,
        char *image_name)
#endif /* _NO_PROTO */
{
    return XmeGetMask( screen, image_name );
}

/************************************************************************
 *
 *  __DtInstallPixmap
 *      Install a pixmap into the pixmap cache.  This is used to add
 *      cached pixmaps which have no image associated with them.
 *
 ************************************************************************/
Boolean
#ifdef _NO_PROTO
__DtInstallPixmap( pixmap, screen, image_name, foreground, background )
        Pixmap pixmap ;
        Screen *screen ;
        char *image_name ;
        Pixel foreground ;
        Pixel background ;
#else
__DtInstallPixmap(
        Pixmap pixmap,
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background )
#endif /* _NO_PROTO */
{
    return _XmInstallPixmap(pixmap, screen, image_name, foreground, background);
}

/************************************************************************
 *
 *  _DtDestroyPixmap
 *      Locate a pixmap in the cache and decrement its reference count.
 *      When the reference count is at zero, free the pixmap.
 *
 ************************************************************************/
Boolean
#ifdef _NO_PROTO
_DtDestroyPixmap( screen, pixmap )
        Screen *screen ;
        Pixmap pixmap ;
#else
_DtDestroyPixmap(
        Screen *screen,
        Pixmap pixmap )
#endif /* _NO_PROTO */
{
    return XmDestroyPixmap( screen, pixmap );
}

/************************************************************************
 *
 *  _DtInstallImage
 *      Add the provided image for the image set and return an
 *      tile id to be used for further referencing.  Keep the
 *      allocation of the image_set array straight.
 *
 ************************************************************************/
Boolean
#ifdef _NO_PROTO
_DtInstallImage( image, image_name, hot_x, hot_y )
        XImage *image ;
        char *image_name ;
        int hot_x;
        int hot_y;
#else
_DtInstallImage(
        XImage *image,
        char *image_name,
        int hot_x,
        int hot_y)
#endif /* _NO_PROTO */
{
    return _XmInstallImage(image, image_name, hot_x, hot_y);
}

/************************************************************************
 *
 *  _DtUninstallImage
 *      Remove an image from the image set.
 *      Return a boolean (True) if the uninstall succeeded.  Return
 *      a boolean (False) if an error condition occurs.
 *
 ************************************************************************/
Boolean
#ifdef _NO_PROTO
_DtUninstallImage( image )
        XImage *image ;
#else
_DtUninstallImage(
        XImage *image )
#endif /* _NO_PROTO */
{
    return XmUninstallImage( image );
}

XImage *
#ifdef _NO_PROTO
_DtGetImage(screen, image_name, image)
        Screen *screen;
        char *image_name;
        XImage **image;
#else
_DtGetImage(
        Screen *screen,
        char *image_name,
        XImage **image)
#endif /* _NO_PROTO */
{
    Boolean result;

    if( (result=_XmGetImage(screen, image_name, image)) == False )
	return False;
    else
	return *image;
}

/*
 * see if this pixmap is in the cache. If it is then return all the
 * gory details about it
 */
/*ARGSUSED*/
Boolean
#ifdef _NO_PROTO
  __DtGetPixmapData(screen, pixmap, image_name, depth, foreground,
                     background, hot_x, hot_y, width, height)
Screen *screen ;
Pixmap pixmap;
char  **image_name ;/* RETURN */
int   *depth;   /* RETURN */
Pixel *foreground ; /* RETURN */
Pixel *background ; /* RETURN */
int     *hot_x, *hot_y; /* RETURN */
unsigned int *width, *height;/* RETURN */
#else
__DtGetPixmapData(
                   Screen *screen,
                   Pixmap pixmap,
                   char **image_name,
                   int *depth,
                   Pixel *foreground,
                   Pixel *background,
                   int *hot_x,
                   int *hot_y,
                   unsigned int *width,
                   unsigned int *height)
#endif /* _NO_PROTO */
{
    return _XmGetPixmapData(screen, pixmap, image_name, depth, foreground,
                             background, hot_x, hot_y, width, height);
}

String
#ifdef _NO_PROTO
_DtGetIconFileName(screen, imageInstanceName, imageClassName, hostPrefix, size)
    Screen      *screen;
    String      imageInstanceName;
    String      imageClassName;
    String      hostPrefix;
    unsigned int size;
#else
_DtGetIconFileName(
    Screen      *screen,
    String      imageInstanceName,
    String      imageClassName,
    String      hostPrefix,
    unsigned int size)
#endif
{
	return XmGetIconFileName(screen, imageInstanceName, imageClassName, 
				 hostPrefix, size);
}

void
#ifdef _NO_PROTO
  _DtFlushIconFileCache(path)
String  path;
#else
 _DtFlushIconFileCache(String   path)
#endif
{
	XmeFlushIconFileCache(path);
}
