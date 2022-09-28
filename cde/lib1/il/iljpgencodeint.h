/* $XConsortium: iljpgencodeint.h /main/cde1_maint/1 1995/07/17 18:58:22 drk $ */
/**---------------------------------------------------------------------
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

#ifndef ILJPGENCODEINT_H
#define ILJPGENCODEINT_H

#ifndef ILJPGENCODE_H
#include "iljpgencode.h"
#endif

#ifndef ILJPGINT_H
#include "iljpgint.h"
#endif

    /*  Private data for encoding JPEG (not JIF), e.g. 
        _iljpgEncodeInit/Execute/Cleanup().
    */

typedef struct {
    float          *pScale;                 /* from DCTScaleTables[] */
    int             horiFactor, vertFactor; /* copied from iljpgDataRec */
    long            width, height;          /* size of input buffer */
    int             mcuXInc, mcuYInc;       /* value to inc (x,y) by, each MCU */
    long            x, y;                   /* where to fetch next MCU */
    int             lastDC;                 /* DC from previous 8x8 block */
    } iljpgECompRec, *iljpgECompPtr;

typedef struct {
    iljpgDataPtr        pData;          /* data passed to _iljpgEncodeInit() */
    float              *DCTScaleTables[4];          /* owned by iljpgendct.c */
    iljpgPtr            pHuffPriv;                  /* owned by iljpgenhuff.c */
    iljpgECompRec       compData[ILJPG_MAX_COMPS];  /* owned by iljpgencode.c */
    } iljpgEncodePrivRec, *iljpgEncodePrivPtr;


    /*  Called by iljpgEncodeInit() to scale the Q table pointed to by "pSrc"
        into "pDst", each a 64 entry table.
    */
    ILJPG_PRIVATE_EXTERN
iljpgError _iljpgEnDCTScale (
    iljpgPtr            pSrc,
    float              *pDst
    );

    /*  DCT encode the 64 int matrix pointed to by "pSrc", storing the results back
        into the same matrix.  "pScale" must point to the 64 float scaled/clipped 
        Q table, as setup by _iljpgEnDCTInit().
    */
    ILJPG_PRIVATE_EXTERN
_iljpgEnDCT (
    int        *pSrc,
    float      *pScale
    );

    /*  Called by iljpgEncodeInit() to init for Huffman encoding.
    */
    ILJPG_PRIVATE_EXTERN
iljpgError _iljpgEnhuffInit (
    iljpgEncodePrivPtr  pPriv
    );

    /*  Called by iljpgEncodeCleanup() to cleanup after Huffman encoding.
    */
    ILJPG_PRIVATE_EXTERN
iljpgError _iljpgEnhuffCleanup (
    iljpgEncodePrivPtr  pPriv
    );

    /*  Huffman encode an 8x8 block (already zig-zagged) from "pSrc" (array of
        64 ints) out to "stream", for component index "comp".  The DC component
        (pSrc[0]) must have already had the lastDC value subtracted from it.
    */
    ILJPG_PRIVATE_EXTERN
iljpgError _iljpgEnhuffExecute (
    iljpgEncodePrivPtr  pPriv,
    int                 comp,
    int                *pSrc,
    ILJPG_ENCODE_STREAM stream
    );

    /*  Flush out any bits left over from Huffman encoding.
    */
    ILJPG_PRIVATE_EXTERN
iljpgError _iljpgEnhuffFlush (
    iljpgEncodePrivPtr  pPriv,
    ILJPG_ENCODE_STREAM stream
    );

#endif
