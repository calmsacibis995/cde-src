/* $XConsortium: ilcontext.h /main/cde1_maint/1 1995/07/17 18:41:20 drk $ */
#ifndef ILCONTEXT_H
#define ILCONTEXT_H

    /*  Defines what a context looks like internally.
    */

#ifndef ILINT_H
#include "ilint.h"
#endif

        /*  Internal view of the context, i.e. the public type "ilContext". */

        /*  Indices into ilContextRec.pAlloc */
#define IL_CONTEXT_ALLOC_EFS    0           /* owned by /ilc/ilefs.c */
#define IL_CONTEXT_MAX_ALLOC    4           /* max # of above pAlloc indices */

typedef struct {
    ilContextPublicRec  p;                  /* public view of context, /ilinc/il.h */

        /*  Owned fields - if you are accessing them outside of the named
            owning file, you're doing something wrong!
        */
    ilPtr               pImageData;         /* owned by /ilc/ilimage.c */
    unsigned int        privateType;        /* owned by /ilc/ilcontext.c */
    ilObjectRec         objectHead;         /* owned by /ilc/ilobject.c */
    ilPtr               pXData;             /* owned by /ilc/ilX.c */

        /*  Array of ptrs which if non-null are freed _after_ all objects destroyed. */
    ilPtr               pAlloc [IL_CONTEXT_MAX_ALLOC];
    } ilContextRec, *ilContextPtr;

        /*  Typecast the given ptr to an ilContextPtr. The given ptr should be an 
            ilContext; this macro does not check.
        */
#define IL_CONTEXT_PTR(_context) ((ilContextPtr)_context)

#endif
