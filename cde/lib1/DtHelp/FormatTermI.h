/* $XConsortium: FormatTermI.h /main/cde1_maint/1 1995/07/17 17:33:18 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FormatTermI.h
 **
 **   Project:     Terminal access to help text
 **
 **   Description: Header file for FormatTerm.h
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
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
#ifndef _DtFormatTermI_h
#define _DtFormatTermI_h

#ifdef _NO_PROTO
int _DtHelpTermCreateCanvas();
int _DtHelpTermGetTopicData();
int _DtHelpGetTopicDataHandles();
#else
int _DtHelpTermCreateCanvas (
        int               maxColumns,
        CanvasHandle *    ret_canvas);

int _DtHelpTermGetTopicData(
    CanvasHandle      canvasHandle,
    VolumeHandle      volHandle,
    char *            locationId,
    char * * *        helpList,
    DtHelpHyperLines ** hyperList);

int _DtHelpGetTopicDataHandles(
    CanvasHandle *    ret_canvasHandle,
    VolumeHandle *    ret_volHandle);
#endif



#endif /* _DtFormatTermI_h */
