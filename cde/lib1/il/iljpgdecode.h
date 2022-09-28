/* $XConsortium: iljpgdecode.h /main/cde1_maint/1 1995/07/17 18:55:55 drk $ */
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



#ifndef ILJPGDECODE_H
#define ILJPGDECODE_H

#ifndef ILJPG_H
#include "iljpg.h"
#endif
#ifndef ILJPGDECODEDEFS_H
#include "iljpgdecodedefs.h"
#endif

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgDecodeJIF (
    ILJPG_DECODE_STREAM stream,
    iljpgDataPtr       *ppData                  /* RETURNED */
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgFreeData (
    iljpgDataPtr        pData
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgDecodeInit (
    iljpgDataPtr        pData,
    iljpgPtr           *pPrivate                /* RETURNED */
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgDecodeCleanup (
    iljpgPtr           pPrivate
    );

    ILJPG_PUBLIC_EXTERN 
iljpgError iljpgDecodeExecute (
    iljpgPtr            pPrivate,
    ILJPG_DECODE_STREAM stream,
    int                 doReset,
    long                nDstLines,
    iljpgPtr            pPixels[],
    long                nBytesPerRow[]
    );

#endif
