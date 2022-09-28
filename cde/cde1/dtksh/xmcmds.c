/* $XConsortium: xmcmds.c /main/cde1_maint/4 1995/10/14 01:41:13 montyb $ */
/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF          */
/*	UNIX System Laboratories, Inc.			        */
/*	The copyright notice above does not evidence any        */
/*	actual or intended publication of such source code.     */

#include	"shell.h" 
#include <signal.h>
#include <malloc.h>
#include <fcntl.h>
#include <nl_types.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/StringDefs.h>
#include <Xm/XmStrDefs.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include "hash.h"
#include "stdio.h"
#define NO_AST
#include "dtksh.h"
#undef NO_AST
#include "xmksh.h"
#include "dtkcmds.h"
#include "XtCvtrs.h"
#include "widget.h"
#include "dtkcvt.h"
#include "xmcmds.h"
#include "xmcvt.h"
#include "msgs.h"
#include "extra.h"
#include "xmwidgets.h"

#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/ArrowB.h>
#include <Xm/ArrowBG.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Command.h>
#include <Xm/CommandP.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/FileSB.h>
#include <Xm/FileSBP.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ListP.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/SelectioBP.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include <Dt/HelpDialog.h>
#include <Dt/HelpQuickD.h>


#ifdef _NO_PROTO

static int _xmcreatefunc() ;
static wtab_t * verifyListWidget() ;
static int List_ItemAndPos() ;
static int List_ItemListAndPos() ;
static int List_WidgetOnly() ;
static int GetSelectedPosList();
static int List_ItemOnly() ;
static int List_PositionOnly() ;
static int ListSelectItem() ;
static int GetMainWindowSeparator() ;
static int CatchAndIgnoreXError() ;
static int AddOrDeleteWMProtocols() ;
static int GetToggleState() ;
static int SetToggleState() ;
static wtab_t * verifyTextWidget() ;
static int Text_Widget() ;
static int Text_VarAndWidget() ;
static int Text_WidgetAndBoolean() ;
static int Text_WidgetAndOneParam() ;
static void GetSelBoxItemCount();
static void GetFileSelBoxFileItemCount();
static void GetFileSelBoxDirItemCount();
static wtab_t * ConvertWidgetToWtab();
static int GetSubWidget() ;

#else

static int _xmcreatefunc( 
                        Widget (*func)(),
                        char *wclass,
                        int argc,
                        char *argv[]) ;
static wtab_t * verifyListWidget( 
                        char *cmd,
                        char *widget) ;
