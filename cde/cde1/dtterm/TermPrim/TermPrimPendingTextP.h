/* 
** $XConsortium: TermPrimPendingTextP.h /main/cde1_maint/1 1995/07/15 01:31:02 drk $
*/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef   _Dt_TermPrimPendingTextP_h
#define   _Dt_TermPrimPendingTextP_h

#include "TermPrimPendingText.h"

#ifdef	   __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define DEFAULT_CHUNK_BUF_SIZE  1024

#define	TextIsPending(list)	(list->head->next != list->tail)

typedef struct _PendingTextChunkRec
{
    unsigned char      *buffer;         /* beginning of buffer             */
    int                 buffLen;        /* length of buffer                */
    unsigned char      *bufPtr;         /* text remaining to be processed  */
    int                 len;            /* bytes remaining to be processed */
    PendingTextChunk    next;           /* next chunk in list              */
    PendingTextChunk    prev;           /* prev chunk in list              */
} PendingTextChunkRec;

typedef struct _PendingTextRec
{
    PendingTextChunk    head;
    PendingTextChunk    tail;
#ifdef   RECYCLE_CHUNKS
    PendingTextChunk    free;
#endif /* RECYCLE_CHUNKS */
} PendingTextRec;
    
#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif /* _Dt_TermPrimPendingTextP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
