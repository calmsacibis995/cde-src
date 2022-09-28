/* $XConsortium: Font.c /main/cde1_maint/2 1995/10/02 14:10:37 lehors $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xresource.h>
#include <Xm/Xm.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "DisplayAreaP.h"
#include "FontI.h"

/*
 * private core engine
 */
#include "StringFuncsI.h"

#ifdef NLS16
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
static  const char    *FontResources[] =
  {
/*! The serif proportional family */
/*! medium upright */
    "*p.6.roman.medium.serif.*.ISO-8859-1:  -dt-application-medium-r-normal-serif-*-80-*-*-p-*-iso8859-1",
    "*p.8.roman.medium.serif.*.ISO-8859-1:  -dt-application-medium-r-normal-serif-*-100-*-*-p-*-iso8859-1",
    "*p.10.roman.medium.serif.*.ISO-8859-1: -dt-application-medium-r-normal-serif-*-120-*-*-p-*-iso8859-1",
    "*p.12.roman.medium.serif.*.ISO-8859-1: -dt-application-medium-r-normal-serif-*-140-*-*-p-*-iso8859-1",
    "*p.14.roman.medium.serif.*.ISO-8859-1: -dt-application-medium-r-normal-serif-*-180-*-*-p-*-iso8859-1",

/*! bold upright */
    "*p.6.roman.bold.serif.*.ISO-8859-1:  -dt-application-bold-r-normal-serif-*-80-*-*-p-*-iso8859-1",
    "*p.8.roman.bold.serif.*.ISO-8859-1:  -dt-application-bold-r-normal-serif-*-100-*-*-p-*-iso8859-1",
    "*p.10.roman.bold.serif.*.ISO-8859-1: -dt-application-bold-r-normal-serif-*-120-*-*-p-*-iso8859-1",
    "*p.12.roman.bold.serif.*.ISO-8859-1: -dt-application-bold-r-normal-serif-*-140-*-*-p-*-iso8859-1",
    "*p.14.roman.bold.serif.*.ISO-8859-1: -dt-application-bold-r-normal-serif-*-180-*-*-p-*-iso8859-1",

/*! medium italic */
    "*p.6.italic.medium.serif.*.ISO-8859-1:  -dt-application-medium-i-normal-serif-*-80-*-*-p-*-iso8859-1",
    "*p.8.italic.medium.serif.*.ISO-8859-1:  -dt-application-medium-i-normal-serif-*-100-*-*-p-*-iso8859-1",
    "*p.10.italic.medium.serif.*.ISO-8859-1: -dt-application-medium-i-normal-serif-*-120-*-*-p-*-iso8859-1",
    "*p.12.italic.medium.serif.*.ISO-8859-1: -dt-application-medium-i-normal-serif-*-140-*-*-p-*-iso8859-1",
    "*p.14.italic.medium.serif.*.ISO-8859-1: -dt-application-medium-i-normal-serif-*-180-*-*-p-*-iso8859-1",

/*! bold italic */
    "*p.6.italic.bold.serif.*.ISO-8859-1:  -dt-application-bold-i-normal-serif-*-80-*-*-p-*-iso8859-1",
    "*p.8.italic.bold.serif.*.ISO-8859-1:  -dt-application-bold-i-normal-serif-*-100-*-*-p-*-iso8859-1",
    "*p.10.italic.bold.serif.*.ISO-8859-1: -dt-application-bold-i-normal-serif-*-120-*-*-p-*-iso8859-1",
    "*p.12.italic.bold.serif.*.ISO-8859-1: -dt-application-bold-i-normal-serif-*-140-*-*-p-*-iso8859-1",
    "*p.14.italic.bold.serif.*.ISO-8859-1: -dt-application-bold-i-normal-serif-*-180-*-*-p-*-iso8859-1",

/*! The sans serif proportional family */
/*! medium upright */
    "*p.6.roman.medium.sans_serif.*.ISO-8859-1:  -dt-application-medium-r-normal-sans-*-80-*-*-p-*-iso8859-1",
    "*p.8.roman.medium.sans_serif.*.ISO-8859-1:  -dt-application-medium-r-normal-sans-*-100-*-*-p-*-iso8859-1",
    "*p.10.roman.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-r-normal-sans-*-120-*-*-p-*-iso8859-1",
    "*p.12.roman.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-r-normal-sans-*-140-*-*-p-*-iso8859-1",
    "*p.14.roman.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-r-normal-sans-*-180-*-*-p-*-iso8859-1",

/*!bold upright */
    "*p.6.roman.bold.sans_serif.*.ISO-8859-1:  -dt-application-bold-r-normal-sans-*-80-*-*-p-*-iso8859-1",
    "*p.8.roman.bold.sans_serif.*.ISO-8859-1:  -dt-application-bold-r-normal-sans-*-100-*-*-p-*-iso8859-1",
    "*p.10.roman.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-r-normal-sans-*-120-*-*-p-*-iso8859-1",
    "*p.12.roman.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-r-normal-sans-*-140-*-*-p-*-iso8859-1",
    "*p.14.roman.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-r-normal-sans-*-180-*-*-p-*-iso8859-1",

/*! medium italic */
    "*p.6.italic.medium.sans_serif.*.ISO-8859-1:  -dt-application-medium-i-normal-sans-*-80-*-*-p-*-iso8859-1",
    "*p.8.italic.medium.sans_serif.*.ISO-8859-1:  -dt-application-medium-i-normal-sans-*-100-*-*-p-*-iso8859-1",
    "*p.10.italic.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-i-normal-sans-*-120-*-*-p-*-iso8859-1",
    "*p.12.italic.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-i-normal-sans-*-140-*-*-p-*-iso8859-1",
    "*p.14.italic.medium.sans_serif.*.ISO-8859-1: -dt-application-medium-i-normal-sans-*-180-*-*-p-*-iso8859-1",

/*! bold italic */
    "*p.6.italic.bold.sans_serif.*.ISO-8859-1:  -dt-application-bold-i-normal-sans-*-80-*-*-p-*-iso8859-1",
    "*p.8.italic.bold.sans_serif.*.ISO-8859-1:  -dt-application-bold-i-normal-sans-*-100-*-*-p-*-iso8859-1",
    "*p.10.italic.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-i-normal-sans-*-120-*-*-p-*-iso8859-1",
    "*p.12.italic.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-i-normal-sans-*-140-*-*-p-*-iso8859-1",
    "*p.14.italic.bold.sans_serif.*.ISO-8859-1: -dt-application-bold-i-normal-sans-*-180-*-*-p-*-iso8859-1",

/*! The monospace family - lump serif & sans serif together */
/*! medium upright */
    "*m.6.roman.medium.*.*.ISO-8859-1:  -dt-application-medium-r-normal-*-*-80-*-*-m-*-iso8859-1",
    "*m.8.roman.medium.*.*.ISO-8859-1:  -dt-application-medium-r-normal-*-*-100-*-*-m-*-iso8859-1",
    "*m.10.roman.medium.*.*.ISO-8859-1: -dt-application-medium-r-normal-*-*-120-*-*-m-*-iso8859-1",
    "*m.12.roman.medium.*.*.ISO-8859-1: -dt-application-medium-r-normal-*-*-140-*-*-m-*-iso8859-1",
    "*m.14.roman.medium.*.*.ISO-8859-1: -dt-application-medium-r-normal-*-*-180-*-*-m-*-iso8859-1",

/*! bold upright */
    "*m.6.roman.bold.*.*.ISO-8859-1:  -dt-application-bold-r-normal-*-*-80-*-*-m-*-iso8859-1",
    "*m.8.roman.bold.*.*.ISO-8859-1:  -dt-application-bold-r-normal-*-*-100-*-*-m-*-iso8859-1",
    "*m.10.roman.bold.*.*.ISO-8859-1: -dt-application-bold-r-normal-*-*-120-*-*-m-*-iso8859-1",
    "*m.12.roman.bold.*.*.ISO-8859-1: -dt-application-bold-r-normal-*-*-140-*-*-m-*-iso8859-1",
    "*m.14.roman.bold.*.*.ISO-8859-1: -dt-application-bold-r-normal-*-*-180-*-*-m-*-iso8859-1",

/*! medium italic */
    "*m.6.italic.medium.*.*.ISO-8859-1:  -dt-application-medium-i-normal-*-*-80-*-*-m-*-iso8859-1",
    "*m.8.italic.medium.*.*.ISO-8859-1:  -dt-application-medium-i-normal-*-*-100-*-*-m-*-iso8859-1",
    "*m.10.italic.medium.*.*.ISO-8859-1: -dt-application-medium-i-normal-*-*-120-*-*-m-*-iso8859-1",
    "*m.12.italic.medium.*.*.ISO-8859-1: -dt-application-medium-i-normal-*-*-140-*-*-m-*-iso8859-1",
    "*m.14.italic.medium.*.*.ISO-8859-1: -dt-application-medium-i-normal-*-*-180-*-*-m-*-iso8859-1",

/*! bold italic */
    "*m.6.italic.bold.*.*.ISO-8859-1:  -dt-application-bold-i-normal-*-*-80-*-*-m-*-iso8859-1",
    "*m.8.italic.bold.*.*.ISO-8859-1:  -dt-application-bold-i-normal-*-*-100-*-*-m-*-iso8859-1",
    "*m.10.italic.bold.*.*.ISO-8859-1: -dt-application-bold-i-normal-*-*-120-*-*-m-*-iso8859-1",
    "*m.12.italic.bold.*.*.ISO-8859-1: -dt-application-bold-i-normal-*-*-140-*-*-m-*-iso8859-1",
    "*m.14.italic.bold.*.*.ISO-8859-1: -dt-application-bold-i-normal-*-*-180-*-*-m-*-iso8859-1",

/*! SYMBOL */
    "*6.*.*.*.*.DT-SYMBOL-1:  -dt-application-medium-r-normal-*-*-80-*-*-*-*-dtsymbol-1",
    "*8.*.*.*.*.DT-SYMBOL-1:  -dt-application-medium-r-normal-*-*-100-*-*-*-*-dtsymbol-1",
    "*10.*.*.*.*.DT-SYMBOL-1: -dt-application-medium-r-normal-*-*-120-*-*-*-*-dtsymbol-1",
    "*12.*.*.*.*.DT-SYMBOL-1: -dt-application-medium-r-normal-*-*-140-*-*-*-*-dtsymbol-1",
    "*14.*.*.*.*.DT-SYMBOL-1: -dt-application-medium-r-normal-*-*-180-*-*-*-*-dtsymbol-1",

#ifdef AIX_ILS
/* for ISO-8859-2 */
    "*.6.*.*.*.*.ISO-8859-2:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-2:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-2: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-2: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-2: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for ISO-8859-5 */
    "*.6.*.*.*.*.ISO-8859-5:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-5:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-5: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-5: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-5: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for ISO-8859-6 */
    "*.6.*.*.*.*.ISO-8859-6:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-6:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-6: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-6: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-6: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for ISO-8859-7 */
    "*.6.*.*.*.*.ISO-8859-7:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-7:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-7: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-7: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-7: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for ISO-8859-8 */
    "*.6.*.*.*.*.ISO-8859-8:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-8:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-8: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-8: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-8: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for ISO-8859-9 */
    "*.6.*.*.*.*.ISO-8859-9:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.ISO-8859-9:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.ISO-8859-9: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.ISO-8859-9: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.ISO-8859-9: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for EUC-KR */
    "*.6.*.*.*.*.EUC-KR:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.EUC-KR:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.EUC-KR: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.EUC-KR: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.EUC-KR: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for EUC-JP */
    "*.6.*.*.*.*.EUC-JP:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.EUC-JP:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.EUC-JP: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.EUC-JP: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.EUC-JP: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for EUC-TW */
    "*.6.*.*.*.*.EUC-TW:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.EUC-TW:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.EUC-TW: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.EUC-TW: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.EUC-TW: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for EUC-CN */
    "*.6.*.*.*.*.EUC-CN:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.EUC-CN:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.EUC-CN: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.EUC-CN: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.EUC-CN: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for IBM-437 */
    "*.6.*.*.*.*.IBM-437:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.IBM-437:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.IBM-437: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.IBM-437: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.IBM-437: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for IBM-850 */
    "*.6.*.*.*.*.IBM-850:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.IBM-850:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.IBM-850: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.IBM-850: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.IBM-850: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for IBM-856 */
    "*.6.*.*.*.*.IBM-856:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.IBM-856:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.IBM-856: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.IBM-856: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.IBM-856: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for IBM-932 */
    "*.6.*.*.*.*.IBM-932:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.IBM-932:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.IBM-932: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.IBM-932: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.IBM-932: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for IBM-1046 */
    "*.6.*.*.*.*.IBM-1046:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.IBM-1046:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.IBM-1046: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.IBM-1046: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.IBM-1046: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
/* for UTF-8 */
    "*.6.*.*.*.*.UTF-8:  -dt-interface user-medium-r-normal-xs*-*-*-*-*-*-*-*-*:",
    "*.8.*.*.*.*.UTF-8:  -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*:",
    "*.10.*.*.*.*.UTF-8: -dt-interface user-medium-r-normal-m*-*-*-*-*-*-*-*-*:",
    "*.12.*.*.*.*.UTF-8: -dt-interface user-medium-r-normal-l*-*-*-*-*-*-*-*-*:",
    "*.14.*.*.*.*.UTF-8: -dt-interface user-medium-r-normal-xl*-*-*-*-*-*-*-*-*:",
#endif /* AIX_ILS */
    NULL,
  };

