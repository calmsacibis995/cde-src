/* $XConsortium: MergeRes.h /main/cde1_maint/1 1995/07/17 18:13:45 drk $ */
/* -*-C-*-
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        MergeRes.h
 **
 **   Project:     DT
 **
 **   Description: Public include file for the resource merging utility.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _MergeRes_h
#define _MergeRes_h

#ifdef _NO_PROTO
void MergeDtResources() ;
#else
void MergeDtResources(Display *display) ;
#endif

/* Do not add anything after this endif. */
#endif /* _MergeRes_h */
