/* $XConsortium: ilcodec.c /main/cde1_maint/1 1995/07/17 18:36:06 drk $ */
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

#include "ilint.h"
#include "ilpipelem.h"
#include "ilpipeint.h"
#include "ilcodec.h"
#include "ilerrors.h"


    /*  ------------------------ ilReallocCompressedBuffer ------------------------ */
    /*  Realloc (or alloc the first time) the pixel buffer for plane "plane" of the 
        compressed image "*pImage", so that its "bufferSize" is a minimum of 
       "minNewSize" bytes in size.
    */
IL_PRIVATE ilBool _ilReallocCompressedBuffer (
    ilImageInfo        *pImage,
    unsigned int        plane,
    unsigned long       minNewSize
    )
{
register ilImagePlaneInfo *pPlane;

    pPlane = &pImage->plane[plane];
    pPlane->bufferSize = minNewSize + 10000;  /* A GUESS - DO SOMETHING SMARTER !!!!! */

    if (!pPlane->pPixels) pPlane->pPixels = (ilPtr)IL_MALLOC (pPlane->bufferSize);
    else pPlane->pPixels = (ilPtr)IL_REALLOC (pPlane->pPixels, pPlane->bufferSize);
    if (!pPlane->pPixels) {
        pPlane->bufferSize = 0;
        return FALSE;
        }
    return TRUE;
}


    /*  ------------------------ ilCopyCompressedExecute ------------------------ */
    /*  Execute() function for ilInsertCompressedCopyFilter() to copy compressed images.
        Copies one strip of compressed data.
    */
static ilError ilCopyCompressedExecute (
    register ilExecuteData  *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
    register ilImagePlaneInfo *pSrcPlane, *pDstPlane;
    long                    nBytes, dstOffset, requiredBufferSize;

    nBytes = pData->compressed.nBytesToRead;        /* # of bytes to write */
    pSrcPlane = &pData->pSrcImage->plane[0];
    if (!pSrcPlane->pPixels || (nBytes <= 0))       /* nothing to copy; exit */
        return IL_OK;

    dstOffset = *pData->compressed.pDstOffset;      /* byte offset into dst buffer */
    pDstPlane = &pData->pDstImage->plane[0];
    requiredBufferSize = nBytes + dstOffset;        /* # bytes needed in dst buffer */

        /*  Check for space in output buffer; realloc/malloc if not enough */
    if (requiredBufferSize > pDstPlane->bufferSize) {
        pDstPlane->pPixels = (pDstPlane->pPixels) ? 
            (ilPtr)IL_REALLOC (pDstPlane->pPixels, requiredBufferSize) :
            (ilPtr)IL_MALLOC (requiredBufferSize);
        if (!pDstPlane->pPixels) {
            pDstPlane->bufferSize = 0;
            return IL_ERROR_MALLOC;
            }
        pDstPlane->bufferSize = requiredBufferSize;
        }

        /*  Copy nBytes from src to dst buffer, using offsets from *pData */
    bcopy (pSrcPlane->pPixels + pData->compressed.srcOffset, 
           pDstPlane->pPixels + dstOffset, nBytes);
    *pData->compressed.pNBytesWritten = nBytes;

    return IL_OK;
}


        /*  ------------------- ilInsertCompressedCopyFilter ------------------------- */
        /*  Insert a "filter" which copies the compressed pipe image to the dest image.
            This is the equivalent to ilInsertCopyFilter(), except for compressed images.
        */
IL_PRIVATE ilBool _ilInsertCompressedCopyFilter (
    ilPipe              pipe
    )
{
        /*  Add a filter which copies src to dst.  No private needed; the Execute()
            function does all necessary setup.
        */
    return (ilAddPipeElement (pipe, IL_FILTER, 0, 0, 
        (ilSrcElementData *)NULL, (ilDstElementData *)NULL, 
        IL_NPF, IL_NPF, IL_NPF, ilCopyCompressedExecute, 0)) ? TRUE : FALSE;

}