#define	GROW_SIZE	5

static	CEBoolean	QuarksInited = FALSE;
static	XrmQuark	StringFontQuark = NULL;
static	XrmName		DefaultFontQuarks[_DtHelpFontQuarkNumber];
static	XrmBinding	FontBindings[_DtHelpFontQuarkNumber] =
        { XrmBindLoosely, XrmBindLoosely, XrmBindLoosely, XrmBindLoosely,
          XrmBindLoosely, XrmBindLoosely, XrmBindLoosely };

static	DtHelpDAFSMetrics  DefaultMetrics = { FALSE, 0, 0 };
static	DtHelpDAFontInfo   DefFontInfo    = { NULL, NULL, NULL, NULL, NULL,
					NULL, NULL, NULL, 0, 0, 0, 0, 0};

/******************************************************************************
 *
 * Semi Public variables.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function: SaveFontStruct (font_struct)
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
SaveFontStruct (font_struct, font_info)
    XFontStruct		*font_struct;
    DtHelpDAFontInfo	*font_info;
#else
SaveFontStruct (
    XFontStruct		*font_struct,
    DtHelpDAFontInfo	*font_info)
#endif /* _NO_PROTO */
{
    int	fontIndex;

    fontIndex = font_info->struct_cnt++;
    if (font_info->struct_cnt >= font_info->max_structs)
      {
	font_info->max_structs += GROW_SIZE;
        if (font_info->font_structs != NULL)
            font_info->font_structs = (XFontStruct **) realloc (
			    ((char *) font_info->font_structs),
			(sizeof (XFontStruct *) * font_info->max_structs));
        else
    	    font_info->font_structs = (XFontStruct **) malloc (
			sizeof (XFontStruct *) * font_info->max_structs);
      }

    font_info->font_structs[fontIndex] = font_struct;

    return (fontIndex);
}

