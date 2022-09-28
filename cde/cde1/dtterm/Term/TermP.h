/*
 * $XConsortium: TermP.h /main/cde1_maint/1 1995/07/15 01:19:59 drk $";
 */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef	_Dt_TermP_h
#define	_Dt_TermP_h

#include "TermPrimP.h"
#include "Term.h"
#include "TermData.h"
#include "TermFunctionKey.h"

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

/* Vt class structure... */

typedef struct _DtTermClassPart
{
    int foo;
} DtTermClassPart;

/* full clas record declaration for Vt class... */
typedef struct _DtTermClassRec {
    CoreClassPart		core_class;
    XmPrimitiveClassPart	primitive_class;
    DtTermPrimitiveClassPart		term_primitive_class;
    DtTermClassPart		term_class;
} DtTermClassRec;

externalref DtTermClassRec dtTermClassRec;

/* vt instance record... */
typedef struct _DtTermPart
{
    DtTermData			td;	/* non-widget terminal data	*/
    Boolean                     autoWrap;
    Boolean                     reverseWrap;
    Boolean			wrapRightAfterInsert;
    Boolean                     sunFunctionKeys;
    Boolean                     c132;
    Boolean			appKeypadMode;
    Boolean			appCursorMode;
    Boolean			formFeedClearsScreen;
    Pixel			customColor[8];
    int				glyphGutter;
    void			*glyphFont;
} DtTermPart;

/* full instance record declaration... */

typedef struct _DtTermRec {
    CorePart		core;
    XmPrimitivePart	primitive;
    DtTermPrimitivePart	term;
    DtTermPart		vt;
} DtTermRec;

/* private function declarations... */
/* end private function declarations... */

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif	/* _Dt_TermP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
