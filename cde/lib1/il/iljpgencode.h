/* $XConsortium: iljpgencode.h /main/cde1_maint/1 1995/07/17 18:57:36 drk $ */
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



#ifndef ILJPGENCODE_H
#define ILJPGENCODE_H

#ifndef ILJPG_H
#include "iljpg.h"
#endif
#ifndef ILJPGENCODEDEFS_H
#include "iljpgencodedefs.h"
#endif

    /*  Optionally returned by iljpgEncodeJIF(): offsets to tables
        within the JIF encoded stream, as needed for writing JPEG to
        TIFF.  QTables[i] points to the 64 bytes of Q values for table
        "i"; DC/ACTables[] to the 16 bytes of code lengths.  All
        offsets are from the beginning of the JIF image.
    */
typedef struct {
    long                QTables[4];
    long                DCTables[4];
    long                ACTables[4];
    } iljpgJIFOffsetsRec, *iljpgJIFOffsetsPtr;


    /*  Default Q, AC and DC tables. */
ILJPG_PUBLIC_EXTERN iljpgByte iljpgLuminanceQTable[];
ILJPG_PUBLIC_EXTERN iljpgByte iljpgChrominanceQTable[];
ILJPG_PUBLIC_EXTERN iljpgByte iljpgLuminanceDCTable[];
ILJPG_PUBLIC_EXTERN iljpgByte iljpgChrominanceDCTable[];
ILJPG_PUBLIC_EXTERN iljpgByte iljpgLuminanceACTable[];
ILJPG_PUBLIC_EXTERN iljpgByte iljpgChrominanceACTable[];


    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgEncodeJIF (
    ILJPG_ENCODE_STREAM stream,
    iljpgDataPtr        pData,
    iljpgJIFOffsetsPtr  pOffsets
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgEncodeInit (
    iljpgDataPtr        pData,
    iljpgPtr           *pPrivate                /* RETURNED */
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgEncodeCleanup (
    iljpgPtr            pPrivate
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgEncodeExecute (
    iljpgPtr            pPrivate,
    ILJPG_ENCODE_STREAM stream,
    long                nSrcLines,
    iljpgPtr            pSrcPixels[],
    long                nSrcBytesPerRow[]
    );

#endif