/******************************************************************************
 * Function: SaveFontSet (font_set)
 *
 *****************************************************************************/
static int
#ifdef _NO_PROTO
SaveFontSet (font_set, font_info)
    XFontSet		 font_set;
    DtHelpDAFontInfo	*font_info;
#else
SaveFontSet (
    XFontSet		 font_set,
    DtHelpDAFontInfo	*font_info)
#endif /* _NO_PROTO */
{
    int		 fontIndex = 0;

    fontIndex = font_info->set_cnt++;
    if (font_info->set_cnt >= font_info->max_sets)
      {
	font_info->max_sets += GROW_SIZE;
        if (font_info->font_sets != NULL)
          {
	    font_info->font_sets = (XFontSet *) realloc (
				((char *) font_info->font_sets),
				(sizeof (XFontSet) * font_info->max_sets));
	    font_info->fs_metrics = (DtHelpDAFSMetrics *) realloc (
				((char *) font_info->fs_metrics),
			(sizeof (DtHelpDAFSMetrics) * font_info->max_sets));
          }
        else
          {
	    font_info->font_sets = (XFontSet *) malloc (
				sizeof(XFontSet) * font_info->max_sets);
	    font_info->fs_metrics = (DtHelpDAFSMetrics *) malloc (
			sizeof (DtHelpDAFSMetrics) * font_info->max_sets);
          }
      }

    font_info->font_sets[fontIndex] = font_set;
    font_info->fs_metrics[fontIndex]   = DefaultMetrics;

    /*
     * want the font sets to go from -1 to -n because negative numbers
     * indicate a font set.
     */
    fontIndex++;
    return (-fontIndex);
}

/******************************************************************************
 * Function: LoadFont (char *font_string, int *ret_index)
 *
 *****************************************************************************/
static short
#ifdef _NO_PROTO
LoadFont (dpy, font_info, font_string, ret_index)
    Display	*dpy;
    DtHelpDAFontInfo	*font_info;
    char	*font_string;
    int		*ret_index;
#else
LoadFont (
    Display	*dpy,
    DtHelpDAFontInfo	*font_info,
    char	*font_string,
    int		*ret_index)
#endif /* _NO_PROTO */
{
    short	 found = False;
    short	 colon = False;
    int		 len;
    char	*junk;
    char	*strPtr;
    char	**missingFontSet = NULL;
    XFontStruct	*fontStruct = NULL;
    XFontSet     fontSet;

    strPtr = font_string;

    /*
     * if working with one byte environment, do it the fast, cheap way.
     */
    if (MB_CUR_MAX == 1)
      {
	len = strlen(strPtr);
	if (len > 0)
	    len--;

	while (len > 0 && strPtr[len] == ' ')
	    len--;

	if (strPtr[len] == ':')
	  {
	    colon = True;
	    junk  = strPtr + len;
	  }
      }
    else
      {
	/*
	 * Multibyte environment, have to go through the characters one at
	 * a time.
	 */
        while (*strPtr != '\0')
          {
	    /*
	     * Process looking for a colon.
	     */
	    len = mblen(strPtr, MB_CUR_MAX);

	    if (len == -1)
	        len = 1;
	    else if (len == 1 && *strPtr == ':')
	      {
	        colon = True;
	        junk  = strPtr;
	      }
	    else if (len != 1 || *strPtr != ' ')
	        colon = False;
	
	    strPtr += len;
          }
      }

    if (colon == False)
      {
	fontStruct = XLoadQueryFont (dpy, font_string);
	if (fontStruct != NULL)
	  {
	    *ret_index =  SaveFontStruct (fontStruct, font_info);
	    found      = True;
	  }
      }
    else
      {
	*junk = '\0';
	fontSet = XCreateFontSet (dpy, font_string, &missingFontSet,
				&len, NULL);
	if (fontSet != NULL)
	  {
	    /*
	     * want the font sets to go from -1 to -n because 0 is
	     * the default font.
	     */
	    *ret_index = SaveFontSet (fontSet, font_info);
	    found      = True;
	  }

	if (len)
	    XFreeStringList (missingFontSet);

	/*
	 * restore colon
	 */
	*junk = ':';
      }

    return found;
}

/******************************************************************************
 *
 * Semi Public Functions
 *
 *****************************************************************************/
