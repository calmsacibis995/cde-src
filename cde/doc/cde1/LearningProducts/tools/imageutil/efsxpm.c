/**---------------------------------------------------------------------
***	
***    file:           efsxpm.c
***
***    description:    Image Library (IL) EFS support for xpm
***
***	
***    (c)Copyright 1992 Hewlett-Packard Co.
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

    /*  Code for reading  xpm (X Pixel Map) format
        files, aka .xpm files
    */

#include "il.h"
#include "ilefs.h"
#include "ilpipelem.h"
#include "ilerrors.h"
#include "ilxpmP.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif


    /*  Private data for each xpm EFS file */
typedef struct {
    ilEFSPageInfo           info;           /* info on current/only page */
    ilClientImage           mainImage;
    ilClientImage           maskImage;
    } xpmPrivRec, *xpmPrivPtr;

static xpmDataType xpmDataTypes[] =
{
 "", "!", "\n", '\0', '\n', "", "", "", "",	/* Natural type */
 "C", "/*", "*/", '"', '"', ",\n", "static char *", "[] = {\n", "};\n",
 "Lisp", ";", "\n", '"', '"', "\n", "(setq ", " '(\n", "))\n",
#ifdef VMS
 NULL
#else
 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#endif
};



        /*  ------------------------ xpmOpen ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
            the number of pages in it and ptr to private, or null if error.
        */
static ilPtr xpmOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register ilError            error;
register xpmPrivPtr        pPriv;
ilEFSPageInfo               info;
char                        buf[BUFSIZ];
int                         l, n = 0;
int                         ErrorStatus;
xpmData                     mdata;          /* xpm file control record */
XpmAttributes               attributes;     /* xpm  attributes */
xpmInternAttrib             attrib;         /* xpm internal attributes */
ilClientImage              image_return;
ilClientImage              shapeimage_return;
unsigned short            *pPalette;
ilImageDes                 *pdes;
ilImageFormat              *pformat;

    attributes.valuemask = 0;

    pPriv = (xpmPrivPtr)malloc (sizeof (xpmPrivRec));
    if (!pPriv) {
        fileType->context->error = IL_ERROR_MALLOC;
        return (ilPtr)NULL;
        }

        /*  Open the file, write mode is not supported.  Error if open fails. */
    if (openMode == IL_EFS_WRITE) {
        fileType->context->error = IL_ERROR_EFS_OPEN;
        return (ilPtr)NULL;
        }
        /*  Opening in read mode: connect to file, return "not mine" error
            if not a xpm file.  Do a quick check of front of file first:
            extract the necessary image info.
        */
    if (_ilefsxpmReadFile(fileName, &mdata) != XpmSuccess) {
        fileType->context->error = IL_ERROR_EFS_OPEN;
        return (ilPtr)NULL;
        }

    _ilefsxpmInitInternAttrib(&attrib);


    /*
     * parse the header file 
     */

    mdata.Bos = '\0';
    mdata.Eos = '\n';
    mdata.Bcmt = mdata.Ecmt = NULL;
    _ilefsxpmNextWord(&mdata, buf);     /* skip the first word */
    l = _ilefsxpmNextWord(&mdata, buf); /* then get the second word */
    if ((l == 3 && !strncmp("XPM", buf, 3)) ||
        (l == 4 && !strncmp("XPM2", buf, 4))) {
        if (l == 3)
            n = 1;                        /* handle XPM as XPM2 C */
        else {
            l = _ilefsxpmNextWord(&mdata, buf);        /* get the type key word */
            /*
             * get infos about this type 
             */
            while (xpmDataTypes[n].type
                   && strncmp(xpmDataTypes[n].type, buf, l))
                n++;
        }
        if (xpmDataTypes[n].type) {
            if (n == 0) {                /* natural type */
                mdata.Bcmt = xpmDataTypes[n].Bcmt;
                mdata.Ecmt = xpmDataTypes[n].Ecmt;
                _ilefsxpmNextString(&mdata);        /* skip the end of headerline */
                mdata.Bos = xpmDataTypes[n].Bos;
            } else {
                _ilefsxpmNextString(&mdata);        /* skip the end of headerline */
                mdata.Bcmt = xpmDataTypes[n].Bcmt;
                mdata.Ecmt = xpmDataTypes[n].Ecmt;
                mdata.Bos = xpmDataTypes[n].Bos;
                mdata.Eos = '\0';
                _ilefsxpmNextString(&mdata);        /* skip the assignment line */
                mdata.InsideString = 1;
            }
            mdata.Eos = xpmDataTypes[n].Eos;

	    if (ErrorStatus = _ilefsxpmParseData(&mdata, &attrib, &attributes)) {
                error = IL_ERROR_EFS_OPEN;
                goto openError;
            }
            pPalette = (unsigned short *)malloc (sizeof (unsigned short) * 3 * 256);
            if (pPalette) {
                attrib.pPalette = pPalette;
                attrib.nPaletteColors = 0;
                ErrorStatus = _ilefsxpmCreateImage((ilObject) fileType, &attrib, &image_return,
				     &shapeimage_return, &attributes);
                if (ErrorStatus >= 0) {
                    pPriv->mainImage = image_return;
                    pPriv->maskImage = shapeimage_return;
                }
                else {
                    free(pPalette);
                    error = IL_ERROR_EFS_OPEN;
                    goto openError;
                }
            }
            else {
                error = IL_ERROR_MALLOC;
                goto openError;
            }
        } else  {
            error = IL_ERROR_EFS_NOT_MINE;
            goto openError;
        }
    } else {
        error = IL_ERROR_EFS_NOT_MINE;
        goto openError;
    }

    *pNPages = 1;

    info.page = 0;                  /* single page file; always page 0 */
    info.attributes = (pPriv->maskImage)? IL_EFS_MASKED : 0;
    info.xRes = 0;
    info.yRes = 0;
    info.width = attrib.width;
    info.height = attrib.height;
    pdes = IL_DES_PALETTE;
    pformat = IL_FORMAT_BYTE; 
    info.des = *pdes;
    info.format = *pformat;
    pPriv->info = info;

