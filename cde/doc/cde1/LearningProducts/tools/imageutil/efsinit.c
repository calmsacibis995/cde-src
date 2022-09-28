/**---------------------------------------------------------------------
***	
***    file:           efsinit.c
***
***    description:    Image Library (IL) EFS File Init() function
***
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



    /*  Version string for 'what' command.  PLEASE UPDATE! */

static char efsVersionString[] = 
    "@(#)HP IL EFS v 2.1  bl03; Image EFS Library for HP-UX 8.07+ & MPower 'libilefs.sl'";



#include "il.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

    /*  TIFF support in efstiff.c  */
extern ilBool efsInitTIFF (
    ilContext               context
    );

    /*  TIFF support in efsgif.c  */
extern ilBool efsInitGIF (
    ilContext               context
    );

    /*  JFIF support in efsjfif.c  */
extern ilBool efsInitJFIF (
    ilContext               context
    );


        /*  ------------------- _ilefsInitStandardFiles ---------------------- */
        /*  Called by the IL when EFS function is called.  Calls the individual
            Init() function for each file type to be supported by EFS.
        */
ilBool _ilefsInitStandardFiles (
    ilContext               context
    )
{

        /*  Call the Init() functions; return error if any of them do */
    if (!_ilefsInitTIFF (context))
        return FALSE;
    if (!_ilefsInitGIF (context))
        return FALSE;
    if (!_ilefsInitJFIF (context))
        return FALSE;
    if (!_ilefsInitXBM (context))
        return FALSE;
    if (!_ilefsInitXWD (context))
        return FALSE;
    if (!_ilefsInitxpm (context))
        return FALSE;
    if (!_ilefsInitSTARBF(context))
        return FALSE;

        /*  Calls to Init() for New file format should be added here. */

    context->error = IL_OK;
    return TRUE;
}
