/* $XConsortium: ilpipelem.h /main/cde1_maint/2 1995/10/08 21:07:55 pascale $ */
/**---------------------------------------------------------------------
***	
***    file:           ilpipelem.h
***
***    description:    Definitions for user-defined and internal pipe elements
***	
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



#ifndef ILPIPELEM_H
#define ILPIPELEM_H

#ifndef ILINT_H
/* for size_t declaration */
#include <stddef.h>
#endif

#ifndef IL_H
#include "il.h"
#endif

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

#ifndef NeedWidePrototypes
#if defined(NARROWPROTO)
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1      /* default to make interropt. easier */
#endif
#endif

#ifdef __cplusplus               /* do not leave open across includes */
extern "C" {                                    /* for C++ V2.0 */
#endif


        /*  ilAddPipeElement (elementType) */
#define IL_PRODUCER         0
#define IL_CONSUMER         1
#define IL_FILTER           2
#define IL_HOOK             3

        /*  ilAddPipeElement (flags) */
#define IL_ADD_PIPE_NO_DST      (1<<0)
#define IL_ADD_PIPE_HOLD_SRC    (1<<1)
#define IL_ADD_PIPE_HOLD_DST    (1<<2)

        /*  ilAddPipeElement (null function pointer) */
#define IL_NPF                  ((ilError (*)())0)

typedef struct {
    ilObject            producerObject;
    ilBool              tempImage;
    long                width, height;
    ilBool              constantStrip;
    long                stripHeight;
    long                recommendedStripHeight;
    unsigned short     *pPalette;
    ilPtr               pCompData;
    long                filler [8];
    } ilPipeInfo;

typedef struct {
    ilPtr               pPrivate;
    ilImageInfo        *pSrcImage;
    ilImageInfo        *pDstImage;
    long                srcLine;
    long               *pNextSrcLine;
    struct {
        long            srcOffset;
        long            nBytesToRead;
        long           *pDstOffset;
        long           *pNBytesWritten;
        } compressed;
    long                filler [8];
    } ilExecuteData;

typedef struct {
    ilObject            consumerImage;
    long                stripHeight;
    ilBool              constantStrip;
    long                minBufferHeight;
    } ilSrcElementData;

typedef struct {
    ilObject            producerObject;
    const ilImageDes    *pDes;
    const ilImageFormat *pFormat;
    long                width, height;
    long                stripHeight;
    ilBool              constantStrip;
    unsigned short     *pPalette;
    ilPtr               pCompData;
    } ilDstElementData;


extern ilBool ilDeclarePipeInvalid (
#if NeedFunctionPrototypes
    ilPipe              pipe,
    ilError             error
#endif
    );

extern unsigned int ilGetPipeInfo (
#if NeedFunctionPrototypes
    ilPipe              pipe,
    ilBool              forceDecompress,
    ilPipeInfo         *pInfo,              /* RETURNED */
    ilImageDes         *pDes,               /* RETURNED */
    ilImageFormat      *pFormat             /* RETURNED */
#endif
    );

extern void ilGetBytesPerRow (
#if NeedFunctionPrototypes
    const ilImageDes    *pDes,
    const ilImageFormat *pFormat,
    long                width,
    long               *pnBytesPerRow       /* RETURNED */
#endif
    );

extern long ilRecommendedStripHeight (
#if NeedFunctionPrototypes
    const ilImageDes    *pDes,
    const ilImageFormat *pFormat,
    long                width,
    long                height
#endif
    );

extern ilPtr ilAddPipeElement (
#if NeedFunctionPrototypes
    ilPipe              pipe,
    int                 elementType,
/* Use portable type for sizeof() operator, bug report OSF_QAR# 32082 */
    size_t              nBytesPrivate,
    unsigned long       flags,
    ilSrcElementData   *pSrcData,
    ilDstElementData   *pDstData,
    ilError           (*Init)(),
    ilError           (*Cleanup)(),
    ilError           (*Destroy)(),
    ilError           (*Execute)(),
    unsigned long       mustBeZero
#endif
    );

#ifdef __cplusplus
}                                /* for C++ V2.0 */
#endif

#endif
