/* $XConsortium: ilXycbcr.c /main/cde1_maint/1 1995/07/17 18:35:10 drk $ */
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
#include "ilXint.h"
#include <math.h>
#include "ilpipelem.h"
#include "ilerrors.h"

    /*  Number of bits of Y and Cb/Cr used to dither before a table lookup to
        convert to a dithered pixel.  The number of levels dithered to (nLY/CbCr) is
        1<<DYCBCR_NBITS_Y/CBCR.  The YCbCr->RGB lookup table is "nLY * nLCbCr * nLCbCr"
        longs in size.
    */
#define DYCBCR_NBITS_Y      6
#define DYCBCR_NBITS_CBCR   4

    /*  Tables generated by /il/util/gendithertables.c */
static unsigned int _il2x2DitherKernel[4] = {42, 234, 170, 106};

        /*  Private data for YCbCr to dithered pixel filter */
typedef struct {
    unsigned long   pixels[1 << (DYCBCR_NBITS_Y + DYCBCR_NBITS_CBCR + DYCBCR_NBITS_CBCR)];
    } ilDitherYCbCrPrivRec, *ilDitherYCbCrPrivPtr;


        /*  ---------------------- ilFastYCbCrDitherExecute ------------------------ */
        /*  Execute() function: dither from subsampled-by-2 YCbCr to 8 bit pixels.
        */
static ilError ilFastYCbCrDitherExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
ilDitherYCbCrPrivPtr        pPriv;
ilImagePlaneInfo           *pPlane;
long                        nLinesDiv2, halfWidth;
long                        CbRowBytes, CrRowBytes;
ilPtr                       pYLine, pCbLine, pCrLine, pDstLine;
register unsigned long     *pTable;
register long               YRowBytes, dstRowBytes, nPixelsDiv2, CbCr, temp0, temp1;
register ilPtr              pY, pCb, pCr, pDst;

        /*  This filter handles a pipe image of YCbCr subsampled by 2 in Cb/Cr only.
            The # of lines of Cb/Cr is therefore 1/2 the # of lines of Y.
            Note: can only handle images with even width/height; checked elsewhere.
        */
    pPlane = pData->pSrcImage->plane;
    YRowBytes = pPlane->nBytesPerRow;
    pYLine = pPlane->pPixels;
    pPlane++;
    CbRowBytes = pPlane->nBytesPerRow;
    pCbLine = pPlane->pPixels;
    pPlane++;
    CrRowBytes = pPlane->nBytesPerRow;
    pCrLine = pPlane->pPixels;
    if (pData->srcLine) {
        pYLine  += pData->srcLine * YRowBytes;
        pCbLine += (pData->srcLine >> 1) * CbRowBytes;
        pCrLine += (pData->srcLine >> 1) * CrRowBytes;
        }
    pPlane = pData->pDstImage->plane;
    dstRowBytes = pPlane->nBytesPerRow;
    pDstLine = pPlane->pPixels + dstLine * dstRowBytes;

    pPriv = (ilDitherYCbCrPrivPtr)pData->pPrivate;
    halfWidth = pData->pSrcImage->width >> 1;   /* 1/2 # of lines, pixels per line */
    nLinesDiv2 = *pNLines >> 1;
    pTable = pPriv->pixels;

        /*  Do 4 pixels at a time, using 4 Ys for each
            Cb,Cr pair: get Cb,Cr into upper bits of "CbCr", then add in each Y
            and use result to index into table that yields 4 dithered pixels:
            <left><right><below><below right>.
        */
    while (nLinesDiv2-- > 0) {
        pY = pYLine;
        pYLine += YRowBytes << 1;       /* skip 2 lines; read 2 lines each loop */
        pCb = pCbLine;
        pCbLine += CbRowBytes;
        pCr = pCrLine;
        pCrLine += CrRowBytes;
        pDst = pDstLine;
        pDstLine += dstRowBytes << 1;   /* skip 2 lines; write 2 lines each loop */
        nPixelsDiv2 = halfWidth;
        while (nPixelsDiv2-- > 0) {
            temp0 = *pCb++;
            CbCr = *pCr++;
            temp0 >>= (8 - DYCBCR_NBITS_CBCR);
            CbCr >>= (8 - DYCBCR_NBITS_CBCR);
            CbCr += temp0 << DYCBCR_NBITS_CBCR;
            CbCr <<= DYCBCR_NBITS_Y;    /* CbCr now in upper bits w/ room for Y */

            temp0 = pY[YRowBytes];      /* do pixel and pixel below */
            temp1 = *pY++;
            temp0 = CbCr + (temp0 >> (8-DYCBCR_NBITS_Y));
            pDst[dstRowBytes] = pTable[temp0] >> 8;
            temp1 = CbCr + (temp1 >> (8-DYCBCR_NBITS_Y));
            *pDst++ = pTable[temp1] >> 24;

            temp0 = pY[YRowBytes];      /* do pixel and pixel below */
            temp1 = *pY++;
            temp0 = CbCr + (temp0 >> (8-DYCBCR_NBITS_Y));
            pDst[dstRowBytes] = pTable[temp0];
            temp1 = CbCr + (temp1 >> (8-DYCBCR_NBITS_Y));
            *pDst++ = pTable[temp1] >> 16;
            }
        }

    return IL_OK;
}

        /*  -------------------- ilFastYCbCrDitherDoubleExecute ---------------------- */
        /*  Execute() function: dither and double the given # of src lines. 
        */
static ilError ilFastYCbCrDitherDoubleExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
ilDitherYCbCrPrivPtr        pPriv;
ilImagePlaneInfo           *pPlane;
long                        nLinesDiv2, halfWidth;
long                        CbRowBytes, CrRowBytes, dstRowBytes;
ilPtr                       pYLine, pCbLine, pCrLine;
register unsigned long     *pTable;
register long               YRowBytes, nPixelsDiv2, CbCr, temp0, temp1;
register long               dstRowShorts, dstRowShortsTimes2, dstRowShortsTimes3;
register ilPtr              pY, pCb, pCr;
register unsigned short    *pDst;
unsigned short             *pDstLine;

        /*  This filter handles a pipe image of YCbCr subsampled by 2 in Cb/Cr only.
            The # of lines of Cb/Cr is therefore 1/2 the # of lines of Y.
            Note: can only handle images with even width/height; checked elsewhere.
        */
    pPlane = pData->pSrcImage->plane;
    YRowBytes = pPlane->nBytesPerRow;
    pYLine = pPlane->pPixels;
    pPlane++;
    CbRowBytes = pPlane->nBytesPerRow;
    pCbLine = pPlane->pPixels;
    pPlane++;
    CrRowBytes = pPlane->nBytesPerRow;
    pCrLine = pPlane->pPixels;
    if (pData->srcLine) {
        pYLine  += pData->srcLine * YRowBytes;
        pCbLine += (pData->srcLine >> 1) * CbRowBytes;
        pCrLine += (pData->srcLine >> 1) * CrRowBytes;
        }

        /*  Access dst buffer as shorts */
    pPlane = pData->pDstImage->plane;
    dstRowBytes = pPlane->nBytesPerRow;
    pDstLine = (unsigned short *)(pPlane->pPixels + dstLine * dstRowBytes);
    dstRowShorts = dstRowBytes >> 1;
    dstRowShortsTimes2 = dstRowShorts * 2;
    dstRowShortsTimes3 = dstRowShorts * 3;

    pPriv = (ilDitherYCbCrPrivPtr)pData->pPrivate;
    halfWidth = pData->pSrcImage->width >> 1;   /* 1/2 # of lines, pixels per line */
    nLinesDiv2 = *pNLines >> 1;
    *pNLines <<= 1;                             /* write 2x # of src lines */
    pTable = pPriv->pixels;

        /*  Do 4 src pixels at a time, using 4 Ys for each Cb,Cr pair: get Cb,Cr into 
            upper bits of "CbCr", then add in each Y and use result to index into table 
            that yields 4 dithered pixels: <left><right><below><below right>.
            To double, output those 4 pixels as shown above; 1 src pixel becomes 4 dst.
        */
    while (nLinesDiv2-- > 0) {
        pY = pYLine;
        pYLine += YRowBytes << 1;       /* skip 2 lines; read 2 lines each loop */
        pCb = pCbLine;
        pCbLine += CbRowBytes;
        pCr = pCrLine;
        pCrLine += CrRowBytes;
        pDst = pDstLine;
        pDstLine += dstRowShorts << 2;   /* skip 4 lines; write 4 lines each loop */
        nPixelsDiv2 = halfWidth;
        while (nPixelsDiv2-- > 0) {
            temp0 = *pCb++;
            CbCr = *pCr++;
            temp0 >>= (8 - DYCBCR_NBITS_CBCR);
            CbCr >>= (8 - DYCBCR_NBITS_CBCR);
            CbCr += temp0 << DYCBCR_NBITS_CBCR;
            CbCr <<= DYCBCR_NBITS_Y;    /* CbCr now in upper bits w/ room for Y */

            temp0 = pY[YRowBytes];      /* do pixel and pixel below */
            temp1 = *pY++;
            temp0 = CbCr + (temp0 >> (8-DYCBCR_NBITS_Y));
            temp0 = pTable[temp0];
            pDst[dstRowShortsTimes3] = (unsigned short)temp0;
            pDst[dstRowShortsTimes2] = (unsigned short)(temp0 >> 16);
            temp1 = CbCr + (temp1 >> (8-DYCBCR_NBITS_Y));
            temp1 = pTable[temp1];
            pDst[dstRowShorts] = (unsigned short)temp1;
            *pDst++ = (unsigned short)(temp1 >> 16);

            temp0 = pY[YRowBytes];      /* do pixel and pixel below */
            temp1 = *pY++;
            temp0 = CbCr + (temp0 >> (8-DYCBCR_NBITS_Y));
            temp0 = pTable[temp0];
            pDst[dstRowShortsTimes3] = (unsigned short)temp0;
            pDst[dstRowShortsTimes2] = (unsigned short)(temp0 >> 16);
            temp1 = CbCr + (temp1 >> (8-DYCBCR_NBITS_Y));
            temp1 = pTable[temp1];
            pDst[dstRowShorts] = (unsigned short)temp1;
            *pDst++ = (unsigned short)(temp1 >> 16);
            }
        }

    return IL_OK;
}



    /*  ---------------------------- ilSetupYCbCrDitherTables ----------------------- */
    /*  Setup *pTable as necessary for the YCbCr to RGB (dither) conversion.
    */
