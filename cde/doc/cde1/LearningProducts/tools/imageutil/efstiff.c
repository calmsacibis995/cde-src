/**---------------------------------------------------------------------
***	
***    file:           efstiff.c
***
***    description:    Image Library (IL) EFS support for TIFF 
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

#include "ilefs.h"
#include "ilfile.h"
#include "ilerrors.h"
#include "ilpipelem.h"
#include <string.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif


    /*  Private data for each TIFF EFS file */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    ilFile                  file;           /* IL's file */
    ilFileImage             pFileImages;    /* ptr to list of pages/images */
    ilFileImage             pFileImage;     /* ptr to current page/image */
    unsigned long           page;           /* current page #; 0 is 1st page */
    } tiffPrivRec, *tiffPrivPtr;

    /*  Private data when writing mask images. */
typedef struct {
    ilClientImage           maskImage;      /* mask image to write */
    ilFile                  file;           /* IL's file */
    long                    xRes, yRes;     /* res given to WriteImage() */
    } maskPrivRec, *maskPrivPtr;


        /*  ------------------------ tiffOpen ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
            the number of pages in it and ptr to private, or null if error.
        */
static ilPtr tiffOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register ilContext          context;
register tiffPrivPtr        pPriv;
FILE                       *stream;
ilFile                      file;
ilFileImage                 pFileImages;
ilBool                      doWrite;

    context = fileType->context;
    doWrite = (openMode == IL_EFS_WRITE);

        /*  Open the file, write mode if requested.  Error if open fails. */
    stream = fopen (fileName, (doWrite) ? "w+" : "r");
    if (!stream) {
        context->error = IL_ERROR_EFS_OPEN;
        return (ilPtr)NULL;
        }

        /*  If opening in write mode, write a header to the file to init it. */
    if (doWrite) {
        if (!ilInitFile (context, stream, 0, TRUE, 0))
            goto openError;
        file = ilConnectFile (context, stream, 0, 0);
        if (!file)
            goto openError;
        *pNPages = 0;
        pFileImages = (ilFileImage)NULL;
        }
    else {
            /*  Opening in read mode: connect to file, return "not mine" error
                if not a TIFF file, or "empty file" if no images & not error.
            */
        int                 nPages;
        register ilFileImage pFileImage;

        file = ilConnectFile (context, stream, 0, 0);
        if (!file) {
            if (context->error == IL_ERROR_FILE_NOT_TIFF)
                context->error = IL_ERROR_EFS_NOT_MINE;
            goto openError;
            }

        pFileImages = ilListFileImages (file, 0);
        if (!pFileImages) {
            if (context->error == IL_OK)
                context->error = IL_ERROR_EFS_EMPTY_FILE;
            goto openError;
            }

            /*  Count the number of pages (> 0!), return to *pNPages. */
        pFileImage = pFileImages;
        nPages = 0;
        while (pFileImage) {
            pFileImage = pFileImage->pNext;
            nPages++;
            }
        *pNPages = nPages;
        }

        /*  Allocate private, save info in it and return a ptr to it. 
            Position to read from page 0: first image in file.
        */
    pPriv = (tiffPrivPtr)malloc (sizeof (tiffPrivRec));
    if (!pPriv) {
        context->error = IL_ERROR_MALLOC;
        goto openError;
        }
    pPriv->stream = stream;
    pPriv->file = file;
    pPriv->pFileImages = pPriv->pFileImage = pFileImages;
    pPriv->page = 0;
    return (ilPtr)pPriv;

    /*  goto point if error after file is opened: close file, return null. */
openError:
    fclose (stream);
    return (ilPtr)NULL;
}


        /*  ------------------------ tiffClose ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Close().  Close the given file.
        */
static ilBool tiffClose (
    ilEFSFile               file
    )
{
register tiffPrivPtr        pPriv;

    pPriv = (tiffPrivPtr)file->pPrivate;
    fclose (pPriv->stream);
    return TRUE;
}


        /*  ------------------------ tiffSeek ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Seek().  Position to the given page,
            by searching the file list for that element - first is page 0.
        */
