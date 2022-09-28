/* $XConsortium: ilutiljpeg.h /main/cde1_maint/2 1995/10/08 21:12:22 pascale $ */
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

#ifndef ILUTILJPEG_H
#define ILUTILJPEG_H

        /*  IL internal defs and functions for JPEG data.
            Exposes functions in ilutiljpeg.c .
        */

#ifndef IL_H
#include "il.h"
#endif
#ifndef ILJPGDECODE_H
#include "iljpgdecode.h"
#endif

    /*  Copy general data (but *not* table info) from IL data into 
        IL JPG package format.
    */
IL_EXTERN void _ilJPEGDataIn (
    const ilImageDes       *pDes,
    long                    width,
    long                    height,
    iljpgDataPtr            pDst
    );

    /*  Copy table info from IL format into IL JPG package format.
        Note: the restartInterval is also copied.
    */
IL_EXTERN void _ilJPEGTablesIn (
    ilJPEGData             *pSrc,
    iljpgDataPtr            pDst
    );

    /*  Free the tables in the given IL JPEG data block, which is not freed.
    */
IL_EXTERN void _ilJPEGFreeTables (
    ilJPEGData             *pData
    );

    /*  Copy data from pSrc to pDst, mallocing and copying contents of tables.
        If an error, free all malloc'd tables in dest, null them and return false.
    */
IL_EXTERN ilBool _ilJPEGCopyData (
    ilJPEGData             *pSrc,
    ilJPEGData             *pDst
    );

#endif
