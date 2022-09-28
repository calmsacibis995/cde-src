/* 
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */ 

#ifndef _SDtSpell_h
#define _SDtSpell_h

#pragma ident "@(#)Spell.h	1.9 96/06/19 SMI"

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* String defines for resources (we may want to move to a better place) */

#define XmCDictionary "Dictionary"
#define XmNdictionary "dictionary"

#define XmNsearchWidget "searchWidget"

#define XmNusePersonal	"usePersonal"
#define XmCUsePersonal	"UsePersonal"

#define XmNfollowLocale	"followLocale"
#define XmCFollowLocale	"FollowLocale"

#define XmNdefaultDictionary	"defaultDictionary"
#define XmCDefaultDictionary	"DefaultDictionary"

/* Class record constants */

externalref WidgetClass sdtSpellSelectionBoxWidgetClass;

typedef struct _SDtSpellSelectionBoxClassRec * SDtSpellSelectionBoxWidgetClass;
typedef struct _SDtSpellSelectionBoxRec      * SDtSpellSelectionBoxWidget;


#ifndef SDtIsSpellSelectionBox
#define SDtIsSpellSelectionBox(w) (XtIsSubclass((w),sdtSpellSelectionBoxWidgetClass))
#endif


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget SDtCreateSpellSelectionBox() ;
extern Widget SDtCreateSpellSelectionDialog() ;
extern Boolean SDtInvokeSpell();
extern Boolean	SDtSpellIsEngineAvail();

#else

extern Widget SDtCreateSpellSelectionBox( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;
extern Widget SDtCreateSpellSelectionDialog( 
                        Widget ds_p,
                        String name,
                        ArgList fsb_args,
                        Cardinal fsb_n) ;
extern Boolean SDtInvokeSpell(Widget textW);
extern Boolean	SDtSpellIsEngineAvail(Widget wid);

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _SDtSpell_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