/******************************************************************************
 * Function:	int __DtHelpFontIndexGet (Display dpy, XrmQuarkList xrm_list)
 *
 * Parameters:	xrm_list	Specifies the quark list for the font desired.
 *
 * Return Value: 0 if successful, -1 if using default font index.
 *
 * Purpose:	Find the appropriate font to use for a quark set.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpFontIndexGet (pDAS, xrm_list, ret_idx)
    DtHelpDispAreaStruct	*pDAS;
    XrmQuarkList		 xrm_list;
    int				*ret_idx;
#else
__DtHelpFontIndexGet (
    DtHelpDispAreaStruct	*pDAS,
    XrmQuarkList		 xrm_list,
    int				*ret_idx)
#endif /* _NO_PROTO */
{
    int		 	 result    = -1;
    int                  fontIndex = pDAS->font_info.def_idx;
    XrmRepresentation    retType;
    XrmValue             retValue;
    char                 buffer[10];
    char                *fontSpec;
    XrmName		 xrmList[3];
    DtHelpDAFontInfo	*fontInfo = &(pDAS->font_info);
    Display		*dpy      = XtDisplay(pDAS->dispWid);

    /*
     * look in my font data base to see if I've already processed
     * this sequence of quarks.
     */
    if (XrmQGetResource (fontInfo->font_idx_db, xrm_list, xrm_list, &retType,
							    &retValue))
      {
	fontIndex = *((int *)retValue.addr);
	result = 0;
      }
    else
      {
	/*
	 * Check my parent's data base for an XLFD spec.
	 */
	fontSpec = NULL;
	if (XrmQGetResource (XrmGetDatabase(dpy), xrm_list, xrm_list,
						    &retType, &retValue))
	  {
	    fontSpec  = ((char *) retValue.addr);
	    if (LoadFont (dpy, fontInfo, fontSpec, &fontIndex))
		result = 0;
	  }

	/*
	 * If my parent didn't have anything, check my private database.
	 */
	if (result != 0 &&
	    XrmQGetResource (fontInfo->def_font_db, xrm_list, xrm_list,
						    &retType, &retValue))
	  {
	    fontSpec  = ((char *) retValue.addr);
	    if (LoadFont (dpy, fontInfo, fontSpec, &fontIndex))
		result = 0;
	  }


	/*
	 * if unsuccessful on finding a font for this quark list.
	 * save out using the default font
	 */
	retValue.size = sizeof (int);
	retValue.addr = (caddr_t) &fontIndex;
	XrmQPutResource (&(fontInfo->font_idx_db),
				((XrmBindingList) FontBindings),
					xrm_list, _DtHelpXrmInt, &retValue);

	/*
	 * remember the character set for this font.
	 */
	sprintf (buffer, "%d", fontIndex);
	retValue.size = sizeof (XrmQuark);
	retValue.addr = (caddr_t) &xrm_list[_DT_HELP_FONT_CHAR_SET];
	xrmList[0] = XrmStringToQuark (buffer);
	xrmList[1] = XrmStringToQuark ("code_set");
	xrmList[2] = NULL;
	XrmQPutResource (&(fontInfo->font_idx_db),
				((XrmBindingList) FontBindings),
					xrmList, _DtHelpXrmQuark, &retValue);

	/*
	 * remember the language for this font.
	 */
	retValue.addr = (caddr_t) &xrm_list[_DT_HELP_FONT_LANG_TER];
	xrmList[1] = XrmStringToQuark ("language");
	XrmQPutResource (&(fontInfo->font_idx_db),
				((XrmBindingList) FontBindings),
					xrmList, _DtHelpXrmQuark, &retValue);

      }

    *ret_idx = fontIndex;
    return result;
}

/******************************************************************************
 * Function:	int __DtHelpFontCharSetQuarkGet (font_index, XrmQuark *ret_quark)
 *
 * Parameters:	font_index	Specifies the font in the font table.
 *		ret_quark	Returns the quark specifying the character
 *				set of the font.
 *
 * Return Value: Returns 0 if successful, -1 if failed.
 *
 * Purpose:	Gets the character set associated with a font.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpFontCharSetQuarkGet (pDAS, font_index, ret_quark)
    DtHelpDispAreaStruct	*pDAS;
    int       font_index;
    XrmQuark  *ret_quark;
#else
__DtHelpFontCharSetQuarkGet (
    DtHelpDispAreaStruct	*pDAS,
    int       font_index,
    XrmQuark  *ret_quark )
#endif /* _NO_PROTO */
{
    XrmRepresentation    retType;
    XrmValue             retValue;
    char                 buffer[10];
    XrmName		 xrmList[3];
    int			 result = 0;

    /*
     * quarkize the font index
     */
    sprintf (buffer, "%d", font_index);
    xrmList[0] = XrmStringToQuark (buffer);
    xrmList[1] = XrmStringToQuark ("code_set");
    xrmList[2] = NULL;

    /*
     * look in my font data base for the quark.
     */
    if (XrmQGetResource (pDAS->font_info.font_idx_db, xrmList, xrmList,
							&retType, &retValue))
	*ret_quark = *((XrmQuark *)retValue.addr);
    else
	result = -1;

    return result;
}

/******************************************************************************
 * Function:	int __DtHelpFontLangQuarkGet (font_index, XrmQuark *ret_quark)
 *
 * Parameters:	font_index	Specifies the font in the font table.
 *		ret_quark	Returns the quark specifying the character
 *				set of the font.
 *
 * Return Value: Returns 0 if successful, -1 if failed.
 *
 * Purpose:	Gets the character set associated with a font.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpFontLangQuarkGet (pDAS, font_index, ret_quark)
    DtHelpDispAreaStruct	*pDAS;
    int       font_index;
    XrmQuark  *ret_quark;
#else
__DtHelpFontLangQuarkGet (
    DtHelpDispAreaStruct	*pDAS,
    int       font_index,
    XrmQuark  *ret_quark )
#endif /* _NO_PROTO */
{
    XrmRepresentation    retType;
    XrmValue             retValue;
    char                 buffer[10];
    XrmName		 xrmList[3];
    int			 result = 0;

    /*
     * quarkize the font index
     */
    sprintf (buffer, "%d", font_index);
    xrmList[0] = XrmStringToQuark (buffer);
    xrmList[1] = XrmStringToQuark ("language");
    xrmList[2] = NULL;

    /*
     * look in my font data base for the quark.
     */
    if (XrmQGetResource (pDAS->font_info.font_idx_db, xrmList, xrmList,
							&retType, &retValue))
	*ret_quark = *((XrmQuark *)retValue.addr);
    else
	result = -1;

    return result;
}