static void ilSetupYCbCrDitherTables (
    register ilYCbCrInfo   *pYCbCr,
    int                     nBitsR,
    int                     nBitsG,
    int                     nBitsB,
    ilPtr                   pMapPixels,
    unsigned long          *pTable
    )
{
int                         nTableEntries, nLevelsY, nLevelsCbCr;
int                         Y, Cb, Cr, refY, refCb, refCr;
register int                R, G, B, pixel, i, temp, kernel;
double                      Lr, Lg, Lb;
double                      ditherR, ditherG, ditherB, ditherY, ditherCbCr;

#define CVT_TO_RGB { \
                int tY = Y - pYCbCr->sample[0].refBlack;\
                int tCb = Cb - pYCbCr->sample[1].refBlack;\
                int tCr = Cr - pYCbCr->sample[2].refBlack;\
                R = tCr * (2 - 2 * Lr) + tY;\
                B = tCb * (2 - 2 * Lb) + tY;\
                G = (tY - Lb * B - Lr * R) / Lg;\
                if (R < 0) R = 0; else if (R > 255) R = 255;\
                if (G < 0) G = 0; else if (G > 255) G = 255;\
                if (B < 0) B = 0; else if (B > 255) B = 255;\
                }

#define YCBCR_DEBUG_TABLE 1
#ifdef YCBCR_DEBUG_TABLE
        /*  Set pMapPixels to identity, for debugging */
    ilByte  debugMapPixels[256];
    int     tmp;
    int     doDebug = FALSE;

    if (doDebug) {
        for (tmp = 0; tmp < 256; tmp++)
            debugMapPixels[tmp] = tmp;
        pMapPixels = debugMapPixels;
        }
