/* $XConsortium: OpenFile.h /main/cde1_maint/1 1995/07/17 20:27:16 drk $ */
/******************************************************************************
       OpenFile.h
       This header file is included by OpenFile.c

******************************************************************************/

#ifndef _OPENFILE_H_INCLUDED
#define _OPENFILE_H_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/FileSB.h>

/******************************************************************************
       The definition of the context structure:
       If you create multiple copies of your interface, the context
       structure ensures that your callbacks use the variables for the
       correct copy.

       For each swidget in the interface, each argument to the Interface
       function, and each variable in the Interface Specific section of the
       Declarations Editor, there is an entry in the context structure.
       and a #define.  The #define makes the variable name refer to the
       corresponding entry in the context structure.
******************************************************************************/

typedef struct
{
        Widget  UxOpenFile;
} _UxCOpenFile;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCOpenFile            *UxOpenFileContext;
#define OpenFile                UxOpenFileContext->UxOpenFile

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

#ifdef _NO_PROTO

Widget  create_OpenFile();

#else /* _NO_PROTO */

Widget  create_OpenFile(void);

#endif /* _NO_PROTO */

#endif  /* _OPENFILE_H_INCLUDED */