/******************************************************************************
 * Function:	void _DtHelpCopyDefaultList (xrm_name)
 *
 * Parameters:	xrm_name	The quark list to fill out.
 *
 * Return Value: void
 *
 * Purpose:	Initializes the font data base.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpCopyDefaultList (xrm_list)
    XrmName		 *xrm_list;
#else
_DtHelpCopyDefaultList (
    XrmName		 *xrm_list)
#endif /* _NO_PROTO */
{
    int     n;

    for (n = 0; n < _DT_HELP_FONT_END + 1; n++)
        xrm_list[n] = DefaultFontQuarks[n];
}

/******************************************************************************
 * Function:	void _DtHelpGetStringQuarks (xrm_name)
 *
 * Parameters:	xrm_name	The quark list to fill out.
 *
 * Return Value: void
 *
 * Purpose:	Initializes the font data base.
 *****************************************************************************/
void
#ifdef _NO_PROTO
_DtHelpGetStringQuarks (xrm_list)
    XrmName		 *xrm_list;
#else
_DtHelpGetStringQuarks (
    XrmName		 *xrm_list)
#endif /* _NO_PROTO */
{
    _DtHelpCopyDefaultList(xrm_list);

    xrm_list[_DT_HELP_FONT_CHAR_SET] = StringFontQuark;
}

/******************************************************************************
 * Function:	void __DtHelpFontDatabaseInit (dpy, default_font, type, tmp)
 *
 * Parameters:	dpy		Specifies the connection to the display server.
 *		default_font	Specifies the base font.
 *		type		Specifies the type of font 'default_font' is.
 *
 * Return Value: void
 *
 * Purpose:	Initializes the font data base.
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
__DtHelpFontDatabaseInit (pDAS, string_font, type, user_font)
    DtHelpDispAreaStruct *pDAS;
    XtPointer		  string_font;
    XmFontType		  type;
    XFontStruct		 *user_font;
#else
__DtHelpFontDatabaseInit (
    DtHelpDispAreaStruct *pDAS,
    XtPointer		  string_font,
    XmFontType		  type,
    XFontStruct		 *user_font)
#endif /* _NO_PROTO */
{
    int     n;
    int     strIdx;
    int     found = FALSE;

    Display		*dpy = XtDisplay(pDAS->dispWid);
    DtHelpDAFontInfo	*fontInfo = &(pDAS->font_info);
    XrmValue		 retValue;
    XrmRepresentation	 retType;
    XrmName		 xrmList[_DtHelpFontQuarkNumber];

    /*
     * build our default data base
     */
    *fontInfo = DefFontInfo;
    for (n = 0; FontResources[n]; n++)
        XrmPutLineResource (&(fontInfo->def_font_db), FontResources[n]);

    /*
     * build the default quark set.
     */
    if (QuarksInited == False)
      {
        StringFontQuark = XrmStringToQuark ("String");
        DefaultFontQuarks[_DT_HELP_FONT_SPACING]  = XrmStringToQuark ("p");
        DefaultFontQuarks[_DT_HELP_FONT_SIZE]     = XrmStringToQuark ("10");
        DefaultFontQuarks[_DT_HELP_FONT_ANGLE]    = XrmStringToQuark ("roman");
        DefaultFontQuarks[_DT_HELP_FONT_WEIGHT]   = XrmStringToQuark ("medium");
        DefaultFontQuarks[_DT_HELP_FONT_TYPE]     =
						XrmStringToQuark ("sans_serif");
        DefaultFontQuarks[_DT_HELP_FONT_LANG_TER] = XrmStringToQuark ("C");
        DefaultFontQuarks[_DT_HELP_FONT_CHAR_SET] =
						XrmStringToQuark ("ISO-8859-1");
        DefaultFontQuarks[_DT_HELP_FONT_END]      = NULL;
	QuarksInited = True;
      }

    /*
     * if the string_font is null, used the default user's font for strings.
     */
    if (string_font == NULL)
      {
/*
 * WARNING...what if the user_font is null? And what type is it?
 */
        string_font = (XtPointer) user_font;
        type        = XmFONT_IS_FONT;
      }

    /*
     * save the string font in the lists
     */
    if (type == XmFONT_IS_FONTSET)
        strIdx = SaveFontSet ((XFontSet) string_font, fontInfo);
    else
        strIdx = SaveFontStruct ((XFontStruct *) string_font, fontInfo);

    /*
     * Save this idx font for ascii files and strings.
     */
    _DtHelpGetStringQuarks(xrmList);

    retValue.size = sizeof (int);
    retValue.addr = (caddr_t) &(strIdx);
    XrmQPutResource (&(fontInfo->font_idx_db), ((XrmBindingList) FontBindings),
                            xrmList, _DtHelpXrmInt, &retValue);

    /*
     * try to get the default font for help information
     * from the parent's database.
     */
    if (XrmQGetResource (XrmGetDatabase(dpy), DefaultFontQuarks,
                         DefaultFontQuarks, &retType, &retValue))
        found = LoadFont (dpy, fontInfo, ((char *) retValue.addr),
                                                    &(fontInfo->def_idx));

    /*
     * If the XLFD font spec wasn't in the parent's database,
     * look at my private database.
     */
    if (found == False &&
                XrmQGetResource (fontInfo->def_font_db,
                            DefaultFontQuarks, DefaultFontQuarks,
                            &retType, &retValue))
        found = LoadFont (dpy, fontInfo, ((char *) retValue.addr),
                                                    &(fontInfo->def_idx));

    /*
     * If still no luck, use the string font passed in.
     */
    if (found == False)
	fontInfo->def_idx = strIdx;

    /*
     * Remember we got this font for this quark string.
     */
    retValue.size = sizeof (int);
    retValue.addr = (caddr_t) &(fontInfo->def_idx);
    XrmQPutResource (&(fontInfo->font_idx_db), ((XrmBindingList) FontBindings),
                            DefaultFontQuarks, _DtHelpXrmInt, &retValue);
}

