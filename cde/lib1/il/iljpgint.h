/* $XConsortium: iljpgint.h /main/cde1_maint/1 1995/07/17 18:59:43 drk $ */
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

#ifndef ILJPGINT_H
#define ILJPGINT_H

#ifndef ILJPG_H
#include "iljpg.h"
#endif

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#ifdef NULL
#undef NULL
#endif

#define FALSE 0
#define TRUE 1
#define NULL 0

typedef int iljpgBool;

    /* JPEG zigzag scanning order (64 entries) */
    ILJPG_PRIVATE_EXTERN 
int _iljpgZigzagTable[];

    /*  Allocate _nBytes from heap and return a ptr to it. */
#ifndef ILJPG_MALLOC
#   define ILJPG_MALLOC(_nBytes)  (malloc (_nBytes))
#endif

    /*  Allocate _nBytes from heap, zero it, and return a ptr to it. */
#ifndef ILJPG_MALLOC_ZERO
#   define ILJPG_MALLOC_ZERO(_nBytes)  (calloc ((_nBytes), 1))
#endif

    /*  Free given block (*_ptr), allocated by ILJPG_MALLOC_ZERO(). */
#ifndef ILJPG_FREE
#   define ILJPG_FREE(_ptr)       (free (_ptr))
#endif

    /*  Validate the given parameter block and return true iff valid.
    */
    ILJPG_PRIVATE_EXTERN 
iljpgBool _iljpgValidPars (
    register iljpgDataPtr pData
    );

#endif