/* close up xpm shop */
    _ilefsxpmSetAttributes(&attrib, &attributes);
/* if we want to save attribute data e.g. hot spot, do it here */
    _ilefsXpmFreeAttributes(&attributes);
    _ilefsxpmFreeInternAttrib(&attrib);
    _ilefsXpmDataClose(&mdata);

    fileType->context->error = IL_OK;
    return (ilPtr)pPriv;

    /*  goto point on error after file open: post "error", close file. */
openError:
    fileType->context->error = error;
    _ilefsXpmFreeAttributes(&attributes);
    _ilefsxpmFreeInternAttrib(&attrib);
    _ilefsXpmDataClose(&mdata);
    free (pPriv);
    return (ilPtr)NULL;
}


        /*  ------------------------ xpmClose ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Close().  remove client images,
            free pixel buffers, color palette.
        */
static ilBool xpmClose (
    ilEFSFile               file
    )
{
register xpmPrivPtr        pPriv;
ilImageInfo      *pInfo;


    pPriv = (xpmPrivPtr)file->pPrivate;
    if (pPriv->mainImage) {
    if (!ilQueryClientImage(pPriv->mainImage, &pInfo, 0L)) {
        if (pInfo->plane[0].pPixels) free(pInfo->plane[0].pPixels);
        if (pInfo->pPalette) free(pInfo->pPalette);
        ilDestroyObject((ilObject) pPriv->mainImage);
    }
    }
    if (pPriv->maskImage) {
    if (!ilQueryClientImage(pPriv->maskImage, &pInfo, 0L)) {
        if (pInfo->plane[0].pPixels) free(pInfo->plane[0].pPixels);
        ilDestroyObject((ilObject) pPriv->maskImage);
    }
    }
    return TRUE;
}


        /*  ------------------------ xpmGetPageInfo ---------------------- */
        /*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
            on the current page.  Called only if file opened in read mode.
        */
static ilBool xpmGetPageInfo (
    ilEFSFile               file,
    ilEFSPageInfo          *pInfo
    )
{
        /*  Return info on the current (only) image in the file. */
    *pInfo = ((xpmPrivPtr)file->pPrivate)->info;
    return TRUE;
}



        /*  ------------------------ xpmReadImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
            to read the current page into the given pipe.
        */
static ilBool xpmReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode 
    )
{
register xpmPrivPtr        pPriv;

        /*  Add a pipe producer to read the entire xpm file into the pipe.
            The des, format and size are in private, from xpmOpen().
        */
    pPriv = (xpmPrivPtr)file->pPrivate;

    if (readMode == IL_EFS_READ_MAIN)
        return (ilReadImage( pipe, pPriv->mainImage));
    else if (pPriv->info.attributes  & IL_EFS_MASKED)
        return (ilReadImage( pipe, pPriv->maskImage));
    else {
	pipe->context->error = IL_ERROR_EFS_READ_MODE;
	return (FALSE);
	}
}



        /*  ------------------------ _ilefsInitxpm  ------------------------- */
        /*  Add xpm EFS file type handler to the given context; 
            return false if error.
        */
ilBool _ilefsInitxpm (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "xpm");
    strcpy (info.displayName, "xpm");
    info.checkOrder = 80;                   /* somewhat arbitrary */

    info.nExtensions = 4;
    strcpy (info.extensions[0], "xpm");
    strcpy (info.extensions[1], "XPM");
    strcpy (info.extensions[2], "pm");
    strcpy (info.extensions[3], "PM");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL);
    info.attributes = IL_EFS_MASK_READS ;

    info.Open = xpmOpen;
    info.GetPageInfo = xpmGetPageInfo;
    info.ReadImage = xpmReadImage;
    info.WriteImage = NULL;
    info.Close = xpmClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}



