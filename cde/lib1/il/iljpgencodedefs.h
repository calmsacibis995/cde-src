/* $XConsortium: iljpgencodedefs.h /main/cde1_maint/1 1995/07/17 18:58:04 drk $ */
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


#ifndef ILJPGENCODEDEFS_H
#define ILJPGENCODEDEFS_H

#include "ilint.h"
#include "ilerrors.h"

    /*  Equate ILJPG errors to IL errors */
#define ILJPG_ERROR_ENCODE_MALLOC       IL_ERROR_MALLOC
#define ILJPG_ERROR_ENCODE_PARS         IL_ERROR_COMPRESSED_DATA
#define ILJPG_ERROR_ENCODE_DCAC_TABLE   IL_ERROR_COMPRESSED_DATA
#define ILJPG_ERROR_ENCODE_Q_TABLE      IL_ERROR_COMPRESSED_DATA


    /*  Define a JPEG stream: "pBuffer" points to a buffer in
        memory, "pPastEndBuffer" points to byte just past end
        (buffer is of size "pPastEndBuffer" - "pBuffer") and
        pDst points to the place to store the next byte.
    */
typedef struct {
    ilPtr           pBuffer;
    ilPtr           pDst;
    ilPtr           pPastEndBuffer;
    } ilJPEGEncodeStream, *ilJPEGEncodeStreamPtr;

#define ILJPG_ENCODE_STREAM ilJPEGEncodeStreamPtr


    /*  Called by ILJPG_ENCODE_PUT_BYTE() macro to reallocate the buffer 
        defined by "pStream" so that there is space for *exactly* "nBytes" bytes
        to be written.  Zero (0) is returned if success else an error code.
    */
IL_EXTERN iljpgError _ilReallocJPEGEncode (
    ilJPEGEncodeStreamPtr   pStream,
    long                    nBytes
    );


    /*  # of bytes by which to expand the size of the output buffer
        when realloc'ing to write one byte.  NOTE: must be >= 1!
    */
#define ILJPG_REALLOC_EXTRA   1024

    /*  Macro to put a byte to a stream.  Return one (1) from macro
        if success, else return zero (0) and set "_error" to a
        non-zero error code.
    */
#define ILJPG_ENCODE_PUT_BYTE(_stream, _byte, _error) (                 \
    ((_stream)->pDst >= (_stream)->pPastEndBuffer) ?                    \
        ((_error = _ilReallocJPEGEncode (_stream, ILJPG_REALLOC_EXTRA)) ? 0 : \
            (*(_stream)->pDst++ = _byte, 1)) :                          \
        (*(_stream)->pDst++ = _byte, 1) )

    /*  Macro to return position within dst buffer, like ftell() */
#define ILJPG_ENCODE_OFFSET(_stream) \
    ((_stream)->pDst - (_stream)->pBuffer)

#endif

