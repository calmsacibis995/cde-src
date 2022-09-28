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

#include "ilu.h"
#include "ilerrors.h"
#include "ilpipelem.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif


    /*  -------------------------- iluChecksumUncompressed ------------------------ */
    /*  Checksum all (if "nLines" is zero) or "nLines" of the given
        *uncompressed* image.
    */
static iluChecksum iluChecksumUncompressed (
    ilBool          doInit,         /* TRUE if first call for this image */
    ilImageInfo    *pInfo,          /* image info for image to checksum */
    long            startLine,      /* line to start at, from 0 */
    long            nLines,         /* # of lines, or if == 0, whole image */
    iluChecksum    *pChecksum       /* ptr to running checksum value */
    )
{
register iluChecksum    code, poly, accum;
register int            i;
int                     nPack, nPlanes, nBits, plane;
long                    nBytesPerLine, width;
register long           nBytes;
ilPtr                   pSrcLine;
register ilPtr          pSrc;
ilImageDes             *pDes;
ilImageFormat          *pFormat;
register ilByte         byte, maskLastByte;


    pDes = pInfo->pDes;
    pFormat = pInfo->pFormat;

    code = (doInit) ? 0 : *pChecksum;
    poly = 0x8408;

    if (pFormat->sampleOrder == IL_SAMPLE_PLANES) {
        nPack   = 1;
        nPlanes = pDes->nSamplesPerPixel;
        }
    else {
        nPlanes = 1;
        nPack   = pDes->nSamplesPerPixel;
        }
    if (nLines == 0)
        nLines = pInfo->height;         /* nLines == 0 implies whole image */

    for (plane = 0; plane < nPlanes; plane++) {
        long    nPlaneLines, planeStartLine;

        planeStartLine = startLine;
        nPlaneLines = nLines;
        width = pInfo->width;
        if (pDes->type == IL_YCBCR) {
            width /= pDes->typeInfo.YCbCr.sample[plane].subsampleHoriz;
            nPlaneLines /= pDes->typeInfo.YCbCr.sample[plane].subsampleVert;
            planeStartLine /= pDes->typeInfo.YCbCr.sample[plane].subsampleVert;
            }

        nBits = width * nPack * pFormat->nBitsPerSample[plane];
        nBytesPerLine = nBits >> 3;
        maskLastByte = (~0) << (7 - ((nBits - 1) % 8));

        pSrcLine = pInfo->plane[0].pPixels + 
                      planeStartLine * pInfo->plane[0].nBytesPerRow;
        while (nPlaneLines-- > 0) {
            pSrc = pSrcLine;
            pSrcLine += pInfo->plane[0].nBytesPerRow;
            nBytes = nBytesPerLine;

            while (nBytes > 0) {
                if (nBytes > 2) {               /* more than 2 bytes left */
                    accum = *pSrc++;
                    accum = (accum << 8) | (*pSrc++ & 0xff);
                    }
                else if (nBytes == 2) {         /* two bytes left */
                    accum = *pSrc++;
                    accum = (accum << 8) | (*pSrc++ & maskLastByte);
                    }
                else {                          /* one (last) byte left */
                    accum = *pSrc++ & maskLastByte;
                    accum = (accum << 8);
                    }
                nBytes -= 2;
                accum ^= code;
                for (i = 0; i < 16; i++) {
                    if (accum & 0x8000) {       /* do a left rotate */
                        accum = (accum << 1) | 0x0001;
                        accum ^= poly;
                        }
                    else accum <<= 1;
                    }
                code = accum;
                }   /* END while bytes */
            }       /* END one line */
        }           /* END one plane */

    *pChecksum = code;                          /* return new checksum value */
}


    /*  ------------------- iluChecksumCompressed ------------------------ */
    /*  Checksum the given part of a *compressed* image.
    */