static ilBool tiffSeek (
    ilEFSFile               file,
    unsigned long           page
    )
{
tiffPrivPtr                 pPriv;
register ilFileImage        pFileImage;
register unsigned long      currentPage;

        /*  Search thru list until desired page # found, or end of list (seek
            beyond end-of-file: not an error; position to last page.  Set
            current page #, ptr to current page file image into pPriv.
        */
    pPriv = (tiffPrivPtr)file->pPrivate;
    pFileImage = pPriv->pFileImages;
    currentPage = 0;
    while (currentPage < page) {
        if (!pFileImage->pNext)     /* last page; seek beyond # of pages */
            break;
        pFileImage = pFileImage->pNext;
        currentPage++;
        }
    pPriv->pFileImage = pFileImage;
    pPriv->page = currentPage;
    return TRUE;
}


        /*  ------------------------ tiffGetPageInfo ---------------------- */
        /*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
            on the current page.  Called only if file opened in read mode.
        */
static ilBool tiffGetPageInfo (
    ilEFSFile               file,
    register ilEFSPageInfo *pInfo
    )
{
tiffPrivPtr                 pPriv;
register ilFileImage        pFileImage;

        /*  Return info on image at pPriv->pFileImage, or EOF if null,
            which will only be the case if sequential read openMode, and
            all images have been read.
        */
    pPriv = (tiffPrivPtr)file->pPrivate;
    pFileImage = pPriv->pFileImage;
    if (!pFileImage)                    /* all images read; EOF */
        pInfo->attributes = IL_EFS_EOF;
    else {
        pInfo->attributes = 0;
        if (pFileImage->pMask)
            pInfo->attributes |= IL_EFS_MASKED;
        pInfo->page = pPriv->page;      /* TIFF images may be misnumbered */
        pInfo->width = pFileImage->width;
        pInfo->height = pFileImage->height;
        pInfo->xRes = pFileImage->xRes;
        pInfo->yRes = pFileImage->yRes;
        pInfo->des = pFileImage->des;
        pInfo->format = pFileImage->format;
        }
    return TRUE;
}


        /*  ------------------------ tiffReadImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
            to read the current page into the given pipe.
        */
static ilBool tiffReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode
    )
{
tiffPrivPtr                 pPriv;
register ilFileImage        pFileImage;

    pPriv = (tiffPrivPtr)file->pPrivate;
    pFileImage = pPriv->pFileImage;
    if (!pFileImage) {                  /* all images read; EOF */
        file->context->error = IL_ERROR_EFS_EOF;
        return FALSE;
        }

        /*  If a read of mask requested: error if no mask, else read it and
            force decompress and convert to std bit format, scaling it if
            not size of main image (all required by EFS, not guaranteed by
            IL TIFF support).
                If not mask read, read main image and position to next image.
        */
    if (readMode == IL_EFS_READ_MASK) {
        long    mainWidth, mainHeight, width, height;

        mainWidth = pFileImage->width;
        mainHeight = pFileImage->height;
        pFileImage = pFileImage->pMask;
        if (!pFileImage) {                  /* no mask image; error */
            file->context->error = IL_ERROR_EFS_READ_MODE;
            return FALSE;
            }
        if (!ilReadFileImage (pipe, pFileImage, (ilRect *)NULL, 0))
            return FALSE;
        ilQueryPipe (pipe, &width, &height, (ilImageDes *)NULL);
        if ((width != mainWidth) || (height != mainHeight))
            if (!ilScale (pipe, mainWidth, mainHeight, IL_SCALE_SAMPLE, NULL))
                return FALSE;
        return (ilConvert (pipe, (ilImageDes *)NULL, IL_FORMAT_BIT, 0, NULL));
        }
    else {
        if (!ilReadFileImage (pipe, pFileImage, (ilRect *)NULL, 0))
            return FALSE;
        pPriv->pFileImage = pFileImage->pNext;
        pPriv->page++;
        }
    return TRUE;
}


        /*  ------------------------ tiffWriteMaskCleanup ---------------------- */
        /*  Cleanup() IL_HOOK function, executed after pipe that writes main image.
            Create and execute a pipe that writes the mask image, associated with the
            main image.  It is the association that forces us to use a Cleanup() function,
            as the main image must be written before it can be associated to.
        */
