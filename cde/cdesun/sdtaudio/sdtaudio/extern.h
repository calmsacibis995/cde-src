/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

/*
 * Contains all the 'extern' globals
 */

#ifndef EXTERN_H

extern nl_catd		msgCatalog;
extern Widget		appShell;
extern XtAppContext	appContext;
extern char	       *programName;

extern char	       *appnameString;
extern XtCallbackRec	DropTransferCbList[];

extern Atom		WM_DELETE_WINDOW;
extern Atom		WM_SAVE_YOURSELF;

extern int 		ww_font_height;
extern XFontStruct     *ww_font;


#endif /* EXTERN_H */
