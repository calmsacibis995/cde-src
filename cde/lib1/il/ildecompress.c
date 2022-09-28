/* $XConsortium: ildecompress.c /main/cde1_maint/1 1995/07/17 18:46:08 drk $ */
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
#include "ilpipelem.h"
#include "ildecomp.h"
#include "ilcodec.h"
#include "ilerrors.h"

/* =================================================================================    
 
           -------------------- ilDecompress() --------------------
   Add a filter to "pipe" (guaranteed to be a pipe in IL_PIPE_FORMING state)
   which decompresses the compressed pipe image.

   ================================================================================= */

IL_PRIVATE void _ilDecompress (
    ilPipe              pipe
    )
{
    unsigned int        state;
    ilPipeInfo          info;                              
    ilImageDes          imdes;
    ilImageFormat       imformat;
     

    /* Get ptr to pipe info and check state.  We should NOT FORCE decompression */
    state =  ilGetPipeInfo(pipe, FALSE, &info, &imdes, &imformat);                    

    /* Verify status of the pipe */
    if (state != IL_PIPE_FORMING) {
        if (!pipe->context->error) {
            ilDeclarePipeInvalid(pipe, IL_ERROR_PIPE_STATE);
            return;
        }
    }                          

    /* Call the appropriate decompression function */
    switch (imdes.compression) {
            /* Image is already decompressed!  Continue, this is NOT an error! */
        case IL_UNCOMPRESSED: 
                break;

        case IL_G3:
                if (!_ilDecompG3 (pipe, &info, &imdes))
                    return;
                break;                       

        case IL_G4:           /* Currently supported CCITT Group4 decompression format */ 
                if (!_ilDecompG4 (pipe, &info, &imdes))                      
                    return;                                                               
                break;                                                                    

        case IL_LZW:          /* Currently supported LZW format */
                if (!_ilDecompLZW (pipe, &info, &imdes, &imformat))
                    return;
                break;                       

        case IL_PACKBITS:
                if (!_ilDecompPackbits (pipe, &info, &imdes, &imformat))
                    return;
                break;                         

        case IL_JPEG:         /* Currently supported JPEG format */
                if (!_ilDecompJPEG (pipe, &info, &imdes))
                    return;
                break;
        default:              /* imdes.compression is not valid       */
                ilDeclarePipeInvalid (pipe, IL_ERROR_COMPRESSION);
                return;

    }  /* end switch */

    pipe->context->error = IL_OK;
}