/******************************************************************************
 * Function:	XFontStruct *__DtHelpFontStructGet (int font_index );
 *
 * Parameters:	font_index	Specifies an index into the Font Structure
 *				List.
 *
 * Return Value: Returns the font structure.
 *
 * Purpose:
 *
 *****************************************************************************/
XFontStruct *
#ifdef _NO_PROTO
__DtHelpFontStructGet (font_info, font_index )
    DtHelpDAFontInfo	font_info;
    int		  font_index;
#else
__DtHelpFontStructGet (
    DtHelpDAFontInfo	font_info,
    int		  font_index)
#endif /* _NO_PROTO */
{

    if (font_index > -1 && font_index < font_info.struct_cnt)
	return font_info.font_structs[font_index];

    return NULL;
}

/******************************************************************************
 * Function:	XFontSet __DtHelpFontSetGet (int font_index);
 *
 * Parameters:	font_index	Specifies an index into the Font Set List.
 *
 * Return Value: Returns the font structure.
 *
 * Purpose:
 *
 *****************************************************************************/
XFontSet
#ifdef _NO_PROTO
__DtHelpFontSetGet (font_info, font_index )
    DtHelpDAFontInfo	font_info;
    int		 font_index;
#else
__DtHelpFontSetGet (
    DtHelpDAFontInfo	font_info,
    int		 font_index)
#endif /* _NO_PROTO */
{

    if (font_index < 0)
      {
	font_index = -font_index;
	font_index--;
	if (font_index < font_info.set_cnt)
	    return font_info.font_sets[font_index];
      }

    return NULL;
}

/******************************************************************************
 * Function:	int __DtHelpDefaultFontIndexGet ();
 *
 * Parameters:
 *
 * Return Value: Returns the index for the default font set/structure.
 *
 * Purpose:
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
__DtHelpDefaultFontIndexGet (pDAS)
    DtHelpDispAreaStruct	*pDAS;
#else
__DtHelpDefaultFontIndexGet (
    DtHelpDispAreaStruct	*pDAS)
#endif /* _NO_PROTO */
{
    return (pDAS->font_info.def_idx);
}

/******************************************************************************
 * Function:	int __DtHelpFontMetrics (int font_index, int &ret_ascent,
 *					&ret_descent);
 *
 * Parameters:
 *
 * Return Value: Returns the index for the default font set/structure.
 *
 * Purpose:
 *
 *****************************************************************************/
void
#ifdef _NO_PROTO
__DtHelpFontMetrics(font_info, font_index,
		ret_ascent, ret_descent, ret_char_width, ret_super, ret_sub)
    DtHelpDAFontInfo	font_info;
    int		  font_index;
    Unit	 *ret_ascent;
    Unit	 *ret_descent;
    Unit	 *ret_char_width;
    Unit	 *ret_super;
    Unit	 *ret_sub;
#else
__DtHelpFontMetrics (
    DtHelpDAFontInfo	font_info,
    int		  font_index,
    Unit	 *ret_ascent,
    Unit	 *ret_descent,
    Unit	 *ret_char_width,
    Unit	 *ret_super,
    Unit	 *ret_sub)