static iluChecksum iluChecksumCompressed (
    ilBool          doInit,         /* TRUE if first call for this image */
    ilImageInfo    *pInfo,          /* image info for image to checksum */
    long            startOffset,    /* start byte offset into compressed data */
    long            nBytes,         /* # of bytes to checksum */
    iluChecksum    *pChecksum       /* ptr to running checksum value */
    )
{
register iluChecksum    code, poly, accum;
register int            i, j;
register ilPtr          pSrc;
ilByte                  byte;

    code = (doInit) ? 0 : *pChecksum;
    poly = 0x8408;
    j = 0;
    accum = 0;

    pSrc = pInfo->plane[0].pPixels + startOffset;
    while (nBytes-- > 0) {
        byte = *pSrc++;
        accum = (accum << 8) | (byte & 0xff);
        j++;
        if ((j & 1)==0) {
            accum ^= code;
            for (i = 0; i < 16; i++) {
                if (accum & 0x8000) {  /* do a left rotate */
                    accum = (accum << 1) | 0x0001;
                    accum ^= poly;
                    }
                else accum <<= 1;
                }
            code = accum;
            }
        }   /* END one byte */

    *pChecksum = code;                  /* return new checksum value */
}



    /*  ------------------------ Checksum Pipe Code ------------------------------- */

    /*  Private data for checksum filter. */
typedef struct {
    iluChecksum        *pChecksum;      /* ptr to area to return value */
    iluChecksum         checksum;       /* running checksum */
    ilBool              firstStrip;     /* true if first strip being executed */
    } iluChecksumPrivRec, *iluChecksumPrivPtr;

static ilError iluChecksumInit (
    iluChecksumPrivPtr  pPriv,
    ilImageInfo        *pSrcImage,
    ilImageInfo        *pDstImage
    )
{
    pPriv->firstStrip = TRUE;
    return IL_OK;
}

static ilError iluChecksumExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
iluChecksumPrivPtr          pPriv;

        /*  Checksum this strip */
    pPriv = (iluChecksumPrivPtr)pData->pPrivate;
    if (pData->pSrcImage->pDes->compression == IL_UNCOMPRESSED) {
        iluChecksumUncompressed (pPriv->firstStrip, pData->pSrcImage, 
            pData->srcLine, *pNLines, &pPriv->checksum);
        *pData->pNextSrcLine = pData->srcLine;
        }
    else {
        iluChecksumCompressed (pPriv->firstStrip, pData->pSrcImage, 
           pData->compressed.srcOffset, pData->compressed.nBytesToRead, 
           &pPriv->checksum);
        *pData->compressed.pDstOffset = pData->compressed.srcOffset;
        *pData->compressed.pNBytesWritten = pData->compressed.nBytesToRead;
        }

    *pPriv->pChecksum = pPriv->checksum;    /* return current checksum */
    pPriv->firstStrip = FALSE;
    return IL_OK;
}


    /*  ------------------------ iluChecksumPipe ------------------------- */
    /*  Adds a filter to the given pipe to calculate a checksum on all bits 
        flowing thru the pipe, and return the result when pipe done executing
        to "*pChecksum", which must point to a *static* iluChecksum.
        Returns true if successful, else false.
            If "forceDecompress" is true, then the image is decompressed before
        calculating the checksum.
    */
ilBool iluChecksumPipe (
    ilPipe              pipe,
    ilBool              forceDecompress,
    iluChecksum        *pChecksum
    )
{
iluChecksumPrivPtr      pPriv;
ilImageDes              des;
ilImageFormat           format;

        /*  Planar compression not supported - JPEG is special though */
    ilGetPipeInfo (pipe, forceDecompress, (ilPipeInfo *)NULL, &des, &format);
    if ((format.sampleOrder == IL_SAMPLE_PLANES)
     && (des.nSamplesPerPixel > 1)
     && (des.compression != IL_UNCOMPRESSED)
     && (des.compression != IL_JPEG))
        return ilDeclarePipeInvalid (pipe, IL_ERROR_NOT_IMPLEMENTED);

        /*  Add a filter to calc checksum - will get error if pipe not in 
            correct state, or if it is not a pipe, etc.
        */
    pPriv = (iluChecksumPrivPtr)ilAddPipeElement (pipe, IL_FILTER, 
        sizeof (iluChecksumPrivRec), IL_ADD_PIPE_NO_DST, 
        (ilSrcElementData *)NULL, (ilDstElementData *)NULL, 
        iluChecksumInit, IL_NPF, IL_NPF, iluChecksumExecute, 0);
    if (!pPriv)
        return FALSE;

    pPriv->pChecksum = pChecksum;
    pipe->context->error = IL_OK;
    return TRUE;
}