static ilError tiffWriteMaskCleanup (
    maskPrivPtr             pPriv,
    ilBool                  aborting
    )
{
    ilPipe                  pipe;
    ilContext               context;
    ilFileImage             pParent;

    context = pPriv->file->context;
    if (aborting)
        return IL_OK;                           /* main pipe aborted: dont write mask */

        /*  Find the last main image in the image list - it will be the one just written
            since page #s were not written, and the IL writes to end of file if no pages.
        */
    pParent = ilListFileImages (pPriv->file, 0);
    if (!pParent)
        return IL_ERROR_EFS_IO;                 /* ? no main image - exit w/ error */
    while (pParent->pNext)                      /* find last image in list */
        pParent = pParent->pNext;

        /*  Form a pipe to read mask image and write to TIFF with PackBits compression.
            Execute it and return error code from it.
            Use Packbits compression, per TIFF recommendation.
        */
    if ((pipe = ilCreatePipe (context, 0))
     && ilReadImage (pipe, pPriv->maskImage)
     && ilWriteFileImage (pipe, pPriv->file, IL_WRITE_MASK, pParent, IL_PACKBITS, 
            (ilPtr)NULL, 0, pPriv->xRes, pPriv->yRes, -1, -1, 0, (ilFileTag *)NULL, 0))
        ilExecutePipe (pipe, 0);                /* execute built pipe */

    return context->error;
}


        /*  ------------------------ tiffWriteImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.WriteImage().  Add a consumer
            to write the pipe image to the given file.
        */
static ilBool tiffWriteImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned long           xRes,
    unsigned long           yRes,
    ilClientImage           maskImage
    )
{
tiffPrivPtr                 pPriv;

    pPriv = (tiffPrivPtr)file->pPrivate;

        /*  Write main image to TIFF using current compression */
    if (!ilWriteFileImage (pipe, pPriv->file, IL_WRITE_MAIN, (ilFileImage)NULL, 
                IL_WRITE_CURRENT_COMPRESSION, (ilPtr)NULL, 0, xRes, yRes, -1, -1, 0,
                (ilFileTag *)NULL, 0))
        return FALSE;

        /*  If a maskImage: add a Cleanup() hook function, to be called when main pipe 
            has completed processing, to write the actual maskImage, using a new pipe.
        */
    if (maskImage) {
        maskPrivPtr     pMaskPriv;
        pMaskPriv = (maskPrivPtr)ilAddPipeElement (pipe, IL_HOOK, sizeof (maskPrivRec), 0,
            (ilSrcElementData *)NULL, (ilDstElementData *)NULL, 
            IL_NPF, tiffWriteMaskCleanup, IL_NPF, IL_NPF, 0);
        if (!pMaskPriv)
            return FALSE;
        pMaskPriv->maskImage = maskImage;
        pMaskPriv->file = pPriv->file;
        pMaskPriv->xRes = xRes;
        pMaskPriv->yRes = yRes;
        }
    return TRUE;
}



        /*  ------------------------ _ilefsInitTIFF ------------------------- */
        /*  Add TIFF EFS file type handler to the given context; 
            return false if error.
        */
ilBool _ilefsInitTIFF (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "TIFF");
    strcpy (info.displayName, "TIFF");
    info.checkOrder = 100;

    info.nExtensions = 4;
    strcpy (info.extensions[0], "tif");
    strcpy (info.extensions[1], "TIF");
    strcpy (info.extensions[2], "tiff");
    strcpy (info.extensions[3], "TIFF");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL)
                   | (1<<IL_EFS_WRITE);
    info.attributes = IL_EFS_MULTI_PAGE_READS | IL_EFS_MULTI_PAGE_WRITES |
                      IL_EFS_MASK_READS | IL_EFS_MASK_WRITES;

    info.Open = tiffOpen;
    info.Seek = tiffSeek;
    info.GetPageInfo = tiffGetPageInfo;
    info.ReadImage = tiffReadImage;
    info.WriteImage = tiffWriteImage;
    info.Close = tiffClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}

