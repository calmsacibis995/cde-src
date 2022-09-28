/*
 * @(#)popup.h 1.3 96/03/07
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */


#ifndef _POPUP_H
#define _POPUP_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _NO_PROTO
void PositionPopup();
#else
void PositionPopup(Widget base, Widget popup);
#endif /* _NO_PROTO */

#ifdef _NO_PROTO
void PositionFrame();
#else
void PositionFrame(int x, int y, int w, int h, Widget popup);
#endif /* _NO_PROTO */


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _POPUP_H */