static int List_ItemAndPos( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int List_ItemListAndPos( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int List_WidgetOnly( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int GetSelectedPosList(
	                Boolean (*func)(),
	                int paramCount,
	                char * errmsg,
                        int argc,
                        char *argv[] ) ;
static int List_ItemOnly( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int List_PositionOnly( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int ListSelectItem( 
                        Boolean usePosition,
                        int argc,
                        char *argv[]) ;
static int GetMainWindowSeparator( 
                        int childType,
                        int argc,
                        char *argv[]) ;
static int CatchAndIgnoreXError( 
                        Display *display,
                        XEvent *event) ;
static int AddOrDeleteWMProtocols( 
                        int argc,
                        char *argv[]) ;
static int GetToggleState( 
                        int argc,
                        char *argv[]) ;
static int SetToggleState( 
                        int argc,
                        char *argv[]) ;
static wtab_t * verifyTextWidget( 
                        char *cmd,
                        char *widget) ;
static int Text_Widget( 
                        Boolean (*func)(),
                        Boolean returnBoolean,
                        int argc,
                        char *argv[]) ;
static int Text_VarAndWidget( 
                        int (*func)(),
                        Boolean varIsString,
                        int argc,
                        char *argv[]) ;
static int Text_WidgetAndBoolean( 
                        void (*func)(),
                        int argc,
                        char *argv[]) ;
static int Text_WidgetAndOneParam( 
                        Boolean (*func)(),
                        Boolean returnBoolean,
                        Boolean paramIsString,
                        char *usageMsg,
                        int argc,
                        char *argv[]) ;
static void GetSelBoxItemCount(
                        Widget w,
			Cardinal * size,
			XrmValue * valueReturn);
static void GetFileSelBoxFileItemCount(
                        Widget w,
			Cardinal * size,
			XrmValue * valueReturn);
static void GetFileSelBoxDirItemCount(
                        Widget w,
			Cardinal * size,
			XrmValue * valueReturn);
static wtab_t * ConvertWidgetToWtab(
	                char * arg0,
                        Widget w);
static int GetSubWidget(
	                char * errmsg,
	                Widget (*func)(),
                        int argc,
                        char *argv[] );

#endif /* _NO_PROTO */


#define MAXARGS 4096
#define SLISTITEMSIZE	16

#define MWSep1 1
#define MWSep2 2
#define MWSep3 3

static char str_APPNAME[] = "DTKSH_APPNAME";
static char str_TOPLEVEL[] = "DTKSH_TOPLEVEL";

static XtConvertArgRec listItemConvertArgs[] = {
    { XtWidgetBaseOffset,
        (XtPointer)XtOffset(XmListWidget, list.itemCount),
        sizeof (int)
    }
};

static XtConvertArgRec selectedListItemConvertArgs[] = {
    { XtWidgetBaseOffset,
        (XtPointer) XtOffset(XmListWidget, list.selectedItemCount),
        sizeof (int)
    }
};

static XtConvertArgRec selBoxItemConvertArgs[] = {
    { XtProcedureArg,
       (XtPointer)GetSelBoxItemCount,
       sizeof (int)
    }
};

static XtConvertArgRec fileSelBoxFileConvertArgs[] = {
    { XtProcedureArg,
       (XtPointer)GetFileSelBoxFileItemCount,
       sizeof (int)
    }
};

static XtConvertArgRec fileSelBoxDirConvertArgs[] = {
    { XtProcedureArg,
       (XtPointer)GetFileSelBoxDirItemCount,
       sizeof (int)
    }
};


static wtab_t *
#ifdef _NO_PROTO
ConvertWidgetToWtab( arg0, w )
	char * arg0;
        Widget w;
#else
ConvertWidgetToWtab(
	char * arg0,
        Widget w)
#endif /* _NO_PROTO */
{
   wtab_t * wtab;
   classtab_t *ctab;

   wtab = (wtab_t *)widget_to_wtab(w);
   /*
    * If the widget class has no resources registered, then this is
    * the first known instance of this widget class, so we need to
    * force the resource list to be loaded.  This can frequently
    * occur if a Motif convenience function is used, which creates
    * a 'hidden' parent.
    */
   ctab = wtab->wclass;
   if (ctab->res == NULL)
      (void)str_to_class(arg0, ctab->cname);

   return(wtab);
}


static void
#ifdef _NO_PROTO
GetSelBoxItemCount( w, size, valueReturn )
        Widget w;
	Cardinal * size;
	XrmValue * valueReturn;
#else
GetSelBoxItemCount(
        Widget w,
	Cardinal * size,
	XrmValue * valueReturn )
#endif /* _NO_PROTO */
{
   static int data;
   XtArgVal av;

   valueReturn->addr = (XtPointer) &data ;
   valueReturn->size = sizeof(int *);
   _XmSelectionBoxGetListItemCount(w, 0, &av);
   data = (int)av;
}

static void
#ifdef _NO_PROTO
GetFileSelBoxDirItemCount( w, size, valueReturn )
        Widget w;
	Cardinal * size;
	XrmValue * valueReturn;
#else
GetFileSelBoxDirItemCount(
        Widget w,
	Cardinal * size,
	XrmValue * valueReturn )
#endif /* _NO_PROTO */
{
   static int data;
   Widget dirList;
   Arg args[5];

   dirList = XmFileSelectionBoxGetChild(w, XmDIALOG_DIR_LIST);
   XtSetArg(args[0], XmNitemCount, &data);
   XtGetValues(dirList, args, 1);
   valueReturn->addr = (XtPointer) &data ;
   valueReturn->size = sizeof(int *);
}

static void
#ifdef _NO_PROTO
GetFileSelBoxFileItemCount( w, size, valueReturn )
        Widget w;
	Cardinal * size;
	XrmValue * valueReturn;
#else
GetFileSelBoxFileItemCount(
        Widget w,
	Cardinal * size,
	XrmValue * valueReturn )
#endif /* _NO_PROTO */
{
   static int data;
   Widget dirList;
   Arg args[5];

   if (FS_StateFlags(w) & XmFS_NO_MATCH)
      data = 0;
   else
   {
      dirList = XmFileSelectionBoxGetChild(w, XmDIALOG_LIST);
      XtSetArg(args[0], XmNitemCount, &data);
      XtGetValues(dirList, args, 1);
   }
   valueReturn->addr = (XtPointer) &data ;
   valueReturn->size = sizeof(int *);
}

int
#ifdef _NO_PROTO
toolkit_initialize( argc, argv )
        int argc ;
        char *argv[] ;
#else
toolkit_initialize(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
	int i;
	char name[8], *var;
	wtab_t *w;
	int newargc;
	char **newargv;
	char envbuf[2048];
	char * errmsg;
	int * lockedFds;

	init_widgets();

	newargv = (char **)malloc((argc - 3) * sizeof(char *));

	/*
	 * We apparently must alloc a long-lived array of strings for argv.
	 * If we don't, then the WM_COMMAND on the Toplevel shell is munged.
	 * This is presumably because XtInitialize just hangs onto a copy
	 * of the pointer to the argv[n] element of interest, but the shell
	 * apparently frees them up after toolkit_initialize returns.
	 * The argument list looks like:
	 * XtInitialize widgVar widgName className appName $@
	 * or:
	 * XtInitialize widgVar widgName className $0 $@
	 */
	for(i = 0, newargc = argc - 4; i < newargc; i++)
	{
		newargv[i] = (char *)malloc(strlen(argv[i + 4]) + 1);
		strcpy(newargv[i], argv[i + 4]);
	}
	newargv[i] = (char *)NULL;

	Toplevel = XtInitialize((char *)NULL, argv[3], (XrmOptionDescRec *)NULL,
							0, &newargc, newargv);

	if (Toplevel == NULL) {
		errmsg = strdup(GETMESSAGE(15,1, 
                                "Unable to initialize the Toolkit"));
		printerr(argv[0], errmsg, NULL);
                free(errmsg);
		env_blank(argv[1]);
		return(1);
	}

        lockedFds = LockKshFileDescriptors();
        DtInitialize(XtDisplay(Toplevel), Toplevel, argv[4], argv[4]);
        UnlockKshFileDescriptors(lockedFds);

	DtkshRegisterNamedIntConverters();
	XtAddConverter(XmRTopItemPosition, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRHorizontalInt, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRVerticalInt, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRWhichButton, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRShellHorizPos, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRShellVertPos, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRShellHorizDim, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRShellVertDim, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRKeySym, XtRString, 
		DtkshCvtKeySymToString, NULL, 0);
	XtAddConverter(XtRInt, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRShort, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRCardinal, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRDimension, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRBooleanDimension, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRPosition, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRHorizontalDimension, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRVerticalDimension, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRHorizontalPosition, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRVerticalPosition, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XmRTextPosition, XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRBoolean, XtRString, 
		DtkshCvtBooleanToString, NULL, 0);
	XtAddConverter(XtRBool, XtRString, 
		DtkshCvtBooleanToString, NULL, 0);
	XtAddConverter(XtRPixel, XtRString, 
		DtkshCvtHexIntToString, NULL, 0);
	XtAddConverter(XtRPixmap, XtRString, 
		DtkshCvtHexIntToString, NULL, 0);
	XtAddConverter("XE_ModifierState", XtRString, 
		DtkshCvtXEModifierStateToString, NULL, 0);
	XtAddConverter("XE_MotionHint", XtRString, 
		DtkshCvtXEMotionHintToString, NULL, 0);
	XtAddConverter("XE_HexValue", XtRString, 
		DtkshCvtHexIntToString, NULL, 0);
	XtAddConverter("XE_IntValue", XtRString, 
		DtkshCvtIntToString, NULL, 0);
	XtAddConverter(XtRPointer, XtRString, 
		DtkshCvtHexIntToString, NULL, 0);
	XtAddConverter("XE_Window", XtRString, 
		DtkshCvtWindowToString, NULL, 0);
	XtAddConverter(XtRScreen, XtRString, 
		DtkshCvtScreenToString, NULL, 0);
	XtAddConverter(XtRString, XtRScreen, 
		DtkshCvtStringToScreen, NULL, 0);
	XtAddConverter(XtRString, XmRTopItemPosition, 
		DtkshCvtStringToTopItemPosition, NULL, 0);
	XtAddConverter(XtRString, XmRUnitType,
		XmCvtStringToUnitType, NULL, 0);
	XtSetTypeConverter(XmRXmString, XtRString,
		(XtTypeConverter)DtkshCvtXmStringToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRWidget, XtRString,
		(XtTypeConverter)DtkshCvtWidgetToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter("MenuWidget", XtRString,
		(XtTypeConverter)DtkshCvtWidgetToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, XtRWidget,
		(XtTypeConverter)DtkshCvtStringToWidget, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "MenuWidget",
		(XtTypeConverter)DtkshCvtStringToWidget, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, XtRCallback,
		(XtTypeConverter)DtkshCvtStringToCallback, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRCallback, XtRString,
		(XtTypeConverter)DtkshCvtCallbackToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "EventMask",
		(XtTypeConverter)DtkshCvtStringToEventMask, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "MWMDecoration",
		(XtTypeConverter)DtkshCvtStringToMWMDecoration, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter("MWMDecoration", XtRString,
		(XtTypeConverter)DtkshCvtMWMDecorationToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "MWMFunctions",
		(XtTypeConverter)DtkshCvtStringToMWMFunctions, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter("MWMFunctions", XtRString,
		(XtTypeConverter)DtkshCvtMWMFunctionsToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "ListItems",
		(XtTypeConverter)DtkshCvtStringToListItems, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "SelectedListItems",
		(XtTypeConverter)DtkshCvtStringToListItems, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "SelBoxItems",
		(XtTypeConverter)DtkshCvtStringToListItems, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "FSFileItems",
		(XtTypeConverter)DtkshCvtStringToListItems, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "FSDirItems",
		(XtTypeConverter)DtkshCvtStringToListItems, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, XmRWidgetClass,
		(XtTypeConverter)DtkshCvtStringToWidgetClass, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XtRString, "PanedWinPosIndex",
		(XtTypeConverter)DtkshCvtStringToPanedWinPosIndex, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter("PanedWinPosIndex", XtRString,
		(XtTypeConverter)DtkshCvtPanedWinPosIndexToString, NULL, 0, 
                XtCacheNone, NULL);
	XtSetTypeConverter("ListItems", XtRString,
		(XtTypeConverter)DtkshCvtListItemsToString, 
                listItemConvertArgs, 1, 
                XtCacheNone, NULL);
	XtSetTypeConverter("SelBoxItems", XtRString,
		(XtTypeConverter)DtkshCvtListItemsToString, 
                selBoxItemConvertArgs, 1, 
                XtCacheNone, NULL);
	XtSetTypeConverter("SelectedListItems", XtRString,
		(XtTypeConverter)DtkshCvtListItemsToString, 
                selectedListItemConvertArgs, 1, 
                XtCacheNone, NULL);
	XtSetTypeConverter("FSFileItems", XtRString,
		(XtTypeConverter)DtkshCvtListItemsToString, 
                fileSelBoxFileConvertArgs, 1, 
                XtCacheNone, NULL);
	XtSetTypeConverter("FSDirItems", XtRString,
		(XtTypeConverter)DtkshCvtListItemsToString, 
                fileSelBoxDirConvertArgs, 1, 
                XtCacheNone, NULL);
	XtSetTypeConverter(XmRWidgetClass, XtRString,
		(XtTypeConverter)DtkshCvtWidgetClassToString, 
                NULL, 0, XtCacheNone, NULL);

        /* 
         * Override standard Xt StringToPixel converter;
         * we need to handle both pixel names and pixel values.
         */
        RegisterXtOverrideConverters();

	w = set_up_w(Toplevel, NULL, argv[1], argv[2], 
                     str_to_class(argv[0], "ApplicationShell"));

	var = env_get(str_TOPLEVEL);
	if (var == NULL || *var == '\0') {
		env_set_var(str_TOPLEVEL, w->widid);
	}

	var = env_get(str_APPNAME);
	if (var == NULL || *var == '\0') {
		env_set_var(str_APPNAME, argv[2]);
	}

	ksh_eval("unset DTKSH_ARGV");
	for (i = 0; i < newargc; i++) {
		sprintf(envbuf, "DTKSH_ARGV[%d]=%s", i, newargv[i]);
		env_set(envbuf);
	}
	return(0);
}

/*
 * There are certain classes of resources which cannot be specified when
 * a widget is created.  These are resources which rely on a unit of
 * measure typically specified with the 'unitType' resource.  Since there
 * is no widget yet when we convert the resources from string to internal
 * representation, there, of course, is no known unitType yet; thus the
 * conversions cannot be performed.  That is why we have this check, and
 * why we have to tell the user to use XtSetValues().
 *
 * There is a second class of resources where this problem also exists;
 * these are the gadget pixmaps.  The problem exists because the string
 * to pixmap converter for gadgets assumes that it has access to the
 * widget ID, so that it can grab the parent.  Since the widget Id does
 * not yet exist, this information does not exist.
 *
 * A third class of problem resources are all pixmap resources.  If these
 * are specified in the same resource list as the foreground or background
 * colors, then the converter uses the existing colors, instead of the new
 * colors.  To overcome this, we postpone the converting of any pixmap
 * resources until after all others have been set.
 */
int
#ifdef _NO_PROTO
toolkit_special_resource( arg0, res, w, parent, class, resource, val, ret, 
                          freeit, postponePixmaps )
        char *arg0 ;
        XtResourceList res ;
        wtab_t *w ;
        wtab_t *parent ;
        classtab_t *class ;
        char *resource ;
        char *val ;
        XtArgVal *ret ;
        int *freeit ;
        Boolean postponePixmaps ;
#else
toolkit_special_resource(
        char *arg0,
        XtResourceList res,
        wtab_t *w,
        wtab_t *parent,
        classtab_t *class,
        char *resource,
        char *val,
        XtArgVal *ret,
        int *freeit,
        Boolean postponePixmaps )
#endif /* _NO_PROTO */
{
   if ((w == NULL) && 
       ((strcmp(res->resource_type, XmRHorizontalDimension) == 0) ||
        (strcmp(res->resource_type, XmRVerticalDimension) == 0)   ||
        (strcmp(res->resource_type, XmRHorizontalPosition) == 0)  ||
        (strcmp(res->resource_type, XmRVerticalPosition) == 0)    ||
        (strcmp(res->resource_type, XmRBooleanDimension) == 0)    ||
        (strcmp(res->resource_type, XmRGadgetPixmap) == 0)))
   {
      return(TRUE);
   }
   else if (postponePixmaps  &&
            ((strcmp(resource, XmNpixmap) == 0) ||
             (strstr(resource, "Pixmap") != NULL)))
   {
      return(TRUE);
   }

   return(FALSE);
}


static int
#ifdef _NO_PROTO
_xmcreatefunc( func, wclass, argc, argv )
        Widget (*func)() ;
        char *wclass ;
        int argc ;
        char *argv[] ;
#else
_xmcreatefunc(
        Widget (*func)(),
        char *wclass,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
	Widget widget, realparent;
	classtab_t *class;
	char *arg0 = argv[0];
	wtab_t *w, *pw, *wtab, *parenttab;
	char *wname, *parentid, *var;
	Arg	args[MAXARGS];
	register int	i;
	int n;
        char * errmsg;
	int pargc;
	char ** pargv;

	if (argc < 4) {
                errmsg = strdup(GETMESSAGE(15,3, 
                     "Usage: %s variable parent name [argument:value ...]"));
		printerrf(str_nill, errmsg, argv[0], NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		return(1);
	}
	var = argv[1];
	parentid = argv[2];
	wname = argv[3];
	pw = str_to_wtab(argv[0], parentid);
	if (pw == NULL) {
		errmsg = strdup(GetSharedMsg(DT_NO_PARENT));
		printerr(argv[0], errmsg, NULL);
                free(errmsg);
		return(1);
	}
	argv += 4;
	argc -= 4;
	if ((class = str_to_class(arg0, wclass)) == NULL) {
		return(1);
	}
	pargc = 0;
	pargv = (char **)XtMalloc(sizeof(char *) * argc);
	parse_args(arg0, argc, argv, NULL, pw, class, &n, args, &pargc, pargv,
                   True);
	widget = func(pw->w, wname, args, n);
	if (widget != NULL) {
		/* Some of the XmCreate* functions return a widget
		 * id whose parent is not necessarily the parent
		 * passed in.  For example, DialogShell returns the
		 * widget of the dialog, not the Shell which is the
		 * real parent.
		 *
		 * So, we check to see if the parent is the same as
		 * the passed-in parent, and if not then we create
		 * a new entry for the real parent.
		 */
		realparent = XtParent(widget);
		if (realparent != pw->w) {
			parenttab = (wtab_t *)widget_to_wtab(realparent);
		} else
			parenttab = pw;
		wtab = set_up_w(widget, parenttab, var, wname, class);

		/* Process any postponed resources */
		if (pargc > 0)
		{
			free_args(n, args);
			n = 0;
			parse_args(arg0, pargc, pargv, wtab, pw, class, &n, 
				   args, NULL, NULL, False);
			XtSetValues(widget, args, n);
		}
	} else {
                errmsg = strdup(GetSharedMsg(DT_WIDGET_CREATE_FAILED));
		printerrf(argv[0], errmsg,
                          wname, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                free(errmsg);
		env_blank(argv[1]);
	}
	free_args(n, args);
	XtFree((char *)pargv);

	return(0);
}

int
#ifdef _NO_PROTO
do_XmCreateArrowButton( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateArrowButton(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateArrowButton, "XmArrowButton", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateArrowButtonGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateArrowButtonGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateArrowButtonGadget, 
                        "XmArrowButtonGadget", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateBulletinBoard( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateBulletinBoard(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateBulletinBoard, "XmBulletinBoard", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateBulletinBoardDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateBulletinBoardDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateBulletinBoardDialog, "XmBulletinBoard",
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateCascadeButton( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateCascadeButton(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateCascadeButton, "XmCascadeButton", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateCascadeButtonGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateCascadeButtonGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateCascadeButtonGadget, 
                        "XmCascadeButtonGadget", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateCommand( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateCommand(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateCommand, "XmCommand", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateDialogShell( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateDialogShell(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateDialogShell, "XmDialogShell", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateDrawingArea( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateDrawingArea(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateDrawingArea, "XmDrawingArea", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateDrawnButton( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateDrawnButton(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateDrawnButton, "XmDrawnButton", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateErrorDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateErrorDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateErrorDialog, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateFileSelectionBox( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateFileSelectionBox(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateFileSelectionBox, "XmFileSelectionBox",
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateFileSelectionDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateFileSelectionDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateFileSelectionDialog, 
                        "XmFileSelectionBox", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateForm( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateForm(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateForm, "XmForm", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateFormDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateFormDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateFormDialog, "XmForm", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateFrame( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateFrame(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateFrame, "XmFrame", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateInformationDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateInformationDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateInformationDialog, "XmMessageBox", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateLabel( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateLabel(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateLabel, "XmLabel", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateLabelGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateLabelGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateLabelGadget, "XmLabelGadget", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateList( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateList(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateList, "XmList", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateMainWindow( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateMainWindow(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateMainWindow, "XmMainWindow", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateMenuBar( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateMenuBar(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateMenuBar, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateMenuShell( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateMenuShell(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateMenuShell, "XmMenuShell", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateMessageBox( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateMessageBox(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateMessageBox, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateMessageDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateMessageDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateMessageDialog, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateOptionMenu( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateOptionMenu(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateOptionMenu, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePanedWindow( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePanedWindow(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePanedWindow, "XmPanedWindow", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePopupMenu( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePopupMenu(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePopupMenu, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePromptDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePromptDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePromptDialog, "XmSelectionBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePulldownMenu( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePulldownMenu(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePulldownMenu, "XmRowColumn", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePushButton( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePushButton(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePushButton, "XmPushButton", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreatePushButtonGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreatePushButtonGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreatePushButtonGadget, "XmPushButtonGadget",
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateQuestionDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateQuestionDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateQuestionDialog, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateRadioBox( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateRadioBox(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateRadioBox, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateRowColumn( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateRowColumn(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateRowColumn, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateScale( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateScale(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateScale, "XmScale", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateScrollBar( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateScrollBar(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateScrollBar, "XmScrollBar", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateScrolledList( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateScrolledList(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateScrolledList, "XmList", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateScrolledText( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateScrolledText(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateScrolledText, "XmText", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateScrolledWindow( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateScrolledWindow(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateScrolledWindow, "XmScrolledWindow", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateSelectionBox( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateSelectionBox(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateSelectionBox, "XmSelectionBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateSelectionDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateSelectionDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateSelectionDialog, "XmSelectionBox", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateSeparator( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateSeparator(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateSeparator, "XmSeparator", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateSeparatorGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateSeparatorGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateSeparatorGadget, "XmSeparatorGadget", 
                        argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateText( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateText(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateText, "XmText", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateTextField( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateTextField(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateTextField, "XmTextField", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateToggleButton( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateToggleButton(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateToggleButton, "XmToggleButton", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateToggleButtonGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateToggleButtonGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateToggleButtonGadget, 
                        "XmToggleButtonGadget", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateWarningDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateWarningDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateWarningDialog, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_XmCreateWorkArea( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateWorkArea(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateWorkArea, "XmRowColumn", argc, argv));
}


int
#ifdef _NO_PROTO
do_XmCreateWorkingDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCreateWorkingDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(XmCreateWorkingDialog, "XmMessageBox", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_DtCreateHelpDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_DtCreateHelpDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(DtCreateHelpDialog, "DtHelpDialog", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_DtCreateHelpQuickDialog( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_DtCreateHelpQuickDialog(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(_xmcreatefunc(DtCreateHelpQuickDialog, "DtHelpQuickDialog", argc, 
                        argv));
}


int
#ifdef _NO_PROTO
do_DtHelpReturnSelectedWidgetId( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_DtHelpReturnSelectedWidgetId(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * results = argv[1]; 
   char * variable = argv[3]; 
   XrmValue f, t;
   int res;
   classtab_t *ctab;
   Widget retWidget;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,4, 
         "Usage: DtHelpReturnSelectedWidgetId variable widget variable"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(variable, ""); 
        env_blank(results);
	return(1);
   }

   res = DtHelpReturnSelectedWidgetId(w->w, NULL, &retWidget);
   XSync(XtDisplay(w->w), False);

   f.addr = (caddr_t)&res;
   f.size = sizeof(int);
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, "HelpReturnSelectedWidgetResultType", &f, XtRString, &t);
   env_set_var(results,  (char *)(t.addr)); 

   if (res != DtHELP_SELECT_VALID)
   {
      alt_env_set_var(variable, ""); 
      return(0);
   }

   w = ConvertWidgetToWtab(arg0, retWidget);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_DtHelpSetCatalogName( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_DtHelpSetCatalogName(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   char * errmsg;

   if (argc != 2) 
   {
      errmsg = strdup(GETMESSAGE(15,5, 
                      "Usage: DtHelpSetCatalogName catalogName"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   DtHelpSetCatalogName(argv[1]);

   return(0);
}


int
#ifdef _NO_PROTO
do_DtHelpQuickDialogGetChild( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_DtHelpQuickDialogGetChild(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   int childType;
   XrmValue f, t;
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,6, 
            "Usage: DtHelpQuickDialogGetChild variable quickHelpWidget child"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(variable, ""); 
	return(1);
   }

   if (w->wclass->class != dtHelpQuickDialogWidgetClass) {
      errmsg = strdup(GETMESSAGE(15,7, 
                      "The widget must be a 'quickHelp' widget"));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   f.addr = argv[3];
   f.size = strlen(argv[3]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "QuickHelpChildType", &t);

   if (t.size && t.addr) 
      childType = *((int *)t.addr);
   else 
   {
      errmsg = strdup(GetSharedMsg(DT_UNKNOWN_CHILD_TYPE));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = DtHelpQuickDialogGetChild(w->w, childType);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


static wtab_t *
#ifdef _NO_PROTO
verifyListWidget( cmd, widget )
        char *cmd ;
        char *widget ;
#else
verifyListWidget(
        char *cmd,
        char *widget )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char * errmsg;

   if ((w = str_to_wtab(cmd, widget)) == NULL)
      return(NULL);

   if (w->wclass->class != xmListWidgetClass) 
   {
      errmsg = strdup(GETMESSAGE(15,8, "The widget must be a 'list' widget"));
      printerr(cmd, errmsg, NULL);
      free(errmsg);
      return(NULL);
   }

   return(w);
}


static int
#ifdef _NO_PROTO
List_ItemAndPos( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
List_ItemAndPos(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmString string;
   int position = 0;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,9, "Usage: %s widget position item"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL, NULL, NULL, 
                NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   string = XmStringCreateLtoR(argv[3], XmFONTLIST_DEFAULT_TAG);
   position = atoi(argv[2]);

   (*func) (w->w, string, position);
   XmStringFree(string);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListAddItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListAddItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemAndPos(XmListAddItem, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListAddItemUnselected( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListAddItemUnselected(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemAndPos(XmListAddItemUnselected, argc, argv));
}


static int
#ifdef _NO_PROTO
List_ItemListAndPos( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
List_ItemListAndPos(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int position = 0;
   XmString *items = NULL;
   int itemCount = 0;
   int itemListSize = 0;
   int i;
   char * errmsg;

   if (argc < 4)
   {
      errmsg = strdup(GETMESSAGE(15,10, "Usage: %s widget position itemList"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   position = atoi(argv[2]);
 
   for (i = 3; i < argc; i++, itemCount++)
   {
      if (itemCount >= itemListSize)
      {
         itemListSize += 10;
         items = (XmString *)XtRealloc((char *)items, 
                                       sizeof(XmString) * itemListSize);
      }
      items[itemCount] = XmStringCreateLtoR(argv[i], XmFONTLIST_DEFAULT_TAG);
   }

   (*func) (w->w, items, itemCount, position);
 
   for (i = 0; i < itemCount; i++)
      XmStringFree(items[i]);
   XtFree((char *)items);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmListAddItems( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListAddItems(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemListAndPos(XmListAddItems, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListAddItemsUnselected( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListAddItemsUnselected(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemListAndPos(XmListAddItemsUnselected, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListReplaceItemsPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListReplaceItemsPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemListAndPos(XmListReplaceItemsPos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListReplaceItemsPosUnselected( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListReplaceItemsPosUnselected(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemListAndPos(XmListReplaceItemsPosUnselected, argc, argv));
}


static int
#ifdef _NO_PROTO
List_WidgetOnly( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
List_WidgetOnly(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   char * errmsg;

   if (argc != 2) 
   {
      errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   (*func) (w->w);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmListDeleteAllItems( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeleteAllItems(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_WidgetOnly(XmListDeleteAllItems, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListDeselectAllItems( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeselectAllItems(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_WidgetOnly(XmListDeselectAllItems, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListUpdateSelectedList( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListUpdateSelectedList(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_WidgetOnly(XmListUpdateSelectedList, argc, argv));
}


static int
#ifdef _NO_PROTO
List_ItemOnly( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
List_ItemOnly(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmString string;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,11, "Usage: %s widget item"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);

   (*func) (w->w, string);
   XmStringFree(string);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListDeleteItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeleteItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemOnly(XmListDeleteItem, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListDeselectItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeselectItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemOnly(XmListDeselectItem, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetBottomItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetBottomItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemOnly(XmListSetBottomItem, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_ItemOnly(XmListSetItem, argc, argv));
}


static int
#ifdef _NO_PROTO
List_PositionOnly( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
List_PositionOnly(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int position = 0;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_POS));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL,
               NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   position = atoi(argv[2]);
   (*func) (w->w, position);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListDeletePos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeletePos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_PositionOnly(XmListDeletePos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListDeselectPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeselectPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_PositionOnly(XmListDeselectPos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetBottomPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetBottomPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_PositionOnly(XmListSetBottomPos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetHorizPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetHorizPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_PositionOnly(XmListSetHorizPos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (List_PositionOnly(XmListSetPos, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListDeleteItemsPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeleteItemsPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int position = 0;
   int count = 0;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,12, 
               "Usage: XmListDeleteItemsPos widget count position"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   count = atoi(argv[2]);
   position = atoi(argv[3]);
   XmListDeleteItemsPos(w->w, count, position);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListDeleteItems( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeleteItems(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmString *items = NULL;
   int itemCount = 0;
   int itemListSize = 0;
   int i;
   char * errmsg;

   if (argc < 3)
   {
      errmsg = strdup(GETMESSAGE(15,13, 
                      "Usage: XmListDeleteItems widget itemList"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   for (i = 2; i < argc; i++, itemCount++)
   {
      if (itemCount >= itemListSize)
      {
         itemListSize += 10;
         items = (XmString *)XtRealloc((char *)items, 
                                       sizeof(XmString) * itemListSize);
      }
      items[itemCount] = XmStringCreateLtoR(argv[i], XmFONTLIST_DEFAULT_TAG);
   }

   XmListDeleteItems(w->w, items, itemCount);
 
   for (i = 0; i < itemCount; i++)
      XmStringFree(items[i]);
   XtFree((char *)items);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmListDeletePositions( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListDeletePositions(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int positionCount = 0;
   int * positionList = NULL;
   int positionListSize = 0;
   int i;
   char * errmsg;

   if (argc < 3)
   {
      errmsg = strdup(GETMESSAGE(15,14, 
           "Usage: XmListDeletePositions widget positionList"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   for (i = 2; i < argc; i++, positionCount++)
   {
      if (positionCount >= positionListSize)
      {
         positionListSize += 10;
         positionList = (int *)XtRealloc((char *)positionList,
                                         sizeof(int) * positionListSize);
      }
      positionList[positionCount] = atoi(argv[i]);
   }

   XmListDeletePositions(w->w, positionList, positionCount);
 
   XtFree((char *)positionList);

   return(0);
}


static int
#ifdef _NO_PROTO
GetSelectedPosList( func, paramCount, errmsg, argc, argv )
	Boolean (*func)();
	int paramCount;
	char * errmsg;
        int argc ;
        char *argv[] ;
#else
GetSelectedPosList(
	Boolean (*func)(),
	int paramCount,
	char * errmsg,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int i;
   char * buf;
   Boolean result;
   int * posList;
   int posCount;
   char pos[25];
   XmString string;

   if (argc != paramCount)
   {
      printerr(str_nill, errmsg, NULL);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[2])) == NULL)
   {
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   if (paramCount == 3)
      result = (*func)(w->w, &posList, &posCount);
   else
   {
      string = XmStringCreateLtoR(argv[3], XmFONTLIST_DEFAULT_TAG);
      result = (*func)(w->w, string, &posList, &posCount);
      XmStringFree(string);
   }

   if (result)
   {
      buf = XtMalloc(1);
      buf[0] = '\0';

      for (i = 0; i < posCount; i++)
      {
         sprintf(pos, "%d", (int)posList[i]);
         buf = XtRealloc(buf, strlen(buf) + strlen(pos) + 2);
         if (i != 0)
            strcat(buf, ",");
         strcat(buf, pos);
      }

      env_set_var(argv[1], buf);
      XtFree ((char *)posList);
      XtFree(buf);
   }
   else
      env_blank(argv[1]);

   return (!result);
}


int
#ifdef _NO_PROTO
do_XmListGetSelectedPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListGetSelectedPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   Boolean result;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,94, 
                   "Usage: XmListGetSelectedPos variable widget"));
   result = GetSelectedPosList(XmListGetSelectedPos, 3, errmsg, argc, argv);
   free(errmsg);
   return(result);
}


int
#ifdef _NO_PROTO
do_XmListGetMatchPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListGetMatchPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   Boolean result;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,95, 
                   "Usage: XmListGetMatchPos variable widget item"));
   result = GetSelectedPosList(XmListGetMatchPos, 4, errmsg, argc, argv);
   free(errmsg);
   return(result);
}


int
#ifdef _NO_PROTO
do_XmListGetKbdItemPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListGetKbdItemPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int position;
   int i;
   char buf[24];
   char * errmsg;

   if (argc != 3)
   {
      errmsg = strdup(GETMESSAGE(15,15, 
                      "Usage: XmListGetKbdItemPos variable widget"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[2])) == NULL)
   {
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   position = XmListGetKbdItemPos(w->w);
   sprintf(buf, "%d", position);
   alt_env_set_var(argv[1],  buf); 
 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListItemExists( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListItemExists(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmString string;
   Boolean res;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,16, "Usage: XmListItemExists widget item"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);

   res = !XmListItemExists(w->w, string);
   XmStringFree(string);
   return(res);
}


int
#ifdef _NO_PROTO
do_XmListItemPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListItemPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int position;
   int i;
   char buf[24];
   XmString item;
   char * errmsg;

   if (argc != 4)
   {
      errmsg = strdup(GETMESSAGE(15,17, 
                      "Usage: XmListItemPos variable widget item"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[2])) == NULL)
   {
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   item = XmStringCreateLtoR(argv[3], XmFONTLIST_DEFAULT_TAG);
   position = XmListItemPos(w->w, item);
   sprintf(buf, "%d", position);
   alt_env_set_var(argv[1],  buf); 
   XmStringFree(item);
 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListPosSelected( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListPosSelected(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Boolean res;
   int position;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,18, 
                      "Usage: XmListPosSelected widget position"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   position = atoi(argv[2]);

   res = !XmListPosSelected(w->w, position);
   return(res);
}


int
#ifdef _NO_PROTO
do_XmListPosToBounds( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListPosToBounds(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Boolean res;
   int position;
   Position x;
   Position y;
   Dimension width;
   Dimension height;
   char buf[24];
   char * errmsg;

   if (argc != 7) 
   {
      errmsg=strdup(GETMESSAGE(15,19, 
            "Usage: XmListPosToBounds widget position variable variable variable variable"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
   {
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      env_blank(argv[6]); 
      return(1);
   }

   position = atoi(argv[2]);

   res = !XmListPosToBounds(w->w, position, &x, &y, &width, &height);

   if (res == False)
   {
      sprintf(buf, "%d", x);
      env_set_var(argv[3],  buf); 
      sprintf(buf, "%d", y);
      env_set_var(argv[4],  buf); 
      sprintf(buf, "%d", width);
      env_set_var(argv[5],  buf); 
      sprintf(buf, "%d", height);
      env_set_var(argv[6],  buf); 
   }
   else
   {
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      env_blank(argv[6]); 
   }
   return(res);
}


static int
#ifdef _NO_PROTO
ListSelectItem( usePosition, argc, argv )
        Boolean usePosition ;
        int argc ;
        char *argv[] ;
#else
ListSelectItem(
        Boolean usePosition,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int i;
   XmString item;
   XrmValue fval, tval;
   int position = 0;
   Boolean notify;
   char * errmsg;

   if (argc != 4)
   {
      if (usePosition)
      {
         errmsg = strdup(GETMESSAGE(15,20, 
                         "Usage: %s widget position notifyFlag"));
      }
      else
      {
         errmsg = strdup(GETMESSAGE(15,21, "Usage: %s widget item notifyFlag"));
      }

      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL, NULL, NULL, 
                NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   if (usePosition)
      position = atoi(argv[2]);
   else
      item = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);

   fval.addr = argv[3];
   fval.size = strlen(argv[3]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      notify = *((Boolean *)(tval.addr));
   else
      return(1);

   if (usePosition)
      XmListSelectPos(w->w, position, notify);
   else
   {
      XmListSelectItem(w->w, item, notify);
      XmStringFree(item);
   }
 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListSelectItem( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSelectItem(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(ListSelectItem(False, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSelectPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSelectPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(ListSelectItem(True, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmListSetAddMode( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetAddMode(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XrmValue fval, tval;
   Boolean state;
   char * errmsg;

   if (argc != 3)
   {
      errmsg = strdup(GETMESSAGE(15,22, 
                      "Usage: XmListSetAddMode widget boolean"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   fval.addr = argv[2];
   fval.size = strlen(argv[2]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      state = *((Boolean *)(tval.addr));
   else
      return(1);

   XmListSetAddMode(w->w, state);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmListSetKbdItemPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmListSetKbdItemPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Boolean res;
   int position;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,23, 
                      "Usage: XmListSetKbdItemPos widget position"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyListWidget(argv[0], argv[1])) == NULL)
      return(1);

   position = atoi(argv[2]);

   res = !XmListSetKbdItemPos(w->w, position);
   return(res);
}


int
#ifdef _NO_PROTO
do_XmMainWindowSetAreas( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMainWindowSetAreas(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w[6];
   register int i;
   char * errmsg;

   if (argc != 7) 
   {
      errmsg=strdup(GETMESSAGE(15,24, 
           "Usage: XmMainWindowSetAreas mainwindow menu command hscroll vscroll work"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   for (i = 1; i < 7; i++) 
   {
      if ((argv[i][0] == '\0') || 
          (strcmp(argv[i], (char *)("NULL")) == 0)) 
      {
         w[i-1] = NULL;
         continue;
      }

      w[i-1] = str_to_wtab(arg0, argv[i]);
      if (w[i-1] == NULL) 
         continue;
   }

   if (w[0] == NULL) {
      errmsg = strdup(GETMESSAGE(15,25, "The 'mainWindow' handle is NULL"));
      printerr(argv[0], errmsg, NULL);
      free(errmsg);
      return(1);
   }

   if (w[0]->wclass->class != xmMainWindowWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_MAIN_WIN_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   XmMainWindowSetAreas(w[0]->w, 
                        w[1] ? w[1]->w : (Widget)NULL, 
                        w[2] ? w[2]->w : (Widget)NULL, 
                        w[3] ? w[3]->w : (Widget)NULL,
                        w[4] ? w[4]->w : (Widget)NULL, 
                        w[5] ? w[5]->w : (Widget)NULL);
   return(0);
}


static int
#ifdef _NO_PROTO
GetMainWindowSeparator( childType, argc, argv )
        int childType ;
        int argc ;
        char *argv[] ;
#else
GetMainWindowSeparator(
        int childType,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,26, "Usage: %s variable mainwindow"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(variable, ""); 
	return(1);
   }

   if (w->wclass->class != xmMainWindowWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_MAIN_WIN_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   if (childType == MWSep1)
      child = XmMainWindowSep1(w->w);
   else if (childType == MWSep2)
      child = XmMainWindowSep2(w->w);
   else if (childType == MWSep3)
      child = XmMainWindowSep3(w->w);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmMainWindowSep1( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMainWindowSep1(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(GetMainWindowSeparator(MWSep1, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmMainWindowSep2( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMainWindowSep2(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(GetMainWindowSeparator(MWSep2, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmMainWindowSep3( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMainWindowSep3(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return(GetMainWindowSeparator(MWSep3, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmProcessTraversal( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmProcessTraversal(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   XrmValue f, t;
   wtab_t *w;
   char * errmsg;

   if (argc != 3) {
      errmsg = strdup(GETMESSAGE(15,27, 
                      "Usage: XmProcessTraversal widget direction"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(argv[0], argv[1]);
   if (w == NULL)
      return(1);

   f.addr = argv[2];
   f.size = strlen(argv[2]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "TraversalDirection", &t);

   if (t.size && t.addr) 
   {
      int direction = ((int *)(t.addr))[0];
      return(!XmProcessTraversal(w->w, direction));
   } 
   else 
   {
      errmsg = strdup(GETMESSAGE(15,28, "Unknown traversal direction: %s"));
      printerrf(argv[0], errmsg, argv[2], NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }
}


int
#ifdef _NO_PROTO
do_XmInternAtom( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmInternAtom(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   XrmValue fval, tval;
   Boolean onlyIfExists;
   char buf[24];
   Atom atom;
   Display * display;
   char * p;
   char * errmsg;

   if (argc != 5)
   {
      errmsg = strdup(GETMESSAGE(15,29, 
             "Usage: XmInternAtom variable display name onlyIfExists"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   display = (Display *)strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GetSharedMsg(DT_BAD_DISPLAY));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL, NULL, NULL, NULL,
                NULL, NULL);
      free(errmsg);
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   fval.addr = argv[4];
   fval.size = strlen(argv[4]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);

   if (tval.size != 0)
      onlyIfExists = *((Boolean *)(tval.addr));
   else
   {
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   atom = XmInternAtom (display, argv[3], onlyIfExists);

   sprintf(buf, "%d", atom);
   alt_env_set_var(argv[1],  buf); 
   return(0);
}


/*
 * Simply prevents non-fatal X errors from killing the application.
 */
static int
#ifdef _NO_PROTO
CatchAndIgnoreXError( display, event )
        Display *display ;
        XEvent *event ;
#else
CatchAndIgnoreXError(
        Display *display,
        XEvent *event )
#endif /* _NO_PROTO */
{
}


int
#ifdef _NO_PROTO
do_XmGetAtomName( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetAtomName(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *name;
   Atom atom;
   char * p;
   int (*oldHandler)();
   Display * display;
   char * errmsg;

   if (argc != 4)
   {
      errmsg = strdup(GETMESSAGE(15,30, 
                      "Usage: XmGetAtomName variable display atom"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   display = (Display *)strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GetSharedMsg(DT_BAD_DISPLAY));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL, NULL, NULL, NULL,
                NULL, NULL);
      free(errmsg);
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   atom = (Atom)strtoul(argv[3], &p, 0);
   if (p == argv[3]) 
   {
      errmsg = strdup(GETMESSAGE(15,31, "The specified atom is invalid: %s"));
      printerrf(argv[0], errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(argv[1], ""); 
      return(1);
   }
   oldHandler = XSetErrorHandler((int(*)())CatchAndIgnoreXError);
   name = XmGetAtomName (display, atom);
   XSetErrorHandler((int(*)())oldHandler);
   if (name == NULL)
   {
      alt_env_set_var(argv[1], ""); 
      return(1);
   }

   alt_env_set_var(argv[1],  name); 
   XtFree(name);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmGetColors( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetColors(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char buf[24];
   Pixel background;
   Pixel foreground;
   Pixel topShadow;
   Pixel bottomShadow;
   Pixel select;
   Colormap colormap;
   Arg args[5];
   char * p;
   char * errmsg;


   if (argc != 7)
   {
      errmsg=strdup(GETMESSAGE(15,32, 
          "Usage: XmGetColors widget background foreground topshadow bottomshadow select"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(argv[0], argv[1]);
   if (w == NULL)
   {
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      env_blank(argv[6]); 
      return(1);
   }

   background = strtoul(argv[2], &p, 0);
   if (p == argv[2]) 
   {
      errmsg = strdup(GETMESSAGE(15,33, "The background pixel is invalid: %s"));
      printerrf(argv[0], errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      env_blank(argv[6]); 
      return(1);
   }

   XtSetArg (args[0], XmNcolormap,  &colormap);
   XtGetValues (w->w, args, 1);

   XmGetColors (XtScreen(w->w), colormap, background, &foreground,
                &topShadow, &bottomShadow, &select);

   sprintf(buf, "0x%x", foreground);
   env_set_var(argv[3],  buf); 
   sprintf(buf, "0x%x", topShadow);
   env_set_var(argv[4],  buf); 
   sprintf(buf, "0x%x", bottomShadow);
   env_set_var(argv[5],  buf); 
   sprintf(buf, "0x%x", select);
   env_set_var(argv[6],  buf); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmUpdateDisplay( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmUpdateDisplay(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char * p;
   char * errmsg;

   if (argc != 2)
   {
      errmsg = strdup(GETMESSAGE(15,34, "Usage: XmUpdateDisplay widget"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(argv[0], argv[1]);
   if (w == NULL)
      return(1);

   XmUpdateDisplay (w->w);
   return(0);
}


static int
#ifdef _NO_PROTO
AddOrDeleteWMProtocols( argc, argv )
        int argc ;
        char *argv[] ;
#else
AddOrDeleteWMProtocols(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char * p;
   Atom * protocolList;
   int i;
   char * errmsg;

   if (argc < 3)
   {
      errmsg = strdup(GETMESSAGE(15,35, 
                      "Usage: %s widget protocol [protocol ...]"));
      printerrf(argv[0], errmsg, argv[0],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(argv[0], argv[1]);
   if (w == NULL)
      return(1);

   protocolList = (Atom*)XtMalloc(sizeof(Atom) * (argc - 2));
   for (i = 2; i < argc; i++)
   {
      protocolList[i - 2] = (Atom)strtoul(argv[i], &p, 0);
      if (p == argv[i])
      {
         errmsg = strdup(GETMESSAGE(15,36, 
                         "The atom specified is invalid: %s"));
         printerrf(argv[0], errmsg, argv[i],
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL);
         free(errmsg);
         XtFree((char *)protocolList);
         return(1);
      }
   }

   if (strcmp(argv[0], "XmAddWMProtocols") == 0)
      XmAddWMProtocols (w->w, protocolList, argc-2);
   else
      XmRemoveWMProtocols (w->w, protocolList, argc-2);
   XtFree((char *)protocolList);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmAddWMProtocols( argc, argv )
        int argc ;
        char **argv ;
#else
do_XmAddWMProtocols(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   return (AddOrDeleteWMProtocols(argc, argv));
}


int
#ifdef _NO_PROTO
do_XmRemoveWMProtocols( argc, argv )
        int argc ;
        char **argv ;
#else
do_XmRemoveWMProtocols(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   return (AddOrDeleteWMProtocols(argc, argv));
}


int
#ifdef _NO_PROTO
do_XmAddWMProtocolCallback( argc, argv )
        int argc ;
        char **argv ;
#else
do_XmAddWMProtocolCallback(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   char * errmsg;

   if (argc != 4)
   {
      errmsg = strdup(GETMESSAGE(15,37, 
            "Usage: XmAddWMProtocolCallback widget protocol ksh-command"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   return(AddOneCallback(argv[0], argv[1], NULL, argv[3], argv[2]));
}

int
#ifdef _NO_PROTO
do_XmRemoveWMProtocolCallback( argc, argv )
        int argc ;
        char **argv ;
#else
do_XmRemoveWMProtocolCallback(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   char * errmsg;

   if (argc != 4)
   {
      errmsg = strdup(GETMESSAGE(15,38, 
          "Usage: XmRemoveWMProtocolCallback widget protocol ksh-command"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   return(RemoveOneCallback (argv[0], argv[1], NULL, argv[3], argv[2], NULL));
}


int
#ifdef _NO_PROTO
do_XmMenuPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMenuPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char * p;
   XEvent * event;
   char * errmsg;

   if (argc != 3)
   {
      errmsg = strdup(GETMESSAGE(15,39, "Usage: XmMenuPosition menu event"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(argv[0], argv[1]);
   if (w == NULL)
      return(1);

   event = (XEvent *)strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,40, "The specified event is invalid: %s"));
      printerrf(argv[0], errmsg, argv[2], NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   XmMenuPosition (w->w, (XButtonPressedEvent *)event);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmCommandAppendValue( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCommandAppendValue(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,41, 
             "Usage: XmCommandAppendValue commandWidget string"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
      return(1);

   if (w->wclass->class != xmCommandWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_CMD_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);
   XmCommandAppendValue(w->w, string);
   XmStringFree(string);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmCommandError( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCommandError(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,42, 
          "Usage: XmCommandError commandWidget errorMessage"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
      return(1);

   if (w->wclass->class != xmCommandWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_CMD_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);
   XmCommandError(w->w, string);
   XmStringFree(string);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmCommandSetValue( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCommandSetValue(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,43, 
              "Usage: XmCommandSetValue commandWidget command"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
      return(1);

   if (w->wclass->class != xmCommandWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_CMD_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);
   XmCommandSetValue(w->w, string);
   XmStringFree(string);

   return(0);
}


int
#ifdef _NO_PROTO
do_XmCommandGetChild( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmCommandGetChild(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   int childType;
   XrmValue f, t;
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,44, 
             "Usage: XmCommandGetChild variable commandWidget child"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(variable, ""); 
	return(1);
   }

   if (w->wclass->class != xmCommandWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_CMD_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   f.addr = argv[3];
   f.size = strlen(argv[3]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "CommandChildType", &t);

   if (t.size && t.addr) 
      childType = *((int *)t.addr);
   else 
   {
      errmsg = strdup(GetSharedMsg(DT_UNKNOWN_CHILD_TYPE));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = XmCommandGetChild(w->w, childType);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmMessageBoxGetChild( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmMessageBoxGetChild(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   int childType;
   XrmValue f, t;
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,45, 
            "Usage: XmMessageBoxGetChild variable commandWidget child"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(variable, ""); 
	return(1);
   }

   if (w->wclass->class != xmMessageBoxWidgetClass) {
      errmsg = strdup(GETMESSAGE(15,46, 
                      "The widget must be a 'messageBox' widget"));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   f.addr = argv[3];
   f.size = strlen(argv[3]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "MessageBoxChildType", &t);

   if (t.size && t.addr) 
      childType = *((int *)t.addr);
   else 
   {
      errmsg = strdup(GetSharedMsg(DT_UNKNOWN_CHILD_TYPE));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = XmMessageBoxGetChild(w->w, childType);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmFileSelectionBoxGetChild( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmFileSelectionBoxGetChild(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   int childType;
   XrmValue f, t;
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,47, 
             "Usage: XmFileSelectionBoxGetChild variable widget child"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   if (w->wclass->class != xmFileSelectionBoxWidgetClass) {
      errmsg = strdup(GETMESSAGE(15,48, 
            "The widget must be a 'file selection box' widget"));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   f.addr = argv[3];
   f.size = strlen(argv[3]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "FileSelChildType", &t);

   if (t.size && t.addr) 
      childType = *((int *)t.addr);
   else 
   {
      errmsg = strdup(GetSharedMsg(DT_UNKNOWN_CHILD_TYPE));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = XmFileSelectionBoxGetChild(w->w, childType);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmSelectionBoxGetChild( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmSelectionBoxGetChild(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XmString string;
   char * variable = argv[1]; 
   int childType;
   XrmValue f, t;
   Widget child;
   classtab_t *ctab;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,49, 
              "Usage: XmSelectionBoxGetChild variable widget child"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   if (w->wclass->class != xmSelectionBoxWidgetClass) {
      errmsg = strdup(GETMESSAGE(15,50, 
             "The widget must be a 'selection box' widget"));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   f.addr = argv[3];
   f.size = strlen(argv[3]) + 1;
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, XtRString, &f, "SelBoxChildType", &t);

   if (t.size && t.addr) 
      childType = *((int *)t.addr);
   else 
   {
      errmsg = strdup(GetSharedMsg(DT_UNKNOWN_CHILD_TYPE));
      printerrf(argv[0], errmsg, argv[2], NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = XmSelectionBoxGetChild(w->w, childType);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}

int
#ifdef _NO_PROTO
do_XmIsTraversable( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmIsTraversable(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
	return(do_single_widget_test_func((int(*)())XmIsTraversable, argc, 
                                          argv));
}


int
#ifdef _NO_PROTO
do_XmScaleGetValue( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmScaleGetValue(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char * variable = argv[2]; 
   char buf[25];
   int scaleValue;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,51, 
                      "Usage: XmScaleGetValue scaleWidget variable"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
   {
      env_blank(variable); 
      return(1);
   }

   if (w->wclass->class != xmScaleWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_SCALE_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      env_blank(variable); 
      return(1);
   }

   XmScaleGetValue(w->w, &scaleValue);
   sprintf(buf, "%d", scaleValue);
   env_set_var(variable,  buf); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmScaleSetValue( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmScaleSetValue(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char buf[25];
   int scaleValue;
   char * p;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,52, 
                      "Usage: XmScaleSetValue scaleWidget value"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
	return(1);

   if (w->wclass->class != xmScaleWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_SCALE_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   scaleValue = strtol(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,53, 
                      "The scale value specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }
   XmScaleSetValue(w->w, scaleValue);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmScrollBarGetValues( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmScrollBarGetValues(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char buf[25];
   int value;
   int sliderSize;
   int increment;
   int pageIncrement;
   Boolean notify;
   char * errmsg;

   if (argc != 6) 
   {
      errmsg=strdup(GETMESSAGE(15,54, 
           "Usage: XmScrollBarGetValues scrollbar variable variable variable variable"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
   {
      env_blank(argv[2]); 
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      return(1);
   }

   if (w->wclass->class != xmScrollBarWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_SCROLLBAR_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      env_blank(argv[2]); 
      env_blank(argv[3]); 
      env_blank(argv[4]); 
      env_blank(argv[5]); 
      return(1);
   }

   XmScrollBarGetValues(w->w, &value, &sliderSize, &increment, &pageIncrement);
   sprintf(buf, "%d", value);
   env_set_var(argv[2],  buf); 
   sprintf(buf, "%d", sliderSize);
   env_set_var(argv[3],  buf); 
   sprintf(buf, "%d", increment);
   env_set_var(argv[4],  buf); 
   sprintf(buf, "%d", pageIncrement);
   env_set_var(argv[5],  buf); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmScrollBarSetValues( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmScrollBarSetValues(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char buf[25];
   int value;
   int sliderSize;
   int increment;
   int pageIncrement;
   Boolean notify;
   char * p;
   XrmValue fval, tval;
   char * errmsg;

   if (argc != 7) 
   {
      errmsg=strdup(GETMESSAGE(15,55, 
           "Usage: XmScrollBarSetValues scrollbar value sliderSize increment pageIncrement notify"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
	return(1);

   if (w->wclass->class != xmScrollBarWidgetClass) {
      errmsg = strdup(GetSharedMsg(DT_SCROLLBAR_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   value = strtol(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,56, "The value specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2], NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   sliderSize = strtoul(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,57, 
                      "The slider size specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   increment = strtoul(argv[4], &p, 0);
   if (p == argv[4])
   {
      errmsg = strdup(GETMESSAGE(15,58, 
                      "The increment specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[4],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   pageIncrement = strtoul(argv[5], &p, 0);
   if (p == argv[5])
   {
      errmsg = strdup(GETMESSAGE(15,59, 
                      "The page increment specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[5],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   fval.addr = argv[6];
   fval.size = strlen(argv[6]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      notify = *((Boolean *)(tval.addr));
   else
      return(1);

   XmScrollBarSetValues(w->w, value, sliderSize, increment, pageIncrement,
                        notify);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmScrollVisible( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmScrollVisible(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   wtab_t * w2;
   char buf[25];
   Dimension lrMargin;
   Dimension tbMargin;
   char * p;
   char * errmsg;

   if (argc != 5) 
   {
      errmsg=strdup(GETMESSAGE(15,60, 
        "Usage: XmScrollVisible scrolledWin widget leftRightMargin topBottomMargin"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
	return(1);

   if (w->wclass->class != xmScrolledWindowWidgetClass) {
      errmsg = strdup(GETMESSAGE(15,61, 
             "The widget must be a 'scrolledWindow' widget"));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w2 = str_to_wtab(arg0, argv[2]);
   if (w2 == NULL) 
   {
      errmsg = strdup(GETMESSAGE(15,62, 
              "The widget to be made visible does not exist."));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   lrMargin = strtol(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,63, 
            "The left/right margin specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3], NULL, NULL, NULL, NULL, NULL, NULL, 
                NULL);
      free(errmsg);
      return(1);
   }

   tbMargin = strtoul(argv[4], &p, 0);
   if (p == argv[4])
   {
      errmsg = strdup(GETMESSAGE(15,64, 
             "The top/bottom margin specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[4], NULL, NULL, NULL, NULL, NULL, NULL, 
                NULL);
      free(errmsg);
      return(1);
   }

   XmScrollVisible(w->w, w2->w, lrMargin, tbMargin);
   return(0);
}


static int
#ifdef _NO_PROTO
GetToggleState( argc, argv )
        int argc ;
        char *argv[] ;
#else
GetToggleState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char * errmsg;

   if (argc != 2) 
   {
      errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
	return(1);

   if ((w->wclass->class != xmToggleButtonWidgetClass) &&
       (w->wclass->class != xmToggleButtonGadgetClass))
   {
      errmsg = strdup(GetSharedMsg(DT_TOGGLE_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   return(do_single_widget_test_func((int(*)())XmToggleButtonGetState, argc, 
                                     argv));
}


int
#ifdef _NO_PROTO
do_XmToggleButtonGetState( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmToggleButtonGetState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (GetToggleState(argc, argv));
}


int
#ifdef _NO_PROTO
do_XmToggleButtonGadgetGetState( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmToggleButtonGadgetGetState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (GetToggleState(argc, argv));
}


static int
#ifdef _NO_PROTO
SetToggleState( argc, argv )
        int argc ;
        char *argv[] ;
#else
SetToggleState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   XrmValue fval, tval;
   Boolean state;
   Boolean notify;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,65, "Usage: %s widget state notify"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL) 
	return(1);

   if ((w->wclass->class != xmToggleButtonWidgetClass) &&
       (w->wclass->class != xmToggleButtonGadgetClass))
   {
      errmsg = strdup(GetSharedMsg(DT_TOGGLE_WIDGET));
      printerr(arg0, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   fval.addr = argv[2];
   fval.size = strlen(argv[2]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      state = *((Boolean *)(tval.addr));
   else
      return(1);

   fval.addr = argv[3];
   fval.size = strlen(argv[3]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      notify = *((Boolean *)(tval.addr));
   else
      return(1);

   XmToggleButtonSetState(w->w, state, notify);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmToggleButtonSetState( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmToggleButtonSetState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (SetToggleState(argc, argv));
}

int
#ifdef _NO_PROTO
do_XmToggleButtonGadgetSetState( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmToggleButtonGadgetSetState(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (SetToggleState(argc, argv));
}


int
#ifdef _NO_PROTO
do_catopen( argc, argv )
        int argc ;
        char **argv ;
#else
do_catopen(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   int * lockedFds;
   char * arg0 = argv[0];
   char * var = argv[1];
   char * catName = argv[2];
   char buf[10];
   char * altCatName;
   char * ptr;
   nl_catd nlmsg_fd = (nl_catd)-1;
   char * errmsg;
#if defined(SVR4) || defined (_AIX) || defined(sco) || defined(DEC)
   char * nextMatch;
#endif

   if (argc != 3)
   {
      errmsg = strdup(GETMESSAGE(15,66, "Usage: catopen variable catName"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   lockedFds = LockKshFileDescriptors();

   if ((nlmsg_fd = catopen(catName, 0)) == (nl_catd)-1)
   {
      /*
       * Try several other formats supported by the various
       * architectures; i.e. if the catName ends in ".cat",
       * then try removing that part, or if the catName does
       * not end in ".cat", then try adding it.
       */

      altCatName = XtMalloc(strlen(catName) + 10);
#if defined(SVR4) || defined (_AIX) || defined(sco) || defined(DEC)
      /* These platforms don't have strrstr() */
      ptr = NULL;
      nextMatch = catName;
      while (nextMatch = strstr(nextMatch, ".cat"))
      {
         ptr = nextMatch;
         nextMatch++;
      }
#else
      ptr = (char *)strrstr(catName, ".cat");
#endif
      if (ptr && (strlen(ptr) == 4))
      {
         /* Strip off the ".cat", and try again */
         *ptr = '\0';
         strcpy(altCatName, catName);
         *ptr = '.';
      } 
      else
      {
         /* Add the ".cat", and try again */
         strcpy(altCatName, catName);
         strcat(altCatName, ".cat");
      }
      nlmsg_fd = catopen(altCatName, 0);
      XtFree(altCatName);
   }
   UnlockKshFileDescriptors(lockedFds);

   sprintf(buf, "%ld", (long)nlmsg_fd);
   alt_env_set_var(var,  buf);
   return(0);
}

int
#ifdef _NO_PROTO
do_catclose( argc, argv )
        int argc ;
        char **argv ;
#else
do_catclose(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   char * arg0 = argv[0];
   char * catId = argv[1];
   nl_catd id;
   char * errmsg;

   if (argc != 2)
   {
      errmsg = strdup(GETMESSAGE(15,67, "Usage: catclose catId"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   sscanf(catId, "%ld", &id);
   catclose(id);
   return(0);
}

int
#ifdef _NO_PROTO
do_catgets( argc, argv )
        int argc ;
        char **argv ;
#else
do_catgets(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   char * arg0 = argv[0];
   char * variable = argv[1];
   char * catId = argv[2];
   char * setNum = argv[3];
   char * msgNum = argv[4];
   char * dftMsg = argv[5];
   char * msg;
   nl_catd id;
   int setNumVal;
   int msgNumVal;
   char * errmsg;

   if (argc != 6)
   {
      errmsg = strdup(GETMESSAGE(15,68, 
             "Usage: catgets variable catId setNum msgNum dftMsg"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   sscanf(catId, "%ld", &id);
   sscanf(setNum, "%ld", &setNumVal);
   sscanf(msgNum, "%ld", &msgNumVal);
   msg = catgets(id, setNumVal, msgNumVal, dftMsg);
   alt_env_set_var(variable, msg);
   return(0);
}


static wtab_t *
#ifdef _NO_PROTO
verifyTextWidget( cmd, widget )
        char *cmd ;
        char *widget ;
#else
verifyTextWidget(
        char *cmd,
        char *widget )
#endif /* _NO_PROTO */
{
   wtab_t *w;
   char * errmsg;

   if ((w = str_to_wtab(cmd, widget)) == NULL)
      return(NULL);

   if ((w->wclass->class != xmTextWidgetClass) &&
       (w->wclass->class != xmTextFieldWidgetClass))
   {
      errmsg = strdup(GETMESSAGE(15,69, 
             "The widget must be a 'text' or 'textField' widget"));
      printerr(cmd, errmsg, NULL);
      free(errmsg);
      return(NULL);
   }

   return(w);
}


static int
#ifdef _NO_PROTO
Text_Widget( func, returnBoolean, argc, argv )
        Boolean (*func)() ;
        Boolean returnBoolean ;
        int argc ;
        char *argv[] ;
#else
Text_Widget(
        Boolean (*func)(),
        Boolean returnBoolean,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Boolean result;
   char * errmsg;

   if (argc != 2) 
   {
      errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   result = (*func) (w->w);

   if (returnBoolean)
      return(!result);
   else
      return(0);
}


int
#ifdef _NO_PROTO
do_XmTextDisableRedisplay( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextDisableRedisplay(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_Widget((Boolean (*)())XmTextDisableRedisplay, False, argc, 
           argv));
}


int
#ifdef _NO_PROTO
do_XmTextEnableRedisplay( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextEnableRedisplay(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_Widget((Boolean (*)())XmTextEnableRedisplay, False, argc, 
           argv));
}


int
#ifdef _NO_PROTO
do_XmTextPaste( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextPaste(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_Widget(XmTextPaste, True, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetEditable( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetEditable(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_Widget(XmTextGetEditable, True, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextRemove( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextRemove(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_Widget(XmTextRemove, True, argc, argv));
}


static int
#ifdef _NO_PROTO
Text_VarAndWidget( func, varIsString, argc, argv )
        int (*func)() ;
        Boolean varIsString ;
        int argc ;
        char *argv[] ;
#else
Text_VarAndWidget(
        int (*func)(),
        Boolean varIsString,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   int result;
   char * string;
   char buf[10];
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,70, "Usage: %s variable widget"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[2])) == NULL)
   {
      alt_env_set_var(argv[1], "");
      return(1);
   }

   if (varIsString)
   {
      string = (char *)(*func) (w->w);
      alt_env_set_var(argv[1],  string);
      XtFree(string);
      return(0);
   }
   else
   {
      result = (*func) (w->w);
      sprintf(buf, "%d", result);
      alt_env_set_var(argv[1],  buf);
      return(0);
   }
}


int
#ifdef _NO_PROTO
do_XmTextGetTopCharacter( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetTopCharacter(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget((int (*)())XmTextGetTopCharacter, False, argc, 
           argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetBaseline( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetBaseline(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget(XmTextGetBaseline, False, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetInsertionPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetInsertionPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget((int (*)())XmTextGetInsertionPosition, False, 
           argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetLastPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetLastPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget((int (*)())XmTextGetLastPosition, False, argc, 
                             argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetMaxLength( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetMaxLength(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget(XmTextGetMaxLength, False, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetSelection( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetSelection(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget((int (*)())XmTextGetSelection, True, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextGetString( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetString(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_VarAndWidget((int (*)())XmTextGetString, True, argc, argv));
}


static int
#ifdef _NO_PROTO
Text_WidgetAndBoolean( func, argc, argv )
        void (*func)() ;
        int argc ;
        char *argv[] ;
#else
Text_WidgetAndBoolean(
        void (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   char buf[10];
   Boolean boolean;
   XrmValue fval, tval;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,71, "Usage: %s widget boolean"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   fval.addr = argv[2];
   fval.size = strlen(argv[2]);
   XtConvert(Toplevel, XtRString, &fval, XtRBoolean, &tval);
   if (tval.size != 0)
      boolean = *((Boolean *)(tval.addr));
   else
      return(1);

   (*func) (w->w, boolean);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextSetEditable( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetEditable(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_WidgetAndBoolean(XmTextSetEditable, argc, argv));
}


int
#ifdef _NO_PROTO
do_XmTextSetAddMode( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetAddMode(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   return (Text_WidgetAndBoolean(XmTextSetAddMode, argc, argv));
}


static int
#ifdef _NO_PROTO
Text_WidgetAndOneParam( func, returnBoolean, paramIsString, usageMsg, argc, 
                        argv)
        Boolean (*func)() ;
        Boolean returnBoolean ;
        Boolean paramIsString ;
        char *usageMsg ;
        int argc ;
        char *argv[] ;
#else
Text_WidgetAndOneParam(
        Boolean (*func)(),
        Boolean returnBoolean,
        Boolean paramIsString,
        char *usageMsg,
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Boolean result;
   int string;
   char buf[10];
   char * p;
   int param;
   char * errmsg;

   if (argc != 3) 
   {
      printerrf(str_nill, usageMsg, arg0, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   if (paramIsString)
      result = (*func) (w->w, argv[2]);
   else
   {
      param = strtoul(argv[2], &p, 0);
      if (p == argv[2])
      {
         errmsg = strdup(GETMESSAGE(15,72, 
                         "The parameter specified is invalid: %s"));
         printerrf(arg0, errmsg, argv[2],
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL);
         free(errmsg);
         return(1);
      }
      result = (*func) (w->w, param);
   }

   if (returnBoolean)
      return (!result);
   else
      return(0);
}


int
#ifdef _NO_PROTO
do_XmTextScroll( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextScroll(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GETMESSAGE(15,73, "Usage: %s widget lines"));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextScroll, False, False, 
                                    errmsg, argc, argv);
   free(errmsg);
   return (retVal);
}


int
#ifdef _NO_PROTO
do_XmTextSetInsertionPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetInsertionPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */

{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_POS));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextSetInsertionPosition, 
                                    False, False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextSetTopCharacter( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetTopCharacter(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_POS));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextSetTopCharacter, False, 
                                   False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextSetMaxLength( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetMaxLength(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GETMESSAGE(15,74, "Usage: %s widget maxLength"));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextSetMaxLength, False, 
                                   False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextSetString( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetString(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GETMESSAGE(15,75, "Usage: %s widget string"));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextSetString, False, True, 
                                   errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextShowPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextShowPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_POS));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextShowPosition, False, 
                                   False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextClearSelection( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextClearSelection(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_TIME));
   retVal = Text_WidgetAndOneParam((Boolean (*)())XmTextClearSelection, False, 
                                    False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextCopy( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextCopy(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_TIME));
   retVal = Text_WidgetAndOneParam(XmTextCopy, True, False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextCut( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextCut(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char * errmsg;
   int retVal;
   
   errmsg = strdup(GetSharedMsg(DT_USAGE_WIDGET_TIME));
   retVal = Text_WidgetAndOneParam(XmTextCut, True, False, errmsg, argc, argv);
   free(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmTextGetSelectionPosition( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextGetSelectionPosition(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   char buf[10];
   XmTextPosition left;
   XmTextPosition right;
   Boolean result;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,76, "Usage: %s widget variable variable"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
   {
      env_blank(argv[2]);
      env_blank(argv[3]);
      return(1);
   }

   result = XmTextGetSelectionPosition(w->w, &left, &right);
   if (result)
   {
      sprintf(buf, "%d", left);
      env_set_var(argv[2],  buf);
      sprintf(buf, "%d", right);
      env_set_var(argv[3],  buf);
   }
   else
   {
      env_blank(argv[2]);
      env_blank(argv[3]);
   }

   return(!result);
}


int
#ifdef _NO_PROTO
do_XmTextInsert( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextInsert(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition position;
   char * p;
   char * errmsg;

   if (argc != 4) 
   {
      errmsg = strdup(GETMESSAGE(15,77, "Usage: %s widget position string"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   position = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GetSharedMsg(DT_BAD_POSITION));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   XmTextInsert(w->w, position, argv[3]);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextPosToXY( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextPosToXY(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition position;
   Position x;
   Position y;
   char * p;
   Boolean result;
   char buf[10];
   char * errmsg;

   if (argc != 5) 
   {
      errmsg = strdup(GETMESSAGE(15,78, 
                      "Usage: %s widget position variable variable"));
      printerrf(str_nill, errmsg, arg0, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
   {
      env_blank(argv[3]);
      env_blank(argv[4]);
      return(1);
   }

   position = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GetSharedMsg(DT_BAD_POSITION));
      printerrf(arg0, errmsg, argv[2], NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      env_blank(argv[3]);
      env_blank(argv[4]);
      return(1);
   }

   result = XmTextPosToXY(w->w, position, &x, &y);
   if (result)
   {
      sprintf(buf, "%d", x);
      env_set_var(argv[3],  buf);
      sprintf(buf, "%d", y);
      env_set_var(argv[4],  buf);
   }
   else
   {
      env_blank(argv[3]);
      env_blank(argv[4]);
   }
   return(!result);
}


int
#ifdef _NO_PROTO
do_XmTextReplace( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextReplace(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition from;
   XmTextPosition to;
   char * p;
   char * errmsg;

   if (argc != 5) 
   {
      errmsg = strdup(GETMESSAGE(15,79, 
              "Usage: %s widget fromPosition toPosition string"));
      printerrf(str_nill, errmsg, arg0, NULL,
               NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   from = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,80, 
               "The 'from' position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   to = strtoul(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,81, 
                      "The 'to' position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   XmTextReplace(w->w, from, to, argv[4]);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextSetSelection( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetSelection(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition first;
   XmTextPosition last;
   Time time;
   char * p;
   char * errmsg;

   if (argc != 5) 
   {
      errmsg = strdup(GETMESSAGE(15,82, 
              "Usage: %s widget firstPosition lastPosition time"));
      printerrf(str_nill, errmsg, arg0, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   first = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,83, 
                      "The first position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   last = strtoul(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,84, 
                      "The last position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   time = strtoul(argv[4], &p, 0);
   if (p == argv[4])
   {
      errmsg = strdup(GETMESSAGE(15,85, "The time specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[4],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   XmTextSetSelection(w->w, first, last, time);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextXYToPos( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextXYToPos(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   Position x;
   Position y;
   XmTextPosition position;
   char * p;
   char buf[10];
   char * errmsg;

   if (argc != 5) 
   {
      errmsg = strdup(GETMESSAGE(15,86, "Usage: %s variable widget x y"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[2])) == NULL)
   {
      alt_env_set_var(argv[1], "");
      return(1);
   }

   x = strtoul(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,87, 
                      "The x position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(argv[1], "");
      return(1);
   }

   y = strtoul(argv[4], &p, 0);
   if (p == argv[4])
   {
      errmsg = strdup(GETMESSAGE(15,88, 
                      "The y position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[4],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      alt_env_set_var(argv[1], "");
      return(1);
   }

   position = XmTextXYToPos(w->w, x, y);
   sprintf(buf, "%d", position);
   alt_env_set_var(argv[1],  buf);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextSetHighlight( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextSetHighlight(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition left;
   XmTextPosition right;
   XmHighlightMode mode;
   char * p;
   XrmValue fval, tval;
   char * errmsg;

   if (argc != 5) 
   {
      errmsg = strdup(GETMESSAGE(15,89, "Usage: %s widget left right mode"));
      printerrf(str_nill, errmsg, arg0, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
      return(1);

   left = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,90, 
                      "The left position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   right = strtoul(argv[3], &p, 0);
   if (p == argv[3])
   {
      errmsg = strdup(GETMESSAGE(15,91, 
                      "The right position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[3],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   fval.addr = argv[4];
   fval.size = strlen(argv[4]);
   XtConvert(Toplevel, XtRString, &fval, "TextHighlightMode", &tval);
   if (tval.size != 0)
      mode = *((XmHighlightMode *)(tval.addr));
   else
      return(1);

   XmTextSetHighlight(w->w, left, right, mode);
   return(0);
}


int
#ifdef _NO_PROTO
do_XmTextFindString( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmTextFindString(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmTextPosition start;
   XmTextDirection direction;
   XmTextPosition position;
   char * p;
   XrmValue fval, tval;
   Boolean result;
   char buf[10];
   char * errmsg;

   if (argc != 6) 
   {
      errmsg = strdup(GETMESSAGE(15,92, 
              "Usage: %s widget start string direction variable"));
      printerrf(str_nill, errmsg, arg0, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      return(1);
   }

   if ((w = verifyTextWidget(argv[0], argv[1])) == NULL)
   {
      env_blank(argv[5]);
      return(1);
   }

   start = strtoul(argv[2], &p, 0);
   if (p == argv[2])
   {
      errmsg = strdup(GETMESSAGE(15,93, 
                      "The start position specified is invalid: %s"));
      printerrf(arg0, errmsg, argv[2],
                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      free(errmsg);
      env_blank(argv[5]);
      return(1);
   }

   fval.addr = argv[4];
   fval.size = strlen(argv[4]);
   XtConvert(Toplevel, XtRString, &fval, "TextSearchDirection", &tval);
   if (tval.size != 0)
      direction = *((XmTextDirection *)(tval.addr));
   else
   {
      env_blank(argv[5]);
      return(1);
   }

   result = XmTextFindString(w->w, start, argv[3], direction, &position);
   if (result)
   {
      sprintf(buf, "%d", position);
      env_set_var(argv[5],  buf);
   }
   else
      env_blank(argv[5]);

   return(!result);
}


static int
#ifdef _NO_PROTO
GetSubWidget( errmsg, func, argc, argv )
	char * errmsg;
	Widget (*func)();
        int argc ;
        char *argv[] ;
#else
GetSubWidget(
	char * errmsg,
	Widget (*func)(),
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char * variable = argv[1]; 
   Widget child;

   if (argc != 3) 
   {
      printerr(str_nill, errmsg, NULL);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   child = (*func)(w->w);
 
   if (child == NULL)
   {
      alt_env_set_var(variable, ""); 
      return(1);
   }

   w = ConvertWidgetToWtab(arg0, child);

   alt_env_set_var(variable,  w->widid); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmOptionLabelGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmOptionLabelGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,96, 
              "Usage: XmOptionLabelGadget variable widget"));
   retVal = GetSubWidget(errmsg, XmOptionLabelGadget, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmOptionButtonGadget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmOptionButtonGadget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,97, 
              "Usage: XmOptionButtonGadget variable widget"));
   retVal = GetSubWidget(errmsg, XmOptionButtonGadget, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmGetVisibility( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetVisibility(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t * w;
   char * results = argv[1]; 
   XrmValue f, t;
   XmVisibility res;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,98, 
         "Usage: XmGetVisibility variable widget"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[2]);
   if (w == NULL) {
        alt_env_set_var(results, ""); 
	return(1);
   }

   res = XmGetVisibility(w->w);

   f.addr = (caddr_t)&res;
   f.size = sizeof(XmVisibility);
   t.addr = NULL;
   t.size = 0;
   XtConvert(w->w, "VisibilityType", &f, XtRString, &t);
   alt_env_set_var(results,  (char *)(t.addr)); 
   return(0);
}


int
#ifdef _NO_PROTO
do_XmGetTearOffControl( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetTearOffControl(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,99, 
              "Usage: XmGetTearOffControl variable widget"));
   retVal = GetSubWidget(errmsg, XmGetTearOffControl, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmGetTabGroup( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetTabGroup(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,100, 
              "Usage: XmGetTabGroup variable widget"));
   retVal = GetSubWidget(errmsg, XmGetTabGroup, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmGetPostedFromWidget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetPostedFromWidget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,101, 
              "Usage: XmGetPostedFromWidget variable widget"));
   retVal = GetSubWidget(errmsg, XmGetPostedFromWidget, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmGetFocusWidget( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmGetFocusWidget(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   int retVal;
   char * errmsg;

   errmsg = strdup(GETMESSAGE(15,102, 
              "Usage: XmGetFocusWidget variable widget"));
   retVal = GetSubWidget(errmsg, XmGetFocusWidget, argc, argv);
   XtFree(errmsg);
   return(retVal);
}


int
#ifdef _NO_PROTO
do_XmFileSelectionDoSearch( argc, argv )
        int argc ;
        char *argv[] ;
#else
do_XmFileSelectionDoSearch(
        int argc,
        char *argv[] )
#endif /* _NO_PROTO */
{
   char *arg0 = argv[0];
   wtab_t *w;
   XmString string;
   char * errmsg;

   if (argc != 3) 
   {
      errmsg = strdup(GETMESSAGE(15,103,
              "Usage: XmFileSelectionDoSearch widget directoryMask"));
      printerr(str_nill, errmsg, NULL);
      free(errmsg);
      return(1);
   }

   w = str_to_wtab(arg0, argv[1]);
   if (w == NULL)
	return(1);

   string = XmStringCreateLtoR(argv[2], XmFONTLIST_DEFAULT_TAG);

   XmFileSelectionDoSearch(w->w, string);
   XmStringFree(string);
   return(0);
}
