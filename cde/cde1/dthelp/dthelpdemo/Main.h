/* $XConsortium: Main.h /main/cde1_maint/1 1995/07/17 21:01:01 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **  File:        Main.h
 **
 **  Project:     CDE dthelpdemo sample program.
 **
 **  Description: This is the main header file for the dthelpdemo
 **               program.  It includes globally referenced variables and
 **               defines.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994
 **      Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Main_h
#define _Main_h

#include "HelpCacheI.h"

/* Option defines for menubar help access */
#define HELP_ON_ITEM     1
#define HELP_ON_TOPIC    2
#define HELP_ON_VERSION  3

/* Option defines for Sample Buttons */
#define CREATE_SAMPLE          1
#define DESTROY_SAMPLE         2
#define CHANGE_CONTENT         3
#define CHANGE_SIZE            4
#define CHANGE_GUI             5
#define SHOW_APP_DEFINED_LINKS 6


/* Global Variables Used by our helpCache */

CacheListStruct *pCacheListHead;
CacheListStruct *pCacheListTale;
int             totalCacheNodes;

/*  Globally referenced widget variables */
extern Widget topLevel;
extern Widget mainShell;
extern Widget manWidget;


#endif /* _Main_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */






