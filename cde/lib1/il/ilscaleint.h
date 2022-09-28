/* $XConsortium: ilscaleint.h /main/cde1_maint/1 1995/07/17 19:04:36 drk $ */
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

#ifndef ILSCALEINT_H
#define ILSCALEINT_H

     /* PRIVATE definitions shared between scaling code, e.g. /ilc/ilscale.c .
        NO OTHER CODE SHOULD INCLUDE THIS FILE !
     */


        /*  In /ilc/ilbigray.c : */

    /*  Called by ilScale().
        Adds an element to "pipe" to scale the pipe image to "dstWidth" by "dstHeight"
        which both must be > 0.  The input pipe image must be a bitonal,
        bit-per-pixel image, with the given "blackIsZero".  "pInfo" points to the
        pipe info, "dstWidth/Height" is the (> 0) size to scale to, and nGrayLevels
        is the number of levels of gray.  The output image is a gray image.
            Only a scale down in both directions is allowed; the following must be true:
        srcWidth must be >= dstWidth and srcHeight must be >= dstHeight!
    */
IL_EXTERN void _ilScaleBitonalToGray (
    ilPipe              pipe,
    unsigned long       dstWidth,
    unsigned long       dstHeight,
    unsigned long       nGrayLevels,
    ilBool              blackIsZero,
    ilPipeInfo         *pInfo
    );

    /*  Called by ilScale().
        Adds an element to "pipe" to scale the pipe image to "dstWidth" by "dstHeight"
        which both must be > 0.  The input pipe image must be a bitonal,
        bit-per-pixel image, with _any_ "blackIsZero".  The resulting image is of the 
        same type. "pInfo" points to the pipe info, "dstWidth/Height" is the (> 0) size 
        to scale to.
            Only a scale down in both directions is allowed; the following must be true:
        srcWidth must be >= dstWidth and srcHeight must be >= dstHeight!
            Logically the same as ilScaleBitonalToGray(): a gray byte is produced for
        each destination pixel, using 256 levels of gray.  If that byte is 
        >= "*pGrayThreshold"; a white pixel is written; else a black pixel is written.
            NOTE: the value at "*pGrayThreshold" is read each time the pipe is executed
        (during Init()); the caller change its value and get different results without
        recreating the pipe.  If "pGrayThreshold" is NULL, a default value is used.
    */
IL_EXTERN void _ilAreaScaleBitonal (
    ilPipe              pipe,
    unsigned long       dstWidth,
    unsigned long       dstHeight,
    int                *pGrayThreshold,
    ilPipeInfo         *pInfo
    );

IL_EXTERN void _ilScaleSample (
    ilPipe              pipe,
    int                 Ncomponents,
    unsigned long       dstWidth,
    unsigned long       dstHeight,
    unsigned long       nLevels,
    ilBool              blackIsZero,
    ilPipeInfo         *pInfo
    );



#endif