#endif /* _NO_PROTO */
{
    XFontStruct **fontList;
    char        **fontNames;
    int          fontCount;
    Unit	 maxAscent  = -1;
    Unit	 maxDescent = -1;
    Unit	 maxCharW   = -1;
    Unit	 superOffset = 0;
    Unit	 subOffset   = 0;
    int          i;

    if (font_index < 0)
      {
	font_index = -font_index;
	font_index--;
	if (font_index < font_info.set_cnt)
	  {
	    if (font_info.fs_metrics[font_index].inited == True)
	      {
		maxAscent  = font_info.fs_metrics[font_index].fm.ascent;
		maxDescent = font_info.fs_metrics[font_index].fm.descent;
		maxCharW   = font_info.fs_metrics[font_index].fm.average_width;
	      }
	    else /* if (font_info.fs_metrics[font_index].inited == False) */
	      {
		fontCount = XFontsOfFontSet(font_info.font_sets[font_index],
                                        &fontList, &fontNames);
		for (i = 0; i < fontCount; i++)
		  {
		    if (fontList[i]->ascent > maxAscent)
			maxAscent = fontList[i]->ascent;

		    if (fontList[i]->descent > maxDescent)
			maxDescent = fontList[i]->descent;

		    if (fontList[i]->max_bounds.width > maxCharW)
			maxCharW = fontList[i]->max_bounds.width;
		  }
		font_info.fs_metrics[font_index].fm.ascent  = maxAscent;
		font_info.fs_metrics[font_index].fm.descent = maxDescent;
		font_info.fs_metrics[font_index].fm.average_width = maxCharW;
		font_info.fs_metrics[font_index].inited      = True;
	      }
	  }
      }
    else if (font_index < font_info.struct_cnt)
      {
	maxAscent  = font_info.font_structs[font_index]->ascent;
	maxDescent = font_info.font_structs[font_index]->descent;
	maxCharW   = font_info.font_structs[font_index]->max_bounds.width;
      }

    superOffset = (maxAscent * 40) / 100 + ((maxAscent % 40) > 19 ? 1 : 0);
    subOffset   = superOffset;

    if (ret_ascent)
        *ret_ascent  = maxAscent;
    if (ret_descent)
        *ret_descent = maxDescent;
    if (ret_char_width)
	*ret_char_width = maxCharW;
    if (ret_super)
	*ret_super = superOffset;
    if (ret_sub)
	*ret_sub = subOffset;
}

/******************************************************************************
 * Function:	int _DtHelpGetExactFontIndex(Display dpy, char *xlfd_spec)
 *
 * Parameters:
 *
 * Return Value: 0 if successful, -1 if using default font index.
 *
 * Purpose:	Try to load the specified font.
 *
 *****************************************************************************/
int
#ifdef _NO_PROTO
_DtHelpGetExactFontIndex (pDAS, lang, char_set, xlfd_spec, ret_idx)
    DtHelpDispAreaStruct	*pDAS;
    const char			*lang;
    const char			*char_set;
    char		 	*xlfd_spec;
    int				*ret_idx;
#else
_DtHelpGetExactFontIndex (
    DtHelpDispAreaStruct	*pDAS,
    const char			*lang,
    const char			*char_set,
    char		 	*xlfd_spec,
    int				*ret_idx)
#endif /* _NO_PROTO */
{
    int			 i = 0;
    int		 	 result    = -1;
    int                  fontIndex = pDAS->font_info.def_idx;
    char		**nameList;
    DtHelpDAFontInfo	*fontInfo = &(pDAS->font_info);
    Display		*dpy      = XtDisplay(pDAS->dispWid);

    /*
     * look in my font data base to see if I've already processed
     * this xlfd_spec.
     */
#ifndef	REALLOC_INCR
#define	REALLOC_INCR	10
#endif
    nameList = fontInfo->exact_fonts;
    if (NULL != nameList)
      {
        for (i = 0; NULL != *nameList && strcmp(*nameList, xlfd_spec); i++)
	    nameList++;
      }

    if (NULL != nameList)
      {
	fontIndex = fontInfo->exact_idx[i];
	result = 0;
      }
    else if (LoadFont(dpy, fontInfo, xlfd_spec, &fontIndex))
      {
	fontInfo->exact_fonts = (char **) _DtHelpCeAddPtrToArray(
						(void **)fontInfo->exact_fonts,
						(void *) xlfd_spec);
	if (NULL != fontInfo->exact_fonts)
	  {
	    if (NULL == fontInfo->exact_idx)
		fontInfo->exact_idx = (int *) malloc(sizeof(int)*REALLOC_INCR);
	    else if ((i + 1) % REALLOC_INCR == 0)
		fontInfo->exact_idx = (int *) realloc(fontInfo->exact_idx,
					sizeof(int) * (i + 1 + REALLOC_INCR));

	    if (NULL != fontInfo->exact_idx)
	      {
		char       buffer[10];
		XrmValue   retValue;
		XrmName    xrmList[3];
		XrmQuark   myQuark;

		fontInfo->exact_idx[i] = fontIndex;
	        result = 0;

		/*
		 * remember the character set for this font.
		 */
		myQuark = XrmStringToQuark(char_set);
		sprintf (buffer, "%d", fontIndex);
		retValue.size = sizeof (XrmQuark);
		retValue.addr = (caddr_t) &myQuark;
		xrmList[0] = XrmStringToQuark (buffer);
		xrmList[1] = XrmStringToQuark ("code_set");
		xrmList[2] = NULL;
		XrmQPutResource (&(fontInfo->font_idx_db),
					((XrmBindingList) FontBindings),
					xrmList, _DtHelpXrmQuark, &retValue);
	
		/*
		 * remember the language for this font.
		 */
		myQuark    = XrmStringToQuark(lang);
		xrmList[1] = XrmStringToQuark ("language");
		XrmQPutResource (&(fontInfo->font_idx_db),
					((XrmBindingList) FontBindings),
					xrmList, _DtHelpXrmQuark, &retValue);
	      }
	  }
      }

    *ret_idx = fontIndex;
    return result;
}

