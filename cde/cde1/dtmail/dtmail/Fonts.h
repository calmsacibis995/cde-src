/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef FONTS_H
#define FONTS_H
 
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Fonts.h	1.2 05/25/95"
#endif

typedef struct _FontType {
        XmFontType      cf_type;
        union {
                XFontStruct     *cf_font;
                XFontSet         cf_fontset;
        } f;
} FontType;

extern Boolean
fontlist_to_font(
        XmFontList       font_list,
        FontType        *new_font);

extern void
load_app_font(
        Widget          w,
        FontType        *userfont,
        char		**fontname);

#endif /* FONTS_H */