#endif

        /*  Build the YCbCr to dither-RGB lookup table.  The table is indexed by
            a YCbCr value <Cb><Cr><Y>, where <Y> is "DYCBCR_NBITS_Y" bits
            in size and <Cb><Cr> are each "DYCBCR_NBITS_CBCR" in size.  Indexed that way
            the table yields either four dithered pixels or a long 24 X pixel.
                Each table entry is calculated by converting the YCbCr value to 
            its RGB equivalent and either dithering to form 4 pixels or a single
            long X pixel.
        */
    nLevelsY = 1 << DYCBCR_NBITS_Y;
    nLevelsCbCr = 1 << DYCBCR_NBITS_CBCR;

    Lr = ((double)pYCbCr->lumaRed / (double)10000);
    Lg = ((double)pYCbCr->lumaGreen / (double)10000);
    Lb = ((double)pYCbCr->lumaBlue / (double)10000);

    ditherR = ((1 << nBitsR) - 1) * (double)256 / (double)255;
    ditherG = ((1 << nBitsG) - 1) * (double)256 / (double)255;
    ditherB = ((1 << nBitsB) - 1) * (double)256 / (double)255;

    for (refCb = 0; refCb < nLevelsCbCr; refCb++) {
        Cb = (255 * refCb) / (nLevelsCbCr - 1);

        for (refCr = 0; refCr < nLevelsCbCr; refCr++) {
            Cr = (255 * refCr) / (nLevelsCbCr - 1);

            for (refY = 0; refY < nLevelsY; refY++) {
                Y = (255 * refY) / (nLevelsY - 1);

                    /*  Store 4 dithered pixels, the result of dithering RGB with
                        a 2x2 kernel.  See /ilc/ildither.c for equivalent code.
                    */
#define DITHER
#ifdef COLORSLAM
                CVT_TO_RGB
                pixel = R >> (8 - nBitsR);
                pixel <<= nBitsG;
                pixel |= G >> (8 - nBitsG);
                pixel <<= nBitsB;
                pixel |= B >> (8 - nBitsB);
                pixel = pMapPixels[pixel];
                *pTable++ = (pixel << 24) | (pixel << 16) | (pixel << 8) | pixel;
#else
#ifdef DIFFUSION
{
                int temp1, temp2;
                int halfR = (1 << (7 - nBitsR)) - 1;
                int halfG = (1 << (7 - nBitsG)) - 1;
                int halfB = (1 << (7 - nBitsB)) - 1;
                int maskR = ~((1 << (8 - nBitsR)) - 1);
                int maskG = ~((1 << (8 - nBitsG)) - 1);
                int maskB = ~((1 << (8 - nBitsB)) - 1);
                CVT_TO_RGB
                for (i = 0, pixel = 0; i < 4; i++) {
                    temp1 = R + halfR;
                    if (temp1 > 255) temp1 = 255; else if (temp1 < 0) temp1 = 0;
                    R += R - (temp1 & maskR);
                    temp = temp1 >> (8 - nBitsR);

                    temp1 = G + halfG;
                    if (temp1 > 255) temp1 = 255; else if (temp1 < 0) temp1 = 0;
                    G += G - (temp1 & maskG);
                    temp <<= nBitsG;
                    temp |= temp1 >> (8 - nBitsG);

                    temp1 = B + halfB;
                    if (temp1 > 255) temp1 = 255; else if (temp1 < 0) temp1 = 0;
                    B += B - (temp1 & maskB);
                    temp <<= nBitsB;
                    temp |= temp1 >> (8 - nBitsB);

                    pixel <<= 8;
                    pixel |= pMapPixels[temp];
                    }
                *pTable++ = pixel;
}
#else
#ifdef DITHER
                CVT_TO_RGB
                R = (double)R * ditherR;
                G = (double)G * ditherG;
                B = (double)B * ditherB;

                for (i = 0, pixel = 0; i < 4; i++) {
                    kernel = _il2x2DitherKernel[i];
                    temp = (R + kernel) >> 8;
                    temp <<= nBitsG;
                    temp |= (G + kernel) >> 8;
                    temp <<= nBitsB;
                    temp |= (B + kernel) >> 8;
                    pixel <<= 8;
                    pixel |= pMapPixels[temp];
                    }
                *pTable++ = pixel;
#endif
#endif
#endif
                }
            }
        }
}

        /*  ------------------------ _ilFastYCbCrDither ---------------------------- */
        /*  Called by ilConvertForXWrite() to do fast dithered display of YCbCr.
            The pipe image must be planar, subsampled by 2 in Cb/Cr, 1 in Y.
            "pMapPixels" points to 256 bytes from the XWC map image, which are folded
            into the lookup table used for this function.
        */
IL_PRIVATE ilBool _ilFastYCbCrDither (
    ilPipe                  pipe,
    ilImageDes             *pDes,
    ilPipeInfo             *pInfo,
    int                     nBitsRed,
    int                     nBitsGreen,
    int                     nBitsBlue,
    ilBool                  doDouble,
    ilPtr                   pMapPixels
    )
{
ilImageDes                  des;
ilDitherYCbCrPrivPtr        pPriv;
ilDstElementData            dstData;
ilSrcElementData            srcData;

        /*  Add a filter to convert subsampled YCbCr to a dithered palette image.
            Force a single strip; image is at least 2 by 2 
        */
    srcData.consumerImage = (ilObject)NULL;
    srcData.stripHeight = pInfo->height;
    srcData.constantStrip = TRUE;
    srcData.minBufferHeight = 0;

    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = IL_DES_GRAY;             /* arbitrary; not really used */
    dstData.pFormat = IL_FORMAT_BYTE;
    dstData.width = pInfo->width;
    dstData.height = pInfo->height;
    dstData.stripHeight = srcData.stripHeight;
    if (doDouble) {
        dstData.width *= 2;
        dstData.height *= 2;
        dstData.stripHeight *= 2;
        }
    dstData.constantStrip = pInfo->constantStrip;
    dstData.pPalette = (unsigned short *)NULL;

    pPriv = (ilDitherYCbCrPrivPtr)ilAddPipeElement (pipe, IL_FILTER, 
            sizeof(ilDitherYCbCrPrivRec), 0, &srcData, &dstData, IL_NPF, IL_NPF, IL_NPF, 
            (doDouble) ? ilFastYCbCrDitherDoubleExecute : ilFastYCbCrDitherExecute, 0);
    if (!pPriv)
        return FALSE;

        /*  Init the table in private */
    ilSetupYCbCrDitherTables (&pDes->typeInfo.YCbCr, nBitsRed, nBitsGreen, nBitsBlue,
                              pMapPixels, pPriv->pixels);

    return TRUE;
}
