/* $XConsortium: GlobSearchI.h /main/cde1_maint/1 1995/07/17 17:33:55 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        GlobSearchI.h
 **
 **   Project:     DtHelp Project
 **
 **   Description: Builds and displays an instance of a DtHelp GlobSearch
 **                Dialog.
 ** 
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _GlobSearchI_h
#define _GlobSearchI_h

/* global var */
extern char _DtHelpDefaultSrchHitPrefixFont[];

#ifdef _NO_PROTO
void _DtHelpGlobSrchDisplayDialog();
void _DtHelpGlobSrchUpdateCurVol();
void  _DtHelpGlobSrchCleanAndClose();
void  _DtHelpGlobSrchInitVars();
#else
void _DtHelpGlobSrchDisplayDialog(
    Widget owner,
    char * searchWord,
    char * curVolume);
void _DtHelpGlobSrchUpdateCurVol(
    Widget widget);
void  _DtHelpGlobSrchCleanAndClose(
       _DtHelpGlobSearchStuff * srch,
       Boolean               destroy);
void  _DtHelpGlobSrchInitVars(
       _DtHelpGlobSearchStuff * srch);
#endif 

#endif /* _GlobSearchI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
